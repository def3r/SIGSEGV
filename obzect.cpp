#include <iostream>
#include <string>
#include <utility>
#include <vector>

template <class K, class V>
class Obzect {
 private:
  std::vector<std::pair<K, V>> obz;

 public:
  Obzect() {}
  Obzect(K &key, V &val) {}

  // friend std::ostream &operator<<(std::ostream &, Obzect<K, V> &);

  // "id": 9
  void push(const K &key, const V &val) {
    std::pair<K, V> *newPair = new std::pair<K, V>;
    newPair->first = key;
    newPair->second = val;

    this->obz.push_back(*newPair);

    return;
  }

  // "user": "horrifyinghorse",
  void push(K &key, V *val) {
    std::pair<K, V *> newPair = new std::pair<K, V *>;
  }

  void display() {
    int decorateSpc = 2;
    typename std::vector<std::pair<K, V>>::iterator it;

    std::cout << "\n";
    std::cout << "{\n";
    for (it = obz.begin(); it != obz.end(); ++it) {
      int spc = decorateSpc;
      while (spc--) {
        std::cout << " ";
      }

      std::cout << (*it).first << ": \"" << (*it).second << "\"";

      std::cout << "\n";
    }
    std::cout << "}\n";

    return;
  }
};

// template <typename K, typename V>
// std::ostream &operator<<(std::ostream &os, const Obzect<K, V> &o) {
//   std::cout << "shit";
//
//   return os;
// }

int main() {
  Obzect<std::string, std::string> o;
  Obzect<std::string, int> o2;

  o.push("yes", "13");
  o.push("user", "09");
  o.push("id", "05");

  std::cout << "O1:\n";
  o.display();

  o2.push("yes", 13);
  o2.push("user", 9);
  o2.push("id", 5);

  std::cout << "\nO2:";
  o2.display();

  return 0;
}
