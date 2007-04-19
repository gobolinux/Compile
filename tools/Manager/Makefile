
 # $@ is the name of the file to be made. 
 # $? is the names of the changed dependents.
 # $< the name of the related file that caused the action. 
 # $* the prefix shared by target and dependent files.

GUI_DIR=GUI
LIB_DIR=lib/python2.3/site-packages
BIN_DIR=bin

all: guis libs

guis : $(LIB_DIR)/ManagerUI.py $(LIB_DIR)/ManagerConfigForm.py $(LIB_DIR)/GenericEditor.py

$(LIB_DIR)/GenericEditor.py: $(GUI_DIR)/GenericEditor.ui
	pyuic $? >$@

$(LIB_DIR)/ManagerUI.py: $(GUI_DIR)/ManagerUI.ui
	mkdir -p $(LIB_DIR)
	pyuic $? > $@

$(LIB_DIR)/ManagerConfigForm.py: $(GUI_DIR)/ManagerConfigForm.ui
	pyuic $? > $@


libs : $(LIB_DIR)/GetInstalled.py $(LIB_DIR)/ManagerRunner.py compilation


$(LIB_DIR)/GetInstalled.py : $(BIN_DIR)/GetInstalled
	cd $(LIB_DIR) && ln -sfn ../../../bin/GetInstalled ./GetInstalled.py

$(LIB_DIR)/ManagerRunner.py : $(BIN_DIR)/ManagerRunner
	cd $(LIB_DIR) && ln -sfn ../../../bin/ManagerRunner ./ManagerRunner.py

compilation :
	cd $(LIB_DIR) && \
	for f in *.py; \
	do python -c "import `basename $$f .py`"; \
	done

clean :
	rm -f $(LIB_DIR)/*.pyo $(LIB_DIR)/*.pyc $(LIB_DIR)/*.py

VERSION=
PROGRAM=Manager
PACKAGE_DIR=$(HOME)
PACKAGE_ROOT=$(PACKAGE_DIR)/$(PROGRAM)
PACKAGE_VDIR=$(PACKAGE_ROOT)/$(VERSION)
PACKAGE_FILE=$(PACKAGE_DIR)/$(PROGRAM)--$(VERSION)--$(shell uname -m).tar.bz2
TARBALL_BASE=$(PROGRAM)-$(VERSION)
TARBALL_ROOT=$(PACKAGE_DIR)/$(TARBALL_BASE)
TARBALL_FILE=$(PACKAGE_DIR)/$(PROGRAM)-$(VERSION).tar.gz
DESTDIR=/Programs/$(PROGRAM)/$(VERSION)/
CVSTAG=`echo $(PROGRAM)_$(VERSION) | tr "[:lower:]" "[:upper:]" | sed  's,\.,_,g'`

version_check:
	@[ "$(VERSION)" = "" ] && { echo -e "Error: run make with VERSION=<version-number>.\n"; exit 1 ;} || exit 0

verify:
	! { cvs up -dP 2>&1 | grep "^[\?]" | grep -v "Resources/SettingsBackup" ;}

cleanup:
	rm -rf Resources/FileHash*
	find * -path "*~" -or -path "*/.\#*" -or -path "*.bak" | xargs rm -f
	cd $(LIB_DIR) && rm -f *.pyc *.pyo *.py

dist: version_check cleanup verify all
	rm -rf $(PACKAGE_ROOT)
	mkdir -p $(PACKAGE_VDIR)
	SignProgram $(PROGRAM)
	cat Resources/FileHash
	ListProgramFiles $(PROGRAM) | cpio -p $(PACKAGE_VDIR)
	cd $(PACKAGE_DIR); tar cvp $(PROGRAM) | bzip2 > $(PACKAGE_FILE)
	rm -rf $(PACKAGE_ROOT)
	@echo; echo "Package at $(PACKAGE_FILE)"
	@echo; echo "Now run 'cvs tag $(CVSTAG)'"; echo
	! { cvs up -dP 2>&1 | grep "^M" ;}
