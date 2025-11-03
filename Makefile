# Makefile for Advent

CC      := clang
CFLAGS  := -std=c23 -Wall -Wextra -Wno-switch-enum -Wno-deprecated-non-prototype -O2

ASTYLE  := astyle --suffix=none --align-pointer=name --pad-oper

SRC     := advent.c
HEADER  := object.h location.h travels.h vocab.h vocab_jmp.h
TARGET  := advent

all: $(TARGET)

$(TARGET): $(SRC) $(HEADER)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

vocab_jmp.h: generator
	./generator > vocab_jmp.h

run: $(TARGET)
	./$(TARGET)

fmt:
	@echo "Formatting source files..."
	$(ASTYLE) $(SRC) $(HEADER)

clean:
	rm -f $(TARGET) *.o

.PHONY: all run clean fmt

