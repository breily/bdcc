typedef unsigned int TWORD;
typedef struct tree_node TNODE;
typedef struct bkp_node BNODE;
typedef struct id_entry IDENT;

/* symbol table entry */
struct id_entry {
   IDENT *i_link;		/* pointer to next entry on hash chain */
   char *i_name;		/* pointer to name in string table */
   TWORD i_type;		/* type code */
   int i_blevel;		/* block level */
   int i_defined;		/* non-zero if identifier is defined */
   int i_width;			/* number of words occupied */
   int i_scope;			/* scope */
   int i_offset;		/* offset in activation frame */
};

/* scopes *** do not rearrange *** */
#define LOCAL  0
#define PARAM  1
#define GLOBAL 2

/* internal types *** do not rearrange *** */
#define T_INT	 (1)      /* integer   */
#define T_DOUBLE (1 << 1) /* double    */
#define T_CHAR   (1 << 2) /* char      */
#define T_PROC   (1 << 3) /* procedure */
#define T_ARRAY  (1 << 4) /* array     */
#define T_PTR    (1 << 5) /* pointer   */

/* backpatch record */
struct bkp_node {
   int b_label;			/* expression with label to backpatch */ 
   union {
      BNODE *b_true;		/* true backpatch list */
      BNODE *b_link;		/* link to next on chain */
   } back;
   BNODE *b_false;		/* false backpatch list */
};

/* expression tree record */
struct tree_node {
   int t_op;
   TWORD t_mode;

   union {
      /* interior node */
      struct {
         TNODE *t_left;   /* left child */
         TNODE *t_right;  /* right child (NULL if unary operator ) */
      } in;

      /* leaf node */
      struct {
         IDENT *t_id;   	/* pointer to symbol table entry */
         char *t_str;       /* pointer to string table entry */
         int t_con;         /* integer constant  */
         int t_reg;         /* register number holding value */
      } ln;
   } val;
};
