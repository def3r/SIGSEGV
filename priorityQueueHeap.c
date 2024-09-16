#include <stdio.h>

// returning a 2D array
int (*maxHeapify(int arr[][2], int hSize, int pos))[2] {
  if (!pos) {
    return arr;
  }

  int c = 0;  // current

  while (c != pos) {
    int curr = c;
    int parent = (curr - 1) / 2;

    while (arr[curr][0] > arr[parent][0]) {
      int tempP = arr[curr][0], tempT = arr[curr][1];
      arr[curr][0] = arr[parent][0];
      arr[curr][1] = arr[parent][1];

      arr[parent][0] = tempP;
      arr[parent][1] = tempT;

      curr = parent;
      parent = (curr - 1) / 2;
    }

    c++;
  }

  return arr;
}

int (*heapSort(int arr[][2], int size, int pos))[2] {
  int sortedIndx = 0;

  while (pos--) {
    maxHeapify(&arr[sortedIndx++], size, pos);
  }

  return arr;
}

void displayArr(int arr[][2], int size, int pos) {
  int i = 0;
  while (pos--) {
    printf("%d %d,  ", arr[i][0], arr[i][1]);
    i++;
  }
  printf("\n");

  return;
}

int main() {
  int hSize = 31;  // 5 levels
  int pos = 0;
  int pQueue[hSize][2];

  int tasks = 0;

  printf("Enter the number of tasks: ");
  scanf("%d", &tasks);
  pos = tasks;

  int i = 0;
  printf("Enter Task Priorities:\n");
  while (tasks--) {
    printf("Enter {priority task_num} of task [#%d]: ", i);
    scanf("%d %d", &pQueue[i][0], &pQueue[i][1]);
    i++;
  }

  printf("\nPriorities as Collected: ");
  displayArr(&pQueue[0], hSize, pos);

  heapSort(&pQueue[0], hSize, pos);

  printf("As per Prioirty [Higher Priority Executes First]: ");
  displayArr(&pQueue[0], hSize, pos);

  return 0;
}
