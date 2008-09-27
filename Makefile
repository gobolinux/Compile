
PROGRAM=Compile
VERSION=1.11.1
PACKAGE_DIR=$(HOME)
PACKAGE_FILE=$(PACKAGE_DIR)/$(PROGRAM)--$(VERSION)--$(shell uname -m).tar.bz2
goboPrograms ?= /Programs
PREFIX=
DESTDIR=$(goboPrograms)/$(PROGRAM)/$(VERSION)
SVNTAG:=$(shell echo $(PROGRAM)_$(VERSION) | tr "[:lower:]" "[:upper:]" | sed  's,\.,_,g')
ifeq (,$(findstring svn,$(VERSION)))
INSTALL_TARGET=install-files
else
INSTALL_TARGET=install-svn
endif

all_files = $(shell ListProgramFiles `pwd`)
man_files = $(shell cd bin; grep -l Parse_Options * | xargs -i echo Shared/man/man1/{}.1)

default:

version_check:
ifeq (,$(findstring svn,$(VERSION)))
	@[ "$(VERSION)" = "" ] && { echo -e "Error: run make with VERSION=<version-number>.\n"; exit 1 ;} || exit 0
else
	@echo "You must specify a version when you run \"make dist\""
	@exit 1
endif

clean: cleanup

cleanup:
	rm -rf Resources/FileHash*
	find * -path "*~" -or -path "*/.\#*" -or -path "*.bak" | xargs rm -f

verify:
	@svn update
	@{ svn status 2>&1 | grep -v "Resources/SettingsBackup" | grep "^\?" ;} && { echo -e "Error: unknown files exist. Please take care of them first.\n"; exit 1 ;} || exit 0
	@{ svn status 2>&1 | grep "^M" ;} && { echo -e "Error: modified files exist. Please checkin/revert them first.\n"; exit 1 ;} || exit 0

dist: version_check verify manuals tarball
	@echo; echo "Press enter to create a subversion tag for version $(VERSION) or ctrl-c to abort."
	@read
	@make tag

tag: verify
	svn cp http://svn.gobolinux.org/tools/trunk/$(PROGRAM) http://svn.gobolinux.org/tools/tags/$(SVNTAG) -m\"Tagging $(PROGRAM) $(VERSION)\"
	@echo "Switching to tag (http://svn.gobolinux.org/tools/tags/$(SVNTAG))"
	@svn switch http://svn.gobolinux.org/tools/tags/$(SVNTAG)
	sed -i 's/^VERSION=.*/VERSION='"$(VERSION)"'/' Makefile
	svn commit -m"Updating version in Makefile." Makefile
	@echo "Switching back to trunk"
	@svn switch http://svn.gobolinux.org/tools/trunk/$(PROGRAM)

tarball: $(PACKAGE_FILE)
	@echo; echo "Tarball at $(PACKAGE_FILE)"

$(PACKAGE_FILE): $(all_files)
	rm -rf $(PACKAGE_DIR)/$(PROGRAM)/$(VERSION)
	mkdir -p $(PACKAGE_DIR)/$(PROGRAM)/$(VERSION)
	@ListProgramFiles `pwd` | cpio -p $(PACKAGE_DIR)/$(PROGRAM)/$(VERSION)
	cd $(PACKAGE_DIR)/$(PROGRAM)/$(VERSION); make clean default
	cd $(PACKAGE_DIR); tar cvp $(PROGRAM)/$(VERSION) | bzip2 > $(PACKAGE_FILE)
	rm -rf $(PACKAGE_DIR)/$(PROGRAM)/$(VERSION)
	rmdir $(PACKAGE_DIR)/$(PROGRAM)
	SignProgram $(PACKAGE_FILE)

manuals: $(man_files)

$(man_files): Shared/man/man1/%.1: bin/%
	@mkdir -p Shared/man/man1
	help2man --name=" " --source="GoboLinux" --no-info $< --output $@

install: $(INSTALL_TARGET)

install-files:
	install -d -m 755 $(PREFIX)$(DESTDIR)
	@echo "Copying files"
	@ListProgramFiles `pwd` | cpio --pass-through --quiet --verbose --unconditional $(PREFIX)$(DESTDIR)

install-svn: install-files
	@echo "Installing subversion information"
	@find -name ".svn" -exec echo "$(PREFIX)$(DESTDIR)/{}" \; -exec cp --archive --remove-destination '{}' "$(PREFIX)$(DESTDIR)"/'{}' \;

.PHONY: default version_check clean cleanup verify dist tag tarball

.PHONY: manuals install install-files install-svn
