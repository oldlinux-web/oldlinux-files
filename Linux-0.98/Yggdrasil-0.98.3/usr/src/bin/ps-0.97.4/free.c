
#define _SYS_TYPES_H
#include <stdio.h>
#include <linux/mm.h>
#include <sys/stat.h>
#include "/usr/src/linux/kernel/blk_drv/blk.h"
#include "ps.h"

#define SWAP_BITS (4096<<3)

#define bitop(name,op) \
static inline int name(char * addr,unsigned int nr) \
{ \
int __res; \
__asm__ __volatile__("bt" op " %1,%2; adcl $0,%0" \
:"=g" (__res) \
:"r" (nr),"m" (*(addr)),"0" (0)); \
return __res; \
}

bitop(bit,"")
bitop(setbit,"s")
bitop(clrbit,"r")

extern int optind;
extern char *optarg;
unsigned long swap_bm;
int pg_shift = 2;
unsigned swap_total;
int verbose = 0;

main(argc, argv)
char **argv;
{
    int i, opt, interv = -1, dev = 0, one = 0;
    int req = 0, mem = 0, swap = 0, buf = 0, ino = 0, file = 0;
    char *devpath = NULL;

    if (open_psdb()) {
	perror("cannot open psdatabase");
	exit(2);
    }
    while ((opt = getopt(argc, argv, "pkcrmsbifvS:d:")) != -1) {
	switch (opt) {
	    case 'p': pg_shift = 0; break;
	    case 'k': pg_shift = 2; break;
	    case 'c': pg_shift = 12; break;
	    case 'r': req = 1; break;
	    case 'm': mem = 1; break;
	    case 's': swap = 1; break;
	    case 'b': buf = 1; break;
	    case 'i': ino = 1; break;
	    case 'f': file = 1; break;
	    case 'S': interv = atoi(optarg); break;
	    case 'd': devpath = optarg; break;
	    case 'v': verbose = 1; break;
	    default:
		fprintf(stderr, "usage: free [-msribpc] [-d block-dev] [-S interval] [swap-file]\n");
		exit(2);
	}
    }
    if (devpath) {
	struct stat st;
	if (stat(devpath, &st) == -1) {
	    fprintf(stderr, "cannot stat ");
	    perror(devpath);
	    exit(2);
	}
	dev = st.st_rdev;
    }
    swap_bm = /* get_kword(k_addr("_swap_bitmap"));  */ 0;

    if (!mem && !req && !swap && !buf && !ino && !file) {
	mem = 1;		/* default: mem and swap */
	swap = (swap_bm != 0);
    }

    if (argc > optind)
	swappath = argv[optind];

    if (swap && swap_bm)
	get_swap_total();

    while (1) {
	if (req)
	    show_req(dev);

	if (!one && (buf || ino || file))
	    printf("            total     space      used     dirty    locked    cached     fr/ch\n");

	if (buf)
	    show_buf(dev);

	if (ino)
	    show_ino(dev);

	if (file)
	    show_desc();

	if (!one && (mem || swap))
	    printf("             total       used      cache       free     shared\n");

	if (mem)
	    show_memory();

	if (swap)
	    show_swap();

	switch (interv) {
	    case 0: break;
	    case -1: exit(0);
	    default: sleep(interv);
	}
	one = (mem + swap + buf + req + ino + file == 1);
    }

    exit(0);
}

show_memory()
{
    static char *memmap;
    static unsigned long paging_pages, _mem_map, _nr_buffers, _nr_free_pages;
    unsigned used=0, freepg=0, shared=0, total=0, reserved=0;
    unsigned long buf_pages, low_buf, free_pages;
    int i;

    if (memmap == NULL) {
	paging_pages = (get_kword(k_addr("_high_memory"))) /4096;
	memmap = xmalloc(paging_pages);
	_mem_map = get_kword(k_addr("_mem_map"));
	_nr_buffers = k_addr("_nr_buffers");
	_nr_free_pages = k_addr("_nr_free_pages");
    }
    buf_pages = get_kword(_nr_buffers) / 4;	/* assumes block size = 1k */
    free_pages = get_kword(_nr_free_pages);
    kmemread(memmap, _mem_map, paging_pages);
    for (i = 0; i < paging_pages; ++i) {
      total++;
      if (memmap[i] & MAP_PAGE_RESERVED)
	reserved++;
      else if (!memmap[i])
	freepg++;
      else {
        used++;
	shared += memmap[i]-1;
      }
    }

    printf("memory: %10d %10d %10d %10d %10d\n",
	total << pg_shift,
	used << pg_shift,
	buf_pages << pg_shift,
	free_pages << pg_shift,
	shared << pg_shift);
}

show_swap()
{
    unsigned freepg=0;
    int i;
    char bitmap[4096];

    printf("swap:   ");

    if (swap_bm == 0)
	printf("No swap device.\n");
    else {
	freepg = 0;
	kmemread(bitmap, swap_bm, 4096);
	for (i = 0; i < SWAP_BITS; ++i)
	    freepg += bit(bitmap, i);

	if (swap_total != 0) {
	    printf("%10d %10d            ", 
		swap_total << pg_shift,
		swap_total - freepg << pg_shift);
	} else
	    printf("                                 ");

	printf("%10d\n", freepg << pg_shift);
    }
}

