#include <stdio.h>

int missingInt(int *arr, int size, int n) {
  // F = a^b^c^d
  // G = a^b^d
  // F ^ G = (a^b^c^d)^(a^b^d)
  //       = (a^a)^(b^b)^c^(d^d)
  //       = 0^0^c^0
  //       = c
  int cleanXOR = 0, arrXOR = 0;
  for (int i = 1; i <= n; i++) {
    cleanXOR ^= i;
  }
  for (int i = 0; i < size; i++) {
    arrXOR ^= arr[i];
  }
  return cleanXOR ^ arrXOR;
}

int main() {
  // In-place Swapping
  //      9           5
  int a = 0b1001, b = 0b0101;
  printf("a = %d, b = %d\n", a, b);
  // a = 1001 ^ 0101 = 1100
  a = a ^ b;
  // b = 1100 ^ 0101 = 1001
  b = a ^ b;
  // a = 1100 ^ 1001 = 0101
  a = a ^ b;
  printf("a = %d, b = %d\n", a, b);

  printf("%d\n", missingInt((int[4]){1, 2, 3, 3}, 4, 3));
}
