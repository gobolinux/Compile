VERSION=

PROGRAM_NAME=ConfigTools

PACKAGE_DIR=$(HOME)
PACKAGE_ROOT=$(PACKAGE_DIR)/$(PROGRAM_NAME)
PACKAGE_BASE=$(PACKAGE_ROOT)/$(VERSION)
PACKAGE_FILE=$(PACKAGE_DIR)/$(PACKAGE_NAME)--$(VERSION)--$(shell uname -m).tar.bz2

PYTHON_VERSION=2.3
PYTHON_LIBS=GenGrubMenu GenLiloConf MouseInfo
PYTHON_SITE=lib/python$(PYTHON_VERSION)/site-packages

all:
	make -C src all
	[ -e $(PYTHON_SITE) ] || mkdir -p $(PYTHON_SITE)
	for f in $(PYTHON_LIBS); \
	do libf=$(PYTHON_SITE)/$$f.py; \
	   rm -f $$libf; ln -nfs ../../../bin/$$f $$libf; \
	done
	DirPythonCompile $(PYTHON_SITE) 2> /dev/null

clean :
	make -C src clean
	for f in $(PYTHON_LIBS); do libf=$(PYTHON_SITE)/$$f; rm -f $$libf.{pyo,pyc,py}; done

version_check:
	@[ "$(VERSION)" = "" ] && { echo -e "Error: run make with VERSION=<version-number>.\n"; exit 1 ;} || exit 0

dist: version_check all
	rm -rf $(PACKAGE_ROOT)
	mkdir -p $(PACKAGE_BASE)
	find * -not -path "*/CVS" -and -not -path "*/CVS/*" -and -not -path "*.py[oc]" -and -not -path "*~" | cpio -p $(PACKAGE_BASE)
	cd $(PACKAGE_DIR); tar cvp $(PROGRAM_NAME) | bzip2 > $(PACKAGE_FILE)
	rm -rf $(PACKAGE_ROOT)
	echo "Package at $(PACKAGE_FILE)"
