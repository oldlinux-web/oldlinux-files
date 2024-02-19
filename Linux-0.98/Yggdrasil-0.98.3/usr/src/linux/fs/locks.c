/*
 *  linux/fs/locks.c
 *
 *  Provide support for fcntl()'s F_GETLK, F_SETLK, and F_SETLKW calls.
 *  Doug Evans, 92Aug07, dje@sspiff.uucp.
 *
 * FIXME: two things aren't handled yet:
 *	- deadlock detection/avoidance (of dubious merit, but since it's in
 *	  the definition, I guess it should be provided eventually)
 *	- mandatory locks (requires lots of changes elsewhere)
 */

#include <asm/segment.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/stat.h>
#include <linux/fcntl.h>

#define OFFSET_MAX	0x7fffffff	/* FIXME: move elsewhere? */

static int copy_flock(struct file *filp, struct file_lock *fl, struct flock *l);
static int conflict(struct file_lock *caller_fl, struct file_lock *sys_fl);
static int overlap(struct file_lock *fl1, struct file_lock *fl2);
static int lock_it(struct file *filp, struct file_lock *caller);
static int unlock_it(struct file *filp, struct file_lock *caller);
static struct file_lock *alloc_lock(struct file *filp, struct file_lock *template);
static void free_lock(struct file *filp, struct file_lock *fl);

static struct file_lock file_lock_table[NR_FILE_LOCKS];
static struct file_lock *file_lock_free_list;

/*
 * Called at boot time to initialize the lock table ...
 */

void fcntl_init_locks(void)
{
	struct file_lock *fl;

	for (fl = &file_lock_table[0]; fl < file_lock_table + NR_FILE_LOCKS - 1; fl++) {
		fl->fl_next = fl + 1;
		fl->fl_owner = NULL;
	}
	file_lock_table[NR_FILE_LOCKS - 1].fl_next = NULL;
	file_lock_table[NR_FILE_LOCKS - 1].fl_owner = NULL;
	file_lock_free_list = &file_lock_table[0];
}

int fcntl_getlk(unsigned int fd, struct flock *l)
{	
	struct flock flock;
	struct file *filp;
	struct file_lock *fl,file_lock;

	if (fd >= NR_OPEN || !(filp = current->filp[fd]))
		return -EBADF;
	verify_area(l, sizeof(*l));
	memcpy_fromfs(&flock, l, sizeof(flock));
	if (flock.l_type == F_UNLCK)
		return -EINVAL;
	if (!copy_flock(filp, &file_lock, &flock))
		return -EINVAL;

	for (fl = filp->f_inode->i_flock; fl != NULL; fl = fl->fl_next) {
		if (conflict(&file_lock, fl)) {
			flock.l_pid = fl->fl_owner->pid;
			flock.l_start = fl->fl_start;
			flock.l_len = fl->fl_end == OFFSET_MAX ? 0 :
				fl->fl_end - fl->fl_start + 1;
			flock.l_whence = fl->fl_whence;
			flock.l_type = fl->fl_type;
			memcpy_tofs(l, &flock, sizeof(flock));
			return 0;
		}
	}

	flock.l_type = F_UNLCK;			/* no conflict found */
	memcpy_tofs(l, &flock, sizeof(flock));
	return 0;
}

/*
 * This function implements both F_SETLK and F_SETLKW.
 */

