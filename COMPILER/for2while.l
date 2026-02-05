%option noyywrap
%{
#include <stdio.h>
#include <string.h>
#include "for2while.tab.h"

#define ASSIGN() yylval.strval = strdup(yytext);
%}

IDENTIFIER [a-zA-Z_][a-zA-Z0-9_]*

%%
"for"     { return T_FOR; }

[(]       { return T_PARANOPEN; }
[)]       { return T_PARANCLOSE; }
[;]       { return T_SEMICOLON; }
[=]       { return T_EQUAL; }
[<]       { return T_LESSTHAN; }
[{]       { return T_CURLYOPEN; }
[}]       { return T_CURLYCLOSE; }

[0-9]+           { ASSIGN() return T_INTLIT; }
{IDENTIFIER}     { ASSIGN() return T_IDENTIFIER; }
{IDENTIFIER}\+\+ { ASSIGN() return T_INCREMENT; }

[\n \t] { }
.       { }
%%
