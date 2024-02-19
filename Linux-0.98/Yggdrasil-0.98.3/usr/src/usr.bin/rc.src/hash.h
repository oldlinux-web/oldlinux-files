typedef struct Function {
	Node *def;
	char *extdef;
} Function;

typedef struct Variable {
	List *def;
	char *extdef;
	struct Variable *n;
} Variable;

typedef struct Htab {
	char *name;
	void *p;
} Htab;

extern Htab *fp, *vp;

#define lookup_fn(s) ((Function *) lookup(s,fp))
#define lookup_var(s) ((Variable *) lookup(s,vp))

extern void *lookup(char *, Htab *);
extern Function *get_fn_place(char *);
extern List *varlookup(char *);
extern Node *fnlookup(char *);
extern Variable *get_var_place(char *, boolean);
extern boolean varassign_string(char *);
extern char **makeenv(void);
extern char *fnlookup_string(char *);
extern char *varlookup_string(char *);
extern void alias(char *, List *, boolean);
extern void starassign(char *, char **, boolean);
extern void delete_fn(char *);
extern void delete_var(char *, boolean);
extern void fnassign(char *, Node *);
extern void fnassign_string(char *);
extern void fnrm(char *);
extern void initenv(char **);
extern void inithash(void);
extern void setsigdefaults(void);
extern void inithandler(void);
extern void varassign(char *, List *, boolean);
extern void varrm(char *, boolean);
extern void whatare_all_vars(void);
extern void whatare_all_signals(void);
extern void prettyprint_var(int, char *, List *);
extern void prettyprint_fn(int, char *, Node *);
