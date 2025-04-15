#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#define BS 5

int bffr[BS];
int count = 0;

sem_t mutex;
sem_t full;
sem_t empty;

void *producer(void *arg) {
  int item, i = 0;
  while (1) {
    item = i++;
    sem_wait(&empty);

    // Critical Section
    sem_wait(&mutex);
    bffr[count++] = item;
    printf("[Producer]: produced %d\n", item);
    sem_post(&mutex);
    // Critical Section

    sem_post(&full);
  }
  return NULL;
}

void *consumer(void *arg) {
  int item;
  while (1) {
    sem_wait(&full);

    // Critical Section
    sem_wait(&mutex);
    item = bffr[--count];
    printf("[Consumer]: consumed %d\n", item);
    sem_post(&mutex);
    // Critical Section

    sem_post(&empty);
  }
  return NULL;
}

int main() {
  pthread_t prod, cons;

  sem_init(&mutex, 0, 1);
  sem_init(&full, 0, 0);
  sem_init(&empty, 0, BS);

  pthread_create(&prod, NULL, producer, NULL);
  pthread_create(&cons, NULL, consumer, NULL);
  pthread_join(prod, NULL);
  pthread_join(cons, NULL);

  sem_destroy(&mutex);
  sem_destroy(&full);
  sem_destroy(&empty);

  return 0;
}
