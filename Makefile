# Makefile for Advent

# Compiler and flags
CC      := clang
CFLAGS  := -std=c23 -Wall -Wextra -Wno-switch-enum -Wno-deprecated-non-prototype -O2

ASTYLE = astyle --suffix=none

# Source and target
SRC     := advent.c
TARGET  := advent

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Run the program
run: $(TARGET)
	./$(TARGET)

fmt:
	@echo "Formatting source files..."
	$(ASTYLE) $(SRC)

# Clean up
clean:
	rm -f $(TARGET) *.o

.PHONY: all run clean format

