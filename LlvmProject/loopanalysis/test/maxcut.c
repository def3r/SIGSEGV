// $ clang -emit-llvm -S -Xclang -disable-O0-optnone maxcut.c -o maxcut_raw.ll
// $ opt -passes="mem2reg" -S maxcut_raw.ll -o maxcut_mem2reg.ll
// $ opt -passes="default<O1>" -S maxcut_raw.ll -o maxcut_O1.ll

int maxcut(int edges[][2], int num_edges, int* partition) {
  int cut = 0;
  for (int i = 0; i < num_edges; i++) {
    int u = edges[i][0];
    int v = edges[i][1];
    if (partition[u] != partition[v])
      cut++;
  }
  return cut;
}
