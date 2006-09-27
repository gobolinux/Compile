
MYCFLAGS = -O2 -Wall

all:
	gcc $(MYCFLAGS) -o AutoLogin AutoLogin.c
	mv AutoLogin ../bin/ 
	chmod a+x ../bin/AutoLogin

debug: MYCFLAGS = -g -DDEBUG
debug: all

static: MYCFLAGS = -static
static: all

clean:
	cd ../bin; rm -f AutoLogin

