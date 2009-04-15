/*
 * test15 - check to see if if works
 */
main()
{
   int i, j, k;
   double a, b, c;

   i = 1;
   k = 2;
   if (i < k)
      printf("this should be printed\n");
   if (k < i)
      printf("this should not be printed\n");
   if (k < i)
      printf("this should not be printed\n");
   else 
      printf("this should be printed\n");
}

