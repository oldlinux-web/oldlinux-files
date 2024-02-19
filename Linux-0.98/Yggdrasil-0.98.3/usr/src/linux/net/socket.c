/* modified by Ross Biro to help support inet sockets. */
#include <linux/signal.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/stat.h>
#include <linux/socket.h>
#include <linux/fcntl.h>
#include <linux/termios.h>

#include <asm/system.h>
#include <asm/segment.h>

#include "kern_sock.h"
#include "socketcall.h"

extern int sys_close(int fd);

extern struct proto_ops unix_proto_ops;
#ifdef INET_SOCKETS
extern struct proto_ops inet_proto_ops;
#endif

static struct {
	short family;
	char *name;
	struct proto_ops *ops;
} proto_table[] = {
	{AF_UNIX,	"AF_UNIX",	&unix_proto_ops},
#ifdef INET_SOCKETS
	{AF_INET,	"AF_INET",	&inet_proto_ops},
#endif
};
#define NPROTO (sizeof(proto_table) / sizeof(proto_table[0]))

static char *
family_name(int family)
{
	int i;

	for (i = 0; i < NPROTO; ++i)
		if (proto_table[i].family == family)
			return proto_table[i].name;
	return "UNKNOWN";
}

static int sock_lseek(struct inode *inode, struct file *file, off_t offset,
		      int whence);
static int sock_read(struct inode *inode, struct file *file, char *buf,
		     int size);
static int sock_write(struct inode *inode, struct file *file, char *buf,
		      int size);
static int sock_readdir(struct inode *inode, struct file *file,
			struct dirent *dirent, int count);
static void sock_close(struct inode *inode, struct file *file);
static int sock_select(struct inode *inode, struct file *file, int which, select_table *seltable);
static int sock_ioctl(struct inode *inode, struct file *file,
		      unsigned int cmd, unsigned int arg);

static struct file_operations socket_file_ops = {
	sock_lseek,
	sock_read,
	sock_write,
	sock_readdir,
	sock_select,
	sock_ioctl,
	NULL,		/* mmap */
	NULL,		/* no special open code... */
	sock_close
};

#define SOCK_INODE(S) ((struct inode *)(S)->dummy)

static struct socket sockets[NSOCKETS];
#define last_socket (sockets + NSOCKETS - 1)
static struct wait_queue *socket_wait_free = NULL;

/*
 * obtains the first available file descriptor and sets it up for use
 */
static int
get_fd(struct inode *inode)
{
	int fd, i;
	struct file *file;

	/*
	 * find a file descriptor suitable for return to the user.
	 */
	for (fd = 0; fd < NR_OPEN; ++fd)
		if (!current->filp[fd])
			break;
	if (fd == NR_OPEN)
		return -1;
	current->close_on_exec &= ~(1 << fd);
	for (file = file_table, i = 0; i < NR_FILE; ++i, ++file)
		if (!file->f_count)
			break;
	if (i == NR_FILE)
		return -1;
	current->filp[fd] = file;
	file->f_op = &socket_file_ops;
	file->f_mode = 3;
	file->f_flags = 0;
	file->f_count = 1;
	file->f_inode = inode;
	file->f_pos = 0;
	return fd;
}

/*
 * reverses the action of get_fd() by releasing the file. it closes the
 * descriptor, but makes sure it does nothing more. called when an incomplete
 * socket must be closed, along with sock_release().
 */
static inline void
toss_fd(int fd)
{
	current->filp[fd]->f_inode = NULL;	/* safe from iput */
	sys_close(fd);
}

static inline struct socket *
socki_lookup(struct inode *inode)
{
	struct socket *sock;

	for (sock = sockets; sock <= last_socket; ++sock)
		if (sock->state != SS_FREE && SOCK_INODE(sock) == inode)
			return sock;
	return NULL;
}

static inline struct socket *
sockfd_lookup(int fd, struct file **pfile)
{
	struct file *file;

	if (fd < 0 || fd >= NR_OPEN || !(file = current->filp[fd]))
		return NULL;
	if (pfile)
		*pfile = file;
	return socki_lookup(file->f_inode);
}

