/*
 * Listener - Listens for specific directories events and take actions 
 * based on rules specified by the user.
 *
 * Copyright (c) 2005,2006  Lucas C. Villa Real <lucasvr@gobolinux.org>
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
 */
#include "listener.h"
#include "rules.h"

static struct directory_info *dir_info;
static int inotify_fd;
static int debug_mode;

static void
debug_printf(const char *format, ...)
{
	if (! debug_mode)
		return;
	printf(format);
}

/* TODO: handle SIGHUP */
void
suicide(int signum)
{
	struct directory_info *ptr;

	for (ptr = dir_info; ptr != NULL; ptr = ptr->next)
		regfree(&ptr->regex);
	free(dir_info);

	close(inotify_fd);
	exit(EXIT_SUCCESS);
}

void *
perform_action(void *thread_info)
{
	pid_t pid;
	char pathname[PATH_MAX];
	struct thread_info *info = (struct thread_info *) thread_info;
	struct directory_info *di = info->di;

	snprintf(pathname, sizeof(pathname), "%s/%s", di->pathname, info->offending_name);

	pid = fork();
	if (pid == 0) {
		char **exec_array, *cmd = di->exec_cmd;
		char exec_cmd[LINE_MAX];
		int len = strlen(cmd);
		int skipped = 0;

		memset(exec_cmd, 0, sizeof(exec_cmd));
		while (2) {
			int skip_bytes = 0;
			char *token = get_token(cmd, &skip_bytes, di->pathname, info);
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
		free(info);
		exec_array = (char **) malloc(4 * sizeof(char *));
		exec_array[0] = "/bin/sh";
		exec_array[1] = "-c";
		exec_array[2] = strdup(exec_cmd);
		exec_array[3] = NULL;
#ifdef DEBUG
		{
			int i;
			for (i = 0; exec_array[i] != NULL; ++i)
				fprintf(stderr, "token: '%s'\n", exec_array[i]);
		}
#endif
		execvp(exec_array[0], exec_array);

	} else if (pid > 0) {
		waitpid(pid, NULL, WUNTRACED);
	} else {
		perror("fork");
	}

	pthread_exit(NULL);
}

struct directory_info *
dir_info_index(struct directory_info *start, int wd)
{
	struct directory_info *ptr;

	for (ptr = start; ptr != NULL; ptr = ptr->next)
		if (ptr->wd == wd)
			return ptr;

	return NULL;
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
	char buf[128];
	
	memset(buf, 0, sizeof(buf));
	
	if (mask & IN_ACCESS)
		snprintf(buf, sizeof(buf), "access");
	if (mask & IN_MODIFY)
		snprintf(buf, sizeof(buf), "%s%s", strlen(buf)?" | ":"", "modify");
	if (mask & IN_ATTRIB)
		snprintf(buf, sizeof(buf), "%s%s", strlen(buf)?" | ":"", "attrib");
	if (mask & IN_CLOSE_WRITE)
		snprintf(buf, sizeof(buf), "%s%s", strlen(buf)?" | ":"", "close write");
	if (mask & IN_CLOSE_NOWRITE)
		snprintf(buf, sizeof(buf), "%s%s", strlen(buf)?" | ":"", "close nowrite");
	if (mask & IN_OPEN)
		snprintf(buf, sizeof(buf), "%s%s", strlen(buf)?" | ":"", "open");
	if (mask & IN_MOVED_FROM)
		snprintf(buf, sizeof(buf), "%s%s", strlen(buf)?" | ":"", "moved from");
	if (mask & IN_MOVED_TO)
		snprintf(buf, sizeof(buf), "%s%s", strlen(buf)?" | ":"", "moved to");
	if (mask & IN_CREATE)
		snprintf(buf, sizeof(buf), "%s%s", strlen(buf)?" | ":"", "create");
	if (mask & IN_DELETE)
		snprintf(buf, sizeof(buf), "%s%s", strlen(buf)?" | ":"", "delete");
	if (mask & IN_DELETE_SELF)
		snprintf(buf, sizeof(buf), "%s%s", strlen(buf)?" | ":"", "delete self");
	if (mask & IN_MOVE_SELF)
		snprintf(buf, sizeof(buf), "%s%s", strlen(buf)?" | ":"", "move self");
	if (! strlen(buf))
		snprintf(buf, sizeof(buf), "unknown");

	return strdup(buf);
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
	struct directory_info *di, *ptr;
	char *mask;

	start_index = i = 0;

	for (ptr = dir_info; ptr != NULL; ptr = ptr->next) {
		di = dir_info_index(ptr, ev->wd);
		if (! di) {
			/* Couldn't find watch descriptor, so this is not a valid event */
			break;
		}

		if (ev->len > PATH_MAX)
			ev->len = PATH_MAX;

		/* 
		 * firstly, check against the watch mask, since a given entry can be
		 * watched twice or even more times
		 */
		if (! (di->mask & ev->mask)) {
			debug_printf("event doesn't come from watch descriptor %d\n", di->wd);
			continue;
		}

		/* verify against regex if we want to handle this event or not */
		memset(offending_name, 0, sizeof(offending_name));
		snprintf(offending_name, ev->len, "%s", ev->name);
		ret = regexec(&di->regex, offending_name, 1, &match, 0);
		if (ret != 0) {
			debug_printf("event from watch descriptor %d, but regex doesn't match\n", di->wd);
			continue;
		}

		/* filter the entry by its type */
		snprintf(stat_pathname, sizeof(stat_pathname), "%s/%s", di->pathname, offending_name);
		ret = stat(stat_pathname, &status);
		if (ret < 0 && di->depends_on_entry) {
			fprintf(stderr, "stat %s: %s\n", stat_pathname, strerror(errno));
			continue;
		}
#if 0
		if (FILTER_DIRS(di->filter) && !FILTER_FILES(di->filter) && (di->depends_on_entry &&
		   (! S_ISDIR(status.st_mode)))) {
			debug_printf("watch descriptor %d listens for DIRS rules, which is not the case here\n", di->wd);
			continue;
		}
		
		if (FILTER_FILES(di->filter) && !FILTER_DIRS(di->filter) && (di->depends_on_entry &&
		   (! S_ISREG(status.st_mode)))) {
			debug_printf("watch descriptor %d listens for FILES rules, which is not the case here\n", di->wd);
			continue;
		}
#endif
		mask = mask_name(ev->mask);
		debug_printf("-> event on dir %s, watch %d\n", di->pathname, di->wd);
		debug_printf("-> filename:    %s\n", offending_name);
		debug_printf("-> event mask:  %#X (%s)\n\n", ev->mask, mask);
		free(mask);

		/* launches a thread to deal with the event */
		info = (struct thread_info *) malloc(sizeof(struct thread_info));
		info->di = di;
		snprintf(info->offending_name, sizeof(info->offending_name), "%s", offending_name);
		pthread_create(&tid, NULL, perform_action, (void *) info);

		/* event treated, that's all! */
		break;
	}
}

void
listen_for_events(void)
{
	size_t n;
	int evnum;
	char buf[1024 * (sizeof(struct inotify_event) + 16)];

	while (2) {
		select_on_inotify();
		n = read(inotify_fd, buf, sizeof(buf));
		if (n < 0) {
			perror("read");
			break;
		} else if (n == 0) {
			continue;
		}

		evnum = 0;
		while (evnum < n) {
			struct inotify_event *event = (struct inotify_event *) &buf[evnum];
			treat_events(event);
			evnum += sizeof(struct inotify_event) + event->len;
		}
	}
}

static uint32_t my_root_mask;
static struct directory_info *my_root;

int
walk_tree(const char *file, const struct stat *sb, int flag)
{
	struct directory_info *di;
	
	if (flag != FTW_D) /* isn't a subdirectory */
		return 0;
	
	/* easily replicate the father's exec_cmd, depends_on_entry, mask, filter, regex and recursive members */
	di = (struct directory_info *) calloc(1, sizeof(struct directory_info));
	memcpy(di, my_root, sizeof(*di));

	/* only needs to differentiate on the pathname, regex and watch descriptor */
	snprintf(di->pathname, sizeof(di->pathname), "%s", file);
	regcomp(&di->regex, di->regex_rule, REG_EXTENDED);
	di->wd = inotify_add_watch(inotify_fd, file, my_root_mask);

	my_root->next = di;
	my_root = di;
		
	fprintf(stdout, "[recursive] Monitoring %s on watch %d\n", di->pathname, di->wd);
	return 0;
}

int
monitor_directory(int i, struct directory_info *di)
{
	uint32_t mask, current_mask;
	struct directory_info *ptr;

	/* 
	 * Check for the existing entries if this directory is already being listened.
	 * If that's true, then we must append a new mask instead of replacing the
	 * current one.
	 */
	for (current_mask = 0, ptr = dir_info; ptr != NULL; ptr = ptr->next) {
		if (! strcmp(ptr->pathname, di->pathname))
			current_mask |= ptr->mask;
	}

	mask = di->mask | current_mask;
	
	if (di->recursive) {
		my_root = di;
		my_root_mask = mask;
		ftw(di->pathname, walk_tree, 1024);
	} else {
		di->wd = inotify_add_watch(inotify_fd, di->pathname, mask);
		fprintf(stdout, "Monitoring %s on watch %d\n", di->pathname, di->wd);
	}

	return 0;
}

void
show_usage(char *program_name)
{
	fprintf(stderr, "Usage: %s [options]\n\nAvailable options are:\n"
			"--config, -c <file>    Use config file as specified in <file>\n"
			"--debug, -d            Do not fork and become a daemon\n"
			"--help, -h             This help\n", program_name);
}

static char short_opts[] = "c:dh";
static struct option long_options[] = {
	{"config", required_argument, NULL, 'c'},
	{"debug",        no_argument, NULL, 'd'},
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
			case 'c':
				config_file = strdup(optarg);
				break;
			case 'd':
				printf("Running in debug mode\n");
				debug_mode = 1;
				break;
			case 'h':
				show_usage(argv[0]);
				return 0;
			default:
				printf("invalid option %d\n", c);
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
	dir_info = assign_rules(config_file, &ret);
	if (ret < 0) {
		free(config_file);
		exit(EXIT_FAILURE);
	}

	free(config_file);

	/* install a signal handler to clean up memory */
	signal(SIGINT, suicide);

	if (debug_mode)
		listen_for_events();
	else {
		pid_t id = fork();

		if (id == 0)
			listen_for_events();
		else if (id < 0 ){
			perror("fork");
			exit(EXIT_FAILURE);
		}
	}
	
	exit(EXIT_SUCCESS);
}

/* vim:set ts=4 sts=0 sw=4: */
