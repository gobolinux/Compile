#*********************************************************************
# Only change stuff from here if you know what you're doing
CC = gcc
INSTALL = install

CFLAGS = -I./include
LDFLAGS =
PREFIX = /usr

all: gobohide

default: all

clean:
	-rm -f *.o *~ gobohide

install:
	$(INSTALL) gobohide $(PREFIX)/sbin
	$(INSTALL) gobohide.8 $(PREFIX)/man/man8
	$(INSTALL) hier.7 $(PREFIX)/man/man7
	
gobohide: gobohide.o
	$(CC) -o gobohide gobohide.o $(LDFLAGS)

gobohide.o: gobohide.c
	$(CC) -c gobohide.c $(CFLAGS)

