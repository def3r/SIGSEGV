#include <iostream>
#include <ostream>

template <typename T>
class _bstreenode;

template <typename T>
class BSTree;

template <typename T>
class _bstreenode {
 protected:
  T data;
  _bstreenode *left, *right;

 public:
  _bstreenode() {}
  _bstreenode(T data) : data(data), left(nullptr), right(nullptr) {}
  _bstreenode(_bstreenode* left, T data)
      : data(data), left(left), right(nullptr) {}
  _bstreenode(T data, _bstreenode* right)
      : data(data), left(nullptr), right(right) {}
  _bstreenode(_bstreenode* left, T data, _bstreenode* right)
      : data(data), left(left), right(right) {}

  friend class BSTree<T>;
};

template <typename T>
class BSTree {
 protected:
  _bstreenode<T>* root;

  _bstreenode<T>*cur, *prev;
  bool dataExists;
  long size;

  void _ptrReset() {
    cur = root;
    prev = root;
  }

  void _searchBSTree(T data) {
    if (cur == nullptr) {
      cur = prev;
      return;
    }

    if (data == cur->data) {
      this->dataExists = true;
      return;
    }

    prev = cur;
    if (data > cur->data) {
      cur = cur->right;
    } else if (data < cur->data) {
      cur = cur->left;
    }

    _searchBSTree(data);

    return;
  }

  void _inorderSucc() {
    prev = cur;
    cur = cur->right;
    while (cur != nullptr && cur->left != nullptr) {
      prev = cur;
      cur = cur->left;
    }
  }

  void _inorder() {
    if (this->cur == nullptr) return;

    _bstreenode<T>* cur = this->cur;

    this->cur = cur->left;
    _inorder();

    std::cout << cur->data << " ";

    this->cur = cur->right;
    _inorder();
  }

  void _removeHelper(T data) {
    _searchBSTree(data);

    if (!dataExists) {
      return;
    }

    dataExists = false;

    // root node with no succ:
    if (prev == cur && cur->right == nullptr) {
      root = root->left;
      prev = root;

      this->size--;
      delete cur;
      cur = nullptr;
      return;
    }

    // leaf node:
    if (cur->left == nullptr && cur->right == nullptr) {
      if (cur == prev->right)
        prev->right = nullptr;
      else
        prev->left = nullptr;

      this->size--;
      delete cur;
      cur = nullptr;
      return;
    }

    // @least 1 child
    if (cur->left != nullptr && cur->right == nullptr) {
      if (cur == prev->right)
        prev->right = cur->left;
      else
        prev->left = cur->left;

      this->size--;
      delete cur;
      cur = nullptr;
      return;
    }
    if (cur->right != nullptr && cur->left == nullptr) {
      if (cur == prev->right)
        prev->right = cur->right;
      else
        prev->left = cur->right;

      this->size--;
      delete cur;
      cur = nullptr;
      return;
    }

    // both child
    _bstreenode<T>* el = cur;
    _inorderSucc();
    el->data = cur->data;
    this->cur = el->right;
    _removeHelper(el->data);

    return;
  }

  void _clear() {
    if (this->cur == nullptr) return;
    _bstreenode<T>* cur = this->cur;

    this->cur = cur->left;
    _clear();

    this->cur = cur->right;
    _clear();

    delete cur;
    cur = nullptr;
  }

 public:
  BSTree() : root(nullptr), cur(this->root), size(0), dataExists(false) {}

  ~BSTree() {
    _ptrReset();
    _clear();
    root = nullptr;
  }

  BSTree<T>& insert(T data) {
    _ptrReset();
    _searchBSTree(data);

    // condition when cur = root;
    if (cur == nullptr) {
      root = new _bstreenode<T>(data);
      return *this;
    }

    _bstreenode<T>* newNode = new _bstreenode<T>(data);
    if (data > cur->data) {
      cur->right = newNode;
    } else {
      cur->left = newNode;
    }
    this->size++;

    return *this;
  }

  BSTree<T>& remove(T data) {
    _ptrReset();
    _removeHelper(data);

    return *this;
  }

  BSTree<T>& clear() {
    _ptrReset();
    _clear();

    this->root = nullptr;
    this->size = 0;

    return *this;
  }

  bool search(T data) {
    _ptrReset();
    _searchBSTree(data);

    bool found = dataExists;
    dataExists = false;

    return found;
  }

  void inorder() {
    _ptrReset();
    _inorder();
    std::cout << "\n";
  }
};

int main() {
  BSTree<std::string> t;
  std::string a[] = {"Apple", "Aaple", "Banana", "Camela", "Zoo", "Joe Hawley"};

  for (auto i : a) {
    t.insert(i);
  }
  t.inorder();

  if (t.search("Joe Hawley")) {
    std::cout << "\nyelwaH eoJ";
  }
}
