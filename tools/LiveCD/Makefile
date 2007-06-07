
VERSION=
PROGRAM=LiveCD
PACKAGE_DIR=$(HOME)
PACKAGE_ROOT=$(PACKAGE_DIR)/$(PROGRAM)
PACKAGE_BASE=$(PACKAGE_ROOT)/$(VERSION)
PACKAGE_FILE=$(PACKAGE_DIR)/$(PROGRAM)--$(VERSION)--$(shell uname -m).tar.bz2
CVSTAG=`echo $(PROGRAM)_$(VERSION) | tr "[:lower:]" "[:upper:]" | sed  's,\.,_,g'`

LANG_TEMP_DIR=Data/Language/.Temp

all: language
	cd src; make all

language:
	[ -e "$(LANG_TEMP_DIR)" ] || mkdir $(LANG_TEMP_DIR)
	for file in ConfigureLiveCD KeymapDialog; \
	do cat bin/$$file | sed "s/tr \(.*\)/tr\(\1\)/g" > $(LANG_TEMP_DIR)/$$file; \
	done
	cd $(LANG_TEMP_DIR)/../; pylupdate LiveCD.pro 2> /dev/null
	rm -f $(LANG_TEMP_DIR)/*
	rmdir $(LANG_TEMP_DIR)

snapshot: VERSION = `date +%Y%m%d`-snapshot
snapshot: dist

version_check:
	@[ "$(VERSION)" = "" ] && { echo -e "Error: run make with VERSION=<version-number>.\n"; exit 1 ;} || exit 0

cleanup:
	rm -rf Resources/FileHash*
	find * -path "*~" -or -path "*/.\#*" | xargs rm -f
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
	@echo; echo "Package at $(PACKAGE_FILE)"
	@echo; echo "Now run 'cvs tag $(CVSTAG)'"; echo
	! { cvs up 2>&1 | grep "^M" ;}

