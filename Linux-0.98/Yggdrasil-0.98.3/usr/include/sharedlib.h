#ifndef _SHAREDLIB_H
#define _SHAREDLIB_H

struct libentry
{
  char *name;
  char *avers;
  unsigned addr;
  unsigned vers;
};

#define QUOTED(x) #x

#define MAJOR_MASK		0x0fff0000
#define MINOR_MASK		0x0000ffff
#define VERSION_MASK		0x0fffffff
#define CLASSIC_BIT		0x10000000

#define data_set_element(set, symbol)   \
  asm(".stabs \"_" #set "\",24,0,0,_" QUOTED(symbol))

#endif
