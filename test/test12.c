/*
 * test12 - check to see if opb works
 */
main()
{
   int i, j, k;

   j = 10;
   k = 4;
   i = j | k;
   printf("i is %d == 14\n", i);
   i = j & k;
   printf("i is %d == 0\n", i);
}

