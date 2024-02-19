BEGIN				{ Pending = 0; }
#
# Special cases.  There are too many, but....
#
$0 == "static int (*realgchar)(void);"	{
				print "static int (*realgchar)();";
				next;
				}
$0 == "static void (*realugchar)(int);"	{
				print "static void (*realugchar)();";
				next;
				}
$0 == "static void b_break(char **), b_cd(char **)," {
				print "static void b_break(), b_cd(),";
				next;
				}
$0 == "\tb_echo(char **), b_eval(char **), b_exit(char **), b_limit(char **)," {
				print "b_echo(), b_eval(), b_exit(), b_limit(),";
				next;
				}
$0 == "\tb_return(char **), b_shift(char **), b_umask(char **), b_wait(char **)," {
				print "b_return(), b_shift(), b_umask(), b_wait(),";
				next;
				}
$0 == "\tb_whatis(char **);"	{
				print "b_whatis();";
				next;
				}
$0 == "Node *treecpy(Node *s, void *(*alloc)(SIZE_T)) {" {
				print "Node *treecpy(s, alloc)";
				printf "Node *s;\nvoid *(*alloc)();\n{\n";
				next;
				}
$0 == "\tvoid (*handler)(int);"	{
				print "\tvoid (*handler)();";
				next;
				}
$0 == "\textern DIR *opendir(const char *);" {
				print "#if 0";
				print;
				next;
				}
$0 == "\textern int closedir(DIR *);" {
				print;
				print "#endif";
				next;
				}
$0 == "\textern int getopt(int, char **, char *);"	{
				print "\textern int getopt();";
				next;
				}
$0 == "\tstatic void (*vectors[])(char *, List *, boolean) = {"	{
				print "\tstatic void (*vectors[])() = {";
				next;
				}
$0 == "\t\tvoid (*handler)(int);"	{
				print "\t\tvoid (*handler)();";
				next;
				}
$0 == "#include <stdarg.h>"	{
				print "#include <varargs.h>";
				next;
				}
#
# General cases.
#
/^static .*\);$/		{
				n = split($0, tmp1, "(");
				if (n != 2) {
					print | "sh -c 'cat >&2'";
					print "ERROR with left parens: ", n | "sh -c 'cat >&2'";
					next;
				}
				printf "%s();\n", tmp1[1];
				next;
				}
/^extern .*\);$/		{
				n = split($0, tmp1, "(");
				if (n != 2) {
					print | "sh -c 'cat >&2'";
					print "ERROR with left parens: ", n | "sh -c 'cat >&2'";
					next;
				}
				printf "%s();\n", tmp1[1];
				next;
				}
/^[a-zA-Z_][a-zA-Z_0-9]*:/	{
				print;
				next;
				}
/^[^ \t#][^(]*\([^)]*\.\.\.\).*{/ {
				if (Pending != 0) {
					print "ERROR: Pending != 0 on function entry" | "sh -c 'cat >&2'";
					next;
				}
				n = split($0, tmp1, "(");
				if (n != 2) {
					print | "sh -c 'cat >&2'";
					print "ERROR with left parens: ", n | "sh -c 'cat >&2'";
					next;
				}
				n = split(tmp1[2], tmp2, ")");
				if (n != 2) {
					print | "sh -c 'cat >&2'";
					print "ERROR with right parens: ", n | "sh -c 'cat >&2'";
					next;
				}
				printf "%s(va_alist)\nva_dcl\n{\n", tmp1[1];
				nargs = split(tmp2[1], args, ",");
				for (i = 1; i <= nargs; i++) {
					if (args[i] == "...") {
						if (i == nargs) {
							Pending = 1;
							continue;
						}
						print "ERROR: ... isn't last argument", $0 | "sh -c 'cat >&2'";
						next;
					}
					decls[i] = args[i] ";";
					t = args[i];
					for (;;) {
						n = split(t, varname, " ");
						if (n > 1) {
							t = varname[n];
							continue;
						}
						n = split(t, varname, "*");
						if (n > 1) {
							t = varname[n];
							continue;
						}
						break;
					}
					type = substr(args[i], 1, length(args[i]) - length(t));
					picks[i] = t " = va_arg(ap, " type ");"

				}
				nargs--;
				next;
				}
Pending == 1 && /va_list/	{
				for (i = 1; i <= nargs; i++)
					print decls[i];
				}
Pending == 1 && /va_start/	{
				n = split($0, tmp1, ",");
				if (n != 2) {
					print "ERROR: bad va_start format" | "sh -c 'cat >&2'";
					next;
				}
				printf "%s);\n", tmp1[1];
				for (i = 1; i <= nargs; i++)
					print picks[i];
				Pending = 0;
				next;
				}
/^[^ \t#][^(]*\([^)]*\).*{/	{
				if (Pending != 0) {
					print "ERROR: Pending != 0 on function entry" | "sh -c 'cat >&2'";
					next;
				}
				n = split($0, tmp1, "(");
				if (n != 2) {
					print | "sh -c 'cat >&2'";
					print "ERROR with left parens: ", n | "sh -c 'cat >&2'";
					next;
				}
				n = split(tmp1[2], tmp2, ")");
				if (n != 2) {
					print | "sh -c 'cat >&2'";
					print "ERROR with right parens: ", n | "sh -c 'cat >&2'";
					next;
				}
#				if (tmp2[2] != " {") {
#					print "ERROR with last component: `", tmp2[2], "'";
#					next;
#				}
				if (tmp2[1] ~ /^[ \t]*void[ \t]*$/ || tmp2[1] ~ /^[ \t]*$/) {
					# no arguments
					printf "%s() {\n\n", tmp1[1];
					next;
				}
				printf "%s(", tmp1[1];
				needcomma = 0;
				nargs = split(tmp2[1], args, ",");
				for (i = 1; i <= nargs; i++) {
					if (args[i] == "...") {
						print "ERROR: ... shouldn't get here", $0 | "sh -c 'cat >&2'";
						next;
					}
					decls[i] = args[i] ";";
					if (needcomma)
						printf ", ";
					needcomma = 1;
					t = args[i];
					if (t ~ /\[\]$/)
						t = substr(t, 1, length(t) - 2);
					for (;;) {
						n = split(t, varname, " ");
						if (n > 1) {
							t = varname[n];
							continue;
						}
						n = split(t, varname, "*");
						if (n > 1) {
							t = varname[n];
							continue;
						}
						break;
					}
					printf "%s", varname[n];
				}
				print ")";
				for (i = 1; i <= nargs; i++)
					print decls[i];
				print "{";
				next;
				}
#
# If we didn't match at all, the line doesn't change.
#
				{ print; }
