%{
#include <stdio.h>
#include <stdlib.h>

int yylex();
void yyerror(const char *s);
%}

%token NUM

// Only 'a+b' or 'a-b'
%glr-parser
%%
S : E { }

E : T '+' T { printf("%d %d + ", $1, $3); }
  | T '-' T { printf("%d %d - ", $1, $3); }

T : NUM { $$ = $1; }
%%

int main() {
  printf("Enter infix expr: ");
  yyparse();
}

void yyerror(const char* s) {
  printf("Err: %s\n", s);
}
