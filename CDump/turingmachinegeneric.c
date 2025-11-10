#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MakeVector(vType, v)                                                   \
  do {                                                                         \
    v = malloc(sizeof(vType));                                                 \
    if (v != NULL) {                                                           \
      memset(v, 0, sizeof(vType));                                             \
    }                                                                          \
  } while (0)

#define VectorPush(v, c)                                                       \
  if (v != NULL) {                                                             \
    if (v->len == v->capacity) {                                               \
      v->capacity = v->capacity == 0 ? 1 : v->capacity;                        \
      void *newArr = malloc(sizeof(*v->arr) * 2 * v->capacity);                \
      memcpy(newArr, v->arr, sizeof(*v->arr) * v->len);                        \
      free(v->arr);                                                            \
      v->arr = newArr;                                                         \
      v->capacity *= 2;                                                        \
    }                                                                          \
    v->arr[v->len++] = c;                                                      \
  }

extern int errno;

enum Dir { LEFT = -1, RIGHT = 1, NOMOVE = 0, SAME };

// clang-format off

typedef struct {
  char   *arr;
  size_t  len;
  size_t  capacity;
} charVector;

typedef struct {
  int    *arr;
  size_t  len;
  size_t  capacity;
} intVector;

typedef struct {
  uint16_t cur, next;
  char     head;
  char     write;
  enum Dir dir;
  bool     halt;
} transition;

typedef struct {
  transition *arr;
  size_t      len;
  size_t      capacity;
} transitionVector;
typedef struct {
  transitionVector *arr;
  size_t            len;
  size_t            capacity;
} transitionVector2;

typedef struct {
  uint16_t           numStates;
  char               blankSymbol;
  char               initState;
  charVector        *inputSymbols;
  charVector        *tapeSymbols;
  intVector         *finalStates;
  transitionVector  *transitions;
} turingMachine;

// clang-format on

turingMachine *tm;

turingMachine *makeTuringMachine() {
  turingMachine *tm = malloc(sizeof(turingMachine));
  if (tm == NULL)
    return NULL;
  memset(tm, 0, sizeof(turingMachine));
  tm->blankSymbol = 'B';
  MakeVector(transitionVector, tm->transitions);
  return tm;
}

bool isWhitespace(char c) { return c == ' ' || c == '\t' || c == '\n'; }

enum Dir stodir(const char *c) {
  if (strcmp(c, "LEFT") == 0)
    return LEFT;
  if (strcmp(c, "RIGHT") == 0)
    return RIGHT;
  if (strcmp(c, "NOMOVE") == 0)
    return NOMOVE;
  return SAME;
}

// char *ltrim(char *s, size_t *len) {
//   if (*len == 0)
//     return s;
//
//   size_t i = 0;
//   while (isWhitespace(s[i]) && s[i] != '\0')
//     i++;
//   *len -= i;
//   return (s + i);
// }
// char *rtrim(char *s, size_t *len) {
//   if (*len == 0)
//     return s;
//
//   size_t i = *len - 1;
//   while ((isWhitespace(s[i])) && i != 0)
//     i--;
//   *len = i + 1;
//   *(s + i + 1) = '\0';
//   return s;
// }
//
// char *trim(char *s, size_t *len) {
//   if (*len == 0)
//     return s;
//   rtrim(s, len);
//   return ltrim(s, len);
// }

void coalesceWhitespace(char *line, size_t *len) {
  if (*len == 0)
    return;

  char buf[*len + 1];
  size_t bufIndex = 0;
  size_t i = 0;
  while (i < *len && line[i] != '\0') {
    if (!isWhitespace(line[i])) {
      if (line[i] == '#' && (i == 0 || line[i - 1] != '\\')) {
        break;
      }
      buf[bufIndex++] = line[i];
    }
    i++;
  }

  *len = bufIndex;
  buf[bufIndex++] = '\0';
  while (bufIndex--) {
    line[bufIndex] = buf[bufIndex];
  }
}

char *parseMacInfo(FILE *f) {
  tm = makeTuringMachine();
  if (tm == NULL) {
    perror("Unable to allocate truing machine\n");
    exit(EXIT_FAILURE);
  }

  char *line = NULL;
  size_t len = 0;
  ssize_t n = 0;

  char *linestart = NULL;
  while ((n = getline(&line, &len, f)) != -1) {
    len = n;
    linestart = line;
    // line = trim(line, &len);
    coalesceWhitespace(line, &len);
    if (len == 0)
      continue;
    else if (line[0] == '[')
      break;
    else if (line[0] == '#')
      continue;

    char *c = line;
    for (; *c != '\0' && *c != '='; c++)
      ;
    if (*c != '=') {
      fprintf(stderr, "Missing '=' in toml file");
      exit(EXIT_FAILURE);
    }

    *c = '\0';
    char *var = line;
    char *val = ++c;

    if (strcmp(var, "blanksymbol") == 0) {
      tm->blankSymbol = val[0];

    } else if (strcmp(var, "numofstates") == 0) {
      tm->numStates = atoi(val);

    } else if (strcmp(var, "inputsymbols") == 0) {
      unsigned long arrLen = strlen(val);
      if (arrLen == 0 || val[0] != '[' || val[arrLen - 1] != ']') {
        fprintf(stderr, "Expected an array [...] for inputsymbols\n");
        exit(EXIT_FAILURE);
      }

      charVector *v;
      MakeVector(charVector, v);
      char *token;
      token = strtok(val, "[,]");
      while (token != NULL) {
        VectorPush(v, token[0]);
        token = strtok(NULL, "[,]");
      }

      tm->inputSymbols = v;

    } else if (strcmp(var, "tapesymbols") == 0) {
      unsigned long arrLen = strlen(val);
      if (arrLen == 0 || val[0] != '[' || val[arrLen - 1] != ']') {
        fprintf(stderr, "Expected an array [...] for tapesymbols\n");
        exit(EXIT_FAILURE);
      }

      charVector *v;
      MakeVector(charVector, v);
      char *token;
      token = strtok(val, "[,]");
      while (token != NULL) {
        VectorPush(v, token[0]);
        token = strtok(NULL, "[,]");
      }

      tm->tapeSymbols = v;

    } else if (strcmp(var, "finalstates") == 0) {
      unsigned long arrLen = strlen(val);
      if (arrLen == 0 || val[0] != '[' || val[arrLen - 1] != ']') {
        fprintf(stderr, "Expected an array [...] for finalstates\n");
        exit(EXIT_FAILURE);
      }

      intVector *v;
      MakeVector(intVector, v);
      char *token;
      token = strtok(val, "[,]");
      while (token != NULL) {
        VectorPush(v, atoi(token));
        token = strtok(NULL, "[,]");
      }

      tm->finalStates = v;

    } else if (strcmp(var, "initialstate") == 0) {
      tm->initState = atoi(val);

    } else {
      printf("%s\t\t%s\n", var, val);
    }

    free(linestart);
    line = NULL;
    len = 0;
  }

  return line;
}

