#include <iostream>
using namespace std;

int main() {
  int a = 1234567890;
  int b = 987654321;
  int c = a * b;

  cout << a << "\n";
  cout << b << "\n";
  cout << "Multiplying above two (int) data types ";
  cout << "return an (int) no matter the overflow\n";
  cout << a << " * " << b << " = " << c << " (int)\n";
  cout << "\nT y p e C a s t i n g\n";
  cout << a << " * " << b << " = " << (long long)a * b << " (long long) result\n";
  cout << "\nlong | long int are @least 32 bits";
  cout << "\nlong long | long long int are @least 64 bits";

  return 0;
}
