extern int strncmp(const char *, const char *, SIZE_T);
extern int strcmp(const char *, const char *);
extern SIZE_T strlen(const char *);
extern char *strchr(const char *, int);
extern char *strrchr(const char *, int);
extern char *strcpy(char *, const char *);
extern char *strncpy(char *, const char *, SIZE_T);
extern char *strcat(char *, const char *);
extern char *strncat(char *, const char *, SIZE_T);
extern void *memcpy(void *, const void *, SIZE_T);

#define streq(x,y) (*(x) == *(y) && strcmp(x,y) == 0)
