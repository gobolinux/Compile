/*
 * Automatically logins the superuser. Needed in the LiveCD.
 * Written by Lucas C. Villa Real
 */ 
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

int main()
{
	struct passwd *pwp = getpwuid(0);
	const char *user = pwp->pw_name ? pwp->pw_name : "gobo";

	execlp("login", "login", "-f", user, NULL);
	return 0;
}
