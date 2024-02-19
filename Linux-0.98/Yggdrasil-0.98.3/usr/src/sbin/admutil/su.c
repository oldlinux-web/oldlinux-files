/* 
 * su.c - Another 'su' command for Linux
 * Created 2-Jul-92 by Peter Orbaek <poe@daimi.aau.dk>
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <errno.h>

void su_to(char *);

void main(int argc, char *argv[])
{
	int login = 0;
	char *user = "root";
	
	if(argc > 1 && !strcmp("-", argv[1])) {
		login = 1;
		argv++; argc--;
	} 
	
	if(argc == 2) {
		user = argv[1];
	}

	if(argc > 2) {
		fprintf(stderr, "Usage: su [-] [username]\n");
		exit(1);
	}

	if(login) {
		setgid(getgid());
		setuid(getuid());
		execlp("login", "login", user, (char *)0);
		perror("su: exec login failed");
		exit(1);

	} else
	  su_to(user);
	/*NOT REACHED*/
}

void su_to(char *user)
{
	struct passwd *pwd, *p;
	struct group *grp;
	char **g;
	char *pwdstr, *myself;

	pwd = getpwnam(user);
	if(!pwd) {
		fprintf(stderr, "su: Who is \"%s\"?\n", user);
		exit(1);
	}

	if(pwd->pw_uid == getuid()) {
		fprintf(stderr, "su: You already are \"%s\"\n", user);
		exit(0);
	}

	if(p = getpwuid(getuid()))
		myself = p->pw_name;
	else {
		fprintf(stderr, "Who are you?\n");
		exit(1);
	}

	grp = getgrgid(pwd->pw_gid);
	if(grp && getuid()) {
		for(g = grp->gr_mem; *g; g++)
			if(!strcmp(myself, *g)) break;
		if(! *g) {
			fprintf(stderr,
				"su: You can't su to groups you are not in\n");
			exit(1);
		}
	}

	if(getuid() && pwd->pw_passwd && pwd->pw_passwd[0]) {
		pwdstr = getpass("Password: ");
		if(strncmp(pwd->pw_passwd, crypt(pwdstr,pwd->pw_passwd),14)) {
			fprintf(stderr, "Sorry.\n");
			/* syslog() this */
			exit(1);
		}
	}

	if(setgid(pwd->pw_gid) < 0) {
		perror("su: setgid");
		exit(1);
	}

	if(setuid(pwd->pw_uid) < 0) {
		perror("su: setuid");
		exit(1);
	}

	initgroups(user, pwd->pw_gid);
	execl(pwd->pw_shell, pwd->pw_shell, (char *)0);

	fprintf(stderr, "su: exec %s failed: %s\n", pwd->pw_shell,
		strerror(errno));
	exit(1);
}

