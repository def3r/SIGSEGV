%{
#include <stdio.h>

int yylex();
void yyerror(const char*);
%}

%token T_BOPEN
%token T_BCLOSE
%token T_OTHER

%glr-parser
%%
expr : T_BOPEN expr T_BCLOSE expr { }
     | /* epsilon */              { }
%%

int main() {
  printf("Enter String: ");

  yyparse();
  return 0;
}

void yyerror(const char* c) {
  fprintf(stderr, c);
}
