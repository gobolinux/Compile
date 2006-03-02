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
    if (! var) \
        var = default_value; \
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
static char *goboSettings;
static char *goboShared;
static char *goboEnvironment;
static char *goboTasks;

int 
create_or_expand_symlink(char *where, char *program_name, char *program_fullpath, char *filename)
{
    int fd, ret;
    char target[PATH_MAX], link_buf[PATH_MAX], new_file[PATH_MAX];
    struct stat status;
    
    if (! strcmp(filename, ".") || ! strcmp(filename, ".."))
        return 0;

    snprintf(target, sizeof(target), "%s/%s", program_fullpath, filename);
    snprintf(new_file, sizeof(new_file), "%s/%s", where, filename);

    /* check for existing symlink */
    fd = open(target, O_RDONLY);
    if (fd == -ENOENT) {
        /* broken link, so just remove it */
        unlink(new_file);
        
    } else if (fd > 0) {
        ret = lstat(new_file, &status);
        if (ret < 0) {
            fprintf(stderr, "lstat %s: %s\n", new_file, strerror(errno));
            return 0;
        }

        if (S_ISDIR(status.st_mode)) {
            /* TODO: needs to expand the directory */
        }
        
        memset(link_buf, 0, sizeof(link_buf));
        readlink(new_file, link_buf, sizeof(link_buf));

        if (!strncmp(link_buf, program_name, strlen(program_name))) {
            /* a file with the same name exists, pointing to another version of this app */
            unlink(new_file);
        } else if (on_conflict == OVERWRITE) {
            unlink(new_file);
        }
            
        close(fd);
    }

    ret = symlink(target, new_file);
    if (ret < 0) {
        perror("symlink");
        return 0;
    }

    return 1;
}

void
do_link(char *where, char *program_name, char *program_fullpath, char *subdir)
{
    int processed_files = 0;
    char target_dir[PATH_MAX];
    DIR *dp;
    FILE *fp;
    struct dirent *entry;

    /* link contents from the given directory */
    snprintf(target_dir, sizeof(target_dir), "%s/%s", program_fullpath, subdir);
    dp = opendir(target_dir);
    if (! dp) {
        /* it can be a file */
        fp = fopen(target_dir, "r");
        if (! fp)
            return;

        /* in this case, subdir represents a given file */
        create_or_expand_symlink(where, program_name, target_dir, subdir);
        return;
    }

    while ((entry = readdir(dp)))
        processed_files += create_or_expand_symlink(where, program_name, target_dir, entry->d_name);

    if (processed_files)
        print_msg("Processed %d files.\n", processed_files);
}

void
symlink_program(char *app_name, char *program_version)
{
    char program_name[PATH_MAX];
    char program_fullpath[PATH_MAX];

    snprintf(program_name, sizeof(program_name), "%s/%s", goboPrograms, app_name);
    snprintf(program_fullpath, sizeof(program_fullpath), "%s/%s", program_name, program_version);
    
    print_msg("Symlinking global settings...\n");
    do_link(goboSettings, program_name, program_version, "../Settings");

    print_msg("Symlinking tasks...\n");
    do_link(goboTasks, program_name, program_version, "Resources/Tasks");

    print_msg("Symlinking executables...\n");
    do_link(goboExecutables, program_name, program_fullpath, "bin");
    do_link(goboExecutables, program_name, program_fullpath, "sbin");
    
    print_msg("Symlinking libraries...\n");
    do_link(goboLibraries, program_name, program_version, "lib");

    print_msg("Symlinking shared...\n");
    do_link(goboShared, program_name, program_version, "Shared");

    print_msg("Symlinking environment...\n");
    do_link(goboEnvironment, program_name, program_version, "Resources/Environment");
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
    char *program_name, *program_version = "Current";
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

    if (argc < 2) {
        fprintf(stderr, "Argument missing: specify the program name or directory.\n");
        exit(EXIT_FAILURE);
    }
    
    program_name = argv[1];
    if (argc > 2)
        program_version = argv[2];

    GET_OR_ASSIGN(goboPrograms,    "goboPrograms",    "/Programs");
    GET_OR_ASSIGN(goboExecutables, "goboExecutables", "/System/Links/Executables");
    GET_OR_ASSIGN(goboLibraries,   "goboLibraries",   "/System/Links/Libraries");
    GET_OR_ASSIGN(goboSettings,    "goboSettings",    "/System/Settings");
    GET_OR_ASSIGN(goboShared,      "goboShared",      "/System/Links/Shared");
    GET_OR_ASSIGN(goboEnvironment, "goboEnvironment", "/System/Links/Environment");
    GET_OR_ASSIGN(goboTasks,       "goboTasks",       "/System/Links/Tasks");

    snprintf(buf, sizeof(buf), "%s/%s/%s", goboPrograms, program_name, program_version);
    dp = opendir(buf);
    if (! dp) {
        print_err("There is no version %s for package %s.\n", program_version, program_name);
        return 1;
    }
    
    print_msg("Symlinking %s %s.\n", program_name, program_version);
    symlink_program(program_name, program_version);
    
    return 0;
}
