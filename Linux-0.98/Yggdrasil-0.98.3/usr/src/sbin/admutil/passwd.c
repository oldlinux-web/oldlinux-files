/* passwd.c - change password on an account */

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <pwd.h>
#include <ctype.h>
#include <time.h>

#define ascii_to_bin(c) ((c)>='a'?(c-59):(c)>='A'?((c)-53):(c)-'.')
#define bin_to_ascii(c) ((c)>=38?((c)-38+'a'):(c)>=12?((c)-12+'A'):(c)+'.')

char *getpass(char *);
char *crypt(char *, char *);

main(argc,argv)
	int argc;
	char *argv[];
{
	struct passwd *pe;
	uid_t uid;
	char *pwdstr, *cryptstr;
	char pwdstr1[10];
	int ucase, lcase, digit;
	char *p, *q;
	time_t tm;
	char salt[2];
	FILE *fp;
	int r;

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
			puts("Only root can change the password for others");
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
		pwdstr = getpass("Enter old password: ");
		if(strncmp(pe->pw_passwd, crypt(pwdstr, pe->pw_passwd), 13)) {
			puts("Illegal password, imposter.");
			exit(1);
		}
	}
	
redo_it:
	pwdstr = getpass("Enter new password: ");
	strncpy(pwdstr1, pwdstr, 9);
	pwdstr = getpass("Re-type new password: ");

	if(strcmp(pwdstr, pwdstr1)) {
		puts("You misspelled it. Password not changed.");
		exit(0);
	}
	
	if((strlen(pwdstr) < 6) && getuid()) {
		puts("The password must be at least 6 chars, try again.");
		goto redo_it;
	}
	
	ucase = lcase = 0;
	for(p = pwdstr; *p; p++) {
		ucase = ucase || isupper(*p);
		lcase = lcase || islower(*p);
		digit = digit || isdigit(*p);
	}
	
	if((!ucase || !lcase) && !digit && getuid()) {
		puts("The password must have both upper- and lowercase");
		puts("letters, or digits; try again.");
		goto redo_it;
	}
	
	r = 0;
	for(p = pwdstr, q = pe->pw_name; *q && *p; q++, p++) {
	  if(tolower(*p) != tolower(*q)) {
	    r = 1;
	    break;
	  }
	}

	for(p = pwdstr + strlen(pwdstr)-1, q = pe->pw_name;
	    *q && p >= pwdstr; q++, p--) {
	  if(tolower(*p) != tolower(*q)) {
	    r += 2;
	    break;
	  }
	}
	  
	if(r != 3) {
	  puts("Please don't use something like your username as password!");
	  goto redo_it;
	}

	/* do various other checks for stupid passwords here... */

	time(&tm);
	salt[0] = bin_to_ascii(tm & 0x3f);
	salt[1] = bin_to_ascii((tm >> 5) & 0x3f);
	cryptstr = crypt(pwdstr, salt);
	
	if(access("/etc/ptmp", 0) == 0) {
		puts("/etc/ptmp exists, can't change password");
		exit(1);
	}
	
	if(!(fp = fopen("/etc/ptmp", "w"))) {
		puts("Can't open /etc/ptmp, can't update password");
		exit(1);
	}

	setpwent();
	while(pe = getpwent()) {
		if(uid == pe->pw_uid) {
			pe->pw_passwd = cryptstr;
		}
		if(putpwent(pe, fp) < 0) {
			puts("Error while writing new password file, password not changed.");
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

	puts("Password changed.");	
	exit(0);
}
