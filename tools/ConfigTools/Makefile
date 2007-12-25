VERSION=
PROGRAM=ConfigTools

PACKAGE_DIR=$(HOME)
PACKAGE_ROOT=$(PACKAGE_DIR)/$(PROGRAM)
PACKAGE_VDIR=$(PACKAGE_ROOT)/$(VERSION)
PACKAGE_FILE=$(PACKAGE_DIR)/$(PROGRAM)--$(VERSION)--$(shell uname -m).tar.bz2
CVSTAG=`echo $(PROGRAM)_$(VERSION) | tr "[:lower:]" "[:upper:]" | sed  's,\.,_,g'`

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
	cd $(PYTHON_SITE) && \
	for f in *.py; \
		do python -c "import `basename $$f .py`"; \
	done

version_check:
	@[ "$(VERSION)" = "" ] && { echo -e "Error: run make with VERSION=<version-number>.\n"; exit 1 ;} || exit 0

clean: cleanup
	rm -rf lib

cleanup:
	rm -rf Resources/FileHash*
	find * -path "*~" -or -path "*/.\#*" -or -path "*.bak" | xargs rm -f
	make -C src clean
	cd $(PYTHON_SITE) && rm -f *.pyc *.pyo;

verify:
	! { cvs up -dP 2>&1 | grep "^[\?]" | grep -v "? Resources/SettingsBackup\|? lib\|? bin/raw_mouseinfo" ;}

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
