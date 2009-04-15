/*
 * test13 - check to see if op2 works
 */
main()
{
   int i, j, k;
   double a, b, c;

   j = 25;
   k = 5;
   i = j + k;      /* integer arithmetic */
   printf("i is %d\n", i);
   i = j * k;      /* integer arithmetic */
   printf("i is %d\n", i);
   i = j / k;      /* integer arithmetic */
   printf("i is %d\n", i);
   i = j - k;      /* integer arithmetic */
   printf("i is %d\n", i);
   b = 12;
   c = 6;
   a = b + c;      /* double arithmetic */
   printf("a is %f\n", a);
   a = b * c;      /* double arithmetic */
   printf("a is %f\n", a);
   a = b / c;      /* double arithmetic */
   printf("a is %f\n", a);
   a = b - c;      /* double arithmetic */
   printf("a is %f\n", a);
   a = i + b;      /* mixed mode */
   printf("a is %f\n", a);
   a = b + i;      /* mixed mode */
   printf("a is %f\n", a);
}

