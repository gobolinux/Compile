CC = gcc
LDFLAGS = -DUSE_NCURSES=0

all: List SymlinkProgram

clean:
	-rm -f List SymlinkProgram *.o *~

install: List SymlinkProgram
	@cp -v List SymlinkProgram ../bin
	@make clean

List: List.o
	$(CC) List.o -o List $(LDFLAGS)

List.o: List.c
	$(CC) -c List.c -Wall -I.

SymlinkProgram: SymlinkProgram.o
	$(CC) SymlinkProgram.o -o SymlinkProgram

SymlinkProgram.o: SymlinkProgram.c
	$(CC) -c SymlinkProgram.c -Wall
