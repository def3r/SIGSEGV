DIGIT [0-9]

%option noyywrap
%{
    #include <stdio.h>
    int pint = 0, nint = 0;
    int pfloat = 0, nfloat = 0;
%}

%%
{DIGIT}+             { pint++; printf("Positive Integer\n"); }
-{DIGIT}+            { nint++; printf("Negative Integer\n"); }
{DIGIT}+\.{DIGIT}*   { pfloat++; printf("Positive Float\n"); }
-{DIGIT}+\.{DIGIT}*  { nfloat++; printf("Negative Float\n"); }
%%

int main() {
    yylex();
    printf("pint: %d, nint: %d\n", pint, nint);
    printf("pfloat: %d, nfloat: %d\n", pfloat, nfloat);
    return 0;
}
