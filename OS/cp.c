#include <stdio.h>
#include <string.h>

char *filter(char *c, char *src) {
  if (!c) {
    c = src;
  } else {
    c++;
  }

  return c;
}

int main(int argc, char **argv) {
  if (argc == 1) {
    printf("No file provided");
    return 1;
  } else if (argc == 2) {
    printf("No destination provided");
    return 1;
  } else if (argc >= 4) {
    printf("Too many args");
    return 1;
  }

  char *srcfName = filter(strrchr(argv[1], '/'), argv[1]);
  char *desfName = filter(strrchr(argv[2], '/'), argv[2]);
  if (strcmp(desfName, "") == 0) {
    desfName = srcfName;
  }

  FILE *sf = fopen(argv[1], "r");
  FILE *df = fopen()
  printf("file names: %s, %s", srcfName, desfName);

  return 0;
}
