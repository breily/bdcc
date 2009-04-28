#!/bin/bash

# set FE to point to your compiler front end
FE=../i386/csem

# This flags shuts up gcc warning messages about printf
GCCFLAGS="-fno-builtin"

testfiles="test1 test2 test3 test4a test4b test5 test6 test7 \
   test8 test9 test10 test11 test12 test13 test14 test15 test16 \
   test17 test18 test19 test20 test21 test22 test23 test24 test25 \
   test26 test27 example-p1 example-p2 ackerman bubblesort shellsort"
# I removed 8q, infinite loop


for j in ${testfiles} ; do
   #echo "Compiling ${j}"
   ${FE} -showtree -codegen <${j}.c >${j}.s
  gcc -m32 -o ${j}.exe ${j}.s
   ./${j}.exe >$j.out
   gcc ${GCCFLAGS} -m32 -o ${j}.exe1 ${j}.c
   ./${j}.exe1 >${j}.out1
   if cmp ${j}.out ${j}.out1 ; then
      #echo "${j} executed correctly"
        echo "${j}          pass"
   else
      #echo "${j} did NOT execute correctly"
        echo "${j}"
#      diff ${j}.out ${j}.out1
   fi
   rm -f ${j}.out ${j}.out1 ${j}.exe ${j}.exe1
done

#echo "Compiling wc"
${FE} -codegen <wc.c >wc.s
gcc -m32 -c in.c
gcc -m32 -o wc.exe wc.s in.o
./wc.exe <wc.c >wc.out
gcc ${GCCFLAGS} -o wc.exe1 wc.c in.c
./wc.exe1 <wc.c >wc.out1
if cmp wc.out wc.out1 ; then
  #echo "wc executed correctly"
  echo "wc          pass"
else
  #echo "wc did NOT execute correctly"
  echo "wc"
  #diff wc.out wc.out1
fi

#echo "Compiling cf"
${FE} -codegen <cf.c >cf.s
gcc -m32 -c in.c
gcc -m32 -o cf.exe cf.s in.o
./cf.exe <cf.c >cf.out
gcc ${GCCFLAGS} -o cf.exe1 cf.c in.c
./cf.exe1 <cf.c >cf.out1
if cmp cf.out cf.out1 ; then
  #echo "cf executed correctly"
  echo "cf          pass"
else
  #echo "cf did NOT execute correctly"
  echo "cf"
  #diff cf.out cf.out1
fi

rm -f *.exe *.exe1 *.out *.out1 *.s
echo "================== End testing ${i} ======================="

