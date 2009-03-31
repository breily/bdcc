#include <stdio.h>
#include <stdlib.h>
#include "cc.h"
#include "ops.h"
#include "tree.h"

int aoff;			/* offset counter for arguments */
int loff;			/* offset counter for locals */
int labelno = 0;	/* global for generating labels */

extern int level;

/* prototypes */
void emittree(TNODE *);
void yyerror(char *);

IDENT *install(char *, int); 
IDENT *lookup(char *, int); 

/*
 * call - procedure invocation
 */
TNODE *call(char *f, TNODE *args) {
    TNODE *ret = (TNODE *) tnode();
    ret->t_op = TO_CALL;

    TNODE *fn = (TNODE *) tnode();
    fn->t_op = TO_NAME;
    // TODO: Figure out scope/level
    //IDENT *i = lookup(f, level);
    IDENT *i = lookup(f, 0);
    if (i == NULL) {
        fprintf(stderr, "warning: function '%s' not found, installing\n", f);
        i = install(f, LOCAL);
        i->i_type = T_INT | T_PTR | T_PROC;
    }
    fn->t_mode = i->i_type;
    fn->val.ln.t_id = i;

    ret->val.in.t_left = fn;
    ret->val.in.t_right = args;
    ret->t_mode = BASETYPE(i->i_type);
    return ret;
}

/*
 * ccand - logical and
 */
BNODE *ccand(BNODE *e1, int m, BNODE *e2) {
   //printf("ccand not implemented\n");
   return ((BNODE *) NULL);
}

/*
 * ccexpr - convert arithmetic expression to logical expression
 */
BNODE *ccexpr(TNODE *e) {
   //printf("ccexpr not implemented\n");
   return ((BNODE *) NULL);
}

/*
 * ccnot - logical not
 */
BNODE *ccnot(BNODE *e) {
   //printf("ccnot not implemented\n");
   return ((BNODE *) NULL);
}

/*
 * ccor - logical or
 */
BNODE *ccor(BNODE *e1, int m, BNODE *e2) {
   //printf("ccor not implemented\n");
   return ((BNODE *) NULL);
}

/*
 * con - constant reference in an expression
 */
TNODE *con(int c) {
    TNODE *ret = (TNODE *) tnode();
    ret->t_op = TO_CON;
    ret->t_mode = T_INT;
    ret->val.ln.t_con = c;
    return ret;
}

/*
 * dcl - add attributes to declaration
 */
IDENT *dcl(IDENT *p, char *name, int type, int width, int scope) {
    if (p == NULL) {
        p = install(name, level);
        p->i_type = type;
        p->i_width = width;
        p->i_blevel = level;
        p->i_scope = level; // TODO scope; ?
        p->i_defined = 0;
    } else {
        p->i_type = type;
        p->i_defined = 1;
        TNODE *e = (TNODE *) tnode();
        e->t_op = TO_ALLOC;
        e->t_mode = p->i_type;

        TNODE *l = (TNODE *) tnode();
        l->t_op = TO_NAME;
        l->val.ln.t_id = p;
        l->t_mode = p->i_type;
        e->val.in.t_left = l;

        TNODE *r = (TNODE *) tnode();
        r->t_op = TO_CON;
        r->val.ln.t_con = 4;    // TODO: Change for other types?
        r->t_mode = p->i_type;
        e->val.in.t_right = r;

        emittree(e);
    }
    return p;
}

/*
 * dofor - for statement
 */
BNODE *dofor(TNODE *e1, int m1, BNODE *e2, int m2, TNODE *e3, BNODE *n, int m3, BNODE *s) {
   //printf("dofor not implemented\n");
   return ((BNODE *) NULL);
}

/*
 * doif - one-arm if statement
 */
BNODE *doif(BNODE *e, int m, BNODE *s) {
   return ((BNODE *) NULL);
}

/*
 * doifelse - if then else statement
 */
BNODE *doifelse(BNODE *e, int m1, BNODE *s1, BNODE *n, int m2, BNODE *s2) {
   return ((BNODE *) NULL);
}

/*
 * doret - return statement
 */
