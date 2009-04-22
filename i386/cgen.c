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
    if (!p) {
        fprintf(stderr, "warning: cgen call on null TNODE\n");
        return;
    }
    switch (p->t_op) {
        case TO_EQU:
            printf("\t.");
            if (p->val.in.t_left->t_op == TO_BLABEL)
                printf("B");
            else if (p->val.in.t_left->t_op == TO_LABEL)
                printf("L");
            printf("%d=.", p->val.in.t_left->val.ln.t_con);
            if (p->val.in.t_right->t_op == TO_BLABEL)
                printf("B");
            else if (p->val.in.t_right->t_op == TO_LABEL)
                printf("L");
            printf("%d\n", p->val.in.t_right->val.ln.t_con);
            break;
        case TO_CMPEQ:
            right(p);
            left(p);
            printf("\tmovl\t$0,%%ecx\n");
            printf("\tpopl\t%%eax\n");
            printf("\tpopl\t%%edx\n");
            printf("\tcmpl\t%%eax,%%edx\n");
            L_number++;
            printf("\tjne\tL0%d\n", L_number);
            printf("\tincl\t%%ecx\n");
            printf("L0%d:\n", L_number);
            printf("\tpushl\t%%ecx\n");
            printf("\tpopl\t%%eax\n");
            printf("\tcmpl\t$0,%%eax\n");
            break;
        case TO_CMPGT:
            right(p);
            left(p);
            printf("\tmovl\t$0,%%ecx\n");
            printf("\tpopl\t%%edx\n");
            printf("\tpopl\t%%eax\n");
            printf("\tcmpl\t%%eax,%%edx\n");
            L_number++;
            printf("\tjle\tL0%d\n", L_number);
            printf("\tincl\t%%ecx\n");
            printf("L0%d:\n", L_number);
            printf("\tpushl\t%%ecx\n");
            printf("\tpopl\t%%eax\n");
            printf("\tcmpl\t$0,%%eax\n");
            break;
        case TO_CMPLT:
            right(p);
            left(p);
            printf("\tmovl\t$0,%%ecx\n");
            printf("\tpopl\t%%edx\n");
            printf("\tpopl\t%%eax\n");
            printf("\tcmpl\t%%eax,%%edx\n");
            L_number++;
            printf("\tjge\tL0%d\n", L_number);
            printf("\tincl\t%%ecx\n");
            printf("L0%d:\n", L_number);
            printf("\tpushl\t%%ecx\n");
            printf("\tpopl\t%%eax\n");
            printf("\tcmpl\t$0,%%eax\n");
            break;
        case TO_JMPT:
            right(p);
            printf("\tjne\t.B%d\n", p->val.in.t_left->val.ln.t_con);
            break;
        case TO_JMP:
            if (p->val.in.t_left->t_op == TO_BLABEL)
                printf("\tjmp\t.B%d\n", p->val.in.t_left->val.ln.t_con);
            else if (p->val.in.t_left->t_op == TO_LABEL)
                printf("\tjmp\t.L%d\n", p->val.in.t_left->val.ln.t_con);
            break;
        case TO_LABEL:
            printf(".L%d:\n", p->val.ln.t_con);
            break;
        case TO_CAST:
            if (p->t_mode & T_INT && p->val.in.t_left->t_mode & T_DOUBLE) {
                if (p->val.in.t_left->t_op == TO_CON) {
                } else {
                    left(p->val.in.t_left);
                    pop("eax");
                    printf("\tfistp\t(%%eax)\n");
                }
            } else if (p->t_mode & T_DOUBLE && p->val.in.t_left->t_mode & T_INT) {
                if (p->val.in.t_left->t_op == TO_CON) {
                    printf("\tpushl\t$%d\n", p->val.in.t_left->val.ln.t_con);
                    printf("\tfild\t(%%esp)\n");
                    pop("eax");
                } else {
                    left(p->val.in.t_left);
                    pop("eax");
                    printf("\tfldl\t(%%eax)\n");
                }
            }
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
            printf("L0%d:\t.asciz\t\"%s\"\n", L_number, p->val.ln.t_str);
            instr(".text");
            printf("\tleal\tL0%d,%%eax\n", L_number);
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
            if (p->t_mode & T_INT) {
                printf("\tmovl\t(%%eax),%%edx\n");
                push("edx");
            } else if (p->t_mode & T_DOUBLE) {
                printf("\tfldl\t(%%eax)\n");
            }
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
        case TO_RS:
            left(p);
            right(p);
            pop("ecx");
            pop("eax");
            printf("\tshr\t%%eax\n");
            push("eax");
            break;
        case TO_LS:
            left(p);
            right(p);
            pop("ecx");
            pop("eax");
            printf("\tshl\t%%eax\n");
            push("eax");
            break;
        case TO_OR:
            left(p);
            right(p);
            pop("eax");
            pop("edx");
            printf("\tor\t%%eax,%%edx\n");
            push("edx");
            break;
        case TO_XOR:
            left(p);
            right(p);
            pop("eax");
            pop("edx");
            printf("\txor\t%%eax,%%edx\n");
            push("edx");
            break;
        case TO_AND:
            left(p);
            right(p);
            pop("eax");
            pop("edx");
            printf("\tand\t%%eax,%%edx\n");
            push("edx");
            break;
        case TO_NEGATE:
            left(p);
            pop("eax");
            printf("\tneg\t%%eax\n");
            push("eax");
            break;
        case TO_COMP:
            left(p);
            pop("eax");
            printf("\tnot\t%%eax\n");
            push("eax");
            break;
        case TO_NAME:
            if (p->val.ln.t_id->i_blevel == 2) {
                /* Globals */
                printf("\tleal\t%s,%%eax\n", p->val.ln.t_id->i_name);
            } else if (p->val.ln.t_id->i_scope > 0) {
                /* Arguments */
                printf("\tleal\t%d(%%ebp),%%eax\n", p->val.ln.t_id->i_offset + 4);
            } else if (p->val.ln.t_id->i_blevel > 2) {
                /* Locals */
                printf("\tleal\t-%d(%%ebp),%%eax\n", p->val.ln.t_id->i_offset);
            }
            // TODO: check this
            push("eax");
            /*
            if (p->t_mode & T_INT) {
                push("eax");
            } else if (p->t_mode & T_DOUBLE) {
                printf("\tfldl\t(%%eax)\n");
            }
            */
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
            fprintf(stderr, "warning: cgen not implemented for '%s'\n",
                opdope[p->t_op].opstring);
    }
    return 0;
}

/*
 * initcgen - initialize code generation
 */
void initcgen() {
}
