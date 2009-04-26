/*
 *  Fails because the 'addl $x,%esp' after function calls is disabled.
 */

/*
 * test26 - check a function call that has nested calls
 */
int foo (int a, int b)
{
   return (a + b);
}
main()
{
   int x;
   x = foo (foo(4, foo (1, 5)), foo (2, 3));
   printf ("x is %d (15)\n", x);
}

