%{
#include "parser.tab.h"
#include <stdlib.h>
#include <stdio.h>
%}

%%
[0-9]+  { printf("yytext: %s\n", yytext); yylval = atoi(yytext); return NUMBER; }
[+*]     { return SYMBOL; }
\n      { return 0; }
[\t ]   ;
.       ;
%%

int yywrap() {
    return 1;
}
