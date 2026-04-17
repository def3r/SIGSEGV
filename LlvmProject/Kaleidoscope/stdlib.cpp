#include <stdio.h>

extern "C" double printd(double X) {
  fprintf(stderr, "%f\n", X);
  return 0;
}
