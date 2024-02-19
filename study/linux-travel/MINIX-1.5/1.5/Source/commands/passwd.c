/* passwd - change a passwd		Author: Adri Koppes */

#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>

char pwd_file[] = "/etc/passwd";
char pw_tmp[] = "/etc/pwtemp";
char bad[] = "Permission denied\n";
char buf[512];

main(argc, argv)
int argc;
char *argv[];
{
  int uid, cn, n;
  int fpin, fpout;
  long salt;
  struct passwd *pwd, *getpwnam(), *getpwuid(), *getpwent();
  char name[9], password[14], sl[2];
  char *getpass(), *crypt(), *itoa();

  uid = getuid();

  if (!access(pw_tmp, 0)) {
	std_err("Temporary file in use.\nTry again later\n");
	exit(1);
  }
  if (argc < 2) {
	pwd = getpwuid(uid);
	strcpy(name, pwd->pw_name);
  } else {
	strcpy(name, argv[1]);
	pwd = getpwnam(name);
  }
  if (!pwd || ((uid != pwd->pw_uid) && uid)) {
	std_err(bad);
	exit(1);
  }
  signal(SIGHUP, SIG_IGN);
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  prints("Changing password for %s\n", name);
  if (pwd->pw_passwd[0] && uid)
	if (strcmp(pwd->pw_passwd, crypt(getpass("Old password: "), pwd->pw_passwd))) {
		std_err(bad);
		exit(1);
	}
  strcpy(password, getpass("New password: "));
  if (password[0] == '\0') {
	std_err("password cannot be null\n");
	exit(1);
  }
  if (strcmp(password, getpass("Retype password: "))) {
	std_err("Passwords don't match\n");
	exit(1);
  }
  time(&salt);
  sl[0] = (salt & 077) + '.';
  sl[1] = ((salt >> 6) & 077) + '.';
  for (cn = 0; cn < 2; cn++) {
	if (sl[cn] > '9') sl[cn] += 7;
	if (sl[cn] > 'Z') sl[cn] += 6;
  }
  if (password[0]) strcpy(password, crypt(password, sl));
  umask(0);
  close(1);
  fpout = creat(pw_tmp, 0600);
  if (fpout != 1) {
	std_err("Can't create temporary file\n");
	exit(1);
  }
  setpwent();
  while ((pwd = getpwent()) != 0) {
	if (!strcmp(name, pwd->pw_name)) pwd->pw_passwd = password;
	prints("%s:%s:%s:", pwd->pw_name, pwd->pw_passwd, itoa(pwd->pw_uid));
	prints("%s:%s:%s:%s\n", itoa(pwd->pw_gid), pwd->pw_gecos, pwd->pw_dir,
	       pwd->pw_shell);
  }
  endpwent();
  close(0);
  if ((fpin = open(pw_tmp, 0)) != 0) {
	std_err("Can't reopen temporary file\n");
	exit(1);
  }
  close(fpout);
  if ((fpout = open(pwd_file, O_RDWR)) < 0) {
	std_err("Can't recreate password file\n");
	unlink(pw_tmp);
	exit(1);
  }
  while (1) {
	n = read(fpin, buf, 512);
	if (n <= 0) break;
	write(1, buf, n);
  }

  close(fpin);
  close(fpout);
  unlink(pw_tmp);
}
