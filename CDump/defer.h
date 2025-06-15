#ifndef DEFER_H
#define DEFER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Implementation

struct deferStatus {
  void (*func)(void*);
  void* args;
};

struct deferStack {
  int deferSize;
  int deferCapacity;
  void* deferArgs;
  struct deferStatus* deferList;
};

void initDeferStack(struct deferStack** dStack);

//  Wrapper Fucntions
// [[[[[[[[[ ]]]]]]]]]

void Close(void* voidFd);
void CloseVerbose(void* voidFd);
void Free(void* memloc);
void FreeVerbose(void* memloc);

#ifdef DEFER_IMPLEMENTATION
#define DeferSetUp()                      \
  struct deferStack* localDeferStack = 0; \
  initDeferStack(&localDeferStack)

#define Defer(function, argument)                                         \
  if (localDeferStack->deferSize >= localDeferStack->deferCapacity) {     \
    struct deferStatus* newDeferList = (struct deferStatus*)malloc(       \
        sizeof(struct deferStatus) * localDeferStack->deferCapacity * 2); \
    if (newDeferList == 0) {                                              \
      fprintf(stderr, "malloc for new defer list failed\n");              \
      exit(EXIT_FAILURE);                                                 \
    }                                                                     \
    memcpy(newDeferList, localDeferStack->deferList,                      \
           sizeof(struct deferStatus) * (localDeferStack->deferSize));    \
    free(localDeferStack->deferList);                                     \
    localDeferStack->deferList = newDeferList;                            \
    localDeferStack->deferCapacity *= 2;                                  \
  }                                                                       \
  localDeferStack->deferArgs = malloc(sizeof(argument));                  \
  memcpy(localDeferStack->deferArgs, &argument, sizeof(argument));        \
  localDeferStack->deferList[localDeferStack->deferSize++] =              \
      (struct deferStatus) {                                              \
    .func = function, .args = localDeferStack->deferArgs                  \
  }

#define Return(retval)                                                      \
  for (int i = localDeferStack->deferSize - 1; i >= 0; i--) {               \
    localDeferStack->deferList[i].func(localDeferStack->deferList[i].args); \
    free(localDeferStack->deferList[i].args);                               \
  }                                                                         \
  free(localDeferStack->deferList);                                         \
  return 0

#define Println(...)   \
  printf(__VA_ARGS__); \
  printf("\n")

void initDeferStack(struct deferStack** dStack) {
  *dStack = (struct deferStack*)malloc(sizeof(struct deferStack));
  if (*dStack == 0) {
    fprintf(stderr, "malloc for defer stack failed\n");
    exit(EXIT_FAILURE);
  }

  (*dStack)->deferSize = 0;
  (*dStack)->deferCapacity = 2;
  (*dStack)->deferArgs = 0;
  (*dStack)->deferList = (struct deferStatus*)malloc(
      sizeof(struct deferStatus) * (*dStack)->deferCapacity);
  if ((*dStack)->deferList == 0) {
    fprintf(stderr, "malloc for defer list failed\n");
    exit(EXIT_FAILURE);
  }
}

void Close(void* voidFd) {
  int fd = *(int*)(voidFd);
  close(fd);
}
void CloseVerbose(void* voidFd) {
  Println("Closing fd=%d", *(int*)(voidFd));
}

void Free(void* memloc) {
  free(*(void**)memloc);
}
void FreeVerbose(void* memloc) {
  Println("Freed memloc=%p", *(void**)memloc);
  Free(memloc);
}
#endif

#endif  // !DEFER_H
