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
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <dirent.h>

#include "tree.h"
#include "inotify.h"

#define MANIFEST_FILE "Manifest"

/**
 * Read line into buffer. If there is not enough room, enlarge buffer.
 * In case nothing could be read due to end-of-file or errors,
 * an empty string is returned in the buffer. The newline character
 * in the end of the line, if any, is stripped.
 *
 * @param file The file handle
 * @param inout_buffer Pointer to the buffer
 * @param inout_buffer_size Pointer to the buffer's size
 */
static void read_line(FILE* file, char** inout_buffer, int* inout_buffer_size) {
   char* buffer = *inout_buffer;
   int buffer_size = *inout_buffer_size;
   int chunk_size = buffer_size;
   char* chunk = buffer;
   do {
      buffer[buffer_size-1] = 0xff;
      char* ok = fgets(chunk, chunk_size, file);
      if (!ok) {
         buffer[0] = '\0';
         return;
      }
      // If buffer was filled but no line was completed, expand and retry
      if (buffer[buffer_size-1] = '\0' && buffer[buffer_size-1] != '\n' && !feof(file)) {
         buffer_size += chunk_size - 1;
         chunk += chunk_size - 1;
         buffer = realloc(buffer, buffer_size);
      } else
         break;
   } while (1);
   char* newline = strrchr(buffer, '\n');
   if (newline)
      *(newline) = '\0';
   *inout_buffer = buffer;
   *inout_buffer_size = buffer_size;
}

/**
 * Read the Manifest file of a package. A manifest file lists the package
 * contents (its files and directories) and uses the following format:
 *
 * <pre>
 * start    :- entries
 * entries  :- { comment | entry } [ newline entries ]?
 * comment  :- "#" [ (any char except 0x0a) ]*
 * entry    :- type " " pathname
 * pathname :- name [ "/" name ]*
 * name     :- (any char except '/' and 0x0a)
 * type     :- { "d" | "f" | "l" }
 * </pre>
 *
 * Notice the above implies no leading, double or trailing slashes.
 * A Manifest file can be generated running the command:
 * cd /Packages/App/1.0; find . -printf "%y %P\n"
 * (requires FindUtils >= 4.2)
 *
 * @param tree Tree handle.
 * @param view_node Inode number of the view root
 * (e.g y in "TREE_ROOT_ID(/)<-x(Fuse)<-y(2.4.1)")
 * @param watch_dir The watch directory (e.g. "/Packages").
 * @param package The package directory (e.g. "Fuse").
 * @param version The version directory (e.g. "2.4.1").
 */
static void read_manifest(tree_t* tree, int view_node, char* watch_dir, char* package, char* version) {
   char* manifest;
   asprintf(&manifest, "%s/%s/%s/%s", watch_dir, package, version, MANIFEST_FILE);
   FILE* file = fopen(manifest, "r");
   if (!file)
      return;
   int buffer_size = 300;
   char* buffer = malloc(buffer_size);
   while (!feof(file)) {
      read_line(file, &buffer, &buffer_size);
      if (buffer[0] == '#' || buffer[0] == '\0' || strlen(buffer) < 3)
         continue;
      char type = buffer[0];
      char* path = &(buffer[2]);
      /* Walk through tokens of path */
      char* word = path;
      char* slash;
      int walk_tree = view_node;
      int* children = NULL;
      bool last = false;
      do {
         slash = strchr(word, '/');
         if (slash)
            *(slash) = '\0';
         else
            last = true;
         int nr_children = 0;
         int err = tree_lookup(tree, walk_tree, word, &walk_tree);
         if (walk_tree == 0) {
            walk_tree = tree_new_entry(tree, walk_tree, word);
         }
         if (walk_tree == 0) {
            fprintf(stderr, "Error: failure adding inodes.\n");
            goto exit;
         }
         if (!last) {
            *(slash) = '/'; // is it necessary to rebuild string?
            word = slash + 1;
         }
      } while (!last);
   }
exit:
   free(buffer);
   fclose(file);
   free(manifest);
}

static void add_package(tree_t* tree, char* watch_dir, char* package) {
   char* dirname;
   asprintf(&dirname, "%s/%s", watch_dir, package);
   inode_watch(dirname);

   DIR* d = opendir(dirname);
   if (!d) return; /* ignore non-directories */
   struct dirent* version;
   int package_node = tree_new_entry(tree, TREE_ROOT_ID, package);
   while ( (version = readdir(d)) ) {
      char* version_name = version->d_name;
      if (version_name[0] == '.')
         continue;
      int view_node = tree_new_entry(tree, package_node, version_name);
      read_manifest(tree, view_node, watch_dir, package, version_name);
   }
   closedir(d);
   free(dirname);
}

int scan_watch_dir(tree_t* tree, char* watch_dir) {
   DIR* d = opendir(watch_dir);
   if (!d) {
      fprintf(stderr, "viewfs: could not perform initial scan on %s.\n", watch_dir);
      return -1;
   }
   struct dirent* ent;
   while ( (ent = readdir(d)) ) {
      if (ent->d_name[0] == '.')
         continue;
      add_package(tree, watch_dir, ent->d_name);
   }
   closedir(d);
   return 0;
}
