#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/stat.h>
#include <linux/socket.h>
#include <linux/un.h>
#include <linux/fcntl.h>
#include <linux/termios.h>

#include <asm/system.h>
#include <asm/segment.h>

#include "kern_sock.h"

static struct unix_proto_data {
	int refcnt;			/* cnt of reference 0=free */
	struct socket *socket;		/* socket we're bound to */
	int protocol;
	struct sockaddr_un sockaddr_un;
	short sockaddr_len;		/* >0 if name bound */
	char *buf;
	int bp_head, bp_tail;
	struct inode *inode;
	struct unix_proto_data *peerupd;
} unix_datas[NSOCKETS];
#define last_unix_data (unix_datas + NSOCKETS - 1)

#define UN_DATA(SOCK) ((struct unix_proto_data *)(SOCK)->data)
#define UN_PATH_OFFSET ((unsigned long)((struct sockaddr_un *)0)->sun_path)

/*
 * buffer size must be power of 2. buffer mgmt inspired by pipe code.
 * note that buffer contents can wraparound, and we can write one byte less
 * than full size to discern full vs empty.
 */
#define BUF_SIZE PAGE_SIZE
#define UN_BUF_AVAIL(UPD) (((UPD)->bp_head - (UPD)->bp_tail) & (BUF_SIZE-1))
#define UN_BUF_SPACE(UPD) ((BUF_SIZE-1) - UN_BUF_AVAIL(UPD))

static int unix_proto_init(void);
static int unix_proto_create(struct socket *sock, int protocol);
static int unix_proto_dup(struct socket *newsock, struct socket *oldsock);
static int unix_proto_release(struct socket *sock, struct socket *peer);
static int unix_proto_bind(struct socket *sock, struct sockaddr *umyaddr,
			   int sockaddr_len);
static int unix_proto_connect(struct socket *sock, struct sockaddr *uservaddr,
			      int sockaddr_len, int flags);
static int unix_proto_socketpair(struct socket *sock1, struct socket *sock2);
static int unix_proto_accept(struct socket *sock, struct socket *newsock, 
			     int flags);
static int unix_proto_getname(struct socket *sock, struct sockaddr *usockaddr,
			      int *usockaddr_len, int peer);
static int unix_proto_read(struct socket *sock, char *ubuf, int size,
			   int nonblock);
static int unix_proto_write(struct socket *sock, char *ubuf, int size,
			    int nonblock);
static int unix_proto_select(struct socket *sock, int sel_type, select_table * wait);
static int unix_proto_ioctl(struct socket *sock, unsigned int cmd,
			    unsigned long arg);
static int unix_proto_listen(struct socket *sock, int backlog);
static int unix_proto_send (struct socket *sock, void *buff, int len,
			    int nonblock, unsigned flags);
static int unix_proto_recv (struct socket *sock, void *buff, int len,
			    int nonblock, unsigned flags);
static int unix_proto_sendto (struct socket *sock, void *buff, int len,
			      int nonblock, unsigned flags,
			      struct sockaddr *addr, int addr_len);
static int unix_proto_recvfrom (struct socket *sock, void *buff, int len,
				int nonblock, unsigned flags,
				struct sockaddr *addr, int *addr_len);

static int unix_proto_shutdown (struct socket *sock, int how);

static int unix_proto_setsockopt (struct socket *sock, int level, int optname,
				  char *optval, int optlen);
static int unix_proto_getsockopt (struct socket *sock, int level, int optname,
				  char *optval, int *optlen);

struct proto_ops unix_proto_ops = {
	unix_proto_init,
	unix_proto_create,
	unix_proto_dup,
	unix_proto_release,
	unix_proto_bind,
	unix_proto_connect,
	unix_proto_socketpair,
	unix_proto_accept,
	unix_proto_getname,
	unix_proto_read,
	unix_proto_write,
	unix_proto_select,
 	unix_proto_ioctl,
 	unix_proto_listen,
 	unix_proto_send,
 	unix_proto_recv,
 	unix_proto_sendto,
 	unix_proto_recvfrom,
 	unix_proto_shutdown,
 	unix_proto_setsockopt,
 	unix_proto_getsockopt,
 	NULL /* unix_proto_fcntl. */
};

