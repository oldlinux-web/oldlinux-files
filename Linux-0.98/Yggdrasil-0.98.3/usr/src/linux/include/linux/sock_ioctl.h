/* ip.h */
/* Contains the structures for communicating with the ip level of the
   sockets.  Currently just for configuration. */
#ifndef _LINUX_SOCK_IOCTL_H
#define _LINUX_SOCK_IOCTL_H

#define MAX_IP_NAME 20
/* some ioctl.  Their values are not special. */
#define IP_SET_DEV 0x2401
#define IP_ADD_ROUTE 0x2402
#define IP_HANDOFF 0x2403

struct ip_config
{
   char name[MAX_IP_NAME];
   unsigned long paddr;
   unsigned long router;
   unsigned long net;
   unsigned long up:1;
};
#endif
