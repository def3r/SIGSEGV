#include <stdio.h>
#include <stdlib.h>

struct node {
  struct node *left;
  int n;
  struct node *right;
};

struct node *top = NULL;
struct node *mid = NULL;
struct node *btm = NULL; // bottom imbalanced node
int score = 0;

int postorderBalanceTraverse(struct node *);

struct node *initTree() {
  struct node *rootNode = (struct node *)malloc(sizeof(struct node));

  return rootNode;
}

struct node *createNewNode(int n) {
  struct node *newNode = (struct node *)malloc(sizeof(struct node));
  newNode->n = n;
  newNode->left = NULL;
  newNode->right = NULL;

  return newNode;
}

int height(struct node *root) {
  int leftHeight = 0;
  int rightHeight = 0;
  if (root == NULL) {
    return 1;
  }

  leftHeight = 1 + height(root->left);
  rightHeight = 1 + height(root->right);

  return leftHeight > rightHeight ? leftHeight : rightHeight;
}

int postorderBalanceTraverse(struct node *tree) {
  int leftHeight = 0;
  int rightHeight = 0;

  if (tree == NULL) {
    return 0;
  }

  leftHeight = 1 + postorderBalanceTraverse(tree->left);
  rightHeight = 1 + postorderBalanceTraverse(tree->right);

  printf("%d ", tree->n);
  printf("%d\n", leftHeight - rightHeight);

  return leftHeight > rightHeight ? leftHeight : rightHeight;
}

struct node *insertNode(struct node **rootNode, int n) {
  top = NULL;
  mid = NULL;
  btm = NULL;

  if (*rootNode == NULL) {
    top = createNewNode(n);
    return top;
  }

  if (n < (*rootNode)->n) {
    (*rootNode)->left = insertNode(&((*rootNode)->left), n);
  } else {
    (*rootNode)->right = insertNode(&((*rootNode)->right), n);
  }

  btm = mid;
  mid = top;
  top = *rootNode;

  score = height(top->left) - height(top->right);
  //  printf("val %d: %d| ", top->n, score);
  //  printf("\n");
  if (abs(score) > 1) {
    if (score < -1) {
      if (mid->left == btm) {
        // RL rotation
        top->right = btm->left;
        mid->left = btm->right;
        btm->left = top;
        btm->right = mid;

        *rootNode = btm;
        top = btm;
      } else {
        // RR rotation
        top->right = mid->left;
        mid->left = top;

        *rootNode = mid;
        top = mid;
        mid = btm;
      }
    } else if (score > 1) {
      if (mid->right == btm) {
        // LR rotation
        top->left = btm->right;
        mid->right = btm->left;
        btm->left = mid;
        btm->right = top;

        *rootNode = btm;
        top = btm;
      } else {
        // LL rotation
        top->left = mid->right;
        mid->right = top;

        *rootNode = mid;
        top = mid;
        mid = btm;
      }
    }
  }

  //  if (mid) {
  //    printf("val %d: %d| ", mid->n, height(mid->left) - height(mid->right));
  //  }
  //  if (btm) {
  //    printf("val %d: %d| ", btm->n, height(btm->left) - height(btm->right));
  //  }

  // postorderBalanceTraverse(rootNode);

  return *rootNode;
}

void inorderTraverse(struct node *tree) {
  if (tree == NULL) {
    return;
  }

  printf("%d ", tree->n);
  inorderTraverse(tree->left);
  inorderTraverse(tree->right);

  return;
}

int main() {
  struct node *avlTree = NULL;

  // Final Test:
  avlTree = insertNode(&avlTree, 21);
  insertNode(&avlTree, 26);
  insertNode(&avlTree, 30);
  insertNode(&avlTree, 9);
  insertNode(&avlTree, 4);
  insertNode(&avlTree, 14);
  insertNode(&avlTree, 28);
  insertNode(&avlTree, 18);
  insertNode(&avlTree, 15);
  insertNode(&avlTree, 10);
  insertNode(&avlTree, 2);
  insertNode(&avlTree, 3);
  insertNode(&avlTree, 7);

  // LL
  //  avlTree = insertNode(&avlTree, 3);
  //  insertNode(&avlTree, 2);
  //  insertNode(&avlTree, 1);

  // RR
  //  avlTree = insertNode(&avlTree, 5);
  //  insertNode(&avlTree, 2);
  //  insertNode(&avlTree, 9);
  //  insertNode(&avlTree, 3);
  //  insertNode(&avlTree, 6);
  //  insertNode(&avlTree, 11);
  //  insertNode(&avlTree, 10);
  //  insertNode(&avlTree, 4);
  //  insertNode(&avlTree, 12);
  //  insertNode(&avlTree, 13);

  //  insertNode(avlTree, 5);
  //  insertNode(avlTree, 5);
  //  insertNode(avlTree, 1);
  //  insertNode(avlTree, 4);
  //  insertNode(avlTree, 2);
  //  insertNode(avlTree, 7);
  //  insertNode(avlTree, 9);
  //  insertNode(avlTree, 10);

  inorderTraverse(avlTree);
  printf("\n\n");
  //  postorderBalanceTraverse(avlTree);
  //  printf("\n");

  return 0;
}
