MYCFLAGS = -O2 -Wall -I../include

all: gobohide

gobohide: gobohide.c
	gcc $(MYCFLAGS) -o gobohide gobohide.c 
	mkdir -p ../sbin
	mv gobohide ../sbin 

install:
	install -d $(PREFIX)/sbin
	install ../sbin/gobohide $(PREFIX)/sbin

debug: MYCFLAGS = -g -DDEBUG
debug: all

static: MYCFLAGS = -static
static: all

clean:
	cd ../sbin; rm -f gobohide
