#include <stdio.h>
#include <stdlib.h>

struct node {
  int n;
  struct node *link;
};

struct node *insertNode(struct node *head, int n) {
  struct node *temp = (struct node *)malloc(sizeof(struct node));
  temp->n = n;
  temp->link = head;
  return temp;
}

struct node *insertEnd(struct node *head, int n) {
  struct node *new = (struct node *)malloc(sizeof(struct node));
  struct node *temp = head;

  new->n = n;
  new->link = NULL;

  if (head == NULL) {
    return new;
  }

  while (temp->link != NULL) {
    temp = temp->link;
  }

  temp->link = new;
  return head;
}

void displayNodes(struct node *head) {
  struct node *temp = head;

  while (temp != NULL) {
    printf("%d ", temp->n);
    temp = temp->link;
  }
  printf("\n");

  return;
}

struct node *hardCodeList(struct node *head) {
  head = insertNode(head, 89);
  head = insertNode(head, 589);
  head = insertNode(head, 625);
  head = insertNode(head, 104);
  head = insertNode(head, 899);
  head = insertNode(head, 123);
  head = insertNode(head, 99);
  head = insertNode(head, 167);
  head = insertNode(head, 738);
  head = insertNode(head, 6);

  return head;
}

// implementing Radix Sort
struct nodeList {
  struct node *ptr;
  struct nodeList *link;
};

struct node *remakeList(struct nodeList *head) {
  struct node *newList = NULL;
  struct nodeList *iter = head;

  while (iter != NULL) {
    struct node *memb = iter->ptr;

    while (memb != NULL) {
      newList = insertEnd(newList, memb->n);
      memb = memb->link;
    }

    iter = iter->link;
  }

  return newList;
}

struct nodeList *init() {
  int i = 0;
  struct nodeList *head = NULL;

  for (i = 0; i < 10; i++) {
    struct nodeList *temp = (struct nodeList *)malloc(sizeof(struct nodeList));
    temp->ptr = NULL;
    temp->link = head;

    head = temp;
  }

  return head;
}

void printList(struct nodeList *head) {
    struct nodeList *current = head;
    int count = 0;

    while (current != NULL) {
        printf("Node %d\n", count++);
        current = current->link;
    }
}

struct node *radixSort(struct nodeList *list, struct node *head) {
  struct nodeList *tempList = list;
  struct node *temp = head;
  int pass = 0;
  int max = head->n;
  int digiPlace = 10;
  int i = 0;

  // calc pass:
  while (temp != NULL) {
    if (temp->n > max) {
      max = temp->n;
    }
    temp = temp->link;
  }
  while (max != 0) {
    max = max / 10;
    pass++;
  }

  temp = head;
  while (pass > 0) {
    while (temp != NULL) {
      int d = (temp->n % digiPlace) / (digiPlace / 10);

      tempList = list;
      for (i = 0; i < d; i++) {
        tempList = tempList->link;
      }

      // insertion at end
      struct node *listNode = tempList->ptr;
      struct node *newNode = (struct node *)malloc(sizeof(struct node));
      newNode->n = temp->n;

      temp = temp->link;

      if (listNode == NULL) {
        tempList->ptr = newNode;
        newNode->link = NULL;
        continue;
      }

      while (listNode->link != NULL) {
        listNode = listNode->link;
      }
      listNode->link = newNode;
      newNode->link = NULL;
    }
    pass--;
    digiPlace *= 10;
    temp = remakeList(list);
    list = init();
  }

  return temp;
}

int main() {
  struct node *head = NULL;
  struct nodeList *list = NULL;
  // hard code 10 values
  head = hardCodeList(head);
  list = init();

  displayNodes(head);

  head = radixSort(list, head);

  displayNodes(head);

  return 0;
}
