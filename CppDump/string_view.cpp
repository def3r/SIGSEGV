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
}
