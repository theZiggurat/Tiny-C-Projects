all: mergesort.o simpleCSVsorter.c simpleCSVsorter.h
		gcc -g -o sorter mergesort.o simpleCSVsorter.c

mergesort.o: mergesort.c simpleCSVsorter.h
		gcc -c -o mergesort.o mergesort.c

clean:
		rm sorter
