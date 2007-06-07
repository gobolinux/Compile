CC = gcc
SYSCONFDIR = /System/Settings
DEVDIR = /System/Kernel/Devices
KERNDIR = /System/Kernel/Modules/Current/build
CFLAGS  = -I. -I$(KERNDIR)/include -DSYSCONFDIR=\"$(SYSCONFDIR)\" -DDEVDIR=\"$(DEVDIR)\" -Wall -g
LDFLAGS = -lpthread

all: listener

clean:
	-rm -f *.o *~

listener: listener.o rules.o
	$(CC) listener.o rules.o -o listener $(LDFLAGS)
	mkdir -p ../bin
	mv listener ../bin

%.o: %.c
	$(CC) -c $< $(CFLAGS)
