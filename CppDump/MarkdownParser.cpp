#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"

#include <ftxui/dom/elements.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace ftxui;

class MarkdownRenderer {
 public:
  MarkdownRenderer() { MarkdownRenderer::populateMap(); };

  void Parse(const std::string& line) {
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
      std::vector<std::string>& styles = it->second;
      for (auto& style : styles) {
        std::cout << style << "\t";
      }
      std::cout << "\n";
    }
  }

  Component getComponent() {
    auto container = Container::Horizontal({});
    for (auto it = parsed.begin(); it != parsed.end(); ++it) {
      auto item = text(it->first);
      std::vector<std::string>& styles = it->second;
      for (auto& style : styles) {
        item |= MarkdownRenderer::decorator[style];
      }
      container->Add(Renderer([=] { return item; }));
    }
    return container;
  }

 private:
  void ParseUtil(const int& offset, const std::string& parsedStyle) {
    updateBegin = true;
    PushText(offset);
    if (ToPush(parsedStyle)) {
      style.push_back(parsedStyle);
    } else {
      style.pop_back();
    }
  }

  void PushText(const int& offset) {
    std::string text =
        (begin >= it - offset) ? "" : std::string(begin, it - offset);
    if (!text.empty()) {
      parsed.push_back(std::make_pair(text, style));
    }
  }

  bool ToPush(const std::string& s) {
    if (style.empty())
      return true;
    return (style.back() != s);
  }

  std::string::const_iterator begin, it;
  char prev = '\0';
  bool updateBegin = false;

  std::vector<std::string> style = {};
  std::vector<std::pair<std::string, std::vector<std::string>>> parsed = {};

  inline static std::unordered_map<std::string, Decorator> decorator;
  inline static void populateMap() {
    MarkdownRenderer::decorator["bold"] = bold;
    MarkdownRenderer::decorator["inverted"] = inverted;
  }
};

int main() {
  auto screen = ScreenInteractive::FitComponent();
  std::string line =
      "Clas**sic: `The work`** is `mysterious` & **`important`** Works?";
  std::cout << line << "\n";
  MarkdownRenderer m;
  m.Parse(line);
  screen.Loop(m.getComponent());
}
