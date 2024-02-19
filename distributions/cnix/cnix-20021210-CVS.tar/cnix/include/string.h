#ifndef STRING_H
#define STRING_H

extern inline void memcpy(void * dest,const void * src, int n)
{
__asm__("cld\n\t"
	"rep\n\t"
	"movsb"
	::"c" (n),"S" (src),"D" (dest)
	);
}

extern inline int strlen(const char * s)
{
register int __res;
__asm__("cld\n\t"
	"repne\n\t"
	"scasb\n\t"
	"notl %0\n\t"
	"decl %0"
	:"=c" (__res):"D" (s),"a" (0),"0" (0xffffffff));
return __res;
}
extern inline
int strcmp(const char * s1, const char * s2)
{
	int i, ret;
	
	ret = 0;
	for(i = 0; s1[i] != '\0' || s2[i] != '\0'; i++){
		if(s1[i] != s2[i]){
			ret = s1[i] > s2[i] ? 1 : -1;
			break;
		}
	}
	

	return ret;
}
extern inline
 int strncmp(const char * cs,const char * ct,int count)
{
        int i,ret;
	
	for(i=0;i<count;i++){
	      if(cs[i]!=ct[i]){
	          ret = cs[i] > ct[i] ? 1 : -1;
	          return ret;
	      }
        }

        return 0;
}
extern inline
char * stpcpy(char *dest,const char *src)
{
       int j;

       for(j=0;src[j]!='\0';j++){
	   dest[j]=src[j];
       }
       dest[j+1] ='\0';

       return dest+j;
}
extern inline
char * strcpy(char *dest,const char *src)
{
       int j;

       for(j=0;src[j]!='\0';j++){
	   dest[j]=src[j];
       }
       dest[j+1] ='\0';

       return dest+j;
}

extern inline
char *strcat(char *dest,const char *src)
{
       int i,j;

       for(i=0;dest[i]!='\0';i++);

       for(j=0;src[j]!='\0';j++){
	   dest[i]=src[j];
	   i++;
      }
       dest[i+1] ='\0';

       return dest;
}


extern inline
char *strncat(char *dest,const char *src,int maxlen)
{
       int i,j;

       for(i=0;dest[i]!='\0';i++);

       for(j=0;j<maxlen;j++){
	   dest[i]=src[j];
	   i++;
      }
       dest[i+1] ='\0';

       return dest;
}

extern inline
char *strchr(const char *s,int c)
{
      int i;

      for(i=0;s[i]!='\0';i++){
	if (s[i]==c)
	      return &s[i];
      }
      return '\0';
}

extern inline
char *strrchr(const char *s,int c)
{
      int i,len;
      
      len = strlen(s);

      for(i=len;s[i]!='\0';i--){
	if (s[i]==c)
	      return &s[i];
      }
      return '\0';
}

extern inline
char *strstr(const char *s1,const char *s2)
{
      int i,j;

      i =0;

      for(j=0;s1[j]!='\0';j++){
	      if(s1[j]==s2[i]){
		  if(strcmp(&s1[j],s2)==0)
		    return &s1[j];
	      }
      }
      
      return '\0';
}


#endif
