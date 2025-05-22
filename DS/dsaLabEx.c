#include <stdio.h>
#include <stdlib.h>

struct node {
  int d, ht;
  struct node *left, *right;
};

int max(int a, int b) {
  return (a > b) ? a : b;
}

int getHt(struct node* root) {
  if (!root) return 0;

  return root->ht;
}

struct node* createNode(int d) {
  struct node* newNode = (struct node*)malloc(sizeof(struct node));
  newNode->d = d;
  newNode->ht = 0;
  newNode->left = NULL;
  newNode->right = NULL;

  return newNode;
}

struct node* insertBST(struct node* root, int d) {
  if (!root) {
    return createNode(d);
  }

  if (d < root->d) {
    root->left = insertBST(root->left, d);
  } else {
    root->right = insertBST(root->right, d);
  }

  root->ht = max(getHt(root->left), getHt(root->right)) + 1;

  return root;
}

int isAVL(struct node* root) {
  if (!root) return 1;

  if (!isAVL(root->left) || !isAVL(root->right)) return 0;

  int bf = getHt(root->left) - getHt(root->right);

  if (abs(bf) > 1) return 0;

  return 1;
}

struct node* rotateRight(struct node* root) {
  struct node* newRoot = root->left;
  root->left = newRoot->right;
  newRoot->right = root;

  root->ht = max(getHt(root->left), getHt(root->right)) + 1;
  newRoot->ht = max(getHt(newRoot->left), getHt(newRoot->right)) + 1;

  return newRoot;
}

struct node* rotateLeft(struct node* root) {
  struct node* newRoot = root->right;
  root->right = newRoot->left;
  newRoot->left = root;

  root->ht = max(getHt(root->left), getHt(root->right)) + 1;
  newRoot->ht = max(getHt(newRoot->left), getHt(newRoot->right)) + 1;

  return newRoot;
}

struct node* buildAVL(struct node* root) {
  if (isAVL(root)) return root;

  root->left = buildAVL(root->left);
  root->right = buildAVL(root->right);

  int bf = getHt(root->left) - getHt(root->right);

  if (bf > 1) { // Left Higher
    if (getHt(root->right) > getHt(root->left)) { // LR
      root->left = rotateLeft(root->left);
    }
    root = rotateRight(root);
  }
  else if(bf < -1) { // Right Higher
    if (getHt(root->left) > getHt(root->right)) { // RL
      root->right = rotateRight(root->right);
    }
    root = rotateLeft(root);
  }

  return root;
}

void preorder(struct node* root) {
  if (!root) return;

  printf("%d ", root->d);
  preorder(root->left);
  preorder(root->right);
}

int main() {
  struct node* tree = NULL;
  tree = insertBST(tree, 3);
  tree = insertBST(tree, 4);
  tree = insertBST(tree, 5);
  tree = insertBST(tree, 6);

  printf("Preorder of tree: ");
  printf("isAVL: %d\n", isAVL(tree));
  preorder(tree);

  tree = buildAVL(tree);

  printf("\nPreorder of AVL balanced tree: ");
  printf("isAVL: %d\n", isAVL(tree));
  printf("isAVL: %d\n", isAVL(tree));
  preorder(tree);

  return 0;
}