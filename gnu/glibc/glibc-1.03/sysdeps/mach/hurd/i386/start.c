#define SET_SP(sp) \
  asm volatile ("movl %0, %%esp" : : "g" (sp) : "%esp")
#define GET_STACK(low, high)						      \
  ({									      \
    register vm_address_t ax asm ("%eax");				      \
    high = ~0U;								      \
    low = ax;								      \
  })
#define LOSE asm ("hlt")
#include <sysdeps/mach/hurd/start.c>
