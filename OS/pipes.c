#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// pipe[2]
//  0: reading end
//  1: writing end
int main() {
  int fd1[2], fd2[2];
  char fixed_str[] = "THIS IS PIPED";
  char input_str[100];
  pid_t p;

  if (pipe(fd1) == -1) {
    fprintf(stderr, "Pipe Failed");
    return 1;
  }
  if (pipe(fd2) == -1) {
    fprintf(stderr, "Pipe Failed");
    return 1;
  }

  printf("Enter a string:");
  scanf("%s", input_str);

  p = fork();
  if (p < 0) {
    fprintf(stderr, "fork Failed");
    return 1;
  }

  if (p > 0) {
    char concat_str[100];
    close(fd1[0]);
    write(fd1[1], input_str, strlen(input_str));
    close(fd1[1]);
    wait(NULL);
    close(fd2[1]);
    read(fd2[0], concat_str, 100);
    close(fd2[0]);
    printf("Concatenated String rec: %s\n", concat_str);
  } else {
    char concat_str[100];
    close(fd1[1]);
    int k = read(fd1[0], concat_str, 100);
    close(fd1[0]);
    for (int i = 0; i < strlen(fixed_str); i++) {
      concat_str[k++] = fixed_str[i];
    }
    concat_str[k] = '\0';
    close(fd2[0]);
    write(fd2[1], concat_str, strlen(concat_str) + 1);
    close(fd2[1]);
  }

  return 0;
}
