extern Node *newnode(int /*enum nodetype*/ t,...);
extern Node *treecpy(Node *s, void *(*)(SIZE_T));
extern void treefree(Node *s);
