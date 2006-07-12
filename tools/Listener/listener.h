#ifndef LISTENER_H
#define LISTENER_H 1

#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <dirent.h>
#include <limits.h>
#include <regex.h>
#include <ftw.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#define _GNU_SOURCE
#include <getopt.h>
#include "inotify.h"
#include "inotify-syscalls.h"


#ifndef SYSCONFDIR
#define SYSCONFDIR      "/System/Settings"
#endif

#define LISTENER_RULES  SYSCONFDIR"/listener.conf"
#define EMPTY_MASK      IN_ONESHOT

#define FILTER_DIRS(m)  S_ISDIR(m)
#define FILTER_FILES(m) S_ISREG(m)

struct thread_info {
	int di_index;					/* the struct directory_info's index */
	char offending_name[PATH_MAX];	/* the file/directory entry we're dealing with */
};

struct directory_info {
	char pathname[PATH_MAX];	/* the pathname being listened */
	int mask;					/* CLOSE_WRITE, MOVED_TO, MOVED_FROM or DELETE */
	char exec_cmd[LINE_MAX];	/* shell command to spawn when triggered */
	regex_t regex;				/* regular expression used to filter {file,dir} names */
	char recursive;				/* recursive flag */

	int wd;						/* this pathname's watch file descriptor */
	int filter;					/* while reading the directory, only look at this kind of entries */
	int depends_on_entry;		/* tells if exec_cmd depends on $ENTRY being still valid to perform its action */
};

#endif /* LISTENER_H */
