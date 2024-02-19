/*
 *  linux/fs/exec.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * #!-checking implemented by tytso.
 */

/*
 * Demand-loading implemented 01.12.91 - no need to read anything but
 * the header into memory. The inode of the executable is put into
 * "current->executable", and page faults do the actual loading. Clean.
 *
 * Once more I can proudly say that linux stood up to being changed: it
 * was less than 2 hours work to get demand-loading completely implemented.
 */

#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/a.out.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/string.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/ptrace.h>
#include <linux/user.h>

#include <asm/segment.h>

extern int sys_exit(int exit_code);
extern int sys_close(int fd);

/*
 * MAX_ARG_PAGES defines the number of pages allocated for arguments
 * and envelope for the new program. 32 should suffice, this gives
 * a maximum env+arg of 128kB !
 */
#define MAX_ARG_PAGES 32

/*
 * These are the only things you should do on a core-file: use only these
 * macros to write out all the necessary info.
 */
#define DUMP_WRITE(addr,nr) \
while (file.f_op->write(inode,&file,(char *)(addr),(nr)) != (nr)) goto close_coredump

#define DUMP_SEEK(offset) \
if (file.f_op->lseek) { \
	if (file.f_op->lseek(inode,&file,(offset),0) != (offset)) \
 		goto close_coredump; \
} else file.f_pos = (offset)		

/*
 * Routine writes a core dump image in the current directory.
 * Currently only a stub-function.
 *
 * Note that setuid/setgid files won't make a core-dump if the uid/gid
 * changed due to the set[u|g]id. It's enforced by the "current->dumpable"
 * field, which also makes sure the core-dumps won't be recursive if the
 * dumping of the process results in another error..
 */
int core_dump(long signr, struct pt_regs * regs)
{
	struct inode * inode = NULL;
	struct file file;
	unsigned short fs;
	int has_dumped = 0;
	register int dump_start, dump_size;
	struct user dump;

	if (!current->dumpable)
		return 0;
	current->dumpable = 0;
/* See if we have enough room to write the upage.  */
	if(current->rlim[RLIMIT_CORE].rlim_cur < PAGE_SIZE/1024) return 0;
	__asm__("mov %%fs,%0":"=r" (fs));
	__asm__("mov %0,%%fs"::"r" ((unsigned short) 0x10));
	if (open_namei("core",O_CREAT | O_WRONLY | O_TRUNC,0600,&inode,NULL)) {
		inode = NULL;
		goto end_coredump;
	}
	if (!S_ISREG(inode->i_mode))
		goto end_coredump;
	if (!inode->i_op || !inode->i_op->default_file_ops)
		goto end_coredump;
	file.f_mode = 3;
	file.f_flags = 0;
	file.f_count = 1;
	file.f_inode = inode;
	file.f_pos = 0;
	file.f_reada = 0;
	file.f_op = inode->i_op->default_file_ops;
	if (file.f_op->open)
		if (file.f_op->open(inode,&file))
			goto end_coredump;
	if (!file.f_op->write)
		goto close_coredump;
	has_dumped = 1;
/* changed the size calculations - should hopefully work better. lbt */
	dump.magic = CMAGIC;
	dump.start_code = 0;
	dump.start_stack = regs->esp & ~(PAGE_SIZE - 1);
	dump.u_tsize = ((unsigned long) current->end_code) >> 12;
	dump.u_dsize = ((unsigned long) (current->brk + (PAGE_SIZE-1))) >> 12;
	dump.u_dsize -= dump.u_tsize;
	dump.u_ssize = 0;
	if (dump.start_stack < TASK_SIZE)
		dump.u_ssize = ((unsigned long) (TASK_SIZE - dump.start_stack)) >> 12;
/* If the size of the dump file exceeds the rlimit, then see what would happen
   if we wrote the stack, but not the data area.  */
	if ((dump.u_dsize+dump.u_ssize+1) * PAGE_SIZE/1024 >
	    current->rlim[RLIMIT_CORE].rlim_cur)
		dump.u_dsize = 0;
/* Make sure we have enough room to write the stack and data areas. */
	if ((dump.u_ssize+1) * PAGE_SIZE / 1024 >
	    current->rlim[RLIMIT_CORE].rlim_cur)
		dump.u_ssize = 0;
       	dump.u_comm = 0;
	dump.u_ar0 = (struct pt_regs *)(((int)(&dump.regs)) -((int)(&dump)));
	dump.signal = signr;
	dump.regs = *regs;
/* Flag indicating the math stuff is valid. We don't support this for the
   soft-float routines yet */
	if (hard_math) {
		if (dump.u_fpvalid = current->used_math) {
			if (last_task_used_math == current)
				__asm__("clts ; fnsave %0"::"m" (dump.i387));
			else
				memcpy(&dump.i387,&current->tss.i387.hard,sizeof(dump.i387));
		}
	} else {
		/* we should dump the emulator state here, but we need to
		   convert it into standard 387 format first.. */
		dump.u_fpvalid = 0;
	}
	__asm__("mov %0,%%fs"::"r" ((unsigned short) 0x10));
	DUMP_WRITE(&dump,sizeof(dump));
	DUMP_SEEK(sizeof(dump));
 /* Dump the task struct.  Not be used by gdb, but could be useful */
	DUMP_WRITE(current,sizeof(*current));
/* Now dump all of the user data.  Include malloced stuff as well */
	DUMP_SEEK(PAGE_SIZE);
/* now we start writing out the user space info */
	__asm__("mov %0,%%fs"::"r" ((unsigned short) 0x17));
/* Dump the data area */
	if (dump.u_dsize != 0) {
		dump_start = dump.u_tsize << 12;
		dump_size = dump.u_dsize << 12;
		DUMP_WRITE(dump_start,dump_size);
	};
/* Now prepare to dump the stack area */
	if (dump.u_ssize != 0) {
		dump_start = dump.start_stack;
		dump_size = dump.u_ssize << 12;
		DUMP_WRITE(dump_start,dump_size);
	};
close_coredump:
	if (file.f_op->release)
		file.f_op->release(inode,&file);
end_coredump:
	__asm__("mov %0,%%fs"::"r" (fs));
	iput(inode);
	return has_dumped;
}

