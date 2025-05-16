#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <variant>

using mapProcess = int (*)(int &);
using namespace std;
using RetTypes = variant<function<int()>, function<int(int)>>;

unordered_map<string, RetTypes> myMap;
unordered_map<string, function<int()>> myMapNoArg;
unordered_map<string, mapProcess> bsMap;

int misery(int &n) {
  cout << "Messiness of misery." << n << endl;

  return 0;
}

int miseryNoArg() {
  cout << "NO ARGS>MISERY." << endl;

  return 0;
}

void populateMyMap() {
  myMap["misery"] = [](int n) { return misery(n); };

  myMapNoArg["mnargs"] = []() { return miseryNoArg(); };

  bsMap["misery"] = misery;
}

int main() {
  int myInt = 9;
  populateMyMap();

  cout << "Maps" << endl;

  if (bsMap.find("misery") != bsMap.end()) {
    bsMap["misery"](myInt);
  }

  if (myMapNoArg.find("mnargs") != myMapNoArg.end()) {
    myMapNoArg["mnargs"]();
  }

  return 0;
}
