/*
 *  Fails - Who knows why, start with arrays.
 */

int up[16];
int down[16];
int rows[8];
int x[8];
main()
{
   int i;
   i = 0;
   while (i < 16) {
      up[i] = down[i] = 0;
      i = i + 1;
   }
   i = 0;
   while (i < 8) {
      rows[i] = x[i] = 0;
      i = i + 1;
   }
queens(0);
}

queens(int c)
{
   int r;

   r = 0;
   while (r < 8) {
      if (rows[r] == 0 && up[r-c+7] == 0 && down[r+c] == 0) {
         rows[r] = 1;
         up[r-c+7] = 1; 
         down[r+c] = 1;
         x[c] = r;
         if (c == 7)
            print();
         else
            queens(c + 1);
         rows[r] = 0;
         up[r-c+7] = 0;
         down[r+c] = 0;
         }
      r = r + 1;
   }
}

print()
{
   int k;

   k = 0;
   while (k < 8) {
      printf(" %d", x[k]);
      k = k + 1;
   }
   printf("\n");
}
