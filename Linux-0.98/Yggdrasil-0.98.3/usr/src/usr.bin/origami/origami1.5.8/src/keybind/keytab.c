#define KEYTAB_C
/*{{{  #includes*/
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
 
#include <local/bool.h>
 
#include "../common/keys.h"
#include "keybind.h"
/*}}}  */
 
PRIVATE KEY *top_level=NULL;
 
/*{{{  key_alloc*/
#ifdef STD_C
PRIVATE KEY *key_alloc(void)
#else
PRIVATE KEY *key_alloc()
#endif
{
  KEY *x=malloc(sizeof(KEY));
 
  /*{{{  no memory?*/
  if (x==NULL) {
    printf(M_NOMEMORY);
    return(NULL);
  }
  /*}}}  */
  x->next=x->n_level=NULL;
  x->code=x->key=0;
  return(x);
}
/*}}}  */
 
/*{{{  add_key*/
#ifdef STD_C
PRIVATE KEY *add_key (int key,KEY *current)
#else
PRIVATE KEY *add_key (key,current)
int key;
KEY *current;
#endif
{
  KEY *k,*a;
 
  /*{{{  is key already used?*/
  for (k = current->n_level;k;k = k->next) {
    /*{{{  used => return it*/
    if (k->key == key) {
      if (k->code) {
        error_po();
        printf (F_ISMASKED,k->code);
        return(0);
      }
      return (k);
    }
    /*}}}  */
    if (k->next == 0) break;
  }
  /*}}}  */
  /*{{{  else generate new tree-entry*/
  /*{{{  get new tree-node*/
  if ((a=key_alloc())==NULL) return(NULL);
  a->key = key;
  /*}}}  */
  if (k == 0)
    current->n_level = a;
  else
    k->next = a;
  return (a);
  /*}}}  */
}
/*}}}  */
/*{{{  set_key_code*/
#ifdef STD_C
PRIVATE bool set_key_code (TOKEN code,KEY *current)
#else
PRIVATE bool set_key_code (code,current)
TOKEN code;
KEY *current;
#endif
{
  if (current->code) {
    error_po();
    printf(F_DUPLICATE,code,current->code);
    return (TRUE);
  }
  if (current->n_level) {
    error_po();
    printf (F_MASKS,code);
    return(TRUE);
  }
  current->code = code;
  return (FALSE);
}
/*}}}  */
 
/*{{{  init_keytab()*/
#ifdef STD_C
KEY *init_keytab(void)
#else
KEY *init_keytab()
#endif
{
  return(top_level=key_alloc());
}
/*}}}  */
/*{{{  add_keysequence*/
#ifdef STD_C
bool add_keysequence(TOKEN k, char *s, int lg)
#else
bool add_keysequence(k,s,lg) TOKEN k; char *s; int lg;
#endif
{
  KEY *key_point=top_level;
 
  k_counter++;
  while (lg--)
    if ((key_point = add_key (*s++,key_point)) == NULL)
      return(TRUE);
  if (set_key_code(k,key_point)) return(TRUE);
  return(FALSE);
}
/*}}}  */
 
/*{{{  count_keynodes*/
#ifdef STD_C
PRIVATE int count_keynodes(KEY *n)
#else
PRIVATE int count_keynodes(n) KEY *n;
#endif
{
  if (n==NULL) return(0);
  return(count_keynodes(n->next)+count_keynodes(n->n_level)+1);
}
/*}}}  */
 
 
/*{{{  write_single_keynode*/
#ifdef STD_C
PRIVATE bool write_single_keynode(KEY *n)
#else
PRIVATE bool write_single_keynode(n)
KEY *n;
#endif
{
  if (n==NULL) return(FALSE);
  putw(n->code,rc);
  /*{{{  abort-mapping?*/
  if (ab_set && n->key==abort_key) {
    printf(M_ABORTUSED);
    return(TRUE);
  }
  /*}}}  */
  putc(n->key,rc);
  if (n->next!=NULL) putw(1,rc); else putw(0,rc);
  if (n->n_level!=NULL) putw(count_keynodes(n->next)+1,rc); else putw(0,rc);
  if (write_single_keynode(n->next)) return(TRUE);
  return(write_single_keynode(n->n_level));
}
/*}}}  */
/*{{{  write_keynodes*/
#ifdef STD_C
bool write_keynodes(void)
#else
bool write_keynodes()
#endif
{
  putc(RC_DEFKEY,rc);
  putw(count_keynodes(top_level),rc);
  if (verbose) {
    fprintf(stderr,"------------\n");
    printf(F_NODES,count_keynodes(top_level));
  }
  return(write_single_keynode(top_level));
}
/*}}}  */
