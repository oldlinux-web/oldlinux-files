extern void assign(List *, List *, boolean);
extern void qredir(Node *);
extern List *append(List *, List*);
extern List *flatten(List *);
extern List *glom(Node *);
extern List *concat(List *, List *);
extern List *varsub(List *, List *);
extern List *word(char *, char *);

struct Fq {
	int pid;
	char *name;
	Fq *n;
};
