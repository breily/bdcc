/*
 *  Fails because the 'addl $x,%esp' after function calls is disabled.
 */

/*
 * test25 - check a function call that has nested calls
 */
int foo (int a, int b)
{
   return (a + b);
}
main()
{
   int x;
   x = foo (1, foo (2, 3));
   printf ("x is %d (6)\n", x);
}

