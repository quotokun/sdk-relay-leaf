# Relay Leaf C Example - Windows Makefile

CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -L../lib -lrelay_leaf

TARGET = example.exe
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "Build complete! Copy relay_leaf.dll to current directory before running."

clean:
	del /f $(TARGET) 2>nul

run: $(TARGET)
	@copy /y ..\lib\relay_leaf.dll . >nul 2>&1
	./$(TARGET)

.PHONY: all clean run
