CC=gcc
CCOPTS=-g -std=gnu99 -Wall

HEADERS=error.h disk.h fat.h file.h
SRC=disk.c fat.c file.c shell.c
OBJ=disk.o fat.o file.o shell.o
BIN=shell

.PHONY: clean

shell: $(SRC)
	$(CC) $(CCOPTS) -o $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CCOPTS) -o $@ $<

clean :
	rm -rf *.o *~ $(OBJ) $(BIN) my_disk.img