#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Trying to implement the defer from go docs (blog)
// https://go.dev/blog/defer-panic-and-recover

struct deferStatus {
  void (*func)(void *);
  void *args;
};

#define DEFERSETUP                                                             \
  int deferSize = 10;                                                          \
  int deferIndex = 0;                                                          \
  void *deferArgs = NULL;                                                      \
  struct deferStatus *deferList =                                              \
      (struct deferStatus *)malloc(sizeof(struct deferStatus) * deferSize)

#define Defer(function, arguments)                                             \
  if (deferIndex >= deferSize) {                                               \
    fprintf(stderr, "MAX defer stack size is 10; defer stack overflow\n");     \
    exit(EXIT_FAILURE);                                                        \
  }                                                                            \
  deferArgs = malloc(sizeof(arguments));                                       \
  memcpy(deferArgs, &arguments, sizeof(arguments));                            \
  deferList[deferIndex++] = (struct deferStatus) {                             \
    .func = function, .args = deferArgs                                        \
  }

#define Return(retVar)                                                         \
  for (int i = deferIndex - 1; i >= 0; i--) {                                  \
    deferList[i].func(deferList[i].args);                                      \
    free(deferList[i].args);                                                   \
  }                                                                            \
  free(deferList);                                                             \
  return retVar

#define Println(...)                                                           \
  printf(__VA_ARGS__);                                                         \
  printf("\n")

// Generic Function to defer: int close(int);
void Close(void *voidFd) {
  int fd = *(int *)(voidFd);
  close(fd);
  Println("Closed fd %d", fd);
}

// Without defer in pure C
bool CopyFile_C(char *dstName, char *srcName) {
  int srcFd = open(srcName, O_RDONLY);
  if (srcFd < 0) {
    return false;
  }

  int dstFd = open(dstName, O_WRONLY | O_TRUNC, 0666);
  if (dstFd < 0) {
    close(srcFd);
    return false;
  }

  printf("Copying files.");

  close(dstFd);
  close(srcFd);
  return true;
}

// Implementing the defer example from the blog
bool CopyFile(char *dstName, char *srcName) {
  DEFERSETUP;

  int srcFd = open(srcName, O_RDONLY);
  if (srcFd < 0) {
    Return(false);
  }
  Defer(Close, srcFd);

  int dstFd = open(dstName, O_WRONLY, 0666);
  if (dstFd < 0) {
    Return(false);
  }
  Defer(Close, dstFd);

  Println("Copying files.");

  Return(true);
}

int main() {
  Println("Hello noobs");
  CopyFile("defer.c", "defer.c");
  return 0;
}
