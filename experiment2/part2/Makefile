numdeal: function.o main.o
	gcc main.o function.o -o numdeal
function.o: function.c function.h
	gcc -c function.c
main.o: main.c function.h
	gcc -c main.c
clean:
	rm -rf *.o
