#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

sem_t mutex, writing;
int readCount = 0;

void *writer(void *arg) {
  int id = *(int *)arg;
  int item, i = 0;
  while (1) {
    item = i++;
    sem_wait(&writing);
    printf("[WRITER %d]: item written %d\n", id, item);
    sleep(2);
    sem_post(&writing);
    sleep(2);
  }
  return NULL;
}
void *reader(void *arg) {
  int id = *(int *)arg;
  int item;
  while (1) {
    sem_wait(&mutex);
    if (readCount++ == 0) {
      sem_wait(&writing);
    }
    sem_post(&mutex);

    printf("[READER %d]: item read\n", id);
    sleep(1);

    sem_wait(&mutex);
    if (--readCount == 0) {
      sem_post(&writing);
    }
    sem_post(&mutex);
    sleep(1);
  }
  return NULL;
}

int main() {
  pthread_t readers[5], writers[2];
  int readerIds[5] = {1, 2, 3, 4, 5};
  int writerIds[2] = {1, 2};

  sem_init(&mutex, 0, 1);
  sem_init(&writing, 0, 1);

  for (int i = 0; i < 5; i++) {
    pthread_create(&readers[i], NULL, reader, &readerIds[i]);
  }
  for (int i = 0; i < 2; i++) {
    pthread_create(&writers[i], NULL, writer, &writerIds[i]);
  }

  for (int i = 0; i < 5; i++) {
    pthread_join(readers[i], NULL);
  }
  for (int i = 0; i < 2; i++) {
    pthread_join(writers[i], NULL);
  }

  sem_destroy(&mutex);
  sem_destroy(&writing);

  return 0;
}