static struct socket *
sock_alloc(int wait)
{
	struct socket *sock;

	while (1) {
		cli();
		for (sock = sockets; sock <= last_socket; ++sock)
			if (sock->state == SS_FREE) {
				sock->state = SS_UNCONNECTED;
				sti();
				sock->flags = 0;
				sock->ops = NULL;
				sock->data = NULL;
				sock->conn = NULL;
				sock->iconn = NULL;
				/*
				 * this really shouldn't be necessary, but
				 * everything else depends on inodes, so we
				 * grab it.
				 * sleeps are also done on the i_wait member
				 * of this inode.
				 * the close system call will iput this inode
				 * for us.
				 */
				if (!(SOCK_INODE(sock) = get_empty_inode())) {
					printk("sock_alloc: no more inodes\n");
					sock->state = SS_FREE;
					return NULL;
				}
				SOCK_INODE(sock)->i_mode = S_IFSOCK;
				sock->wait = &SOCK_INODE(sock)->i_wait;
				PRINTK("sock_alloc: socket 0x%x, inode 0x%x\n",
				       sock, SOCK_INODE(sock));
				return sock;
			}
		sti();
		if (!wait)
			return NULL;
		PRINTK("sock_alloc: no free sockets, sleeping...\n");
		interruptible_sleep_on(&socket_wait_free);
		if (current->signal & ~current->blocked) {
			PRINTK("sock_alloc: sleep was interrupted\n");
			return NULL;
		}
		PRINTK("sock_alloc: wakeup... trying again...\n");
	}
}

static inline void
sock_release_peer(struct socket *peer)
{
	peer->state = SS_DISCONNECTING;
	wake_up(peer->wait);
}

static void
sock_release(struct socket *sock)
{
	int oldstate;
	struct socket *peersock, *nextsock;

	PRINTK("sock_release: socket 0x%x, inode 0x%x\n", sock,
	       SOCK_INODE(sock));
	if ((oldstate = sock->state) != SS_UNCONNECTED)
		sock->state = SS_DISCONNECTING;
	/*
	 * wake up anyone waiting for connections
	 */
	for (peersock = sock->iconn; peersock; peersock = nextsock) {
		nextsock = peersock->next;
		sock_release_peer(peersock);
	}
	/*
	 * wake up anyone we're connected to. first, we release the
	 * protocol, to give it a chance to flush data, etc.
	 */
	peersock = (oldstate == SS_CONNECTED) ? sock->conn : NULL;
	if (sock->ops)
		sock->ops->release(sock, peersock);
	if (peersock)
		sock_release_peer(peersock);
	sock->state = SS_FREE;		/* this really releases us */
	wake_up(&socket_wait_free);
}

static int
sock_lseek(struct inode *inode, struct file *file, off_t offset, int whence)
{
	PRINTK("sock_lseek: huh?\n");
	return -EBADF;
}

static int
sock_read(struct inode *inode, struct file *file, char *ubuf, int size)
{
	struct socket *sock;

	PRINTK("sock_read: buf=0x%x, size=%d\n", ubuf, size);
	if (!(sock = socki_lookup(inode))) {
		printk("sock_read: can't find socket for inode!\n");
		return -EBADF;
	}
	if (sock->flags & SO_ACCEPTCON)
		return -EINVAL;
	return sock->ops->read(sock, ubuf, size, (file->f_flags & O_NONBLOCK));
}

static int
sock_write(struct inode *inode, struct file *file, char *ubuf, int size)
{
	struct socket *sock;

	PRINTK("sock_write: buf=0x%x, size=%d\n", ubuf, size);
	if (!(sock = socki_lookup(inode))) {
		printk("sock_write: can't find socket for inode!\n");
		return -EBADF;
	}
	if (sock->flags & SO_ACCEPTCON)
		return -EINVAL;
	return sock->ops->write(sock, ubuf, size,(file->f_flags & O_NONBLOCK));
}

static int
sock_readdir(struct inode *inode, struct file *file, struct dirent *dirent,
	     int count)
{
	PRINTK("sock_readdir: huh?\n");
	return -EBADF;
}