#ifdef SOCK_DEBUG
void
sockaddr_un_printk(struct sockaddr_un *sockun, int sockaddr_len)
{
	char buf[sizeof(sockun->sun_path) + 1];

	sockaddr_len -= UN_PATH_OFFSET;
	if (sockun->sun_family != AF_UNIX)
		printk("sockaddr_un: <BAD FAMILY: %d>\n", sockun->sun_family);
	else if (sockaddr_len <= 0 || sockaddr_len >= sizeof(buf)-1)
		printk("sockaddr_un: <BAD LENGTH: %d>\n", sockaddr_len);
	else {
		memcpy(buf, sockun->sun_path, sockaddr_len);
		buf[sockaddr_len] = '\0';
		printk("sockaddr_un: '%s'[%d]\n", buf,
		       sockaddr_len + UN_PATH_OFFSET);
	}
}
#endif
  
/* don't have to do anything. */
static int
unix_proto_listen (struct socket *sock, int backlog)
{
  return (0);
}

static int
unix_proto_setsockopt(struct socket *sock, int level, int optname,
		      char *optval, int optlen)
{
    return (-EOPNOTSUPP);
}

static int
unix_proto_getsockopt(struct socket *sock, int level, int optname,
		      char *optval, int *optlen)
{
    return (-EOPNOTSUPP);
}

static int
unix_proto_sendto(struct socket *sock, void *buff, int len, int nonblock, 
		  unsigned flags,  struct sockaddr *addr, int addr_len)
{
	return (-EOPNOTSUPP);
}     

static int
unix_proto_recvfrom(struct socket *sock, void *buff, int len, int nonblock, 
		    unsigned flags, struct sockaddr *addr, int *addr_len)
{
	return (-EOPNOTSUPP);
}     

static int
unix_proto_shutdown (struct socket *sock, int how)
{
	return (-EOPNOTSUPP);
}

static int
unix_proto_send(struct socket *sock, void *buff, int len, int nonblock,
		unsigned flags)
{
	/* this error needs to be checked. */
	if (flags != 0)
	  return (-EINVAL);
	return (unix_proto_write (sock, buff, len, nonblock));
}

static int
unix_proto_recv(struct socket *sock, void *buff, int len, int nonblock,
		unsigned flags)
{
	/* this error needs to be checked. */
	if (flags != 0)
	  return (-EINVAL);
	return (unix_proto_read (sock, buff, len, nonblock));
}


static struct unix_proto_data *
unix_data_lookup(struct sockaddr_un *sockun, int sockaddr_len)
{
	struct unix_proto_data *upd;

	for (upd = unix_datas; upd <= last_unix_data; ++upd) {
		if (upd->refcnt && upd->socket &&
		    upd->sockaddr_len == sockaddr_len &&
		    memcmp(&upd->sockaddr_un, sockun, sockaddr_len) == 0)
			return upd;
	}
	return NULL;
}

static struct unix_proto_data *
unix_data_alloc(void)
{
	struct unix_proto_data *upd;

	cli();
	for (upd = unix_datas; upd <= last_unix_data; ++upd) {
		if (!upd->refcnt) {
			upd->refcnt = 1;
			sti();
			upd->socket = NULL;
			upd->sockaddr_len = 0;
			upd->buf = NULL;
			upd->bp_head = upd->bp_tail = 0;
			upd->inode = NULL;
			upd->peerupd = NULL;
			return upd;
		}
	}
	sti();
	return NULL;
}

static inline void
unix_data_ref(struct unix_proto_data *upd)
{
	++upd->refcnt;
	PRINTK("unix_data_ref: refing data 0x%x (%d)\n", upd, upd->refcnt);
}

static void
unix_data_deref(struct unix_proto_data *upd)
{
	if (upd->refcnt == 1) {
		PRINTK("unix_data_deref: releasing data 0x%x\n", upd);
		if (upd->buf) {
			free_page((unsigned long)upd->buf);
			upd->buf = NULL;
			upd->bp_head = upd->bp_tail = 0;
		}
	}
	--upd->refcnt;
}

/*
 * upon a create, we allocate an empty protocol data, and grab a page to
 * buffer writes
 */
