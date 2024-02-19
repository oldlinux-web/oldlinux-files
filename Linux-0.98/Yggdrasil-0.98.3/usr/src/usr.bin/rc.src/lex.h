enum redirtype {
	FROM, CREATE, APPEND, HEREDOC, HERESTRING
};

typedef struct Pipe {
	int left,right;
} Pipe;

typedef struct Dup {
	enum redirtype type;
	int left,right;
} Dup;

typedef struct Redir {
	enum redirtype type;
	int fd;
} Redir;

typedef struct Word {
	char *w;
	char *m;
} Word;

extern int yylex(void);
extern void inityy(void);
extern void yyerror(const char *);
extern void scanerror(char *);
extern void print_prompt2(void);

extern const char nw[], dnw[];
