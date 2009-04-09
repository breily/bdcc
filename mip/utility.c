#include <stdio.h>
#include <stdlib.h>
#include "cc.h"
#include "ops.h"
#include "tree.h"

struct treeops opdope[] = {
   TO_ILLG, "ILLG", 0,
   TO_STR, "STRG", LEAFTYPE,
   TO_NAME, "NAME", LEAFTYPE,
   TO_DEREF, "@", UNARYTYPE,
   TO_ASSIGN, "=", MISCBINTYPE,
   TO_CAST, "CAST", UNARYTYPE,
   TO_CON, "CNST", LEAFTYPE,
   TO_COMP, "~", UNARYTYPE,
   TO_NEGATE, "-", UNARYTYPE,
   TO_JMP, "JMP", UNARYTYPE,
   TO_JMPT, "JMPT", MISCBINTYPE,
   TO_CMPEQ, "==", SIMPLEBINTYPE,
   TO_CMPNE, "!=", SIMPLEBINTYPE,
   TO_CMPLT, "<", SIMPLEBINTYPE,
   TO_CMPLE, "<=", SIMPLEBINTYPE,
   TO_CMPGT, ">", SIMPLEBINTYPE,
   TO_CMPGE, ">=", SIMPLEBINTYPE,
   TO_LABEL, "LABEL", LEAFTYPE,
   TO_BLABEL, "BLABEL", LEAFTYPE,
   TO_RET, "RET", UNARYTYPE,
   TO_LIST, "LIST", MISCBINTYPE,
   TO_FUNC, "FUNC", MISCBINTYPE,
   TO_PLUS, "+", SIMPLEBINTYPE,
   TO_MINUS, "-", SIMPLEBINTYPE,
   TO_MUL, "*", SIMPLEBINTYPE,
   TO_DIV, "/", SIMPLEBINTYPE,
   TO_MOD, "%", SIMPLEBINTYPE,
   TO_OR, "|", SIMPLEBINTYPE,
   TO_XOR, "^", SIMPLEBINTYPE,
   TO_AND, "&", SIMPLEBINTYPE,
   TO_LS, "<<", SIMPLEBINTYPE,
   TO_RS, ">>", SIMPLEBINTYPE,
   TO_APLUS, "+=", AOPBINTYPE1,
   TO_AMINUS, "-=", AOPBINTYPE1,
   TO_AMUL, "*=", AOPBINTYPE1,
   TO_ADIV, "/=", AOPBINTYPE1,
   TO_AMOD, "%=", AOPBINTYPE2,
   TO_AOR, "|=", AOPBINTYPE2,
   TO_AXOR, "^=", AOPBINTYPE2,
   TO_AAND, "&=", AOPBINTYPE2,
   TO_ALS, "<<=", AOPBINTYPE2,
   TO_ARS, ">>=", AOPBINTYPE2,
   TO_PREPLUS, "++x", MISCBINTYPE,
   TO_POSTPLUS, "x++", MISCBINTYPE,
   TO_PREMINUS, "--x", MISCBINTYPE,
   TO_POSTMINUS, "x--", MISCBINTYPE,
   TO_CALL, "CALL", MISCBINTYPE,
   TO_EQU, "EQU", MISCBINTYPE,
   TO_FEND, "FEND", UNARYTYPE,
   TO_ALLOC, "ALLOC", MISCBINTYPE,
   TO_REG, "REGISTER", LEAFTYPE
};

int indent = 0;
extern int codegen, showtree;

BNODE *bnode(void);
TNODE *tnode(void);
void printnode(TNODE *), emittree(TNODE *), printtree(TNODE *);
void printtype(TWORD); void initcgen(void);

int cgen(TNODE *);

/*
 * bnode - allocate and initialize a backpatch node
 */
BNODE *bnode() {
   BNODE *b;

   b = (BNODE *) malloc (sizeof(struct bkp_node));
   b->b_label = 0; 
   b->back.b_true = (BNODE *) NULL;
   b->b_false = (BNODE *) NULL;
   return b;
}

