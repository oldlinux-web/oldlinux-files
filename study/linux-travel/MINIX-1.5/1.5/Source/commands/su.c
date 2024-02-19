/* su - become super-user		Author: Patrick van Kleef */

#include <sys/types.h>
#include <sgtty.h>
#include <pwd.h>

main(argc, argv)
int argc;
char *argv[];
{
  register char *name;
  char *crypt();
  char *shell = "/bin/sh";
  char *shell2 = "/usr/bin/sh";
  int nr;
  char password[14];
  struct sgttyb args;
  register struct passwd *pwd;
  struct passwd *getpwnam();

  if (argc > 1)
	name = argv[1];
  else
	name = "root";

  if ((pwd = getpwnam(name)) == 0) {
	std_err("Unknown id: ");
	std_err(name);
	std_err("\n");
	exit(1);
  }
  if (pwd->pw_passwd[0] != '\0' && getuid() != 0) {
	std_err("Password: ");
	ioctl(0, TIOCGETP, &args);	/* get parameters */
	args.sg_flags = args.sg_flags & (~ECHO);
	ioctl(0, TIOCSETP, &args);
	nr = read(0, password, 14);
	password[nr - 1] = 0;
	std_err("\n");
	args.sg_flags = args.sg_flags | ECHO;
	ioctl(0, TIOCSETP, &args);
	if (strcmp(pwd->pw_passwd, crypt(password, pwd->pw_passwd))) {
		std_err("Sorry\n");
		exit(2);
	}
  }
  setgid(pwd->pw_gid);
  setuid(pwd->pw_uid);
  if (pwd->pw_shell[0]) shell = pwd->pw_shell;
  execl(shell, shell, "-i", (char *) 0);
  execl(shell2, shell2, "-i", (char *) 0);
  std_err("No shell\n");
  exit(3);
}
