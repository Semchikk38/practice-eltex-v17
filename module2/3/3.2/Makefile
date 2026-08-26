CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = main
TEST_TARGET = test

all: $(TARGET) $(TEST_TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_TARGET): test.c main.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(TARGET) $(TEST_TARGET)