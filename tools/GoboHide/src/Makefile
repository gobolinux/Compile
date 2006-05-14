MYCFLAGS = -O2 -Wall

all: gobohide

gobohide: gobohide.c
	gcc $(MYCFLAGS) -o gobohide gobohide.c 
	mkdir -p ../sbin
	mv gobohide ../sbin 

debug: MYCFLAGS = -g -DDEBUG
debug: all

static: MYCFLAGS = -static
static: all

clean:
	cd ../sbin; rm -f gobohide
