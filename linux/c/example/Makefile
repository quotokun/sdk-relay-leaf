# Relay Leaf C Example - Linux Makefile

CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -L../lib -lrelay_leaf -Wl,-rpath,'$$ORIGIN/../lib'

TARGET = example
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	export LD_LIBRARY_PATH=../lib:$$LD_LIBRARY_PATH && ./$(TARGET)

.PHONY: all clean run
