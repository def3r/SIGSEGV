%{
#include "anbn.tab.h"
%}

%%
a       { return TOKEN_A; }
b       { return TOKEN_B; }
\n      { return 0; }
[\t ]   ;
.       { return TOKEN_OTHER; }
%%

int yywrap() {
    return 1;
}
