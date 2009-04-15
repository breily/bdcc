/*
 * test24 - check a function call with exactly six arguments
            where the last one is a double
 */
foo (int a, int b, int c, int d, int e, double f)
{
   printf ("e is %d (10)\n", e);
   printf ("f is %f (21.0)\n", f);
}
main()
{
   double x;

   x = 21;
   foo (1, 2, 3, 4, 10, x);
}

