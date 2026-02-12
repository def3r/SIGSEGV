%option noyywrap
%{
#include <stdio.h>
#include "even0s.tab.h"
%}

%%
"0"  return T_ZERO;
%%
