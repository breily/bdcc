/*
 *  Fails due to having a triple || - I need to patch them  together somehow.
 */

main()
{
   int c, nw, nl, nc;
   int inword;

   nl = nw = nc = 0;
   inword = 0;
   while ((c = in()) != -1) {
      nc = nc + 1;
      if (c == 10)
         nl = nl + 1;
      if (c == 32 || c == 10 || c == 9)
         inword = 0;
      else if (inword == 0) {
         inword = 1;
         nw = nw + 1;
         }
      }
   printf("\t%d\t%d\t%d\n", nl, nw, nc);
}
