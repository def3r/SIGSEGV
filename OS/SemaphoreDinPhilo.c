#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#define PHILOSOPHERS 5

// Using this mutex to prevent deadlock
sem_t mutex;
sem_t forks[PHILOSOPHERS];

void *philosophers(void *arg) {
  int id = *(int *)arg;
  while (1) {
    printf("[THINK]: Philosopher %d\n", id);
    sleep(1);

    sem_wait(&mutex);
    sem_wait(&forks[id]);
    sem_wait(&forks[(id + 1) % PHILOSOPHERS]);
    sem_post(&mutex);

    printf("[DINE]: Philosopher %d\n", id);
    sleep(1);

    sem_post(&forks[id]);
    sem_post(&forks[(id + 1) % PHILOSOPHERS]);

    printf("[DONE]: Philosopher %d\n", id);
    sleep(1);
  }

  return NULL;
}

int main() {
  pthread_t philosopher[PHILOSOPHERS];
  int philIds[PHILOSOPHERS];
  sem_init(&mutex, 0, 1);
  for (int i = 0; i < PHILOSOPHERS; i++) {
    sem_init(&forks[i], 0, 1);
  }

  for (int i = 0; i < PHILOSOPHERS; i++) {
    philIds[i] = i;
    pthread_create(&philosopher[i], NULL, philosophers, &philIds[i]);
  }

  for (int i = 0; i < PHILOSOPHERS; i++) {
    pthread_join(philosopher[i], NULL);
  }

  sem_destroy(&mutex);
  for (int i = 0; i < PHILOSOPHERS; i++) {
    sem_destroy(&forks[i]);
  }

  return 0;
}
