#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// File Descriptor index 0, 1, 2 reserved for stdin, stdout, stderr
int main() {
  write(1, "syscall write to stdout\n", 24);
  system("ls | grep \"sys\"");

  int fd = open("sys.sus", O_WRONLY | O_CREAT);
  if (fd == -1) {
    perror("unable to execute suscall;\nfd returned -1");
    exit(1);
  }

  printf("%d\n", fd);

  printf("Writing to sys.sus\n");
  char *writeThis = "written to sys.sus\0";
  int wrSuccess = write(fd, writeThis, strlen(writeThis));
  if (wrSuccess == -1) {
    perror("Unable to write to file descriptor");
    exit(1);
  }
  printf("Written successfully\n");

  if (close(fd) < 0) {
    perror("unable to execute close suscall");
    exit(1);
  }
  return 0;
}
