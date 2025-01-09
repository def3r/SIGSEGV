#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX 5000

void fStatus(FILE *f, const char *fName) {
  if (f) {
    return;
  }
  printf("Unable to find file %s", fName);
  exit(1);
}

int main(int argc, char **argv) {
  if (argc <= 1) {
    printf("No files provided");
  }

  bool stdWrite = true;
  char *fWrite = NULL;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], ">") == 0) {
      stdWrite = false;
      if (i + 1 >= argc) {
        printf("provided '>' to write but no target file specified");
        exit(1);
      }
      fWrite = argv[i + 1];
    }
  }
  FILE *des = stdWrite ? NULL : fopen(fWrite, "w");
  stdWrite ? NULL : fStatus(des, fWrite);

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], ">") == 0) {
      break;
    }
    char bffr[MAX];
    FILE *src = fopen(argv[i], "r");
    fStatus(src, argv[i]);
    while (fgets(bffr, MAX, src)) {
      stdWrite ? printf("%s", bffr) : fputs(bffr, des);
    }
    fclose(src);
  }

  if (!stdWrite) {
    fclose(des);
  }
  return 0;
}
