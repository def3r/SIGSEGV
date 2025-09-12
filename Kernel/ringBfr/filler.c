#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define DRIVER_NAME "/dev/def3r-0"
#define PUSH_DATA _IOW('a', 'b', struct data *)

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
  d->length = 18;
  d->data = malloc(d->length);
  memcpy(d->data, "This is a sentence", d->length);

  int ret = ioctl(fd, PUSH_DATA, d);
  if (ret < 0) {
    perror("ioctl");
    return 69;
  }

  close(fd);
  free(d->data);
  free(d);
  return ret;
}
