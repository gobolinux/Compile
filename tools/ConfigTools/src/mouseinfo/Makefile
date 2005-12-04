CC=gcc
CFLAGS=-Wall -I.

all: mouseinfo

clean:
	rm -f *.o *~ ../../bin/raw_mouseinfo

mouseinfo: kudzu.o mouseinfo.o psaux.o serial.o usb.o
	$(CC) kudzu.o mouseinfo.o psaux.o serial.o usb.o -o ../../bin/raw_mouseinfo

kudzu.o: kudzu.c
	$(CC) -c kudzu.c $(CFLAGS)
	
mouseinfo.o: mouseinfo.c
	$(CC) -c mouseinfo.c $(CFLAGS)
	
psaux.o: psaux.c
	$(CC) -c psaux.c $(CFLAGS)

serial.o: serial.c
	$(CC) -c serial.c $(CFLAGS)
	
usb.o: usb.c
	$(CC) -c usb.c $(CFLAGS)
