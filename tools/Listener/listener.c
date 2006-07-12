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

		dprintf("-> event on    %s\n", dir_info[i]->pathname);
		dprintf("-> filename:   %s\n", offending_name);
		dprintf("-> event mask: %#X (%s)\n\n", ev->mask, mask_name(ev->mask));

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
int monitor_index;
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

int
monitor_directory(int i, struct directory_info **di)
{
	int j;
	uint32_t mask, current_mask;

	/* 
	 * Check for the existing entries if this directory is already being listened.
	 * If that's true, then we must append a new mask instead of replacing the
	 * current one.
	 */
	for (current_mask = 0, j = 0; j < i; ++j) {
		if (! strcmp(di[i]->pathname, di[j]->pathname))
			current_mask |= di[j]->mask;
	}

	mask = di[i]->mask | current_mask;
	di[i]->wd = inotify_add_watch(inotify_fd, di[i]->pathname, mask);

	if (di[i]->recursive) {
		//dirinfo_index = i;
		dirinfo_mask = mask;
		ftw(di[i]->pathname, walk_tree, 1024);
	} else {
		monitor_index++;
		fprintf(stdout, "Monitoring %s\n", di[i]->pathname);
	}
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

	listen_for_events();
	exit(EXIT_SUCCESS);
}

/* vim:set ts=4 sts=0 sw=4: */
