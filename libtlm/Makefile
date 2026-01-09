CC      := gcc
CFLAGS  := -Wall -Wextra -Iinc
AR      := ar
ARFLAGS := rcs

SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)
LIB := libtlm.a

all: $(LIB)

$(LIB): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(LIB)