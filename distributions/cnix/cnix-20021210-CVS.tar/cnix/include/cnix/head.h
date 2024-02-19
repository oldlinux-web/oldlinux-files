#ifndef HEAD_H
#define HEAD_H

typedef struct desc_struct{
	unsigned long a, b;
}desc_table[256];

extern unsigned long kp_dir[1024];
extern desc_table gdt, idt;

extern int end; /* edata, etext ..., I think need to clear bss or ... */

#endif
