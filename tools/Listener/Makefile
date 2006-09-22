CC = gcc
DESTDIR = /usr
SYSCONFDIR = /etc
DEVDIR = /dev
KERNDIR = /System/Kernel/Modules/Current/build
CFLAGS  = -I. -I$(KERNDIR)/include -DSYSCONFDIR=\"$(SYSCONFDIR)\" -DDEVDIR=\"$(DEVDIR)\" -Wall -DDEBUG=1 -g
LDFLAGS = -lpthread

all: listener

install: listener listener.conf
	install -m 755 -o 0 -g 0 listener $(DESTDIR)/bin
	install -m 644 -o 0 -g 0 listener.conf $(SYSCONFDIR)

clean:
	-rm -f *.o *~ listener

listener: listener.o rules.o
	$(CC) listener.o rules.o -o listener $(LDFLAGS)

%.o: %.c
	$(CC) -c $< $(CFLAGS)