BNODE *doret(TNODE *e) {
    BNODE *ret = (BNODE *) bnode();

    TNODE *tmp = (TNODE *) tnode();
    tmp->t_op = TO_RET;
    if (e) {
        tmp->t_mode = e->t_mode;
    } else {
        tmp->t_mode = 0;
    }
    tmp->val.in.t_left = e;
    emittree(tmp);

    return ret;
}

/*
 * dostmts - statement list
 */
BNODE *dostmts(BNODE *sl, int m, BNODE *s) {
   //printf("dostmts not implemented\n");
   return ((BNODE *) NULL);
}

/*
 * dowhile - while statement
 */
BNODE *dowhile(int m1, BNODE *e, int m2, BNODE *s) {
   //printf("dowhile not implemented\n");
   return ((BNODE *) NULL);
}

/*
 * exprs - form a list of expressions
 */
TNODE *exprs(TNODE *l, TNODE *e) {
    TNODE *lst = (TNODE *) tnode();
    lst->t_op = TO_LIST;
    lst->t_mode = 0;
    lst->val.in.t_left = l;
    lst->val.in.t_right = e;
    return lst;
}

/*
 * fhead - beginning of function body
 */
IDENT *fhead(IDENT *p) {
    p->i_defined = 1;

    TNODE *e = (TNODE *) tnode();
    e->t_op = TO_FUNC;
    e->t_mode = p->i_type;

    TNODE *l = (TNODE *) tnode();
    l->t_op = TO_NAME;
    l->t_mode = p->i_type | T_PTR;
    l->val.ln.t_id = p;
    e->val.in.t_left = l;

    TNODE *r = (TNODE *) tnode();
    r->t_op = TO_LIST;
    r->t_mode = 0;

    /*
    TNODE *arg_size = (TNODE *) tnode();
    arg_size->t_op = TO_CON;
    arg_size->t_mode = T_INT;
    arg_size->val.ln.t_con = 0; // TODO: ?
    */
    TNODE *arg_size = con(0);

    /*
    TNODE *loc_size = (TNODE *) tnode();
    loc_size->t_op = TO_CON;
    loc_size->t_mode = T_INT;
    loc_size->val.ln.t_con = 0; // TODO: ?
    */
    TNODE *loc_size = con(0);

    r->val.in.t_left = arg_size;
    r->val.in.t_right = loc_size;

    e->val.in.t_right = r;

    emittree(e);
    return p;
}

/*
 * fname - function declaration
 */
IDENT *fname(TWORD t, char *id) {
    IDENT *ret = install(id, level);
    ret->i_type = t | T_PROC | T_PTR;
    ret->i_name = id;
    ret->i_blevel = level;
    ret->i_defined = 0;
    ret->i_scope = GLOBAL;
    enterblock();
    return ret;
}

/*
 * ftail - end of function body
 */
void ftail(IDENT *p, BNODE *s, int m) {
    TNODE *tmp = (TNODE *) tnode();
    tmp->t_op = TO_FEND;
    tmp->t_mode = p->i_type;

    TNODE *f = (TNODE *) tnode();
    f->t_op = TO_NAME;
    f->t_mode = p->i_type;
    f->val.ln.t_id = p;

    tmp->val.in.t_left = f;
    emittree(tmp);
}


/*
 * id - variable reference
 */
TNODE *id(char *x) {
    IDENT *s = lookup(x, 0);    // TODO: 0 or level?
    if (s == NULL) {
        fprintf(stderr, "error: '%s' not found, installing\n", x);
        s = install(x, LOCAL);
        s->i_type = T_INT;
    }
    TNODE *ret = (TNODE *) tnode();
    ret->t_op = TO_NAME;
    ret->t_mode = s->i_type;
    ret->val.ln.t_id = s;
    return ret;
}

/*
 * aindex - subscript
 */
TNODE *aindex(TNODE *x, TNODE *i) {
   printf("aindex not implemented\n");
   return ((TNODE *) NULL);
}

/*
 * m - generate a label
 */
int m(void) {
   return 0;
}

/*
 * n - generate goto and return backpatch pointer
 */