int fcntl_setlk(unsigned int fd, unsigned int cmd, struct flock *l)
{	
	struct file *filp;
	struct file_lock *fl,file_lock;
	struct flock flock;

	/*
	 * Get arguments and validate them ...
	 */

	if (fd >= NR_OPEN || !(filp = current->filp[fd]))
		return -EBADF;
	verify_area(l, sizeof(*l));
	memcpy_fromfs(&flock, l, sizeof(flock));
	if (!copy_flock(filp, &file_lock, &flock))
		return -EINVAL;
	switch (file_lock.fl_type) {
	case F_RDLCK :
		if (!(filp->f_mode & 1))
			return -EBADF;
		break;
	case F_WRLCK :
		if (!(filp->f_mode & 2))
			return -EBADF;
		break;
	case F_UNLCK :
		break;
	}

	/*
	 * F_UNLCK needs to be handled differently ...
	 */

	if (file_lock.fl_type == F_UNLCK)
		return unlock_it(filp, &file_lock);

	/*
	 * Scan for a conflicting lock ...
	 */

repeat:
	for (fl = filp->f_inode->i_flock; fl != NULL; fl = fl->fl_next) {
		if (!conflict(&file_lock, fl))
			continue;
		/*
		 * File is locked by another process. If this is F_SETLKW
		 * wait for the lock to be released.
		 * FIXME: We need to check for deadlocks here.
		 */
		if (cmd == F_SETLKW) {
			interruptible_sleep_on(&fl->fl_wait);
			goto repeat;
		}
		return -EAGAIN;
	}

	/*
	 * Lock doesn't conflict with any other lock ...
	 */

	return lock_it(filp, &file_lock);
}

/*
 * This function is called when the file is closed.
 */

void fcntl_remove_locks(struct task_struct *task, struct file *filp)
{
	struct file_lock *fl,*next;

	for (fl = filp->f_inode->i_flock; fl != NULL; ) {
		/*
		 * If this one is freed, {fl_next} gets clobbered when the
		 * entry is moved to the free list, so grab it now ...
		 */
		next = fl->fl_next;
		if (fl->fl_owner == task)
			free_lock(filp, fl);
		fl = next;
	}
}

/*
 * Verify a "struct flock" and copy it to a "struct file_lock" ...
 * Result is a boolean indicating success.
 */

static int copy_flock(struct file *filp, struct file_lock *fl, struct flock *l)
{
	off_t start;

	if (!filp->f_inode)	/* just in case */
		return 0;
	if (!S_ISREG(filp->f_inode->i_mode))
		return 0;
	if (l->l_type != F_UNLCK && l->l_type != F_RDLCK && l->l_type != F_WRLCK)
		return 0;
	switch (l->l_whence) {
	case 0 /*SEEK_SET*/ : start = 0; break;
	case 1 /*SEEK_CUR*/ : start = filp->f_pos; break;
	case 2 /*SEEK_END*/ : start = filp->f_inode->i_size; break;
	default : return 0;
	}
	if ((start += l->l_start) < 0 || l->l_len < 0)
		return 0;
	fl->fl_type = l->l_type;
	fl->fl_start = start;	/* we record the absolute position */
	fl->fl_whence = 0;	/* FIXME: do we record {l_start} as passed? */
	if (l->l_len == 0 || (fl->fl_end = start + l->l_len - 1) < 0)
		fl->fl_end = OFFSET_MAX;
	fl->fl_owner = current;
	fl->fl_wait = NULL;		/* just for cleanliness */
	return 1;
}

/*
 * Determine if lock {sys_fl} blocks lock {caller_fl} ...
 */

static int conflict(struct file_lock *caller_fl, struct file_lock *sys_fl)
{
	if (caller_fl->fl_owner == sys_fl->fl_owner)
		return 0;
	if (!overlap(caller_fl, sys_fl))
		return 0;
	switch (caller_fl->fl_type) {
	case F_RDLCK :
		return sys_fl->fl_type != F_RDLCK;
	case F_WRLCK :
		return 1;	/* overlapping region not owned by caller */
	}
	return 0;	/* shouldn't get here, but just in case */
}

static int overlap(struct file_lock *fl1, struct file_lock *fl2)
{
	if (fl1->fl_start <= fl2->fl_start) {
		return fl1->fl_end >= fl2->fl_start;
	} else {
		return fl2->fl_end >= fl1->fl_start;
	}
}

