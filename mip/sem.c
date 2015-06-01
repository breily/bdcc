#define debug 1

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
#if debug
    BNODE *a;
    fprintf(stdout, "# --> start ccand info\n");
    if (e1) {
        printf("# e1->b_label = %d\n", e1->b_label);
        a = e1->back.b_true;
        while (a) {
            fprintf(stdout, "# e1->back.b_true = %d\n", a->b_label);
            a = a->back.b_link;
        }
        a = e1->b_false;
        while (a) {
            fprintf(stdout, "# e1->b_false = %d\n", a->b_label);
            a = a->back.b_true;
        }
    }
    fprintf(stdout,  "# m = %d\n", m);
    if (e2) {
        printf("# e2->b_label = %d\n", e2->b_label);
        a = e2->back.b_true;
        while (a) {
            fprintf(stdout, "# e2->back.b_true = %d\n", a->b_label);
            a = a->back.b_link;
        }
        a = e2->b_false;
        while (a) {
            fprintf(stdout, "# e2->b_false = %d\n", a->b_label);
            a = a->back.b_link;
        }
    }
    fprintf(stdout, "# labelno = %d\n", labelno);
    fprintf(stdout, "# --> end ccand info\n");
#endif

    BNODE *cp = e1->back.b_true;
    while (cp) {
        if (should_gen(cp->b_label - 1)) {
            TNODE *bl_eq1 = (TNODE *) tnode();
            bl_eq1->t_op = TO_BLABEL;
            bl_eq1->val.ln.t_con = cp->b_label - 1;
            TNODE *l_eq1 = (TNODE *) tnode();
            l_eq1->t_op = TO_LABEL;
            l_eq1->val.ln.t_con = m;
            TNODE *eq1 = (TNODE *) tnode();
            eq1->t_op = TO_EQU;
            eq1->val.in.t_left = bl_eq1;
            eq1->val.in.t_right = l_eq1;
            emittree(eq1);

            equs[equs_n] = cp->b_label - 1;
            equs_n++;
        }
        cp = cp->back.b_link;
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
#if debug
    BNODE *a;
    fprintf(stdout, "# --> start ccor info\n");
    if (e1) {
        printf("# e1->b_label = %d\n", e1->b_label);
        a = e1->back.b_true;
        while (a) {
            fprintf(stdout, "# e1->back.b_true = %d\n", a->b_label);
            a = a->back.b_link;
        }
        a = e1->b_false;
        while (a) {
            fprintf(stdout, "# e1->b_false = %d\n", a->b_label);
            a = a->back.b_link;
        }
    }
    fprintf(stdout,  "# m = %d\n", m);
    if (e2) {
        printf("# e2->b_label = %d\n", e2->b_label);
        a = e2->back.b_true;
        while (a) {
            fprintf(stdout, "# e2->back.b_true = %d\n", a->b_label);
            a = a->back.b_link;
        }
        a = e2->b_false;
        while (a) {
            fprintf(stdout, "# e2->b_false = %d\n", a->b_label);
            a = a->back.b_link;
        }
    }
    fprintf(stdout, "# labelno = %d\n", labelno);
    fprintf(stdout, "# --> end ccor info\n");
#endif

    BNODE *cp = e1->b_false;
    while (cp) {
        if (should_gen(cp->b_label - 1)) {
            TNODE *bl_eq1 = (TNODE *) tnode();
            bl_eq1->t_op = TO_BLABEL;
            bl_eq1->val.ln.t_con = cp->b_label - 1;
            TNODE *l_eq1 = (TNODE *) tnode();
            l_eq1->t_op = TO_LABEL;
            l_eq1->val.ln.t_con = m;
            TNODE *eq1 = (TNODE *) tnode();
            eq1->t_op = TO_EQU;
            eq1->val.in.t_left = bl_eq1;
            eq1->val.in.t_right = l_eq1;
            emittree(eq1);

            equs[equs_n] = cp->b_label - 1;
            equs_n++;
        }
        cp = cp->back.b_link;
    }
    
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
#if debug
    BNODE *a;
    printf("# --> start dofor info\n");
    printf("# m1 = %d\n", m1);
    if (e2) {
        printf("# e2->b_label = %d\n", e2->b_label);
        a = e2->back.b_true;
        while (a) {
            printf("# e2->back.b_true = %d\n", a->b_label);
            a = a->back.b_link;
        }
        a = e2->b_false;
        while (a) {
            printf("# e2->b_false = %d\n", a->b_label);
            a = a->back.b_link;
        }
    }
    printf("# m2 = %d\n", m2);
    if (n) {
        printf("# n->b_label = %d\n", n->b_label);
        a = n->back.b_link;
        while (a) {
            printf("# n->back.b_link = %d\n", a->b_label);
            a = a->back.b_link;
        }
    }
    printf("# m3 = %d\n", m3);
    if (s) {
        printf("# s->b_label = %d\n", s->b_label);
        a = s->back.b_link;
        while (a) {
            printf("# s->back.b_link = %d\n", a->b_label);
            a = a->back.b_link;
        }
    }
    printf("# labelno = %d\n", labelno);
#endif

    BNODE *cp = e2->back.b_true;
    while (cp) {
        if (should_gen(cp->b_label - 1)) {
            TNODE *tbl = (TNODE *) tnode();
            tbl->t_op = TO_BLABEL;
            tbl->val.ln.t_con = cp->b_label - 1;
            TNODE *tl = (TNODE *) tnode();
            tl->t_op = TO_LABEL;
            tl->val.ln.t_con = m3;
            TNODE *tequ = (TNODE *) tnode();
            tequ->t_op = TO_EQU;
            tequ->val.in.t_left = tbl;
            tequ->val.in.t_right = tl;
            emittree(tequ);

            equs[equs_n] = cp->b_label - 1;
            equs_n++;
        }
        cp = cp->back.b_link;
    }

    TNODE *jmp_l = (TNODE *) tnode();
    jmp_l->t_op = TO_LABEL;
    jmp_l->val.ln.t_con = m2;
    TNODE *jmp = (TNODE *) tnode();
    jmp->t_op = TO_JMP;
    jmp->t_mode = T_INT;
    jmp->val.in.t_left = jmp_l;
    emittree(jmp);

    m();

    cp = e2->b_false;
    while (cp) {
        if (should_gen(cp->b_label - 1)) {
            TNODE *fbl = (TNODE *) tnode();
            fbl->t_op = TO_BLABEL;
            fbl->val.ln.t_con = cp->b_label - 1; // + 1 ?
            TNODE *fl = (TNODE *) tnode();
            fl->t_op = TO_LABEL;
            fl->val.ln.t_con = labelno;
            TNODE *fequ = (TNODE *) tnode();
            fequ->t_op = TO_EQU;
            fequ->val.in.t_left = fbl;
            fequ->val.in.t_right = fl;
            emittree(fequ);

            equs[equs_n] = cp->b_label - 1;
            equs_n++;
        }
        cp = cp->back.b_link;
    }

    while (n) {
        if (should_gen(n->b_label)) {
            TNODE *nbl = (TNODE *) tnode();
            nbl->t_op = TO_BLABEL;
            nbl->val.ln.t_con = n->b_label;
            TNODE *nl = (TNODE *) tnode();
            nl->t_op = TO_LABEL;
            nl->val.ln.t_con = m1;
            TNODE *nequ = (TNODE *) tnode();
            nequ->t_op = TO_EQU;
            nequ->val.in.t_left = nbl;
            nequ->val.in.t_right = nl;
            emittree(nequ);

            equs[equs_n] = n->b_label;
            equs_n++;
        }
        n = n->back.b_link;
    }

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
BNODE *doif(BNODE *e, int m1, BNODE *s) {
#if debug
    BNODE *a;
    fprintf(stdout, "# --> start doif info\n");
    if (e) {
        printf("# e->b_label = %d\n", e->b_label);
        a = e->back.b_true;
        while (a) {
            fprintf(stdout, "# e->back.b_true = %d\n", a->b_label);
            a = a->back.b_link;
        }
        a = e->b_false;
        while (a) {
            fprintf(stdout, "# e->b_false = %d\n", a->b_label);
            a = a->back.b_link;
        }
    }
    printf("# m = %d\n", m1);
    if (s) {
        printf("# s->b_label = %d\n", s->b_label);
        a = s->back.b_link;
        while (a) {
            fprintf(stdout, "# s->back.b_link = %d\n", a->b_label);
            a = a->back.b_link;
        }
    }
    printf("# labelno = %d\n", labelno);
    printf("# --> end doif info\n");
#endif

    BNODE *cp = e->back.b_true;
    while (cp) {
        if (should_gen(cp->b_label - 1)) {
            TNODE *bl = (TNODE *) tnode();
            bl->t_op = TO_BLABEL;
            bl->val.ln.t_con = cp->b_label - 1;
            TNODE *l = (TNODE *) tnode();
            l->t_op = TO_LABEL;
            l->val.ln.t_con = m1;
            TNODE *equ = (TNODE *) tnode();
            equ->t_op = TO_EQU;
            equ->val.in.t_left = bl;
            equ->val.in.t_right = l;
            emittree(equ);

            equs[equs_n] = cp->b_label - 1;
            equs_n++;
        }
        cp = cp->back.b_link;
    }

    m();

    cp = e->b_false;
    while (cp) {
        if (should_gen(cp->b_label - 1)) {
            TNODE *bl = (TNODE *) tnode();
            bl->t_op = TO_BLABEL;
            bl->val.ln.t_con = cp->b_label - 1;
            TNODE *l = (TNODE *) tnode();
            l->t_op = TO_LABEL;
            l->val.ln.t_con = labelno;
            TNODE *equ = (TNODE *) tnode();
            equ->t_op = TO_EQU;
            equ->val.in.t_left = bl;
            equ->val.in.t_right = l;
            emittree(equ);

            equs[equs_n] = cp->b_label - 1;
            equs_n++;
        }
        cp = cp->back.b_link;
    }

    backpatch(e->back.b_true, m1);
    BNODE *b = (BNODE *) bnode();
    b->b_label = m1;
    if (s) { b->back.b_link = (BNODE *) merge(e->b_false, s->back.b_link); }
    else { b->back.b_link = (BNODE *) merge(e->b_false, (BNODE *) NULL); }

    return b;
}

/*
 * doifelse - if then else statement
 */
BNODE *doifelse(BNODE *e, int m1, BNODE *s1, BNODE *n, int m2, BNODE *s2) {
#if debug
    BNODE *a;
    fprintf(stdout, "# --> start doifelse info\n");
    if (e) {
        printf("# e->b_label = %d\n", e->b_label);
        a = e->back.b_true;
        while (a) {
            fprintf(stdout, "# e->back.b_true = %d\n", a->b_label);
            a = a->back.b_link;
        }
        a = e->b_false;
        while (a) {
            fprintf(stdout, "# e->b_false = %d\n", a->b_label);
            a = a->back.b_link;
        }
    }
    printf("# m1 = %d\n", m1);
    if (s1) {
        printf("# s1->b_label = %d\n", s1->b_label);
        a = s1->back.b_link;
        while (a) {
            fprintf(stdout, "# s1->back.b_link = %d\n", a->b_label);
            a = a->back.b_link;
        }
    }
    if (n) {
        printf("# n->b_label = %d\n", n->b_label);
        a = n->back.b_link;
        while (a) {
            fprintf(stdout, "# n->back.b_link = %d\n", a->b_label);
            a = a->back.b_link;
        }
    }
    printf("# m2 = %d\n", m2);
    if (s2) {
        printf("# s2->b_label = %d\n", s2->b_label);
        a = s2->back.b_link;
        while (a) {
            fprintf(stdout, "# s2->back.b_link = %d\n", a->b_label);
            a = a->back.b_link;
        }
    }
    printf("# labelno = %d\n", labelno);
    printf("# --> end doifelse info\n");
#endif

    BNODE *f = e->back.b_true;
    while (f) {
        if (should_gen(f->b_label - 1)) {
        TNODE *bl1 = (TNODE *) tnode();
        bl1->t_op = TO_BLABEL;
        bl1->val.ln.t_con = f->b_label - 1;
        TNODE *l1 = (TNODE *) tnode();
        l1->t_op = TO_LABEL;
        l1->val.ln.t_con = m1;
        TNODE *equ1 = (TNODE *) tnode();
        equ1->t_op = TO_EQU;
        equ1->val.in.t_left = bl1;
        equ1->val.in.t_right = l1;
        emittree(equ1);

        equs[equs_n] = f->b_label - 1;
        equs_n++;
        }
        f = f->back.b_link;
    }

    BNODE *cp = e->b_false;
    while (cp) {
        if (should_gen(cp->b_label - 1)) {
            TNODE *blx = (TNODE *) tnode();
            blx->t_op = TO_BLABEL;
            blx->val.ln.t_con = cp->b_label - 1;
            TNODE *lx = (TNODE *) tnode();
            lx->t_op = TO_LABEL;
            lx->val.ln.t_con = m2;
            TNODE *equx = (TNODE *) tnode();
            equx->t_op = TO_EQU;
            equx->val.in.t_left = blx;
            equx->val.in.t_right = lx;
            emittree(equx);
            
            equs[equs_n] = cp->b_label - 1;
            equs_n++;
        }
        cp = cp->back.b_link;
    }
    
    if (should_gen(m1 - 1)) {
        TNODE *blx = (TNODE *) tnode();
        blx->t_op = TO_BLABEL;
        blx->val.ln.t_con = m1 - 1;
        TNODE *lx = (TNODE *) tnode();
        lx->t_op = TO_LABEL;
        lx->val.ln.t_con = m2;
        TNODE *equx = (TNODE *) tnode();
        equx->t_op = TO_EQU;
        equx->val.in.t_left = blx;
        equx->val.in.t_right = lx;
        emittree(equx);
        
        equs[equs_n] = m1 - 1;
        equs_n++;
    }

    m();

    if (should_gen(n->b_label)) {
        TNODE *nn1 = (TNODE *) tnode();
        nn1->t_op = TO_BLABEL;
        nn1->val.ln.t_con = n->b_label;
        TNODE *nn2 = (TNODE *) tnode();
        nn2->t_op = TO_LABEL;
        nn2->val.ln.t_con = labelno;
        TNODE *eq3 = (TNODE *) tnode();
        eq3->t_op = TO_EQU;
        eq3->val.in.t_left = nn1;
        eq3->val.in.t_right = nn2;
        emittree(eq3);

        equs[equs_n] = n->b_label;
        equs_n++;
    }

    backpatch(e->back.b_true, m1);
    backpatch(e->b_false, m2);

    BNODE *temp;
    if (s1) { temp = (BNODE *) merge(s1->back.b_link, n->back.b_link); }
    else { temp = (BNODE *) merge((BNODE *) NULL, n->back.b_link); }
    BNODE *ret = (BNODE *) bnode();
    ret->b_label = labelno;
    if (s2) { ret->back.b_link = (BNODE *) merge(temp, s2->back.b_link); }
    else { ret->back.b_link = (BNODE *) merge(temp, (BNODE *) NULL); }

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
#if debug
    BNODE *a;
    printf("# --> start dowhile info\n");
    printf("# m1 = %d\n", m1);
    if (e) {
        printf("# e->b_label = %d\n", e->b_label);
        a = e->back.b_true;
        while (a) {
            printf("# e->back.b_true = %d\n", a->b_label);
            a = a->back.b_link;
        }
        a = e->b_false;
        while (a) {
            printf("# e->b_false = %d\n", a->b_label);
            a = a->back.b_link;
        }
    }
    printf("# m2 = %d\n", m2);
    if (s) {
        printf("# s->b_label = %d\n", s->b_label);
        a = s->back.b_link;
        while (a) { 
            printf("# s->back.b_link = %d\n", a->b_label);
            a = a->back.b_link;
        }
    }
    printf("# labelno = %d\n", labelno);
    printf("# <-- end dowhile info\n");
#endif
 
    BNODE *cp = e->back.b_true;
    while (cp) {
        if (should_gen(cp->b_label - 1)) {
            TNODE *bl = (TNODE *) tnode();
            bl->t_op = TO_BLABEL;
            bl->val.ln.t_con = cp->b_label - 1;
            TNODE *l0 = (TNODE *) tnode();
            l0->t_op = TO_LABEL;
            l0->val.ln.t_con = m2;
            TNODE *equ = (TNODE *) tnode();
            equ->t_op = TO_EQU;
            equ->val.in.t_left = bl;
            equ->val.in.t_right = l0;
            emittree(equ);

            equs[equs_n] = cp->b_label - 1;
            equs_n++;
        }
        cp = cp->back.b_link;
    }

    TNODE *l1 = (TNODE *) tnode();
    l1->t_op = TO_LABEL;
    l1->val.ln.t_con = m1;
    TNODE *j = (TNODE *) tnode();
    j->t_op = TO_JMP;
    j->t_mode = T_INT;
    j->val.in.t_left = l1;
    emittree(j);

    m();

    cp = e->b_false;
    while (cp) {
        if (should_gen(cp->b_label - 1)) {
            TNODE *bl = (TNODE *) tnode();
            bl->t_op = TO_BLABEL;
            bl->val.ln.t_con = cp->b_label - 1;
            TNODE *l0 = (TNODE *) tnode();
            l0->t_op = TO_LABEL;
            l0->val.ln.t_con = labelno;
            TNODE *equ = (TNODE *) tnode();
            equ->t_op = TO_EQU;
            equ->val.in.t_left = bl;
            equ->val.in.t_right = l0;
            emittree(equ);

            equs[equs_n] = cp->b_label - 1;
            equs_n++;
        }
        cp = cp->back.b_link;
    }

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
        ret->val.in.t_right = nr;
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
    cmp_node->t_mode = promote(x, y);
    if (cmp_node->t_mode & T_DOUBLE && x->t_mode & T_INT) {
        TNODE *nl = (TNODE *) tnode();
        nl->t_op = TO_CAST;
        nl->t_mode = cmp_node->t_mode;
        nl->val.in.t_left = x;
        cmp_node->val.in.t_left = nl;
    } else {
        cmp_node->val.in.t_left = x;
    }
    if (cmp_node->t_mode & T_DOUBLE && y->t_mode & T_INT) {
        TNODE *nr = (TNODE *) tnode();
        nr->t_op = TO_CAST;
        nr->t_mode = cmp_node->t_mode;
        nr->val.in.t_left = y;
        cmp_node->val.in.t_right = nr;
    } else {
        cmp_node->val.in.t_right = y;
    }

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