/*
 * Note that a shared library must be both readable and executable due to
 * security reasons.
 *
 * Also note that we take the address to load from from the file itself.
 */
int sys_uselib(const char * library)
{
#define libnum	(current->numlibraries)
	struct inode * inode;
	struct buffer_head * bh;
	struct exec ex;
	int error;

	if (!library || get_limit(0x17) != TASK_SIZE)
		return -EINVAL;
	if ((libnum >= MAX_SHARED_LIBS) || (libnum < 0))
		return -EINVAL;
	error = namei(library,&inode);
	if (error)
		return error;
	if (!inode->i_sb || !S_ISREG(inode->i_mode) || !permission(inode,MAY_READ)) {
		iput(inode);
		return -EACCES;
	}
	if (!(bh = bread(inode->i_dev,bmap(inode,0),inode->i_sb->s_blocksize))) {
		iput(inode);
		return -EACCES;
	}
	if (!IS_RDONLY(inode)) {
		inode->i_atime = CURRENT_TIME;
		inode->i_dirt = 1;
	}
	ex = *(struct exec *) bh->b_data;
	brelse(bh);
	if (N_MAGIC(ex) != ZMAGIC || ex.a_trsize || ex.a_drsize ||
		ex.a_text+ex.a_data+ex.a_bss>0x3000000 ||
		inode->i_size < ex.a_text+ex.a_data+ex.a_syms+N_TXTOFF(ex)) {
		iput(inode);
		return -ENOEXEC;
	}
	current->libraries[libnum].library = inode;
	current->libraries[libnum].start = ex.a_entry;
	current->libraries[libnum].length = (ex.a_data+ex.a_text+0xfff) & 0xfffff000;
	current->libraries[libnum].bss = (ex.a_bss+0xfff) & 0xfffff000;
#if 0
	printk("Loaded library %d at %08x, length %08x\n",
		libnum,
		current->libraries[libnum].start,
		current->libraries[libnum].length);
#endif
	libnum++;
	return 0;
#undef libnum
}

/*
 * create_tables() parses the env- and arg-strings in new user
 * memory and creates the pointer tables from them, and puts their
 * addresses on the "stack", returning the new stack pointer value.
 */
