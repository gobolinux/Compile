CC = gcc
DESTDIR = /usr
SYSCONFDIR = /etc
DEVDIR = /dev
CFLAGS  = -I. -DSYSCONFDIR=\"$(SYSCONFDIR)\" -DDEVDIR=\"$(DEVDIR)\" -Wall
LDFLAGS = -lpthread

all: listener

install: listener listener.conf
	install -m 755 -o 0 -g 0 listener $(DESTDIR)/bin
	install -m 644 -o 0 -g 0 listener.conf $(SYSCONFDIR)

clean:
	-rm -f *.o *~ listener

listener: listener.o
	$(CC) listener.o -o listener $(LDFLAGS)

listener.o: listener.c
	$(CC) -c listener.c $(CFLAGS)
