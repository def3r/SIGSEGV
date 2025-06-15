#define DEFER_IMPLEMENTATION
#include "defer.h"
#include <stdlib.h>

// *(void **)localDeferStack->deferArgs = (void *)str;
int main() {
  DeferSetUp();

  Println("Hello noobs");

  // No memory leakage here at all!
  // Try Defer(FreeVerbose, str) to
  // see if all 3 loc get freed
  char *
  str = malloc(69);
  Defer(Free, str);
  str = malloc(69);
  Defer(Free, str);
  str = malloc(69);
  Defer(Free, str);

  Return(0);
}
