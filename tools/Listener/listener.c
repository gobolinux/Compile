/*
 * Listener - Listens for specific directories events and take actions 
 * based on rules specified by the user.
 *
 * Copyright (c) 2005  Lucas Correia Villa Real <lucasvr@gobolinux.org>
 * 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * Changelog
 *    11/07/2006  lucasvr   Included initial support for recursive rules.
 *    15/08/2005  lucasvr   Included support for variables on listener.conf;
 *                          Allowing the user to specify the config file;
 *                          Recognizing multiple declarations of the same target.
 *    01/07/2005  hisham    Gave to lucasvr a birthday's gift, replacing
 *                          the ugly system() call by a nice execvp() one :-)
 *    01/07/2005  lucasvr   Moved from fcntl to inotify!
 *    29/06/2005  lucasvr   First version
 */
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

static struct directory_info **dir_info;
static int inotify_fd;

/* TODO: handle SIGHUP */
void
suicide(int signum)
{
	int i;

	for (i = 0; dir_info[i] != NULL; ++i) {
		regfree(&dir_info[i]->regex);
		free(dir_info[i]);
	}
	free(dir_info);

	close(inotify_fd);
	exit(EXIT_SUCCESS);
}

char *
get_token(char *cmd, int *skip_bytes, char *pathname, struct thread_info *info)
{
	int i=0, j=0, skip=0;
	char line[LINE_MAX], work_line[LINE_MAX];
	char *entry_ptr, *ptr;

	if (! cmd || ! strlen(cmd)) {
		*skip_bytes = 0;
		return NULL;
	}

	memset(line, 0, sizeof(line));
	memset(work_line, 0, sizeof(work_line));

	while (isblank(*cmd)) {
		cmd++;
		skip++;
	}
	while (*cmd && ! isblank(*cmd)) {
		line[i++] = *(cmd++);
		skip++;
	}
	*skip_bytes = skip;

	if ((entry_ptr = strstr(line, "$ENTRY_RELATIVE"))) {
		int wi = 0;
		for (ptr=line; ptr != entry_ptr; ptr++)
			work_line[wi++] = (*ptr)++;

		for (j=0; j<strlen(info->offending_name); ++j)
			work_line[wi++] = info->offending_name[j];

		/* skip '$ENTRY_RELATIVE' and copy the remaining data */
		for (ptr+=15; *ptr; ptr++)
			work_line[wi++] = (*ptr)++;

		return strdup(work_line);

	} else if ((entry_ptr = strstr(line, "$ENTRY"))) {
		int wi = 0;
		for (ptr=line; ptr != entry_ptr; ptr++)
			work_line[wi++] = (*ptr)++;

		for (j=0; j<strlen(pathname); ++j)
			work_line[wi++] = pathname[j];

		work_line[wi++] = '/';

		for (j=0; j<strlen(info->offending_name); ++j)
			work_line[wi++] = info->offending_name[j];

		/* skip '$ENTRY' and copy the remaining data */
		for (ptr+=6; *ptr; ptr++)
			work_line[wi++] = (*ptr)++;

		return strdup(work_line);
	}
	return strdup(line);
}

void *
perform_action(void *thread_info)
{
	pid_t pid;
	char pathname[PATH_MAX];
	struct thread_info *info = (struct thread_info *) thread_info;
	int i = info->di_index;

	snprintf(pathname, sizeof(pathname), "%s/%s", dir_info[i]->pathname, info->offending_name);
	free(info);

	pid = fork();
	if (pid == 0) {
		char **exec_array, *cmd = dir_info[i]->exec_cmd;
		char exec_cmd[LINE_MAX];
		int len = strlen(cmd);
		int skipped = 0;

		memset(exec_cmd, 0, sizeof(exec_cmd));
		while (2) {
			int skip_bytes = 0;
			char *token = get_token(cmd, &skip_bytes, dir_info[i]->pathname, info);
			if (! token)
				break;

			cmd += skip_bytes;
			skipped += skip_bytes;

			strcat(exec_cmd, token);
			strcat(exec_cmd, " ");
			free(token);

			if (skipped >= len)
				break;
		}
		exec_array = (char **) malloc(4 * sizeof(char *));
		exec_array[0] = "/bin/sh";
		exec_array[1] = "-c";
		exec_array[2] = strdup(exec_cmd);
		exec_array[3] = NULL;
#ifdef DEBUG	
		for (i = 0; exec_array[i] != NULL; ++i)
			fprintf(stderr, "token: '%s'\n", exec_array[i]);
#endif
		execvp(exec_array[0], exec_array);

	} else if (pid > 0) {
		waitpid(pid, NULL, WUNTRACED);
	} else {
		perror("fork");
	}

	pthread_exit(NULL);
}

