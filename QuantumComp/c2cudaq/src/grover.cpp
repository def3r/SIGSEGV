#include <c2cudaq.h>
#include <cudaq.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>

// ── QFT helpers (repeated here; no shared header for __qpu__ structs) ─────────
struct factor_qft_fwd {
    void operator()(cudaq::qview<> q) __qpu__ {
        int M = q.size();
        for (int si = 0; si < M; ++si) {
            int i = M - 1 - si;
            h(q[i]);
            for (int sj = 0; sj < i; ++sj) {
                int j = i - 1 - sj;
                double angle = M_PI / (double)(1 << (i - j));
                r1<cudaq::ctrl>(angle, q[j], q[i]);
            }
        }
    }
};

struct factor_qft_inv {
    __qpu__ void operator()(cudaq::qview<> q) {
        int M = q.size();
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < i; ++j) {
                double angle = -M_PI / (double)(1 << (i - j));
                r1<cudaq::ctrl>(angle, q[j], q[i]);
            }
            h(q[i]);
        }
    }
};

// ── Quantum-verified factor kernel ────────────────────────────────────────────
// Computes a * b via QFT multiplier and measures the product register.
// Used to verify a candidate (a, b) factor pair for n = a*b.
// Note: not full Grover — uses quantum multiplication for verification only.
//       Full Grover uncomputation requires explicit inverse mul (simulator-only
//       workaround via cudaq::reset would be needed; see README).
struct verify_mul_kernel {
    __qpu__ void operator()(int a, int b, int sa, int sb) {
        cudaq::qvector ra(sa), rb(sb);
        int acc_size = sa + sb;
        cudaq::qvector acc(acc_size);

        for (int i = 0; i < sa; ++i) if ((a >> i) & 1) x(ra[i]);
        for (int j = 0; j < sb; ++j) if ((b >> j) & 1) x(rb[j]);

        factor_qft_fwd{}(acc);

        for (int i = 0; i < sa; ++i)
            for (int j = 0; j < sb; ++j) {
                int p = i + j;
                for (int k = p; k < acc_size; ++k) {
                    double angle = M_PI / (double)(1LL << (k - p));
                    r1<cudaq::ctrl>(angle, ra[i], rb[j], acc[k]);
                }
            }

        factor_qft_inv{}(acc);
        mz(acc);
    }
};

// ── Decode product bitstring → integer ───────────────────────────────────────
static int64_t decode_product(const std::string& bits) {
    std::string rev = bits;
    std::reverse(rev.begin(), rev.end());
    return (int64_t)std::stoull(rev, nullptr, 2);
}

// ── Public API ────────────────────────────────────────────────────────────────
std::pair<int64_t, int64_t> c2q_factor(int64_t n) {
    if (n < 4)
        throw std::invalid_argument("c2q_factor: n must be >= 4");

    int num_result = 0;
    { int64_t tmp = n; while (tmp) { ++num_result; tmp >>= 1; } }
    int num_state  = (num_result + 1) / 2;
    int total_q    = 2 * num_state + num_result;

    if (num_result > 14)
        throw std::runtime_error(
            "c2q_factor: n too large — needs " + std::to_string(num_result) +
            "-bit factors; limit is n < 2^14 (qubits: 4*ceil(bits/2) <= 28)");

    // Iterate over candidate factors and quantum-verify each with exact bit widths.
    int64_t limit = (int64_t)std::sqrt((double)n) + 1;
    for (int64_t a = 2; a <= limit; ++a) {
        if (n % a != 0) continue;
        int64_t b = n / a;

        // Compute exact bit widths for a and b so no truncation occurs.
        int sa = 0; { int64_t tmp = a; while (tmp) { ++sa; tmp >>= 1; } }
        int sb = 0; { int64_t tmp = b; while (tmp) { ++sb; tmp >>= 1; } }
        int verify_q = sa + sb + sa + sb; // ra + rb + acc(sa+sb)
        if (verify_q > 28) {
            // Fall back to classical check when circuit would exceed simulator limit.
            return {a, b};
        }

        auto result  = cudaq::sample(verify_mul_kernel{}, (int)a, (int)b, sa, sb);
        int64_t prod = decode_product(result.most_probable());

        if (prod == n) return {a, b};
    }

    return {1, n}; // trivial — n is prime
}
