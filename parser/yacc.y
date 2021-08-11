%{
#include <stdio.h>
void yyerror(const char *s) {
  printf("error %s\n",s);
}
%}

%token NUM

%%
S : E '\n'  { printf("success\n"); }
  ;
E : E '+' T 
  | E '-' T 
  | T 
  ;
T : T '*' F
  | T '/' F
  | F
  ;
F : '(' E ')'
  | NUM
  ;
%%

int main(int argc,const char **argv) {
  freopen(argv[1],"r",stdin);
  return yyparse();
} 