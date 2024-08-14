#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <variant>

using namespace std;
using RetTypes = variant<int>;

unordered_map<string, function<RetTypes(int n)>> myMap;
unordered_map<string, function<RetTypes()>> myMapNoArg;

int misery(int n) {
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
}

int main() {
  populateMyMap();

  cout << "Maps" << endl;

  if (myMap.find("misery") != myMap.end()) {
    myMap["misery"](9);
  }

  if (myMapNoArg.find("mnargs") != myMapNoArg.end()) {
    myMapNoArg["mnargs"]();
  }

  return 0;
}
