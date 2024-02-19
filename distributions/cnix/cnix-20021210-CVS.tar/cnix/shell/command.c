#include <unistd.h>
#include <const.h>
#include <stdarg.h>
#include <asm/io.h>
#include <asm/regs.h>
#include <asm/system.h>
#include <cnix/sched.h>
#include <cnix/cfile.h>
#include <string.h>

int volatile count = 0;
int volatile flag = 0; 
long volatile timer_count = 0;
int volatile sflag = 0;

extern void reboot(void);

shell_init()
{


        init();



}

static void init()
{
	int i;
	unsigned char c;
	unsigned char buff[20];
	/* PAGE_SIZE is the size of the stack of user stack */
	/* _syscall2(int, exec, fn_t, func, int, size) */
	/* exec(shell, PAGE_SIZE); */
	
	for(;;){
		printk("\n[cnix@ 51.net]# ");
		i = 0;
		c = getchar();
		while(c != '\n' && i < 20){
			printk("%c", c);
			buff[i++] = c;
			c = getchar();
			while (c ==127){
			  delchar();
			  buff[i]='\0';
			  i--;			  
			  c = getchar();
			}


		}

		buff[i] = '\0';
		printk("\n");
		if(c == '\n'){


		        if(!strcmp(buff, "help"))
		                help();

		        if(!strcmp(buff, "malloc"))
		                test_malloc();
			if(!strcmp(buff, "partition"))
			        print_part_list();
		        if(!strcmp(buff, "readme"))
		                read_file("readme");
		        if(!strcmp(buff, "shell"))
		                exe_shell("readme");
		        if(!strcmp(buff, "abc"))
		                read_file("abc");

			if(!strcmp(buff, "cnix"))
				printk("Welcome!");
			else if(!strcmp(buff, "reboot"))
				reboot();
			continue;
		}

		printk("%s", buff);
	}
}
/*for test ,and delete it some days later*/

test_malloc()
{
         char *abc;
	 char *s1 = "12345";

         abc = cnix_malloc(0x10);
	 strcpy(abc,s1);
	 
	 printk("\naddress:%x,%s\n",abc,abc);
}
read_file(char *filename)
{

      struct cfile *file_test;
      
      file_test = cnix_open("cnix",filename);
      
      printk("\n%s\n",file_test->file_buf);
}

static char *exe_entry;

exe_shell(char *filename)
{
      struct cfile *file_test;
      int i;

      file_test = cnix_open("cnix",filename);
      for(i=0x1000;i<0x100f;i++)
	      printk("%x ",file_test->file_buf[i]);
      printk("\n");
      exe_entry = &file_test->file_buf[0x1000];
	__asm__("push %%eax\n\t"
		"call *%%eax\n\t"
		"popl %%eax\n\t"
		:::"ax");

}
help()
{
  printk("\n");
  printk("partition    shell     readme   cnix   help\n");
  
}
