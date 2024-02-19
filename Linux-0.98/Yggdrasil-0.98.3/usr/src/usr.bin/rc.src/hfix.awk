/^extern void b_exec\(char \*\*\), funcall\(char \*\*\), b_dot\(char \*\*\), b_builtin\(char \*\*\);$/ {
				print "extern void b_exec(), funcall(), b_dot(), b_builtin();";
				next;
				}
/^extern void qsort\(void \*, SIZE_T, SIZE_T, int \(\*\)\(const void \*, const void \*\)\);$/ {
				print "extern void qsort();";
				next;
				}
/^extern Node \*treecpy\(Node \*s, void \*\(\*\)\(SIZE_T\)\);$/ {
				print "extern Node *treecpy();";
				next;
				}
/^extern .*\);/ || /^typedef .*\);/		{
				n = split($0, tmp1, "(");
				if (n != 2) {
					print | "sh -c 'cat >&2'";
					print "ERROR with left parens: ", n | "sh -c 'cat >&2'";
					next;
				}
				printf "%s();\n", tmp1[1];
				next;
				}
				{ print; }
