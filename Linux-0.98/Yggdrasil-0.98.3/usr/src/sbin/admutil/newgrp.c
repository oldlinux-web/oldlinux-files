/* setgrp.c - by Michael Haardt. Set the gid if possible */
/* Added a bit more error recovery/reporting - poe */

#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <stdio.h>
#include "pathnames.h"

int main(int argc, char *argv[])
{
  struct passwd *pw_entry;
  struct group *gr_entry;
  char *shell;

  if ((pw_entry=getpwuid(getuid()))==(struct passwd*)0)
  {
    puts("newgrp: Who are you?");
    fflush(stdout);
    exit(1);
  }

  shell=(pw_entry->pw_shell[0] ? pw_entry->pw_shell : _PATH_BSHELL);

  if (argc<2) setgid(pw_entry->pw_gid);
  else
  {
    if ((gr_entry=getgrnam(argv[1]))==(struct group*)0) {
    	puts("newgrp: No such group.");
    	exit(1);
    } else {
      char **look;
      int notfound=1;

      if (getuid())
      {
        look=gr_entry->gr_mem;
        while (*look && (notfound=strcmp(*look++,pw_entry->pw_name)));
        if (notfound) {
        	puts("newgrp: You are not a member of this group.");
        	exit(1);
        }
        else setgid(gr_entry->gr_gid);
      }
      else setgid(gr_entry->gr_gid);
    }
  }
  setuid(getuid());
  fflush(stdout);
  execl(shell,shell,(char*)0);
  puts("No shell");
  fflush(stdout);
  exit(1);
}