static int
unix_proto_create(struct socket *sock, int protocol)
{
	struct unix_proto_data *upd;

	PRINTK("unix_proto_create: socket 0x%x, proto %d\n", sock, protocol);
	if (protocol != 0) {
		PRINTK("unix_proto_create: protocol != 0\n");
		return -EINVAL;
	}
	if (!(upd = unix_data_alloc())) {
		printk("unix_proto_create: can't allocate buffer\n");
		return -ENOMEM;
	}
	if (!(upd->buf = (char *)get_free_page(GFP_USER))) {
		printk("unix_proto_create: can't get page!\n");
		unix_data_deref(upd);
		return -ENOMEM;
	}
	upd->protocol = protocol;
	upd->socket = sock;
	UN_DATA(sock) = upd;
	PRINTK("unix_proto_create: allocated data 0x%x\n", upd);
	return 0;
}

static int
unix_proto_dup(struct socket *newsock, struct socket *oldsock)
{
	struct unix_proto_data *upd = UN_DATA(oldsock);

	return unix_proto_create(newsock, upd->protocol);
}

static int
unix_proto_release(struct socket *sock, struct socket *peer)
{
	struct unix_proto_data *upd = UN_DATA(sock);

	PRINTK("unix_proto_release: socket 0x%x, unix_data 0x%x\n",
	       sock, upd);
	if (!upd)
		return 0;
	if (upd->socket != sock) {
		printk("unix_proto_release: socket link mismatch!\n");
		return -EINVAL;
	}
	if (upd->inode) {
		PRINTK("unix_proto_release: releasing inode 0x%x\n",
		       upd->inode);
		iput(upd->inode);
		upd->inode = NULL;
	}
	UN_DATA(sock) = NULL;
	upd->socket = NULL;
	if (upd->peerupd)
		unix_data_deref(upd->peerupd);
	unix_data_deref(upd);
	return 0;
}

/*
 * bind a name to a socket. this is where much of the work is done. we
 * allocate a fresh page for the buffer, grab the appropriate inode and
 * set things up.
 *
 * XXX what should we do if an address is already bound? here we return
 * EINVAL, but it may be necessary to re-bind. i think thats what bsd does
 * in the case of datagram sockets
 */
static int
unix_proto_bind(struct socket *sock, struct sockaddr *umyaddr,
		int sockaddr_len)
{
	struct unix_proto_data *upd = UN_DATA(sock);
	char fname[sizeof(((struct sockaddr_un *)0)->sun_path) + 1];
	int i;
	unsigned long old_fs;

	PRINTK("unix_proto_bind: socket 0x%x, len=%d\n", sock,
	       sockaddr_len);
	if (sockaddr_len <= UN_PATH_OFFSET ||
	    sockaddr_len >= sizeof(struct sockaddr_un)) {
		PRINTK("unix_proto_bind: bad length %d\n", sockaddr_len);
		return -EINVAL;
	}
	if (upd->sockaddr_len || upd->inode) {
		printk("unix_proto_bind: already bound!\n");
		return -EINVAL;
	}
	verify_area(umyaddr, sockaddr_len);
	memcpy_fromfs(&upd->sockaddr_un, umyaddr, sockaddr_len);
	if (upd->sockaddr_un.sun_family != AF_UNIX) {
		PRINTK("unix_proto_bind: family is %d, not AF_UNIX (%d)\n",
		       upd->sockaddr_un.sun_family, AF_UNIX);
		return -EINVAL;
	}

	memcpy(fname, upd->sockaddr_un.sun_path, sockaddr_len-UN_PATH_OFFSET);
	fname[sockaddr_len-UN_PATH_OFFSET] = '\0';
	old_fs = get_fs();
	set_fs(get_ds());
	i = do_mknod(fname, S_IFSOCK | 0777, 0);
	if (i == 0)
		i = open_namei(fname, 0, S_IFSOCK, &upd->inode, NULL);
	set_fs(old_fs);
	if (i < 0) {
		printk("unix_proto_bind: can't open socket %s\n", fname);
		return i;
	}

	upd->sockaddr_len = sockaddr_len;	/* now its legal */
	PRINTK("unix_proto_bind: bound socket address: ");
#ifdef SOCK_DEBUG
	sockaddr_un_printk(&upd->sockaddr_un, upd->sockaddr_len);
#endif
	return 0;
}

