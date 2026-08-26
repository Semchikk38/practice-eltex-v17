CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -lm
TARGET = main

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(TARGET)