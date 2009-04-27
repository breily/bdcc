#include <stdio.h>
#include <stdlib.h>
#include "cc.h"
#include "ops.h"
#include "tree.h"

int aoff    = 0;	/* offset counter for arguments */
int loff    = 0;	/* offset counter for locals */
int labelno = 0;	/* global for generating labels */

extern int level;

/* prototypes */
void emittree(TNODE *);
void yyerror(char *);

IDENT *install(char *, int); 
IDENT *lookup(char *, int); 

BNODE *rel(int, TNODE *, TNODE *);

int equs[1024];
int equs_n = 0;

int should_gen(int x) {
    int i;
    for (i = 0; i < equs_n; i++) {
        if (equs[i] == x) return 0;
    }
    return 1;
}

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
    BNODE *tmp = e1;
    int lbl1 = -1;
    while (tmp) {
    //    printf("e1->back.b_true = %d\n", tmp->b_label);
        lbl1 = tmp->b_label;
        tmp = tmp->back.b_true;
    }
    //printf("m = %d\n", m);
    if (lbl1 != -1 && should_gen(lbl1 - 1)) {
        TNODE *bl_eq1 = (TNODE *) tnode();
        bl_eq1->t_op = TO_BLABEL;
        bl_eq1->val.ln.t_con = lbl1 - 1;
        TNODE *l_eq1 = (TNODE *) tnode();
        l_eq1->t_op = TO_LABEL;
        l_eq1->val.ln.t_con = m;
        TNODE *eq1 = (TNODE *) tnode();
        eq1->t_op = TO_EQU;
        eq1->val.in.t_left = bl_eq1;
        eq1->val.in.t_right = l_eq1;
        emittree(eq1);

        equs[equs_n] = lbl1 - 1;
        equs_n++;
    }

    tmp = e1;
    int lbl2 = -1;
    while (tmp) {
        //printf("e1->b_false = %d\n", tmp->b_label);
        lbl2 = tmp->b_label;
        tmp = tmp->b_false;
    }
    int target = -1;
    tmp = e2;
    while (tmp) {
        //printf("e2->back.b_link = %d\n", tmp->b_label);
        target = tmp->b_label;
        tmp = tmp->back.b_link;
    }
    if (lbl2 != -1 && target != -1 && should_gen(lbl2 - 1)) {
        TNODE *bl_eq1 = (TNODE *) tnode();
        bl_eq1->t_op = TO_BLABEL;
        bl_eq1->val.ln.t_con = lbl2 - 1;
        TNODE *l_eq1 = (TNODE *) tnode();
        l_eq1->t_op = TO_LABEL;
        l_eq1->val.ln.t_con = target + 1;
        TNODE *eq1 = (TNODE *) tnode();
        eq1->t_op = TO_EQU;
        eq1->val.in.t_left = bl_eq1;
        eq1->val.in.t_right = l_eq1;
        emittree(eq1);

        equs[equs_n] = lbl2 - 1;
        equs_n++;
    }

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
    TNODE *zero = (TNODE *) tnode();
    zero->t_op = TO_CON;
    zero->t_mode = T_INT;
    zero->val.ln.t_con = 0;

    return (BNODE *) rel(TO_CMPNE, e, zero);
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
    if (scope == PARAM) {
        /* Arguments */
        if (type & T_DOUBLE) p->i_width = p->i_width * 2;
        aoff += p->i_width * 4;
        p->i_offset = aoff;
        p->i_type |= type;
        p->i_scope = scope;
        p->i_defined = 1;
        p->i_blevel = level;
        return p;
    } else if (level == 2) {
        if (p == NULL) {
            /* Globals */
            p = install(name, level);
            p->i_type |= type;
            p->i_width = width;
            p->i_blevel = level;
            p->i_scope = scope;
            p->i_defined = 0;
        } else {
            /* Globals */
            p->i_type |= type;
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
            /* Locals and Arguments */
            p = install(name, level);
            p->i_type |= type;
            p->i_width = width;
            p->i_blevel = level;
            p->i_scope = scope;
            p->i_defined = 0;
        } else {
            /* Locals */
            if (type & T_DOUBLE) p->i_width = p->i_width * 2;
            p->i_type |= type;
            p->i_defined = 1;
            loff += p->i_width * 4;
            p->i_offset = loff;
        }
        return p;
    }
}

/*
 * dofor - for statement
 */
