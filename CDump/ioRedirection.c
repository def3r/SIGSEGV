#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  // close the stdin
  // When we close a fd, it's value is free to be
  // used again by open()
  // open() assigns free fd with least val to the
  // opened stream.
  //
  // Thus, closing stdin frees 0 and then the next
  // open() instruction is now guaranteed to have
  // fd=0
  //
  // Since by default stdin is 0, the newly opened
  // file will now act as the stdin for the proc
  close(0);
  int fdin = open("ioRedirectionInp.txt", O_RDONLY);
  if (fdin < 0) {
    fprintf(stderr, "Cannot find file ioRedirectionInp.txt\n");
    exit(EXIT_FAILURE);
  }

  int a, b;
  printf("Enter 2 nums: ");
  scanf("%d %d", &a, &b);
  printf("\nRead: %d %d\n", a, b);

  // Same can be easily done to stdout and stderr
}
