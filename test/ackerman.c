/*
 *  Fails - Unknown
 */


/* ACKERMAN BENCHMARK PROGRAM:
/*    This program tests the recursive function call
/*    mechanism of a compiler. Gives execution time
/*    for comparison of compilers.
*/
int main () {
  int Result;

  Result = Ack (3,6);
  printf ("    - Numerical result of test is %d\n", Result);
  return 0;
}


int Ack (int LowerBound, int UpperBound) {
/*
/* Performs recursive calls for test purposes.
*/

   if (LowerBound == 0)
      return (UpperBound+1);
   else if (UpperBound == 0)
      return (Ack(LowerBound-1, 1));
   else
       return (Ack(LowerBound-1, Ack(LowerBound, UpperBound-1)));
}
