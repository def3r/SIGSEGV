#include <iostream>
#include <memory>  // smart pointers? (std::unique_ptr @least)
#include <string>
#include <utility>  // std::move

// Move Sematics \\

/*
 * lvalues: (usually left values)
 *    have names and have some address assoc with em
 *    eg: int x = 10;     x -> lvalue
 *
 * rvalues: (usually temp values)
 *    have no address assoc with em
 *    eg: x = 1 + 2;      1 + 2 (rvalue) -> returns a temp val
 *        int& ref = 5;   not possible
 *        int&& rval = 5; works as && is used for rvalue refs
 *
 * std::move -> converts lvalue to a temporary rvalue (to move)
 *
 */
class Arr_Raw {
 public:
  // int&& -> only rvalue refs allowed
  //          ie no lvalue refs can be used
  //          to construct Arr
  Arr_Raw(int&& size, int&& val) : size(std::move(size)) {
    arr = new int[size];
    for (int i = 0; i < size; i++)
      arr[i] = val;
  }

  // this looks so wierd, int*&&, int*& ???
  Arr_Raw(int&& size, int*&& arr)
      : size(std::move(size)), arr(std::move(arr)) {}

  void display() {
    if (arr != nullptr)
      Arr_Raw::display(size, arr);
  }

  inline static void display(int size, int arr[]) {
    if (arr == nullptr)
      return;
    for (int i = 0; i < size; i++)
      std::cout << arr[i] << " ";
    std::cout << "\n";
  }

 private:
  int* arr = nullptr;
  int size;
};

class Arr {
 public:
  Arr(int&& size, std::unique_ptr<int[]>&& arr)
      : size(std::move(size)), ptr(std::move(arr)) {}

  void display() { Arr::display(size, ptr); }

  inline static void display(int size, std::unique_ptr<int[]>& arr) {
    if (arr == nullptr)
      return;
    for (int i = 0; i < size; i++)
      std::cout << arr[i] << " ";
    std::cout << "\n";
  }

 private:
  std::unique_ptr<int[]> ptr;
  int size;
};

int main() {
  std::cout << "Using Raw pointers\n";
  Arr_Raw a(3, 4);
  Arr_Raw b(3, std::move((int[]){0, 1, 2}));
  a.display();
  b.display();
  {
    // Making a raw pointer pointing to the heap loc
    int* arr = new (int[]){9, 8, 7};
    std::cout << "Arr in main: ";
    Arr_Raw::display(3, arr);

    // When this raw pointer is std::move'd, the raw pointer
    // does not automatically points to nullptr
    //
    // it will still point to the same loc
    Arr_Raw c(3, std::move(arr));
    std::cout << "c in main: ";
    c.display();

    // displays same val as before
    std::cout << "Arr in main: ";
    Arr_Raw::display(3, arr);
  }

  std::cout << "\n\nUsing Smart Pointers\n";

  // Allocated on the heap... not exactly
  // Not the ptr itself, it just wraps the raw pointer in a
  // class that handles everything for that pointer.
  //
  // So the Object of unique_ptr is on stack, containin a raw
  // ptr that is pointing to the heap loc of the obj.
  std::unique_ptr<int[]> ptr = std::make_unique<int[]>(3);
  ptr[2] = 6, ptr[1] = 5, ptr[0] = 4;

  std::cout << "Arr in main: ";
  Arr::display(3, ptr);

  // When this Smart Pointer is std::move'd its raw pointer
  // becomes nullptr
  Arr c(3, std::move(ptr));
  std::cout << "c in main: ";
  c.display();

  std::cout << "Arr in main: ";
  Arr::display(3, ptr);

  std::cout << "\n\nstd::move'in strings\n";
  std::string s1 = "This is the content of S1";
  std::string s2;
  // if done = std::move(s1) @ declaration-> Move Constructor is called
  // instead of Move Assignment Operator (operator=(std::string&& rval))

  std::cout << "S1: " << s1 << "\n";
  std::cout << "S2: " << s2 << "\n";
  std::cout << "s2 = std::move(s1)\n";
  s2 = std::move(s1);  // Move Assignment Operator overloaded in std::string
  std::cout << "S1: " << s1 << "\n";
  std::cout << "S2: " << s2 << "\n";
}
