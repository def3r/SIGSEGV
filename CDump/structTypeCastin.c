#include <stdio.h>
#include <stdlib.h>

// Another xv6 W
// Allocating the object on the heap
// and then typecasting the reference
// to it while passing it around to
// different functions
//
// { How bad I've tried to do something
//     like this with classes in Cpp    }

#define Println(...)                                                           \
  printf(__VA_ARGS__);                                                         \
  printf("\n")

#define EXEC 1
#define REDIR 2

#define MAXARGS 10

struct cmd {
  int type;
};

struct execcmd {
  int type;
  char *execStr;
};

struct redircmd {
  int type;
  char *execStr;
  char *redirFile;
};

void parseType(struct cmd *c) {
  switch (c->type) {
  case EXEC:
    Println("got type: EXEC");
    struct execcmd *ec = (struct execcmd *)c;
    Println("execStr=%s", ec->execStr);
    break;
  case REDIR:
    Println("got type: REDIR");
    struct redircmd *rc = (struct redircmd *)c;
    Println("execStr=%s", rc->execStr);
    Println("redirFile=%s", rc->redirFile);
    break;
  }
  Println("");
}

int main() {
  struct cmd *c;

  Println("Creating an execcmd struct");
  struct execcmd *ec = (struct execcmd *)malloc(sizeof(struct execcmd));
  ec->type = EXEC;
  ec->execStr = "ls -l";
  Println("Passing execcmd typecasted to struct cmd");
  parseType((struct cmd *)ec);

  Println("Creating an redir struct");
  struct redircmd *rc = (struct redircmd *)malloc(sizeof(struct redircmd));
  rc->type = REDIR;
  rc->execStr = "ls -l";
  rc->redirFile = "input.txt";
  Println("Passing redir typecasted to struct cmd");
  parseType((struct cmd *)rc);

  free(ec);
  free(rc);
  return 0;
}
