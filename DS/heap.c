#include <stdio.h>
#include <stdlib.h>

int* maxHeapInsert(int* heap, int size, int* pos, int n) {
  if (*pos >= size) {
    printf("Heap has reached it's limit");
    exit(1);
  }

  int currPos = (*pos);
  int parentPos = (currPos - 1) / 2;

  *(heap + *pos) = n;
  *pos += 1;

  if (*pos == 1) return heap;  // indexing breaks when pos is 0

  while (*(heap + parentPos) < n) {
    int temp = *(heap + parentPos);
    *(heap + parentPos) = n;
    *(heap + currPos) = temp;

    currPos = parentPos;
    parentPos = (currPos - 1) / 2;

    if (currPos == 0) break;  // indexing breaks when pos is 0
  }

  return heap;
}

int* minHeapInsert(int* heap, int size, int* pos, int n) {
  if (*pos >= size) {
    printf("Heap has reached it's limit");
    exit(1);
  }

  int currPos = (*pos);
  int parentPos = (currPos - 1) / 2;

  *(heap + *pos) = n;
  *pos += 1;

  if (*pos == 1) return heap;  // indexing breaks when pos is 0

  while (*(heap + parentPos) > n) {
    int temp = *(heap + parentPos);
    *(heap + parentPos) = n;
    *(heap + currPos) = temp;

    currPos = parentPos;
    parentPos = (currPos - 1) / 2;

    if (currPos == 0) break;  // indexing breaks when pos is 0
  }

  return heap;
}

void displayHeap(int* heap, int size, int pos) {
  if (pos >= size) {
    printf("Invalid heap provided to display");
    exit(1);
  }

  printf("Heap in order [top to bottom; left to right]: ");

  int i = 0;
  while (pos--) {
    printf("%d ", *(heap + i));
    i++;
  }

  printf("\n");

  return;
}

int main() {
  // heap with indexing 0
  int maxHeapSize = 31;  // 5 levels
  int maxHeap[maxHeapSize];
  int maxPos = 0;

  int minHeapSize = 31;
  int minHeap[minHeapSize];
  int minPos = 0;

  int testArr[] = {3, 1, 6, 4, 9, 8, 5};

  for (int i = 0; i < 7; i++) {
    maxHeapInsert(&maxHeap[0], maxHeapSize, &maxPos, testArr[i]);
  }

  for (int i = 0; i < 7; i++) {
    minHeapInsert(&minHeap[0], minHeapSize, &minPos, testArr[i]);
  }

  displayHeap(&maxHeap[0], maxHeapSize, maxPos);
  displayHeap(&minHeap[0], minHeapSize, minPos);
  return 0;
}
