BUILD_TYPE = release

CC=gcc
CFLAGS=-I. -std=gnu99 -Wall
DEPS = parser.h

ifeq ($(BUILD_TYPE), debug)
CFLAGS += -Wextra -DDEBUG -ggdb
endif

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

bf: main.o parser.o
	gcc -o bf main.o parser.o -I.


.PHONY: clean
clean:
	rm -rf *.o bf