/*
 * perform a connection. we can only connect to unix sockets (i can't for
 * the life of me find an application where that wouldn't be the case!)
 */
static int
unix_proto_connect(struct socket *sock, struct sockaddr *uservaddr,
		   int sockaddr_len, int flags)
{
	int i;
	struct unix_proto_data *serv_upd;
	struct sockaddr_un sockun;

	PRINTK("unix_proto_connect: socket 0x%x, servlen=%d\n", sock,
	       sockaddr_len);
	if (sockaddr_len <= UN_PATH_OFFSET ||
	    sockaddr_len >= sizeof(struct sockaddr_un)) {
		PRINTK("unix_proto_connect: bad length %d\n", sockaddr_len);
		return -EINVAL;
	}
	verify_area(uservaddr, sockaddr_len);
	memcpy_fromfs(&sockun, uservaddr, sockaddr_len);
	if (sockun.sun_family != AF_UNIX) {
		PRINTK("unix_proto_connect: family is %d, not AF_UNIX (%d)\n",
		       sockun.sun_family, AF_UNIX);
		return -EINVAL;
	}
	if (!(serv_upd = unix_data_lookup(&sockun, sockaddr_len))) {
		PRINTK("unix_proto_connect: can't locate peer\n");
		return -EINVAL;
	}
	if ((i = sock_awaitconn(sock, serv_upd->socket)) < 0) {
		PRINTK("unix_proto_connect: can't await connection\n");
		return i;
	}
	unix_data_ref(UN_DATA(sock->conn));
	UN_DATA(sock)->peerupd = UN_DATA(sock->conn); /* ref server */
	return 0;
}

/*
 * to do a socketpair, we make just connect the two datas, easy! since we
 * always wait on the socket inode, they're no contention for a wait area,
 * and deadlock prevention in the case of a process writing to itself is,
 * ignored, in true unix fashion!
 */
static int
unix_proto_socketpair(struct socket *sock1, struct socket *sock2)
{
	struct unix_proto_data *upd1 = UN_DATA(sock1), *upd2 = UN_DATA(sock2);

	unix_data_ref(upd1);
	unix_data_ref(upd2);
	upd1->peerupd = upd2;
	upd2->peerupd = upd1;
	return 0;
}

/*
 * on accept, we ref the peer's data for safe writes
 */
static int
unix_proto_accept(struct socket *sock, struct socket *newsock, int flags)
{
   struct socket *clientsock;

	PRINTK("unix_proto_accept: socket 0x%x accepted via socket 0x%x\n",
	       sock, newsock);

	/*
	 * if there aren't any sockets awaiting connection, then wait for
	 * one, unless nonblocking
	 */
	while (!(clientsock = sock->iconn)) {
		if (flags & O_NONBLOCK)
			return -EAGAIN;
		interruptible_sleep_on(sock->wait);
		if (current->signal & ~current->blocked) {
			PRINTK("sys_accept: sleep was interrupted\n");
			return -ERESTARTSYS;
		}
	}

	/*
	 * great. finish the connection relative to server and client,
	 * wake up the client and return the new fd to the server
	 */
	sock->iconn = clientsock->next;
	clientsock->next = NULL;
	newsock->conn = clientsock;
	clientsock->conn = newsock;
	clientsock->state = SS_CONNECTED;
	newsock->state = SS_CONNECTED;
	wake_up(clientsock->wait);
        unix_data_ref (UN_DATA(newsock->conn));
	UN_DATA(newsock)->peerupd = UN_DATA(newsock->conn);
	return 0;
}

/*
 * gets the current name or the name of the connected socket.
 */
