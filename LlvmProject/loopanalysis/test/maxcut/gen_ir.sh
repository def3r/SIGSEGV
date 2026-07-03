#!/usr/bin/env bash
# gen_ir.sh — Compile all test cases through the same pipeline as the reference,
# then run maxcut-cpp-pass and report whether each case was detected.
#
# Pipeline (per the project README):
#   1. clang -S -emit-llvm -O0 -fno-inline -Xclang -disable-O0-optnone
#            -fno-discard-value-names <src>
#   2. llvm-extract -func=<mangled> <src>.ll -S -o <src>_ex.ll
#   3. opt -passes="sroa,mem2reg,loop-simplify,lcssa,indvars,simplifycfg,
#                   instcombine,simplifycfg,instcombine" <src>_ex.ll -S -o <src>_opt.ll
#   4. opt -load-pass-plugin ../../build/MinPass.so -passes=maxcut-cpp-pass
#           -disable-output <src>_opt.ll 2>&1

set -euo pipefail

CLANG="${CLANG:-clang}"
OPT="${OPT:-opt}"
LLVM_EXTRACT="${LLVM_EXTRACT:-llvm-extract}"
PLUGIN="$(dirname "$0")/../../build/MinPass.so"
OPT_PASSES="sroa,mem2reg,loop-simplify,lcssa,indvars,simplifycfg,instcombine,simplifycfg,instcombine"

RED='\033[0;31m'
GRN='\033[0;32m'
YLW='\033[0;33m'
RST='\033[0m'

# process <src.cpp> <func_grep_pattern> <expect: DETECT|REJECT>
process() {
    local src="$1"
    local pattern="$2"
    local expect="$3"
    local base="${src%.cpp}"

    printf "%-35s  " "$src"

    # Step 1: compile to unoptimized IR
    "$CLANG" -S -emit-llvm -O0 -fno-inline \
        -Xclang -disable-O0-optnone \
        -fno-discard-value-names \
        "$src" -o "${base}.ll" 2>/dev/null

    # Step 2: extract the target function (find its mangled name by pattern)
    local mangled
    mangled=$(grep -o '@_Z[^(]*' "${base}.ll" | \
              grep "$pattern" | head -1 | tr -d '@')
    if [[ -z "$mangled" ]]; then
        printf "${RED}ERROR: could not find function matching '%s' in %s.ll${RST}\n" "$pattern" "$base"
        return 1
    fi

    "$LLVM_EXTRACT" -func="$mangled" "${base}.ll" -S -o "${base}_ex.ll" 2>/dev/null

    # Step 3: optimize (same passes as reference)
    "$OPT" -passes="$OPT_PASSES" "${base}_ex.ll" -S -o "${base}_opt.ll" 2>/dev/null

    # Step 4: run the pass; capture stderr (all pass output goes to errs())
    local output
    output=$("$OPT" -load-pass-plugin "$PLUGIN" \
                    -passes=maxcut-cpp-pass \
                    -disable-output \
                    "${base}_opt.ll" 2>&1)

    local detected=0
    echo "$output" | grep -q "MaxCut-CPP pattern matched" && detected=1

    if [[ "$expect" == "DETECT" ]]; then
        if [[ $detected -eq 1 ]]; then
            printf "${GRN}PASS${RST}  (detected as expected)\n"
        else
            printf "${RED}FAIL${RST}  (should have been detected — FALSE NEGATIVE)\n"
        fi
    elif [[ "$expect" == "REJECT" ]]; then
        if [[ $detected -eq 0 ]]; then
            printf "${GRN}PASS${RST}  (not detected as expected)\n"
        else
            printf "${RED}FAIL${RST}  (incorrectly detected — FALSE POSITIVE)\n"
        fi
    elif [[ "$expect" == "FP-BUG" ]]; then
        # We expect the pass to fire (incorrectly) — documenting the bug
        if [[ $detected -eq 1 ]]; then
            printf "${YLW}BUG ${RST}  (detected — FALSE POSITIVE confirmed)\n"
        else
            printf "${GRN}FIXED${RST} (no longer a false positive — bug may be fixed)\n"
        fi
    elif [[ "$expect" == "FN-BUG" ]]; then
        # We expect the pass to miss (incorrectly) — documenting the bug
        if [[ $detected -eq 0 ]]; then
            printf "${YLW}BUG ${RST}  (not detected — FALSE NEGATIVE confirmed)\n"
        else
            printf "${GRN}FIXED${RST} (now detected — bug may be fixed)\n"
        fi
    fi
}

cd "$(dirname "$0")"

echo "=============================================="
echo " MaxCut Pass Test Suite"
echo "=============================================="
echo ""
echo "Legend:"
echo "  PASS   = correct behavior"
echo "  FAIL   = unexpected behavior"
echo "  BUG    = incorrect behavior (known bug, documented)"
echo "  FIXED  = previously-buggy case now behaves correctly"
echo ""
echo "--- True Positives (should be DETECTED) ---"
# Function name substring used to grep the .ll file for the mangled symbol
process tp_basic.cpp    "compute_maxcut[^_]"  DETECT
echo ""
echo "--- True Negatives (should NOT be detected) ---"
process tn_min_cut.cpp         "compute_mincut"       REJECT
process tn_sum.cpp             "compute_total_cut"    REJECT
process fp_diff_containers.cpp "compute_bip_cut"      REJECT
process fp_three_finds.cpp     "compute_hyper_cut"    REJECT
process fp_score_no_max.cpp    "score_cut_only"       REJECT
process fn_sge_compare.cpp     "compute_maxcut_sge"   REJECT
echo ""
echo "--- False Positives (bugs: should reject, currently detects) ---"
process tn_directed.cpp "compute_directed"    FP-BUG
process tn_both_in.cpp  "compute_max_int"     FP-BUG
echo ""
echo "--- False Negatives (bugs/limitations: should detect, currently misses) ---"
process tp_val_only.cpp        "compute_maxcut_v"         FN-BUG
process fn_long_edges.cpp      "compute_maxcut_ll"        FN-BUG
process fn_uint_subset.cpp     "compute_maxcut_uint"      FN-BUG
process fn_nonzero_init.cpp    "compute_maxcut_sentinel"  FN-BUG
process fn_weight2.cpp         "compute_maxcut_w2"        FN-BUG
process fn_count_not_find.cpp  "compute_maxcut_cnt"       FN-BUG
echo ""
echo "=============================================="
