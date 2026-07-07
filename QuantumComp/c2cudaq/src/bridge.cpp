#include <c2cudaq.h>
#include <c2cudaq/internal.h>
#include <utility>
#include <vector>

using namespace c2cudaq;

// Bridge between the LLVM maxcut-cpp-pass and c2q_maxcut.
//
// The pass replaces a detected brute-force MaxCut loop nest with:
//
//   %r = call i32 @maxcut_impl(ptr %subsets, ptr %edges, ptr %best_S)
//
// where:
//   %subsets  - vector<vector<int>>* of precomputed subsets (ignored; QAOA
//               derives the partition space from qubit count, not enumeration)
//   %edges    - vector<pair<int,int>>* of graph edges (unweighted: w = 1.0)
//   %best_S   - vector<int>* to write the best partition into, or null
//
// Returns: cut value (number of crossing edges for the best partition found).
extern "C" int maxcut_impl(const std::vector<std::vector<int>>* /*subsets*/,
                           const std::vector<std::pair<int, int>>* edges,
                           std::vector<int>* best_S) {
  // Build Graph from unweighted edge list.
  // num_nodes is derived as max endpoint index + 1.
  Graph g;
  g.num_nodes = 0;
  for (auto& [u, v] : *edges) {
    g.num_nodes = std::max(g.num_nodes, std::max(u, v) + 1);
    g.edges.emplace_back(u, v, 1.0);
  }
  if (g.num_nodes == 0 || g.edges.empty())
    return 0;

  GraphResult r = c2q_maxcut(g);

  if (best_S)
    *best_S = decode_partition(r.partition);

  return r.objective;
}
