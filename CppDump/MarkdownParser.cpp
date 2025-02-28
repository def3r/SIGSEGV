#include <iostream>
#include <string>
#include <utility>
#include <vector>

class MarkdownRenderer {
public:
  MarkdownRenderer() {};

  void Parse(const std::string &line) {
    begin = line.begin();
    for (it = line.begin(); it != line.end(); ++it) {
      if (*it == '*' && prev == '*') {
        ParseUtil(1, "bold");
      } else if (*it == '`') {
        ParseUtil(0, "inverted");
      }

      if (updateBegin) {
        begin = it + 1;
        updateBegin = false;
      }

      prev = *it;
    }
    PushText(0);
  }

  void PrintParsed() {
    for (auto it = parsed.begin(); it != parsed.end(); ++it) {
      std::cout << it->first << "\t\t:\t\t";
      std::vector<std::string> &styles = it->second;
      for (auto &style : styles) {
        std::cout << style << "\t";
      }
      std::cout << "\n";
    }
  }

private:
  void ParseUtil(const int &offset, const std::string &parsedStyle) {
    updateBegin = true;
    PushText(offset);
    if (PushFirst(parsedStyle)) {
      style.push_back(parsedStyle);
    } else {
      style.pop_back();
    }
  }

  void PushText(const int &offset) {
    std::string text =
        (begin >= it - offset) ? "" : std::string(begin, it - offset);
    if (!text.empty()) {
      parsed.push_back(std::make_pair(text, style));
    }
  }

  bool PushFirst(const std::string &s) {
    if (style.empty())
      return true;
    return (style.back() != s);
  }

  std::string::const_iterator begin, it;
  char prev = '\0';
  bool updateBegin = false;

  std::vector<std::string> style = {};
  std::vector<std::pair<std::string, std::vector<std::string>>> parsed = {};
};

int main() {
  std::string line =
      "**Classic:`The work` is `mysterious` & `important`** Works?";
  std::cout << line << "\n";
  MarkdownRenderer m;
  m.Parse(line);
  m.PrintParsed();
}
