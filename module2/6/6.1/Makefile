CC = gcc
CFLAGS = -Wall -Wextra -std=c99
AR = ar
ARFLAGS = rcs
LIB = liblist.a

all: $(LIB) main test

$(LIB): list.o
	$(AR) $(ARFLAGS) $@ $^

list.o: list.c list.h contact.h
	$(CC) $(CFLAGS) -c $< -o $@

main: main.c $(LIB)
	$(CC) $(CFLAGS) $< -L. -llist -o $@

test: test.c $(LIB)
	$(CC) $(CFLAGS) $< -L. -llist -o $@

clean:
	rm -f *.o *.a main test