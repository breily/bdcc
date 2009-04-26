/*
 *  Fails - Arrays, maybe more
 */

main()
{
   int i, c, f[128], total;
   double x, y;

   total = 0;
   i = 0;
   while (i < 128) {
      f[i] = 0;
      i = i + 1;
   }
   while ((c = in()) != -1) {
      c = c & 127;
      f[c] = f[c] + 1;
      total = total + 1;
      }
   printf("letter	count	frequency\n");
   i = 0;
   while (i < 128) {
      if (f[i] != 0) {
         if (i <= 32)
            printf("%d", i);
         else
            printf("%c", i);
         x = f[i];
         y = total;
         printf("\t%d\t%f\n", f[i], x/y);
         }
      i = i + 1;
      }
}

