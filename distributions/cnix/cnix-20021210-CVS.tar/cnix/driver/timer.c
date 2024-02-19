#include <asm/io.h>
#include <asm/cmos.h>
#include <asm/system.h>
#include <cnix/sched.h>

#define HZ    100
#define LATCH (1193182/HZ)

struct bios_time{
	int second;
	int minute;
	int hour;
	int day;
	int month;
	int year;
	int century;
};

/* fn_t is defined in sched.h. */

extern int put_irq_handler(int irq, fn_t fn);
extern void schedule(void);

long volatile timer_count;
long volatile boot_time;

struct bios_time bios_time;

/*delay 50 ms by asmcos*/
delay_50ms()
{
      int time_delay;
      
      time_delay = timer_count;
      while((timer_count - time_delay ) < 6);
}
/*delay a second by asmcos*/
delay_s(int second)
{	
	int i;
	for(i=0;i<20 *second;i++)
		delay_50ms();
}

/* do some computing, and ..., struct regs_t */
static void do_timer(void)
{
	int hour, minute, second, seconds;
	unsigned char *vid_ptr = (unsigned char *)0xb8000 + 144;

	seconds = bios_time.hour * 3600 + bios_time.minute * 60 + 
		bios_time.second + (timer_count / HZ);

	hour = (seconds / 3600) % 24;
	minute = (seconds % 3600) / 60;
	second = seconds % 60;

	BIN_TO_BCD(hour); BIN_TO_BCD(minute); BIN_TO_BCD(second);
	
	*vid_ptr++ = ((hour >> 4) & 0x0f) + '0'; 
	*vid_ptr++ = 0x04; 
	*vid_ptr++ = (hour & 0x0f) + '0';
	*vid_ptr++ = 0x04;
	*vid_ptr++ = ':';
	*vid_ptr++ = 0x04;
	*vid_ptr++ = ((minute >> 4) & 0x0f) + '0';
	*vid_ptr++ = 0x04;
	*vid_ptr++ = (minute & 0x0f) + '0';
	*vid_ptr++ = 0x04;
	*vid_ptr++ = ':';
	*vid_ptr++ = 0x04;
	*vid_ptr++ = ((second >> 4) & 0x0f) + '0';
	*vid_ptr++ = 0x04;
	*vid_ptr++ = (second & 0x0f) + '0';
	*vid_ptr++ = 0x04;

	timer_count++;		

	/* Now do something really useful. */
	if(current != &(init_task.task))
		if(current->counter)
			current->counter--;
		else
			current->need_sched = 1;
}

#define MINUTE	60
#define HOUR	(MINUTE * 60)
#define DAY	(HOUR * 24)
#define YEAR	(DAY * 365)

/* the months of leap year */
static int month[12] = {
	0,
	DAY * (31),
	DAY * (31 + 29),
	DAY * (31 + 29 + 31),
	DAY * (31 + 29 + 31 + 30),
	DAY * (31 + 29 + 31 + 30 + 31),
	DAY * (31 + 29 + 31 + 30 + 31 + 30),
	DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31),
	DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31),
	DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30),
	DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31),
	DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30),
};

static long time_to_second(struct bios_time * time)
{
	long ret;
	int year;

	/* this century is 1 less than what we offen think of */
	year = time->year + time->century * 100 - 1970;

	/* 1972 % 4 == 0, 1973 - 1970 == 3, 3 + 1 == 4 */
	ret = year * YEAR + DAY * ((year + 1) / 4);	

	ret += month[time->month - 1];
	/* if it's not leap year */
	if(time->month > 2 && ((year + 2) % 4))
		ret -= DAY;
	ret += DAY * (time->day - 1);
	ret += HOUR * time->hour;
	ret += MINUTE * time->minute;
	ret += time->second;

	return ret;
}

timer_init()
{
	bios_time.second = CMOS_READ(RTC_SECOND);
	bios_time.minute = CMOS_READ(RTC_MINUTE);
	bios_time.hour = CMOS_READ(RTC_HOUR);
	bios_time.day = CMOS_READ(RTC_DAY_OF_MONTH);
	bios_time.month = CMOS_READ(RTC_MONTH);
	bios_time.year = CMOS_READ(RTC_YEAR);
	bios_time.century = CMOS_READ(RTC_CENTURY);
	BCD_TO_BIN(bios_time.second);	BCD_TO_BIN(bios_time.minute);
	BCD_TO_BIN(bios_time.hour);	BCD_TO_BIN(bios_time.day);
	BCD_TO_BIN(bios_time.month);	BCD_TO_BIN(bios_time.year);
	BCD_TO_BIN(bios_time.century);

	boot_time = time_to_second(&bios_time);

	printk("Boot time: %u\n", boot_time);
	printk("Century %d Year %02d Month %02d Day %02d Time: %02d : %02d : %02d\n", 
			bios_time.century,
			bios_time.year, bios_time.month,
			bios_time.day, bios_time.hour, 
			bios_time.minute, bios_time.second);

	timer_count = 0;		/* initilize */

  	/* set timer rate */
	outb(0x34, 0x43);		/* binary, mode 2, LSB/MSB, ch 0 */
	outb(LATCH & 0xff, 0x40);	/* LSB */
	outb(LATCH >> 8, 0x40);		/* MSB */

	put_irq_handler(0x00, do_timer);
}
