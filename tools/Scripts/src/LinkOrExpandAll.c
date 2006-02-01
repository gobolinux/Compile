/*
 * LinkOrExpandAll
 * port to C by Hisham Muhammad, 2006
 * based on port to Python by Andre Detsch, 2004
 * based on bash original by Hisham Muhammad, 2002?
 *
 * Released under the GNU GPL v.2 or later.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>
#include <assert.h>

#ifndef OK
#define OK 0
#endif

static char* goboPrograms = NULL;
static char realpathGoboPrograms[PATH_MAX];
static int lenGoboPrograms = 0;
static char* relativeGoboPrograms = NULL;
static bool overwrite = false;
static bool relative = false;

inline void os_write(int fd, ...) {
   va_list ap;
   va_start(ap, fd);
   for(;;) {
      char* s = va_arg(ap, char*);
      if (!s) break;
      write(fd, s, strlen(s));
   }
}

inline bool os_path_islink(char* path) {
   struct stat stbuf;
   lstat(path, &stbuf);
   return S_ISLNK(stbuf.st_mode);
}

inline bool os_path_isdir(char* path) {
   struct stat stbuf;
   stat(path, &stbuf);
   return S_ISDIR(stbuf.st_mode);
}

typedef struct {
   char* name;
   DIR* dir;
} os_dir;

static char* os_listdir(os_dir* dir) {
   if (!dir->dir) {
      dir->dir = opendir(dir->name);
      if (!dir->dir) return NULL;
   }
   struct dirent* dp;
   while ((dp = readdir(dir->dir))) {
      char* entry = dp->d_name;
      if (entry[0] == '.' && (entry[1] == '\0' || entry[1] == '.'))
         continue;
      return strdup(entry);
   }
   closedir(dir->dir);
   dir->dir = NULL;
   return NULL;
}

inline void string_replace1(char* dest, char* buffer, char* from, char* to, int len) {
   char tmpbuffer[len+1];
   char* out;
   if (dest == buffer)
      out = tmpbuffer;
   else
      out = dest;
   char* match = strstr(buffer, to);
   if (match) {
      int index = match-buffer;
      int indexrest = index + strlen(to);
      strncpy(out, buffer, index);
      strncpy(out + index, to, len - index);
      strncpy(out + indexrest, match + strlen(from), len - indexrest);
   }
   if (dest == buffer)
      strcpy(buffer, tmpbuffer);
}

inline void string_set(char* buffer, int len, ...) {
   va_list ap;
   char tmpbuffer[len+1];
   va_start(ap, len);
   char* at = tmpbuffer;
   int rest = len;
   for(;;) {
      char* s = va_arg(ap, char*);
      if (!s) break;
      int l = strlen(s);
      snprintf(at, rest, s);
      at += l;
      rest -= l;
   }
   *at = '\0';
   strncpy(buffer, tmpbuffer, len);
}

typedef enum {
   Debug,
   Verbose,
   Normal,
   Terse,
   Error
} LogMode;

static const char* colorGray = "\033[1;30m";
static const char* colorBoldBlue = "\033[1;34m";
static const char* colorBrown = "\033[33m";
static const char* colorYellow = "\033[1;33m";
static const char* colorBoldGreen = "\033[1;32m";
static const char* colorBoldRed = "\033[1;31m";
static const char* colorCyan = "\033[36m";
static const char* colorBoldCyan = "\033[1;36m";
static const char* colorRedWhite = "\033[41;37m";
static const char* colorNormal = "\033[0m";

static void Log(LogMode mode, char* s, ...) {
   va_list ap;
   va_start(ap, s);
   char buffer[1024];
   if (strchr(s, '%')) {
      vsnprintf(buffer, 1023, s, ap);
      s = buffer;
   }
   const char* color;
   char* fdName;
   switch (mode) {
   case Debug: color = colorRedWhite; fdName = "debugFD"; break;
   case Verbose: color = colorNormal; fdName = "verboseFD"; break;
   case Normal: color = colorCyan; fdName = "normalFD"; break;
   case Terse: color = colorBoldCyan; fdName = "terseFD"; break;
   case Error: color = colorBoldRed; fdName = "errorFD"; break;
   }
   char* fdString = getenv(fdName); if (!fdString) goto failSafe;
   int fd = atoi(fdString);
   os_write(fd, colorGray, getenv("scriptName"), ":", colorNormal, " ", color, s, colorNormal, "\n", NULL);
   return;
  failSafe:
   fprintf(stderr, "%s[LOG PROBLEM]%s %s\n", colorRedWhite, colorNormal, s);
}

#define Log_Debug(s, ...) Log(Debug, s, ## __VA_ARGS__)
#define Log_Verbose(s, ...) Log(Verbose, s, ## __VA_ARGS__)
#define Log_Normal(s, ...) Log(Normal, s, ## __VA_ARGS__)
#define Log_Terse(s, ...) Log(Terse, s, ## __VA_ARGS__)
#define Log_Error(s, ...) Log(Error, s, ## __VA_ARGS__)

static int count = 0;

static void Link_Or_Expand(char* new);

// returns a new copy of the path, owned by the caller.
static char* points_to(char* p) {
   char points[PATH_MAX+1];
   if (access(p, R_OK) == OK) {
      realpath(p, points);
   } else if (os_path_islink(p)) {
      realpath(p, points);
      string_replace1(points, points, "/share/", "/Shared/", PATH_MAX);
      if (*points != '/') {
         char* dname = dirname(p);
         string_set(points, PATH_MAX, dname, '/', points);
         free(dname);
      }
   } else {
      strncpy(points, p, PATH_MAX);
   }
   return strdup(points);
}

static void link_inside(char* realnew, char* bn) {
   Log_Verbose("Linking files from '%s' in directory '%s'", realnew, bn);
   chdir(bn);
   os_dir dir = { .name = realnew };
   char* entry;
   while ((entry = os_listdir(&dir))) {
      char buffer[PATH_MAX+1];
      snprintf(buffer, PATH_MAX, "%s/%s", realnew, entry);
      Link_Or_Expand(buffer);
      free(entry);
   }
   chdir("..");
}

static bool belongs_to_same_app(char* realold, char* realnew) {
   if (strlen(realold) < lenGoboPrograms) return false;
   if (strlen(realnew) < lenGoboPrograms) return false;
   // realnew instead of new because of typical MergeTree links
   char* appold = realold + lenGoboPrograms;
   char* appnew = realnew + lenGoboPrograms;
   char* slash = strchr(appold, '/');
   if (!slash) return false;
   // case-insensitive
   if (strncasecmp(appold, appnew, slash - appold) == 0)
      return true;
   return false;
}

static void create_single_link(char* src, char* dest) {
   char dotdest[PATH_MAX+1];
   snprintf(dotdest, PATH_MAX, "./%s", dest);
   if (relative) {
      char relativesrc[PATH_MAX+1];
      string_replace1(relativesrc, src, realpathGoboPrograms, relativeGoboPrograms, PATH_MAX);
      symlink(relativesrc, dotdest);
   } else {
      symlink(src, dotdest);
   }
}

static void Link_Or_Expand(char* new) {
   count++;
   if (relative) {
      char* prefix = getenv("goboPrefix");
      char* candidate = goboPrograms;
      if (prefix) {
         assert(strlen(candidate) >= strlen(prefix));
         candidate += strlen(prefix);
      }
      if (*candidate == '/')
         candidate++;
      char buffer[1024];
      char* walk = buffer;
      for (int i = 0; i < 10; i++) {
         strncpy(walk, "../", 1024 - (walk-buffer));
         walk += 3;
         strncpy(walk, candidate, 1024 - (walk-buffer));
         if (access(buffer, W_OK) == OK) {
            relativeGoboPrograms = strdup(buffer);
            break;
         }
      }
   }
   fprintf(stderr, "relativeGoboPrograms = %s\n", relativeGoboPrograms);
   char* realnew = points_to(new);
   char* bn = strdup(basename(new));
   char realold[PATH_MAX];

   // if name of new is not being used, or is used by a broken link...
   if (access(bn, R_OK) != OK) {
      // if is a broken link, remove it
      if (os_path_islink(bn)) {
         unlink(bn);
      }
      realold[0] = '\0';
   } else {
      realpath(bn, realold);
   }

   // 1: new is a broken link
   if (access(realnew, R_OK) != OK) {
      Log_Debug("Skipping %s because it is a broken link under %s.", new, goboPrograms, realnew);
      // TODO: Do nothing by now. I'm not sure what is the correct behavior
      goto leave;
   }
   
   // 2: name of new is not being used
   if (*realold == '\0') {
      Log_Verbose("Creating link: %s", bn);
      Log_Debug("symlink1 %s ./%s", realnew, bn);
      create_single_link(realnew, bn);
      goto leave;
   }
   
   bool bnIsLink = os_path_islink(bn);
   
   // 3: probably upgrading a program version
   if (bnIsLink && belongs_to_same_app(realold, realnew)) {
      Log_Verbose("Replacing link: %s", new);
      Log_Debug("symlink2 %s ./%s", realnew, bn);
      char dotbn[PATH_MAX+1];
      snprintf(dotbn, PATH_MAX, "./%s", bn);
      unlink(dotbn);
      create_single_link(realnew, bn);
      goto leave;
   }

   bool bnIsDir = os_path_isdir(bn);
   bool realnewIsDir = os_path_isdir(realnew);

   // 4: name of new was being used by an directory (probably with links)
   if ((!bnIsLink) && bnIsDir && realnewIsDir) {
      link_inside(realnew, bn);
      goto leave;
   }

   bool realoldIsDir = os_path_isdir(realold);
   
   // 5: name of new was being used by an link to a directory
   if (bnIsLink && realoldIsDir && realnewIsDir) {
      Log_Normal("Creating expanded directory '%s'...", bn);
      unlink(bn);
      mkdir(bn, 0777);
      chdir(bn);
      Log_Verbose("Linking files from '%s' in directory '%s'...", realold, bn);
      os_dir dir = { .name = realold };
      char* i;
      while ((i = os_listdir(&dir))) {
         char* oldbn = strdup(basename(i));
         char realold_i[PATH_MAX+1];
         snprintf(realold_i, PATH_MAX, "%s/%s", realold, i);
         create_single_link(realold_i, oldbn);
         free(oldbn);
         free(i);
      }
      chdir("..");
      link_inside(realnew, bn);
      goto leave;
   }
   
   // 6: conflict for a same name
   if (!(realoldIsDir || realnewIsDir)) {
      Log_Error("Conflict: %s", realold);
      if (overwrite) {
         char dotbn[PATH_MAX+1];
         snprintf(dotbn, PATH_MAX, "./%s", bn);
         unlink(dotbn);
         create_single_link(realnew, bn);
         Log_Normal("Replaced with: %s", realnew);
      }
      goto leave;
   }
   
   // 7: if one of [realold, realnew] is a dir and the other isn't, we have an "unsolvable" conflict
   if ( (!realoldIsDir && realnewIsDir) || (realoldIsDir && !realnewIsDir) ) {
      Log_Error("Conflict: cannot create expanded directory '%s'.", bn);
      goto leave; 
   }
   
   Log_Error("SERIOUS: Should never enter here.");
   
  leave:
   free(realnew);
   free(bn);
}

int main(int argc, char** argv) {
   goboPrograms = getenv("goboPrograms");
   if (!goboPrograms) {
      Log_Error("Could not determine $goboPrograms");
      exit(1);
   }
   lenGoboPrograms = strlen(goboPrograms);
   // if goboPrograms ends with a '/'
   if (goboPrograms[lenGoboPrograms - 1] == '/')
      lenGoboPrograms--;
   realpath(goboPrograms, realpathGoboPrograms);
   if (argc < 2) {
      fprintf(stderr, "Usage: %s <dir> [--overwrite] [--relative]\n", argv[0]);
      exit(1);
   }
   while (argc > 2) {
      argc--;
      if (strcmp(argv[argc], "--relative") == 0) {
         relative = true;
      } else if (strcmp(argv[argc], "--overwrite") == 0) {
         overwrite = true;
      }
   }

   os_dir dir = { .name = argv[1] };
   char* i;
   while ((i = os_listdir(&dir))) {
      char entry[PATH_MAX+1];
      snprintf(entry, PATH_MAX, "%s/%s", argv[1], i);
      Link_Or_Expand(entry);
      free(i);
   }

   char msg[1024];
   snprintf(msg, 1023, "Processed %d file%s.", count, count == 1 ? "" : "s");
   Log_Normal(msg);
}
