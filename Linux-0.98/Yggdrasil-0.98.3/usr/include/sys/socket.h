#ifndef _SYS_SOCKET_H
#define _SYS_SOCKET_H

#include <traditional.h>
#include <linux/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

int socket _ARGS ((int __family, int __type, int __protocol));
int socketpair _ARGS ((int __family, int __type, int __protocol,
		int __sockvec[2]));
int bind _ARGS ((int __sockfd, struct sockaddr *__my_addr,
		int __addrlen));
int connect _ARGS ((int __sockfd, struct sockaddr *__serv_addr,
		int __addrlen));
int listen _ARGS ((int __sockfd, int __backlog));
int accept _ARGS ((int __sockfd, struct sockaddr *__peer,
		int *__paddrlen));
int getsockopt _ARGS ((int __s, int __level, int __optname,
		void *__optval, int *__optlen));
int setsockopt _ARGS ((int __s, int __level, int __optname,
		const void *__optval, int optlen));
int getsockname _ARGS ((int __sockfd, struct sockaddr *__addr,
		int *__paddrlen));
int getpeername _ARGS ((int __sockfd, struct sockaddr *__peer,
		int *__paddrlen));
int send _ARGS ((int __sockfd, const void *__buff, int __len,
		 unsigned int __flags));
int recv _ARGS ((int __sockfd, void *__buff, int __len,
		 unsigned int __flags));
int sendto _ARGS ((int __sockfd, const void *__buff, int __len,
		 unsigned int __flags, const struct sockaddr *__to,
		 int __tolen));
int recvfrom _ARGS ((int __sockfd, void *__buff, int __len,
		 unsigned int __flags, struct sockaddr *__from,
		 int *__fromlen));

int rcmd _ARGS ((char **__ahost, unsigned short __inport,
		const char *__locuser, const char *__remuser,
		const char *__cmd, int *__fd2p));
int rresvport _ARGS ((int *__port));
int ruserok _ARGS ((const char *__rhost, int __superuser,
		const char *__ruser, const char *__luser));
int rexec _ARGS ((char **__ahost, int __inport, const char *__user,
		 const char *__passwd, const char *__cmd,
		 int *__fd2p));

int shutdown _ARGS ((int __s, int __how));
int vhangup _ARGS ((void));

#ifdef __cplusplus
}
#endif

#endif /* _SYS_SOCKET_H */
