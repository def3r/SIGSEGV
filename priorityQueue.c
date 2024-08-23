#include <stdio.h>
#include <stdlib.h>

struct node {
  struct node* plink;
  int n;
  struct node* nlink;
};

struct node* enqueue(struct node** front, struct node** rear, int n) {
  struct node* newNode = (struct node*)malloc(sizeof(struct node));
  newNode->plink = NULL;
  newNode->n = n;
  newNode->nlink = NULL;

  if (*front == NULL) {
    *front = newNode;
    *rear = newNode;
    return *front;
  }

  struct node* curr = (*front);
  struct node* prev = NULL;

  while(curr != NULL && curr->n > n) {
    prev = curr;
    curr = curr->nlink;
  }

  if (prev == NULL) {
    newNode->nlink = (*front);
    (*front)->plink = newNode;
    *front = newNode;
    return *front;
  }

  newNode->plink = prev;
  newNode->nlink = prev->nlink;
  prev->nlink = newNode;

  if (newNode->nlink != NULL) {
    newNode->nlink->plink = newNode;
  }

  if (newNode ->nlink == NULL) {
    *rear = newNode;
  }

  return *front;
}

int dequeue(struct node** front, struct node** rear) {
  if ((*rear) == NULL) {
    printf("Queue underflow\n");
    return 0;
  }

  struct node* prev = (*rear)->plink;
  int n = (*rear)->n;
  if (prev != NULL) { prev->nlink = NULL; }

  free(*rear);

  *rear = prev;

  return n;
}

int main() {
  struct node* front = NULL;
  struct node* rear = NULL;

  enqueue(&front, &rear, 5);
  enqueue(&front, &rear, 81);
  enqueue(&front, &rear, 61);
  enqueue(&front, &rear, 90);
  enqueue(&front, &rear, 2);
  enqueue(&front, &rear, 33);

  for (int i = 0; i < 10; i++) {
    printf("%d ", dequeue(&front, &rear));
  }

  return 0;
}
