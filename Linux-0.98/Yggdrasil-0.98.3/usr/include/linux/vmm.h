#ifndef _LINUX_VMM_H
#define _LINUX_VMM_H

/*
 * Linux kernel virtual memory manager primitives.
 * The idea being to have a "virtual" mm in the same way
 * we have a virtual fs - giving a cleaner interface to the
 * mm details, and allowing different kinds of memory mappings
 * (from shared memory to executable loading to arbitrary
 * mmap() functions).
 */

/*
 * This struct defines a memory VMM memory area. There is one of these
 * per VM-area/task.  A VM area is any part of the process virtual memory
 * space that has a special rule for the page-fault handlers (ie a shared
 * library, the executable area etc).
 */
struct vm_area_struct {
	struct task_struct * vm_task;		/* VM area parameters */
	unsigned long vm_start;
	unsigned long vm_end;
	struct vm_area_struct * vm_next;	/* ordered linked list */
	struct vm_area_struct * vm_share;	/* circular linked list */
	struct inode * vm_inode;
	unsigned long vm_offset;
	struct vm_operations_struct * vm_ops;
};

/*
 * These are the virtual MM functions - opening of an area, closing it (needed to
 * keep files on disk up-to-date etc), pointer to the functions called when a
 * no-page or a wp-page exception occurs, and the function which decides on sharing
 * of pages between different processes.
 */
struct vm_operations_struct {
	void (*open)(struct vm_area_struct * area);
	void (*close)(struct vm_area_struct * area);
	void (*nopage)(struct vm_area_struct * area, unsigned long address);
	void (*wppage)(struct vm_area_struct * area, unsigned long address);
	void (*share)(struct vm_area_struct * old, struct vm_area_struct * new, unsigned long address);
};

#endif