static unsigned long * create_tables(char * p,int argc,int envc)
{
	unsigned long *argv,*envp;
	unsigned long * sp;

	sp = (unsigned long *) (0xfffffffc & (unsigned long) p);
	sp -= envc+1;
	envp = sp;
	sp -= argc+1;
	argv = sp;
	put_fs_long((unsigned long)envp,--sp);
	put_fs_long((unsigned long)argv,--sp);
	put_fs_long((unsigned long)argc,--sp);
	while (argc-->0) {
		put_fs_long((unsigned long) p,argv++);
		while (get_fs_byte(p++)) /* nothing */ ;
	}
	put_fs_long(0,argv);
	while (envc-->0) {
		put_fs_long((unsigned long) p,envp++);
		while (get_fs_byte(p++)) /* nothing */ ;
	}
	put_fs_long(0,envp);
	return sp;
}

/*
 * count() counts the number of arguments/envelopes
 */
static int count(char ** argv)
{
	int i=0;
	char ** tmp;

	if (tmp = argv)
		while (get_fs_long((unsigned long *) (tmp++)))
			i++;

	return i;
}

/*
 * 'copy_string()' copies argument/envelope strings from user
 * memory to free pages in kernel mem. These are in a format ready
 * to be put directly into the top of new user memory.
 *
 * Modified by TYT, 11/24/91 to add the from_kmem argument, which specifies
 * whether the string and the string array are from user or kernel segments:
 * 
 * from_kmem     argv *        argv **
 *    0          user space    user space
 *    1          kernel space  user space
 *    2          kernel space  kernel space
 * 
 * We do this by playing games with the fs segment register.  Since it
 * it is expensive to load a segment register, we try to avoid calling
 * set_fs() unless we absolutely have to.
 */
static unsigned long copy_strings(int argc,char ** argv,unsigned long *page,
		unsigned long p, int from_kmem)
{
	char *tmp, *pag = NULL;
	int len, offset = 0;
	unsigned long old_fs, new_fs;

	if (!p)
		return 0;	/* bullet-proofing */
	new_fs = get_ds();
	old_fs = get_fs();
	if (from_kmem==2)
		set_fs(new_fs);
	while (argc-- > 0) {
		if (from_kmem == 1)
			set_fs(new_fs);
		if (!(tmp = (char *)get_fs_long(((unsigned long *)argv)+argc)))
			panic("argc is wrong");
		if (from_kmem == 1)
			set_fs(old_fs);
		len=0;		/* remember zero-padding */
		do {
			len++;
		} while (get_fs_byte(tmp++));
		if (p < len) {	/* this shouldn't happen - 128kB */
			set_fs(old_fs);
			return 0;
		}
		while (len) {
			--p; --tmp; --len;
			if (--offset < 0) {
				offset = p % PAGE_SIZE;
				if (from_kmem==2)
					set_fs(old_fs);
				if (!(pag = (char *) page[p/PAGE_SIZE]) &&
				    !(pag = (char *) page[p/PAGE_SIZE] =
				      (unsigned long *) get_free_page(GFP_USER))) 
					return 0;
				if (from_kmem==2)
					set_fs(new_fs);

			}
			*(pag + offset) = get_fs_byte(tmp);
		}
	}
	if (from_kmem==2)
		set_fs(old_fs);
	return p;
}

static unsigned long change_ldt(unsigned long text_size,unsigned long * page)
{
	unsigned long code_limit,data_limit,code_base,data_base;
	int i;

	code_limit = TASK_SIZE;
	data_limit = TASK_SIZE;
	code_base = data_base = 0;
	current->start_code = code_base;
	set_base(current->ldt[1],code_base);
	set_limit(current->ldt[1],code_limit);
	set_base(current->ldt[2],data_base);
	set_limit(current->ldt[2],data_limit);
/* make sure fs points to the NEW data segment */
	__asm__("pushl $0x17\n\tpop %%fs"::);
	data_base += data_limit;
	for (i=MAX_ARG_PAGES-1 ; i>=0 ; i--) {
		data_base -= PAGE_SIZE;
		if (page[i])
			put_dirty_page(current,page[i],data_base);
	}
	return data_limit;
}

