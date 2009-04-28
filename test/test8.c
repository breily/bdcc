/*
 * test8 - test function call with arguments
 */
test5(int x, double y, int z)
{
   printf("x is %d\n", x);
   printf("z is %d\n", z);
   printf("y is %f\n", y);
}
main()
{
   int i, j, k;
   double a, b, c;

   i = 3;
   b = 5;
   k = 100;
   test5(i, b, k);
}
