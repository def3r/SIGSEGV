#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"

#include <algorithm>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <iostream>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace ftxui;

enum Style {
  NONE = 0,
  H1 = 1,
  H2 = 2,
  H3 = 3,
  H4 = 4,
  H5 = 5,
  H6 = 6,
  BOLD,
  CODE,
  CODEBLOCK,
  ITALIC,
};
typedef std::vector<Style> Styles;

class MarkdownRenderer {
 public:
  MarkdownRenderer() { MarkdownRenderer::populateMap(); };

  void Parse(const std::string& line) {
    lineStyle =
        (lineStyle != Style::CODEBLOCK) ? Style::NONE : Style::CODEBLOCK;
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
          count = 1;
          ParseUtil(Style::ITALIC, "*");
        } else if (count == 2) {
          ParseUtil(Style::BOLD, "**");
        } else {
          if (!style.empty()) {
            if (style.back() == Style::ITALIC) {
              count = 1;
              ParseUtil(Style::ITALIC, "*");
            } else if (style.back() == Style::BOLD) {
              count = 2;
              ParseUtil(Style::BOLD, "**");
            }
          } else {
            count = 1;
            ParseUtil(Style::ITALIC, "*");
          }
        }
      } else if (*it == '`') {
        count = lookAhead(line, '`');
        if (count == 1) {
          ParseUtil(Style::CODE, "`");
        } else if (count < 3) {
          count = 1;
          ParseUtil(Style::CODE, "`");
        } else if (count == 3 && isFirst) {
          // ParseUtil(Style::CODEBLOCK);
          if (lineStyle == Style::CODEBLOCK)
            lineStyle = Style::NONE;
          else
            lineStyle = Style::CODEBLOCK, renderBlank = true;
          updateBegin = true;
        }
      } else if (*it == '_') {
        count = lookAhead(line, '_');
        if (count == 1) {
          ParseUtil(Style::ITALIC, "_");
        }
      } else if (*it == '-') {
        count = lookAhead(line, '-');
        if (count == 1 && followsWhiteSpace && isFirst) {
          pos = std::distance(begin, it);
          bffrline.replace(pos, 1, "•");
          isFirst = false;
          continue;
        }
      } else if (*it == '#' && isFirst) {
        count = lookAhead(bffrline, '#');
        if (followsWhiteSpace) {
          lineStyle = (Style)count;
          pos = std::distance(begin, it);
          bffrline.replace(pos, count + 1, "");  // +1 for the WhiteSpace
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
    corrections();
  }

  void Debug() {
    std::stringstream lineStyleStr;
    if (lineStyle != Style::NONE) {
      lineStyleStr << lineStyle;
    }

    for (auto it = parsed.begin(); it != parsed.end(); ++it) {
      std::cout << it->first << "\t\t:\t\t";
      Styles& styles = it->second;
      for (auto& style : styles) {
        std::cout << style << "\t";
      }
      std::cout << lineStyleStr.str() << "\n";
    }
  }

  Component getComponent() {
    auto container = Container::Horizontal({});
    if (parsed.empty()) {
      container->Add(Renderer([] { return text(""); }));
    }
    if (renderBlank) {
      renderBlank = false;
      return container;
    }
    for (auto it = parsed.begin(); it != parsed.end(); ++it) {
      auto item = text(it->first);
      Styles& styles = it->second;
      for (auto& style : styles) {
        item |= MarkdownRenderer::decorator[style];
      }
      container->Add(Renderer([=] { return item; }));
    }
    if (lineStyle != Style::NONE) {
      container |= MarkdownRenderer::decorator[lineStyle];
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
  void corrections() {
    while (!style.empty()) {
      std::string marker = style_metadata.back().first;
      int pos = style_metadata.back().second;
      for (pos; pos < parsed.size(); pos++) {  // NOLINT
        auto& data = parsed[pos];
        if (pos == style_metadata.back().second)
          data.first = marker + data.first;
        auto style_it =
            std::find(data.second.begin(), data.second.end(), style.back());
        if (style_it != data.second.end())
          data.second.erase(style_it);
      }
      style.pop_back();
      style_metadata.pop_back();
    }
  }

  int lookAhead(const std::string& line, char&& c) {
    int count = 1;
    while ((it + count) != line.end() && *(it + count) == c) {
      count++;
    }
    followsWhiteSpace = (*(it + count) == ' ');
    return count;
  }

  void ParseUtil(const Style& parsedStyle, const std::string& marker) {
    updateBegin = true;
    PushText();
    if (ToPush(parsedStyle, marker)) {
      style.push_back(parsedStyle);
      style_metadata.push_back({marker, parsed.size()});
    } else {
      style.pop_back();
      style_metadata.pop_back();
    }
  }

  void PushText() {
    std::string text = (begin >= it) ? "" : std::string(begin, it);
    if (!text.empty()) {
      parsed.push_back(std::make_pair(text, style));
    }
  }

  bool ToPush(const Style& s, const std::string& marker) {
    if (style.empty())
      return true;
    return (style.back() != s || style_metadata.back().first != marker);
  }

  std::string::const_iterator begin, it;
  bool updateBegin = false;
  bool followsWhiteSpace = false;
  bool renderBlank = false;
  std::string bffrline;

  Styles style = {};
  Style lineStyle = Style::NONE;
  std::vector<std::pair<std::string, Styles>> parsed = {};
  std::vector<std::pair<std::string, unsigned int>> style_metadata;

  inline static std::unordered_map<Style, std::string> marker;
  inline static std::unordered_map<Style, Decorator> decorator;
  inline static void populateMap() {
    MarkdownRenderer::decorator[Style::BOLD] = bold;
    MarkdownRenderer::decorator[Style::CODE] = inverted;
    MarkdownRenderer::decorator[Style::CODEBLOCK] =
        bgcolor(Color::RGB(30, 31, 30));
    MarkdownRenderer::decorator[Style::ITALIC] = dim;
    MarkdownRenderer::decorator[Style::H1] = bgcolor(Color::Red3);
    MarkdownRenderer::decorator[Style::H2] = bgcolor(Color::Green3);
    MarkdownRenderer::decorator[Style::H3] = bgcolor(Color::Blue3);
    MarkdownRenderer::decorator[Style::H4] = bgcolor(Color::Red);
    MarkdownRenderer::decorator[Style::H5] = bgcolor(Color::Green);
    MarkdownRenderer::decorator[Style::H6] = bgcolor(Color::Blue);
  }
};

int main() {
  auto screen = ScreenInteractive::FitComponent();
  MarkdownRenderer m;
  std::vector<std::string> lines;
  lines.push_back(
      "Clas**sic: `The work`** is `mysterious` & **`important`** Works?");
  lines.push_back("*italic* & _italic_ # no heading");
  lines.push_back("This ``should be ``invisible");
  lines.push_back("is this ***bold & italic?***");
  lines.push_back("this sentence **does not****make any sense");
  lines.push_back("crazy *init? _ibet `it is");
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
  lines.push_back("and * neither this ");
  lines.push_back("# Heading 1 **on fuego**");
  lines.push_back("## Heading 2 `this is inverted lol`");
  lines.push_back("### Heading 3");
  lines.push_back("#### Heading 4");
  lines.push_back("##### Heading 5");
  lines.push_back("###### Heading 6");
  lines.push_back("```this is nothgin```");  // Not supported ?

  auto container = Container::Vertical({});
  for (auto& line : lines) {
    std::cout << line << "\n";
  }
  for (auto& line : lines) {
    m.Parse(line);
    container->Add(std::move(m.getComponent()));
    m.Debug();
  }

  std::cout << "\nParsed:\n";
  screen.Loop(container);
}
