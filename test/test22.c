/*
 * test22 - check a function call with more than six arguments
 */
foo (int a, int b, int c, int d, int e, int f, int g, int h)
{
   printf ("g is %d (10)\n", g);
   printf ("h is %d (51)\n", h);
}
main()
{
   foo (1, 2, 3, 4, 5, 6, 10, 51);
}