int
sock_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
	   unsigned int arg)
{
	struct socket *sock;

	PRINTK("sock_ioctl: inode=0x%x cmd=0x%x arg=%d\n", inode, cmd, arg);
	if (!(sock = socki_lookup(inode))) {
		printk("sock_ioctl: can't find socket for inode!\n");
		return -EBADF;
	}
	return sock->ops->ioctl(sock, cmd, arg);
}

static int
sock_select(struct inode *inode, struct file *file, int sel_type, select_table * wait)
{
	struct socket *sock;

	PRINTK("sock_select: inode = 0x%x, kind = %s\n", inode,
	       (sel_type == SEL_IN) ? "in" :
	       (sel_type == SEL_OUT) ? "out" : "ex");
	if (!(sock = socki_lookup(inode))) {
		printk("sock_select: can't find socket for inode!\n");
		return 0;
	}
	/*
	 * we can't return errors to select, so its either yes or no.
	 */
	if (sock->ops && sock->ops->select)
		return sock->ops->select(sock, sel_type, wait);
	return 0;
}

void
sock_close(struct inode *inode, struct file *file)
{
	struct socket *sock;

	PRINTK("sock_close: inode=0x%x (cnt=%d)\n", inode, inode->i_count);
	/*
	 * it's possible the inode is NULL if we're closing an unfinished
	 * socket.
	 */
	if (!inode)
		return;
	if (!(sock = socki_lookup(inode))) {
		printk("sock_close: can't find socket for inode!\n");
		return;
	}
	sock_release(sock);
}

int
sock_awaitconn(struct socket *mysock, struct socket *servsock)
{
	struct socket *last;

	PRINTK("sock_awaitconn: trying to connect socket 0x%x to 0x%x\n",
	       mysock, servsock);
	if (!(servsock->flags & SO_ACCEPTCON)) {
		PRINTK("sock_awaitconn: server not accepting connections\n");
		return -EINVAL;
	}

	/*
	 * put ourselves on the server's incomplete connection queue.
	 */
	mysock->next = NULL;
	cli();
	if (!(last = servsock->iconn))
		servsock->iconn = mysock;
	else {
		while (last->next)
			last = last->next;
		last->next = mysock;
	}
	mysock->state = SS_CONNECTING;
	mysock->conn = servsock;
	sti();

	/*
	 * wake up server, then await connection. server will set state to
	 * SS_CONNECTED if we're connected.
	 */
	wake_up(servsock->wait);
	if (mysock->state != SS_CONNECTED) {
		interruptible_sleep_on(mysock->wait);
		if (mysock->state != SS_CONNECTED) {
			/*
			 * if we're not connected we could have been
			 * 1) interrupted, so we need to remove ourselves
			 *    from the server list
			 * 2) rejected (mysock->conn == NULL), and have
			 *    already been removed from the list
			 */
			if (mysock->conn == servsock) {
				cli();
				if ((last = servsock->iconn) == mysock)
					servsock->iconn = mysock->next;
				else {
					while (last->next != mysock)
						last = last->next;
					last->next = mysock->next;
				}
				sti();
			}
			return mysock->conn ? -EINTR : -EACCES;
		}
	}
	return 0;
}

/*
 * perform the socket system call. we locate the appropriate family, then
 * create a fresh socket.
 */
static int
sock_socket(int family, int type, int protocol)
{
	int i, fd;
	struct socket *sock;
	struct proto_ops *ops;

	PRINTK("sys_socket: family = %d (%s), type = %d, protocol = %d\n",
	       family, family_name(family), type, protocol);

	/*
	 * locate the correct protocol family
	 */
	for (i = 0; i < NPROTO; ++i)
		if (proto_table[i].family == family)
			break;
	if (i == NPROTO) {
		PRINTK("sys_socket: family not found\n");
		return -EINVAL;
	}
	ops = proto_table[i].ops;

	/*
	 * check that this is a type that we know how to manipulate and
	 * the protocol makes sense here. the family can still reject the
	 * protocol later.
	 */
	if ((type != SOCK_STREAM &&
	     type != SOCK_DGRAM &&
	     type != SOCK_SEQPACKET &&
	     type != SOCK_RAW) ||
	    protocol < 0)
		return -EINVAL;

	/*
	 * allocate the socket and allow the family to set things up. if
	 * the protocol is 0, the family is instructed to select an appropriate
	 * default.
	 */
	if (!(sock = sock_alloc(1))) {
		printk("sys_socket: no more sockets\n");
		return -EAGAIN;
	}
	sock->type = type;
	sock->ops = ops;
	if ((i = sock->ops->create(sock, protocol)) < 0) {
		sock_release(sock);
		return i;
	}

	if ((fd = get_fd(SOCK_INODE(sock))) < 0) {
		sock_release(sock);
		return -EINVAL;
	}

	return fd;
}