static void read_omagic(struct inode *inode, int bytes)
{
	struct buffer_head *bh;
	int n, blkno, blk = 0;
	char *dest = (char *) 0;
	unsigned int block_size;

	block_size = 1024;
	if (inode->i_sb)
		block_size = inode->i_sb->s_blocksize;
	while (bytes > 0) {
		if (!(blkno = bmap(inode, blk)))
			sys_exit(-1);
		if (!(bh = bread(inode->i_dev, blkno, block_size)))
			sys_exit(-1);
		n = (blk ? block_size : block_size - sizeof(struct exec));
		if (bytes < n)
			n = bytes;

		memcpy_tofs(dest, (blk ? bh->b_data :
				bh->b_data + sizeof(struct exec)), n);
		brelse(bh);
		++blk;
		dest += n;
		bytes -= n;
	}
	iput(inode);
	current->executable = NULL;
}

/*
 * 'do_execve()' executes a new program.
 *
 * NOTE! We leave 4MB free at the top of the data-area for a loadable
 * library.
 */
int do_execve(unsigned long * eip,long tmp,char * filename,
	char ** argv, char ** envp)
{
	struct inode * inode;
	struct buffer_head * bh;
	struct exec ex;
	unsigned long page[MAX_ARG_PAGES];
	int i,argc,envc;
	int e_uid, e_gid;
	int retval;
	int sh_bang = 0;
	unsigned long p=PAGE_SIZE*MAX_ARG_PAGES-4;
	int ch;

	if ((0xffff & eip[1]) != 0x000f)
		panic("execve called from supervisor mode");
	for (i=0 ; i<MAX_ARG_PAGES ; i++)	/* clear page-table */
		page[i]=0;
	retval = namei(filename,&inode);	/* get executable inode */
	if (retval)
		return retval;
	argc = count(argv);
	envc = count(envp);
	
restart_interp:
	if (!S_ISREG(inode->i_mode)) {	/* must be regular file */
		retval = -EACCES;
		goto exec_error2;
	}
	if (IS_NOEXEC(inode)) {		/* FS mustn't be mounted noexec */
		retval = -EPERM;
		goto exec_error2;
	}
	if (!inode->i_sb) {
		retval = -EACCES;
		goto exec_error2;
	}
	i = inode->i_mode;
	if (IS_NOSUID(inode) && (((i & S_ISUID) && inode->i_uid != current->
	    euid) || ((i & S_ISGID) && inode->i_gid != current->egid)) &&
	    !suser()) {
		retval = -EPERM;
		goto exec_error2;
	}
	/* make sure we don't let suid, sgid files be ptraced. */
	if (current->flags & PF_PTRACED) {
		e_uid = current->euid;
		e_gid = current->egid;
	} else {
		e_uid = (i & S_ISUID) ? inode->i_uid : current->euid;
		e_gid = (i & S_ISGID) ? inode->i_gid : current->egid;
	}
	if (current->euid == inode->i_uid)
		i >>= 6;
	else if (in_group_p(inode->i_gid))
		i >>= 3;
	if (!(i & 1) &&
	    !((inode->i_mode & 0111) && suser())) {
		retval = -EACCES;
		goto exec_error2;
	}
	if (!(bh = bread(inode->i_dev,bmap(inode,0),inode->i_sb->s_blocksize))) {
		retval = -EACCES;
		goto exec_error2;
	}
	if (!IS_RDONLY(inode)) {
		inode->i_atime = CURRENT_TIME;
		inode->i_dirt = 1;
	}
	ex = *((struct exec *) bh->b_data);	/* read exec-header */
	if ((bh->b_data[0] == '#') && (bh->b_data[1] == '!') && (!sh_bang)) {
		/*
		 * This section does the #! interpretation.
		 * Sorta complicated, but hopefully it will work.  -TYT
		 */

		char buf[128], *cp, *interp, *i_name, *i_arg;
		unsigned long old_fs;

		strncpy(buf, bh->b_data+2, 127);
		brelse(bh);
		iput(inode);
		buf[127] = '\0';
		if (cp = strchr(buf, '\n')) {
			*cp = '\0';
			for (cp = buf; (*cp == ' ') || (*cp == '\t'); cp++);
		}
		if (!cp || *cp == '\0') {
			retval = -ENOEXEC; /* No interpreter name found */
			goto exec_error1;
		}
		interp = i_name = cp;
		i_arg = 0;
		for ( ; *cp && (*cp != ' ') && (*cp != '\t'); cp++) {
 			if (*cp == '/')
				i_name = cp+1;
		}
		if (*cp) {
			*cp++ = '\0';
			i_arg = cp;
		}
		/*
		 * OK, we've parsed out the interpreter name and
		 * (optional) argument.
		 */
		if (sh_bang++ == 0) {
			p = copy_strings(envc, envp, page, p, 0);
			p = copy_strings(--argc, argv+1, page, p, 0);
		}
		/*
		 * Splice in (1) the interpreter's name for argv[0]
		 *           (2) (optional) argument to interpreter
		 *           (3) filename of shell script
		 *
		 * This is done in reverse order, because of how the
		 * user environment and arguments are stored.
		 */
		p = copy_strings(1, &filename, page, p, 1);
		argc++;
		if (i_arg) {
			p = copy_strings(1, &i_arg, page, p, 2);
			argc++;
		}
		p = copy_strings(1, &i_name, page, p, 2);
		argc++;
		if (!p) {
			retval = -ENOMEM;
			goto exec_error1;
		}
		/*
		 * OK, now restart the process with the interpreter's inode.
		 */
		old_fs = get_fs();
		set_fs(get_ds());
		retval = namei(interp,&inode);
		set_fs(old_fs);
		if (retval)
			goto exec_error1;
		goto restart_interp;
	}
	brelse(bh);
	if ((N_MAGIC(ex) != ZMAGIC && N_MAGIC(ex) != OMAGIC) ||
		ex.a_trsize || ex.a_drsize ||
		ex.a_text+ex.a_data+ex.a_bss>0x3000000 ||
		inode->i_size < ex.a_text+ex.a_data+ex.a_syms+N_TXTOFF(ex)) {
		retval = -ENOEXEC;
		goto exec_error2;
	}
	if (N_TXTOFF(ex) != BLOCK_SIZE && N_MAGIC(ex) != OMAGIC) {
		printk("%s: N_TXTOFF != BLOCK_SIZE. See a.out.h.", filename);
		retval = -ENOEXEC;
		goto exec_error2;
	}
	if (!sh_bang) {
		p = copy_strings(envc,envp,page,p,0);
		p = copy_strings(argc,argv,page,p,0);
		if (!p) {
			retval = -ENOMEM;
			goto exec_error2;
		}
	}
/* OK, This is the point of no return */
	current->dumpable = 1;
	for (i=0; (ch = get_fs_byte(filename++)) != '\0';)
		if (ch == '/')
			i = 0;
		else
			if (i < 8)
				current->comm[i++] = ch;
	if (i < 8)
		current->comm[i] = '\0';
	if (current->executable)
		iput(current->executable);
	i = current->numlibraries;
	while (i-- > 0) {
		iput(current->libraries[i].library);
		current->libraries[i].library = NULL;
	}
	if (e_uid != current->euid || e_gid != current->egid ||
	    !permission(inode,MAY_READ))
		current->dumpable = 0;
	current->numlibraries = 0;
	current->executable = inode;
	current->signal = 0;
	for (i=0 ; i<32 ; i++) {
		current->sigaction[i].sa_mask = 0;
		current->sigaction[i].sa_flags = 0;
		if (current->sigaction[i].sa_handler != SIG_IGN)
			current->sigaction[i].sa_handler = NULL;
	}
	for (i=0 ; i<NR_OPEN ; i++)
		if ((current->close_on_exec>>i)&1)
			sys_close(i);
	current->close_on_exec = 0;
	clear_page_tables(current);
	if (last_task_used_math == current)
		last_task_used_math = NULL;
	current->used_math = 0;
	p += change_ldt(ex.a_text,page);
	p -= MAX_ARG_PAGES*PAGE_SIZE;
	p = (unsigned long) create_tables((char *)p,argc,envc);
	current->brk = ex.a_bss +
		(current->end_data = ex.a_data +
		(current->end_code = ex.a_text));
	current->start_stack = p;
	current->rss = (TASK_SIZE - p + PAGE_SIZE-1) / PAGE_SIZE;
	current->suid = current->euid = e_uid;
	current->sgid = current->egid = e_gid;
	if (N_MAGIC(ex) == OMAGIC)
		read_omagic(inode, ex.a_text+ex.a_data);
	eip[0] = ex.a_entry;		/* eip, magic happens :-) */
	eip[3] = p;			/* stack pointer */
	if (current->flags & PF_PTRACED)
		send_sig(SIGTRAP, current, 0);
	return 0;
exec_error2:
	iput(inode);
exec_error1:
	for (i=0 ; i<MAX_ARG_PAGES ; i++)
		free_page(page[i]);
	return(retval);
}
