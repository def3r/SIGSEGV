// for ( initializer; condition; inc )
//
// TO
//
// initializer;
// while (condition) {
// stmt;
// increment;
// }

// cat for2while.txt | ./for2while.out

%{
#include <stdio.h>
#include <string.h>

void yyerror(const char* c);
int yylex();

char *make_str(char *s);
char *make_str2(char *s, char c, char *t);

char *res = NULL;
%}

%union {
  char *strval;
}

%token <strval> T_INTLIT
%token <strval> T_INCREMENT
%token <strval> T_IDENTIFIER

%token T_FOR
%token T_EQUAL
%token T_LESSTHAN
%token T_CURLYOPEN
%token T_SEMICOLON
%token T_PARANOPEN
%token T_PARANCLOSE
%token T_CURLYCLOSE

%type <strval> statement
%type <strval> for_loop
%type <strval> init
%type <strval> cond
%type <strval> inc

// First symbol is the start symboll
%%
program    : statement  { res = $1; }

statement  : for_loop   { $$ = make_str($1); }
           | /*epsion*/ { $$ = make_str(""); }

for_loop   : T_FOR T_PARANOPEN
               init  T_SEMICOLON
               cond  T_SEMICOLON
               inc
             T_PARANCLOSE T_CURLYOPEN statement T_CURLYCLOSE {
               $$ = malloc(512);
               snprintf($$, 512,
                 "\n%s;\nwhile(%s) {\n%s\n%s;\n}\n", $3, $5, $10, $7);
               free($3); free($5); free($7); free($10);
             }

init       : T_IDENTIFIER T_EQUAL T_INTLIT    {
               $$ = make_str2($1, '=', $3);
               free($1); free($3);
             }

cond       : T_IDENTIFIER T_LESSTHAN T_INTLIT {
               $$ = make_str2($1, '<', $3);
               free($1); free($3);
             }

inc        : T_INCREMENT { $$ = make_str($1); free($1); }
%%

int main() {
  yyparse();

  if (res != NULL) {
    printf(res);
  }

  return 0;
}

void yyerror(const char* c) {
  printf("Err: %s", c);
}

char *make_str(char *s) {
  size_t len = strlen(s) + 1;
  char *res = malloc(len);
  memcpy(res, s, len);
  return res;
}

char *make_str2(char *s, char c, char *t) {
  size_t len = strlen(s) + strlen(t) + 1 + 3;
  char *res = malloc(len);
  snprintf(res, len, "%s %c %s", s, c, t);
  return res;
}
