%{
#include <stdio.h>

int yylex();
void yyerror(const char*);
%}

%token T_ZERO

%glr-parser
%%
S: T_ZERO S T_ZERO { }
 | /* epsilon */
%%

void yyerror(const char* str) {
  printf(str);
}

int main() {
  return yyparse();
}
