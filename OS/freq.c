#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX 5000

int main(int argc, char **argv) {
  if (argc <= 1) {
    printf("No file provided");
    exit(1);
  } else if (argc == 2) {
    printf("No string provided");
    exit(1);
  } else if (argc > 3) {
    printf("Ignoring args starting from: '%s'", argv[3]);
  }

  FILE *f = fopen(argv[1], "r");
  if (!f) {
    printf("Unable to open file %s", argv[1]);
    exit(1);
  }
  int count = 0;
  char bffr[MAX];
  while (fgets(bffr, MAX, f)) {
    char *token = strtok(bffr, " ");
    while (token != NULL) {
      if (strcmp(token, argv[2]) == 0) {
        printf(" %s\n", token);
        count++;
      }
      token = strtok(NULL, " ");
    }
  }
  printf(" %d %s", count, argv[2]);

  return 0;
}
