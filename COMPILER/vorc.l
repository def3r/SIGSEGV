%option noyywrap
%{
    #include <stdio.h>
    int vowels = 0, consonants = 0;
%}

%%
[aeiou]             { vowels++; }
[b-df-hj-np-tv-z]   { consonants++; }
%%

int main() {
    yylex();
    printf("vowels: %d, consonants: %d\n", vowels, consonants);
    return 0;
}
