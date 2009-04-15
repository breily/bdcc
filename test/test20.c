/*
 * test20 - check to see if relationals work
 */
main()
{
   int i, j, k;

   i = 1;
   j = 2;
   k = 3;
   if (i > k)
      printf ("this should NOT be printed\n");
   if (k > j)
      printf ("this should be printed\n");
}

