#include <stdio.h>
#include <stdlib.h>

struct node {
  int num;
  int pos;
  struct node* link;
};

struct nodeList {
  struct node* nodes;
  struct nodeList* link;
};

struct node* insertNode(struct node* head, int n, int p) {
  struct node* newNode = (struct node*)malloc(sizeof(struct node));
  newNode->num = n;
  newNode->pos = p;
  newNode->link = NULL;

  if (head == NULL) { return newNode; }

  struct node* temp = head;
  struct node* prev = head;

  // edge case: insertion @ beginning:
  // eg insert a node which is present @pos 2
  // but currently nodes are (pos): 4->6->NULL
  if (head->pos > p) {
    newNode->link = head;
    return newNode;
  }

  while(temp != NULL && temp->pos < p) {
    prev = temp;
    temp = temp->link;
  }

  if (temp != NULL && temp->pos == p) {
    temp->num = n;
    free(newNode);
    return head;
  }

  newNode->link = temp;
  prev->link = newNode;

  return head;
}

struct nodeList* initList (int rows) {
  struct nodeList* head = NULL;
  int i = 0;

  for (i = 0; i < rows; i++) {
    struct nodeList* new = (struct nodeList*)malloc(sizeof(struct nodeList));
    new->nodes = NULL;
    new->link = head;
    head = new;
  }

  return head;
}

struct nodeList* insertInList (struct nodeList* m, int r, int c, int d) {
  struct nodeList* temp = m;
  int i;

  if (m == NULL) { return NULL; }

  for (i = 0; i < r; i++) {
    temp = temp->link;
  }

  temp->nodes = insertNode(temp->nodes, d, c);

  return m;
}

void displaySparseMatrix(struct nodeList* matrix, int r, int c) {
  struct node* temp = NULL;

  while (matrix != NULL) {
    temp = matrix->nodes;

    for (int i = 0; i < c; i++) {
      if (temp != NULL && temp->pos - 1 == i) {
        printf("%d ", temp->num);
        temp = temp->link;
        continue;
      }

      printf ("0 ");
    }

    printf("\n");
    matrix = matrix->link;
  }
}

void uiCreateSparseMatrix() {
  int x, y;
  int n;
  int row, col;
  float data;

  printf("Enter matrix dim: [x, y]: ");
  scanf("%d %d", &x, &y);

  struct nodeList* matrix = initList(x);

  printf("Enter Total Members to Insert: ");
  scanf("%d", &n);

  printf("Enter Data into Sparse Matrix: [Row, Col, Data]\n");
  for (int i = 0; i < n; i++) {
    printf("%d: ", i + 1);
    scanf("%d %d %f", &row, &col, &data);

    row-= 1;

    if (row > x || col > y) {
      printf("Invalid row or col");
      exit(1);
    }

    insertInList(matrix, row, col, data);
  }

  printf("\nYour Sparse Matrix is:\n");
  displaySparseMatrix(matrix, x, y);

}

int main() {
  uiCreateSparseMatrix();

  return 0;
}
