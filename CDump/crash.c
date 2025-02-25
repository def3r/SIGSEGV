#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
  printf("\033[2k\rMornin\n");
  printf("Trying to crash!\n");
  unsigned long long size = 0;
  int fd;
  char bffr[40];
  void* ptr;

  while (1) {
    ptr = malloc(1024);
    memset(ptr, 0, 1024);
    printf("\033[2k\r%lld", ++size);
    float mb = size / 1024.0f;
    float gb = mb / 1024.0f;
    int len = sprintf(bffr, "%lld KB\n%.3f MB\n%.3f GB", size, mb, gb);
    fd = open("size.txt", O_WRONLY, 0777);
    write(fd, bffr, len);
    close(fd);
  }
  return 0;
}
