/*
 *  PWDutil.c - /etc/passwd access utility
 *
 *  The /etc/passwd file's format is:
 *    user:password:uid:gid:gecos:dir:shell
 *
 *  Functions performed by pwdutil
 *	- add/remove entries
 *	- disable/reenable login for user
 *	- change any field of an existing entry
 *
 *  Usage: pwdutil [ardec] [-f] <username> [parameters]
 *	- "-f" forces issue: no checks are done
 *	- for remove/disable/enable no parameters are needed
 *	- for add the parameters are: uid gid gecos dir shell
 *	  (and password on stdin!)
 *	- for change, the parameters are as above, preceeded by a flag, '-u',
 *	  '-p', etc.  Password is read from stdin
 *
 *  Written by Steven Robbins
 *
 *  v1.00 - released on 10 August 1992 as mkpwent
 *  v1.01 - fixed security hole, and extended functionality
 */

#include <pwd.h>
#include <grp.h>
#include <getopt.h>
#include <time.h>
#include <sys/stat.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


static char *me;			/* name of this program */
static int force_it = 0;


void 
fail(char *fmt, ...){
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    endpwent();
    exit(1);
    }


uid_t 
uid(char *ch_uid) {
/*
 *  Can be either a number or a '+' (uid of last entry +1) or '+gid' (uid of
 *  last entry with this gid +1).  The gid can be a string.
 */
    long u;
    gid_t gid(char *);
    int g = -1;
    struct passwd *p;

    if (*ch_uid == '+') {
	if (ch_uid[1] != '\0')  g = gid(&(ch_uid[1]));

	u = g-1;	/* if nobody with this group, return g */
	while ((p = getpwent()) != NULL)
	    if (g == -1 || g == p->pw_gid)  u = p->pw_uid;
	endpwent();
	fprintf(stderr,"Uid %s --> %d\n", ch_uid, ++u);
	return u;
	}

    if (sscanf(ch_uid, "%ld", &u) != 1)
	fail("invalid uid %s\n", ch_uid);
    return u;
    }


