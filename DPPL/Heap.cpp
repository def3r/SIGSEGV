#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <utility>
#include <vector>

class Heap {
 public:
  Heap() { Initialize(); }
  Heap(size_t freeListSize) : freeListSize(freeListSize) { Initialize(); }

  void insert(int data) {
    if (!ExtractNode()) {
      std::cout << "Heap Memory fully utilised, cannot insert " << data << "\n";
      exit(1);
    }
    ptr->data = data;
    InsertHeap();
  }
  void insert(const std::vector<int>& datav) {
    for (const auto& data : datav) {
      insert(data);
    }
  }

  void remove() {
    if (!ExtractMin()) {
      std::cout << "Cannot remove from a heap with size " << heap.size()
                << "\n";
      exit(1);
    }
    InsertNode(ptr);
  }

  void display() {
    std::cout << "Free List: ";
    ptr = freeList;
    while (ptr != nullptr) {
      std::cout << ptr->data << " <-> ";
      ptr = ptr->next;
    }
    std::cout << "nullptr\n";

    std::cout << "Heap: ";
    for (const auto& node : heap) {
      std::cout << node->data << " ";
    }
    std::cout << "\n";
  }

  void tree() {
    int n = heap.size();
    int levels = std::log2(n) + 1;
    int index = 0;

    for (int level = 0; level < levels; ++level) {
      int nodesOnLevel = std::pow(2, level);
      int spacing = std::pow(2, levels - level) - 1;
      for (int i = 0; i < spacing / 2; ++i) {
        std::cout << "   ";
      }

      for (int i = 0; i < nodesOnLevel && index < n; ++i) {
        std::cout << std::setw(2) << heap[index++]->data;
        for (int j = 0; j < spacing; ++j) {
          std::cout << "   ";
        }
      }
      std::cout << "\n";
    }
  }

 private:
  typedef struct Node {
    Node* prev = nullptr;
    int data = 0;
    Node* next = nullptr;
  } Node;
  Node *freeList = nullptr, *ptr = nullptr;
  std::vector<Node*> heap = {};
  size_t freeListSize = 10;

  void Initialize() {
    for (int i = 0; i < freeListSize; i++) {
      if (!ptr) {
        freeList = new Node;
        ptr = freeList;
        continue;
      }
      ptr->next = new Node;
      ptr->next->prev = ptr;
      ptr = ptr->next;
    }
    ptr = freeList;
  }

  void InsertNode(Node* node) {
    if (freeList) {
      freeList->prev = node;
      node->next = freeList;
    }
    freeList = node;
  }

  Node* ExtractNode() {
    if (!freeList) {
      return nullptr;
    }
    ptr = freeList;
    freeList = freeList->next;
    if (freeList) {
      freeList->prev = nullptr;
    }
    ptr->next = nullptr;
    ptr->prev = nullptr;
    return ptr;
  }

  void InsertHeap() {
    if (!ptr) {
      std::cout << "Cannot insert a nullptr to min heap";
      exit(1);
    }
    heap.push_back(ptr);
    int i = heap.size() - 1;
    while (i != 0 && heap[(i - 1) / 2]->data > heap[i]->data) {
      std::swap(heap[(i - 1) / 2], heap[i]);
      i = (i - 1) / 2;
    }
  }

  void Heapify(size_t index) {
    size_t smallest = index;
    size_t left = 2 * index + 1;
    size_t right = 2 * index + 2;
    if (left < heap.size() && heap[left]->data < heap[smallest]->data) {
      smallest = left;
    }
    if (right < heap.size() && heap[right]->data < heap[smallest]->data) {
      smallest = right;
    }
    if (smallest != index) {
      std::swap(heap[index], heap[smallest]);
      Heapify(smallest);
    }
  }

  Node* ExtractMin() {
    if (!heap.size()) {
      return nullptr;
    }
    if (heap.size() == 1) {
      ptr = heap[0];
      heap.pop_back();
      return ptr;
    }
    ptr = heap[0];
    heap[0] = heap[heap.size() - 1];
    heap.pop_back();
    Heapify(0);
    return ptr;
  }
};

int main() {
  Heap heap;

  heap.insert({3, 9, 4, 11, 5, 1});
  heap.display();
  heap.remove();
  heap.display();
  heap.insert({7, 1, 8, 6});
  // heap.insert({17, 11, 81, 61, 10, 22, 23});
  heap.display();
  heap.tree();

  return 1;
}
