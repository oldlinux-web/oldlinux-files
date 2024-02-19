#include <stdio.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include "ps.h"


extern int optind;
extern char *optarg;


int pg_shift = 2;
int Sum = 0;
unsigned char *memmap;
unsigned pages;
unsigned long low_memory;

main(argc, argv)
char **argv;
{
    unsigned long _task;
    struct task_struct *taskp, task;
    int nr;

    if (open_psdb()) {
	perror("cannot open psdatabase");
	exit(2);
    }

    _task = k_addr("_task");

    if (argc == 1) {
	for (nr = 1; nr < NR_TASKS; ++nr) {
	    kmemread(&taskp, _task + 4*nr, 4);
	    if (taskp) {
		kmemread(&task, taskp, sizeof(task));
		show_m(&task);
	    }
	}
    } else {
	if (argv[1][0] == 's') {
	    unsigned dent;
	    printf("last swapped page: %05x000",
		(dent = get_kword(k_addr("_dir_entry.0"))) * 1024 +
		get_kword(k_addr("_page_entry.1")));
	    printf("  (task %d)\n", dent >> 4);
	    exit(0);

	}
	while (--argc > 0) {
	    ++argv;
	    if ((nr = atoi(*argv)) >= NR_TASKS || nr <= 0) {
		fprintf(stderr, "invalid task number: %d\n", nr);
		continue;
	    }

	    kmemread(&taskp, _task + 4*nr, 4);
	    if (taskp) {
		kmemread(&task, taskp, sizeof(task));
		show_m(&task);
	    } else
		fprintf(stderr, "no active process in slot %d\n", nr);
	}
    }

    exit(0);
}


#define	TEXT_PAGE	0x1000000
#define	DATA_PAGE	0x2000000
#define	BSS_PAGE	0x3000000
#define	LIB_PAGE	0x4000000

#define	SHARED		0x00100
#define	SWAPPED		0x00200
#define	MMAP		0x00400


show_m(task)
struct task_struct *task;
{
    int i;
    unsigned long buf[PAGE_SIZE/4], *pte;
    unsigned long pdir, ptbl;
    unsigned data_pag, bss_pag, brk_pag, addr;
    unsigned state, old_state= ~1, map_nr;
    int size=0, resident=0;
    int wr=0, dirty=0, acc=0, share=0, trs=0, lrs=0;
    int sh_lib=0, dt=0;
    int tpag = task->end_code / PAGE_SIZE;
    int ndup = 0;

    if (memmap == NULL)
	get_memmap();

    data_pag = task->end_code/PAGE_SIZE;
    bss_pag = task->end_data/PAGE_SIZE;
    brk_pag = (task->brk+0xfff) / PAGE_SIZE;

    printf("%08x %08x %08x\n", task->end_code, task->end_data, task->brk);
    printf("\nPid %d: %-8.8s\n", task->pid, task->comm);
    pdir = task->tss.cr3 + (task->start_code >> 20);
    addr = 0;
    for (i = 0; i < 16; ++i, pdir += 4) {
	ptbl = get_kword(pdir);
	if (ptbl == 0) {
	    tpag -= 1024;
	    addr += 1024;
	    continue;
	}
	kmemread(buf, ptbl & 0xfffff000, sizeof buf);

	for (pte = buf; pte < &buf[1024]; ++pte, ++addr, --tpag) {
	    if (addr < data_pag)
		state = TEXT_PAGE;
	    else
		if (addr < bss_pag)
		    state = DATA_PAGE;
		else
		    if (addr < brk_pag)
			state = BSS_PAGE;
		    else
			if (addr >= TASK_SIZE/PAGE_SIZE)
			    state = LIB_PAGE;
			else
			    state = 0;

	    if (*pte != 0) {
		++size;
		if (*pte & 1) {
		    state |= (*pte & 0x63);

		    ++resident;
		    if (tpag > 0) {
			++trs;
			/*
			if (*pte & 0x40) ++dt;
			*/
		    }
		    if (i == 15) {
			++lrs;
			if (*pte & 0x40) ++dt;
		    }
		    if ((map_nr = MAP_NR(*pte)) < pages) {
			state |= memmap[map_nr] << 16;
			if (memmap[map_nr] > 1) {
			    state |= SHARED;
			    if (i == 15)
				++sh_lib;
			    else
				++share;
			}
		    } else
			state |= MMAP;

		    if (*pte & 2) ++wr;
		    if (*pte & 0x40) ++dirty;
		    if (*pte & 0x20) ++acc;

		} else
		    if (*pte)
			state |= SWAPPED;

	    }
	    if (state != old_state) {
		switch (ndup) {
		    case 2: prstate(addr-2, old_state);
		    case 1: prstate(addr-1, old_state);
		    case 0: break;
		    default:
			    prstate(-1, old_state);
			    prstate(addr-1, old_state);
		}
		prstate(addr, state);
		ndup = 0;
		old_state = state;
	    } else
		++ndup;

	}
    }
    printf("accessed: %d,  dirty: %d,  writable: %d\n", acc, dirty, wr);
}

char *types[] = {
    "STACK", "TEXT", "DATA", "BSS", "LIB",
};

prstate(addr, state)
int addr, state;
{
    if (addr == -1)
	printf("      **  ");
    else {
	printf("%8x: ", addr * PAGE_SIZE);
    }

    printf("%-5s  ", types[state >> 24]);

    if (state & 0xffff) {

	if (state & SWAPPED)
	    printf("<swapped>\n");
	else
	    printf("%c%c%c%c   %3d\n",
		state & 0x20 ? 'A' : '-',
		state & 0x40 ? 'D' : '-',
		state & 0x02 ? 'W' : '-',
		state & MMAP ? 'M' : ' ',
		(state >> 16) & 0xff);
    } else
	printf("\n");
}

get_memmap()
{
    static unsigned long _mem_map;

    low_memory = get_kword(k_addr("_low_memory"));
    pages = (get_kword(k_addr("_high_memory")) - low_memory) / 4096;
    if (memmap == NULL) {
	_mem_map = get_kword(k_addr("_mem_map"));
	memmap = (unsigned char *) xmalloc(pages);
    }
    kmemread(memmap, _mem_map, pages);
}