gid_t 
gid(char *ch_gid){
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


char 
itos(int i){		/* map integer 0-63 into salt character */
    i = i & 0x3F;
    if (i < 26)		return 'a' + i;
    if (i < 52)		return 'A' + i - 26;
    if (i < 62)		return '0' + i - 52;
    /* else */		return (i == 62 ? '.' : '/');
    }


char *
password(){
/*
 *  Maps a plaintext password into encrypted form.  If password starts
 *  with '*', it is not encrypted.
 */
    char salt[2];	/* salt characters from set [a-zA-Z0-9./] */
    time_t tm;
    char *plain;

    plain = getpass("New password: ");
    if (isatty(0)) {
	char *p_again;
	if ((p_again = malloc(strlen(plain)+1)) == NULL)
	    fail("out of memory\n");
	strcpy(p_again,plain);
	plain = getpass("Retype new password: ");
	if (strcmp(plain, p_again))
	    fail("the passwords didn't match -- nothing changed\n");
	}

    if (*plain == '*')  return plain;

    time(&tm);
    salt[0] = itos(tm);
    salt[1] = itos(tm >> 6);

    return crypt(plain, salt);
    }


void 
pw_check(struct passwd *pw, int new_entry){
/*
 *  Sanity checks for things.  Returning is successful.
 */
    char *s;
    struct passwd *p;
    struct stat st;

    /* check name */
    s = pw->pw_name;
    if (s == NULL || *s == '\0')
	fail("null username invalid\n");

    if (force_it) return;

    /* check for duplicate name or uid if a new entry */
    if (new_entry)
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
	else fail("can't find the shell '%s'\n", pw->pw_shell);
    }


/*
**  Here are the function functions.
*/

struct passwd * 
do_add(int argc, char *argv[]) {
/*
**  argv has: name uid gid gecos dir shell
*/
    static struct passwd p;

    if (argc != 6) fail("required parameters: name uid gid name dir shell\n");

    p.pw_name = argv[0];
    p.pw_uid = uid(argv[1]);
    p.pw_gid = gid(argv[2]);
    p.pw_gecos = argv[3];
    p.pw_dir = argv[4];
    p.pw_shell = argv[5];

    pw_check (&p, 1);

    p.pw_passwd = password();
    return &p;
    }


struct passwd *
do_remove(char *name) {
    struct passwd *p;

    if ((p = getpwnam(name)) == NULL)
	fail ("unknown user: %s\n", name);
    return p;
    }


struct passwd *
do_disable(char *name) {
/*
**  Prepend a '*' to the password field.
*/
    struct passwd *p;
    char *pass;

    if ((p = getpwnam(name)) == NULL)
	fail ("unknown user: %s\n", name);

    if (*(p->pw_passwd) == '*')
	fail ("logins for %s already disabled\n", name);

    if ((pass = malloc( strlen(p->pw_passwd)+2 )) == NULL)
	fail ("out o' memory -- sorry\n", "");

    pass[0] = '*';
    strcpy ( &(pass[1]), p->pw_passwd );
    p->pw_passwd = pass;
    return p;
    }


struct passwd *
do_enable(char *name) {
/*
**  Remove leading '*' from the password field.
*/
    struct passwd *p;

    if ((p = getpwnam(name)) == NULL)
	fail ("unknown user: %s\n", name);

    if (*(p->pw_passwd) != '*')
	fail ("logins for %s not disabled\n", name);

    p->pw_passwd++;
    return p;
    }


struct passwd *
do_change(int argc, char *argv[]) {
/*
**  argv has: <name> [-p] [-u uid] [-g gid] [-n gecos] [-d dir] [-s shell]
**
*/
    int c;
    struct passwd *p;

    if (argc < 2)
	fail("ya need to specify something to change!\n");

    if ((p = getpwnam(argv[0])) == NULL)
	fail("unknown userid: %s\n", argv[0]);

    argv[0] = "change";
    while ((c = getopt(argc, argv, "pu:g:n:d:s:")) != EOF)
	switch (c) {
	    case 'p': p->pw_passwd = password(); break;
	    case 'u': p->pw_uid = uid(optarg);  break;
	    case 'g': p->pw_gid = gid(optarg);  break;
	    case 'n': p->pw_gecos = optarg;  break;
	    case 'd': p->pw_dir = optarg;  break;
	    case 's': p->pw_shell = optarg;  break;
	    case '?':
	    default : fail("unknown change option `%c'\n", c);
	    }
    pw_check (p, 0);
    return p;
    }


#define		ADD_ENTRY	1
#define		REMOVE_ENTRY	2
#define		CHANGE_ENTRY	3

void 
pw_write(struct passwd *p_new, int flag){
    FILE *fp;
    struct passwd *p;

    if (access("/etc/ptmp", 0) == 0)
	fail("/etc/ptmp exists\n");

    if (!(fp = fopen("/etc/ptmp", "w")))
	fail("can't open /etc/ptmp\n");

    setpwent();
    while ((p = getpwent()) != NULL) {
	if (p_new != NULL && p->pw_uid >= p_new->pw_uid) {
	    if (flag == ADD_ENTRY) {
		if (putpwent(p_new, fp) < 0)
		    fail("error copying %s to new passwd file\n", p_new->pw_name);
		p_new = NULL;
		}
	    if (flag == REMOVE_ENTRY && !strcmp(p->pw_name, p_new->pw_name)){
		p_new = NULL;
		continue;
		}
	    if (flag == CHANGE_ENTRY && !strcmp(p->pw_name, p_new->pw_name)) {
		p = p_new;
		p_new = NULL;
		}
	    }
	if (putpwent(p, fp) < 0)
	    fail("error copying %s to new passwd file\n", p->pw_name);
	}

    if ((p_new != NULL) && (putpwent(p_new, fp) < 0))
	fail("error copying %s to new passwd file\n", p_new->pw_name);

    fclose(fp);
    endpwent();

    unlink("/etc/passwd.OLD");
    link("/etc/passwd", "/etc/passwd.OLD");
    unlink("/etc/passwd");
    link("/etc/ptmp", "/etc/passwd");
    unlink("/etc/ptmp");
    chmod("/etc/passwd", 0644);
    }


int 
main(int argc, char *argv[]) {
/*
**  argv is:  pwdutil <funct> [-f] <name> [stuff]
*/
    int funct, flag = 0;
    struct passwd *pw = NULL;
    int newargc;
    char **newargv;

    me = argv[0];
    if (argc < 3)  fail("at least two arguments are required\n");
    funct = *argv[1];

    if (argv[2][0] == '-' && argv[2][1] == 'f') {
	fprintf(stderr,"Overriding sanity checks!\n");
	force_it = 1;
	newargc = argc-3;
	newargv = &(argv[3]);
	}
    else {
	newargc = argc-2;
	newargv = &(argv[2]);
	}

    switch (funct){
	case 'a':  pw = do_add(newargc, newargv);
		   flag = ADD_ENTRY;  break;
	case 'r':  pw = do_remove(*newargv);  
		   flag = REMOVE_ENTRY;  break;
	case 'd':  pw = do_disable(*newargv);  
		   flag = CHANGE_ENTRY;  break;
	case 'e':  pw = do_enable(*newargv);
		   flag = CHANGE_ENTRY;  break;
	case 'c':  pw = do_change(newargc, newargv);
		   flag = CHANGE_ENTRY;  break;
	default : fail("usage: %s [ardec] <user> [parameters]\n", me);
	}

    pw_write(pw, flag);
    return 0;
    }
