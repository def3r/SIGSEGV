#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define DRIVER_NAME "/dev/def3r-0"
#define POP_DATA _IOR('a', 'c', struct data *)

struct data {
  int length;
  char *data;
};

int main(void) {
  int fd = open(DRIVER_NAME, O_RDWR);
  if (fd < 0) {
    perror("open");
    return 69;
  }

  struct data *d = malloc(sizeof(struct data));
  d->data = malloc(20);
  fflush(stdout);
  int ret = ioctl(fd, POP_DATA, d);
  if (ret == -EINVAL) {
    printf("no item to pop\n");
  } else if (ret == 0) {
    printf("d length: %d\n", d->length);
    printf("d data  : %s\n", d->data);
  } else {
    perror("ioctl");
    return 69;
  }

  close(fd);
  free(d->data);
  free(d);

  return ret;
}
