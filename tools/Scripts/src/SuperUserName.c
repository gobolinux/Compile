#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>

int main(int argc, char **argv)
{
	struct passwd *pwp = getpwuid(0);
	printf("%s\n", pwp ? pwp->pw_name : "root");
	return 0;
}