get_swap_total()
{
    char bitmap[4096];
    unsigned total = 0;
    int i, fd;

    if ((fd = open(swappath, 0)) == -1)
	return;

    if (read(fd, bitmap, sizeof bitmap) != sizeof bitmap) {
	fprintf(stderr, "cannot read %s\n", swappath);
	exit(1);
    }
    close(fd);
    if (strncmp(bitmap+4086, "SWAP-SPACE", 10)) {
	fprintf(stderr, "%s: bad swap signature\n", swappath);
	exit(1);
    }
    for (i = 0; i < SWAP_BITS - 80; ++i)
	total += bit(bitmap, i);

    swap_total = total;
}

show_req(dev)
{
    struct request request[NR_REQUEST];
    int i;
    int readreq = 0, writereq = 0;
    int maxread = 0, maxwrite = 0;
    static unsigned long _request;

    if (!_request)
	_request = k_addr("_request");
    kmemread(request, _request, sizeof request);
    if (dev) {
	for (i=0; i<NR_REQUEST; ++i)
	    if (request[i].dev == dev && (request[i].nr_sectors & 1))
		break;
	if (i < NR_REQUEST) {
	    while (1) {
		printf("%5d%c  ", request[i].sector, request[i].cmd ? 'w' : 'r');
		if (!request[i].next)
		    break;
		i = ((int) request[i].next - _request) / sizeof(struct request);
	    }
	    printf(" --\n");
	}
	return;
    }
    for (i=0; i<NR_REQUEST; ++i) {
	if (dev && request[i].dev != dev)
		continue;
	if (request[i].dev != -1) {
	    if (request[i].cmd) {
		++writereq;
		if (request[i].nr_sectors > maxwrite)
		    maxwrite = request[i].nr_sectors;
	    } else {
		++readreq;
		if (request[i].nr_sectors > maxread)
		    maxread = request[i].nr_sectors;
	    }
	}
    }
    printf("blk requests:  read: %d (max = %d)  write: %d (max = %d)  free: %d\n",
	readreq, maxread, writereq, maxwrite,
	NR_REQUEST - readreq - writereq);
}


show_buf(dev)
{
    int nr_buf;
    int i;
    struct buffer_head buf;
    int in_use = 0, dirt = 0, locked = 0, unused = 0, cached = 0;
    int chained = 0, buffers, size = 0;
    unsigned long high_memory, tmp, first;

    nr_buf = get_kword(k_addr("_nr_buffers"));
    buffers = nr_buf;
    high_memory = get_kword(k_addr("_high_memory"));
    first = get_kword(k_addr("_free_list"));

    for (tmp = first; buffers-- > 0; tmp = (unsigned long)buf.b_next_free) {
	if (tmp < 0 || tmp > high_memory) {
	    fprintf(stderr, "Bad pointer in free list: %x\n", tmp);
	    exit(1);
	}
	kmemread(&buf, tmp, sizeof buf);
	if (buf.b_dev == 0) ++unused;
	size += buf.b_size;
	if (dev && buf.b_dev != dev)
		continue;
	if (buf.b_count) ++in_use;
	if (buf.b_dirt) ++dirt;
	if (buf.b_lock) ++locked;
	if (buf.b_dev) ++cached;
	if (buf.b_reqnext) ++chained;
	if (verbose)
	    printf("%04x %5d  %c%c%c%c %3d\n", buf.b_dev, buf.b_blocknr,
		buf.b_count ? 'U' : ' ',
		buf.b_dirt ? 'D' : ' ',
		buf.b_lock ? 'L' : ' ',
		buf.b_reqnext ? 'C' : ' ',
		buf.b_count);
	if ((unsigned long)buf.b_next_free == first) {
	    if (buffers)
	      printf ("Warning: %d buffers are not counted in following totals\n",
		      buffers);
	    break;
	}
    }
    printf("buffers:%9d %9d %9d %9d %9d %9d %9d\n",
	nr_buf, size / 1024, in_use, dirt, locked, cached, chained);
}

show_ino(dev)
{
    struct inode ino[NR_INODE];
    int in_use = 0, dirt = 0, locked = 0, unused=NR_INODE;
    int i;

    kmemread(ino, k_addr("_inode_table"), sizeof ino);
    for (i = 0; i < NR_INODE; ++i) {
	if (ino[i].i_count) --unused;
	if (dev && ino[i].i_dev != dev)
		continue;
	if (ino[i].i_count) ++in_use;
	if (ino[i].i_dirt) ++dirt;
	if (ino[i].i_lock) ++locked;
    }
    printf("inodes: %9d           %9d %9d %9d           %9d\n",
	NR_INODE, in_use, dirt, locked, unused);
}

show_desc()
{
    struct file file[NR_FILE];
    int in_use = 0;
    int i;

    kmemread(file, k_addr("_file_table"), sizeof file);
    for (i = 0; i < NR_FILE; ++i) {
	if (file[i].f_count) ++in_use;
    }
    printf("descr:  %9d           %9d                               %9d\n",
	NR_FILE, in_use, NR_FILE-in_use);
}