int
dir_info_index(int index_start, int wd)
{
	int i;

	for (i = index_start; dir_info[i] != NULL; ++i)
		if (dir_info[i]->wd == wd)
			return i;

	return -1;
}

void
select_on_inotify(void)
{
	int ret;
	fd_set read_fds;

	FD_ZERO(&read_fds);
	FD_SET(inotify_fd, &read_fds);

	ret = select(inotify_fd + 1, &read_fds, NULL, NULL, NULL);
	if (ret == -1)
		perror("select");
}

char *
mask_name(int mask)
{
	switch(mask) {
		case IN_MOVED_FROM:
			return "moved from";
		case IN_MOVED_TO:
			return "moved to";
		case IN_CLOSE_WRITE:
			return "close write";
		case IN_CREATE:
			return "create";
		case IN_MODIFY:
			return "modify";
		case IN_DELETE:
			return "delete";
		default:
			return "unknown";
	}
}

void
treat_events(struct inotify_event *ev)
{
	pthread_t tid;
	regmatch_t match;
	int i, ret, start_index;
	struct thread_info *info;
	struct stat status;
	char stat_pathname[PATH_MAX], offending_name[PATH_MAX];

	start_index = i = 0;

	while (dir_info[i] != NULL) {
		i = dir_info_index(start_index, ev->wd);
		if (i < 0) {
			/* Couldn't find watch descriptor, so this is not a valid event */
			break;
		}

		/* update start_index so that the next search starts on the next entry */
		start_index = i+1;

		if (ev->len > PATH_MAX)
			ev->len = PATH_MAX;

		/* 
		 * firstly, check against the watch mask, since a given entry can be
		 * watched twice or even more times
		 */
		if (! (dir_info[i]->mask & ev->mask)) {
			/* no match, ignore this event for this descriptor */
			continue;
		}

		/* verify against regex if we want to handle this event or not */
		memset(offending_name, 0, sizeof(offending_name));
		snprintf(offending_name, ev->len, "%s", ev->name);
		ret = regexec(&dir_info[i]->regex, offending_name, 1, &match, 0);
		if (ret != 0) {
			/* no match, ignore this event for this descriptor */
			continue;
		}

		/* filter the entry by its type */
		snprintf(stat_pathname, sizeof(stat_pathname), "%s/%s", dir_info[i]->pathname, offending_name);
		ret = stat(stat_pathname, &status);
		if (ret < 0 && dir_info[i]->depends_on_entry) {
			fprintf(stderr, "stat %s: %s\n", stat_pathname, strerror(errno));
			continue;
		}
		if (FILTER_DIRS(dir_info[i]->filter) && 
				(dir_info[i]->depends_on_entry && (! S_ISDIR(status.st_mode))))
			continue;
		if (FILTER_FILES(dir_info[i]->filter) && 
				(dir_info[i]->depends_on_entry && (! S_ISREG(status.st_mode))))
			continue;
#ifdef DEBUG	
		printf("-> event on    %s\n", dir_info[i]->pathname);
		printf("-> filename:   %s\n", offending_name);
		printf("-> event mask: %#X (%s)\n\n", ev->mask, mask_name(ev->mask));
#endif
		/* launches a thread to deal with the event */
		info = (struct thread_info *) malloc(sizeof(struct thread_info));
		info->di_index = i;
		snprintf(info->offending_name, sizeof(info->offending_name), "%s", offending_name);
		pthread_create(&tid, NULL, perform_action, (void *) info);
	}
}