static int
unix_proto_getname(struct socket *sock, struct sockaddr *usockaddr,
		   int *usockaddr_len, int peer)
{
	struct unix_proto_data *upd;
	int len;

	PRINTK("unix_proto_getname: socket 0x%x for %s\n", sock,
	       peer ? "peer" : "self");
	if (peer) {
		if (sock->state != SS_CONNECTED) {
			PRINTK("unix_proto_getname: socket not connected\n");
			return -EINVAL;
		}
		upd = UN_DATA(sock->conn);
	}
	else
		upd = UN_DATA(sock);
	verify_area(usockaddr_len, sizeof(*usockaddr_len));
	if ((len = get_fs_long(usockaddr_len)) <= 0)
		return -EINVAL;
	if (len > upd->sockaddr_len)
		len = upd->sockaddr_len;
	if (len) {
		verify_area(usockaddr, len);
		memcpy_tofs(usockaddr, &upd->sockaddr_un, len);
	}
	put_fs_long(len, usockaddr_len);
	return 0;
}

/*
 * we read from our own buf.
 */
static int
unix_proto_read(struct socket *sock, char *ubuf, int size, int nonblock)
{
	struct unix_proto_data *upd;
	int todo, avail;

	if ((todo = size) <= 0)
		return 0;
	upd = UN_DATA(sock);
	while (!(avail = UN_BUF_AVAIL(upd))) {
		if (sock->state != SS_CONNECTED) {
			PRINTK("unix_proto_read: socket not connected\n");
			return (sock->state == SS_DISCONNECTING) ? 0 : -EINVAL;
		}
		PRINTK("unix_proto_read: no data available...\n");
		if (nonblock)
			return -EAGAIN;
		interruptible_sleep_on(sock->wait);
		if (current->signal & ~current->blocked) {
			PRINTK("unix_proto_read: interrupted\n");
			return -ERESTARTSYS;
		}
		if (sock->state == SS_DISCONNECTING) {
			PRINTK("unix_proto_read: disconnected\n");
			return 0;
		}
	}

	/*
	 * copy from the read buffer into the user's buffer, watching for
	 * wraparound. then we wake up the writer
	 */
	do {
		int part, cando;

		if (avail <= 0) {
			PRINTK("unix_proto_read: AVAIL IS NEGATIVE!!!\n");
			send_sig(SIGKILL,current,1);
			return -EINTR;
		}

		if ((cando = todo) > avail)
			cando = avail;
		if (cando > (part = BUF_SIZE - upd->bp_tail))
			cando = part;
		PRINTK("unix_proto_read: avail=%d, todo=%d, cando=%d\n",
		       avail, todo, cando);
		verify_area(ubuf, cando);
		memcpy_tofs(ubuf, upd->buf + upd->bp_tail, cando);
		upd->bp_tail = (upd->bp_tail + cando) & (BUF_SIZE-1);
		ubuf += cando;
		todo -= cando;
		if (sock->state == SS_CONNECTED)
			wake_up(sock->conn->wait);
		avail = UN_BUF_AVAIL(upd);
	} while (todo && avail);
	return size - todo;
}

/*
 * we write to our peer's buf. when we connected we ref'd this peer so we
 * are safe that the buffer remains, even after the peer has disconnected,
 * which we check other ways.
 */
static int
unix_proto_write(struct socket *sock, char *ubuf, int size, int nonblock)
{
	struct unix_proto_data *pupd;
	int todo, space;

	if ((todo = size) <= 0)
		return 0;
	if (sock->state != SS_CONNECTED) {
		PRINTK("unix_proto_write: socket not connected\n");
		if (sock->state == SS_DISCONNECTING) {
			send_sig(SIGPIPE,current,1);
			return -EINTR;
		}
		return -EINVAL;
	}
	pupd = UN_DATA(sock)->peerupd;	/* safer than sock->conn */

	while (!(space = UN_BUF_SPACE(pupd))) {
		PRINTK("unix_proto_write: no space left...\n");
		if (nonblock)
			return -EAGAIN;
		interruptible_sleep_on(sock->wait);
		if (current->signal & ~current->blocked) {
			PRINTK("unix_proto_write: interrupted\n");
			return -ERESTARTSYS;
		}
		if (sock->state == SS_DISCONNECTING) {
			PRINTK("unix_proto_write: disconnected (SIGPIPE)\n");
			send_sig(SIGPIPE,current,1);
			return -EINTR;
		}
	}

	/*
	 * copy from the user's buffer to the write buffer, watching for
	 * wraparound. then we wake up the reader
	 */
	do {
		int part, cando;

		if (space <= 0) {
			PRINTK("unix_proto_write: SPACE IS NEGATIVE!!!\n");
			send_sig(SIGKILL,current,1);
			return -EINTR;
		}

		/*
		 * we may become disconnected inside this loop, so watch
		 * for it (peerupd is safe until we close)
		 */
		if (sock->state == SS_DISCONNECTING) {
			send_sig(SIGPIPE,current,1);
			return -EINTR;
		}
		if ((cando = todo) > space)
			cando = space;
		if (cando > (part = BUF_SIZE - pupd->bp_head))
			cando = part;
		PRINTK("unix_proto_write: space=%d, todo=%d, cando=%d\n",
		       space, todo, cando);
		verify_area(ubuf, cando);
		memcpy_fromfs(pupd->buf + pupd->bp_head, ubuf, cando);
		pupd->bp_head = (pupd->bp_head + cando) & (BUF_SIZE-1);
		ubuf += cando;
		todo -= cando;
		if (sock->state == SS_CONNECTED)
			wake_up(sock->conn->wait);
		space = UN_BUF_SPACE(pupd);
	} while (todo && space);
	return size - todo;
}

