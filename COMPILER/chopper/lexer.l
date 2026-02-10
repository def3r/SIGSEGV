%option noyywrap
%{
#include <stdio.h>

// #include "parser.tab.h"
%}

NUM   [0-9]
IDEN  [a-zA-Z_][a-zA-Z0-9_]*

%%
"fn"    { printf("T_FUNC "); }
"main"  { printf("T_MAIN "); }
"int"   { printf("T_INT "); }
"str"   { printf("T_STR "); }
"return" { printf("T_RETURN "); }

{NUM}     { printf("T_INTLIT "); }
{IDEN}    { printf("T_IDEN "); }

\/\/.*    { printf("COMMNET "); }
":"       { printf("T_COLON "); }
";"       { printf("T_SCOLON "); }
"("       { printf("T_POPEN "); }
")"       { printf("T_PCLOSE "); }
"["       { printf("T_SOPEN "); }
"]"       { printf("T_SCLOSE "); }
"{"       { printf("T_COPEN "); }
"}"       { printf("T_CCLOSE "); }
"&"       { printf("T_AMP "); }
","       { printf("T_COMMA "); }

"\""[^\"]*"\"" { printf("T_STRLIT "); }

[ \t] { }
[\n]  { printf("\n"); }
.       { }
%%

int main() {
    return yylex();
}