/*
 * Add a lock to a file ...
 * Result is 0 for success or -ENOLCK.
 *
 * We try to be real clever here and always minimize the number of table
 * entries we use. For example we merge adjacent locks whenever possible. This
 * consumes a bit of cpu and code space, is it really worth it? Beats me.
 *
 * I've tried to keep the following as small and simple as possible. If you can
 * make it smaller or simpler, please do. /dje 92Aug11
 *
 * WARNING: We assume the lock doesn't conflict with any other lock.
 */

static int lock_it(struct file *filp, struct file_lock *caller)
{
	struct file_lock *fl,*new;

	/*
	 * It's easier if we allocate a slot for the lock first, and then
	 * release it later if we have to (IE: if it can be merged with
	 * another). This way the for() loop always knows that {caller} is an
	 * existing entry. This will cause the routine to fail unnecessarily
	 * in rare cases, but perfection can be pushed too far. :-)
	 */

	if ((caller = alloc_lock(filp, caller)) == NULL)
		return -ENOLCK;

	/*
	 * First scan to see if we are changing/augmenting an existing lock ...
	 */

	for (fl = filp->f_inode->i_flock; fl != NULL; fl = fl->fl_next) {
		if (caller->fl_owner != fl->fl_owner)
			continue;
		if (caller == fl)
			continue;
		if (!overlap(caller, fl)) {
			/*
			 * Detect adjacent regions (if same lock type) ...
			 */
			if (caller->fl_type != fl->fl_type)
				continue;
			if (caller->fl_end + 1 == fl->fl_start) {
				fl->fl_start = caller->fl_start;
				free_lock(filp, caller);
				caller = fl;
				/* must continue, may overlap others now */
			} else if (caller->fl_start - 1 == fl->fl_end) {
				fl->fl_end = caller->fl_end;
				free_lock(filp, caller);
				caller = fl;
				/* must continue, may overlap others now */
			}
			continue;
		}
		/*
		 * We've found an overlapping region. Is it a change of lock
		 * type, or are we changing the size of the locked space?
		 */
		if (caller->fl_type != fl->fl_type) {
			if (caller->fl_start > fl->fl_start && caller->fl_end < fl->fl_end) {
				/*
				 * The new lock splits the old one in two ...
				 * {fl} is the bottom piece, {caller} is the
				 * new lock, and {new} is the top piece.
				 */
				if ((new = alloc_lock(filp, fl)) == NULL) {
					free_lock(filp, caller);
					return -ENOLCK;
				}
				fl->fl_end = caller->fl_start - 1;
				new->fl_start = caller->fl_end + 1;
				return 0;
			}
			if (caller->fl_start <= fl->fl_start && caller->fl_end >= fl->fl_end) {
				/*
				 * The new lock completely replaces old one ...
				 */
				free_lock(filp, fl);
				return 0;
			}
			if (caller->fl_end < fl->fl_end) {
				fl->fl_start = caller->fl_end + 1;
				/* must continue, may be more overlaps */
			} else if (caller->fl_start > fl->fl_start) {
				fl->fl_end = caller->fl_start - 1;
				/* must continue, may be more overlaps */
			} else {
				printk("lock_it: program bug: unanticipated overlap\n");
				free_lock(filp, caller);
				return -ENOLCK;
			}
		} else {	/* The new lock augments an existing lock ... */
			int grew = 0;

			if (caller->fl_start < fl->fl_start) {
				fl->fl_start = caller->fl_start;
				grew = 1;
			}
			if (caller->fl_end > fl->fl_end) {
				fl->fl_end = caller->fl_end;
				grew = 1;
			}
			free_lock(filp, caller);
			caller = fl;
			if (!grew)
				return 0;
			/* must continue, may be more overlaps */
		}
	}

	/*
	 * New lock doesn't overlap any regions ...
	 * alloc_lock() has already been called, so we're done!
	 */

	return 0;
}

/*
 * Handle F_UNLCK ...
 * Result is 0 for success, or -EINVAL or -ENOLCK.
 * ENOLCK can happen when a lock is split into two.
 */

