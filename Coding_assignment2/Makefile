CC ?= $(CROSS_COMPILE)gcc
INCLUDES ?=
FLAVOR ?= 0
# Convert FLAVOR number to FLAVOR_NAME
ifeq ($(FLAVOR),0)
    FLAVOR_NAME = femto
else ifeq ($(FLAVOR),1)
    FLAVOR_NAME = pico
else ifeq ($(FLAVOR),2)
    FLAVOR_NAME = nano
else
    $(error Invalid FLAVOR value. Use 0 (femto), 1 (pico), or 2 (nano))
endif

CFLAGS ?= -g -Wall $(INCLUDES)
CFLAGS += -Dshell_flavor=$(FLAVOR)
TARGET = $(FLAVOR_NAME)_shell
SRC = poly_shell.c
OBJ = $(SRC:.c=.o)


$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

help:
	@echo "Available make targets:"
	@echo "  make          - Build the shell (default: femto flavor)"
	@echo "  make FLAVOR=1 - Build pico shell"
	@echo "  make FLAVOR=2 - Build nano shell"
	@echo "  make clean    - Remove compiled files"
	@echo "  make help     - Show this help message"

.PHONY: all clean