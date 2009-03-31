#define LEAFTYPE        (1)
#define SIMPLEBINTYPE   (1 << 1)
#define AOPBINTYPE1     (1 << 2)
#define AOPBINTYPE2     (1 << 3)
#define MISCBINTYPE     (1 << 4)
#define UNARYTYPE       (1 << 5)
#define BINTYPE         (SIMPLEBINTYPE | AOPBINTYPE1 | AOPBINTYPE2 | MISCBINTYPE)

#define LEAF(x)         (opdope[x->t_op].flags & LEAFTYPE)
#define UNARY(x)        (opdope[x->t_op].flags & UNARYTYPE)
#define SIMPLEBINARY(x) (opdope[x->t_op].flags & SIMPLEBINTYPE)
#define AOPBINARY1(x)   (opdope[x->t_op].flags & AOPBINTYPE1)
#define AOPBINARY2(x)   (opdope[x->t_op].flags & AOPBINTYPE2)
#define MISCBINARY(x)   (opdope[x->t_op].flags & MISCBINTYPE)
#define BINARY(x)       (opdope[x->t_op].flags & BINTYPE)

#define LEFT(x)     (x->val.in.t_left)
#define RIGHT(x)    (x->val.in.t_right)
#define NAME(x)     (x->val.ln.t_id->i_name)
#define OFFSET(x)   (x->val.ln.t_id->i_offset)
#define CON(x)      (x->val.ln.t_con)
#define BASETYPE(x) (x & (T_INT | T_DOUBLE))

struct treeops {
   int opcode;
   char *opstring;
   int flags;
};
