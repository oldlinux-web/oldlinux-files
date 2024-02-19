#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <a.out.h>
#include <errno.h>

#define VERSION "1.1"

int main(int argc, char **argv)
{
  int i;
  
  while ((i = getopt(argc, argv, "v")) != EOF)
    switch (i)
      {
      case 'v':
	printf("%s: version %s (%s)\n", argv[0], VERSION, __DATE__);
	break;
      }

  if (optind >= argc)
    {
      printf("usage: %s [-v] prog ...\n", argv[0]);
      exit(0);
    }

  for (i = optind; i < argc; i++)
    {
      pid_t pid;
      FILE *file;
      struct exec exec;
      char errmsg[256];

      if ((file = fopen(argv[i], "rb")) == NULL)
	{
	  sprintf(errmsg, "%s: can't open '%s'", argv[0], argv[i]);
	  perror(errmsg);
	  continue;
	}

      if (fread(&exec, sizeof exec, 1, file) < 1 ||
	  (N_MAGIC(exec) != ZMAGIC && N_MAGIC(exec) != OMAGIC))
	{
	  fprintf(stderr, "%s: '%s' is not an executable file\n", argv[0], argv[i]);
	  fclose(file);
	  continue;
	}

      if (optind < argc-1)
	{
	  printf("%s:\n", argv[i]);
	  fflush(stdout);
	}

      pid = fork();

      if (pid < 0)
	{
	  sprintf(errmsg, "%s: can't fork", argv[0]);
	  perror(errmsg);
	  exit(1);
	}
      else if (pid == 0)
	{
	  execl(argv[i], NULL);
	  sprintf(errmsg, "%s: can't execute '%s'", argv[0], argv[i]);
	  perror(errmsg);
	  exit(1);
	}
      else
	waitpid(pid, NULL, 0);

      fclose(file);
    }

  exit(0);
}
