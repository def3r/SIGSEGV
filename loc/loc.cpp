// TODO:
// 1. Multithread
// 2. Specific Format for defining extensions and comments

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

typedef struct Directory {
  std::vector<std::string> files = {};
  std::string cwd = "";
} Directory;
typedef std::vector<Directory> Directories;

class Crawler {
 public:
  Crawler(const std::string& path) { this->cwd = path; }

  void traverse() {
    Directory dir = {.cwd = this->cwd};
    std::vector<std::string> childDirs = {};
    for (const auto& entry : fs::directory_iterator(this->cwd)) {
      if (!PushDirectory(entry, childDirs) && ToPushFile(entry)) {
        dir.files.push_back(entry.path());
      }
    }
    dirs.push_back(dir);
    while (!childDirs.empty()) {
      this->cwd = childDirs.back();
      traverse();
      childDirs.pop_back();
    }
  }

  void display() {
    for (const auto& dir : dirs) {
      std::cout << dir.cwd << std::endl;
      for (const auto& dirFiles : dir.files) {
        std::cout << dirFiles << std::endl;
      }
      std::cout << std::endl;
    }
  }

  void beginCount() {
    for (const auto& dir : dirs) {
      for (const auto& file : dir.files) {
        HandleFile(file);
      }
    }
  }

  void LOC() {
    std::cout << "loc: " << loc << "\tNoUseLines: " << noUseLines << "\n";
  }

  void alsoIgnore(const std::string& dir) { ignore.push_back(dir); }
  void trackExtension(const std::string& ext) { extensions.push_back(ext); }
  void setComments(const std::string& singleLine,
                   const std::string& multiLineBegin,
                   const std::string& multiLineEnd) {
    comments = singleLine;
    multiLineComments = {multiLineBegin, multiLineEnd};
  }

 private:
  Directories dirs = {};
  std::string cwd = ".";
  unsigned long long loc = 0;
  unsigned long long noUseLines = 0;
  std::string line;

  std::vector<std::string> ignore = {".git", "node_modules"};
  std::vector<std::string> extensions = {};
  std::string comments = "";
  std::pair<std::string, std::string> multiLineComments = {"", ""};

  std::string::iterator GetFirstChar() {
    std::string::iterator it = line.begin();
    for (; it != line.end(); ++it) {
      if (*it == ' ' || *it == '\t') {
        continue;
      }
      break;
    }
    return it;
  }

  bool PushDirectory(const fs::directory_entry& entry,
                     std::vector<std::string>& childDirs) {
    if (!entry.is_directory()) {
      return false;
    }
    if (std::find(ignore.begin(), ignore.end(), entry.path().filename()) ==
        ignore.end()) {
      childDirs.push_back(entry.path());
    }
    return true;
  }

  bool ToPushFile(const fs::directory_entry& entry) {
    if (extensions.empty()) {
      return true;
    }
    return (entry.path().has_extension() &&
            std::find(extensions.begin(), extensions.end(),
                      entry.path().extension()) != extensions.end());
  }

  void HandleFile(const std::string& file) {
    std::fstream f(file);
    bool multiLineComment = false;

    if (!f) {
      std::cout << "Cannot open file: " << file << "\n";
      exit(1);
    }

    while (std::getline(f, line)) {
      std::string::iterator it = GetFirstChar();

      // Multi Line comments end
      if (multiLineComment) {
        noUseLines++;
        std::string_view sv(&*it);
        if (sv.find(multiLineComments.second) != std::string_view::npos) {
          multiLineComment = false;
        }
        continue;
      }

      // BlankLine
      if (*it == '\0') {
        noUseLines++;
        continue;
      }

      // Single line comments
      if (std::distance(it, line.end()) >= comments.length()) {
        std::string_view sv(&*it, comments.length());
        if (sv == comments) {
          noUseLines++;
          continue;
        }
      }

      // Multi Line comments begin
      if (std::distance(it, line.end()) >= multiLineComments.first.length()) {
        std::string_view sv(&*it, multiLineComments.first.length());
        if (sv == multiLineComments.first) {
          noUseLines++;
          sv = {&*(it + multiLineComments.first.length())};
          if (sv.find(multiLineComments.second) == std::string_view::npos) {
            multiLineComment = true;
          }
          continue;
        }
      }

      loc++;
    }
    f.close();
  }
};

int main() {
  std::string path = "../..";

  Crawler crawler(path);
  crawler.trackExtension(".c");
  crawler.setComments("//", "/*", "*/");
  // crawler.setComments("#", "[NO]", "[NO]");

  crawler.traverse();
  crawler.display();

  crawler.beginCount();
  crawler.LOC();
}
