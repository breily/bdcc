/*
 * test4 - arrays with doubles
 */
double m[6];

int scale(double x) {
   int i;

   if (x == 0)
      return 0;
   i = 0;
   while (i < 6) {
      m[i] = m[i] * x;
      i = i + 1;
   }
return 1;
}

main()
{
   int i;
   double z;
   m[0] = 0;
   m[1] = 1;
   m[2] = 2;
   m[3] = 3;
   m[4] = 4;
   m[5] = 5;
   z = 10;
   if (scale(z)) {
     i = 0;
      while (i < 6) {
         printf("m[%d]", i);
         printf(" = %f\n", m[i]);
	 i = i + 1;
      }
   }
   else
      printf("scale factor is zero\n");
}
