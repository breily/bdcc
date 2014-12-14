/*  SHELL SORT BENCHMARK PROGRAM:
/*         This program sorts an array of integers by
/*         comparing distant elements first and gradually
/*         comparing nearer elements.
*/
int Array [1001], Seed;



/*
 * Initializes the seed used in tthe random number generator.
 */
int InitSeed () {
   Seed = 17563;
   return 0;
}


/*
 * Generates a random integer between the "Seed"
 * and 8095.
 */
int RandomInt () {
   Seed = ((Seed * 133) + 81) % 8095;
   return (Seed);
}

/*
 * Initializes an array of size "Elements" with
 * randomly generated integers.
 */
Initialize (int Elements) {
     int  Index;

   InitSeed();
   for (Index = 0; Index <= (Elements - 1); Index = Index + 1)
      Array [Index] = RandomInt ();
}


/*
 * Verifies that an array of size "Elements" is
 * sorted in ascending order. It prints the numbers
 * of the elements that are out of order.
 */
int CheckOrderOf (int Elements) {
   int  Index;

   for (Index = 0; Index <= (Elements - 2); Index = Index + 1)
      if (Array [Index] > Array [Index + 1])
          printf ("     - Elements < %d , %d > are out of order\n", Index, Index + 1);
   return 0;
}


/*
 * Sorts an array of integers of size 'ArraySize'
 * in ascending order.
 */
int ShellSort(int IntArray[], int ArraySize) {
   int  Gap, i, j, Temp;

   Gap = ArraySize / 2;
   while (Gap > 0)  {
      i = Gap;
      while (i < ArraySize) {
         j = i - Gap;
         while (j >= 0 && IntArray [j] > IntArray [j + Gap]) {
            Temp = IntArray [j];
            IntArray [j] = IntArray [j + Gap];
            IntArray [j + Gap] = Temp;
            j = j - Gap;
            }
            i = i + 1;
         }
      Gap = Gap / 2;
   }
}

int main () {
   int  Elements;

   Elements = 100;
   printf ("\n *** SHELL SORT BENCHMARK TEST ***\n\n");
   printf ("RESULTS OF TEST:\n\n");
   while (Elements <= 1000) {
      Initialize (Elements);
      ShellSort (Array, Elements);
      CheckOrderOf (Elements);
      printf ("     - Number of elements sorted is %d\n", Elements);
      Elements = Elements + 100;
   }
}
