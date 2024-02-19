/* Define the machine-dependent type `jmp_buf'.  Intel 386 version.  */

typedef struct
  {
    long int __bx, __si, __di;
#ifdef SAVE_387_CONTROL_WORD
    short int __cw;
#endif
    void * __bp;
    void * __sp;
    void * __pc;
  } __jmp_buf[1];
