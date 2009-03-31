# include "cc.h"
# include "y.tab.h"
# include <stdio.h>
# include <ctype.h>

# define MAXTOK 100	/* maximum token size */

# define LETTER 'a'
# define DIGIT '0'

int lineno = 1;		/* current line number */

void comment(void), initlex(void), initrw(int, char *), putbak(int), skip(void);
int istype(int), quote(char *), yylex(void);
char *slookup(char *);
IDENT *install(char *, int), *lookup(char *, int);

/* 
 * initlex - initialize lexical analyzer
 */
void initlex() {
   initrw (SET, "=");
   initrw (SETOR, "|=");
   initrw (SETXOR, "^=");
   initrw (SETAND, "&=");
   initrw (SETLSH, "<<=");
   initrw (SETRSH, ">>=");
   initrw (SETADD, "+=");
   initrw (SETSUB, "-=");
   initrw (SETMUL, "*=");
   initrw (SETDIV, "/=");
   initrw (SETMOD, "%=");
   initrw (MINUSMINUS, "--");
   initrw (PLUSPLUS, "++");
   initrw (OR, "||");
   initrw (AND, "&&");
   initrw (BITOR, "|");
   initrw (BITXOR, "^");
   initrw (BITAND, "&");
   initrw (EQ, "==");
   initrw (NE, "!=");
   initrw (GT, ">");
   initrw (GE, ">=");
   initrw (LT, "<");
   initrw (LE, "<=");
   initrw (LSH, "<<");
   initrw (RSH, ">>");
   initrw (ADD, "+");
   initrw (SUB, "-");
   initrw (MUL, "*");
   initrw (DIV, "/");
   initrw (MOD, "%");
   initrw (NOT, "!");
   initrw (COM, "~");
   initrw (IF, "if");
   initrw (FOR, "for");
   initrw (ELSE, "else");
   initrw (WHILE, "while");
   initrw (RETURN, "return");
   initrw (DOUBLE, "double");
   initrw (INT, "int");
}

/* 
 * initrw - initialize a reserved word entry
 */
void initrw(int k, char *s) {
   IDENT *p;
 
   p = install (slookup (s), 1);
   p->i_type = k;
   p->i_defined = 1;
}

/*
 * yylex - fetch next token
 */
int yylex() {
   int c, i, type;
   char lin[MAXTOK];
   IDENT *p;

   i = 0;
   type = RESERVED;
   skip ();
   switch (istype (c = getchar())) {
      case EOF:
         return (-1);
         break;
      case LETTER: 
         putbak (c);
         while ((isalpha(c = getchar()) || isdigit(c) || c == '_') && i < MAXTOK)
            lin[i++] = c;
         putbak (c);
         type = ID;
         break;
      case DIGIT:
         putbak (c);
         while (isdigit(c = getchar()) && i < MAXTOK)
            lin[i++] = c;
         putbak (c);
         type = CON;
         break;
      case '(': case ')': case ',': case '.': case ':': 
      case ';': case '?': case '[': case ']': case '{':
      case '}':
         type = lin[i++] = c;
         break;
      case '~':
         lin[i++] = c;
         break;
      case '!': case '%': case '*': case '/': case '^': case '=':
         lin[i++] = c;
         c = getchar();
         if (c == '=')
            lin[i++] = c;
         else
            putbak (c);
         break;
      case '&': case '+': case '-': case '|':
         lin[i++] = c;
         c = getchar();
         if (c == '=' || c == lin[i-1])
            lin[i++] = c;
         else
            putbak (c);
         break;
      case '<': case '>':
         lin[i++] = c;
         c = getchar();
         if (c == lin[i-1]) {
            lin[i++] = c;
            c = getchar();
            }
         if (c == '=')
            lin[i++] = c;
         else
            putbak (c);
         break;
      case '"':
         i = quote (lin);
         type = STR;
         break;
      default:
        fprintf (stderr, "illegal character: %o\n", c);
        return (yylex ());
      }
   lin[i] = 0;
   p = lookup (yylval.str_ptr = slookup (lin), 0);
   if (p != NULL && p->i_blevel == 1)
      type = p->i_type;
   return (type);
}

/*
 * skip - eat blanks, comments
 */
void skip() {
   int c1, c2;

   c1 = getchar(); 
   for (;;)
      if (isspace(c1)) {
         if (c1 == '\n')
            lineno++;
         c1 = getchar();
         }
      else if (c1 == '/' && (c2 = getchar()) == '*') {
         comment();
         c1 = getchar();
         }
      else if (c1 == '/') {
         putbak (c2);
         break;
         }
      else
         break;
   putbak (c1);
}

/*
 * comment - skip over comment
 */
void comment() {
   int c1, c2;

   for (;;) {
      c1 = getchar();
      if (c1 == EOF)
         break;
      else if (c1 == '*' && (c2 = getchar()) == '/')
         break;
      else if (c1 == '*')
         putbak (c2);
   }
}

/*
 * istype - classify character
 */
int istype(int c) {
   if (isalpha(c))
      return(LETTER);
   else if (isdigit(c))
      return(DIGIT);
   else
      return(c);
}

/*
 * putbak - push character back onto input
 */
void putbak(int c) {
   if (c != EOF) ungetc(c, stdin);
}

/*
 * quote - get quoted string
 */
int quote(char lin[]) {
   int c, i, j, peek;

   i = j = 0;
   c = getchar();
   for (;;) {
      if (c == '"')
         break;
      if (c == EOF) {
         fprintf (stderr, "missing quote\n");
         break;
         }
      if (c == '\\') {
         peek = getchar();
         if (peek == '\n') {   /* ignore escaped newline */
            c = getchar();
            continue;
            }
         else
            putbak (peek);
         }
      j++;   			/* count characters inside string */
      lin[i] = c;  	/* copy next char */
      c = getchar();
      if (lin[i++] == '\\' && c != EOF) {
         lin[i++] = c;   	/* copy escaped char */
         c = getchar();
         }
      }
   lin[i++] = '\0';
   return (i);
}
