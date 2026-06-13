#include <stdio.h>

#define mult(n, m) \
  a = n, b = m;    \
  d = a * b

int multiply(int a, int b) {
  return a * b;
}

int main() {
  int a, b, c, d;
  c = multiply(a, b);

  mult(2, 3);
  printf("%d x %d = %d; ", a, b, d);

  mult(7, 11);
  printf("%d x %d = %d; ", a, b, d);

  mult(13, 11);
  printf("%d x %d = %d; ", a, b, d);

  mult(113, 11);
  printf("%d x %d = %d; ", a, b, d);
}
