#ifndef NO_JUMP_TABLE
#include <termcap.h>

speed_t ospeed = 0;
/* If OSPEED is 0, we use this as the actual baud rate.  */
int tputs_baud_rate = 0;
char PC;

char *BC;
char *UP;
#endif
