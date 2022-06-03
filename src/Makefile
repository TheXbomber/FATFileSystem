CC=gcc
CCOPTS=-g -std=gnu99 -Wall

HEADERS=error.h disk.h fat.h file.h
SRC=main.c disk.c fat.c
OBJ=main.o disk.o fat.o
BIN=main

.PHONY: clean

main: $(SRC)
	$(CC) $(CCOPTS) -o $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CCOPTS) -o $@ $<

clean :
	rm -rf *.o *~ $(OBJ) $(BIN) my_disk.img