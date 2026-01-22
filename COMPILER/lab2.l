%{
    #include <stdio.h>
    int count = 0;
%}

%%
[0-9] { count++; printf("TOKEN_DIGIT");}
%%

int main() {
    yylex();
    printf("count = %d\n", count);
    return 0;
}

int yywrap(void) {
    return 1;
}
