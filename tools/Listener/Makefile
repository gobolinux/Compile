VERSION=
PROGRAM=Listener

PACKAGE_DIR=$(HOME)
PACKAGE_ROOT=$(PACKAGE_DIR)/$(PROGRAM)
PACKAGE_VDIR=$(PACKAGE_ROOT)/$(VERSION)
PACKAGE_FILE=$(PACKAGE_DIR)/$(PROGRAM)--$(VERSION)--$(shell uname -m).tar.bz2
CVSTAG=`echo $(PROGRAM)_$(VERSION) | tr "[:lower:]" "[:upper:]" | sed  's,\.,_,g'`

all:
	make -C src

version_check:
	@[ "$(VERSION)" = "" ] && { echo -e "Error: run make with VERSION=<version-number>.\n"; exit 1 ;} || exit 0

clean: cleanup
	rm -rf bin

cleanup:
	rm -rf Resources/FileHash*
	find * -path "*~" -or -path "*/.\#*" -or -path "*.bak" | xargs rm -f
	make -C src clean

verify:
	! { cvs up -dP 2>&1 | grep "^[\?]" | grep -v "Resources/SettingsBackup\|? bin" ;}

dist: all version_check cleanup verify
	rm -rf $(PACKAGE_ROOT)
	mkdir -p $(PACKAGE_VDIR)
	SignProgram $(PROGRAM)
	cat Resources/FileHash
	ListProgramFiles $(PROGRAM) | cpio -p $(PACKAGE_VDIR)
	cd $(PACKAGE_DIR); tar cvp $(PROGRAM) | bzip2 > $(PACKAGE_FILE)
	rm -rf $(PACKAGE_ROOT)
	@echo; echo "Package at $(PACKAGE_FILE)"
	@echo; echo "Now run 'cvs tag $(CVSTAG)'"; echo
