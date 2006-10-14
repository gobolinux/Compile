
VERSION=
PROGRAM=Scripts
PACKAGE_DIR=$(HOME)
PACKAGE_ROOT=$(PACKAGE_DIR)/$(PROGRAM)
PACKAGE_BASE=$(PACKAGE_ROOT)/$(VERSION)
PACKAGE_FILE=$(PACKAGE_DIR)/$(PROGRAM)--$(VERSION)--$(shell uname -m).tar.bz2

PYTHON_VERSION=2.3
PYTHON_LIBS=FindPackage GetAvailable GuessLatest CheckDependencies DescribeProgram
PYTHON_SITE=lib/python$(PYTHON_VERSION)/site-packages

all: python
	sed -i~ "s/CURRENT_SCRIPTS_VERSION=.*#/CURRENT_SCRIPTS_VERSION="${VERSION}" #/g" bin/CreateRootlessEnvironment
	rm -f bin/CreateRootlessEnvironment~
	cd src; make all

debug: python
	cd src; make debug

python:
	for f in $(PYTHON_LIBS); \
	do libf=$(PYTHON_SITE)/$$f.py; \
	   rm -f $$libf; ln -nfs ../../../bin/$$f $$libf; \
	done

version_check:
	@[ "$(VERSION)" = "" ] && { echo -e "Error: run make with VERSION=<version-number>.\n"; exit 1 ;} || exit 0

clean: cleanup

cleanup:
	rm -rf Resources/FileHash*
	find * -path "*~" -or -path "*/.\#*" -or -path "*.bak" | xargs rm -f
	cd src; make clean

verify:
	! { cvs up -dP 2>&1 | grep "^[\?]" | grep -v "Resources/SettingsBackup" ;}

dist: version_check cleanup verify all
	rm -rf $(PACKAGE_ROOT)
	mkdir -p $(PACKAGE_BASE)
	SignProgram $(PROGRAM)
	cat Resources/FileHash
	ListProgramFiles $(PROGRAM) | cpio -p $(PACKAGE_BASE)
	cd $(PACKAGE_DIR); tar cvp $(PROGRAM) | bzip2 > $(PACKAGE_FILE)
	rm -rf $(PACKAGE_ROOT)
	@echo; echo "Package at $(PACKAGE_FILE)"; echo
	! { cvs up -dP 2>&1 | grep "^M" | grep -v CreateRootlessEnvironment ;}

