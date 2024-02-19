/* ptrace.c */
/* By Ross Biro 1/23/92 */

#include <linux/head.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <errno.h>
#include <asm/segment.h>
#include <asm/system.h>
#include <sys/ptrace.h>
/* does not yet catch signals sent when the child dies. in
   exit.c or in signal.c. */

/* determines which flags the user has access to. */
/* 1 = access 0 = no access */
#define FLAG_MASK 0x00000dd9

/* set's the trap flag. */
#define TRAP_FLAG 0x100

/* check's for granularity. */
#define GRANULARITY 0x00800000

/* this is the number to subtract from the top of the stack. To find
   the local frame. */

#define MAGICNUMBER 68

void do_no_page (unsigned long, unsigned long, struct task_struct *);
void write_verify (unsigned long);

/* change a pid into a task struct. */
static inline int get_task(int pid)
{
  int i;
  for (i =0; i < NR_TASKS; i++)
    {
      if (task[i] != NULL && (task[i]->pid == pid)) return (i);
    }
  return (-1);
}

/* this routine will get a word off of the processes priviledged stack. 
   the offset is how far from the base addr as stored in the TSS.  
   this routine assumes that all the priviledged stacks are in our
   data space. */
   
static inline int
get_stack_long(struct task_struct *task, int offset)
{
  unsigned char *stack;
  stack = (unsigned char *)task->tss.esp0;
  stack += offset;
  return (*((int *)stack));

}

/* this routine will put a word on the processes priviledged stack. 
   the offset is how far from the base addr as stored in the TSS.  
   this routine assumes that all the priviledged stacks are in our
   data space. */
   
static inline int
put_stack_long(struct task_struct *task, int offset, unsigned short data)
{
  unsigned char *stack;
  stack = (unsigned char *)task->tss.esp0;
  stack += offset;
  *(int *)stack = data;
  return (0);

}

/* this routine will get a word out of an arbitrary 
   tasks data space.  It likes to have the task number
   rather than the task pointer.  Perhaps the number
   should be included in the pointer. */
/* seg = 0 if I space */
static inline int get_long (int tsk, long addr, unsigned seg, int *data)
{
  int i;
  int limit;
  int cur;
  unsigned long address;
  unsigned long page;
  unsigned oldfs;
  /* find the task number of the current task. */
  for (i = 0; i < NR_TASKS ; i ++)
    {
      if (task[i] == current) break;
    }
  if (i == NR_TASKS) 
    {
      panic ("PTRACE: Can't find current task\n");
    }
  cur = i;

  /* we will need to check the redaability of the segment
     and then the byte in order to avoid segment violations. */
  seg++;
  limit=(task[tsk]->ldt[seg].a) & 0xffff;
  /* this should be constant amound all of our segments, but we
     had better check anyway. */
  if (task[tsk]->ldt[seg].b & GRANULARITY) limit = limit << 12;

  if (limit <= addr+4) return (-EIO);

  /* Now compute the address, and make sure that it is present. */
  address = ((task[tsk]->ldt[seg].a & 0xffff000) >> 8) |
            ((task[tsk]->ldt[seg].b & 0xff) << 16 ) |
	    (task[tsk]->ldt[seg].b & 0xff000000);

  page = *((unsigned long*) ((address >> 20) & 0xffc));
  /* see if it is present. */
  if (! (page & PAGE_PRESENT))
    {
      do_no_page (0, address, task[tsk]);
    }

  oldfs=get_fs();
  /* now convert seg to the right format. */
  seg = seg << 3 | 0x4;

  cli(); /* we are about to change our ldt, we better do it
	    with interrupts off.  Perhaps we should call schedule
	    first so that we won't be taking too much extra time. */
  lldt(tsk);
  set_fs(seg);
  *data = get_fs_long((void *)addr); /* we are assuming kernel space
					is in the gdt here. */
  lldt(cur);
  set_fs(oldfs);
  sti();
  return (0);
}

/* this routine will get a word out of an arbitrary 
   tasks data space.  It likes to have the task number
   rather than the task pointer.  Perhaps the number
   should be included in the pointer. */
