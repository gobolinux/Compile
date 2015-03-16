
PROGRAM=Compile
VERSION=git-$(shell date +%Y%m%d)
goboPrograms ?= /Programs
PREFIX=
DESTDIR=$(goboPrograms)/$(PROGRAM)/$(VERSION)

man_files = $(shell cd bin; grep -l Parse_Options * | xargs -i echo share/man/man1/{}.1)

default: manuals

clean:
	rm -rf Resources/FileHash*
	find * -path "*~" -or -path "*/.\#*" -or -path "*.bak" | xargs rm -f

manuals: $(man_files)

$(man_files): share/man/man1/%.1: bin/%
	@mkdir -p share/man/man1
	help2man --name=" " --source="GoboLinux" --no-info $< --output $@

install: manuals
	install -d -m 755 $(PREFIX)$(DESTDIR)
	@echo "Copying files"
	@ListProgramFiles `pwd` | cpio --pass-through --quiet --verbose --unconditional $(PREFIX)$(DESTDIR)

