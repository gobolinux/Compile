#include <stdio.h>
#include <string.h>
#include <errno.h>

inline int endsWith(char* haystack, char* needle) {
  char* objext = strstr(haystack, needle);
  if (objext) {
    objext += strlen(needle);
    if (! *objext)
      return 1;
  }
  return 0;
}

int main(int argc, char** argv) {
  char head[2];
  int ok;
  FILE* file;
  if (argc < 2)
    return 100;
  if ( (! strcmp(argv[1], "core")) || (endsWith(argv[1], ".o")) )
    return 1;
  file = fopen(argv[1], "r");
  if (errno)
    return 100;
  ok = fread(&head, 2, 1, file);
  fclose(file);
  if (!ok)
    return 100;
  if ((head[0]==127&&head[1]=='E') || (head[0]=='#' && head[1]=='!'))
    return 0;
  else
    return 1;
}