static int unlock_it(struct file *filp, struct file_lock *caller)
{
	int one_unlocked = 0;
	struct file_lock *fl,*next;

	for (fl = filp->f_inode->i_flock; fl != NULL; ) {
		if (caller->fl_owner != fl->fl_owner || !overlap(caller, fl)) {
			fl = fl->fl_next;
			continue;
		}
		one_unlocked = 1;
		if (caller->fl_start > fl->fl_start && caller->fl_end < fl->fl_end) {
			/*
			 * Lock is split in two ...
			 * {fl} is the bottom piece, {next} is the top piece.
			 */
			if ((next = alloc_lock(filp, fl)) == NULL)
				return -ENOLCK;
			fl->fl_end = caller->fl_start - 1;
			next->fl_start = caller->fl_end + 1;
			return 0;
		}
		/*
		 * At this point we know there is an overlap and we know the
		 * lock isn't split into two ...
		 *
		 * Unless the lock table is broken, entries will not overlap.
		 * IE: User X won't have an entry locking bytes 1-3 and another
		 * entry locking bytes 3-5. Therefore, if the area being
		 * unlocked is a subset of the total area, we don't need to
		 * traverse any more of the list. The code is a tad more
		 * complicated by this optimization. Perhaps it's not worth it.
		 *
		 * WARNING: We assume free_lock() does not alter
		 *	{fl_start, fl_end}.
		 *
		 * {fl_next} gets clobbered when the entry is moved to
		 * the free list, so grab it now ...
		 */
		next = fl->fl_next;
		if (caller->fl_start <= fl->fl_start && caller->fl_end >= fl->fl_end) {
			free_lock(filp, fl);
		} else if (caller->fl_start > fl->fl_start) {
			fl->fl_end = caller->fl_start - 1;
		} else {
			/* caller->fl_end < fl->fl_end */
			fl->fl_start = caller->fl_end + 1;
		}
		if (caller->fl_start >= fl->fl_start && caller->fl_end <= fl->fl_end)
			return 0;		/* no more to be found */
		fl = next;
		/* must continue, there may be more to unlock */
	}

	return one_unlocked ? 0 : -EINVAL;
}

static struct file_lock *alloc_lock(struct file *filp, struct file_lock *template)
{
	struct file_lock *new;

	if (file_lock_free_list == NULL)
		return NULL;			/* no available entry */
	if (file_lock_free_list->fl_owner != NULL)
		panic("alloc_lock: broken free list\n");

	new = file_lock_free_list;		/* remove from free list */
	file_lock_free_list = file_lock_free_list->fl_next;

	*new = *template;

	new->fl_next = filp->f_inode->i_flock;	/* insert into file's list */
	filp->f_inode->i_flock = new;

	new->fl_owner = current;	/* FIXME: needed? */
	new->fl_wait = NULL;
	return new;
}

/*
 * Add a lock to the free list ...
 *
 * WARNING: We must not alter {fl_start, fl_end}. See unlock_it().
 */

static void free_lock(struct file *filp, struct file_lock *fl)
{
	struct file_lock **fl_p;

	if (fl->fl_owner == NULL)	/* sanity check */
		panic("free_lock: broken lock list\n");

	/*
	 * We only use a singly linked list to save some memory space
	 * (the only place we'd use a doubly linked list is here).
	 */

	for (fl_p = &filp->f_inode->i_flock; *fl_p != NULL; fl_p = &(*fl_p)->fl_next) {
		if (*fl_p == fl)
			break;
	}
	if (*fl_p == NULL) {
		printk("free_lock: lock is not in file's lock list\n");
	} else {
		*fl_p = (*fl_p)->fl_next;
	}

	fl->fl_next = file_lock_free_list;	/* add to free list */
	file_lock_free_list = fl;
	fl->fl_owner = NULL;			/* for sanity checks */

	wake_up(&fl->fl_wait);
}
