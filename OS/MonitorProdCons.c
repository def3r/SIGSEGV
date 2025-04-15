#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define BS 5

int bffr[BS];
int count = 0;

pthread_mutex_t mutex;
pthread_cond_t cond_full;
pthread_cond_t cond_empty;

void *producer(void *arg) {
  int item, i = 0;
  while (1) {
    item = i++;
    pthread_mutex_lock(&mutex);
    // Interesting, this needs to be inside a while block
    // and not if block, POSIX is the becuase.
    //
    // POSIX implementation of condition vars allow it to
    // return and wakeup the thread `spuriously` from its
    // sleep even if it was not signaled and also if it
    // returned false! In its documentation, they say it
    // `may` wake up spuriously
    //
    // Reason- Portability and Performance(womp womp)
    while (count >= BS) {
      pthread_cond_wait(&cond_empty, &mutex);
    }

    bffr[count++] = item;
    printf("[Producer]: Produced %d\n", item);

    pthread_cond_signal(&cond_full);
    pthread_mutex_unlock(&mutex);

    sleep(1);
  }
  return NULL;
}
void *consumer(void *arg) {
  int item = 0;
  while (1) {
    pthread_mutex_lock(&mutex);
    while (count <= 0) {
      pthread_cond_wait(&cond_full, &mutex);
    }

    item = bffr[count--];
    printf("[Consumer]: Consumed %d\n", item);

    pthread_cond_signal(&cond_empty);
    pthread_mutex_unlock(&mutex);

    sleep(2);
  }
  return NULL;
}

int main() {
  pthread_t prod, cons;
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond_full, NULL);
  pthread_cond_init(&cond_empty, NULL);

  pthread_create(&prod, NULL, producer, NULL);
  pthread_create(&cons, NULL, consumer, NULL);
  pthread_join(prod, NULL);
  pthread_join(cons, NULL);

  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond_full);
  pthread_cond_destroy(&cond_empty);

  return 0;
}
