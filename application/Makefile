CC := gcc
CFLAGS := -Wall -Wextra \
          -Iinc \
          -I../common/inc \
          -I../libtlm/inc

LDLIBS := ../libtlm/libtlm.a -lpthread

BIN_DIR := bin

# Find all .c files in the current directory
SRC := $(wildcard *.c)

# Generate binary names in bin/ with the same name as the source file (without .c)
BIN := $(patsubst %.c,$(BIN_DIR)/%,$(SRC))

.PHONY: all clean

# Default: build all binaries
all: $(BIN_DIR) $(BIN)

# Create bin directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Pattern rule: compile any .c to bin/<name>
$(BIN_DIR)/%: %.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $< $(LDLIBS) -o $@

# Clean all binaries
clean:
	rm -rf $(BIN_DIR)
