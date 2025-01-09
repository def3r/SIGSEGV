#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX 5000

char *filter(char *c, char *src) {
  if (!c) {
    return src;
  }
  return ++c;
}

void fStatus(FILE *f, const char *fName) {
  if (f) {
    return;
  }
  printf("Unable to find file %s", fName);
  exit(1);
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

  /* Conditions:
   * 1. cp path/to/src/file.ext path/to/dest/
   * 2. cp path/to/src/file.ext path/to/dest/newFileName.ext
   *
   * Well, truns out im a fucking idiot
   */

  char bffr[MAX];
  char *src = argv[1];
  char *des = argv[2];
  char *srcfName = filter(strrchr(argv[1], '/'), argv[1]);
  char *desfName = filter(strrchr(argv[2], '/'), argv[2]);
  if (strcmp(desfName, "") == 0) {
    desfName = srcfName;
    strcat(des, srcfName);
  }

  FILE *sf = fopen(src, "r");
  FILE *df = fopen(des, "w");
  fStatus(sf, src);
  fStatus(df, des);

  while (fgets(bffr, MAX, sf)) {
    fputs(bffr, df);
  }

  fclose(sf);
  fclose(df);
  return 0;
}