/* seg = 0 if I space */
static inline int put_long (int tsk, long addr, int data, unsigned seg)
{
  int i;
  int limit;
  unsigned oldfs;
  unsigned long address;
  unsigned long page;
  int cur;
  /* find the task number of the current task. */
  for (i = 0; i < NR_TASKS ; i ++)
    {
      if (task[i] == current) break;
    }
  if (i == NR_TASKS) 
    {
      panic ("PTRACE: Can't find current task\n");
    }
  cur = i;

  /* we will need to check the readability of the segment
     and then the byte in order to avoid segment violations. */
  seg++;
  limit=(task[tsk]->ldt[seg].a) & 0xffff;
  /* this should be constant amound all of our segments, but we
     had better check anyway. */
  if (task[tsk]->ldt[seg].b & GRANULARITY) limit = limit << 12;

  if (limit <= addr+4) return (-EIO);

  /* Now compute the address, and make sure that it is present. */
  address = ((task[tsk]->ldt[seg].a & 0xffff000) >> 8) |
            ((task[tsk]->ldt[seg].b & 0xff) << 16 ) |
	    (task[tsk]->ldt[seg].b & 0xff000000);

  page = *((unsigned long*) ((address >> 20) & 0xffc));
  /* see if it is present. */
  if (! (page & PAGE_PRESENT))
    {
      do_no_page (0, address, task[tsk]);
    }
  write_verify (address);

  oldfs=get_fs();
  /* now convert seg to the right format. */
  seg = seg << 3 | 0x4;

  cli(); /* we are about to change our ldt, we better do it
	    with interrupts off.  Perhaps we should call schedule
	    first so that we won't be taking too much extra time. */
  lldt(tsk);
  set_fs(seg);
  put_fs_long(data,(void *)addr);
  lldt(cur);
  set_fs(oldfs);
  sti();
  return (0);
}


int
sys_ptrace( unsigned long *buffer)
/* Perform ptrace(request, pid, addr, data) syscall */
{
  long request, pid, data;
  long addr;
  struct task_struct *child;
  int childno;

  request = get_fs_long(buffer++);
  pid = get_fs_long(buffer++);
  addr = get_fs_long(buffer++); /* assume long = void * */
  data = get_fs_long(buffer++);

  if (request == 0)
    {
      /* set the ptrace bit in the proccess flags. */
      current->flags |= PF_PTRACED;
      return (0);
    }

  childno=get_task(pid);

  if (childno < 0)
    return (-ESRCH);
  else
    child = task[childno];

  if (child->p_pptr != current ||
      !(child->flags & PF_PTRACED) || child->state != TASK_STOPPED)
    return (-ESRCH);

  switch (request)
    {
	/* when I and D space are seperate, these will need to be fixed. */
    case 1: /* read word at location addr. */ 
    case 2: {
	int tmp;
	int res;
	res = get_long(childno, addr, 1, &tmp);
	if (res < 0)
		return res;
	verify_area(data, 4);
	put_fs_long( tmp, (unsigned long *)data);
	return 0;
    }

    case 3: /* read the word at location addr in the USER area. */
      {
	int tmp;
	addr = addr >> 2; /* temporary hack. */
	if (addr < 0 || addr >= 17)
		return (-EIO);
	verify_area(data, 4);
	tmp = get_stack_long (child, 4*addr-MAGICNUMBER);
	put_fs_long(tmp,(unsigned long *)data);
	return (0);
      }
    case 4: /* write the word at location addr. */
    case 5:
      /* when I and D space are seperate, this will have to be fixed. */
      if (put_long(childno, addr, data, 1)) return (-EIO);
      return (0);
      
    case 6: /* write the word at location addr in the USER area */
      addr = addr >> 2; /* temproary hack. */
      if (addr < 0 || addr >= 17) return (-EIO);
      if (addr == ORIG_EAX) return (-EIO);
      if (addr == EFL)   /* flags. */
	{
	  data &= FLAG_MASK;
	  data |= get_stack_long(child, EFL*4-MAGICNUMBER)  & ~FLAG_MASK;
	}
      
      if (put_stack_long(child, 4*addr-MAGICNUMBER, data)) return (-EIO);
      return (0);
      
    case 7: /* restart after signal. */
      {
	long tmp;
	child->signal=0;
	if (data > 0 && data <= NSIG)
	  child->signal = 1<<(data-1);
	child->state = 0;
	/* make sure the single step bit is not set. */
	tmp = get_stack_long (child, 4*EFL-MAGICNUMBER) & ~TRAP_FLAG;
	put_stack_long(child, 4*EFL-MAGICNUMBER,tmp);
	return (0);
      }

    case 8: /* make the child exit.  Best I can do is send it a sigkill. 
	       perhaps it should be put in the status that it want's to 
	       exit. */
      {
	long tmp;
	child->state = 0;
	child->signal = 1 << (SIGKILL -1 );
	/* make sure the single step bit is not set. */
	tmp = get_stack_long (child, 4*EFL-MAGICNUMBER) & ~TRAP_FLAG;
	put_stack_long(child, 4*EFL-MAGICNUMBER,tmp);
	return (0);
      }
      
    case 9:   /* set the trap flag. */
      {
	long tmp;
	tmp = get_stack_long (child, 4*EFL-MAGICNUMBER) | TRAP_FLAG;
	put_stack_long(child, 4*EFL-MAGICNUMBER,tmp);
	child->state = 0;
	child->signal=0;
	if (data > 0 && data <NSIG)
	  child->signal= 1<<(data-1);
	/* give it a chance to run. */
	return (0);
      }

    default:
      return (-EIO);
    }

}