BNODE *n(void) {
   //printf("n not implemented\n");
   return ((BNODE *) NULL);
}

/*
 * op1 - unary operators
 */
TNODE *op1(int op, TNODE *x) {
   printf("op1 not implemented\n");
   return ((TNODE *) NULL);
}

/*
 * op_arith - arithmetic operators
 */
TNODE *op_arith(int op, TNODE *x, TNODE *y) {
    TNODE *ret = (TNODE *) tnode();
    ret->t_op = op;
    ret->t_mode = promote(x, y);
    ret->val.in.t_left = x;
    ret->val.in.t_right = y;
    if (op == TO_DIV && y->t_op == TO_CON && y->val.ln.t_con == 0) {
        fprintf(stderr, "warning: division by zero\n");
    }
    return ret;
}

/*
 * op_bitwise - bitwise operators
 */
TNODE *op_bitwise(int op, TNODE *x, TNODE *y) {
    TNODE *ret = (TNODE *) tnode();
    ret->t_op = op;
    if (x->t_mode != T_INT || y->t_mode != T_INT) {
        fprintf(stderr, "error: bitwise ops require integer arguments\n");
    }
    ret->t_mode = T_INT;
    ret->val.in.t_left = x;
    ret->val.in.t_right = y;
    return ret;
}

/*
 * op_com - complement operator
 */
TNODE *op_com(TNODE *x) {
    TNODE *ret = (TNODE *) tnode();
    ret->t_op = TO_COMP;
    if (x->t_mode != T_INT) {
        fprintf(stderr, "error: complement op requires integer argument\n");
    }
    ret->t_mode = T_INT;
    ret->val.in.t_left = x;
    return ret;
}

/*
 * op_deref - dereference an lval
 */
TNODE *op_deref(TNODE *x) {
    TNODE *ret = (TNODE *) tnode();
    ret->t_op = TO_DEREF;
    ret->t_mode = BASETYPE(x->t_mode);
    ret->val.in.t_left = x;
    return ret;
}

/*
 * op_mod - modulus operator
 */
TNODE *op_mod(TNODE *x, TNODE *y) {
    TNODE *ret = (TNODE *) tnode();
    ret->t_op = TO_MOD;
    ret->t_mode = T_INT;
    ret->val.in.t_left = x;
    ret->val.in.t_right = y;
    return ret;
}

/*
 * op_neg - negation operator
 */
TNODE *op_neg(TNODE *x) {
    TNODE *ret = (TNODE *) tnode();
    ret->t_op = TO_NEGATE;
    ret->t_mode = x->t_mode;
    ret->val.in.t_left = x;
    return ret;
}

/*
 * rel - relational operators
 */
BNODE *rel(int op, TNODE *x, TNODE *y) {
   printf("rel not implemented\n");
   return ((BNODE *) NULL);
}

/*
 * set - assignment operators
 */
TNODE *set(TNODE *x, TNODE *y) {
   TNODE *ret = (TNODE *) tnode();
   ret->t_op = TO_ASSIGN;
   ret->t_mode = BASETYPE(x->t_mode);
   ret->val.in.t_left = x;
   if (x->t_mode != y->t_mode) {
        TNODE *casted = (TNODE *) tnode();
        casted->t_op = TO_CAST;
        casted->t_mode = x->t_mode;
        casted->val.in.t_left = y;
        ret->val.in.t_right = casted;
   } else {
        ret->val.in.t_right = y;
   }
   return ret;
}

/*
 * setaug1 - augmented arithmetic assignment operators
 */
TNODE *setaug1(int op, TNODE *x, TNODE *y) {
   printf("setaug not implemented\n");
   return ((TNODE *) NULL);
}

/*
 * setaug2 - augmented logical assignment operators
 */
TNODE *setaug2(int op, TNODE *x, TNODE *y) {
   printf("setaug not implemented\n");
   return ((TNODE *) NULL);
}

/*
 * string - generate code for a string
 */
TNODE *string(char *s) {
    TNODE *ret = (TNODE *) tnode();
    ret->t_op = TO_STR;
    ret->t_mode = T_PTR;
    ret->val.ln.t_str = s;
    return ret;
}
