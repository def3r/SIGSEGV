#include <stdio.h>
#include <stdlib.h>

#define M 5

struct Node* bInsertHelper(struct Node* bTree, int key);
struct Node* bInsert(struct Node* bTree, int key);

struct Node* mergeFlag = NULL;

struct Node {
  int size;
  struct Node* link[M + 1];
  int keys[M];  // technically only M-1 keys are allowed but we take M for
                // simplicity of splitting nodes up;
};

void rShift(struct Node* n, int indx) {
  int m = M;
  n->link[m] = n->link[m - 1];
  while (--m > indx) {
    n->keys[m] = n->keys[m - 1];
    n->link[m] = n->link[m - 1];
  }
}

struct Node* initNode() {
  struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
  newNode->size = 0;
  int m = M;

  newNode->link[M] = NULL;
  while (m--) {
    newNode->link[m] = NULL;
    newNode->keys[m] = 0;
  }

  return newNode;
}

struct Node* bSplit(struct Node* bTree) {
  struct Node* newNode = initNode();
  struct Node* leftChild = initNode();
  struct Node* rightChild = initNode();

  int splitIndex = (M % 2) ? bTree->size / 2 : bTree->size / 2 - 1;

  newNode->keys[0] = bTree->keys[splitIndex];
  newNode->size++;

  int childIndex = 0;
  for (int i = 0; i < M; i++) {
    if (i > splitIndex) {
      rightChild->keys[childIndex] = bTree->keys[i];
      rightChild->link[childIndex++] = bTree->link[i];
      rightChild->size++;
    } else if (i < splitIndex) {
      leftChild->keys[childIndex] = bTree->keys[i];
      leftChild->link[childIndex++] = bTree->link[i];
      leftChild->size++;
    } else {
      leftChild->link[childIndex] = bTree->link[i];
      childIndex = 0;
    }
  }
  rightChild->link[childIndex] = bTree->link[M];

  newNode->link[0] = leftChild;
  newNode->link[1] = rightChild;

  free(bTree);
  mergeFlag = newNode;

  return newNode;
}

struct Node* bMerge(struct Node* bTree) {
  if (bTree->keys[bTree->size - 1] < mergeFlag->keys[0]) {
    bTree->keys[bTree->size] = mergeFlag->keys[0];
    bTree->link[bTree->size] = mergeFlag->link[0];
    bTree->link[++bTree->size] = mergeFlag->link[1];

    mergeFlag = NULL;
    return bTree;
  }

  for (int i = 0; i < bTree->size; i++) {
    if (mergeFlag->keys[0] < bTree->keys[i]) {
      rShift(bTree, i);
      bTree->keys[i] = mergeFlag->keys[0];
      bTree->link[i] = mergeFlag->link[0];
      bTree->link[++i] = mergeFlag->link[1];
      bTree->size++;

      mergeFlag = NULL;
      return bTree;
    }
  }

  mergeFlag = NULL;

  return bTree;
}

struct Node* bInsert(struct Node* bTree, int key) {
  mergeFlag = NULL;
  bTree = bInsertHelper(bTree, key);

  // merge
  if (mergeFlag) {
    bTree = bMerge(bTree);
  }

  // split
  if (bTree->size == M) {
    bTree = bSplit(bTree);
  }

  return bTree;
}

struct Node* bInsertHelper(struct Node* bTree, int key) {
  if (!bTree) {
    struct Node* newNode = initNode();
    newNode->keys[newNode->size++] = key;

    return newNode;
  }

  if (key > bTree->keys[bTree->size - 1]) {
    if (bTree->link[bTree->size]) {
      bTree->link[bTree->size] = bInsert(bTree->link[bTree->size], key);
    } else {
      bTree->keys[bTree->size++] = key;
    }
  } else {
    for (int i = 0; i < bTree->size; i++) {
      if (key < bTree->keys[i]) {
        if (bTree->link[i]) {
          bTree->link[i] = bInsert(bTree->link[i], key);
          break;
        }
        rShift(bTree, i);
        bTree->keys[i] = key;
        bTree->size++;
        break;
      }
    }
  }

  return bTree;
}

void bDisplay(struct Node* bTree) {
  if (!bTree) return;

  int i = -1;
  while (i++ < bTree->size - 1) {
    bDisplay(bTree->link[i]);
    printf("%d ", bTree->keys[i]);
  }
  bDisplay(bTree->link[i]);

  return;
}

int main() {
  struct Node* bTree = NULL;
  int test[] = {23, 9,  7,  3, 45, 1, 5,  14, 25,
                24, 13, 11, 8, 19, 4, 31, 35, 56};
  int it = -1;

  while (++it < 18) {
    bTree = bInsert(bTree, test[it]);
  }

  bDisplay(bTree);

  return 0;
}
