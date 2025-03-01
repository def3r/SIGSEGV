#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"

#include <ftxui/dom/elements.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// g++ MarkdownParser.cpp -L. -lftxui-component -lftxui-dom -lftxui-screen

using namespace ftxui;
class MarkdownRenderer;

enum Style { BOLD, CODE, CODEBLOCK, ITALIC };
typedef std::vector<Style> Styles;

class Grammar {
 public:
  typedef struct Rules {
    std::vector<std::pair<unsigned int, Styles>> rules = {};

    Rules() {}

    template <typename... Args>
    Rules& add(unsigned int count, Args... args) {
      static_assert((std::is_same_v<Args, Style> && ...),
                    "Only Style types are allowed!");
      if (count == 0)
        return *this;
      rules.push_back({count, (Styles){args...}});
      return *this;
    }

  } Rules;

  Grammar() {}

  Rules& operator[](const char& c) {
    if (rules.find(c) == rules.end()) {
      rules[c] = {};
    }
    return rules[c];
  }

  friend class MarkdownRenderer;

 private:
  std::unordered_map<char, Rules> rules;
};

class MarkdownRenderer {
 public:
  MarkdownRenderer() {
    MarkdownRenderer::populateMap();
    MarkdownRenderer::populateGrammar();
  };

  void Parse(const std::string& line) {
    parsed = {};
    begin = line.begin();
    int count = 1;
    auto& grammar = MarkdownRenderer::grammar.rules;
    for (it = line.begin(); it != line.end(); ++it) {
      if (grammar.find(*it) == grammar.end()) {
        continue;
      }
      updateBegin = true;
      count = lookAhead(line, *it);
      auto rules = grammar[*it].rules;
      for (auto rule = rules.begin(); rule != rules.end(); ++rule) {
        if (count < rule->first && rule != rules.begin()) {
          count = (--rule)->first;
          ParseUtil((rule)->second);
          break;
        }
        if (count == rule->first) {
          ParseUtil(rule->second);
          break;
        }
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
      Styles& styles = it->second;
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
      Styles& styles = it->second;
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
  int lookAhead(const std::string& line, const char& c) {
    int count = 1;
    while ((it + count) != line.end() && *(it + count) == c) {
      count++;
    }
    return count;
  }

  void ParseUtil(const Styles& parsedStyles) {
    PushText();
    if (ToPush(parsedStyles)) {
      for (auto& parsedStyle : parsedStyles)
        style.push_back(parsedStyle);
    } else {
      for (int i = parsedStyles.size(); i > 0; i--)
        style.pop_back();
    }
  }

  void PushText() {
    std::string text = (begin >= it) ? "" : std::string(begin, it);
    if (!text.empty()) {
      parsed.push_back(std::make_pair(text, style));
    }
  }

  bool ToPush(const Styles& s) {
    if (style.empty())
      return true;
    return (style.back() != s.back());
  }

  std::string::const_iterator begin, it;
  bool updateBegin = false;

  Styles style = {};
  std::vector<std::pair<std::string, Styles>> parsed = {};

  inline static std::unordered_map<Style, Decorator> decorator;
  inline static void populateMap() {
    MarkdownRenderer::decorator[Style::BOLD] = bold;
    MarkdownRenderer::decorator[Style::CODE] = inverted;
    MarkdownRenderer::decorator[Style::ITALIC] = dim;
    MarkdownRenderer::decorator[Style::CODEBLOCK] = inverted;
  }
  inline static Grammar grammar;
  inline static void populateGrammar() {
    // clang-format off
    MarkdownRenderer::grammar
      ['*']
      .add(1, Style::ITALIC)
      .add(2, Style::BOLD);

    MarkdownRenderer::grammar
      ['_']
      .add(1, Style::ITALIC)
      .add(2, Style::BOLD);

    MarkdownRenderer::grammar
      ['`']
      .add(1, Style::CODE)
      .add(3, Style::CODE);
    // clang-format on
  }
};

int main() {
  auto screen = ScreenInteractive::FitComponent();
  MarkdownRenderer m;
  std::vector<std::string> lines;
  lines.push_back(
      "Clas**sic: `The work`** is `mysterious` & **`important`** Works?");
  lines.push_back("*italic* & _italic_");
  lines.push_back("This ``shouldn't ``print");
  lines.push_back("```");
  lines.push_back("#include <unistd.h>");
  lines.push_back("");
  lines.push_back("int main() {");
  lines.push_back("   write(1, \"nice!\", 5);");
  lines.push_back("}");
  lines.push_back("```");

  auto container = Container::Vertical({});
  for (auto& line : lines) {
    std::cout << line << "\n";
  }
  for (auto& line : lines) {
    m.Parse(line);
    container->Add(std::move(m.getComponent()));
    m.PrintParsed();
  }
  std::cout << "Parsed:\n";
  screen.Loop(container);
}
