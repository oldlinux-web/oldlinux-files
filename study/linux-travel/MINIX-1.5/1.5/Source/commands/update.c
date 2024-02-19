/* update - do sync periodically		Author: Andy Tanenbaum */

#include <sys/types.h>
#include <signal.h>

main()
{
  int fd, buf[2];


  /* Disable SIGTERM */
  signal(SIGTERM, SIG_IGN);

  /* Release all (?) open file descriptors. */
  close(0);
  close(1);
  close(2);

  /* Release current directory to avoid locking current device. */
  chdir("/");

  /* Open some files to hold their inodes in core. */
/*open("/bin", 0);*/
/*open("/lib", 0);*/
/*open("/etc", 0);*/
/*open("/tmp", 0);*/


  /* Flush the cache every 30 seconds. */
  while (1) {
	sync();
	sleep(30);
  }
}
