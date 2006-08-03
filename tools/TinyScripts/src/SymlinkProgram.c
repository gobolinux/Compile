/*
 * SymlinkProgram - a simplistic version of GoboLinux' SymlinkProgram, in C
 *
 * Copyright (C) 2005 Lucas Correia Villa Real <lucasvr@gobolinux.org>
 * Released under the GNU GPL.
 *
 *
 * Changelog:
 * 03/10/2005 - [lucasvr] First version
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>
#include <dirent.h>

#define PROGRAM_NAME     "SymlinkProgram"
#define COLOR_RED        "\033[1;31;40m"
#define COLOR_GREEN      "\033[22;36m"
#define COLOR_DARK_GREY  "\033[01;30m"

#define KEEP      0
#define OVERWRITE 1
static int on_conflict = KEEP;

const char shortopts[] = "hc:";
static struct option long_options[] = {
    {"conflict",    1, NULL, 1},
    {"help",        0, NULL, 0},
    {0, 0, 0, 0}
};

#define GET_OR_ASSIGN(var,env,default_value) ({ \
    var=getenv(env); \
    if (! var) { \
        var = default_value; \
		setenv(env, var, 1); \
	} \
})

#define print_err(msg,arg...) ({\
    printf("%s%s: %s", COLOR_DARK_GREY, PROGRAM_NAME, COLOR_RED); \
    printf(msg, ##arg); \
})

#define print_msg(msg,arg...) ({\
    printf("%s%s: %s", COLOR_DARK_GREY, PROGRAM_NAME, COLOR_GREEN); \
    printf(msg, ##arg); \
})

static char *goboPrograms;
static char *goboExecutables;
static char *goboLibraries;
static char *goboHeaders;
static char *goboManuals;
static char *goboSettings;
static char *goboShared;
static char *goboEnvironment;
static char *goboTasks;

int
dir_exists(char *dirname)
{
	DIR *dp = opendir(dirname);
	if (! dp)
		return 0;
	
	closedir(dp);
	return 1;
}

int
empty_dir(char *dirname)
{
	int i, n;
	DIR *dp;
	struct dirent **namelist;
	
	dp = opendir(dirname);
	if (! dp)
		/* consider it empty */
		return 1;
	
	n = scandir(dirname, &namelist, NULL, alphasort);
	if (n < 0) {
		perror("scandir");
		closedir(dp);
		return 1;
	}
	closedir(dp);
	
	for (i=0; i<n; ++i) {
		if (! strcmp(namelist[i]->d_name, ".") ||
			! strcmp(namelist[i]->d_name, ".."))
			continue;

		/* consider it not empty */
		return 0;
	}
	return 1;
}

char *
guess_last_version(char *app_name)
{
	int i, n;
	DIR *dp;
	struct dirent **namelist;
	char *last_version = NULL, program_name[PATH_MAX];
	
    snprintf(program_name, sizeof(program_name), "%s/%s", goboPrograms, app_name);
	dp = opendir(program_name);
    if (! dp) {
        print_err("There is no entry %s inside %s.\n", program_name, goboPrograms);
        return NULL;
    }
	closedir(dp);

	n = scandir(program_name, &namelist, NULL, alphasort);
	if (n < 0) {
		perror("scandir");
		return NULL;
	}
	
	for (i=0; i<n; ++i) {
		if (! strcmp(namelist[i]->d_name, ".") ||
			! strcmp(namelist[i]->d_name, "..") ||
			! strcmp(namelist[i]->d_name, "Current") ||
			! strcmp(namelist[i]->d_name, "Settings") ||
			! strcmp(namelist[i]->d_name, "Variable"))
			continue;

		/* check if the entry is a valid directory */
		memset(program_name, 0, sizeof(program_name));
		snprintf(program_name, sizeof(program_name), "%s/%s/%s",
				goboPrograms, app_name, namelist[i]->d_name);
		
		if (! dir_exists(program_name))
			continue;

		last_version = namelist[i]->d_name;
	}
	
	return last_version;
}

void
do_link(char *to, char *from)
{
	char **argv;
	char *overwrite = (on_conflict == OVERWRITE) ? "--overwrite" : "";
	char *relative = "";
	
	if (empty_dir(from)) {
		unlink(from);
		return;
	}

	argv = (char **) malloc(sizeof(char *) * 5);
	argv[0] = "/System/Links/Executables/LinkOrExpandAll";
	argv[1] = from;
	argv[2] = overwrite;
	argv[3] = relative;
	argv[4] = NULL;
	
	chdir(to);
	link_or_expand_all_main(4, argv);
	free(argv);
}

void
do_exec(char *cmd)
{
    int pid = fork();
    
    if (pid == 0)
        execl(cmd, cmd, NULL);
    else if (pid > 0)
        waitpid(pid, NULL, WUNTRACED);
    else
        perror("fork");
}