void
listen_for_events(void)
{
	struct inotify_event event[128];
	size_t n;
	int evnum;

	while (2) {
		select_on_inotify();
		n = read(inotify_fd, event, sizeof(event));
		if (n < 0) {
			perror("read");
			break;
		} else if (n == 0) {
			continue;
		}

		for (evnum = 0; evnum < n/sizeof(struct inotify_event); ++evnum)
			treat_events(&event[evnum]);
	}
}

/* TODO: monitor_index vs dirinfo_index */
static int monitor_index;
//static int dirinfo_index;
static uint32_t dirinfo_mask;

int
walk_tree(const char *file, const struct stat *sb, int flag)
{
	if (flag == FTW_D) {
//		int i = dirinfo_index;

		/* is a subdirectory */
		dir_info[monitor_index]->wd = inotify_add_watch(inotify_fd, file, dirinfo_mask);
		fprintf(stdout, "[recursive] Monitoring %s on watch %d\n", file, monitor_index);
		monitor_index++;
	}
	return 0;
}

static int num_subdirs = 1;

int
count_subdirs(const char *file, const struct stat *sb, int flag)
{
	if (flag == FTW_D) {
		/* is a subdirectory */
		printf("-> %s\n", file);
		num_subdirs++;
	}
	return 0;
}

int
monitor_directory(int i)
{
	int j;
	uint32_t mask, current_mask;

	/* 
	 * Check for the existing entries if this directory is already being listened.
	 * If that's true, then we must append a new mask instead of replacing the
	 * current one.
	 */
	for (current_mask = 0, j = 0; j < i; ++j) {
		if (! strcmp(dir_info[i]->pathname, dir_info[j]->pathname))
			current_mask |= dir_info[j]->mask;
	}

	mask = dir_info[i]->mask | current_mask;
	dir_info[i]->wd = inotify_add_watch(inotify_fd, dir_info[i]->pathname, mask);

	if (dir_info[i]->recursive) {
		//dirinfo_index = i;
		dirinfo_mask = mask;
		ftw(dir_info[i]->pathname, walk_tree, 1024);
	} else {
		monitor_index++;
		fprintf(stdout, "Monitoring %s\n", dir_info[i]->pathname);
	}
	return 0;
}

int
parse_masks(char *masks, int rule)
{
	int ret = EMPTY_MASK;

	if ((strstr(masks, "CLOSE_WRITE")))
		ret |= IN_CLOSE_WRITE;
	if ((strstr(masks, "MOVED_TO")))
		ret |= IN_MOVED_TO;
	if ((strstr(masks, "DELETE")))
		ret |= IN_DELETE;
	if ((strstr(masks, "CREATE")))
		ret |= IN_CREATE;
	if ((strstr(masks, "MODIFY")))
		ret |= IN_MODIFY;

	return ret;
}

int
expect_rule_start(FILE *fp)
{
	char *token;
	char buf[LINE_MAX];

	while (! feof (fp)) {
		memset(buf, 0, sizeof(buf));
		fgets(buf, sizeof(buf), fp);
		if ((buf == NULL) || (buf[0] == '#') || ((strlen(buf)) == 0))
			continue;

		token = strtok(buf, " \t\n");
		if (token == NULL)
			continue;

		if (token[strlen(token)-1] == '\n')
			token[strlen(token)-1] = '\0';

		if (! strcmp (token, "{"))
			return 0;
		else
			break;
	}

	return -1;
}

int
expect_rule_end(FILE *fp)
{
	char *token;
	char buf[LINE_MAX];

	while (! feof (fp)) {
		memset(buf, 0, sizeof(buf));
		fgets(buf, sizeof(buf), fp);
		if ((buf == NULL) || (buf[0] == '#') || ((strlen(buf)) == 0))
			continue;

		token = strtok(buf, " \t\n");
		if (token == NULL)
			continue;

		if (token[strlen(token)-1] == '\n')
			token[strlen(token)-1] = '\0';

		if (! strcmp (token, "}"))
			return 0;
		else
			break;
	}

	return -1;
}

