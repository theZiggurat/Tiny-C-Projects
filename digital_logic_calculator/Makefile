all: truthtable

second: truthtable.c
	gcc -Wall -Werror -fsanitize=address -o -lm truthtable truthtable.c

clean:
	rm -f truthtable
