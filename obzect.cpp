#include <any>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

class ObzectItem {
 public:
  ObzectItem *pItem;
  ObzectItem *nItem;

  virtual ~ObzectItem() {}

  virtual void setPPtr(ObzectItem *pItem) = 0;
  virtual void setNPtr(ObzectItem *nItem) = 0;
  virtual void setPtr(ObzectItem *pItem, ObzectItem *nItem) = 0;

  virtual std::any getKey() = 0;
  virtual std::any getKeyPtr() = 0;
  virtual bool setKey(const std::any &) = 0;

  virtual std::any getValue() = 0;
  virtual std::any getValuePtr() = 0;
  virtual bool setValue(const std::any &) = 0;

  virtual void displayKey() = 0;
  virtual void displayValue() = 0;
};

template <class K, class V>
class ObzItemExplicit : public ObzectItem {
 protected:
  std::pair<K, V> kvPair;

  template <typename T>
  T decorate(T item) {
    if constexpr (std::is_same<T, std::string>::value) {
      return "\"" + item + "\"";
    }
    return item;
  }

  K decorateKey() { return this->decorate(kvPair.first); }
  V decorateValue() { return this->decorate(kvPair.second); }

 public:
  ObzItemExplicit(const K &key, const V &val) {
    this->kvPair.first = key;
    this->kvPair.second = val;

    this->pItem = nullptr;
    this->nItem = nullptr;
  }

  void setPPtr(ObzectItem *pItem) { this->pItem = pItem; }
  void setNPtr(ObzectItem *nItem) { this->nItem = nItem; }
  void setPtr(ObzectItem *pItem, ObzectItem *nItem) {
    this->pItem = pItem;
    this->nItem = nItem;
  }

  std::any getKey() { return this->kvPair.first; }

  std::any getKeyPtr() { return &this->kvPair.first; }

  bool setKey(const std::any &key) {
    if (key.type() != typeid(K)) {
      std::cerr << "\nNew key type(" << key.type().name()
                << ") differs from init key(" << this->decorateKey()
                << ") type(" << typeid(this->kvPair.first).name() << ")";
      return false;
    }

    this->kvPair.first = std::any_cast<K>(key);

    return true;
  }

  std::any getValue() { return this->kvPair.second; }

  std::any getValuePtr() { return &this->kvPair.second; }

  bool setValue(const std::any &val) {
    if (val.type() != typeid(V)) {
      std::cerr << "\nNew value type(" << val.type().name()
                << ") differs from init val(" << this->decorateValue()
                << ") type(" << typeid(this->kvPair.second).name() << ")";
      return false;
    }

    this->kvPair.second = std::any_cast<V>(val);

    return true;
  }

  void displayKey() { std::cout << this->decorateKey(); }
  void displayValue() { std::cout << this->decorateValue(); }
};

class Obzect {
 protected:
  ObzectItem *head;
  ObzectItem *tail;

  ObzectItem *cursor;

 public:
  Obzect() : head(nullptr), tail(nullptr) {}

  ~Obzect() {
    ObzectItem *it = this->head;
    while (it) {
      ObzectItem *del = it;
      delete del;
      it = it->nItem;
    }
  }

  template <typename K, typename V>
  void push(const K &key, const V &val) {
    if (this->find(key)) {
      cursor->setValue(std::any(val));
      return;
    }

    ObzItemExplicit<K, V> *newItem = new ObzItemExplicit<K, V>(key, val);

    if (this->head == nullptr) {
      this->head = newItem;
    } else {
      this->tail->setNPtr(newItem);
    }

    this->tail = newItem;
    newItem->setPtr(this->tail, nullptr);

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

    if constexpr (std::is_same<V, std::any>::value) {
      return this->cursor->getValue();
    }

    return std::any_cast<V>(this->cursor->getValue());
  }

  template <typename V, typename K>
  V key(const char *key) {
    return this->key<V, std::string>(std::string(key));
  }

  void clear() {
    ObzectItem *it = this->head;
    while (it) {
      ObzectItem *del = it;
      delete del;
      it = it->nItem;
    }
    this->head = nullptr;
    this->tail = nullptr;
    this->tail = nullptr;
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

  template <typename K>
  std::any operator[](const K &key) {
    return this->key<std::any>(key);
  }

  std::any operator[](const char *key) { return this->key<std::any>(key); }
};

int main() {
  Obzect o1;
  o1.push("Name", "Ayaan");
  o1.push("Username", "Horrifyinghorse");
  o1.push("ID", 28);
  o1.push(5, 9);
  o1.push(3.14, "Pi");
  o1.push(3.14, "22/7");

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

  std::cout << "\nOverriding name";
  o1.push("Name", "Khan");

  std::cout << "\nName: " << std::any_cast<int>(o1[5]);

  o1.clear();
  o1.push("Err", 404);
  o1.display();

  return 0;
}
