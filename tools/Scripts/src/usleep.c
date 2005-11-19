
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
   if (argc < 2) {
      fprintf(stderr, "Usage: usleep <usecs>");
      exit(1);
   }
   usleep(atoi(argv[1]));
}
