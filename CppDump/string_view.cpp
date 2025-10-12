#include <iostream>
#include <string>
#include <string_view>

// What is string_view anyways?
//  It is a **non owning reference** to a string;
//  Allows string operations (not modifications)
//  without copying any string
//
// -> It does not allocate / own the string data
//      It refs the existing data!
//
// -> fast string ops
//
// Thus, the allocation overhead for string analysis
// is minimised

int main() {
  std::string s = "String View is interesting\n";
  std::string_view sv = s;

  std::cout << sv;

  sv.remove_prefix(7);
  std::cout << sv;
  std::cout << "But the init string remains unchanged: " << s;

  // Potentially Dangerous
  // the sv still points to the old s which could
  // result in Undefined Behaviour
  //
  // s = "Now This!\n";
  // std::cout << s << sv;

  sv = sv.substr(5, 2);
  std::cout << sv << "\n";

  // The string view should not outlive the pointed string!
  {
    std::string s = "Another String View\n";
    sv = s;
    std::cout << "Inside: \t" << sv;
    {
      std::string_view sv1 = s;
      sv = sv1;
      std::cout << "Nested Inside:\t" << sv;  // Valid as string is still alive
    }
    std::cout << "Inside: \t" << sv;  // Still in the same scope as the string
  }
  std::cout << "Out of Scope:\t" << sv;  // Undefined behavior

  struct Foo {
    // Constructor copy by val
    void Assign(std::string s) {
      sv = s;
      // copy of s will be destoryed as the func returns
      // sv outlives s => undefined behavior
    }
    void AssignRef(std::string& s) {
      sv = s;
      // s is a reference to a string
      // we have no idea abt the life of string
      // ub if s is reassigned/destroyed
    }
    void Print() { std::cout << sv << std::endl; }
    std::string_view sv;
  };

  s = "This is a String View in a Struct!";
  Foo f;
  f.Assign(s);
  f.Print();
  f.AssignRef(s);
  f.Print();
  std::cout << "Reassign s\n";
  s = "Hello :D";
  f.Print();
  {
    std::string s = "This is inside a temp scope F";
    f.AssignRef(s);
    std::cout << "Inside scope: ";
    f.Print();
  }
  std::cout << "Outside scope: ";
  f.Print();  // undefined behavior
}
