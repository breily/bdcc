/*
 * test6 - test function with parameters
 */
test3(int a, double b)
{
  int i;
  i = printf("a is %d\n", a);
  i = printf("b is %f\n", b);
}
main()
{
   int i;
   double f;

   i = 10;
   f = 15;
   test3(i, f);
}
