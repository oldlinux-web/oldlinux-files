enum nodetype {
	rANDAND, ASSIGN, BACKQ, rBANG, BODY, CBODY, NOWAIT, BRACE, CONCAT,
	rCOUNT, rELSE, rFLAT, rDUP, EPILOG, NEWFN, FORIN, rIF, QWORD,
	rOROR, rPIPE, PRE, rREDIR, RMFN, ARGS, rSUBSHELL, rCASE,
	rSWITCH, MATCH, VAR, VARSUB, rWHILE, rWORD, LAPPEND, NMPIPE
};

typedef struct Node Node;

struct Node {
	enum nodetype type;
	union { char *s; int i; Node *p; } u[4];
};
