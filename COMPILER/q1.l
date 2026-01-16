%option noyywrap
%{
#include <stdio.h>
%}

%%
0*[0-9]|0*[0-9][0-9]  { printf("hello"); }
0*[1-9][0-9][0-9]+    { printf("bye"); }
%%

int main() {
    // Lex analyser, print hello if num is < 100 else Bye
    return yylex();
}