static int
sock_socketpair(int family, int type, int protocol, int usockvec[2])
{
	int fd1, fd2, i;
	struct socket *sock1, *sock2;

	PRINTK("sys_socketpair: family = %d, type = %d, protocol = %d\n",
	       family, type, protocol);

	/*
	 * obtain the first socket and check if the underlying protocol
	 * supports the socketpair call
	 */
	if ((fd1 = sock_socket(family, type, protocol)) < 0)
		return fd1;
	sock1 = sockfd_lookup(fd1, NULL);
	if (!sock1->ops->socketpair) {
		sys_close(fd1);
		return -EINVAL;
	}

	/*
	 * now grab another socket and try to connect the two together
	 */
	if ((fd2 = sock_socket(family, type, protocol)) < 0) {
		sys_close(fd1);
		return -EINVAL;
	}
	sock2 = sockfd_lookup(fd2, NULL);
	if ((i = sock1->ops->socketpair(sock1, sock2)) < 0) {
		sys_close(fd1);
		sys_close(fd2);
		return i;
	}
	sock1->conn = sock2;
	sock2->conn = sock1;
	sock1->state = SS_CONNECTED;
	sock2->state = SS_CONNECTED;

	verify_area(usockvec, 2 * sizeof(int));
	put_fs_long(fd1, &usockvec[0]);
	put_fs_long(fd2, &usockvec[1]);

	return 0;
}

/*
 * binds a name to a socket. nothing much to do here since its the
 * protocol's responsibility to handle the local address
 */
static int
sock_bind(int fd, struct sockaddr *umyaddr, int addrlen)
{
	struct socket *sock;
	int i;

	PRINTK("sys_bind: fd = %d\n", fd);
	if (!(sock = sockfd_lookup(fd, NULL)))
		return -EBADF;
	if ((i = sock->ops->bind(sock, umyaddr, addrlen)) < 0) {
		PRINTK("sys_bind: bind failed\n");
		return i;
	}
	return 0;
}

/*
 * perform a listen. basically, we allow the protocol to do anything
 * necessary for a listen, and if that works, we mark the socket as
 * ready for listening.
 */
static int
sock_listen(int fd, int backlog)
{
	struct socket *sock;

	PRINTK("sys_listen: fd = %d\n", fd);
	if (!(sock = sockfd_lookup(fd, NULL)))
		return -EBADF;
	if (sock->state != SS_UNCONNECTED) {
		PRINTK("sys_listen: socket isn't unconnected\n");
		return -EINVAL;
	}
	if (sock->flags & SO_ACCEPTCON) {
		PRINTK("sys_listen: socket already accepting connections!\n");
		return -EINVAL;
	}
	if (sock->ops && sock->ops->listen)
	  sock->ops->listen (sock, backlog);
	sock->flags |= SO_ACCEPTCON;
	return 0;
}

/*
 * for accept, we attempt to create a new socket, set up the link with the
 * client, wake up the client, then return the new connected fd.
 */
