/*
 * test17 - check to see if || works
 */
main()
{
   int i, j, k;

   i = 1;
   j = 2;
   k = 3;
   if (i < j || k > j)
      printf("this should be printed\n");
   else
      printf("this should not be printed\n"); 
   if (j < i || i < k)
      printf("this should be printed\n");
   else
      printf("this should not be printed\n"); 
   if (j < i || k < i)
      printf("this should not be printed\n"); 
   else
      printf("this should be printed\n");
}

