typedef void builtin_t(char **);

extern builtin_t *isbuiltin(char *);
extern void b_exec(char **), funcall(char **), b_dot(char **), b_builtin(char **);
extern char *which(char *, boolean);
