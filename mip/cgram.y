/*
 * This grammar for this C contains 1 shift/reduce conflict
 * and 1 reduce/reduce conflict
 */
%{
#include <stdio.h>
#include "cc.h"
#include "ops.h"

BNODE *bnode(void);
BNODE *n(void); 
BNODE *dostmts(BNODE *, int, BNODE *), *doif(BNODE *, int, BNODE *);
BNODE *doifelse(BNODE *, int, BNODE *, BNODE *, int, BNODE *);
BNODE *dowhile(int, BNODE *, int, BNODE *);
BNODE *dofor(TNODE *, int, BNODE *, int, TNODE *, BNODE *, int, BNODE *);
BNODE *doret(TNODE *), *rel(int, TNODE *, TNODE *);
BNODE *ccand (BNODE *, int, BNODE *);
BNODE *bldbnode(int, BNODE *, BNODE *);
BNODE *ccor(BNODE *, int, BNODE *);
BNODE *ccnot(BNODE *), *ccexpr(TNODE *);
TNODE *exprs(TNODE *, TNODE *), *string(char *), *set(TNODE *, TNODE *);
TNODE *setaug1 (int, TNODE *, TNODE *);
TNODE *setaug2 (int, TNODE *, TNODE *);
TNODE *id(char *), *op_bitwise(int, TNODE *, TNODE *);
TNODE *op_arith(int, TNODE *, TNODE *);
TNODE *op_mod(TNODE *, TNODE *);
TNODE *op_neg(TNODE *);
TNODE *op_com(TNODE *);
TNODE *op_deref(TNODE *);
TNODE *call(char *, TNODE *);
TNODE *con(int);
TNODE *aindex(TNODE *, TNODE *);
int m(void);
int yylex(void);
IDENT *dcl(IDENT *, char *, int, int, int);
IDENT *fhead(IDENT *), *fname(TWORD, char *);
void emittree(TNODE *);
void ftail(IDENT *, BNODE *, int); 

extern int lineno;
void yyerror(char *msg) {
   fprintf(stderr, " %s.  Line %d\n", msg, lineno);
}

%}

%union {
   int inttype;
   char *str_ptr;
   BNODE *bkp_ptr;
   IDENT *id_ptr;
   TNODE *tree_ptr;
}
%token <str_ptr> ID CON STR
%token CHAR ELSE DOUBLE FOR IF INT RESERVED RETURN WHILE
%right LVAL
%right SET SETOR SETXOR SETAND SETLSH SETRSH SETADD SETSUB SETMUL SETDIV SETMOD
%left  OR
%left  AND
%left  BITOR
%left  BITXOR
%left  BITAND
%left  EQ NE
%left  GT GE LT LE
%left  LSH RSH
%left  ADD SUB
%left  INC DEC
%left  MUL DIV MOD
%right UNARY NOT COM MINUSMINUS PLUSPLUS
%type <tree_ptr> lval expr exprs expro
%type <bkp_ptr>  cexpr cexpro stmt stmts block n func 
%type <id_ptr>   dcl dclr fhead fname args
%type <inttype>  type m
%%
prog    : externs	        	{}
        ;

externs :                       {}
        | externs extern        {}
        ;

extern  : dcl ';'               {}				
        | func                  {}
        ;

dcls    :                       {}
        | dcls dcl ';'          {}
        ;

dcl     : type dclr             { $$ = dcl($2, (char *) NULL, $1, 0, 0); }
        | dcl ',' dclr          { $$ = dcl($3, (char *) NULL, $1->i_type&~T_ARRAY, 0, 0); }
        ;

dclr    : ID                    { $$ = dcl((IDENT *) NULL, $1, 0, 1, 0); }
        | ID '[' ']'            { $$ = dcl((IDENT *) NULL, $1, T_ARRAY, 1, 0); }
        | ID '[' CON ']'        { $$ = dcl((IDENT *) NULL, $1, T_ARRAY, atoi($3), 0); }
        ;

type    : CHAR                  { $$ = T_INT; }
        | DOUBLE                { $$ = T_DOUBLE; }
        | INT                   { $$ = T_INT; }
        ;

func    : fhead stmts '}' m     { ftail($1, $2, $4); }
        ;

fhead   : fname fargs '{' dcls  { $$ = fhead($1); }
        ;

fname   : type ID               { $$ = fname($1, $2); }
        | ID                    { $$ = fname(T_INT, $1); }
        ;

fargs   : '(' args ')'          {}
        | '(' ')'               {}
        ;

args    : type dclr             { $$ = dcl($2, (char *) NULL, $1, 0, PARAM); }
        | args ',' type dclr    { $$ = dcl($4, (char *) NULL, $3, 0, PARAM); }
        ;

