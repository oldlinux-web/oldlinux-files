/*
	The st.c file is a sub-stub file.  I just wanted to have all the detect code, etc in the
	mid level driver present and working.  If no one else volunteers for this, I'll
	do it - but it's low on my list of priorities.
*/
#include <linux/config.h>

#ifdef CONFIG_BLK_DEV_ST
#include "scsi.h"
#include "st.h"

#define MAJOR_NR 9
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include "../blk.h" 

Scsi_Tape scsi_tapes[MAX_ST];
static int st_sizes[MAX_ST];
int NR_ST=0;

void do_st_request(void)
{
	panic("There is no st driver.\n\r");
}

unsigned long st_init(unsigned long memory_start, unsigned long memory_end)
{
	blk_dev[MAJOR_NR].request_fn = do_st_request;
	blk_size[MAJOR_NR] = st_sizes;
	return memory_start;
}
#endif	