static int
sock_accept(int fd, struct sockaddr *upeer_sockaddr, int *upeer_addrlen)
{
	struct file *file;
	struct socket *sock, *newsock;
	int i;

	PRINTK("sys_accept: fd = %d\n", fd);
	if (!(sock = sockfd_lookup(fd, &file)))
		return -EBADF;
	if (sock->state != SS_UNCONNECTED) {
		PRINTK("sys_accept: socket isn't unconnected\n");
		return -EINVAL;
	}
	if (!(sock->flags & SO_ACCEPTCON)) {
		PRINTK("sys_accept: socket not accepting connections!\n");
		return -EINVAL;
	}

	if (!(newsock = sock_alloc(0))) {
		printk("sys_accept: no more sockets\n");
		return -EAGAIN;
	}
	newsock->type = sock->type;
	newsock->ops = sock->ops;
	if ((i = sock->ops->dup(newsock, sock)) < 0) {
		sock_release(newsock);
		return i;
	}

	if ((fd = get_fd(SOCK_INODE(newsock))) < 0) {
		sock_release(newsock);
		return -EINVAL;
	}
	i = newsock->ops->accept(sock, newsock, file->f_flags);

	if ( i < 0)
	  {
	     sys_close (fd);
	     return (i);
	  }

	PRINTK("sys_accept: connected socket 0x%x via 0x%x\n",
	       sock, newsock);

	if (upeer_sockaddr)
		newsock->ops->getname(newsock, upeer_sockaddr,
				      upeer_addrlen, 1);

	return fd;
}

/*
 * attempt to connect to a socket with the server address.
 */
static int
sock_connect(int fd, struct sockaddr *uservaddr, int addrlen)
{
	struct socket *sock;
	struct file *file;
	int i;

	PRINTK("sys_connect: fd = %d\n", fd);
	if (!(sock = sockfd_lookup(fd, &file)))
		return -EBADF;
	if (sock->state != SS_UNCONNECTED) {
		PRINTK("sys_connect: socket not unconnected\n");
		return -EINVAL;
	}
	i = sock->ops->connect(sock, uservaddr, addrlen, file->f_flags);
	if (i < 0) {
		PRINTK("sys_connect: connect failed\n");
		return i;
	}
	return 0;
}

static int
sock_getsockname(int fd, struct sockaddr *usockaddr, int *usockaddr_len)
{
	struct socket *sock;

	PRINTK("sys_getsockname: fd = %d\n", fd);
	if (!(sock = sockfd_lookup(fd, NULL)))
		return -EBADF;
	return sock->ops->getname(sock, usockaddr, usockaddr_len, 0);
}

static int
sock_getpeername(int fd, struct sockaddr *usockaddr, int *usockaddr_len)
{
	struct socket *sock;

	PRINTK("sys_getpeername: fd = %d\n", fd);
	if (!(sock = sockfd_lookup(fd, NULL)))
		return -EBADF;
	return sock->ops->getname(sock, usockaddr, usockaddr_len, 1);
}


/* send - shutdown added by bir7@leland.stanford.edu */

static int
sys_send( int fd, void * buff, int len, unsigned flags)
{
	struct socket *sock;
	struct file *file;

	PRINTK("sys_send (fd = %d, buff = %X, len = %d, flags = %X)\n",
	       fd, buff, len, flags);

	if (fd < 0 || fd >= NR_OPEN || 	((file = current->filp[fd]) == NULL))
	  return (-EBADF);

	if (!(sock = sockfd_lookup(fd, NULL)))
		return (-ENOTSOCK);

	return (sock->ops->send (sock, buff, len, (file->f_flags & O_NONBLOCK),
				 flags));

}

static int
sys_sendto( int fd, void * buff, int len, unsigned flags,
	   struct sockaddr *addr, int addr_len)
{
	struct socket *sock;
	struct file *file;

	PRINTK("sys_sendto (fd = %d, buff = %X, len = %d, flags = %X,"
	       " addr=%X, alen = %d\n", fd, buff, len, flags, addr, addr_len);

	if (fd < 0 || fd >= NR_OPEN || 	((file = current->filp[fd]) == NULL))
	  return (-EBADF);

	if (!(sock = sockfd_lookup(fd, NULL)))
		return (-ENOTSOCK);

	return (sock->ops->sendto (sock, buff, len,
				   (file->f_flags & O_NONBLOCK),
				   flags, addr, addr_len));

}