static int
unix_proto_select(struct socket *sock, int sel_type, select_table * wait)
{
	struct unix_proto_data *upd, *peerupd;

	/*
	 * handle server sockets specially
	 */
	if (sock->flags & SO_ACCEPTCON) {
		if (sel_type == SEL_IN) {
			PRINTK("sock_select: %sconnections pending\n",
			       sock->iconn ? "" : "no ");
			if (sock->iconn)
				return 1;
			select_wait(sock->wait, wait);
			return sock->iconn ? 1 : 0;
		}
		PRINTK("sock_select: nothing else for server socket\n");
		select_wait(sock->wait, wait);
		return 0;
	}

	if (sel_type == SEL_IN) {
		upd = UN_DATA(sock);
		PRINTK("unix_proto_select: there is%s data available\n",
		       UN_BUF_AVAIL(upd) ? "" : " no");
		if (UN_BUF_AVAIL(upd))	/* even if disconnected */
			return 1;
		else if (sock->state != SS_CONNECTED) {
			PRINTK("unix_proto_select: socket not connected (read EOF)\n");
			return 1;
		}
		select_wait(sock->wait,wait);
		return 0;
	}
	if (sel_type == SEL_OUT) {
		if (sock->state != SS_CONNECTED) {
			PRINTK("unix_proto_select: socket not connected (write EOF)\n");
			return 1;
		}
		peerupd = UN_DATA(sock->conn);
		PRINTK("unix_proto_select: there is%s space available\n",
		       UN_BUF_SPACE(peerupd) ? "" : " no");
		if (UN_BUF_SPACE(peerupd) > 0)
			return 1;
		select_wait(sock->wait,wait);
		return 0;
	}
	/* SEL_EX */
	PRINTK("unix_proto_select: there are no exceptions here?!\n");
	return 0;
}

static int
unix_proto_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
	struct unix_proto_data *upd, *peerupd;

	upd = UN_DATA(sock);
	peerupd = (sock->state == SS_CONNECTED) ? UN_DATA(sock->conn) : NULL;

	switch (cmd) {

	case TIOCINQ:
		if (sock->flags & SO_ACCEPTCON)
			return -EINVAL;
		verify_area((void *)arg, sizeof(unsigned long));
		if (UN_BUF_AVAIL(upd) || peerupd)
			put_fs_long(UN_BUF_AVAIL(upd), (unsigned long *)arg);
		else
			put_fs_long(1, (unsigned long *)arg); /* read EOF */
		break;

	case TIOCOUTQ:
		if (sock->flags & SO_ACCEPTCON)
			return -EINVAL;
		verify_area((void *)arg, sizeof(unsigned long));
		if (peerupd)
			put_fs_long(UN_BUF_SPACE(peerupd),
				    (unsigned long *)arg);
		else
			put_fs_long(0, (unsigned long *)arg);
		break;

	default:
		return -EINVAL;
	}
	return 0;
}

static int
unix_proto_init(void)
{
	struct unix_proto_data *upd;

	PRINTK("unix_proto_init: initializing...\n");
	for (upd = unix_datas; upd <= last_unix_data; ++upd)
		upd->refcnt = 0;
	return 0;
}
