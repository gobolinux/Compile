/*
 * RemoveBroken - Remove broken links
 *
 * Copyright (C) 2004-2005 Lucas Correia Villa Real <lucasvr@gobolinux.org>
 * Released under the GNU GPL.
 *
 */
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
	int ret, i;

	if (argc == 1) {
		/* read from stdin */
		
		while (! feof(stdin)) {
			char filename[PATH_MAX];
			struct stat status;

			memset(filename, 0, sizeof(filename));
			fgets(filename, sizeof(filename), stdin);
			if (strlen(filename))
				filename[strlen(filename)-1] = '\0';
				
			if (! strcmp(filename, ".") || ! strcmp(filename, "..") || ! strlen(filename))
				continue;

			ret = stat(filename, &status);
			if (ret < 0) {
				printf("%s\n", filename);
				unlink(filename);
			}
		}
		
	} else {
		/* read from arguments */
		
		for (i=1; i<argc; ++i) {
			char *filename = argv[i];
			struct stat status;
		
			if (! strcmp(filename, ".") || ! strcmp(filename, "..") || ! strlen(filename))
				continue;

			ret = stat(filename, &status);
			if (ret < 0) {
				printf("%s\n", filename);
				unlink(filename);
			}
		}
	}

	return 0;
}
