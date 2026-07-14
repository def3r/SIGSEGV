#!/usr/bin/env bash
# run_integration.sh - full pipeline: compile → pass → link → run
#
# Steps:
#   1. clang: C++ source → unoptimised LLVM IR
#   2. opt:   optimisation passes + maxcut-cpp-pass → transformed IR
#              (brute-force loops replaced with @maxcut_impl)
#   3. clang: transformed IR → object file
#   4. link:  object + libc2cudaq.a → binary using cudaq's clang-16 directly
#   5. run the binary
#
# Why not nvq++?
#   nvq++ is just a shell wrapper around cudaq's clang-16 that adds -L/-l flags
#   for the cudaq runtime (lcudaq, lnvqir, lnvqir-qpp, etc.).  We invoke
#   cudaq's clang-16 directly so we can control the exact link line without
#   the nvq++ wrapper adding unwanted flags.
#
# Why system clang++ to link (not cudaq's clang-16)?
#   The pass was built to match libstdc++ IR (__gnu_cxx::__normal_iterator).
#   libc++ (cudaq's runtime) uses __wrap_iter and the pass doesn't fire on it.
#   So we compile with system clang++ (libstdc++) for steps 1-3, then link
#   with system clang++ which implicitly brings in libstdc++.so — satisfying
#   all libstdc++ symbols while also pulling in the cudaq runtime libs via -L/-l.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
C2CUDAQ_ROOT="/home/def3r/def3r/SIGSEGV/QuantumComp/c2cudaq"

CLANG="${CLANG:-clang++}"
OPT="${OPT:-/home/def3r/def3r/llvm-project/install/bin/opt}"
CUDAQ_DIR="${CUDAQ_DIR:-/home/def3r/.cudaq}"

PLUGIN="$PROJECT_ROOT/build/MinPass.so"
LIB="$C2CUDAQ_ROOT/build/libc2cudaq.a"

OPT_PASSES="sroa,mem2reg,loop-simplify,lcssa,indvars,simplifycfg,instcombine<no-verify-fixpoint>,simplifycfg,instcombine<no-verify-fixpoint>"

SRC="$SCRIPT_DIR/maxcut_e2e.cpp"
BUILD="$SCRIPT_DIR/build"
mkdir -p "$BUILD"

echo "=========================================="
echo " Compiling "
echo "=========================================="
echo ""

# ── Step 1: C++ → unoptimised IR ─────────────────────────────────────────────
echo "[1/4] clang: source → LLVM IR"
"$CLANG" -S -emit-llvm -O0 -fno-inline \
    -Xclang -disable-O0-optnone \
    -fno-discard-value-names \
    -std=c++20 \
    "$SRC" -o "$BUILD/maxcut_e2e.ll" 2>&1
echo "      $BUILD/maxcut_e2e.ll"

# ── Step 2: optimise + apply pass ────────────────────────────────────────────
echo ""
echo "[2/4] opt: passes + maxcut-cpp-pass → transformed IR"
"$OPT" -load-pass-plugin "$PLUGIN" \
    -passes="$OPT_PASSES,maxcut-cpp-pass" \
    "$BUILD/maxcut_e2e.ll" -S -o "$BUILD/maxcut_e2e_transformed.ll" 2>&1
echo "      $BUILD/maxcut_e2e_transformed.ll"

echo ""
if grep -q "call i32 @maxcut_impl" "$BUILD/maxcut_e2e_transformed.ll"; then
    echo "      ✓  @maxcut_impl injected — pass fired"
    echo ""
    echo "      Call site:"
    grep "maxcut_impl" "$BUILD/maxcut_e2e_transformed.ll" \
        | grep -v "^;" | grep -v "@\.str" | sed 's/^/        /'
else
    echo "      ✗  @maxcut_impl NOT found — pass did not fire"
    exit 1
fi

# ── Step 3: transformed IR → object ──────────────────────────────────────────
echo ""
echo "[3/4] clang: transformed IR → object"
"$CLANG" -c "$BUILD/maxcut_e2e_transformed.ll" -o "$BUILD/maxcut_e2e.o" 2>&1
echo "      $BUILD/maxcut_e2e.o"

# ── Step 4: link with cudaq runtime ──────────────────────────────────────────
echo ""
echo "[4/4] clang: link with libc2cudaq.a + cudaq runtime"
"$CLANG" \
    -Wl,-rpath,"$CUDAQ_DIR/lib" \
    -L"$CUDAQ_DIR/lib" \
    "$BUILD/maxcut_e2e.o" \
    "$LIB" \
    -lc++ -lcudaq -lcudaq-logger -lcudaq-common \
    -lcudaq-ensmallen -lcudaq-nlopt -lcudaq-operator \
    -lcudaq-mlir-runtime -lcudaq-builder \
    -lcudaq-em-default -lcudaq-platform-default \
    -lnvqir -lnvqir-qpp \
    -o "$BUILD/maxcut_e2e" 2>&1
echo "      $BUILD/maxcut_e2e"

# ── Run ───────────────────────────────────────────────────────────────────────
echo ""
echo "=========================================="
echo " Running"
echo "=========================================="
echo ""
"$BUILD/maxcut_e2e"
