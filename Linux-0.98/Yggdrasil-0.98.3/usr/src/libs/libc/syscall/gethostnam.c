#include <string.h>
#include <unistd.h>
#include <errno.h>

int gethostname(char *name, int len)
{
	struct utsname uts;

	uname(&uts);
	if (strlen(uts.nodename)+1 > len) {
		errno = EINVAL;
		return -1;
	}
	strcpy(name, uts.nodename);
	return 0;
}
