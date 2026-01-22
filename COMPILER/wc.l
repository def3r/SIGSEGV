CHARACTER [a-zA-Z]

%option noyywrap
%{
    #include <stdio.h>
    #include <string.h>
    int words = 0, chr = 0, lines = 0;
%}

%%
\n             { chr++; lines++; }
[ \t]          { chr++; }
{CHARACTER}    { chr++; }
{CHARACTER}[ ] { chr += 2; words++; }
{CHARACTER}\n  { chr += 2; lines++; words++; }
.              { chr++; }
%%

int main() {
    yylex();
    printf("words: %d, characters: %d, lines: %d\n", words, chr, lines);
    return 0;
}
