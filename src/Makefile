CC = gcc
CCOPTS = -g -std=gnu99 -Wall

HEADERS_DIR = ./headers
_HEADERS = error.h disk.h fat.h file.h
HEADERS = $(patsubst %,$(HEADERS_DIR)/%,$(_HEADERS))
SRC = disk.c fat.c file.c shell.c
OBJ = disk.o fat.o file.o shell.o main.o
BIN = shell main

.PHONY: clean

shell: $(SRC)
	$(CC) $(CCOPTS) -o $@ $^

main: main.c disk.c fat.c file.c
	$(CC) $(CCOPTS) -o $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CCOPTS) -o $@ $<

clean :
	rm -rf *.o *~ $(OBJ) $(BIN) my_disk.img