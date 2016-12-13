BUILD_TYPE = release

CC     = gcc
CFLAGS = -I. -std=gnu99 -Wall -O2
DEPS   = parser.h

ifeq ($(BUILD_TYPE), debug)
CFLAGS += -Wextra -DDEBUG -ggdb
endif

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

bf: main.o parser.o
	$(CC) -o bf main.o parser.o -I.


.PHONY: clean
clean:
	rm -rf *.o bf
