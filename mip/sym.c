/* symbol table management */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cc.h"

#define STABSIZE 119			/* hash table size for strings */
#define ITABSIZE 37			    /* hash table size for identifiers */

int level;				        /* current block level */

struct s_chain {
   char *s_ptr;
   struct s_chain *s_next;
} *str_table[STABSIZE] = {0};		/* string hash table */

IDENT *id_table[ITABSIZE] = {0};	/* identifier hash table */

void dump(int, FILE *), enterblock(void);
void leaveblock(void), sdump(FILE *);
void yyerror (char *);
char *alloc(unsigned), *slookup(char *);
int hash(char *);
IDENT *install(char *, int), *lookup(char *, int); 

/*
 *  dump - dump identifiers with block level >= blev to f
 */
void dump(int blev, FILE *f) {
   IDENT **i, *p;

   fprintf(f, "Dumping identifier table\n");
   fprintf(f, "Name\tblev\ttype\tdefined\n");
   for (i = id_table; i < &id_table[ITABSIZE]; i++)
      for (p = *i; p; p = p->i_link)
         if (p->i_blevel >= blev)
            fprintf (f, "%s\t%d\t%d\t%d\n", p->i_name, p->i_blevel, 
             p->i_type, p->i_defined);
}

/*
 * enterblock - enter a new block
 */
void enterblock() {
   level++;
}

/*
 * install - install name with block level blev, return ptr 
 */
IDENT *install(char *name, int blev) {
   IDENT *ip, **q;
   
   if (blev < 0)
      blev = level;
   ip = (IDENT *) alloc (sizeof(IDENT));
   ip->i_name = name;		/* set fields of symbol table */
   ip->i_blevel = blev;
   ip->i_offset = ip->i_width = ip->i_type = ip->i_scope = 0;

   for (q = &id_table[hash (name) % ITABSIZE]; *q; q = &((*q)->i_link))
      if (blev >= (*q)->i_blevel)
         break;
   ip->i_link = *q;
   *q = ip;
   return (ip);
}

/*
 * leaveblock - exit a block
 */
void leaveblock() {
   IDENT **i, *p, *tmp;

   if (level > 0) {
      for (i = id_table; i < &id_table[ITABSIZE]; i++) {
         for (p = *i; p; p = tmp)
            if (p->i_blevel < level)
               break;
            else {
               tmp = p->i_link;
               free (p);
            }
         *i = p;
      }
      level--;
   }
}

/*
 * lookup - lookup name, return ptr; use default scope if blev == 0
 */
IDENT *lookup(char *name, int blev) {
   IDENT *p;

   for (p = id_table[hash (name) % ITABSIZE]; p; p = p->i_link)
      if (name == p->i_name && (blev == 0 || blev == p->i_blevel))
         return (p);
   return (NULL);
}

/*
 * sdump - dump string table to f
 */
void sdump(FILE *f) {
   struct s_chain **s, *p;

   fprintf (f, "Dumping string table\n");
   for (s = str_table; s < &str_table[STABSIZE]; s++)
      for (p = *s; p; p = p->s_next)
         fprintf(f, "%s\n", p->s_ptr);
}

/*
 * slookup - lookup str in string table, install if necessary, return ptr 
 */
char *slookup(char str[]) {
   struct s_chain *p;
   int i, k;
   
   for (k = i = 0; i < 5; i++)	/* simple hash function */
      if (str[i])
         k += str[i];
      else
         break;
   
   k %= STABSIZE;
   for (p = str_table[k]; p; p = p->s_next)
      if (strcmp (str, p->s_ptr) == 0)
         return (p->s_ptr);
   p = (struct s_chain *) alloc (sizeof(struct s_chain));
   p->s_next = str_table[k];
   str_table[k] = p;
   p->s_ptr = (char *) alloc ((unsigned) strlen (str) + 1);
   p->s_ptr = strcpy (p->s_ptr, str);
   return (p->s_ptr);
}

/*
 * hash - hash name, turn address into hash number
 */
int hash(char *s) {
   return ((int ) s);
}   

/*
 * alloc - alloc space
 */
char *alloc(unsigned n) {
   char *p;
   if ((p = malloc (n)) == NULL) {
      yyerror("csem: out of space");
      exit(1);
   }
   return p;
}
