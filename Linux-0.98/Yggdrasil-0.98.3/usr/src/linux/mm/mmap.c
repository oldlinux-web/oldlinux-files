/*
 *	linux/mm/mmap.c
 *
 * Written by obz.
 */
#include <linux/stat.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/mman.h>
#include <linux/string.h>

#include <asm/segment.h>
#include <asm/system.h>

/*
 * description of effects of mapping type and prot in current implementation.
 * this is due to the current handling of page faults in memory.c. the expected
 * behavior is in parens:
 *
 * map_type	prot
 *		PROT_NONE	PROT_READ	PROT_WRITE	PROT_EXEC
 * MAP_SHARED	r: (no) yes	r: (yes) yes	r: (no) yes	r: (no) no
 *		w: (no) yes	w: (no) copy	w: (yes) yes	w: (no) no
 *		x: (no) no	x: (no) no	x: (no) no	x: (yes) no
 *		
 * MAP_PRIVATE	r: (no) yes	r: (yes) yes	r: (no) yes	r: (no) no
 *		w: (no) copy	w: (no) copy	w: (copy) copy	w: (no) no
 *		x: (no) no	x: (no) no	x: (no) no	x: (yes) no
 *
 * the permissions are encoded as cxwr (copy,exec,write,read)
 */
#define MTYP(T) ((T) & MAP_TYPE)
#define PREAD(T,P) (((P) & PROT_READ) ? 1 : 0)
#define PWRITE(T,P) (((P) & PROT_WRITE) ? (MTYP(T) == MAP_SHARED ? 2 : 10) : 0)
#define PEXEC(T,P) (((P) & PROT_EXEC) ? 4 : 0)
#define PERMISS(T,P) (PREAD(T,P)|PWRITE(T,P)|PEXEC(T,P))

#define CODE_SPACE(addr) ((((addr)+4095)&~4095) < \
			  current->start_code + current->end_code)

static caddr_t
mmap_chr(unsigned long addr, size_t len, int prot, int flags,
	 struct inode *inode, unsigned long off)
{
	int major, minor;

	major = MAJOR(inode->i_rdev);
	minor = MINOR(inode->i_rdev);

	/*
	 * for character devices, only /dev/[k]mem may be mapped. when the
	 * swapping code is modified to allow arbitrary sources of pages,
	 * then we can open it up to regular files.
	 */

	if (major != 1 || (minor != 1 && minor != 2))
		return (caddr_t)-ENODEV;

	/*
	 * we only allow mappings from address 0 to high_memory, since thats
	 * the range of our memory [actually this is a lie. the buffer cache
	 * and ramdisk occupy higher memory, but the paging stuff won't
	 * let us map to it anyway, so we break it here].
	 *
	 * this call is very dangerous! because of the lack of adequate
	 * tagging of frames, it is possible to mmap over a frame belonging
	 * to another (innocent) process. with MAP_SHARED|MAP_WRITE, this
	 * rogue process can trample over the other's data! we ignore this :{
	 * for now, we hope people will malloc the required amount of space,
	 * then mmap over it. the mm needs serious work before this can be
	 * truly useful.
	 */

	if (len > high_memory || off > high_memory - len) /* avoid overflow */
		return (caddr_t)-ENXIO;

	if (remap_page_range(addr, off, len, PERMISS(flags, prot)))
		return (caddr_t)-EAGAIN;
	
	return (caddr_t)addr;
}

caddr_t
sys_mmap(unsigned long *buffer)
{
	unsigned long base, addr;
	unsigned long len, limit, off;
	int prot, flags, fd;
	struct file *file;
	struct inode *inode;

	addr = (unsigned long)	get_fs_long(buffer);	/* user address space*/
	len = (size_t)		get_fs_long(buffer+1);	/* nbytes of mapping */
	prot = (int)		get_fs_long(buffer+2);	/* protection */
	flags = (int)		get_fs_long(buffer+3);	/* mapping type */
	fd = (int) 		get_fs_long(buffer+4);	/* object to map */
	off = (unsigned long)	get_fs_long(buffer+5);	/* offset in object */

	if (fd >= NR_OPEN || fd < 0 || !(file = current->filp[fd]))
		return (caddr_t) -EBADF;
	if (addr > TASK_SIZE || (addr+(unsigned long) len) > TASK_SIZE)
		return (caddr_t) -EINVAL;
	inode = file->f_inode;

	/*
	 * do simple checking here so the lower-level routines won't have
	 * to. we assume access permissions have been handled by the open
	 * of the memory object, so we don't do any here.
	 */

	switch (flags & MAP_TYPE) {
	case MAP_SHARED:
		if ((prot & PROT_WRITE) && !(file->f_mode & 2))
			return (caddr_t)-EINVAL;
		/* fall through */
	case MAP_PRIVATE:
		if (!(file->f_mode & 1))
			return (caddr_t)-EINVAL;
		break;

	default:
		return (caddr_t)-EINVAL;
	}

	/*
	 * obtain the address to map to. we verify (or select) it and ensure
	 * that it represents a valid section of the address space. we assume
	 * that if PROT_EXEC is specified this should be in the code segment.
	 */
	if (prot & PROT_EXEC) {
		base = get_base(current->ldt[1]);	/* cs */
		limit = get_limit(0x0f);		/* cs limit */
	} else {
		base = get_base(current->ldt[2]);	/* ds */
		limit = get_limit(0x17);		/* ds limit */
	}

	if (flags & MAP_FIXED) {
		/*
		 * if MAP_FIXED is specified, we have to map exactly at this
		 * address. it must be page aligned and not ambiguous.
		 */
		if ((addr & 0xfff) || addr > 0x7fffffff || addr == 0 ||
		    (off & 0xfff))
			return (caddr_t)-EINVAL;
		if (addr + len > limit)
			return (caddr_t)-ENOMEM;
	} else {
		/*
		 * we're given a hint as to where to put the address.
		 * that we still need to search for a range of pages which
		 * are not mapped and which won't impact the stack or data
		 * segment.
		 * in linux, we only have a code segment and data segment.
		 * since data grows up and stack grows down, we're sort of
		 * stuck. placing above the data will break malloc, below
		 * the stack will cause stack overflow. because of this
		 * we don't allow nonspecified mappings...
		 */
		return (caddr_t)-ENOMEM;
	}

	/*
	 * determine the object being mapped and call the appropriate
	 * specific mapper. the address has already been validated, but
	 * not unmapped
	 */
	if (S_ISCHR(inode->i_mode))
		addr = (unsigned long)mmap_chr(base + addr, len, prot, flags,
					       inode, off);
	else
		addr = (unsigned long)-ENODEV;
	if ((long)addr > 0)
		addr -= base;

	return (caddr_t)addr;
}

int sys_munmap(unsigned long addr, size_t len)
{
	unsigned long base, limit;

	base = get_base(current->ldt[2]);	/* map into ds */
	limit = get_limit(0x17);		/* ds limit */

	if ((addr & 0xfff) || addr > 0x7fffffff || addr == 0 ||
	    addr + len > limit)
		return -EINVAL;
	if (unmap_page_range(base + addr, len))
		return -EAGAIN; /* should never happen */
	return 0;
}
