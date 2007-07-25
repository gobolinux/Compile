
MYCFLAGS = -O2 -Wall

STATIC=-static
ifeq ($(shell uname -s),Darwin)
   STATIC=
endif

all:
	echo $(shell uname -m)
	echo $(STATIC)
	gcc $(MYCFLAGS) -o SuperUserName SuperUserName.c
	gcc $(MYCFLAGS) -o IsExecutable IsExecutable.c
	gcc $(MYCFLAGS) -o usleep usleep.c
	gcc $(MYCFLAGS) -o LinkOrExpandAll LinkOrExpandAll.c -std=c99
	gcc $(MYCFLAGS) -o List List.c
	gcc $(MYCFLAGS) -o RescueSymlinkProgram RescueSymlinkProgram.c $(STATIC)
	strip RescueSymlinkProgram
	if [ -e "SuperUserName.exe" ]; then \
		for f in SuperUserName IsExecutable usleep LinkOrExpandAll List; do \
			mv "$$f.exe" "$$f"; \
		done; \
	fi
	mv SuperUserName IsExecutable usleep LinkOrExpandAll List RescueSymlinkProgram ../bin/ 
	chmod a+x ../bin/*

debug: MYCFLAGS = -g -DDEBUG
debug: all

static: MYCFLAGS = $(STATIC)
static: all

clean:
	cd ../bin; rm -f SuperUserName IsExecutable usleep LinkOrExpandAll List RescueSymlinkProgram

