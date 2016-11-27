CC= gcc
CFLAGS= -std=gnu99 -Wall

all: bf

bf: main.o
	$(CC) $(CFLAGS) main.o -o bf

.PHONY: clean
clean:
	rm -rf *o bf
