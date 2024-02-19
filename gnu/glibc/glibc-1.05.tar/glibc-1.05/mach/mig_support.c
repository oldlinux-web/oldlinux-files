#include <mach/mach.h>

static mach_port_t reply_port;

/* Called at startup with NOP==0.  */
void
__mig_init (int nop)
{
  if (!nop)
    reply_port = MACH_PORT_NULL;
}

/* Called by MiG to get a reply port.  */
mach_port_t
__mig_get_reply_port (void)
{
  if (reply_port == MACH_PORT_NULL)
    reply_port = __mach_reply_port ();

  return reply_port;
}

/* Called by MiG to deallocate the reply port.  */
mach_port_t
__mig_dealloc_reply_port (void)
{
  mach_port_t port = reply_port;
  reply_port = MACH_PORT_NULL;	/* So the mod_refs RPC won't use it.  */
  __mach_port_mod_refs (__mach_task_self (), port,
			MACH_PORT_RIGHT_RECEIVE, -1);
}
