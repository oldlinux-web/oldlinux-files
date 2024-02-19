#include <const.h>
#include <asm/system.h>
#include <cnix/head.h>
#include <cnix/config.h>

#define KP_DIR_SIZE (HIGH_MEM / (PAGE_SIZE * 1024))
#define TOTAL_PAGES	((HIGH_MEM - LOW_MEM) / PAGE_SIZE)

#define invalidate() \
__asm__("movl %%eax, %%cr3"::"a"((unsigned long)kp_dir))

unsigned long low_page_mem = (unsigned long)&end;

struct page{
	int count;
	/* This could be also get from the address of page struct */
	unsigned long addr;	
	struct page * prev;
	struct page * next;
};

/* from LOW_MEM to ... HIGH_MEM */
struct page mem_map[TOTAL_PAGES + 1];
struct page * free_pg = &mem_map[TOTAL_PAGES];	
			/* as head, count will be used to count free 
			* pages, the last one in this queue's next points it.
			* When operating on it, do we need to cli ? 
			*/

void init_list(void)
{
	free_pg->count = 0;
	free_pg->addr = 0;
	free_pg->prev = (struct page *)free_pg;
	free_pg->next = (struct page *)free_pg;
}

/* add pg to tail */
int add_list(struct page * pg)
{
	/* need to lock ??? */
	free_pg->count++;
	if(free_pg->count == 0){
		free_pg->prev = free_pg->next = pg;
		pg->prev = pg->next = free_pg;
	}else{
		pg->next = free_pg;
		pg->prev = free_pg->prev;
		free_pg->prev->next = pg;
		free_pg->prev = pg;
	}
	
	return 0;
}

/* get pg from head */
struct page * del_list(void)
{
	struct page * pg;
	
	if(free_pg->count == 0)
		return (struct pg *)0;
	/* need to lock */
	free_pg->count--;
	pg = free_pg->next;
	free_pg->next = pg->next;
	pg->next->prev = free_pg;
	pg->prev = pg->next = (struct page *)0;
	
	return (struct page *)pg;
}

void mem_init(void)
{
	int i, j, index;
	long * kpd_ptr, * kp_ptr;
	unsigned long from_8M;

	kpd_ptr = (unsigned long *)kp_dir;	
	low_page_mem = (low_page_mem + (PAGE_SIZE - 1)) & (~(PAGE_SIZE - 1));
	kp_ptr = (long *)low_page_mem;	
	from_8M = 0x800000;
	
	/* setup paging for 8M - HIGH_MEM */
	for(i = 0; i < KP_DIR_SIZE; kpd_ptr++, i++){
		/* item 0 and item 1 have been initialized in head.S */
		if(i > 1){
			*kpd_ptr = (unsigned long)kp_ptr | 7;
			for(j = 0; j < (PAGE_SIZE / 4); kp_ptr++, j++){
				*kp_ptr = from_8M | 7;
				from_8M += PAGE_SIZE;
			}
		}
	}

	invalidate();
	
	/* from low_page_mem cound be given out */
	low_page_mem += PAGE_SIZE * (KP_DIR_SIZE - 2);

	/* reserve memory under low_page_mem */
	for(index = 0; index < (low_page_mem -LOW_MEM) / PAGE_SIZE; index++){
		/* I think this count will not be used. */
		mem_map[index].count = 1;
		mem_map[index].addr = LOW_MEM + (i * PAGE_SIZE);
		mem_map[index].prev = (struct page *)0;
		mem_map[index].next = (struct page *)0;
	}
	
	init_list();
	for(; index < TOTAL_PAGES; index++){
		mem_map[index].count = 0;
		mem_map[index].addr = LOW_MEM + (index * PAGE_SIZE);
		add_list(&mem_map[index]);
	}	
}

