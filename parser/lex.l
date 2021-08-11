%{
#include "stdio.h"
#include "y.tab.h"
%}

%%
(((([1-9])([0-9])*)|(0x[0-9A-Z]+)|(0[0-7]*))(U|L|UL|LL|ULL)?)|(((([1-9][0-9]*)\.[0-9]+(e[+|-]?[1-9][0-9]*)?)|[1-9][0-9]*e[+|-]?[1-9][0-9]*)(L)?) { printf("num = %s\n",yytext);return NUM; }
[-/+*()\n] { printf("mark = '%s'\n", yytext[0]=='\n'?"\\n":yytext);return yytext[0]; }
. { printf("other\n");return 0; }
%%

int yywrap(void) { 
  return 1;
}
