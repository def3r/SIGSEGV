#include <unistd.h>
#include <string>

int main() {
  // Raw string literals!
  // defined by: R"()"

  std::string raw = R"(
    There is no way!
      This is real?
  )";
  write(1, raw.c_str(), raw.length());
}
