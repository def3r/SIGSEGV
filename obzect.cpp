#include <any>
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>

class ObzectItem {
 public:
  ObzectItem *pItem;
  ObzectItem *nItem;

  virtual void setPPtr(ObzectItem *pItem) = 0;
  virtual void setNPtr(ObzectItem *nItem) = 0;
  virtual void setPtr(ObzectItem *pItem, ObzectItem *nItem) = 0;

  virtual std::any getKey() = 0;
  virtual std::any getValue() = 0;

  virtual void displayKey() = 0;
  virtual void displayValue() = 0;
};

template <class K, class V>
class ObzItemExplicit : public ObzectItem {
 protected:
  std::pair<K, V> newPair;

  template <typename T>
  void displayDecorate(T item) {
    if (typeid(T) == typeid(std::string)) {
      std::cout << "\"" << item << "\"";
    } else {
      std::cout << item;
    }
    return;
  }

 public:
  ObzItemExplicit(const K &key, const V &val) {
    this->newPair.first = key;
    this->newPair.second = val;

    this->pItem = nullptr;
    this->nItem = nullptr;
  }

  void setPPtr(ObzectItem *pItem) { this->pItem = pItem; }
  void setNPtr(ObzectItem *nItem) { this->nItem = nItem; }
  void setPtr(ObzectItem *pItem, ObzectItem *nItem) {
    this->pItem = pItem;
    this->nItem = nItem;
  }

  std::any getKey() { return this->newPair.first; }
  std::any getValue() { return this->newPair.second; }

  void displayKey() { this->displayDecorate(this->newPair.first); }
  void displayValue() { this->displayDecorate(this->newPair.second); }
};

class Obzect {
 protected:
  ObzectItem *head;
  ObzectItem *tail;

  ObzectItem *cursor;

 public:
  Obzect() : head(nullptr), tail(nullptr) {}

  template <typename K, typename V>
  void push(const K &key, const V &val) {
    ObzItemExplicit<K, V> *newItem = new ObzItemExplicit<K, V>(key, val);

    if (this->head == nullptr) {
      this->head = newItem;
    } else {
      this->tail->setNPtr(newItem);
    }

    newItem->setPtr(this->tail, nullptr);
    this->tail = newItem;

    return;
  }

  void push(const char *key, const char *val) {
    this->push(std::string(key), std::string(val));
  }

  template <typename V>
  void push(const char *key, V val) {
    this->push(std::string(key), val);
  }

  template <typename K>
  void push(K key, const char *val) {
    this->push(key, std::string(val));
  }

  template <typename K>
  bool find(const K &key) {
    ObzectItem *it = this->head;
    while (it) {
      // std::cout << "Ret Type: " << it->getKey().type().name();

      this->cursor = it;

      if (it->getKey().type() == typeid(key)) {
        K itKey = std::any_cast<K>(it->getKey());
        if (itKey == key) {
          return 1;
        }
      }
      it = it->nItem;
    }
    return 0;
  }

  bool find(const char *key) { return this->find(std::string(key)); }

  template <typename V, typename K>
  V key(const K &key) {
    if (!this->find(key)) {
      std::cerr << "Obzect cannot find the key specified: `" << key << "`\n";
      exit(1);
    }

    return std::any_cast<V>(this->cursor->getValue());
  }

  template <typename V, typename K>
  V key(const char *key) {
    return this->key<V, std::string>(std::string(key));
  }

  //  template <typename K>
  //  std::any key(const K &key) {
  //    if (!this->find(key)) {
  //      std::cerr << "\nObzect cannot find the key specified: `" << key <<
  //      "`\n"; exit(1);
  //    }
  //
  //    return this->cursor->getValue();
  //  }

  void display() {
    int decorateSpc = 2;
    ObzectItem *it = this->head;

    std::cout << "\n{\n";
    while (it) {
      int spc = decorateSpc;
      while (spc--) {
        std::cout << " ";
      }

      it->displayKey();
      std::cout << ": ";
      it->displayValue();
      std::cout << ",\n";

      it = it->nItem;
    }
    std::cout << "}\n";

    return;
  }
};

int main() {
  Obzect o1;
  o1.push("Name", "Ayaan");
  o1.push("Username", "Horrifyinghorse");
  o1.push("ID", 28);
  o1.push(5, 9);

  o1.display();

  if (o1.find("ID")) {
    std::cout << "Find return YES!";
  } else {
    std::cout << "Find return No!";
  }

  std::cout << "\nUsing key:";
  std::cout << "\nUsername = " << o1.key<std::string>("Name");
  std::cout << "\nID = " << o1.key<int>("ID");

  std::cout << "\n5 = " << o1.key<int>(5);

  return 0;
}