m       :                       { $$ = m(); }
        ;

n       :                       { $$ = n(); }
        ;

block   : '{' stmts '}'         { $$ = $2; }
        ;

stmts   :                       { $$ = 0; }
        | stmts m stmt          { $$ = dostmts($1, $2, $3); }
        ;

stmt    : expr ';'
                { emittree($1); $$ = (BNODE *) NULL; }
        | IF '(' cexpr ')' m stmt
                { $$ = doif($3, $5, $6); }
        | IF '(' cexpr ')' m stmt n ELSE m stmt
                { $$ = doifelse($3, $5, $6, $7, $9, $10); }
        | WHILE '(' m cexpr ')' m stmt
                { $$ = dowhile($3, $4, $6, $7); }
        | FOR '(' expro ';' m cexpro ';' m expro n ')' m stmt
                { $$ = dofor($3, $5, $6, $8, $9, $10, $12, $13); }
        | RETURN ';'
                { $$ = doret((TNODE *) NULL); }
        | RETURN expr ';'
                { $$ = doret($2); }
        | block
                { $$ = $1; }
        | ';'
                { $$ = 0; }
        ;

cexpro  :		    	{ $$ = bnode (); $$->back.b_true = n(); }
        | cexpr			{ }
        ;

cexpr   : expr EQ expr          { $$ = rel (TO_CMPEQ, $1, $3); }
        | expr NE expr          { $$ = rel (TO_CMPNE, $1, $3); }
        | expr LE expr          { $$ = rel (TO_CMPLE, $1, $3); }
        | expr GE expr          { $$ = rel (TO_CMPGE, $1, $3); }
        | expr LT expr          { $$ = rel (TO_CMPLT,  $1, $3); }
        | expr GT expr          { $$ = rel (TO_CMPGT,  $1, $3); }
        | cexpr AND m cexpr     { $$ = ccand ($1, $3, $4); }
        | cexpr OR m cexpr      { $$ = ccor ($1, $3, $4); }
        | NOT cexpr             { $$ = ccnot ($2); }
        | '(' cexpr ')'         { $$ = $2; }
        | expr                  { $$ = ccexpr ($1); }
        ;

exprs   : expr                  { $$ = $1; }
        | exprs ',' expr        { $$ = exprs ($1, $3); }
        ;

expro   :			            {}
        | expr			        { emittree ($1); }
        ;

expr    : lval SET expr           { $$ = set($1, $3); }
        | expr BITOR expr         { $$ = op_bitwise(TO_OR,  $1, $3); }
        | expr BITXOR expr        { $$ = op_bitwise(TO_XOR, $1, $3); }
        | expr BITAND expr        { $$ = op_bitwise(TO_AND, $1, $3); }
        | expr LSH expr           { $$ = op_bitwise(TO_LS,  $1, $3); }
        | expr RSH expr           { $$ = op_bitwise(TO_RS,  $1, $3); }
        | expr ADD expr           { $$ = op_arith(TO_PLUS,  $1, $3); }
        | expr SUB expr           { $$ = op_arith(TO_MINUS, $1, $3); }
        | expr MUL expr           { $$ = op_arith(TO_MUL,   $1, $3); }
        | expr DIV expr           { $$ = op_arith(TO_DIV,   $1, $3); }
        | expr MOD expr           { $$ = op_mod($1, $3); }
        | BITAND lval %prec UNARY { $$ = $2; }
        | SUB expr %prec UNARY    { $$ = op_neg($2); }
        | COM expr                { $$ = op_com($2); }
        | lval %prec LVAL         { $$ = op_deref($1); }
        | ID '(' ')'              { $$ = call($1, (TNODE *) NULL); }
        | ID '(' exprs ')'        { $$ = call($1, $3); }
        | '(' expr ')'            { $$ = $2; }
        | CON                     { $$ = con(atoi($1)); }
        | STR	        		  { $$ = string($1); }
        ;

lval    : ID                    { $$ = id ($1); }
        | lval '[' expr ']'     { $$ = aindex ($1, $3); }
        ;

%%
int showtree = 0;
int codegen = 0;
int binary = 0;

void enterblock(void), initlex(void);
int main(int, char **), strcmp (const char *, const char *), yyparse(void);

/* 
 * main - read a program, and parse it
 */
int main(int argc, char *argv[]) {
   int i;

   for (i = 1; i < argc; i++) {
       if (strcmp (argv[i], "-showtree") == 0)     showtree = 1;
       else if (strcmp (argv[i], "-codegen") == 0) codegen = 1;
       else if (strcmp (argv[i], "-binary") == 0)  {
           binary = 1;
           codegen = 1;
       }
   }
   enterblock();
   initlex();
   enterblock();
   (void) yyparse();
   return 0;
}
