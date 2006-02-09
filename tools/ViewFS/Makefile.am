
bin_PROGRAMS = viewfs
EXTRA_DIST = GlobalView

AM_CFLAGS = -std=c99 -D_FILE_OFFSET_BITS=64

viewfs_SOURCES = src/viewfs.c   src/viewfs.h \
                 src/events.c   src/events.h \
                 src/cmdline.c  src/cmdline.h \
                 src/tree.c     src/tree.h \
                 src/watchdir.c src/watchdir.h \
                 src/inotify.c  src/inotify.h