void
symlink_program(char *name, char *version)
{
    int printed = 0;
    char dirname[PATH_MAX];

	/* TODO: 
	 * - symlink Variable
	 * - symlink wrappers
	 * - Shared needs special treatment, doesn't it?
	 * - run FixLibtoolLa
	 * - rebuild environment cache
	 * - remove unused directories
	 * - remove broken links
	 */
	
    snprintf(dirname, sizeof(dirname), "%s/%s/Settings", goboPrograms, name);
    if (dir_exists(dirname)) {
        print_msg("Symlinking global settings...\n");
        do_link(goboSettings, dirname);
    }

#define set_dir(buf, subdir) \
    snprintf(buf, sizeof(buf), "%s/%s/%s/%s", goboPrograms, name, version, subdir)

    set_dir(dirname, "Resources/Tasks");
    if (dir_exists(dirname)) {
        print_msg("Symlinking tasks...\n");
        do_link(goboTasks, dirname);
    }

    set_dir(dirname, "lib");
    if (dir_exists(dirname)) {
        print_msg("Symlinking libraries...\n");
        do_link(goboLibraries, dirname);
        
        print_msg("Updating library database (ldconfig)...\n");
        do_exec("ldconfig");
    }

    set_dir(dirname, "include");
    if (dir_exists(dirname)) {
        print_msg("Symlinking headers...\n");
        do_link(goboHeaders, dirname);
    }
    
    set_dir(dirname, "info");
    if (dir_exists(dirname)) {
        char to[PATH_MAX];
        
        print_msg("Symlinking info...\n");
        sprintf(to, "%s/info", goboManuals);
        do_link(to, dirname);
    }
    
    set_dir(dirname, "man");
    if (dir_exists(dirname)) {
        int i;
        char to[PATH_MAX];
        char from[PATH_MAX];
        
        print_msg("Symlinking manuals...\n");
        for (i=0; i<9; ++i) {
            sprintf(from, "%s/man%d", dirname, i);
            sprintf(to, "%s/man/man%d", goboManuals, i);
            do_link(to, from);
        }
    }
	
    set_dir(dirname, "Shared");
    if (dir_exists(dirname)) {
        print_msg("Symlinking shared...\n");
        do_link(goboShared, dirname);
    }
    
    set_dir(dirname, "bin");
    if (dir_exists(dirname)) {
        print_msg("Symlinking executables...\n");
        do_link(goboExecutables, dirname);
        printed = 1;
    }
    
    set_dir(dirname, "sbin");
    if (dir_exists(dirname)) {
        if (! printed)
            print_msg("Symlinking executables...\n");
        do_link(goboExecutables, dirname);
    }

    set_dir(dirname, "Environment");
    if (dir_exists(dirname)) {
        print_msg("Symlinking environment...\n");
        do_link(goboEnvironment, dirname);
    }
}

void
usage(char *program_name)
{
	fprintf(stderr,
			"%s: Link a program from the /Programs hierarchy in the /System tree.\n\n"
            "Usage: %s [<options>] <program_name> [<program_version>]\n\n"
            "Options:                    \n"
			"    -h, --help              \n"
			"    -c, --conflict <option> \n"
            "        What to do on conflicts, 'keep' or 'overwrite'.  The default value is 'keep'.\n\n",
			program_name, program_name);

    exit(EXIT_FAILURE);
}

int
main(int argc, char **argv)
{
    int c, index;
    char buf[PATH_MAX];
    char *program_name, *program_version = NULL;
    DIR *dp;

    while ((c=getopt_long(argc, argv, shortopts, long_options, &index)) != -1) {
		switch (c) {
			case 0:
				break;
			case 'c':
                if (! strcmp(optarg, "keep"))
                    on_conflict = KEEP;
                else if (! strcmp(optarg, "overwrite"))
                    on_conflict = OVERWRITE;
                else {
                    fprintf(stderr, "invalid option %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
				break;
            case 'h':
				usage(argv[0]);
                break;
			case '?':
				break;
			default:
				printf("invalid option %d\n", c);
				usage(argv[0]);
				break;
		}
    }

    GET_OR_ASSIGN(goboPrograms,    "goboPrograms",    "/Programs");
    GET_OR_ASSIGN(goboExecutables, "goboExecutables", "/System/Links/Executables");
    GET_OR_ASSIGN(goboLibraries,   "goboLibraries",   "/System/Links/Libraries");
    GET_OR_ASSIGN(goboHeaders,     "goboHeaders",     "/System/Links/Headers");
    GET_OR_ASSIGN(goboManuals,     "goboManuals",     "/System/Links/Manuals");
    GET_OR_ASSIGN(goboSettings,    "goboSettings",    "/System/Settings");
    GET_OR_ASSIGN(goboShared,      "goboShared",      "/System/Links/Shared");
    GET_OR_ASSIGN(goboEnvironment, "goboEnvironment", "/System/Links/Environment");
    GET_OR_ASSIGN(goboTasks,       "goboTasks",       "/System/Links/Tasks");

	/* set log file descriptors for LinkOrExpandAll */
	setenv("debugFD",   "9", 1); /* we don't want debug messages to appear */
	setenv("verboseFD", "1", 1);
	setenv("normalFD",  "1", 1);
	setenv("terseFD",   "1", 1);
	setenv("errorFD",   "2", 1);
	setenv("scriptName", argv[0], 1);

    if (argc < 2) {
        fprintf(stderr, "Argument missing: specify the program name or directory.\n");
        exit(EXIT_FAILURE);
    }
    
    program_name = argv[1];
    if (argc > 2)
        program_version = argv[2];
	else {
		program_version = guess_last_version(program_name);
		if (! program_version) {
			print_err("No valid version found for package %s\n", program_name);
			exit(EXIT_FAILURE);
		}
	}
	
    snprintf(buf, sizeof(buf), "%s/%s/%s", goboPrograms, program_name, program_version);
    dp = opendir(buf);
    if (! dp) {
        print_err("There is no version %s for package %s.\n", program_version, program_name);
        return 1;
    }
	closedir(dp);
    
    print_msg("Symlinking %s %s.\n", program_name, program_version);
    symlink_program(program_name, program_version);
    
    return 0;
}

/*
 * vim: ts=4 sw=4
 */