char *
get_rule_for(char *entry, FILE *fp)
{
	char *token = NULL;
	char buf[LINE_MAX];

	while (! feof (fp)) {
		memset(buf, 0, sizeof(buf));
		fgets(buf, sizeof(buf), fp);
		if ((buf == NULL) || (buf[0] == '#') || ((strlen(buf)) == 0))
			continue;
		else if ((buf[0] == '{') || (buf[0] == '}'))
			return NULL;
		else
			break;
	}

	/* check for ENTRY match */
	if (! strstr (buf, entry))
		return NULL;

	token = strtok(buf, "=");
	if (! token)
		return NULL;

	/* get the RULE associated with ENTRY */
	token = token + strlen(token) + 1;
	while (*token == '\t' || *token == ' ')
		token++;

	if (! token)
		return NULL;

	if (token[strlen(token)-1] == '\n')
		token[strlen(token)-1] = '\0';

	return strdup(token);
}

int
assign_rules(char *config_file)
{
	int i, n, subdirs, ret;
	FILE *fp;
	char *token, regex_rule[LINE_MAX];

	fp = fopen(config_file, "r");
	if (! fp) {
		fprintf(stderr, "fopen %s: %s\n", config_file, strerror(errno));
		return -1;
	}

	/* read how many rules we have */
	n = subdirs = 0;
	while (! feof(fp)) {
		char buf[LINE_MAX];
		char pathname[LINE_MAX];

		memset(buf, 0, sizeof(buf));
		fgets(buf, sizeof(buf), fp);
		if ((buf == NULL) || (buf[0] == '#') || ((strlen(buf)) == 0))
			continue;
		else if (buf[0] == '{')
			n++;
		else if (strstr(buf, "TARGET")) {
			char *token = strtok(buf, " \t");
			token = strtok(NULL, " \t");
			token = strtok(NULL, " \t");
			if (! token) {
				fprintf(stderr, "Error: one or more TARGET entries don't have a value assigned to\n");
				return -1;
			}
			token[strlen(token)-1] = '\0';
			sprintf(pathname, token);

		} else if (strstr(buf, "RECURSIVE") && strstr(buf, "YES")) {
			ftw(pathname, count_subdirs, 1024);
			subdirs += num_subdirs;
			num_subdirs = 0;
		}
	}

	/* there's no rules at all */
	if (n == 0)
		return 0;

	rewind(fp);

	dir_info = (struct directory_info **) calloc(n+1+num_subdirs, sizeof(struct directory_info *));
	if (! dir_info) {
		perror("calloc");
		return -ENOMEM;
	}

	printf("dir_info alocado com %d neguinhos\n", n+1+num_subdirs);

	/* register the pathname */
	for (monitor_index = 0; monitor_index < n+num_subdirs; /* do not increment */) {
		/* monitor_index is incremented inside monitor_directory() */
		i = monitor_index;

		/* expects to find the '{' character */
		if ((expect_rule_start(fp)) < 0) {
			fprintf(stderr, "Error: could not find the rule start marker '{'\n");
			return -1;
		}

		/* populates the dir_info struct */
		token = get_rule_for("TARGET", fp);
		if (! token) {
			fprintf(stderr, "Error on rule #%d: missing TARGET entry\n", i+1);
			return -1;
		}

		dir_info[i] = (struct directory_info *) calloc(1, sizeof(struct directory_info));
		snprintf(dir_info[i]->pathname, sizeof(dir_info[i]->pathname), token);
		free(token);

		/* register the masks */
		token = get_rule_for("WATCHES", fp);
		if (! token) {
			fprintf(stderr, "Error on rule #%d: missing WATCHES entry\n", i+1);
			return -1;
		}

		dir_info[i]->mask = parse_masks(token, i+1);
		if (dir_info[i]->mask == EMPTY_MASK) {
			fprintf(stderr, "Error on rule #%d: invalid WATCH %s\n", i+1, token);
			return -1;
		}
		free(token);

		/* get the exec command */
		token = get_rule_for("SPAWN", fp);
		if (! token) {
			fprintf(stderr, "Error on rule #%d: missing SPAWN command\n", i+1);
			return -1;
		}
		snprintf(dir_info[i]->exec_cmd, sizeof(dir_info[i]->exec_cmd), token);

		/* if there's $ENTRY on the SPAWN command, this rule expects it to exist */
		dir_info[i]->depends_on_entry = (strstr(token, "$ENTRY") == NULL ? 0 : 1);
		free(token);

		/* get the filters */
		token = get_rule_for("LOOKAT", fp);
		if (! token) {
			fprintf(stderr, "Error on rule #%d: missing LOOKAT entry\n", i+1);
			return -1;
		}

		if (! strcasecmp(token, "DIRS"))
			dir_info[i]->filter = S_IFDIR;
		else if (! strcasecmp(token, "FILES"))
			dir_info[i]->filter = S_IFREG;
		else {
			fprintf(stderr, "Error on rule #%d: invalid LOOKAT option %s\n", i+1, token);
			free(token);
			return -1;
		}
		free(token);

		/* get the regex rule */
		token = get_rule_for("ACCEPT_REGEX", fp);
		if (! token) {
			fprintf(stderr, "Error on rule #%d: missing ACCEPT_REGEX entry\n", i+1);
			return -1;
		}

		snprintf(regex_rule, sizeof(regex_rule), "%s", token);
		free(token);

		ret = regcomp(&dir_info[i]->regex, regex_rule, REG_EXTENDED);
		if (ret != 0) {
			char err_msg[256];
			regerror(ret, &dir_info[i]->regex, err_msg, sizeof(err_msg) - 1);
			fprintf(stderr, "Regex error \"%s\": %s\n", regex_rule, err_msg);
			return -1;
		}

		/* get the recursive flag */
		token = get_rule_for("RECURSIVE", fp);
		if (! token) {
			fprintf(stderr, "Error on rule #%d: missing RECURSIVE entry\n", i+1);
			return -1;
		}

		if (! strcasecmp(token, "NO"))
			dir_info[i]->recursive = 0;
		else if (! strcasecmp(token, "YES"))
			dir_info[i]->recursive = 1;
		else {
			fprintf(stderr, "Error on rule #%d: invalid RECURSIVE option %s\n", i+1, token);
			free(token);
			return -1;
		}
		free(token);

		/* expects to find the '}' character */
		if ((expect_rule_end(fp)) < 0) {
			fprintf(stderr, "Error: could not find the rule end marker '}'\n");
			return -1;
		}

		/* create the monitor rules */
		ret = monitor_directory(i);
		if (ret < 0)
			return ret;
	}

	fclose(fp);
	return 0;
}

