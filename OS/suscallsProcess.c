#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  printf("The state rn:\n");

  int pid = fork();
  if (pid < 0)
    exit(1);

  if (pid == 0) {
    sleep(1);
    printf("The child\n");
    printf("PPID %d && PID %d\n", getppid(), getpid());
    return 0; // return the child fork()
  } else {
    system("ps");
    printf("The parent with PPID %d && PID %d\n", getppid(), getpid());

    // If do not wait, Parent terminates leavin the child Orphan.
    // The child does not terminate, it completes and return
    wait(NULL);
  }

  /*
   *
   * This happens when a clown does not return the forked child
   *
   */
  // This prints 27 lines, => 3 iterations of 9 processes?
  // Although the claim is 7 `New Processes` are created
  // 9 - 1 Parent = 8?
  //
  // int c = 0;
  // for (c = 0; c < 3; c++) {
  //   int pid = fork();
  //   printf("%dth iteration, current PID: %d\n", c, getpid());
  // }

  printf("\n\nCurrent PID:%d\n", getpid());
  int c = 0;
  for (c = 0; c < 3; c++) {
    int pid = fork();
    if (pid == 0) {
      continue;
    } else {
      printf("%dth iteration, current PID: %d\n", c, getpid());
      return 0; // break;
    }
    printf("%dth iteration, current PID: %d\n", c, getpid());
  }
  write(0, "Using exec\n", 11);

  return 0;
}