static int
sys_recv( int fd, void * buff, int len, unsigned flags)
{
	struct socket *sock;
	struct file *file;

	PRINTK("sys_recv (fd = %d, buff = %X, len = %d, flags = %X)\n",
	       fd, buff, len, flags);

	if (fd < 0 || fd >= NR_OPEN || 	((file = current->filp[fd]) == NULL))
	  return (-EBADF);

	if (!(sock = sockfd_lookup(fd, NULL)))
		return (-ENOTSOCK);

	return (sock->ops->recv (sock, buff, len,(file->f_flags & O_NONBLOCK),
				 flags));

}

static int
sys_recvfrom( int fd, void * buff, int len, unsigned flags,
	     struct sockaddr *addr, int *addr_len)
{
	struct socket *sock;
	struct file *file;

	PRINTK("sys_recvfrom (fd = %d, buff = %X, len = %d, flags = %X,"
	       " addr=%X, alen=%X\n", fd, buff, len, flags, addr, addr_len);

	if (fd < 0 || fd >= NR_OPEN || 	((file = current->filp[fd]) == NULL))
	  return (-EBADF);

	if (!(sock = sockfd_lookup(fd, NULL)))
		return (-ENOTSOCK);

	return (sock->ops->recvfrom (sock, buff, len,
				     (file->f_flags & O_NONBLOCK),
				     flags, addr, addr_len));

}


static int
sys_setsockopt (int fd, int level, int optname, char *optval, int optlen)
{
	struct socket *sock;
	struct file *file;
	
	PRINTK ("sys_setsockopt(fd=%d, level=%d, optname=%d,\n",fd, level,
		optname);
	PRINTK ("               optval = %X, optlen = %d)\n", optval, optlen);

	if (fd < 0 || fd >= NR_OPEN || 	((file = current->filp[fd]) == NULL))
	  return (-EBADF);

	if (!(sock = sockfd_lookup(fd, NULL)))
		return (-ENOTSOCK);

	return (sock->ops->setsockopt (sock, level, optname, optval, optlen));

}

static int
sys_getsockopt (int fd, int level, int optname, char *optval, int *optlen)
{
	struct socket *sock;
	struct file *file;
	PRINTK ("sys_getsockopt(fd=%d, level=%d, optname=%d,\n",fd, level,
		optname);
	PRINTK ("               optval = %X, optlen = %X)\n", optval, optlen);

	if (fd < 0 || fd >= NR_OPEN || 	((file = current->filp[fd]) == NULL))
	  return (-EBADF);

	if (!(sock = sockfd_lookup(fd, NULL)))
	    return (-ENOTSOCK);
	    
	return (0);
	return (sock->ops->getsockopt (sock, level, optname, optval, optlen));

}


static int
sys_shutdown( int fd, int how)
{
	struct socket *sock;
	struct file *file;

	PRINTK("sys_shutdown (fd = %d, how = %d)\n",fd, how);

	file = current->filp[fd];
	if (fd < 0 || fd >= NR_OPEN || file == NULL)
	  return (-EBADF);

	if (!(sock = sockfd_lookup(fd, NULL)))
		return (-ENOTSOCK);

	return (sock->ops->shutdown (sock, how));

}

int
sock_fcntl(struct file *filp, unsigned int cmd, unsigned long arg)
{
   struct socket *sock;
   sock = socki_lookup (filp->f_inode);
   
   if (sock != NULL && sock->ops != NULL && sock->ops->fcntl != NULL)
     return (sock->ops->fcntl (sock, cmd, arg));

   return (-EINVAL);
}


/*
 * system call vectors. since i want to rewrite sockets as streams, we have
 * this level of indirection. not a lot of overhead, since more of the work is
 * done via read/write/select directly
 */
