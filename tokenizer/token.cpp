
/*
 * Holy Book: https://craftinginterpreters.com/contents.html
 */

// LOX Interpreter

#include <iostream>
#include <map>
#include <string>
#include <vector>

class Parcer;
class Scanner;

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

std::map<int, int> kw;

// populating the keywords

class Token {
 private:
  enum TokenType type;
  std::string lexeme;
  std::string literal;
  int line;

  std::string enumToString() {
    switch (type) {
      case (COMMA):
        return "COMMA";
      case (DOT):
        return "DOT";
      case (MINUS):
        return "MINUS";
      case (PLUS):
        return "PLUS";
      case (SEMICOLON):
        return "SEMICOLON";
      case (SLASH):
        return "SLASH";
      case (STAR):
        return "STAR";
      case (LEFT_PAREN):
        return "LEFT_PAREN";
      case (RIGHT_PAREN):
        return "RIGHT_PAREN";
      case (LEFT_BRACE):
        return "LEFT_BRACE";
      case (RIGHT_BRACE):
        return "RIGHT_BRACE";
      case (BANG):
        return "BANG";
      case (BANG_EQUAL):
        return "BANG_EQUAL";
      case (EQUAL):
        return "EQUAL";
      case (EQUAL_EQUAL):
        return "EQUAL_EQUAL";
      case (GREATER):
        return "GREATER";
      case (GREATER_EQUAL):
        return "GREATER_EQUAL";
      case (LESS):
        return "LESS";
      case (LESS_EQUAL):
        return "LESS_EQUAL";
      case (IDENTIFIER):
        return "IDENTIFIER";
      case (STRING):
        return "STRING";
      case (NUMBER):
        return "NUMBER";
      case (AND):
        return "AND";
      case (CLASS):
        return "CLASS";
      case (ELSE):
        return "ELSE";
      case (FALSE):
        return "FALSE";
      case (FUN):
        return "FUN";
      case (FOR):
        return "FOR";
      case (IF):
        return "IF";
      case (NIL):
        return "NIL";
      case (OR):
        return "OR";
      case (PRINT):
        return "PRINT";
      case (RETURN):
        return "RETURN";
      case (SUPER):
        return "SUPER";
      case (THIS):
        return "THIS";
      case (TRUE):
        return "TRUE";
      case (VAR):
        return "VAR";
      case (WHILE):
        return "WHILE";
      case (eof):
        return "eof";
      default:
        return "";
    }
  }

 public:
  Token(TokenType type, std::string lexeme, std::string literal, int line)
      : type(type), lexeme(lexeme), literal(literal), line(line) {}

  std::string toString() {
    return enumToString() + "\t\t\t" + lexeme + "\t\t\t" + literal;
  }
};

class Parcer {
 protected:
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

  void run(std::string src);
};

class Scanner : protected Parcer {
 private:
  std::string src;
  int start;
  int current;
  int line;

  std::vector<Token *> tokens;

  static std::map<std::string, TokenType> keywords;

  char advance() { return this->src.at(current++); }

  char peek() { return (isAtEnd()) ? '\0' : this->src.at(current); }

  char peekNext() {
    return (this->current + 1 > this->src.length()) ? '\0'
                                                    : this->src.at(current + 1);
  }

  bool isAtEnd() { return this->current >= this->src.length(); }

  bool match(char expected) {
    if (isAtEnd()) return false;
    if (this->src.at(this->current) != expected) return false;

    this->current++;
    return true;
  }

  void string() {
    while (peek() != '"' && !isAtEnd()) {
      //!! multi line string
      if (peek() == '\n') line++;
      advance();
    }

    if (isAtEnd()) {
      error(line, "Unterminated String.");
      return;
    }

    // Closing "
    advance();

    // Trim the surrounding quotes
    std::string value =
        this->src.substr(this->start + 1, this->current - this->start - 2);
    addToken(STRING, value);
  }

