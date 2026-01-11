CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11 $(shell pkg-config --cflags gtk+-3.0) -I../libtlm/inc
LDFLAGS := $(shell pkg-config --libs gtk+-3.0) -L../libtlm -ltlm -lpthread

TARGET  := gui
SRC     := gui.c
LIBTLM_DIR := ../libtlm
LIBTLM  := $(LIBTLM_DIR)/libtlm.a

all: $(TARGET)

# Rule to ensure the library is built
$(LIBTLM):
	$(MAKE) -C $(LIBTLM_DIR)

$(TARGET): $(SRC) $(LIBTLM)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)
	$(MAKE) -C $(LIBTLM_DIR) clean

.PHONY: all clean
