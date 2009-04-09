CFLAGS= -c -g  -DRIGHTTOLEFT
CC=gcc
csem.exe:	cgen.o sym.o scan.o sem.o cgram.o utility.o
	$(CC) -g -o csem cgen.o sym.o scan.o sem.o cgram.o utility.o

csemdum:	cgendum.o sym.o scan.o semdum.o cgram.o utility.o
	$(CC) -g -o csem cgendum.o sym.o scan.o semdum.o cgram.o utility.o

y.tab.h cgram.c: ../mip/cgram.y
	bison --file-prefix=y -d ../mip/cgram.y
	mv y.tab.c cgram.c
	$(CC) $(CFLAGS) -I../mip cgram.c

cgen.o:	../mip/cc.h ../mip/ops.h ../mip/tree.h cgen.c
	$(CC) $(CFLAGS) -I../mip cgen.c

cgendum.o:	../mip/cc.h ../mip/ops.h ../mip/tree.h
	$(CC) $(CFLAGS) -I../mip cgendum.c

sym.o:	../mip/sym.c ../mip/cc.h y.tab.h
	$(CC) $(CFLAGS) -I. ../mip/sym.c

scan.o:	../mip/scan.c ../mip/cc.h y.tab.h
	$(CC) $(CFLAGS) -I. ../mip/scan.c

sem.o:	../mip/sem.c ../mip/cc.h ../mip/ops.h y.tab.h
	$(CC) $(CFLAGS) -I. ../mip/sem.c

semdum.o:	../mip/semdum.c ../mip/cc.h ../mip/ops.h y.tab.h
	$(CC) $(CFLAGS) -I. ../mip/semdum.c

utility.o: ../mip/utility.c ../mip/cc.h ../mip/ops.h ../mip/tree.h
	$(CC) $(CFLAGS) -I. ../mip/utility.c

example:
	./csem -showtree -codegen < example-p1.c > example-p1.s
	gcc -m32 -c example-p1.s
	gcc -m32 -o example-p1 example-p1.o
	./example-p1

example2:
	./csem -showtree -codegen < example-p2.c > example-p2.s
	gcc -m32 -c example-p2.s
	gcc -m32 -o example-p2 example-p2.o
	./example-p2

clean:
	rm -f *.o *.s csem cgram.c y.tab.h example-p1 example-p2 *test