char *parseTransition(FILE *f) {
  transition t = {.cur = UINT16_MAX,
                  .dir = SAME,
                  .head = '\0',
                  .next = UINT16_MAX,
                  .write = '\0',
                  .halt = false};

  char *line = NULL;
  size_t len = 0;
  ssize_t n = 0;

  char *linestart = NULL;
  while ((n = getline(&line, &len, f)) != -1) {
    len = n;
    linestart = line;
    coalesceWhitespace(line, &len);
    if (len == 0)
      continue;
    else if (line[0] == '[')
      break;
    else if (line[0] == '#')
      continue;

    char *c = line;
    for (; *c != '\0' && *c != '='; c++)
      ;
    if (*c != '=') {
      fprintf(stderr, "Missing '=' in toml file");
      exit(EXIT_FAILURE);
    }

    *c = '\0';
    char *var = line;
    char *val = ++c;

    if (strcmp(var, "cur") == 0) {
      t.cur = atoi(val);

    } else if (strcmp(var, "head") == 0) {
      t.head = atoi(val);

    } else if (strcmp(var, "next") == 0) {
      t.next = atoi(val);

    } else if (strcmp(var, "dir") == 0) {
      t.dir = stodir(val);

    } else if (strcmp(var, "write") == 0) {
      t.write = val[0];

    } else if (strcmp(var, "halt") == 0) {
      t.halt = strcmp(val, "true") == 0;

    } else {
      printf("%s|\n", line);
    }

    free(linestart);
    line = NULL;
    len = 0;
  }
  if (t.cur == UINT16_MAX) {
    fprintf(stderr, "Expected a cur state for transition\n");
    exit(EXIT_FAILURE);
  }
  VectorPush(tm->transitions, t);
  return line;
}

void parseLine(FILE *f, char *line, size_t len) {
  if (len == 0)
    return;
  if (line[0] == '#')
    return;

  if (strcmp(line, "[turingmachine]") == 0) {
    line = parseMacInfo(f);
    parseLine(f, line, strlen(line));

  } else if (strcmp(line, "[[transition]]") == 0) {
    if (tm == NULL) {
      fprintf(stderr, "%s\n", line);
      fprintf(stderr, "Transition for unknown turing machine");
      exit(EXIT_FAILURE);
    }

    line = parseTransition(f);
    parseLine(f, line, strlen(line));
  } else {
    printf("%s|\n", line);
  }
}

void parseTOML(const char *fname) {
  FILE *f = fopen(fname, "r");
  if (!f) {
    perror("Unable to open file");
    exit(EXIT_FAILURE);
  }

  char *line = NULL;
  size_t len = 0;
  ssize_t n = 0;

  char *linestart = NULL;
  while ((n = getline(&line, &len, f)) != -1) {
    len = n;
    linestart = line;
    // line = trim(line, &len);
    coalesceWhitespace(line, &len);
    parseLine(f, line, len);

    free(linestart);
    line = NULL;
    len = 0;
  }

  return;
}

int main() {
  parseTOML("tm.toml");

  printf("\n");
  printf("Loaded data:\nTuring Machine:\n");
  printf("No of States: %d\n", tm->numStates);
  printf("Blank sym: %c\n", tm->blankSymbol);
  printf("init state: %d\n", tm->initState);
  printf("final states: ");
  for (int i = 0; i < tm->finalStates->len; i++) {
    printf("%d,", tm->finalStates->arr[i]);
  }
  printf("\ninput symbols: ");
  for (int i = 0; i < tm->inputSymbols->len; i++) {
    printf("%c,", tm->inputSymbols->arr[i]);
  }
  printf("\ntape symbols: ");
  for (int i = 0; i < tm->tapeSymbols->len; i++) {
    printf("%c,", tm->tapeSymbols->arr[i]);
  }
  printf("\ntransitions: ");

  if (tm->transitions != NULL) {
    printf("total = %ld\n", tm->transitions->len);
    for (int i = 0; i < tm->transitions->len; i++) {
      printf("  Transition #%d\n", i);
      printf("    cur: %d, next: %d, head: %c, write: %c, dir: %d, halt: %d\n",
             tm->transitions->arr[i].cur, tm->transitions->arr[i].next,
             tm->transitions->arr[i].head, tm->transitions->arr[i].write,
             tm->transitions->arr[i].dir, tm->transitions->arr[i].halt);
    }
  } else {
    printf("No transitinos for the tm\n");
  }
}
