AUTOMAKE_OPTIONS = foreign no-dependencies

# Define a C macro LOCALEDIR indicating where catalogs will be installed.
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

# Link time dependencies.
LDADD = @LIBINTL@

bin_PROGRAMS = gobohide
gobohide_SOURCES = gobohide.c
sysconf_DATA = gobohide.conf
