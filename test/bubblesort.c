/*
 *  Fails - Unknown, start with arrays
 */

/* BUBBLESORT BENCHMARK PROGRAM:
/*       This program tests the basic loop constructs,
/*       integer comparisons, and simple array handling
/*       of compilers by sorting 10 arrays of randomly
/*       generated integers. The number of elements in the
/*       arrays vary from 100 to 1000 in increments of
/*       1000.
*/

int Array [1001];
int Seed;

/*
 * Initializes the seed for the random number generator.
 */
int InitSeed () {
  /*  Seed = 17563; */
  Seed = 5;
}

/*
 * Sorts an array of integers of size "Elements" in
 * ascending order.
 */
int BubbleSort (int Elements)
{
      int Sorted;
      int Temp, LastIndex, Index;

   Sorted = 0;

   for (LastIndex = Elements; LastIndex >= 2; LastIndex = LastIndex - 1)  {
      Sorted = 1;
      for (Index = 1; Index <= (LastIndex - 1); Index = Index + 1)
        if (Array [Index] > Array [Index + 1])
           {
           Temp = Array [Index];
           Array [Index] = Array [Index + 1];
           Array [Index + 1] = Temp;
           Sorted = 0;
           }
    if (Sorted)
       return 0;
    }
}
/*
 * Generates a random integer between the "Seed"
 *  and 8095.
 */
int RandomInt () {
   Seed = ((Seed * 133) + 81) % 8095;
   return (Seed);
}

/*
 * Initializes an array of size "Elements" with
 * randomly generated integers.
 */
int Initialize (int Elements) {
   int  Index;

   InitSeed ();
   for (Index = 1; Index <= Elements; Index = Index + 1)
       Array [Index] = RandomInt ();
   return 0;
}

/*
 * Verifies that an array of size "Elements" is
 * sorted in ascending order. It prints the numbers
 * of the elements that are out of order.
*/
int CheckOrderOf (int Elements)
{
     int  Index;

   for (Index = 1; Index <= (Elements - 1); Index = Index + 1)
      if (Array [Index] > Array [Index + 1])
         printf ("     - Elements < %d , %d > are out of order\n", Index, Index + 1);
   return 0;
}



int main () {
  int  Elements;
  Elements = 100;

  printf ("\n *** BUBBLE SORT BENCHMARK TEST ***\n\n");
  printf ("RESULTS OF TEST:\n\n");
  while (Elements <= 1000) {
     Initialize (Elements);
     BubbleSort (Elements);
     CheckOrderOf (Elements);
     printf ("     - Number of elements sorted is %d\n", Elements);
     Elements = Elements + 100;
  }
  return 0;
}

