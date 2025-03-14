CC ?= $(CROSS_COMPILE)gcc
INCLUDES = -Iinclude
CFLAGS ?= -g -Wall $(INCLUDES)
INCLUDES ?=
SRC = $(wildcard *.c)

OUT = $(SRC:.c=)

all: $(OUT)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OUT) $(TARGET)

.PHONY: all clean