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
    backpatch(e1->back.b_true, m);
    BNODE *b = (BNODE *) bnode();
    b->b_label = labelno;
    b->back.b_true = e2->back.b_true;
    b->b_false = (BNODE *) merge(e1->b_false, e2->b_false);
    return b;
}

/*
 * ccexpr - convert arithmetic expression to logical expression
 */
BNODE *ccexpr(TNODE *e) {
    printf("* ccexpr called\n");
    // TODO:
    BNODE *b = (BNODE *) bnode();
    b->b_label = labelno;
    return b;
}

/*
 * ccnot - logical not
 */
BNODE *ccnot(BNODE *e) {
    BNODE *b = (BNODE *) bnode();
    b->b_label = labelno;
    b->back.b_true = e->b_false;
    b->b_false = e->back.b_true;
    return b;
}

/*
 * ccor - logical or
 */
BNODE *ccor(BNODE *e1, int m, BNODE *e2) {
    backpatch(e1->b_false, m);
    BNODE *b = (BNODE *) bnode();
    b->b_label = labelno;
    b->back.b_true = (BNODE *) merge(e1->back.b_true, e2->back.b_true);
    b->b_false = e2->b_false;
    return b;
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
    if (level == 2) {
        if (p == NULL) {
            p = install(name, level);
            p->i_type = type;
            p->i_width = width;
            p->i_blevel = level;
            p->i_scope = scope;
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
    } else if (level > 2) {
        if (p == NULL) {
            loff += width * 4;
            p = install(name, level);
            p->i_type = type;
            p->i_width = width;
            p->i_blevel = level;
            p->i_scope = scope;
            p->i_defined = 0;
            p->i_offset = loff;
        } else {
            p->i_type = type;
        }
        return p;
    } else if (scope == PARAM) {
        printf("param scope\n");
    }
}

/*
 * dofor - for statement
 */
BNODE *dofor(TNODE *e1, int m1, BNODE *e2, int m2, TNODE *e3, BNODE *n, int m3, BNODE *s) {
   return ((BNODE *) NULL);
}

/*
 * doif - one-arm if statement
 */
BNODE *doif(BNODE *e, int m, BNODE *s) {
    TNODE *bl = (TNODE *) tnode();
    bl->t_op = TO_BLABEL;
    bl->val.ln.t_con = e->back.b_true->b_label - 1;

    TNODE *l = (TNODE *) tnode();
    l->t_op = TO_LABEL;
    l->val.ln.t_con = m;

    TNODE *equ = (TNODE *) tnode();
    equ->t_op = TO_EQU;
    equ->val.in.t_left = bl;
    equ->val.in.t_right = l;

    emittree(equ);

    /*
    BNODE *cp = e;
    while (cp) {
        printf("doif (b_true): e label = %d\n", cp->b_label);
        cp = cp->back.b_true;
    }
    cp = e;
    while (cp) {
        printf("doif (b_false): e label = %d\n", cp->b_label);
        cp = cp->b_false;
    }
    cp = s;
    while (cp) {
        printf("doif: s label = %d\n", cp->b_label);
        cp = cp->back.b_link;
    }
    printf("doif m: %d\n", m);
    printf("patching...\n");
    */

    backpatch(e->back.b_true, m);

    BNODE *b = (BNODE *) bnode();
    b->b_label = m;
    if (s) { b->back.b_link = (BNODE *) merge(e->b_false, s->back.b_link); }
    else { b->back.b_link = (BNODE *) merge(e->b_false, (BNODE *) NULL); }

    /*
    cp = e;
    while (cp) {
        printf("doif (b_true): e label = %d\n", cp->b_label);
        cp = cp->back.b_true;
    }
    cp = e;
    while (cp) {
        printf("doif (b_false): e label = %d\n", cp->b_label);
        cp = cp->b_false;
    }
    cp = s;
    while (cp) {
        printf("doif: s label = %d\n", cp->b_label);
        cp = cp->back.b_link;
    }
    printf("doif m: %d\n", m);
    */

    return b;
}

/*
 * doifelse - if then else statement
 */
BNODE *doifelse(BNODE *e, int m1, BNODE *s1, BNODE *n, int m2, BNODE *s2) {
    TNODE *bl1 = (TNODE *) tnode();
    bl1->t_op = TO_BLABEL;
    bl1->val.ln.t_con = e->back.b_true->b_label - 1;
    TNODE *l1 = (TNODE *) tnode();
    l1->t_op = TO_LABEL;
    l1->val.ln.t_con = m1;
    TNODE *equ1 = (TNODE *) tnode();
    equ1->t_op = TO_EQU;
    equ1->val.in.t_left = bl1;
    equ1->val.in.t_right = l1;
    emittree(equ1);

    TNODE *bl2 = (TNODE *) tnode();
    bl2->t_op = TO_BLABEL;
    bl2->val.ln.t_con = e->b_false->b_label - 1;
    TNODE *l2 = (TNODE *) tnode();
    l2->t_op = TO_LABEL;
    l2->val.ln.t_con = m2;
    TNODE *equ2 = (TNODE *) tnode();
    equ2->t_op = TO_EQU;
    equ2->val.in.t_left = bl2;
    equ2->val.in.t_right = l2;
    emittree(equ2);

    /*
    BNODE *cp = e;
    while (cp) {
        printf("doifelse (b_true): e label = %d\n", cp->b_label);
        cp = cp->back.b_true;
    }
    cp = e;
    while (cp) {
        printf("doifelse (b_false): e label = %d\n", cp->b_label);
        cp = cp->b_false;
    }
    cp = s1;
    while (cp) {
        printf("doifelse: s1 label = %d\n", cp->b_label);
        cp = cp->back.b_link;
    }
    cp = s2;
    while (cp) {
        printf("doifelse: s2 label = %d\n", cp->b_label);
        cp = cp->back.b_link;
    }
    printf("doifelse: m1 = %d\n", m1);
    printf("doifelse: m2 = %d\n", m2);
    printf("patching...\n");
    */

    backpatch(e->back.b_true, m1);
    backpatch(e->b_false, m2);

    BNODE *temp;
    if (s1) { temp = (BNODE *) merge(s1->back.b_link, n->back.b_link); }
    else { temp = (BNODE *) merge((BNODE *) NULL, n->back.b_link); }
    BNODE *ret = (BNODE *) bnode();
    ret->b_label = labelno;
    if (s2) { ret->back.b_link = (BNODE *) merge(temp, s2->back.b_link); }
    else { ret->back.b_link = (BNODE *) merge(temp, (BNODE *) NULL); }

    /*
    cp = e;
    while (cp) {
        printf("doifelse (b_true): e label = %d\n", cp->b_label);
        cp = cp->back.b_true;
    }
    cp = e;
    while (cp) {
        printf("doifelse (b_false): e label = %d\n", cp->b_label);
        cp = cp->b_false;
    }
    cp = s1;
    while (cp) {
        printf("doifelse: s1 label = %d\n", cp->b_label);
        cp = cp->back.b_link;
    }
    cp = s2;
    while (cp) {
        printf("doifelse: s2 label = %d\n", cp->b_label);
        cp = cp->back.b_link;
    }
    printf("doifelse: m1 = %d\n", m1);
    printf("doifelse: m2 = %d\n", m2);
    */

    return ret;
}

/*
 * doret - return statement
 */
BNODE *doret(TNODE *e) {
    BNODE *ret = (BNODE *) bnode();
    ret->b_label = labelno;

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
    if (sl) {
        BNODE *last = sl;
        while (last->back.b_link) last = last->back.b_link;
        last->back.b_link = s;
        
        if (last->b_label > 1) {
            TNODE *bl = (TNODE *) tnode();
            bl->t_op = TO_BLABEL;
            bl->val.ln.t_con = last->b_label - 1;
            TNODE *l = (TNODE *) tnode();
            l->t_op = TO_LABEL;
            l->val.ln.t_con = m;
            TNODE *equ = (TNODE *) tnode();
            equ->t_op = TO_EQU;
            equ->val.in.t_left = bl;
            equ->val.in.t_right = l;
            emittree(equ);
        }
    } else {
        sl = (BNODE *) bnode();
        sl->b_label = m;
        sl->back.b_link = s;
    }
    /*
    if (s) {
        TNODE *bl = (TNODE *) tnode();
        bl->t_op = TO_BLABEL;
        bl->val.ln.t_con = 100;
        TNODE *l = (TNODE *) tnode();
        l->t_op = TO_LABEL;
        l->val.ln.t_con = 100;

        TNODE *equ = (TNODE *) tnode();
        equ->t_op = TO_EQU;
        equ->val.in.t_left = bl;
        equ->val.in.t_right = l;
        
        printf("# Generated from dostmts\n");
        emittree(equ);
    }
    */

    return sl;
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

    // TODO: double align
    TNODE *arg_size = con(aoff);
    TNODE *loc_size = con(loff);
    loff = aoff = 0;

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
    labelno++;
    TNODE *nd = (TNODE *) tnode();
    nd->t_op = TO_LABEL;
    nd->t_mode = 0;
    nd->val.ln.t_con = labelno;
    emittree(nd);
    return labelno;
}

/*
 * n - generate goto and return backpatch pointer
 */
BNODE *n(void) {
    labelno++;
    TNODE *nd = (TNODE *) tnode();
    nd->t_op = TO_BLABEL;
    nd->t_mode = 0;
    nd->val.ln.t_con = labelno;

    TNODE *jmp = (TNODE *) tnode();
    jmp->t_op = TO_JMP;
    jmp->t_mode = T_INT;
    jmp->val.in.t_left = nd;

    emittree(jmp);

    BNODE *ret = (BNODE *) bnode();
    ret->b_label = labelno;
    return ret;
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
    TNODE *cmp_node = (TNODE *) tnode();
    cmp_node->t_op = op;
    // TODO: cast
    cmp_node->t_mode = x->t_mode;
    cmp_node->val.in.t_left = x;
    cmp_node->val.in.t_right = y;

    TNODE *bl_node = (TNODE *) tnode();
    bl_node->t_op = TO_BLABEL;
    bl_node->t_mode = 0;
    bl_node->val.ln.t_con = ++labelno;

    TNODE *jmpt_node = (TNODE *) tnode();
    jmpt_node->t_op = TO_JMPT;
    jmpt_node->t_mode = T_INT;
    jmpt_node->val.in.t_left = bl_node;
    jmpt_node->val.in.t_right = cmp_node;

    emittree(jmpt_node);

    TNODE *jmp = (TNODE *) tnode();
    jmp->t_op = TO_JMP;
    jmp->t_mode = T_INT;

    TNODE *bl = (TNODE *) tnode();
    bl->t_op = TO_BLABEL;
    bl->t_mode = 0;
    bl->val.ln.t_con = ++labelno;

    jmp->val.in.t_left = bl;
    emittree(jmp);

    BNODE *truelist = (BNODE *) bnode();
    truelist->b_label = labelno;
    BNODE *falselist = (BNODE *) bnode();
    falselist->b_label = labelno + 1;

    BNODE *b = (BNODE *) bnode();
    b->back.b_true = truelist;
    b->b_false = falselist;
    b->b_label = 0;

    return b;
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
