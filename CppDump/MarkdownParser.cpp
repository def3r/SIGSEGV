#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"

#include <ftxui/dom/elements.hpp>
#include <iostream>
#include <iterator>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace ftxui;

class MarkdownRenderer {
 public:
  MarkdownRenderer() { MarkdownRenderer::populateMap(); };

  void Parse(const std::string& line) {
    parsed = {};
    bffrline = line;
    begin = bffrline.begin();
    int count = 1;
    unsigned int pos = 0;
    bool isFirst = true;

    for (it = bffrline.begin(); it != bffrline.end(); ++it) {
      if (*it == '*') {
        count = lookAhead(line, '*');
        if (count == 1) {
          if (isFirst && followsWhiteSpace) {
            pos = std::distance(begin, it);
            bffrline.replace(pos, 1, "•");
            isFirst = false;
            continue;
          }
          ParseUtil("dim");
        } else if (count == 2) {
          ParseUtil("bold");
        }
      } else if (*it == '`') {
        count = lookAhead(line, '`');
        if (count == 1) {
          ParseUtil("inverted");
        } else if (count < 3) {
          count = 1;
          ParseUtil("inverted");
        } else if (count == 3) {
          ParseUtil("inverted");
        }
      } else if (*it == '_') {
        count = lookAhead(line, '_');
        if (count == 1) {
          ParseUtil("dim");
        }
      } else if (*it == '-') {
        count = lookAhead(line, '-');
        if (count == 1 && followsWhiteSpace && isFirst) {
          pos = std::distance(begin, it);
          bffrline.replace(pos, 1, "•");
          isFirst = false;
          continue;
        }
      }

      if (isFirst && (*it != ' ' && *it != '\t')) {
        isFirst = false;
      }

      if (updateBegin) {
        begin = it + count;
        it = it + --count;
        updateBegin = false;
      }
    }
    PushText();
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
    if (parsed.empty()) {
      container->Add(Renderer([] { return text(""); }));
    }
    return container;
  }

  void clear() {
    std::string s = "";
    begin = s.end(), it = s.end();
    updateBegin = false;

    style = {}, parsed = {};
  }

 private:
  int lookAhead(const std::string& line, char&& c) {
    int count = 1;
    while ((it + count) != line.end() && *(it + count) == c) {
      count++;
    }
    followsWhiteSpace = (*(it + count) == ' ');
    return count;
  }

  void ParseUtil(const std::string& parsedStyle) {
    updateBegin = true;
    PushText();
    if (ToPush(parsedStyle)) {
      style.push_back(parsedStyle);
    } else {
      style.pop_back();
    }
  }

  void PushText() {
    std::string text = (begin >= it) ? "" : std::string(begin, it);
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
  bool followsWhiteSpace = false;
  std::string bffrline;

  std::vector<std::string> style = {};
  std::vector<std::pair<std::string, std::vector<std::string>>> parsed = {};

  inline static std::unordered_map<std::string, Decorator> decorator;
  inline static void populateMap() {
    MarkdownRenderer::decorator["bold"] = bold;
    MarkdownRenderer::decorator["inverted"] = inverted;
    MarkdownRenderer::decorator["dim"] = dim;
  }
};

int main() {
  auto screen = ScreenInteractive::FitComponent();
  MarkdownRenderer m;
  std::vector<std::string> lines;
  lines.push_back(
      "Clas**sic: `The work`** is `mysterious` & **`important`** Works?");
  lines.push_back("*italic* & _italic_");
  lines.push_back("This ``should be ``invisible");
  lines.push_back("```");
  lines.push_back("#include <unistd.h>");
  lines.push_back("");
  lines.push_back("int main() {");
  lines.push_back("   write(1, \"nice!\", 5);");
  lines.push_back("}");
  lines.push_back("```");
  lines.push_back("* this is a point!");
  lines.push_back("- this is also a point!");
  lines.push_back("but - this is not a point!");
  lines.push_back("and * neither this");

  auto container = Container::Vertical({});
  for (auto& line : lines) {
    std::cout << line << "\n";
    // container->Add(std::move(m.getComponent()));
  }
  for (auto& line : lines) {
    m.Parse(line);
    // std::cout << line << "\n";
    container->Add(std::move(m.getComponent()));
    m.PrintParsed();
  }

  std::cout << "Parsed:\n";
  screen.Loop(container);
}
