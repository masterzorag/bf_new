BUILD_TYPE = release

CC      = gcc
CFLAGS  = -std=gnu99 -Wall -O3
LDFLAGS =
DEPS    = parser.h signal.h

ifeq ($(BUILD_TYPE), debug)
CFLAGS += -Wextra -DDEBUG -ggdb
endif

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

bf: main.o parser.o signal.o
	$(CC) -o bf main.o parser.o signal.o


.PHONY: clean
clean:
	rm -rf *.o bf
