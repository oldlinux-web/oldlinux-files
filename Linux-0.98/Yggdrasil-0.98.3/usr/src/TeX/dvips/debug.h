/*
 * contains the debug stuff
 */

#ifdef DEBUG

#define dd(darg)	((darg)&debug_flag)

#define D_SPECIAL		(1<<0)
#define D_PATHS			(1<<1)
#define D_FONTS			(1<<2)
#define D_PAGE			(1<<3)

#endif /* DEBUG */
