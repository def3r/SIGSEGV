#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

// This means nothing, be it 27 bytes,
// the SHMSIZE will be rounded off to the
// nearest PAGE_SIZE, ie for 27 bytes,
// 4096 ($ getconf PAGESIZE) ie 4kB ie 1
// page is reserved. fire.
#define SHMSIZE 27

int main() {
  char c;
  int shmid;
  key_t key;
  char *shm, *s;

  // key for shared memory
  key = 9999;
  shmid = shmget(key, SHMSIZE, IPC_CREAT | 0666);
  if (shmid < 0) {
    perror("shmget");
    exit(0);
  }

  shm = (char*)shmat(shmid, NULL, 0);
  if (shm == (char*)-1) {
    perror("shmat");
    exit(0);
  }

  s = shm;
  for (c = 'a'; c <= 'z'; c++) {
    *s++ = c;
  }

  // So, one can easily overflow the buffer
  // If mprotect() is not used, it depends whether
  // the process will result SIGSEGV or corrupt the
  // data.
  //
  // If the address of overflew memory is not mapped in the
  // process address space, the program will return SIGSEGV
  // just as it would in case of accessing invalid heap memory
  // if the address of overflew memory is mapped, the data will
  // be corrupted without any SIGSEGV resulting in undefined
  // behaviour
  //
  // Altho here for 27 bytes to be read, 4kB is allocated
  // and no other task is performed using the remainin space
  // thus no data loss and no undefined behaviour
  *s++ = '|';
  *s++ = '|';
  *s = 0;
  printf("Waiting to be Read by consumer :)");
  while (*shm != '*') {
    sleep(1);
  }

  return 0;
}
