CC := gcc

CFLAGS := -Wall -Wextra \
          -Iinc \
          -I../common/inc \
          -I../libtle/inc

# link the static libraries
LDFLAGS := ../common/libcommon.a ../libtle/libtle.a

SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)
BIN := daemon

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(BIN)
