#include <cnix/config.h>
/*cnix malloc memery form 2M to 4M*/

static char * start_mem ;
static char * end_mem ;

static char * current_mem;
static char * next_mem;


char *
cnix_malloc(int size)
{
      current_mem = next_mem;
      if( current_mem + size >end_mem )
	    return -1;
      
      next_mem = current_mem +size;
      return current_mem;
}
cnix_malloc_init()
{
      start_mem = 0x3E00000;
      end_mem =   start_mem+0x200000;
      next_mem = start_mem;
}