void
show_usage(char *program_name)
{
	fprintf(stderr, "Usage: %s [options]\n\nAvailable options are:\n"
			"--config, -c <file>    Use config file as specified in <file>\n"
			"--help, -h             This help\n", program_name);
}

static char short_opts[] = "c:h";
static struct option long_options[] = {
	{"config", required_argument, NULL, 'c'},
	{"help",         no_argument, NULL, 'h'},
	{0, 0, 0, 0}
};

int
main(int argc, char **argv)
{
	int ret, c, index;
	char *config_file = NULL;

	/* check for arguments */
	while ((c = getopt_long(argc, argv, short_opts, long_options, &index)) != -1) {
		switch (c) {
			case 0:
			case '?':
				return 1;
			case 'h':
				show_usage(argv[0]);
				return 0;
			case 'c':
				config_file = strdup(optarg);
				break;
			default:
				printf ("invalid option %d\n", c);
				show_usage (argv[0]);
		}
	}

	/* opens the inotify device */
	inotify_fd = inotify_init();
	if (inotify_fd < 0) {
		perror("inotify_init");
		exit(EXIT_FAILURE);
	}

	if (! config_file)
		config_file = strdup(LISTENER_RULES);

	/* read rules from listener.rules */
	ret = assign_rules(config_file);
	if (ret < 0) {
		free(config_file);
		exit(EXIT_FAILURE);
	}

	free(config_file);

	/* install a signal handler to clean up memory */
	signal(SIGINT, suicide);

	listen_for_events();
	exit(EXIT_SUCCESS);
}

/* vim:set ts=4 sts=0 sw=4: */