unsigned long get_one_page(void)
{
	unsigned long * addr;
	struct page * pg;

	pg = del_list();
	if(pg){
		/* clear the page */
		addr = (unsigned long *)pg->addr;

		do{
			*addr = 0;
			addr++;
		}while(((unsigned long)addr - pg->addr) < PAGE_SIZE);

		return pg->addr;
	}

	return 0;
}

void free_one_page(unsigned long addr)
{
	if(addr & 0xfff)
		panic("in free_one_page, address not alignment\n");

	/* I think kernel pages will not be freed */
	if(addr < low_page_mem)
		return;
			
	/* can't be this large */
	if(addr >= HIGH_MEM) 
		return;

	addr = (addr - LOW_MEM) >> 12;
	/* need to lock ??? */
	mem_map[addr].count--;
	if(mem_map[addr].count)
		return;
	add_list(&mem_map[addr]);
}

/* I think when one process to do exit, it must release all pages it takes,
   and then it's page dir, then it's pcb. */
/* from is page dir, and size is how many items in page dir. what's in %cr3 now
 * I think, it's kp_dir.
 */
int free_page_tables(unsigned long from, int size)
{
	unsigned long * pg_table, *pg_dir, nr;

	if(from & 0xfff)
		panic("in free_page_tables, error not alignment\n");
	
	if(from == kp_dir)
		panic("in free_page_tables, trying to free kernel page dir\n");
	
	pg_dir = (unsigned long *)from;
	
	for(; size-- > 0; pg_dir++){
		if(!(*pg_dir & 1))
			continue;

		/* if it's kernel page dir item */
		if((((unsigned long)pg_dir & (PAGE_SIZE - 1)) / 4) 
				< KP_DIR_SIZE){
			*pg_dir = 0;
			continue;
		}

		nr = 1024;
		pg_table = (unsigned long *)*pg_dir;
		for(; nr-- > 0; pg_table++){
			if(*pg_table & 1)
				free_one_page(*pg_table & 0xfffff000);
			*pg_table = 0;
		}
		
		/* sometimes I think it's not necessary to & ... */
		free_one_page(*pg_dir & 0xfffff000);

		*pg_dir = 0;
	}		

	return 0;
}

int copy_page_tables(unsigned long from, unsigned long to, int size)
{
	int nr;
	unsigned long * from_dir, * to_dir;
	unsigned long * from_pg_table, * to_pg_table, this_pg;
	
	if(from & 0xfff || to & 0xfff)
		panic("in copy_page_tables, address not alignment\n");

	from_dir = (unsigned long *)from;
	to_dir = (unsigned long *)to;

	for(; size-- > 0; from_dir++, to_dir++){
		if(*to_dir)
			panic("in copy_page_tables, \
				page dir item already exist\n"); 

		if(!(*from_dir & 1))
				continue;

		/* if it's kernel page dir item, skip next ... */
		if((((unsigned long)from_dir & (PAGE_SIZE - 1)) / 4) 
				< KP_DIR_SIZE){
			*to_dir = *from_dir;
			continue;
		}

		from_pg_table = (unsigned long *)(*from_dir & 0xfffff000);

		if(!(to_pg_table = (unsigned long *)get_one_page()))
				return -1;
		*to_dir = (unsigned long)to_pg_table | 7;	

		nr = 1024;
		for(; nr-- > 0; from_pg_table++, to_pg_table++){
			this_pg = *from_pg_table;
			if(!(this_pg & 1))
				continue;
			
			/* could not be given out, I think this condition 
			 * doesn't exist.
			 */
			if(this_pg < low_page_mem)
				panic("in copy_page_tables, can't happen.");
			
			/* copy on write, now write page_fault. */
			this_pg &= ~2;
			*from_pg_table = this_pg;
			*to_pg_table = this_pg;

			this_pg = (this_pg - LOW_MEM) >> 12;
			/* need to lock ???, will be decrement in ??? */
			mem_map[this_pg].count++;
		}
	}

	return 0;
}

