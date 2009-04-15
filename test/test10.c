/*
 * test10 - check to see if set works
 */
main()
{
   int i, j, k;
   double a, b, c;
  
   j = 5;
   a = 100;
   b = 200;
   i = j;	/* integer to integer */
   printf("i is %d\n", i);
   i = a;	/* double to integer */
   printf("i is %d\n", i);
   a = b;       /* double to double */
   printf("a is %f\n", a);
   a = i;       /* integer to double */
   printf("a is %f\n", a);
}

