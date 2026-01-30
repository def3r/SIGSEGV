%{
#include "balancedParan.tab.h"
%}

%%
[(]     { return T_BOPEN; }
[)]     { return T_BCLOSE; }
\n      { return 0; }
[ \t]   { }
.       { return T_OTHER; }
%%

int yywrap(void) {
  return 1;
}