  bool isDigit(char c) { return c >= '0' && c <= '9'; }
  bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
  }
  bool isAlphaNumeric(char c) { return isDigit(c) || isAlpha(c); }

  void number() {
    while (isDigit(peek())) advance();

    // looking for decimals
    if (peek() == '.' && isDigit(peekNext())) {
      // Consume .
      advance();

      while (isDigit(peek())) advance();
    }

    // addToken(NUMBER, std::stod(this->src.substr(start, current - start)));
    addToken(NUMBER, this->src.substr(start, current - start));
  }

  void identifier() {
    while (isAlphaNumeric(peek())) advance();

    std::string text =
        this->src.substr(this->start, this->current - this->start);
    TokenType type =
        (keywords.find(text) != keywords.end()) ? keywords[text] : IDENTIFIER;

    addToken(type);
  }

  void addToken(enum TokenType type) {
    addToken(type, "");
    return;
  }

  void addToken(enum TokenType type, std::string literal) {
    std::string text = this->src.substr(start, current - start);
    tokens.push_back(new Token(type, text, literal, line));
    return;
  }

 public:
  Scanner(std::string src) : src(src), line(1), start(0), current(0) {
    populateKeywords();
  }

  static void populateKeywords();

  // std::vector<Token *> scanTokens(std::string src) {
  std::vector<Token *> scanTokens() {
    while (!isAtEnd()) {
      // We are at the beginning of the next lexeme.
      this->start = this->current;
      scanToken();
    }

    this->tokens.push_back(new Token(eof, "", "", line));
    return this->tokens;
  }

  void scanToken() {
    char c = advance();

    switch (c) {
      case '(':
        addToken(LEFT_PAREN);
        break;
      case ')':
        addToken(RIGHT_PAREN);
        break;
      case '{':
        addToken(LEFT_BRACE);
        break;
      case '}':
        addToken(RIGHT_BRACE);
        break;
      case ',':
        addToken(COMMA);
        break;
      case '.':
        addToken(DOT);
        break;
      case '-':
        addToken(MINUS);
        break;
      case '+':
        addToken(PLUS);
        break;
      case ';':
        addToken(SEMICOLON);
        break;
      case '*':
        addToken(STAR);
        break;

      // two chars
      case '!':
        addToken(match('=') ? BANG_EQUAL : BANG);
        break;
      case '=':
        addToken(match('=') ? EQUAL_EQUAL : EQUAL);
        break;
      case '>':
        addToken(match('=') ? GREATER_EQUAL : GREATER);
        break;
      case '<':
        addToken(match('=') ? LESS_EQUAL : LESS);
        break;

      // i s  t h i s  a  c o m m e n t ?
      case '/':
        if (match('/')) {
          while (peek() != '\n' && !isAtEnd()) advance();
        } else {
          addToken(SLASH);
          break;
        }

      // waste
      case ' ':
      case '\r':
      case '\t':
        break;

      case '\n':
        this->line++;
        break;

      // Handling strings
      case '"':
        string();
        break;

      // default handles the number literals
      default:
        if (isDigit(c)) {
          number();
        } else if (isAlpha(c)) {
          identifier();
        } else {
          std::string msg = "Unexpected Character ";
          msg.push_back(c);
          this->error(line, msg);
        }
        break;
    }
  }
};

void Parcer::run(std::string src) {
  Scanner *scanner = new Scanner(src);
  std::vector<Token *> tokens = scanner->scanTokens();

  std::cout << "the Tokens are:\n";
  for (auto token : tokens) {
    std::cout << token->toString() << "\n";
  }
  return;
}
std::map<std::string, TokenType> Scanner::keywords;

void Scanner::populateKeywords() {
  keywords["and"] = AND;
  keywords["class"] = CLASS;
  keywords["else"] = ELSE;
  keywords["false"] = FALSE;
  keywords["for"] = FOR;
  keywords["fun"] = FUN;
  keywords["if"] = IF;
  keywords["nil"] = NIL;
  keywords["or"] = OR;
  keywords["print"] = PRINT;
  keywords["return"] = RETURN;
  keywords["super"] = SUPER;
  keywords["this"] = THIS;
  keywords["true"] = TRUE;
  keywords["var"] = VAR;
  keywords["while"] = WHILE;
}

int main() {
  std::string usrIn;
  Parcer parser;

  // REPL
  while (1) {
    std::getline(std::cin, usrIn);
    parser.run(usrIn);
    std::cout << "\n";
  }
  return 0;
}
