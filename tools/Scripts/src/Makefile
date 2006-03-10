
all:
	gcc $(MYCFLAGS) -o SuperUserName SuperUserName.c
	gcc $(MYCFLAGS) -o IsExecutable IsExecutable.c
	gcc $(MYCFLAGS) -o usleep usleep.c
	gcc $(MYCFLAGS) -o LinkOrExpandAll LinkOrExpandAll.c -std=c99
	gcc $(MYCFLAGS) -o List List.c
	mv SuperUserName IsExecutable usleep LinkOrExpandAll List ../bin/ 

debug: MYCFLAGS += -g -DDEBUG
debug: all

static: MYCFLAGS += -static
static: all

clean:
	cd ../bin; rm -f SuperUserName IsExecutable usleep LinkOrExpandAll List

