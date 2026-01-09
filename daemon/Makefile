CC := gcc

CFLAGS := -Wall -Wextra \
          -Iinc \
		  -I../common/inc

SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)
BIN := daemon

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(BIN)
