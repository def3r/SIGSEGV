%{
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int yylex();
void yyerror(const char*);

bool accept = false;
%}

// S -> aSb / ab

%token TOKEN_A
%token TOKEN_B
%token TOKEN_OTHER

%%
expr : TOKEN_A expr TOKEN_B { }
     | TOKEN_A TOKEN_B      { accept = true; }
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

