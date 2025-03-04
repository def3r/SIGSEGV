#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#define SHMSIZE 27

int main() {
  char *shm, *s;
  key_t key;
  int shmid;

  key = 9999;

  shmid = shmget(key, SHMSIZE, 0666);
  if (shmid < 0) {
    perror("shmid");
    exit(1);
  }

  shm = shmat(shmid, NULL, 0);
  if (shm == (char*)-1) {
    perror("shmat");
    exit(1);
  }

  for (s = shm; *s != 0; s++) {
    printf("%c", *s);
    fflush(stdout);
  }
  printf("\n");

  *shm = '*';

  return 0;
}