BNODE *dofor(TNODE *e1, int m1, BNODE *e2, int m2, TNODE *e3, BNODE *n, int m3, BNODE *s) {
    int lbl1, lbl2;
    BNODE *cp = e2;
    while (cp->back.b_true) cp = cp->back.b_true;
    lbl1 = cp->b_label - 1;

    cp = e2;
    while (cp->b_false) cp = cp->b_false;
    lbl2 = cp->b_label + 1;

    if (should_gen(lbl1)) {
    TNODE *bl_eq1 = (TNODE *) tnode();
    bl_eq1->t_op = TO_BLABEL;
    bl_eq1->val.ln.t_con = lbl1;
    TNODE *l_eq1 = (TNODE *) tnode();
    l_eq1->t_op = TO_LABEL;
    l_eq1->val.ln.t_con = m3;
    TNODE *eq1 = (TNODE *) tnode();
    eq1->t_op = TO_EQU;
    eq1->val.in.t_left = bl_eq1;
    eq1->val.in.t_right = l_eq1;
    emittree(eq1);

    equs[equs_n] = lbl1;
    equs_n++;
    }

    if (should_gen(lbl2)) {
    TNODE *bl_eq2 = (TNODE *) tnode();
    bl_eq2->t_op = TO_BLABEL;
    bl_eq2->val.ln.t_con = lbl2;
    TNODE *l_eq2 = (TNODE *) tnode();
    l_eq2->t_op = TO_LABEL;
    l_eq2->val.ln.t_con = m1;
    TNODE *eq2 = (TNODE *) tnode();
    eq2->t_op = TO_EQU;
    eq2->val.in.t_left = bl_eq2;
    eq2->val.in.t_right = l_eq2;
    emittree(eq2);

    equs[equs_n] = lbl2;
    equs_n++;
    }

    TNODE *jmp_l = (TNODE *) tnode();
    jmp_l->t_op = TO_LABEL;
    jmp_l->val.ln.t_con = m2;
    TNODE *jmp = (TNODE *) tnode();
    jmp->t_op = TO_JMP;
    jmp->t_mode = T_INT;
    jmp->val.in.t_left = jmp_l;
    emittree(jmp);

    if (s) backpatch(s->back.b_link, m2);
    backpatch(e2->back.b_true, m2);

    BNODE *ret = (BNODE *) bnode();
    ret->b_label = labelno;
    ret->back.b_link = e2->b_false;

    return ret;
}

/*
 * doif - one-arm if statement
 */
BNODE *doif(BNODE *e, int m, BNODE *s) {
    if (should_gen(e->back.b_true->b_label - 1)) {
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

    equs[equs_n] = e->back.b_true->b_label - 1;
    equs_n++;
    }

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
    if (should_gen(e->back.b_true->b_label - 1)) {
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

    equs[equs_n] = e->back.b_true->b_label - 1;
    equs_n++;
    }

    BNODE *tmp = e;
    while (tmp) {
        //printf("e->b_false = %d\n", tmp->b_label);
        tmp = tmp->b_false;
    }
    //printf("m2 = %d\n", m2);

    if (should_gen(e->b_label)) {
    TNODE *blx = (TNODE *) tnode();
    blx->t_op = TO_BLABEL;
    blx->val.ln.t_con = e->b_label;
    TNODE *lx = (TNODE *) tnode();
    lx->t_op = TO_LABEL;
    lx->val.ln.t_con = m2;
    TNODE *equx = (TNODE *) tnode();
    equx->t_op = TO_EQU;
    equx->val.in.t_left = blx;
    equx->val.in.t_right = lx;
    emittree(equx);
    
    equs[equs_n] = e->b_label;
    equs_n++;
    }

    if (should_gen(e->b_false->b_label - 1)) {
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
    
    equs[equs_n] = e->b_false->b_label - 1;
    equs_n++;
    }

    // New
    int lblx;
    if (s2) lblx = s2->b_label;
    else lblx = labelno + 1;
    if (should_gen(n->b_label)) {
    TNODE *nn1 = (TNODE *) tnode();
    nn1->t_op = TO_BLABEL;
    nn1->val.ln.t_con = n->b_label;
    TNODE *nn2 = (TNODE *) tnode();
    nn2->t_op = TO_LABEL;
    nn2->val.ln.t_con = lblx;
    TNODE *eq3 = (TNODE *) tnode();
    eq3->t_op = TO_EQU;
    eq3->val.in.t_left = nn1;
    eq3->val.in.t_right = nn2;
    emittree(eq3);

    equs[equs_n] = n->b_label;
    equs_n++;
    }
    // End New

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
            if (should_gen(last->b_label - 1)) {
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

                equs[equs_n] = last->b_label - 1;
                equs_n++;
            }
        }
    } else {
        sl = (BNODE *) bnode();
        sl->b_label = m;
        sl->back.b_link = s;
    }
    return sl;
}

