struct Rq {
	Node *r;
	struct Rq *n;
};

extern void exec(List *, boolean);
extern void doredirs(void);
