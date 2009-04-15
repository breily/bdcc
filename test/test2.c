/*
 * test2 - check linkage mechanism
 */
main()
{
   int i;

   i = 10;

   printf("i is %d\n", i);
   i = i + 3;
   foo(i);
}
foo(int j)
{
   printf("j is %d\n", j);
}
