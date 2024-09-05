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
int flagCheck = 0;

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
  int score = 0;

  if (tree == NULL) {
    return 0;
  }

  leftHeight = 1 + postorderBalanceTraverse(tree->left);
  rightHeight = 1 + postorderBalanceTraverse(tree->right);
  score = leftHeight - rightHeight;

  if (abs(score) > 1 && !flagCheck) {
    printf("Inavlid AVL Tree at node: %d\n", tree->n);
    printf("balance Factor: %d\n", score);
    flagCheck = 1;
  }

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
    printf("Unbalanced Node after insertion: %d", top->n);
    printf("\nBalance Factor: %d\n", score);

    if (score < -1) {
      if (mid->left == btm) {
        // RL rotation
        printf("Applying RL Rotation on nodes %d->%d->%d", top->n, mid->n,
               btm->n);
        top->right = btm->left;
        mid->left = btm->right;
        btm->left = top;
        btm->right = mid;

        *rootNode = btm;
        top = btm;
      } else {
        // RR rotation
        printf("Applying RR Rotation on nodes %d->%d->%d", top->n, mid->n,
               btm->n);
        top->right = mid->left;
        mid->left = top;

        *rootNode = mid;
        top = mid;
        mid = btm;
      }
    } else if (score > 1) {
      if (mid->right == btm) {
        // LR rotation
        printf("Applying LR Rotation on nodes %d->%d->%d", top->n, mid->n,
               btm->n);
        top->left = btm->right;
        mid->right = btm->left;
        btm->left = mid;
        btm->right = top;

        *rootNode = btm;
        top = btm;
      } else {
        // LL rotation
        printf("Applying LL Rotation on nodes %d->%d->%d", top->n, mid->n,
               btm->n);
        top->left = mid->right;
        mid->right = top;

        *rootNode = mid;
        top = mid;
        mid = btm;
      }
    }
    printf("\n\n");
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

struct node *deleteNode(struct node **rootNode, int n) {
  if (*rootNode == NULL) {
    return NULL;
  }

  if (n < (*rootNode)->n) {
    (*rootNode)->left = deleteNode(&(*rootNode)->left, n);
  } else if (n > (*rootNode)->n) {
    (*rootNode)->right = deleteNode(&(*rootNode)->right, n);
  } else {
    struct node *temp = *rootNode;
    if (temp->right) {
      while (temp->right->right) {
        temp = temp->right;
      }
      struct node *max = temp->right;
      temp->right = max->left;
      max->left = (*rootNode)->left;
      max->right = (*rootNode)->right;
      *rootNode = max;
    } else {
      *rootNode = temp->left;
    }
  }

  if (*rootNode == NULL) {
    return NULL;
  }

  top = *rootNode;
  score = height(top->left) - height(top->right);
  //  printf("val %d: %d| ", top->n, score);
  //  printf("\n");
  if (abs(score) > 1) {
    printf("Unbalanced Node after deletion: %d", top->n);
    printf("\nBalance Factor: %d\n", score);

    int leftHt = height(top->left);
    int rtHt = height(top->right);
    if (leftHt > rtHt) {
      mid = top->left;
    } else {
      mid = top->right;
    }

    leftHt = height(mid->left);
    rtHt = height(mid->right);
    if (leftHt > rtHt) {
      btm = mid->left;
    } else {
      btm = mid->right;
    }

    if (score < -1) {
      if (mid->left == btm) {
        // RL rotation
        printf("Applying RL Rotation on nodes %d->%d->%d", top->n, mid->n,
               btm->n);
        top->right = btm->left;
        mid->left = btm->right;
        btm->left = top;
        btm->right = mid;

        *rootNode = btm;
        top = btm;
      } else {
        // RR rotation
        printf("Applying RR Rotation on nodes %d->%d->%d", top->n, mid->n,
               btm->n);
        top->right = mid->left;
        mid->left = top;

        *rootNode = mid;
        top = mid;
        mid = btm;
      }
    } else if (score > 1) {
      if (mid->right == btm) {
        // LR rotation
        printf("Applying LR Rotation on nodes %d->%d->%d", top->n, mid->n,
               btm->n);
        top->left = btm->right;
        mid->right = btm->left;
        btm->left = mid;
        btm->right = top;

        *rootNode = btm;
        top = btm;
      } else {
        // LL rotation
        printf("Applying LL Rotation on nodes %d->%d->%d", top->n, mid->n,
               btm->n);
        top->left = mid->right;
        mid->right = top;

        *rootNode = mid;
        top = mid;
        mid = btm;
      }
    }
    printf("\n\n");
  }

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
  avlTree = insertNode(&avlTree, 10);
  insertNode(&avlTree, 20);
  insertNode(&avlTree, 30);
  insertNode(&avlTree, 40);
  insertNode(&avlTree, 50);
  insertNode(&avlTree, 25);

  //  avlTree = insertNode(&avlTree, 30);
  //  insertNode(&avlTree, 20);
  //  insertNode(&avlTree, 70);
  //  insertNode(&avlTree, 10);
  //  insertNode(&avlTree, 26);
  //  insertNode(&avlTree, 55);
  //  insertNode(&avlTree, 80);
  //  insertNode(&avlTree, 24);
  //  insertNode(&avlTree, 28);
  //  insertNode(&avlTree, 50);
  //  insertNode(&avlTree, 60);
  //  insertNode(&avlTree, 90);
  //  insertNode(&avlTree, 57);
  //
  // Final Test:
  //  avlTree = insertNode(&avlTree, 21);
  //  insertNode(&avlTree, 26);
  //  insertNode(&avlTree, 30);
  //  insertNode(&avlTree, 9);
  //  insertNode(&avlTree, 4);
  //  insertNode(&avlTree, 14);
  //  insertNode(&avlTree, 28);
  //  insertNode(&avlTree, 18);
  //  insertNode(&avlTree, 15);
  //  insertNode(&avlTree, 10);
  //  insertNode(&avlTree, 2);
  //  insertNode(&avlTree, 3);
  //  insertNode(&avlTree, 7);

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
  avlTree = deleteNode(&avlTree, 20);
  inorderTraverse(avlTree);
  printf("\n\n");
  avlTree = deleteNode(&avlTree, 25);
  inorderTraverse(avlTree);
  printf("\n\n");
  avlTree = deleteNode(&avlTree, 10);
  inorderTraverse(avlTree);
  // postorderBalanceTraverse(avlTree);
  // if (!flagCheck) {
  //   printf("Valid AVL Tree");
  // }

  //  avlTree = deleteNode(&avlTree, 26);
  //  inorderTraverse(avlTree);

  //  printf("\n\n");
  //  avlTree = deleteNode(&avlTree, 28);
  //  inorderTraverse(avlTree);

  //  printf("\n\n");
  //  avlTree = deleteNode(&avlTree, 24);
  //  inorderTraverse(avlTree);

  //  postorderBalanceTraverse(avlTree);
  //  printf("\n");

  return 0;
}