/*
 * emittree - emit the tree (either as code or as readable)
 */
void emittree (TNODE *n) {
   int rn;

   initcgen();
   if (showtree) printtree(n);
   if (codegen)  cgen(n);
   if (showtree) printf("\n");
}

/*
 * logbase2 - return log base 2 of number
 */
int logbase2(int x) {
   int i;
   for (i = 0; !(x & 1); i++) x >>= 1;
   return i;
}

/*
 * printnode - print a tree node in a readable format
 */
void printnode(TNODE *n) {
   int i;
   static char *space="   ";

   for (i = indent; i; i--)
      printf("%s", space);

   printf("# %s ", opdope[n->t_op].opstring);
   printtype(n->t_mode);

   if (LEAF(n))
      switch (n->t_op) {
         case TO_STR:
            printf(" value=%s", n->val.ln.t_str);
            break;

         case TO_NAME:
            printf(" name=\"%s\", scope = %d, offset=%d",
             n->val.ln.t_id->i_name, n->val.ln.t_id->i_scope,
             n->val.ln.t_id->i_offset);
            break;

         case TO_CON:
            printf(" value=%d", n->val.ln.t_con);
            break;

         case TO_LABEL:
            printf(" label=%d", n->val.ln.t_con);
            break;

         case TO_BLABEL:
            printf(" blabel=%d", n->val.ln.t_con);
            break;

         case TO_REG:
            printf(" register=%d", n->val.ln.t_reg);
            break;

         default:
            fprintf(stderr, "illegal opcode\n");
            exit(1);
      }
   printf("\n");
}

/*
 * printtree - print the tree in a readable format
 */
void printtree(TNODE *r) {
   if (r == (TNODE *) NULL) return;
   printnode(r);
   indent++;
   if (BINARY(r)) {
      printtree (r->val.in.t_left);
      printtree (r->val.in.t_right);
   }
   else if (UNARY(r))
      printtree (r->val.in.t_left);
   indent--;
}

/*
 * printtype - print the type in a symbolic way
 */
void printtype(TWORD t) {
   if (t & T_PTR)    printf("PTR|");
   if (t & T_ARRAY)  printf("ARRAY|");
   if (t & T_PROC)   printf("PROC|");
   if (t & T_DOUBLE) printf("DOUBLE");
   if (t & T_INT)    printf("INT");
   return;
}
 
/*
 * tnode - allocate and initialize a tree node
 */
TNODE *tnode() {
   TNODE *t;

   t = (TNODE *) malloc (sizeof(struct tree_node));
   t->t_op = 0;
   t->t_mode = 0;
   t->val.in.t_left = (TNODE *) NULL;
   t->val.in.t_right = (TNODE *) NULL;
   return t;
}

/*
 * cerror - print a fatal compiler message and exit
 */
void cerror(char *s) {
   fprintf(stderr, "!! error: %s\n", s);
   exit(1);
}

/*
 * promote - produce T_FLOAT if either arg is of type float,
 *           otherwise return T_INT.
 */
int promote(TNODE *l, TNODE *r) {
    if (l->t_mode & T_DOUBLE || r->t_mode & T_DOUBLE) {
        return T_DOUBLE;
    }
    return T_INT;
}

/*
void backpatch_true(BNODE *b, int m) {
    while (b) {
        b->b_label = m;
        b = b->back.b_true;
    }
}

void backpatch_false(BNODE *b, int m) {
    while (b) {
        b->b_label = m;
        b = b->b_false;
    }
}
*/

void backpatch(BNODE *b, int m) {
    while (b) {
        b->b_label = m;
        b = b->back.b_link;
    }
}

BNODE *merge(BNODE *a, BNODE *b) {
    if (!b) return a;
    if (!a) return b;
    BNODE *c = a;
    while (c->back.b_link) c = c->back.b_link;
    c->back.b_link = b;
    return c;
}
