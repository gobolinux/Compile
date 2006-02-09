/*
Copyright (c) 2005, IBM Corporation All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met: Redistributions of source code must retain the above
copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.  Neither the name
of the IBM Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without specific
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
*/

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "cmdline.h"

static char* default_watch_dir = "/Packages";

static char* default_argv[] = {"viewfs", "--help"};

static bool try_param(int argc, char** argv, int* i, char* flag, char* error_message, char** data) {
   int len = strlen(flag);
   if (strncmp(argv[*i], flag, len) == 0) {
      if (argv[*i][len] == '\0') {
         if (*i == argc - 1) {
            fprintf(stderr, "viewfs: %s: expected argument: %s\n", flag, error_message);
            exit(1);
         }
         *data = argv[*i+1];
         (*i)++;
      } else {
         *data = argv[*i]+len;
      }
      return true;
   }
   return false;
}

int parse_cmdline(int argc, char** argv, char** out_watch_dir, char** out_mountpoint, int* out_foreground) {
   char* watch_dir = default_watch_dir;
   char* mountpoint = NULL;
   int foreground = 0;
   if (argc == 1) {
      argc = 2;
      argv = default_argv;
   }
   for (int i = 1; i < argc; i++) {
      char* entry = NULL;
      if (strcmp(argv[i], "--help") == 0) {
         fprintf(stderr, "Run the viewfs daemon.\n\n");
         fprintf(stderr, "Usage:\n");
         fprintf(stderr, "   viewfs [-w <watchdir>] <mountpoint> [-f]\n\n");
         fprintf(stderr, "\t-w\tSpecify a directory to watch for entries. Default is %s\n", default_watch_dir);
         fprintf(stderr, "\t-f\tRun in foreground, do not daemonize.\n\n");
         return -1;
      }
      if (try_param(argc, argv, &i, "-w", "absolute path of entries dir", &watch_dir))
         continue;
      if (strcmp(argv[i], "-f") == 0) {
         foreground = 1;
         continue;
      }
      if (!mountpoint) {
         mountpoint = argv[i];
      }
   }
   if (!mountpoint) {
      fprintf(stderr, "viewfs: expected a mount point.\n");
      return -1;
   }
   *out_mountpoint = strdup(mountpoint);
   *out_watch_dir = strdup(watch_dir);
   *out_foreground = foreground;
   return 0;
}
