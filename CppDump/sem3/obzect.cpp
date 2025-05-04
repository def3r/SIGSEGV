#include <algorithm>
#include <any>
#include <iostream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

// example
#include <vector>

class ObzectItem {
 public:
  ObzectItem *pItem;
  ObzectItem *nItem;

  virtual ~ObzectItem() {}

  virtual void setPPtr(ObzectItem *pItem) = 0;
  virtual void setNPtr(ObzectItem *nItem) = 0;
  virtual void setPtr(ObzectItem *pItem, ObzectItem *nItem) = 0;

  virtual std::string getKey() = 0;
  virtual std::string *getKeyPtr() = 0;
  virtual bool setKey(const std::any &) = 0;

  virtual std::any getValue() = 0;
  virtual std::any getValuePtr() = 0;
  virtual bool setValue(const std::any &) = 0;
};

template <class V>
class ObzItemExplicit : public ObzectItem {
 protected:
  std::pair<std::string, V *> kvPair;

 public:
  ObzItemExplicit(const std::string &key, const V &val) {
    this->kvPair.first = key;
    this->kvPair.second = new V(val);

    this->pItem = nullptr;
    this->nItem = nullptr;
  }

  void setPPtr(ObzectItem *pItem) { this->pItem = pItem; }
  void setNPtr(ObzectItem *nItem) { this->nItem = nItem; }
  void setPtr(ObzectItem *pItem, ObzectItem *nItem) {
    this->pItem = pItem;
    this->nItem = nItem;
  }

  std::string getKey() { return this->kvPair.first; }

  std::string *getKeyPtr() { return &this->kvPair.first; }

  bool setKey(const std::any &key) {
    if (key.type() != typeid(std::string)) {
      std::cerr << "\nNew key type(" << key.type().name()
                << ") differs from init key(" << ") type("
                << typeid(this->kvPair.first).name() << ")";
      return false;
    }

    this->kvPair.first = std::any_cast<std::string>(key);

    return true;
  }

  std::any getValue() { return *(this->kvPair.second); }

  std::any getValuePtr() { return this->kvPair.second; }

  bool setValue(const std::any &val) {
    if (val.type() != typeid(V)) {
      std::cerr << "\nNew value type(" << val.type().name()
                << ") differs from init val(" << ") type("
                << typeid(*(this->kvPair.second)).name() << ")";
      return false;
    }

    this->kvPair.second = new V(std::any_cast<V>(val));

    return true;
  }
};

class Obzect {
 protected:
  ObzectItem *head;
  ObzectItem *tail;
  ObzectItem *cursor;

  std::vector<std::string> keys;
  std::unordered_map<std::string, ObzectItem *> keyObzMap;

 public:
  Obzect() : head(nullptr), tail(nullptr), cursor(nullptr) {}

  ~Obzect() {
    ObzectItem *it = this->head;
    while (it) {
      ObzectItem *del = it;
      delete del;
      it = it->nItem;
    }
  }

  template <typename V>
  void push(const std::string &key, const V &val) {
    if (this->find(key)) {
      cursor->setValue(std::any(val));
      return;
    }

    ObzItemExplicit<V> *newItem = new ObzItemExplicit<V>(key, val);

    this->keys.push_back(key);
    this->keyObzMap[key] = newItem;
    this->cursor = newItem;

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

  bool find(const std::string &key) {
    if (std::find(keys.begin(), keys.end(), key) != keys.end()) {
      this->cursor = this->keyObzMap[key];
      return 1;
    }

    return 0;
  }

  bool find(const char *key) { return this->find(std::string(key)); }

  template <typename V>
  V key(const std::string &key) {
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

  std::vector<std::string> getKeys() {
    std::vector<std::string> keys;
    ObzectItem *it = this->head;

    while (it) {
      keys.push_back(it->getKey());
      it = it->nItem;
    }

    return keys;
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

  template <typename V>
  V operator[](const std::string &key) {
    return this->key<V>(key);
  }

  std::any operator[](const char *key) { return this->key<std::any>(key); }
};

void display(Obzect &obz) {
  int decorateSpc = 2;
  std::vector<std::string> keys = obz.getKeys();
  std::vector<std::string>::iterator it;

  std::cout << "\n{\n";
  for (it = keys.begin(); it != keys.end(); ++it) {
    int spc = decorateSpc;
    while (spc--) {
      std::cout << " ";
    }

    std::cout << *it << ": " << ",\n";
  }
  std::cout << "}\n";

  return;
}

int main() {
  Obzect o1;
  o1.push("Name", "Ayaan");
  o1.push("Username", "Horrifyinghorse");
  o1.push("ID", 28);
  display(o1);

  if (o1.find("ID")) {
    std::cout << "Find return YES!";
  } else {
    std::cout << "Find return No!";
  }

  std::cout << "\nUsing key:";
  std::cout << "\nUsername = " << o1.key<std::string>("Name");
  std::cout << "\nID = " << o1.key<int>("ID");

  std::cout << "\nOverriding name";
  o1.push("Name", "Khan");

  std::cout << "\nName: " << std::any_cast<std::string>(o1["Name"]);

  o1.clear();
  o1.push("Err", 404);

  std::cout << "New obz: o2\n";
  Obzect o2;

  std::vector<int> v = {1, 2, 3, 4, 5};

  int arr[] = {1, 2, 3, 4, 5, 9};

  o2.push("arr", arr);
  o2.push("vec", v);

  arr[2] = -1;
  v.at(2) = -1;
  o2.key<std::vector<int>>("vec")[2] = 1;

  std::vector<int> vrec = o2.key<std::vector<int>>("vec");
  int *rec = o2.key<int *>("arr");

  std::cout << "\nSo arr should be 3: " << rec[2];
  std::cout << "\nSo vec should be 3: " << vrec.at(2);

  return 0;
}
