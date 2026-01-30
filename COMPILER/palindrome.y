%{
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int yylex();
void yyerror(const char*);

bool accept = false;
%}

%token TOKEN_A
%token TOKEN_B
%token TOKEN_OTHER

// Palindrome cannot be parsed using LALR(1)
// Because bison is a deterministic parser, while palindrome is non
// deterministic CFG. Thus at compilation, this gives a warning for
// shift-reduce and reduce-reduce conflict.
//
// glr-parser is an extension of LR parser, this handles non-deterministic as
// well as ambiguous G
//
// https://www.gnu.org/software/bison/manual/html_node/GLR-Parsers.html
%glr-parser
%%
expr : TOKEN_A expr TOKEN_A { }
     | TOKEN_B expr TOKEN_B { }
     | TOKEN_A              { accept = true; }
     | TOKEN_B              { accept = true; }
     | /* epsilon */        { accept = true; }
%%

int main() {
  printf("Enter Expr: ");
  yyparse();

  printf("Accept: %d", accept);
  return 0;
}

void yyerror(const char* s) {
  printf("Invaiid Expr\n");
  accept = false;
}


