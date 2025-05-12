#include <stdio.h>
#include <stdarg.h>
// stdarg
// va_list   -> conatins the list of variadic args
// va_start  -> enables access to variadic args
//      param: va_list instance,
//             named parameter preceding first var arg (optional in some c vers)
// va_arg    -> access the next var arg; it returns the val in da list
// va_end    -> ends traversal of var args


// __VA_ARGS__ -> a compiler defined macro
//                has no definition in a library, but is known to compiler
//                it refers to the ...

// crazy workaround for passing NULL at end
#define execl(...) execl_implementation(__VA_ARGS__, NULL)

const void execl_implementation(const char* path, ...) {
  printf("%s\t", path);

  va_list args;
  va_start(args, path);
  const char* arg = va_arg(args, const char*);
  while (arg != NULL) {
    printf("%s\t", arg);
    arg = va_arg(args, char*);
  }
  va_end(args);

  printf("\n");
  return;
}

int addInts(int count, ...) {
  printf("%d\n---\n", count);

  va_list args;
  va_start(args, count);
  int sum;
  while (count-- > 0) {
    sum += va_arg(args, int);
    printf("%d\n", sum);
  }
  va_end(args);
  return sum;
}

int main() {
  addInts(3, 3, 6, 9);

  execl("bin/ls", "ls", "../");

  // what in the american pope is ts shyt 😭🙏
  const int a = 5, b = 9;
  const const const const int* p = &a;
  // this const const const const const is valid
  // Apparently the compiler silently drops the redundant
  // qualifiers

  printf("*p = %d\n", *p);
  p = &b;
  printf("*p = %d\n", *p);

  // to make a pointer const:
  const int* const q = &b;

  // ts again 😭🙏
  const const const int const const const r = b;

  return 0;
}
