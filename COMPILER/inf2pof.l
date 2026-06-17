%option noyywrap
%{
#include "inf2pof.tab.h"
%}

NUMBER [0-9]

%%
{NUMBER}+ { yylval = atoi(yytext); return NUM; }
"-"       { return '-'; }
"+"       { return '+'; }
\n        { }
.         { }
%%
