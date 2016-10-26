PROGRAM = Compile
VERSION = git-$(shell date +%Y%m%d)
goboPrograms ?= /Programs
PREFIX ?=
DESTDIR = $(PREFIX)/$(goboPrograms)/$(PROGRAM)/$(VERSION)
INSTALL_DIR = install -d
INSTALL_FILE = install
man_files = $(shell grep -l Parse_Options bin/* | xargs -i echo {}.1)
scripts = ApplyVariables Compile FetchArchive GoboPath2Ruby NewVersion PrepareProgram  AutoPatch ContributeRecipe FiboSandbox NoRecipe RecipeLint UnionSandbox ColorMake EditRecipe GetRecipe MakeRecipe PackRecipe SandboxInstall UpdateRecipes 

.PHONY: all clean install uninstall

default: all

manuals: $(man_files)

$(man_files): %.1: %
	@echo "Generating man page $@"
	help2man --name=" " --source="GoboLinux" --no-info $< --output $@

clean_manuals:
	@echo "Cleaning man pages"
	echo $(man_files) | tr '\n' ' ' | xargs -d ' ' -n 1 rm -f

clean_resources:
	rm -rf Resources/FileHash*

clean: clean_manuals clean_resources

install_manuals: manuals
	@$(INSTALL_DIR) -d -m 755 $(DESTDIR)/share/man/man1
	echo $(man_files) | tr '\n' ' ' | xargs -d ' ' -n 1 -i \
		$(INSTALL_FILE) -m 644 {} $(DESTDIR)/share/man/man1

install_scripts:
	@echo "Installing scripts"
	$(INSTALL_DIR) -d -m 755 $(DESTDIR)/bin
	echo $(scripts) | tr '\n' ' ' | xargs -d ' ' -n 1 -i \
	   	$(INSTALL_FILE) -m 755 bin/{} $(DESTDIR)/bin

install_lib:
	@echo "Installing lib"
	$(INSTALL_DIR) -d -m 755 $(DESTDIR)
	cp -r lib $(DESTDIR) 
			
install_resources:
	@echo "Installing Resources"
	cp -r Resources $(DESTDIR) 
			
install_share_data:
	@echo "Installing share data"
	cp -rf share $(DESTDIR)

install_functions:
	@echo "Installing Functions"
	cp -rf Functions $(DESTDIR)

install_docs:
	@echo "Installing docs"
	cp -rf doc $(DESTDIR)
	cp -rf examples $(DESTDIR)/doc

prepare_install:
	@echo "Installing $(PROGRAM) into $(DESTDIR)"
	$(INSTALL_DIR) -d -m 755 $(DESTDIR)

install: install_scripts install_manuals prepare_install install_lib install_resources install_share_data install_functions
	@echo "Installed $(PROGRAM) into $(DESTDIR)"
