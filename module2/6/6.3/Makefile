CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -fPIC
LDFLAGS = -ldl -lm
PLUGIN_DIR = plugins
TARGET = main
TEST_TARGET = test

all: $(TARGET) $(TEST_TARGET) plugins

$(TARGET): main.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(TEST_TARGET): test.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

PLUGINS = add sub mul div pow
PLUGIN_SRCS = $(addprefix $(PLUGIN_DIR)/, $(addsuffix .c, $(PLUGINS)))
PLUGIN_SOS = $(addprefix $(PLUGIN_DIR)/, $(addsuffix .so, $(PLUGINS)))

plugins: $(PLUGIN_SOS)

$(PLUGIN_DIR)/%.so: $(PLUGIN_DIR)/%.c
	$(CC) $(CFLAGS) -shared $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET) $(TEST_TARGET) $(PLUGIN_SOS)

.PHONY: all plugins clean