int
sys_socketcall(int call, unsigned long *args)
{
	switch (call) {
	case SYS_SOCKET:
		verify_area(args, 3 * sizeof(long));
		return sock_socket(get_fs_long(args+0),
				   get_fs_long(args+1),
				   get_fs_long(args+2));

	case SYS_BIND:
		verify_area(args, 3 * sizeof(long));
		return sock_bind(get_fs_long(args+0),
				 (struct sockaddr *)get_fs_long(args+1),
				 get_fs_long(args+2));

	case SYS_CONNECT:
		verify_area(args, 3 * sizeof(long));
		return sock_connect(get_fs_long(args+0),
				    (struct sockaddr *)get_fs_long(args+1),
				    get_fs_long(args+2));

	case SYS_LISTEN:
		verify_area(args, 2 * sizeof(long));
		return sock_listen(get_fs_long(args+0),
				   get_fs_long(args+1));

	case SYS_ACCEPT:
		verify_area(args, 3 * sizeof(long));
		return sock_accept(get_fs_long(args+0),
				   (struct sockaddr *)get_fs_long(args+1),
				   (int *)get_fs_long(args+2));

	case SYS_GETSOCKNAME:
		verify_area(args, 3 * sizeof(long));
		return sock_getsockname(get_fs_long(args+0),
					(struct sockaddr *)get_fs_long(args+1),
					(int *)get_fs_long(args+2));

	case SYS_GETPEERNAME:
		verify_area(args, 3 * sizeof(long));
		return sock_getpeername(get_fs_long(args+0),
					(struct sockaddr *)get_fs_long(args+1),
					(int *)get_fs_long(args+2));

	case SYS_SOCKETPAIR:
		verify_area(args, 4 * sizeof(long));
		return sock_socketpair(get_fs_long(args+0),
				       get_fs_long(args+1),
				       get_fs_long(args+2),
				       (int *)get_fs_long(args+3));

      case SYS_SEND:
	  verify_area(args, 4 * sizeof (unsigned long));
	  return ( sys_send (get_fs_long(args+0),
			     (void *)get_fs_long(args+1),
			     get_fs_long(args+2),
			     get_fs_long(args+3)));
			     
      case SYS_SENDTO:
	  verify_area(args, 6 * sizeof (unsigned long));
	  return ( sys_sendto (get_fs_long(args+0),
			     (void *)get_fs_long(args+1),
			     get_fs_long(args+2),
			     get_fs_long(args+3),
			     (struct sockaddr *)get_fs_long(args+4),
			     get_fs_long(args+5)));

    
      case SYS_RECV:
	  verify_area(args, 4 * sizeof (unsigned long));
	  return ( sys_recv (get_fs_long(args+0),
			     (void *)get_fs_long(args+1),
			     get_fs_long(args+2),
			     get_fs_long(args+3)));
			     
      case SYS_RECVFROM:
	  verify_area(args, 6 * sizeof (unsigned long));
	  return ( sys_recvfrom (get_fs_long(args+0),
				 (void *)get_fs_long(args+1),
				 get_fs_long(args+2),
				 get_fs_long(args+3),
				 (struct sockaddr *)get_fs_long(args+4),
				 (int *)get_fs_long(args+5)));

      case SYS_SHUTDOWN:
	  verify_area (args, 2* sizeof (unsigned long));
	  return ( sys_shutdown (get_fs_long (args+0),
				 get_fs_long (args+1)));

      case SYS_SETSOCKOPT:
	  verify_area (args, 5*sizeof (unsigned long));
	  return (sys_setsockopt (get_fs_long (args+0),
				  get_fs_long (args+1),
				  get_fs_long (args+2),
				  (char *)get_fs_long (args+3),
				  get_fs_long (args+4)));


      case SYS_GETSOCKOPT:
	  verify_area (args, 5*sizeof (unsigned long));
	  return (sys_getsockopt (get_fs_long (args+0),
				  get_fs_long (args+1),
				  get_fs_long (args+2),
				  (char *)get_fs_long (args+3),
				  (int *)get_fs_long (args+4)));

	default:
		return -EINVAL;
	}
}

void
sock_init(void)
{
	struct socket *sock;
	int i, ok;

	for (sock = sockets; sock <= last_socket; ++sock)
		sock->state = SS_FREE;
	for (i = ok = 0; i < NPROTO; ++i) {
		printk("sock_init: initializing family %d (%s)\n",
		       proto_table[i].family, proto_table[i].name);
		if ((*proto_table[i].ops->init)() < 0) {
			printk("sock_init: init failed.\n",
			       proto_table[i].family);
			proto_table[i].family = -1;
		}
		else
			++ok;
	}
	if (!ok)
		printk("sock_init: warning: no protocols initialized\n");
	return;
}

