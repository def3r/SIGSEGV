%option noyywrap
%{
#include <string.h>
#include "binstr1.tab.h"
%}

%%
[01]        { yylval.count = atoi(yytext); return BIN; }
\n          { return 0; }
[ \t]       ;        /* ignore spaces */
.           ;        /* ignore other characters */
%%

