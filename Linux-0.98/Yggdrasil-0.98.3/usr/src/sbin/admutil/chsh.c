/* chsh.c - change ones shell */

#include <sys/types.h>
#include <stdio.h>
#include <pwd.h>
#include <unistd.h>

char *getpass();

#define MAX_SHELLS 20
#define until(b) while(!(b))

char buffer[8000];

main(argc, argv)
	int argc;
	char *argv[];
{
	struct passwd *pe;
	char *pwdstr, *newsh, *bufptr;
	FILE *fp;
	char *shells[MAX_SHELLS];
	int n,nr;
	uid_t uid;
		
	if(argc > 2) {
		puts("Too many arguments");
		exit(1);
	} else if(argc == 2) {
		pe = getpwnam(argv[1]);
		if(!pe) {
			printf("Unknown user: %s\n", argv[1]);
			exit(1);
		}
		if(getuid()) {
			puts("Only root can change the shell for others");
			exit(1);
		}
		uid = pe->pw_uid;
	} else {
		uid = getuid();
	}
	

	if(!(pe = getpwuid(uid))) {
		puts("Can't find you in password file?!");
		exit(1);
	}

	if(getuid() && pe->pw_passwd && pe->pw_passwd[0]) {
		pwdstr = getpass("Enter password: ");
		if(strncmp(pe->pw_passwd, crypt(pwdstr, pe->pw_passwd), 14)) {
			puts("Illegal password, imposter.");
			exit(1);
		}
	}

	printf("The current shell is: %s\n", pe->pw_shell);

redo_it:
	if(fp = fopen("/etc/shells", "r")) {
		puts("You can choose one of the following:");
		nr = 1;
		bufptr = buffer;
		while(fgets(bufptr, 79, fp) && nr < MAX_SHELLS 
		      && bufptr < buffer + 7999) {
			printf("%d: %s", nr, bufptr);
			bufptr[strlen(bufptr)-1] = '\0';
			shells[nr-1] = bufptr;
			bufptr += strlen(bufptr)+1;
			nr++;
		}
		fclose(fp);
		
		do {
			printf("Enter a number between 1 and %d: ", nr-1);
			fflush(stdout);
			scanf("%d", &n);
		} until(1 <= n && n <= nr-1);
		newsh = shells[n-1];
	} else {
		printf("Enter the full path of the new shell: ");
		fflush(stdout);
		fgets(buffer, 70, stdin);
		buffer[strlen(buffer)-1] = '\0';
		newsh = buffer;
	}
	
	if(access(newsh, 0100) < 0) {
		printf("Can't find %s, try again...\n", newsh);
		goto redo_it;
	}

	/* now write new passwd file */
	if(access("/etc/ptmp", 0) == 0) {
		puts("/etc/ptmp exists, can't change shell");
		exit(1);
	}
	
	if(!(fp = fopen("/etc/ptmp", "w"))) {
		puts("Can't open /etc/ptmp, can't change shell");
		exit(1);
	}

	setpwent();
	while(pe = getpwent()) {
		if(uid == pe->pw_uid) {
			pe->pw_shell = newsh;
		}
		if(putpwent(pe, fp) < 0) {
			puts("Error while writing new password file, shell not changed.");
			fclose(fp);
			endpwent();
			unlink("/etc/ptmp");
			exit(1);
		}
	}
	fclose(fp);
	endpwent();

	unlink("/etc/passwd.OLD");
	link("/etc/passwd", "/etc/passwd.OLD");
	unlink("/etc/passwd");
	link("/etc/ptmp", "/etc/passwd");
	unlink("/etc/ptmp");
	chmod("/etc/passwd", 0644);

	puts("Shell changed.");	
	
}
