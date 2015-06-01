/*
 *  Fails when adding double + integer.
 */

/*
 * test11 - check to see if casting works
 */
main()
{
   int i, j, k;
   double a, b, c;

   i = 3;
   j = 5;
   i = i + j;       /* integer  = integer + integer */
   printf("i is %d\n", i);
   a = 10;
   b = 3;
   a = a + b;       /* double = double + double */
   printf("a is %f\n", a);
   i = i + b;       /* integer = double + integer */
   printf("i is %d\n", i);
   a = a + i;       /* double = double + integer */
   printf("a is %f\n", a);
}

