
VERSION=
PROGRAM=Compile
SCRIPTS_DIR=/Programs/$(PROGRAM)/Current
PACKAGE_DIR=$(HOME)
PACKAGE_FILE=$(PACKAGE_DIR)/$(PROGRAM)--$(VERSION)--$(shell uname -m).tar.bz2
DESTDIR=/Programs/Compile/$(VERSION)/
SVNTAG=`echo $(PROGRAM)_$(VERSION) | tr "[:lower:]" "[:upper:]" | sed  's,\.,_,g'`

default:

version_check:
	@[ "$(VERSION)" = "" ] && { echo -e "Error: run make with VERSION=<version-number>.\n"; exit 1 ;} || exit 0

clean: cleanup

cleanup:
	rm -rf Resources/FileHash*
	find * -path "*~" -or -path "*/.\#*" -or -path "*.bak" | xargs rm -f

verify:
	@svn update
	@{ svn status 2>&1 | grep -v "Resources/SettingsBackup" | grep "^\?" ;} && { echo -e "Error: unknown files exist. Please take care of them first.\n"; exit 1 ;} || exit 0
	@{ svn status 2>&1 | grep "^M" ;} && { echo -e "Error: modified files exist. Please checkin/revert them first.\n"; exit 1 ;}

dist: version_check cleanup verify
	rm -rf $(PACKAGE_DIR)/$(PROGRAM)/$(VERSION)
	mkdir -p $(PACKAGE_DIR)/$(PROGRAM)/$(VERSION)
	ListProgramFiles $(shell pwd) | cpio -p $(PACKAGE_DIR)/$(PROGRAM)/$(VERSION)
	cd $(PACKAGE_DIR)/$(PROGRAM)/$(VERSION); make default
	cd $(PACKAGE_DIR); tar cvp $(PROGRAM)/$(VERSION) | bzip2 > $(PACKAGE_FILE)
	rm -rf $(PACKAGE_DIR)/$(PROGRAM)/$(VERSION)
	rmdir $(PACKAGE_DIR)/$(PROGRAM)
	SignProgram $(PACKAGE_FILE)
	@echo; echo "Package at $(PACKAGE_FILE)"
	@echo; echo "Now make a tag by running \`svn cp http://svn.gobolinux.org/tools/trunk/$(PROGRAM) http://svn.gobolinux.org/tools/tags/$(SVNTAG) -m\"Tagging $(PROGRAM) $(VERSION)\"\`"

manuals:
	mkdir -p man/man1
	for i in `cd bin && grep -l Parse_Options *`; do bn=`basename $$i`; help2man --name=" " --source="GoboLinux" --no-info $$bn > man/man1/$$bn.1; done

install: version_check
	cp -a * $(DESTDIR)
