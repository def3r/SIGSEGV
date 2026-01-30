%{
#include <stdio.h>
#include <stdlib.h>

int yylex();
void yyerror(const char*);
%}

%token NUMBER
%token SYMBOL

%%
expr : NUMBER SYMBOL NUMBER {
    printf("Valid Expr");
  }

expr : NUMBER SYMBOL SYMBOL {
    printf("Valid Expr Inc");
  }

%%

int main() {
  printf("Enter Expr: ");
  yyparse();
  return 0;
}

void yyerror(const char* s) {
  printf("Invaiid Expr\n");
}
