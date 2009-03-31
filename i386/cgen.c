#include "cc.h"
#include <stdio.h>
#include <stdlib.h>
#include "ops.h"
#include "tree.h"

extern struct treeops opdope[];

int L_number = 0;

void left(TNODE *p)  { cgen(p->val.in.t_left); }
void right(TNODE *p) { cgen(p->val.in.t_right); }
void pop(char *reg)  { printf("\tpopl\t%%%s\n", reg); }
void push(char *reg) { printf("\tpushl\t%%%s\n", reg); }
void instr(char *i)  { printf("\t%s\n", i); }

/*
 * cgen - walk tree and emit code
 */
int cgen(TNODE *p) {
    if (!p) return;
    switch (p->t_op) {
        case TO_CAST:
            fprintf(stderr, "error: no cast\n");
            break;
        case TO_FEND:
            printf("\tmovl\t%%ebp,%%esp\n");
            pop("ebp");
            instr("ret");
            break;
        case TO_RET:
            left(p);
            pop("eax");
            printf("\tmovl\t%%ebp,%%esp\n");
            pop("ebp");
            instr("ret");
            break;
        case TO_STR:
            L_number++;
            instr(".data");
            printf("L%02d:\t.asciz\t\"%s\"\n", L_number, p->val.ln.t_str);
            instr(".text");
            printf("\tleal\tL%02d,%%eax\n", L_number);
            push("eax");
            break;
        case TO_LIST:
            right(p);
            left(p);
            break;
        case TO_CALL:
            right(p);
            printf("\tcall\t%s\n", p->val.in.t_left->val.ln.t_id->i_name);
            // TODO: Change 8 to number of args * 4
            printf("\taddl\t$8,%%esp\n");
            push("eax");
            break;
        case TO_DEREF:
            left(p);
            printf("\tpopl\t%%eax\n");
            printf("\tmovl\t(%%eax),%%edx\n");
            push("edx");
            break;
        case TO_MINUS:
            left(p);
            right(p);
            if ((p->t_mode & T_INT) || (p->t_mode & T_PTR)) {
                pop("eax");
                pop("edx");
                printf("\tsubl\t%%eax,%%edx\n");    // TODO: register order?
                push("edx");
            } else if (p->t_mode & T_DOUBLE) {
                instr("fsubp");
            }
            break;
        case TO_DIV:
            left(p);
            right(p);
            if (p->t_mode & T_INT) { 
                pop("ecx");
                pop("eax");
                instr("cltd");
                printf("\tidivl\t%%ecx\n");
                push("eax");
            } else if (p->t_mode & T_DOUBLE) {
                instr("fdivrp");
            }
            break;
        case TO_MOD:
            left(p);
            right(p);
            pop("ecx");
            pop("eax");
            instr("cltd");
            printf("\tidivl\t%%ecx\n");
            push("edx");
            break;
        case TO_PLUS:
            left(p);
            right(p);
            if ((p->t_mode & T_INT) || (p->t_mode & T_PTR)) {
                pop("eax");
                pop("edx");
                printf("\taddl\t%%edx,%%eax\n");
                push("eax");
            } else if (p->t_mode & T_DOUBLE) {
                instr("faddp");
                printf("\tfstl\t(%%eax)\n");
                push("eax");
            }
            break;
        case TO_MUL:
            left(p);
            right(p);
            if (p->t_mode & T_INT) {
                pop("eax");
                pop("edx");
                printf("\timul\t%%eax,%%edx\n");
                push("edx");
            } else if (p->t_mode & T_DOUBLE) {
                instr("fmulp");
            }
            break;
        case TO_NAME:
            printf("\tleal\t%s,%%eax\n", p->val.ln.t_id->i_name);
            if (p->t_mode & T_INT) {
                push("eax");
            } else if (p->t_mode & T_DOUBLE) {
                printf("\tfldl\t(%%eax)\n");
            }
            break;
        case TO_CON:
            printf("\tpushl\t$%d\n", p->val.ln.t_con);
            break;
        case TO_ASSIGN:
            left(p);
            right(p);
            if (p->t_mode & T_INT) {
                pop("eax");
                pop("edx");
                printf("\tmovl\t%%eax,(%%edx)\n");
                push("eax");
            } else if (p->t_mode & T_DOUBLE) {
                pop("eax");
                printf("\tfstl\t(%%eax)\n");
            }
            pop("eax");
            break;
        case TO_ALLOC:
            printf("\t.comm\t%s,%d\n",
                   p->val.in.t_left->val.ln.t_id->i_name,
                   p->val.in.t_right->val.ln.t_con
            );
            break;
        case TO_FUNC:
            instr(".text");
            printf("\t.globl\t%s\n", 
                   p->val.in.t_left->val.ln.t_id->i_name);
            printf("\t.type\t%s, @function\n", 
                   p->val.in.t_left->val.ln.t_id->i_name);
            printf("%s:\n",
                   p->val.in.t_left->val.ln.t_id->i_name);
            push("ebp");
            printf("\tmovl\t%%esp,%%ebp\n");
            printf("\tsubl\t$%d,%%esp\n",
                   8 + p->val.in.t_right->val.in.t_right->val.ln.t_con);
            break;
        default:
            fprintf(stderr, "\t# cgen not implemented for %s\n",
                opdope[p->t_op].opstring);
    }
    return 0;
}

/*
 * initcgen - initialize code generation
 */
void initcgen() {
}
