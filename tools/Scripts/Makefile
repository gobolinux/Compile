
VERSION=
PACKAGE_DIR=$(HOME)
PACKAGE_ROOT=$(PACKAGE_DIR)/Scripts
PACKAGE_BASE=$(PACKAGE_ROOT)/$(VERSION)
PACKAGE_FILE=$(PACKAGE_DIR)/Scripts--$(VERSION)--$(shell uname -m).tar.bz2

PYTHON_VERSION=2.3
PYTHON_LIBS=FindPackage GetAvailable GuessLatest CheckDependencies
PYTHON_SITE=lib/python$(PYTHON_VERSION)/site-packages

all:
	cd src; make all
	for f in $(PYTHON_LIBS); \
	do libf=$(PYTHON_SITE)/$$f.py; \
	   rm $$libf; ln -nfs ../../../bin/$$f $$libf; \
	done

version_check:
	@[ "$(VERSION)" = "" ] && { echo -e "Error: run make with VERSION=<version-number>.\n"; exit 1 ;} || exit 0

dist: version_check all
	rm -rf $(PACKAGE_ROOT)
	mkdir -p $(PACKAGE_BASE)
	find * -not -path "*/CVS" -and -not -path "*/CVS/*" -and -not -path "*.py[oc]" -and -not -path "*~" | cpio -p $(PACKAGE_BASE)
	cd $(PACKAGE_DIR); tar cvp Scripts | bzip2 > $(PACKAGE_FILE)
	rm -rf $(PACKAGE_ROOT)
	echo "Package at $(PACKAGE_FILE)"
