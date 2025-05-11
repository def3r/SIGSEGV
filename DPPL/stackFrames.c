// gcc -S -m32 stackFrames.c

void bar(int a, int b) {
  int x, y;

  x = 69;
  y = 420;
  return;
}

void foo() {
  // The rvals 420 and 69 need stack space
  // to be allocated as args, thus allocate
  // 4 + 4 + 4 + 4 = 16 bytes for local vars
  // 
  // and definitely some alignment by the compiler
  bar(420, 69);

  int a = 100;
  int b = 200;
}
