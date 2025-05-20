#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
  printf("getuid = %d\n", getuid());

  // getuid returns 0 if the proc execs in root mode
  if (!getuid()) {
    printf("executed in root mode\n");
    return 0;
  }
  printf("proc no root\nForcing root...\n");

  char buf[100];
  int rl = readlink("/proc/self/exe", buf, 100);
  if (rl < 0) {
    perror("readlink error:");
    return 1;
  }
  buf[rl] = '\0';
  int ex = execlp("sudo", "sudo", buf, NULL);
  printf("Failed to restart application `%s`\n, error: %d", buf, ex);
  perror("oh no");
}
