#include <termio.h>
#include <cstdio>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

class Input {
 public:
  Input() {}

  std::string in() {
    bool exit = false;
    while (!exit) {
      char c = std::cin.get();
      ss << c;
      std::cout << ss.str();
    }

    return ss.str();
  }

 private:
  std::stringstream ss;
};

int main() {
  Input inp;
  inp.in();
}
