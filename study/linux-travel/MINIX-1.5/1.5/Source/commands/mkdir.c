/* mkdir - make a directory		Author: Adri Koppes */

#include <sys/types.h>
#include <signal.h>

int error = 0;

main(argc, argv)
int argc;
char **argv;
{
  if (argc < 2) {
	std_err("Usage: mkdir directory...\n");
	exit(1);
  }
  signal(SIGHUP, SIG_IGN);
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  while (--argc) makedir(*++argv);
  if (error) exit(1);
}

makedir(dirname)
char *dirname;
{
  char dots[128], parent[128];
  int sl = 0, i = 0;

  while (dirname[i])
	if (dirname[i++] == '/') sl = i;
  strncpy(parent, dirname, sl);
  parent[sl] = '\0';
  strcat(parent, ".");
  if (access(parent, 2)) {
	stderr3("mkdir: can't access ", parent, "\n");
	exit(1);
  }
  if (mkdir(dirname, 0777)) {
	stderr3("mkdir: can't create ", dirname, "\n");
	error++;
	return;
  }
}

stderr3(s1, s2, s3)
char *s1, *s2, *s3;
{
  std_err(s1);
  std_err(s2);
  std_err(s3);
}
