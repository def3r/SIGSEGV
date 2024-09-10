#include <stdio.h>
#include <stdlib.h>

int heapCondition(int current, int parent, int flag) {
  if (flag && current > parent) return 1;
  if (!flag && current < parent) return 1;

  return 0;
}

int* heapify(int* mHeap, int size, int pos, int flag) {
  int i = 0;

  while (i != pos) {
    int curr = i;
    int parent = (curr - 1) / 2;

    while (parent >= 0 &&
           heapCondition(*(mHeap + curr), *(mHeap + parent), flag)) {
      int temp = *(mHeap + curr);
      *(mHeap + curr) = *(mHeap + parent);
      *(mHeap + parent) = temp;

      curr = parent;
      parent = (curr - 1) / 2;
    }

    i++;
  }

  return mHeap;
}

int* minHeapify(int* mHeap, int size, int pos) {
  heapify(mHeap, size, pos, 0);

  return mHeap;
}

int* maxHeapify(int* mHeap, int size, int pos) {
  heapify(mHeap, size, pos, 1);

  return mHeap;
}

int searchInHeap(int* mHeap, int size, int pos, int item) {
  int i = 0;

  while (pos--) {
    if (*(mHeap + i) == item) {
      return i;
    }
    i++;
  }

  return -1;
}

void heapDisplay(int* minHeap, int size, int pos) {
  int i = 0;

  while (i != pos) {
    printf("%d ", *(minHeap + i++));
  }

  printf("\n");

  return;
}

int* minHeapInsert(int* minHeap, int size, int* pos, int n) {
  if (*pos >= size) {
    printf("Heap Limit Reached.");
    exit(1);
  }

  int currPos = *pos;
  int parentPos = (currPos - 1) / 2;

  *(minHeap + currPos) = n;
  (*pos)++;

  minHeapify(&minHeap[0], size, *pos);

  // Operations just on the inserted element:
  //
  //  while (parentPos >= 0 && *(minHeap + parentPos) > n) {
  //    *(minHeap + currPos) = *(minHeap + parentPos);
  //    *(minHeap + parentPos) = n;
  //
  //    currPos = parentPos;
  //    parentPos = (currPos - 1) / 2;
  //  }

  return minHeap;
}

int* minHeapDelete(int* minHeap, int size, int* pos, int item) {
  int indx = searchInHeap(minHeap, size, *pos, item);

  if (indx == -1) {
    printf("Item (%d) does not exist in minHeap", item);
    exit(1);
  }

  *(minHeap + indx) = *(minHeap + *pos - 1);  // sice pos points to empty block
  (*pos)--;
  minHeapify(minHeap, size, *pos);

  return minHeap;
}

int deleteMax(int* minHeap, int size, int* pos) {
  int max = *(minHeap + 0);
  int i = 0;

  while (++i < *pos) {
    if (*(minHeap + i) > max) {
      max = *(minHeap + i);
    }
  }

  minHeapDelete(minHeap, size, pos, max);

  return max;
}

int* buildMaxHeap(int* arr, int size, int* pos) {
  maxHeapify(arr, size, *pos);

  return arr;
}

// Ascending Order Heapsort
int* heapSort(int* arr, int size, int pos) {
  // inplace heapsort Ist try
  int unsortIndx = 0;

  while (unsortIndx < pos) {
    minHeapify(arr + unsortIndx, size, pos - unsortIndx);
    unsortIndx++;
  }

  return arr;
}

int main() {
  int hSize = 31;  // 5 levels
  int pos = 0;
  int mHeap[hSize];

  int totalEl;

  minHeapInsert(&mHeap[0], hSize, &pos, 56);
  minHeapInsert(&mHeap[0], hSize, &pos, 34);
  minHeapInsert(&mHeap[0], hSize, &pos, 23);
  minHeapInsert(&mHeap[0], hSize, &pos, 89);
  minHeapInsert(&mHeap[0], hSize, &pos, 15);
  minHeapInsert(&mHeap[0], hSize, &pos, 37);
  minHeapInsert(&mHeap[0], hSize, &pos, 82);
  minHeapInsert(&mHeap[0], hSize, &pos, 98);
  minHeapInsert(&mHeap[0], hSize, &pos, 91);
  minHeapInsert(&mHeap[0], hSize, &pos, 55);

  heapDisplay(&mHeap[0], hSize, pos);

  printf("Delete the max element from minHeap:\n");
  deleteMax(&mHeap[0], hSize, &pos);

  heapDisplay(&mHeap[0], hSize, pos);
  minHeapInsert(&mHeap[0], hSize, &pos, 98);

  printf("Deleting element 15 from minHeap\n");
  minHeapDelete(&mHeap[0], hSize, &pos, 15);

  heapDisplay(&mHeap[0], hSize, pos);

  printf("maxHeapifying the current minHeap : \n");
  maxHeapify(&mHeap[0], hSize, pos);

  heapDisplay(&mHeap[0], hSize, pos);

  printf("Searching for 15: %d\n", searchInHeap(&mHeap[0], hSize, pos, 15));

  printf("\n\nThe dynamic part:");
  printf("\nEnter Total Elements in your Array: ");
  scanf("%d", &totalEl);

  if (totalEl > hSize) {
    printf("Aforementioned Array cannot represent a maxHeap of size %d\n",
           hSize);
    exit(1);
  }

  int arrToHeap[hSize];
  int pos2 = totalEl;
  int i = 0;

  for (i = 0; i < totalEl; i++) {
    printf("Enter Array[%d]: ", i);
    scanf("%d", &arrToHeap[i]);
  }

  printf("\n\nInitial Array: ");
  heapDisplay(&arrToHeap[0], hSize, pos2);

  buildMaxHeap(&arrToHeap[0], hSize, &pos2);

  printf("After Max Heapifying it: ");
  heapDisplay(&arrToHeap[0], hSize, pos2);

  printf("In place Heap Sort on the array: ");
  heapSort(&arrToHeap[0], hSize, pos2);
  heapDisplay(&arrToHeap[0], hSize, pos2);

  return 0;
}
