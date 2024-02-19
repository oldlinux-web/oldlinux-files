#include "kernel.h"
#include "minix/com.h"
#include "proc.h"
#include "dp8390.h"
#include "assert.h"
#include "dp8390info.h"

#include "eplinfo.h"


struct eplinfo eplinfo = {0x280};

struct dp8390info dp8390info = {0x290, 6, 27, EPLUS_BASE, EPLUS_BASE};

extern vir_bytes eplus_seg;


#if !NONET
getheader(paddr, pkthead)
    phys_bytes paddr;
    struct rcvdheader *pkthead;
{
  vir_bytes  offset;
  char get_byte();

  offset = (paddr - EPLUS_BASE)&0xFFFF;
  pkthead->rp_status = get_byte(eplus_seg, offset);
  pkthead->rp_next = get_byte(eplus_seg, offset+1);
  pkthead->rp_rbcl = get_byte(eplus_seg, offset+2);
  pkthead->rp_rbch = get_byte(eplus_seg, offset+3);
}


short
getbint(paddr)
    phys_bytes paddr;
{
  vir_bytes offset;
  short t;
    
  offset = (paddr - EPLUS_BASE)&0xFFFF;
  return (((short)get_byte(eplus_seg, offset) & 0xFF)<<8) + 
	((short)get_byte(eplus_seg, offset+1) & 0xFF);
}
#endif





