#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  printf("[leader]\tThis is the leader proc: pid=%d, gid=%d\n", getpid(),
         getgid());

  pid_t pid = fork();
  if (pid == 0) {
    printf("[child]\t\tThis child of leader. pid=%d\n", getpid());
    sleep(2);

    pid_t pidChild = fork();
    if (pidChild == 0) {
      printf("[grandchild]\tThis grand child of leader. pid=%d, ppid=%d\n",
             getpid(), getppid());
      sleep(3);

      // systemDih adopts
      printf("[grandchild]\tNow the init should adopt me.. ppid=%d\n",
             getppid());
      exit(0);
    }
    sleep(2);
    printf("[child]\t\tChild of leader ded\n");
    exit(0);
  } else {
    waitpid(pid, NULL, 0);
  }

  printf("[leader]\tleader completed\n");
  sleep(2);

  return 0;
}
