CC = gcc
CFLAGS = -Wall
LDFLAGS =

TARGETS = List RemoveBroken SymlinkProgram
CVS_DIR = tools/Scripts/src
CVS_FILES = $(CVS_DIR)/LinkOrExpandAll.c $(CVS_DIR)/List.c $(CVS_DIR)/List.h


all: SymlinkProgram.o LinkOrExpandAll.o List.o RemoveBroken.o
	$(CC) $(LDFLAGS) SymlinkProgram.o LinkOrExpandAll.o -o SymlinkProgram
	$(CC) $(LDFLAGS) List.o -o List
	$(CC) $(LDFLAGS) RemoveBroken.o -o RemoveBroken
	mv $(TARGETS) ../bin && chmod +x ../bin/*
	ln -sf List ../bin/l

clean:
	rm -Rf *.o *~ tools/ LinkOrExpandAll.c  List.c  List.h
	cd ../bin; rm -f $(TARGETS) l

LinkOrExpandAll.o: LinkOrExpandAll.c
	@GrepReplace "int main(" "int link_or_expand_all_main(" LinkOrExpandAll.c
	$(CC) $(CFLAGS) -c LinkOrExpandAll.c -std=c99

List.c LinkOrExpandAll.c:
	@cvs -d:pserver:anonymous@cvs.savannah.nongnu.org:/sources/goboscripts co $(CVS_FILES)
	@mv $(CVS_DIR)/*.c $(CVS_DIR)/*.h .
	@rm -Rf tools/

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
