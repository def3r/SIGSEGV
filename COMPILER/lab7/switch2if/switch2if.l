%option noyywrap
%{
#include <stdio.h>
#include <string.h>
#include "switch2if.tab.h"
#define ASSIGN() yylval.strval = strdup(yytext);
%}

IDENTIFIER [a-zA-Z_][a-zA-Z0-9_]*
INTLIT     [0-9]+
STRLIT     \"([^\\\"]|\\.)*\"

%%
"switch"          { return T_SWITCH; }
"case"            { return T_CASE; }
"default"         { return T_DEFAULT; }
"break"           { return T_BREAK; }
"("               { return T_PARANOPEN; }
")"               { return T_PARANCLOSE; }
"{"               { return T_CURLYOPEN; }
"}"               { return T_CURLYCLOSE; }
":"               { return T_COLON; }
";"               { return T_SEMICOLON; }

{STRLIT}          { ASSIGN() return T_ATOM; }
{INTLIT}          { ASSIGN() return T_ATOM; }
{IDENTIFIER}      { ASSIGN() return T_ATOM; }

[+\-*/%=!<>&|^~.,\[\]] { ASSIGN() return T_ATOM; }

[ \t\n]+          { /* skip whitespace */ }
.                 { ASSIGN() return T_ATOM; }
%%
