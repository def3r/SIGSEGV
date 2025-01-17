#include <stdio.h>

#define MAXCSE

// gcc --save-temps main.c
// strace -i ./a.out
// gcc -static main.c

int main() {
  printf("1 2 3 4");

  #ifdef MAXCSE
  printf("lol");
  #endif

  #undef MAXCSE

  #ifdef MAXCSE
  printf("not vis");
  #endif

  return 0;
}
