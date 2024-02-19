#define __LIBRARY__
#include <unistd.h>
#include <sys/socket.h>
#include "socketcall.h"

static inline
_syscall2(long,socketcall,int,call,unsigned long *,args);

_syscall0(int,vhangup);

int
socket(int family, int type, int protocol)
{
	unsigned long args[3];

	args[0] = family;
	args[1] = type;
	args[2] = protocol;
	return socketcall(SYS_SOCKET, args);
}

int
socketpair(int family, int type, int protocol, int sockvec[2])
{
	unsigned long args[4];

	args[0] = family;
	args[1] = type;
	args[2] = protocol;
	args[3] = (unsigned long)sockvec;
	return socketcall(SYS_SOCKETPAIR, args);
}


int
bind(int sockfd, struct sockaddr *myaddr, int addrlen)
{
	unsigned long args[3];

	args[0] = sockfd;
	args[1] = (unsigned long)myaddr;
	args[2] = addrlen;
	return socketcall(SYS_BIND, args);
}

int
connect(int sockfd, struct sockaddr *saddr, int addrlen)
{
	unsigned long args[3];

	args[0] = sockfd;
	args[1] = (unsigned long)saddr;
	args[2] = addrlen;
	return socketcall(SYS_CONNECT, args);
}

int
listen(int sockfd, int backlog)
{
	unsigned long args[2];

	args[0] = sockfd;
	args[1] = backlog;
	return socketcall(SYS_LISTEN, args);
}

int
accept(int sockfd, struct sockaddr *peer, int *paddrlen)
{
	unsigned long args[3];

	args[0] = sockfd;
	args[1] = (unsigned long)peer;
	args[2] = (unsigned long)paddrlen;
	return socketcall(SYS_ACCEPT, args);
}

int
getsockname(int sockfd, struct sockaddr *addr, int *paddrlen)
{
	unsigned long args[3];

	args[0] = sockfd;
	args[1] = (unsigned long)addr;
	args[2] = (unsigned long)paddrlen;
	return socketcall(SYS_GETSOCKNAME, args);
}

int
getpeername(int sockfd, struct sockaddr *peer, int *paddrlen)
{
	unsigned long args[3];

	args[0] = sockfd;
	args[1] = (unsigned long)peer;
	args[2] = (unsigned long)paddrlen;
	return socketcall(SYS_GETPEERNAME, args);
}

/* send, sendto added by bir7@leland.stanford.edu */

int
send (int sockfd, const void *buffer, int len, unsigned flags)
{
  unsigned long args[4];
  args[0] = sockfd;
  args[1] = (unsigned long) buffer;
  args[2] = len;
  args[3] = flags;
  return (socketcall (SYS_SEND, args));

}

int
sendto (int sockfd, const void *buffer, int len, unsigned flags,
	const struct sockaddr *to, int tolen)
{
  unsigned long args[6];
  args[0] = sockfd;
  args[1] = (unsigned long) buffer;
  args[2] = len;
  args[3] = flags;
  args[4] = (unsigned long) to;
  args[5] = tolen;
  return (socketcall (SYS_SENDTO, args));
}


/* recv, recvfrom added by bir7@leland.stanford.edu */

int
recv (int sockfd, void *buffer, int len, unsigned flags)
{
  unsigned long args[4];
  args[0] = sockfd;
  args[1] = (unsigned long) buffer;
  args[2] = len;
  args[3] = flags;
  return (socketcall (SYS_RECV, args));

}

int
recvfrom (int sockfd, void *buffer, int len, unsigned flags,
	struct sockaddr *to, int *tolen)
{
  unsigned long args[6];
  args[0] = sockfd;
  args[1] = (unsigned long) buffer;
  args[2] = len;
  args[3] = flags;
  args[4] = (unsigned long) to;
  args[5] = (unsigned long) tolen;
  return (socketcall (SYS_RECVFROM, args));
}

/* shutdown by bir7@leland.stanford.edu */
int
shutdown (int sockfd, int how)
{
  unsigned long args[2];
  args[0] = sockfd;
  args[1] = how;
  return (socketcall (SYS_SHUTDOWN, args));
}

/* [sg]etsockoptions by bir7@leland.stanford.edu */
int
setsockopt (int fd, int level, int optname, const void *optval,
	int optlen)
{
	unsigned long args[5];
	args[0]=fd;
	args[1]=level;
	args[2]=optname;
	args[3]=(unsigned long)optval;
	args[4]=optlen;
	return (socketcall (SYS_SETSOCKOPT, args));
}

int
getsockopt (int fd, int level, int optname, void *optval, int *optlen)
{
	unsigned long args[5];
	args[0]=fd;
	args[1]=level;
	args[2]=optname;
	args[3]=(unsigned long)optval;
	args[4]=(unsigned long)optlen;
	return (socketcall (SYS_SETSOCKOPT, args));
}
