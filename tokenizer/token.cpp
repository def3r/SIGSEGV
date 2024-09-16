/*
 * Holy Book: https://craftinginterpreters.com/contents.html
 */

#include <iostream>
#include <random>
#include <string>
#include <vector>

enum TokenType {
  // Single-character tokens.
  COMMA,
  DOT,
  MINUS,
  PLUS,
  SEMICOLON,
  SLASH,
  STAR,
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,

  // One or two character tokens.
  BANG,
  BANG_EQUAL,
  EQUAL,
  EQUAL_EQUAL,
  GREATER,
  GREATER_EQUAL,
  LESS,
  LESS_EQUAL,

  // Literals.
  IDENTIFIER,
  STRING,
  NUMBER,

  // Keywords.
  AND,
  CLASS,
  ELSE,
  FALSE,
  FUN,
  FOR,
  IF,
  NIL,
  OR,
  PRINT,
  RETURN,
  SUPER,
  THIS,
  TRUE,
  VAR,
  WHILE,

  eof,
};

class Token {
 private:
  enum TokenType type;
  std::string lexeme;
  std::string literal;
  int line;

 public:
  Token(TokenType type, std::string lexeme, std::string literal, int line)
      : type(type), lexeme(lexeme), literal(literal), line(line) {}

  std::string toString() { return type + " " + lexeme + " " + literal; }
};

class Parcer {
 private:
  bool hadError;

  void error(int line, std::string msg) {
    this->hadError = true;
    this->report(line, "", msg);
    return;
  }

  void report(int line, std::string where, std::string msg) {
    std::cout << "Error@ " << line << " " << where << " -> " << msg << "\n";
    return;
  }

 public:
  Parcer() : hadError(false) {}

  void run(std::string src) {
    std::cout << "the Tokens are:\n";
    return;
  }
};

class Scanner {
 private:
  std::string src;
  int start = 0;
  int current = 0;
  int line;

  std::vector<Token*> tokens;

  bool isAtEnd() { return this->current >= this->src.length(); }

  void addToken(enum TokenType type) {
    addToken(type, "");
    return;
  }

  void addToken(enum TokenType type, std::string literal) {
    std::string text = this->src.substr(start, current);
    tokens.push_back(new Token(type, text, literal, line));
    return;
  }

 public:
  Scanner(std::string src) : src(src), line(1) {}

  std::vector<Token*> scan(std::string src) {
    this->src = src;

    for (char c : src) {
    }

    return this->tokens;
  }
};

int main() {
  std::string usrIn;
  Parcer parser;

  // REPL
  while (1) {
    std::getline(std::cin, usrIn);
    parser.run(usrIn);
  }
  return 0;
}
