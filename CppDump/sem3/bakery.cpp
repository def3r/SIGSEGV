#include <iostream>

using namespace std;

class Item {
 protected:
  int price;

 public:
  int getBasePrice() { return this->price; }
};

class Product : public Item {};
class Flavor : public Item {};

class Cake : public Product {
 public:
  Cake(Flavor& f) { this->price = 500; }
  Cake() { this->price = 500; }
};

class Cookies : public Product {
 public:
  Cookies() { this->price = 200; }
};

class Bread : public Product {
 public:
  Bread() { this->price = 100; }
};

class Doughnut : public Product {
 public:
  Doughnut() { this->price = 100; }
};

class Choco : public Flavor {
 public:
  Choco() { this->price = 50; }
};

class Strawberry : public Flavor {
 public:
  Strawberry() { this->price = 30; }
};

class Vanilla : public Flavor {
 public:
  Vanilla() { this->price = 40; }
};

class Orange : public Flavor {
 public:
  Orange() { this->price = 60; }
};

class Invoice {
 protected:
  int totalCost;

 public:
  Invoice() { this->totalCost = 0; }

  void addProduct(Product p) { this->totalCost += p.getBasePrice(); }

  void addFlavor(Flavor f) { this->totalCost += f.getBasePrice(); }

  int getCost() { return this->totalCost; }
};

bool bind(Invoice& i, int& pChoice, int& fChoice) {
  switch (pChoice) {
    case 1:
      i.addProduct(Cake());
      break;
    case 2:
      i.addProduct(Cookies());
      break;
    case 3:
      i.addProduct(Bread());
      break;
    case 4:
      i.addProduct(Doughnut());
      break;
  }

  switch (fChoice) {
    case 1:
      i.addFlavor(Choco());
      break;
    case 2:
      i.addFlavor(Strawberry());
      break;
    case 3:
      i.addFlavor(Vanilla());
      break;
    case 4:
      i.addFlavor(Orange());
      break;
  }

  return 1;
}

int main() {
  Invoice i;
  int p = -1;
  int c = -1;

  while (p < 0 || p > 5) {
    cout << "\n\nPick a Product:";
    cout << "\n1. Cake " << Cake().getBasePrice();
    cout << "\n2. Cookies " << Cookies().getBasePrice();
    cout << "\n3. Bread " << Bread().getBasePrice();
    cout << "\n4. Doughnut " << Doughnut().getBasePrice();
    cout << "\n";
    cin >> p;
  }
  while (c < 0 || c > 5) {
    cout << "\n\nPick a Flavor:\n";
    cout << "\n1. Choco " << Choco().getBasePrice();
    cout << "\n2. Strawberry " << Strawberry().getBasePrice();
    cout << "\n3. Vanilla " << Vanilla().getBasePrice();
    cout << "\n4. Orange " << Orange().getBasePrice();
    cout << "\n";
    cin >> c;
  }

  bool r = bind(i, p, c);

  std::cout << "Total Cost: " << i.getCost();
}
