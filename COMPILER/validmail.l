%option noyywrap
%{
#include <stdio.h>
#include <stdbool.h>

bool valid = true;
%}

AL [a-zA-Z]
AN [a-zA-Z0-9]

%%
{AL}{AN}+"@"{AN}+"."{AL}{AL}+ { }
.   { valid = false; }
%%

int main() {
  yylex();
  printf("valid = %s\n", (valid ? "True" : "False"));
}
