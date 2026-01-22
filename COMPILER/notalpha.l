ALPHA [a-zA-Z]

%option noyywrap
%{
    #include <stdio.h>
    #include <stdbool.h>
    bool not_alpha = false;
%}

%%
{ALPHA}|[\n \t]   ; /* Nothing */
.                 { not_alpha= true; }
%%

int main() {
    yylex();
    printf("not_alpha = %s", (not_alpha ? "True" : "False"));
    return 0;
}
