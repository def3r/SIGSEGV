/*
 * switch2if.y
 *
 * Converts switch statements to if-else chains.
 *
 * switch (expr) {        if (expr == v1) {
 *   case v1: S1 break;      S1
 *   case v2: S2 break;  } else if (expr == v2) {
 *   default:  S3            S2
 * }                     } else {
 *                            S3
 *                        }
 *
 * Nesting works because 'statement' is recursive —
 * an inner switch is just another statement inside a case body.
 *
 * Usage:  cat input.c | ./switch2if.out
 */

%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void yyerror(const char *c);
int  yylex();

/* concat helpers */
static char *sdup(const char *s);
static char *scat2(const char *a, const char *b);
static char *scat3(const char *a, const char *b, const char *c);
static char *scat4(const char *a, const char *b, const char *c, const char *d);

char *result = NULL;
%}

%union { char *strval; }

%token <strval> T_ATOM          /* any non-keyword token passed through */
%token T_SWITCH
%token T_CASE
%token T_DEFAULT
%token T_BREAK
%token T_COLON
%token T_SEMICOLON
%token T_PARANOPEN
%token T_PARANCLOSE
%token T_CURLYOPEN
%token T_CURLYCLOSE

%type <strval> program
%type <strval> stmts
%type <strval> statement
%type <strval> switch_stmt
%type <strval> expr
%type <strval> case_list
%type <strval> case_clause
%type <strval> default_clause

%%

program
    : stmts             { result = $1; }
    ;


stmts
    : stmts statement   {
          $$ = scat2($1, $2);
          free($1); free($2);
      }
    | /* ε */           { $$ = sdup(""); }
    ;


statement
    : switch_stmt       { $$ = $1; }
    | T_ATOM            { $$ = $1; /* already strdup'd by ASSIGN() */ }
    | T_SEMICOLON       { $$ = sdup(";"); }
    | T_COLON           { $$ = sdup(":"); }
    | T_CURLYOPEN stmts T_CURLYCLOSE {
          /* pass-through braced blocks that are NOT switch bodies */
          char *tmp = scat3("{\n", $2, "\n}");
          free($2);
          $$ = tmp;
      }
    | T_PARANOPEN stmts T_PARANCLOSE {
          char *tmp = scat3("(", $2, ")");
          free($2);
          $$ = tmp;
      }
    ;


switch_stmt
    : T_SWITCH T_PARANOPEN expr T_PARANCLOSE
      T_CURLYOPEN case_list T_CURLYCLOSE
      {
          /*
           * $3  = the switch expression  (e.g. "x")
           * $6  = the already-built if-else chain
           *
           * case_list builds the chain with a placeholder "__SWITCHEXPR__"
           * so we can slot in the actual expression here.  We do a simple
           * search-and-replace pass.
           */
          const char *ph  = "__SWITCHEXPR__";
          size_t phlen    = strlen(ph);
          const char *expr_s = $3;
          size_t exprlen  = strlen(expr_s);
          const char *src = $6;
          size_t srclen   = strlen(src);

          /* count occurrences */
          size_t count = 0;
          for (const char *p = src; (p = strstr(p, ph)); p += phlen)
              count++;

          size_t newlen = srclen + count * (exprlen - phlen) + 1;
          char  *out    = malloc(newlen);
          char  *dst    = out;
          const char *p = src;
          while (*p) {
              if (strncmp(p, ph, phlen) == 0) {
                  memcpy(dst, expr_s, exprlen);
                  dst += exprlen;
                  p   += phlen;
              } else {
                  *dst++ = *p++;
              }
          }
          *dst = '\0';

          free($3); free($6);
          $$ = out;
      }
    ;

expr
    : expr T_ATOM       {
          $$ = scat2($1, $2);
          free($1); free($2);
      }
    | expr T_PARANOPEN expr T_PARANCLOSE {
          char *tmp = scat4($1, "(", $3, ")");
          free($1); free($3);
          $$ = tmp;
      }
    | T_ATOM            { $$ = $1; }
    ;

case_list
    : case_list case_clause   {
          /*
           * Join: first clause becomes "if", subsequent ones "else if".
           * We detect "if (" at the start of $2 to add "else " prefix.
           */
          if (strncmp($2, "if (", 4) == 0) {
              char *tmp = scat3($1, "\nelse ", $2);
              free($1); free($2);
              $$ = tmp;
          } else {
              /* default clause starts with "else {" already */
              char *tmp = scat2($1, $2);
              free($1); free($2);
              $$ = tmp;
          }
      }
    | case_clause       { $$ = $1; }
    ;

case_clause
    : T_CASE T_ATOM T_COLON stmts T_BREAK T_SEMICOLON {
          /*
           * $2 = case value (atom)
           * $4 = body statements (already recursively converted)
           */
          char *tmp = malloc(strlen($2) + strlen($4) + 64);
          sprintf(tmp, "if (__SWITCHEXPR__ == %s) {\n%s\n}", $2, $4);
          free($2); free($4);
          $$ = tmp;
      }
    ;


default_clause
    : T_DEFAULT T_COLON stmts {
          char *tmp = scat3("\nelse {\n", $3, "\n}");
          free($3);
          $$ = tmp;
      }
    ;

/* Allow default anywhere in case_list by making it a case_clause variant */
/* (Re-use the case_list rule — default_clause feeds into it via this.)   */
case_clause
    : T_DEFAULT T_COLON stmts {
          char *tmp = scat3("\nelse {\n", $3, "\n}");
          free($3);
          $$ = tmp;
      }
    ;

%%

int main(void) {
    yyparse();
    if (result) {
        printf("%s\n", result);
        free(result);
    }
    return 0;
}

void yyerror(const char *c) {
    fprintf(stderr, "Parse error: %s\n", c);
}

static char *sdup(const char *s) {
    size_t n = strlen(s) + 1;
    char  *r = malloc(n);
    memcpy(r, s, n);
    return r;
}

static char *scat2(const char *a, const char *b) {
    size_t la = strlen(a), lb = strlen(b);
    char  *r  = malloc(la + lb + 1);
    memcpy(r,      a, la);
    memcpy(r + la, b, lb + 1);
    return r;
}

static char *scat3(const char *a, const char *b, const char *c) {
    char *tmp = scat2(a, b);
    char *res = scat2(tmp, c);
    free(tmp);
    return res;
}

static char *scat4(const char *a, const char *b, const char *c, const char *d) {
    char *tmp = scat3(a, b, c);
    char *res = scat2(tmp, d);
    free(tmp);
    return res;
}