/*
 * dowhile - while statement
 */
BNODE *dowhile(int m1, BNODE *e, int m2, BNODE *s) {
    if (should_gen(m1 + 1)) {
    TNODE *bl = (TNODE *) tnode();
    bl->t_op = TO_BLABEL;
    bl->val.ln.t_con = m1 + 1;
    TNODE *l0 = (TNODE *) tnode();
    l0->t_op = TO_LABEL;
    l0->val.ln.t_con = m2;
    TNODE *equ = (TNODE *) tnode();
    equ->t_op = TO_EQU;
    equ->val.in.t_left = bl;
    equ->val.in.t_right = l0;
    emittree(equ);

    equs[equs_n] = m1 + 1;
    equs_n++;
    }

    TNODE *l1 = (TNODE *) tnode();
    l1->t_op = TO_LABEL;
    l1->val.ln.t_con = m1;
    TNODE *j = (TNODE *) tnode();
    j->t_op = TO_JMP;
    j->t_mode = T_INT;
    j->val.in.t_left = l1;
    emittree(j);

    backpatch(s->back.b_link, m1);
    backpatch(e->back.b_true, m2);
    
    BNODE *b = (BNODE *) bnode();
    b->b_label = labelno;
    b->back.b_link = e->b_false;

    return b;
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
    fprintf(stderr, "warning: function arguments and locals are not double aligned\n");
    TNODE *arg_size = con(aoff);
    TNODE *loc_size = con(loff);

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
    // TODO: check if already defined
    IDENT *ret = install(id, level);
    ret->i_type = t | T_PROC | T_PTR;
    ret->i_name = id;
    ret->i_blevel = level;
    ret->i_defined = 0;
    ret->i_scope = GLOBAL;
    enterblock();
    loff = aoff = 0;
    return ret;
}

/*
 * ftail - end of function body
 */
void ftail(IDENT *p, BNODE *s, int m) {
    BNODE *last = s;
    while (last->back.b_link) last = last->back.b_link;
    if (last->b_label > 1) {
        if (should_gen(last->b_label - 1)) {
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

            equs[equs_n] = last->b_label - 1;
            equs_n++;
        }
    }

    TNODE *retval = (TNODE *) tnode();
    retval->t_op = TO_CON;
    retval->t_mode = T_INT;
    retval->val.ln.t_con = 0;
    doret(retval);

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

    TNODE *shift_val = (TNODE *) tnode();
    shift_val->t_op = TO_CON;
    shift_val->t_mode = T_INT;
    if (x->t_mode & T_INT) shift_val->val.ln.t_con = 2;
    else if (x->t_mode & T_DOUBLE) shift_val->val.ln.t_con = 3;

    TNODE *ls = (TNODE *) tnode();
    ls->t_op = TO_LS;
    ls->t_mode = T_INT;
    ls->val.in.t_left = i;
    ls->val.in.t_right = shift_val;

    TNODE *offset = (TNODE *) tnode();
    offset->t_op = TO_PLUS;
    offset->t_mode = x->t_mode;
    offset->val.in.t_left = x;
    offset->val.in.t_right = ls;

    return offset;
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
   fprintf(stderr, "op1 not implemented\n");
   return ((TNODE *) NULL);
}

/*
 * op_arith - arithmetic operators
 */
TNODE *op_arith(int op, TNODE *x, TNODE *y) {
    TNODE *ret = (TNODE *) tnode();
    ret->t_op = op;
    ret->t_mode = promote(x, y);
    if (ret->t_mode & T_DOUBLE && x->t_mode & T_INT) {
        TNODE *nl = (TNODE *) tnode();
        nl->t_op = TO_CAST;
        nl->t_mode = T_DOUBLE;
        nl->val.in.t_left = x;
        ret->val.in.t_left = nl;
    } else {
        ret->val.in.t_left = x;
    }
    if (ret->t_mode & T_DOUBLE && y->t_mode & T_INT) {
        TNODE *nr = (TNODE *) tnode();
        nr->t_op = TO_CAST;
        nr->t_mode = T_DOUBLE;
        nr->val.in.t_left = y;
        ret->val.in.t_left = nr;
    } else {
        ret->val.in.t_right = y;
    }
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
    fprintf(stderr, "warning: no casting is done for rel ops\n");
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
   if (!(x->t_mode & y->t_mode)) {
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
   fprintf(stderr, "setaug not implemented\n");
   return ((TNODE *) NULL);
}

/*
 * setaug2 - augmented logical assignment operators
 */
TNODE *setaug2(int op, TNODE *x, TNODE *y) {
   fprintf(stderr, "setaug not implemented\n");
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
