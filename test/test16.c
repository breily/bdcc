/*
 *  Fails - && must not be working.
 */

/*
 * test16 - check to see if && works
 */
main()
{
   int i, j, k;

   i = 1;
   j = 2;
   k = 3;
   if (i < j && k > j)
      printf("this should be printed\n");
   else
      printf("this should not be printed\n"); 
   if (i < j && j > k)
      printf("this should not be printed\n"); 
   else
      printf("this should be printed\n");
}

