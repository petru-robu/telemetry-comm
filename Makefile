CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LIBNAME = libtle.a

# Foldere surse
DAEMON_DIR = daemon
LIB_DIR    = libtle

# Surse
DAEMON_SRC = $(DAEMON_DIR)/main.c $(DAEMON_DIR)/server.c
LIB_SRC    = $(LIB_DIR)/tle.c

# Obiecte
DAEMON_OBJ = $(DAEMON_SRC:.c=.o)
LIB_OBJ    = $(LIB_SRC:.c=.o)

# Targeturi implicite
all: daemon $(LIBNAME)

# Daemon
daemon: $(DAEMON_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Biblioteca statica
$(LIBNAME): $(LIB_OBJ)
	ar rcs $@ $^

clean:
	rm -f daemon $(LIBNAME) $(DAEMON_OBJ) $(LIB_OBJ)
