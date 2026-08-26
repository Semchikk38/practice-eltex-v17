CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -fPIC
LDFLAGS = -shared
LIB = liblist.so

all: $(LIB) main test

$(LIB): list.o
	$(CC) $(LDFLAGS) $^ -o $@

list.o: list.c list.h contact.h
	$(CC) $(CFLAGS) -c $< -o $@

main: main.c $(LIB)
	$(CC) $(CFLAGS) $< -L. -llist -o $@

test: test.c $(LIB)
	$(CC) $(CFLAGS) $< -L. -llist -o $@

clean:
	rm -f *.o *.so main test