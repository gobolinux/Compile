/* gobohide.c: Set/Unset the "hide directory" flag to a directory */

/*
 * Copyright (C) 2002 CScience.ORG World Domination Inc.
 *
 * This program is Free Software; you can redistributed it
 * and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * Author: Felipe W Damasio <felipewd@terra.com.br>
 * 
 * Changes:
 * 0.10 - Lucas C. Villa Real
 *        Handling multiple entries.
 *        
 * 0.09 - Lucas C. Villa Real
 *        We were passing (struct gobolinux_hide **) to ioctl(),
 *        changed to (struct gobolinux_hide *);
 *        Added an exit call after purge_list().
 *
 * 0.08a- Felipe W Damasio
 *        Added -f, --flush option to flush (delete all) the hide list.
 *
 * 0.08 - Felipe W Damasio
 *        Error-checking on "open" calls cleanup code.
 *        
 * 0.07 - Lucas C. Villa Real
 *        Added a valid file descriptor to be used on statistics ioctl calls.
 *        
 * 0.05 - Lucas C. Villa Real 
 *        Real path is now being handled, so the statistics can now 
 *        show something better than simple inode numbers.
 *
 * 0.04 - Lucas C. Villa Real 
 *        Added statistics support.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <unistd.h>
#include <getopt.h>
#include <limits.h>
#ifndef PATH_MAX
#define PATH_MAX 8192
#endif
#include <pwd.h>
#include <sys/ioctl.h>
#include "gobolinux.h"

/* Paranoia setting */
#ifndef FIGOBOLINUX
#define FIGOBOLINUX _IOW(0x00, 0x22, size_t) /* gobolinux-fs ioctl */
#endif

void usage (int status);

static int show_help = 0;
static int show_version = 0;

const char shortopts[]  = "h:u:lf";
struct option longopts[] = {
	{"hide",     1, 0, 'h'},
	{"unhide",   1, 0, 'u'},
	{"list",     0, 0, 'l'},
	{"flush",    0, 0, 'f'},
	{"help",     0, &show_help, 1},
	{"version",  0, &show_version, 1},
	{ 0, 0, 0, 0 }
};

/* The invocation name of the program */
static char *program_name;
const char version[] = "0.10";

void
usage (int status)
{
	if (status) { /* Show help */
		fprintf (stdout, 
		"%s: Hide/Unhide a directory\n\n"
		"-h, --hide     Hide the directory\n"
		"-u, --unhide   Unhide the directory\n"
		"-l, --list     List the hidden directories\n"
		"-f, --flush    Flush the hide list\n"
		"    --version  Show the program version\n"
		"    --help     Show this message\n", 
		program_name);
	} else {
		fprintf (stdout,
		"Copyright (C) 2002 CScience.ORG World Domination Inc.\n\n"
		"This program is Free Software; you can redistributed it\n"
		"and/or modify it under the terms of the GNU General Public\n"
		"License as published by the Free Software Foundation.\n\n"
		"%s version %s\n", program_name, version);
	}
	exit (0);
}

void
err_quit (int status, char *file)
{
	fprintf (stderr, "%s is neither a directory "
			 "nor a symbolic link\n", file);
	exit (status);
}

void
generic_ioctl (char *dir, int operation)
{
	int fd;
	struct gobolinux_hide hide;
	struct stat stats;

	/* We're only interested in directories */
	fd = open (dir, O_RDONLY|O_NOFOLLOW);
	if (fd < 0) { /* We're opening a sym-link */
		fd = open (dir, O_RDONLY); /* The symlink must point to a valid directory */
		if (fd < 0) {
			perror (dir);
			exit (EXIT_FAILURE);
		}
		if (lstat (dir, &stats) == -1) { /* Do not follow the link */
			perror ("lstat");
			exit (EXIT_FAILURE);
		}
		if (!S_ISLNK(stats.st_mode)) err_quit (1, dir);
		hide.symlink = 1;
	} else {
		/* We opened a directory, let's get its inode number */
		if (fstat (fd, &stats) == -1) {
			perror ("fstat");
			exit (EXIT_FAILURE);
		}
		if (!S_ISDIR(stats.st_mode)) err_quit (1, dir);
		hide.symlink = 0;
	}

	hide.pathname  = dir;
	hide.operation = operation;
	hide.inode     = stats.st_ino;
	
	if (ioctl (fd, FIGOBOLINUX, &hide) == -1) {
		perror ("ioctl");
	}

	close (fd);
}

