INSTALL = install
PREFIX = /usr
SYSCONFDIR = /etc

all:
	make -C src

default: all

clean:
	make -C src clean

install: all
    install -d $(SYSCONFDIR)
    install gobohide.conf $(SYSCONFDIR)
	make -C src install
	make -C man install
