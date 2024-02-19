/*
 *  Mkpwent - add an entry to the /etc/passwd file
 *
 *  The /etc/passwd file's format is:
 *    user:password:uid:gid:info:dir:shell
 *
 *  usage: mkpwent user password uid gid info dir shell
 *
 *  Written by Steven Robbins
 */

#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


static char *me;	/* name of this program */


void usage(void){
    fprintf(stderr, "usage: %s user password uid gid info dir shell\n", me);
    fprintf(stderr, "\t-uid of '-' means one greater than the last entry in "
		    "passwd file\n");
    exit(1);
    }


void fail(char *s1, char *s2){
    fprintf(stderr, "%s: ", me);
    fprintf(stderr, s1, s2);
    exit(1);
    }


uid_t uid(char *ch_uid){
/*
 *  Can be either a number or a dash (= last entry + 1)
 */
    long u;
    struct passwd *p;

    if (*ch_uid == '-'){
	while ((p = getpwent()) != NULL)
	    u = p->pw_uid;
	endpwent();
	return u+1;
	}

    if (sscanf(ch_uid, "%ld", &u) != 1)
	fail("invalid uid %s\n", ch_uid);
    return u;
    }


gid_t gid(char *ch_gid){
/*
 *  Can be either a number, or a name.
 */
    long g;
    struct group *gp;

    if (sscanf(ch_gid, "%ld", &g) == 1)
	return g;

    gp = getgrnam(ch_gid);
    endgrent();
    if (gp == NULL)  fail("invalid gid %s\n", ch_gid);
    return gp->gr_gid;
    }


char itos(int i){	/* map integer 0-63 into salt character */
    i = i & 0x3F;
    if (i < 26)		return 'a' + i;
    if (i < 52)		return 'A' + i - 26;
    if (i < 62)		return '0' + i - 52;
    /* else */		return (i == 62 ? '.' : '/');
    }


char *password(char *plain){
/*
 *  Maps a plaintext password into encrypted form.  If password starts
 *  with '*', it is not encrypted.
 */
    char salt[2];	/* salt characters from set [a-zA-Z0-9./] */
    time_t tm;

    if (*plain == '*')  return plain;

    time(&tm);
    salt[0] = itos(tm);
    salt[1] = itos(tm >> 6);

    return crypt(plain, salt);
    }


int main(int argc, char *argv[]){
    struct passwd pw;
    int pw_check(struct passwd *);
    int pw_write(struct passwd *);

    me = argv[0];
    if (argc != 8) usage();

    pw.pw_name = argv[1];
    pw.pw_passwd = password(argv[2]);
    pw.pw_uid = uid(argv[3]);
    pw.pw_gid = gid(argv[4]);
    pw.pw_gecos = argv[5];
    pw.pw_dir = argv[6];
    pw.pw_shell = argv[7];

    return !(pw_check(&pw) && pw_write(&pw));
    }


int pw_check(struct passwd *pw){
/*
 *  Sanity checks for everything.
 */
    char *s;
    struct passwd *p;
    struct stat st;

    /* check name */
    s = pw->pw_name;
    if (s == NULL || *s == '\0')
	fail("null username\n", "");

    /* check for duplicate name or uid */
    while ((p = getpwent()) != NULL){
	if (!strcmp(p->pw_name, pw->pw_name))
	    fail("username in use: %s\n", p->pw_name);
	if (p->pw_uid == pw->pw_uid)
	    fail("uid in use by %s\n", p->pw_name);
	}

    /* check that home directory (if it exists) is rwx */
    if (stat(pw->pw_dir, &st) == 0){
	if (!S_ISDIR(st.st_mode))
	    fail("file '%s' is not a directory!\n", pw->pw_dir);
	if (st.st_uid != pw->pw_uid)
	    fail("directory '%s' not owned by user\n", pw->pw_dir);
	if (st.st_gid != pw->pw_gid)
	    fail("directory '%s' hasn't the correct gid\n", pw->pw_dir);
	if ((st.st_mode & S_IRWXU) != S_IRWXU)
	    fail("directory '%s' is not rwx!\n", pw->pw_dir);
	}

    /* check for execute access on shell */
    if (stat(pw->pw_shell, &st) == 0){
	if (!S_ISREG(st.st_mode))
	    fail("shell '%s' is not a regular file!\n", pw->pw_shell);
	if ( !(st.st_mode & S_IXOTH) &&
	     !((st.st_mode & S_IXGRP) && (st.st_gid == pw->pw_gid)) &&
	     !((st.st_mode & S_IXUSR) && (st.st_uid == pw->pw_uid)) )
	    fail("no execute access for shell '%s'\n", pw->pw_shell);
	}
	else fail("can't stat the shell '%s'\n", pw->pw_shell);

    return 1;
    }


int pw_write(struct passwd *pw){
    FILE *fp;
    struct passwd *p;

    if (access("/etc/ptmp", 0) == 0)
	fail("/etc/ptmp exists\n", "");

    if (!(fp = fopen("/etc/ptmp", "w")))
	fail("can't open /etc/ptmp\n", "");

    setpwent();
    while ((p = getpwent()) != NULL)
	if (putpwent(p, fp) < 0) {
	    fail("error copying to new passwd file\n", "");
	    }
    if (putpwent(pw, fp) < 0){
	fail("error writing new passwd file (new entry)\n", "");
	}

    fclose(fp);
    endpwent();

    unlink("/etc/passwd.OLD");
    link("/etc/passwd", "/etc/passwd.OLD");
    unlink("/etc/passwd");
    link("/etc/ptmp", "/etc/passwd");
    unlink("/etc/ptmp");
    chmod("/etc/passwd", 0644);

    return 1;
    }
