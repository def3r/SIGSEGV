%option noyywrap
%{
#include <stdio.h>
%}

%%
int|float|char         { printf("Keyword: %s\n", yytext); }
[0-9]+                 { printf("Number: %s\n", yytext); }
[a-zA-Z_][a-zA-Z0-9_]* { printf("Identifier: %s\n", yytext); }
[ \t\n]+               { }
.                      { printf("Symbol: %s\n", yytext); }
%%

int main() {
    yylex();
    return 0;
}
