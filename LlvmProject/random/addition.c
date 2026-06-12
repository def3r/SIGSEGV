// using llvm 15 (yes, 15)
//
// Emit the llvm IR:
// $ clang -emit-llvm -O0 -S addition.c -o addition.ll
//
// Perform the mem2reg pass:
// $ clang -emit-llvm -S -Xclang -disable-O0-optnone addition.c -o addition.ll
// $ opt -passes="mem2reg,inline" -S addition.ll -o after_mem2reg.ll
//
// -disable-O0-optnone :: by default clang -O0 adds a `optnone` attr to every
// function, using this flag the attr is not added.
//
// Architecture Constraints:
// 1. Find the structure containing: instr add
// 2. Want to avoid recursive funcs
// 3. How much to produce, every single new addn instruction needs its own
// quantum circuit
// 4. 

int sum(int a, int b) {
  return a + b;
}

int main() {
  int a = 7;
  int b = 11;
  int c = sum(a, b);  // structure 1
  int d = a + b;      // structure 2
  return c;
}
