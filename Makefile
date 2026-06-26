CC ?= gcc

CFLAGS = -std=c99 -Wall -Wextra -pedantic

SRC = \
	src/main.c \
	src/arquivo.c \
	src/huffman.c \
	src/heap.c \
	src/log.c

OBJ = $(SRC:.c=.o)

TARGET = Menu.exe

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run