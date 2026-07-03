// tp_basic.cpp — TRUE POSITIVE
// Same algorithm as the reference (maxcut_actual.cpp), different
// function/variable names. Expected: DETECTED by maxcut-cpp-pass.
//
// What's the same as reference:
//   - Outer loop: iterate all subsets, track maximum cut
//   - Inner loop: iterate edges, two std::find calls, XOR condition, add-1
//   accumulator
//   - Latch GEP strides: 8 (pair<int,int>) and 24 (vector<int>)
//   - icmp sgt for max update, phi merge after branch
//
// Extract target function for IR analysis:
//   llvm-extract
//   -func=_Z14compute_maxcutSt6vectorIiSaIiEES_ISt4pairIiiESaIS3_EE

#include <algorithm>
#include <vector>
using namespace std;

// Declared, not defined — appears as external declaration in extracted IR.
// Structurally equivalent to setsubs() in the reference.
vector<vector<int>> enumerate_subsets(vector<int>& vertices);

// Same algorithm as reference actual(), but renamed throughout.
// The pass should detect this as MaxCut.
int compute_maxcut(vector<int> nodes, vector<pair<int, int>> edges) {
  vector<vector<int>> partitions = enumerate_subsets(nodes);

  int best_val = 0;
  vector<pair<int, int>> best_edges;
  vector<int> best_S;

  for (auto S : partitions) {
    int crossing = 0;
    vector<pair<int, int>> crossing_edges;

    for (auto [a, b] : edges) {
      bool a_in = find(S.begin(), S.end(), a) != S.end();
      bool b_in = find(S.begin(), S.end(), b) != S.end();
      // XOR: exactly one endpoint in S
      if ((a_in && !b_in) || (!a_in && b_in)) {
        crossing++;
        crossing_edges.push_back({a, b});
      }
    }

    if (crossing > best_val) {
      best_val = crossing;
      best_edges = crossing_edges;
      best_S = S;
    }
  }

  return best_val;
}
