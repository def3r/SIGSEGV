%{
#include <stdio.h>
#include <stdlib.h>

int yylex();
void yyerror(const char *s);
%}

%union {
  int count;
};

%token <count> BIN

%type <count> L
%type <count> B

%%
S : L { printf("%d\n", $1); }

L : L B { $$ = $1 + $2;  }
  | B { $$ = $1; }

B : BIN { $$ = ($1 == 1); }

%%

int main()
{
    printf("Enter expression: ");
    yyparse();
    return 0;
}

void yyerror(const char *s)
{
    printf("Invalid expression\n");
}

