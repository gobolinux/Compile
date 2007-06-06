VERSION=
PROGRAM=AbsTK

PACKAGE_DIR=$(HOME)
PACKAGE_ROOT=$(PACKAGE_DIR)/$(PROGRAM)
PACKAGE_BASE=$(PACKAGE_ROOT)/$(VERSION)
PACKAGE_FILE=$(PACKAGE_DIR)/$(PROGRAM)--$(VERSION)--$(shell uname -m).tar.bz2
CVSTAG=`echo $(PROGRAM)_$(VERSION) | tr "[:lower:]" "[:upper:]" | sed  's,\.,_,g'`

PYTHON_VERSION=2.4
PYTHON_LIBS=cwizard qtwizard wizard
PYTHON_SITE=lib/python$(PYTHON_VERSION)/site-packages

all:
	[ -e $(PYTHON_SITE) ] || mkdir -p $(PYTHON_SITE)
	for f in $(PYTHON_LIBS); \
	do libf=$(PYTHON_SITE)/$$f.py; \
	   rm -f $$libf; ln -nfs ../../../src/$$f.py $$libf; \
	done
	
	cd $(PYTHON_SITE) && \
	for f in *.py; \
	do python -c "import `basename $$f .py`" &> /dev/null; \
	done

version_check:
	@[ "$(VERSION)" = "" ] && { echo -e "Error: run make with VERSION=<version-number>.\n"; exit 1 ;} || exit 0

clean: cleanup

cleanup:
	rm -rf Resources/FileHash*
	find * -path "*~" -or -path "*/.\#*" -or -path "*.bak" | xargs rm -f
	for f in $(PYTHON_LIBS); do libf=$(PYTHON_SITE)/$$f; rm -f $$libf.{pyo,pyc,py}; done
	for d in $(PYTHON_SITE) lib/python$(PYTHON_VERSION) lib; \
	do [ -d $$d ] && echo rmdir $$d || true; \
	done

verify:
	! { cvs up -dP 2>&1 | grep "^[\?]" | grep -v "Resources/SettingsBackup" ;}

dist: version_check cleanup verify
	rm -rf $(PACKAGE_ROOT)
	mkdir -p $(PACKAGE_BASE)
	SignProgram $(PROGRAM)
	cat Resources/FileHash
	ListProgramFiles $(PROGRAM) | cpio -p $(PACKAGE_BASE)
	cd $(PACKAGE_DIR); tar cvp $(PROGRAM) | bzip2 > $(PACKAGE_FILE)
	rm -rf $(PACKAGE_ROOT)
	@echo; echo "Package at $(PACKAGE_FILE)"
	@echo; echo "Now run 'cvs tag $(CVSTAG)'"; echo
	! { cvs up -dP 2>&1 | grep "^M" ;}

