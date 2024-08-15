#include <stdio.h>
#include <stdlib.h>

struct node {
  int n;
  struct node *link;
};

struct nodeList {
  struct node *nodes;
  struct nodeList *link;
};

struct node *nodeInsertEnd(struct node *tail, int n) {
  struct node *newNode = (struct node *)malloc(sizeof(struct node));
  newNode->n = n;

  if (tail == NULL) {
    newNode->link = newNode;
    return newNode;
  }

  newNode->link = tail->link;
  tail->link = newNode;

  return newNode;
}

struct nodeList *init() {
  struct nodeList *head = NULL;
  int totalIndx = 9;

  for (int i = totalIndx; i >= 0; i--) {
    struct nodeList *newNodeList =
        (struct nodeList *)malloc(sizeof(struct nodeList));
    newNodeList->nodes = NULL;
    newNodeList->link = head;

    head = newNodeList;
  }

  return head;
}

void insertNodeList(struct nodeList *head, int n, int pos) {
  struct nodeList *temp = head;
  struct node *newNode = (struct node *)malloc(sizeof(struct node));
  newNode->n = n;
  newNode->link = NULL;

  while (pos > 0) {
    temp = temp->link;
    pos--;
  }

  struct node *nodesAtIndx = temp->nodes;

  if (nodesAtIndx == NULL) {
    temp->nodes = newNode;
    return;
  }

  while (nodesAtIndx->link != NULL) {
    nodesAtIndx = nodesAtIndx->link;
  }
  nodesAtIndx->link = newNode;

  return;
}

struct node *remakeIt(struct nodeList *originalHead) {
  struct nodeList *head = originalHead;
  struct node *theHead = NULL;
  struct node *theTail = NULL;
  int i = 0;

  for (i = 0; i < 10; i++) {
    if (head->nodes == NULL) {
      head = head->link;
      continue;
    }

    struct node *temp = head->nodes;

    if (theHead == NULL) {
      theHead = temp;
    }
    if (theTail != NULL) {
      theTail->link = temp;
    }
    while (temp->link != NULL) {
      temp = temp->link;
    }
    temp->link = theHead;
    theTail = temp;

    head->nodes = NULL;
    head = head->link;
  }

  return theTail;
}

int countNodeList(struct nodeList *head) {
  struct nodeList *temp = head;
  int count = 0;

  while (temp != NULL) {
    count++;
    temp = temp->link;
  }

  return count;
}

void displayNodes(struct node *tail) {
  if (tail == NULL) {
    return;
  }

  struct node *head = tail->link;
  int completeFlag = 0;

  while (!completeFlag) {
    printf("%d ", head->n);
    head = head->link;

    if (head == tail->link) {
      completeFlag = 1;
    }
  }

  return;
}

struct node *hardCodeNodes(struct node *tail) {
  tail = nodeInsertEnd(tail, 782);
  tail = nodeInsertEnd(tail, 82);
  tail = nodeInsertEnd(tail, 375);
  tail = nodeInsertEnd(tail, 9);
  tail = nodeInsertEnd(tail, 283);
  tail = nodeInsertEnd(tail, 832);
  tail = nodeInsertEnd(tail, 387);
  tail = nodeInsertEnd(tail, 102);
  tail = nodeInsertEnd(tail, 38);

  return tail;
}

int rdxSortPassCount(struct node *tail) {
  struct node *temp = tail->link;
  int max = temp->n;
  int completeFlag = 0;
  int pass = 0;

  while (!completeFlag) {
    if (max < temp->n) {
      max = temp->n;
    }

    temp = temp->link;
    if (temp == tail->link) {
      completeFlag = 1;
    }
  }

  while (max != 0) {
    pass++;
    max = max / 10;
  }

  return pass;
}

struct node *radixSort(struct node *tail) {
  if (tail == NULL) {
    return NULL;
  }

  struct nodeList *container = NULL;
  int pass = rdxSortPassCount(tail);
  int powerTo10 = 10;

  while (pass > 0) {
    struct node *head = tail->link;
    int completeFlag = 0;
    container = init();

    while (!completeFlag) {
      int digit = (head->n % powerTo10) / (powerTo10 / 10);
      insertNodeList(container, head->n, digit);

      head = head->link;
      if (head == tail->link) {
        completeFlag = 1;
      }
    }

    tail = remakeIt(container);
    powerTo10 *= 10;
    pass--;
  }

  return tail;
}

int main() {
  struct node *tail = NULL;
  tail = hardCodeNodes(tail);

  struct nodeList *test = NULL;
  test = init();

  displayNodes(tail);
  printf("\ntest contains %d nodeList nodes", countNodeList(test));

  tail = radixSort(tail);
  printf("\n");
  displayNodes(tail);

  return 0;
}
