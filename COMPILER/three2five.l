ALPHA   [a-zA-Z]
NOALPHA [^a-zA-Z]
SEQ     {NOALPHA}*{ALPHA}
SEQ3    {SEQ}{SEQ}{SEQ}{NOALPHA}*
SEQ4    {SEQ3}{SEQ}{NOALPHA}*
SEQ5    {SEQ4}{SEQ}{NOALPHA}*

%option noyywrap
%{
    #include <stdio.h>
%}

%%
^{SEQ5}|{SEQ4}|{SEQ3}$ { printf("3-5 Alphabets\n"); fflush(stdout); }
\n
.
%%

int main() {
    yylex();
    return 0;
}
