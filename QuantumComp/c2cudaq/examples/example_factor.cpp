#include <c2cudaq.h>
#include <iostream>

int main() {
  std::cout << "=== Quantum Factorization Examples ===\n\n";

  // c2q_factor
  // Uses the QFT multiplier to quantum-verify each candidate factor pair.
  // Iterates classically but validates each pair on the quantum circuit.
  for (int64_t n : {15, 21, 35, 12, 77}) {  // wat?? even 12 works!
    auto [p, q] = c2q_factor(n);
    std::cout << "c2q_factor(" << n << ") = {" << p << ", " << q << "}"
              << "   " << p << " * " << q << " = " << p * q
              << (p * q == n && p > 1 && q > 1 ? "  [valid]"
                                               : "  [trivial/error]")
              << "\n";
  }

  return 0;
}