struct gobolinux_hide *
get_stats ()
{
	int i, fd;
	struct stat stats;
	struct gobolinux_hide *hide;

	hide = (struct gobolinux_hide *) calloc (1, sizeof (struct gobolinux_hide));
	if (!hide) {
		perror ("calloc");
		exit (EXIT_FAILURE);
	}
	
	/* Get a valid file descriptor */
	fd = open ("/", O_RDONLY|O_NOFOLLOW);
	if (fd == -1) {
		perror ("open");
		exit (EXIT_FAILURE);
	}
	
	if (fstat (fd, &stats) == -1) {
		perror ("fstat");
		exit (1);
	}
	
	if (!S_ISDIR(stats.st_mode)) 
		err_quit (1, "/");

	hide->operation           = GETSTATSUIDNUMBER;
	hide->stats.hidden_inodes = 0;
	hide->inode               = stats.st_ino;
	
	/* Do the ioctl call passing a valid file descriptor */
	if ((ioctl (fd, FIGOBOLINUX, hide)) < 0) {
		perror ("ioctl");
		exit (EXIT_FAILURE);
	} else if (hide->stats.hidden_inodes <= 0) {
		return NULL;
	}
	
	hide->stats.hidden_list = (char **) calloc (hide->stats.hidden_inodes, sizeof (char *));
	if (!hide->stats.hidden_list) {
		perror ("calloc");
		exit (EXIT_FAILURE);
	}
	
	for (i = 0; i < hide->stats.hidden_inodes; ++i) {
		hide->stats.hidden_list[i] = (char *) calloc (PATH_MAX, sizeof (char));
		if (!hide->stats.hidden_list[i]) {
			perror ("calloc");
			exit (EXIT_FAILURE);
		}
	}
	
	hide->operation = GETSTATSUID;
	/* Get the inodes list */
	if ((ioctl (fd, FIGOBOLINUX, hide)) < 0) {
		for (i = 0; i < hide->stats.hidden_inodes; i++)
			free (hide->stats.hidden_list[i]);
		free (hide->stats.hidden_list);
		perror ("ioctl");
		exit (EXIT_FAILURE);
	}

	return hide;
}

void
list_hidden (void)
{
	int i;
	struct gobolinux_hide *hide;
	
	hide = get_stats ();
	if (!hide) {
		fprintf (stderr, "No inodes being hidden\n");
		return;
	}
	
	fprintf (stdout, "Hidden directories:\n");
	for (i = 0; i < hide->stats.filled_size; ++i) {
		fprintf (stdout, "%s\n", hide->stats.hidden_list[i]);
		free (hide->stats.hidden_list[i]);
	}
	fprintf (stdout, "\n");
	free (hide->stats.hidden_list);
	free (hide);
}

void
purge_list ()
{
	int i;
	struct gobolinux_hide *hide;

	hide = get_stats ();
	if (!hide)
		return; /* No list to purge */

	for (i = 0; i < hide->stats.filled_size; i++) {
		generic_ioctl (hide->stats.hidden_list[i], GOBOLINUX_UNHIDEINODE);
		free (hide->stats.hidden_list[i]);
	}
	
	free (hide->stats.hidden_list);
	free (hide);
}

int
main (int argc, char **argv)
{
	int c;
	int a = -1, purge = 0;
	const char *dir = NULL;

	program_name = argv[0];
	while ((c = getopt_long (argc, argv, shortopts, longopts, 0)) != -1) {
		switch (c) {
		case 'h': a = GOBOLINUX_HIDEINODE;
			  dir = optarg;
			  break;
		case 'u': a = GOBOLINUX_UNHIDEINODE;
			  dir = optarg;
			  break;
		case 'l': a = GETSTATSUID;
			  break;
		case 'f': purge = 1;
			  break;
		}
	}

	if (show_help)
		usage (1);
	if (show_version)
		usage (0);

	/* Only the superuser is allowed to execute further */
	if (getuid () != 0) {
		fprintf (stderr, "Must be superuser\n");
		exit (EXIT_SUCCESS);
	}
	
	if (purge) {
		purge_list ();
		exit (EXIT_SUCCESS);
	}

	switch (a) {
		case -1:
			fprintf (stderr, 
					 "%s: You must specify at least one option!\n\n"
				  	 "try '%s --help' for more information\n", 
				  	 program_name, program_name);
			break;

		case GETSTATSUID:
			list_hidden ();
			break;
				  
		default:
			generic_ioctl (dir, a);
			while (optind < argc)
				generic_ioctl (argv[optind++], a);
	}
	
	exit (EXIT_SUCCESS);
}

/*
 * vim: sw=4 ts=4
 */
