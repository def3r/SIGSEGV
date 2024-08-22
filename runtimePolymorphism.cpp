#include <iostream>

using namespace std;

class Box {
protected:
  int l, b, h;

public:
  Box (int l, int b, int h) {
    this->l = l;
    this->b = b;
    this->h = h;
  }

  int volume() {
    return l * b * h;
  }

  virtual int sa() {
    return 2 * (l * b + b * h + h * l);
  }
};

class OpenBox : public Box {
public:
  OpenBox(int l, int b, int h) : Box(l, b, h) { }

  int sa() {
    return 2 * (b * h + h * l) + l * b;
  }
};

int main() {
  Box* myBox[3] = { new Box(1, 2, 3), new OpenBox(1, 2, 3), new Box(2, 3, 4)};

  for(int i =  0; i <3; i++) {
    cout << myBox[i]->sa() << endl;
  }

  return 0;
}
