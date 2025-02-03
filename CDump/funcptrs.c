#include <stdio.h>
#include <stdlib.h>

typedef struct array {
  int size;
  int *arr;
  long (*sum)(struct array *);
  void (*display)(struct array *);

} Array;

long ArraySum(Array *arr) {
  long res = 0;
  for (int i = 0; i < arr->size; i++) {
    res += arr->arr[i];
  }
  return res;
}

void ArrayDisplay(Array *arr) {
  printf("[");
  for (int i = 0; i < arr->size; i++) {
    printf("%d, ", *(arr->arr + i));
  }
  if (arr->size > 0)
    printf("\b\b]");
  return;
}

Array *ArrayConstructor(Array *arr, int size, int content[]) {
  arr->size = size;
  arr->arr = (int *)calloc(size, sizeof(int));
  for (int i = 0; i <= sizeof(content) / sizeof(content[0]); i++) {
    arr->arr[i] = content[i];
  }
  arr->sum = &ArraySum;
  arr->display = &ArrayDisplay;
  return arr;
}

int main() {
  Array a;
  ArrayConstructor(&a, 5, (int[]){1, 2, 3});
  a.display(&a);

  return 0;
}
