#include <stdbool.h>
#include <string.h>

bool aisalphanum(const char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9');
}

bool aiswspace(const char c) {
  return (c == ' ' || c == '\n' || c == '\t' || c == '\0');
}
