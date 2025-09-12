#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define DRIVER_NAME "/dev/def3r-0"
#define SET_SIZE_OF_QUEUE _IOW('a', 'a', int *)

int main(void) {
  int fd = open(DRIVER_NAME, O_RDWR);
  if (fd < 0) {
    perror("open");
    return 69;
  }
  int size = 3;
  int ret = ioctl(fd, SET_SIZE_OF_QUEUE, &size);
  close(fd);
  if (ret < 0) {
    perror("ioctl");
    return 69;
  }
  return ret;
}
