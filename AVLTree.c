#include <stdio.h>
#include <stdlib.h>


struct node {
  struct node* left;
  int n;
  struct node* right;
};

int postorderBalanceTraverse(struct node*);

struct node* initTree () {
  struct node* rootNode = (struct node*)malloc(sizeof(struct node));

  return rootNode;
}

struct node* createNewNode(int n) {
  struct node* newNode = (struct node*)malloc(sizeof(struct node));
  newNode->n = n;
  newNode->left = NULL;
  newNode->right = NULL;

  return newNode;
}

struct node* insertNode(struct node* rootNode, int n) {
  if (rootNode == NULL) {
    return createNewNode(n); 
  }
  
  if (n < rootNode->n) {
    rootNode->left = insertNode(rootNode->left, n);
  } else {
    rootNode->right = insertNode(rootNode->right, n);
  }

//  postorderBalanceTraverse(rootNode);
//  printf("\n");
  return rootNode;
}

int height(struct node* root) {
  int leftHeight = 0;
  int rightHeight = 0;
  if (root == NULL) { return 1; }

  leftHeight = 1 + height(root->left);
  rightHeight = 1 + height(root->right);

  return leftHeight > rightHeight ? leftHeight : rightHeight;
}

int postorderBalanceTraverse(struct node* tree) {
  int leftHeight = 0;
  int rightHeight = 0;

  if (tree == NULL) { return 0; }

  leftHeight = 1 + postorderBalanceTraverse(tree->left);
  rightHeight = 1 + postorderBalanceTraverse(tree->right);

  printf("%d ", tree->n);
  printf("%d\n", leftHeight - rightHeight);

  return leftHeight > rightHeight ? leftHeight : rightHeight;
}

void inorderTraverse(struct node* tree) {
  if (tree == NULL) { return; }

  inorderTraverse(tree->left);
  printf("%d ", tree->n);
  inorderTraverse(tree->right);

  return;
}

int main() {
  struct node* avlTree = NULL;
  avlTree = insertNode(avlTree, 3);
  insertNode(avlTree, 5);
  insertNode(avlTree, 1);
  insertNode(avlTree, 4);
  insertNode(avlTree, 2);
  insertNode(avlTree, 7);
  insertNode(avlTree, 9);
  insertNode(avlTree, 10);

  inorderTraverse(avlTree); 
  printf("\n");

  return 0;
}
