CC=gcc
CFLAGS=-I. -std=gnu99 -Wall -ggdb
DEPS = parser.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

bf: main.o parser.o
	gcc -o bf main.o parser.o -I.


.PHONY: clean
clean:
	rm -rf *.o bf