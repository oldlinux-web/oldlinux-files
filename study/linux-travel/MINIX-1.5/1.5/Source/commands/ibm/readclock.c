/* readclock - read the AT real time clock	Authors: T. Holm & E. Froese */

/************************************************************************/
/*									*/
/*   readclock.c							*/
/*									*/
/*		Read the AT real time clock, write the time to		*/
/*		standard output in a form usable by date(1).		*/
/*		If the system is an AT then the time is read		*/
/*		from the built-in clock, and written to standard	*/
/*		output in the form:					*/
/*									*/
/*			mmddyyhhmmss					*/
/*									*/
/*		If the system is not an AT then ``-q'' is written 	*/
/*		to standard output. This is useful for placement in	*/
/*		the ``/etc/rc'' script:					*/
/*									*/
/*	 	  /usr/bin/date `/usr/bin/readclock` </dev/tty  	*/
/*									*/
/************************************************************************/
/*    origination          1987-Dec-29              efth                */
/************************************************************************/



#define CPU_TYPE_SEGMENT   0xFFFF	/* BIOS segment for CPU type 	 */
#define CPU_TYPE_OFFSET    0x000E	/* BIOS offset for CPU type 	 */
#define PC_AT              0xFC	/* IBM code for PC-AT (0xFFFFE) */
#define PS_386		0xF8 /* IBM code for 386 PS/2's */	


#define CLK_ELE 0x70		/* ptr corresponding to element of time to be*/
#define CLK_IO 0x71		/* read or written is written to port clk_ele*/
/* The element can then be read or written by */
/* Reading or writing port clk_io.            */

#define  YEAR             9	/* Clock register addresses	 */
#define  MONTH            8
#define  DAY              7
#define  HOUR             4
#define  MINUTE           2
#define  SECOND           0
#define  STATUS           0x0b

#define  BCD_TO_DEC(x)	  ( (x>>4) * 10 + (x & 0x0f) )


struct time {
  unsigned year;
  unsigned month;
  unsigned day;
  unsigned hour;
  unsigned minute;
  unsigned second;
};



main()
{
  struct time time1;
  struct time time2;
  int i;
  int cpu_type;	

  cpu_type = peek(CPU_TYPE_SEGMENT, CPU_TYPE_OFFSET);
  if (cpu_type != PS_386 && cpu_type != PC_AT) {
	printf("-q\n");
	exit(1);
  }
  for (i = 0; i < 10; i++) {
	get_time(&time1);
	get_time(&time2);

	if (time1.year == time2.year &&
	    time1.month == time2.month &&
	    time1.day == time2.day &&
	    time1.hour == time2.hour &&
	    time1.minute == time2.minute &&
	    time1.second == time2.second) {
		printf("%02d%02d%02d%02d%02d%02d\n",
		       time1.month, time1.day, time1.year,
		       time1.hour, time1.minute, time1.second);
		exit(0);
	}
  }

  printf("-q\n");
  exit(1);
}



/***********************************************************************/
/*                                                                     */
/*    get_time( time )                                                 */
/*                                                                     */
/*    Update the structure pointed to by time with the current time    */
/*    as read from the hardware real-time clock of the AT.             */
/*    If necessary, the time is converted into a binary format before  */
/*    being stored in the structure.                                   */
/*                                                                     */
/***********************************************************************/

get_time(t)
struct time *t;
{
  t->year = read_register(YEAR);
  t->month = read_register(MONTH);
  t->day = read_register(DAY);
  t->hour = read_register(HOUR);
  t->minute = read_register(MINUTE);
  t->second = read_register(SECOND);



  if ((read_register(STATUS) & 0x04) == 0) {
	/* Convert BCD to binary if necessary */
	t->year = BCD_TO_DEC(t->year);
	t->month = BCD_TO_DEC(t->month);
	t->day = BCD_TO_DEC(t->day);
	t->hour = BCD_TO_DEC(t->hour);
	t->minute = BCD_TO_DEC(t->minute);
	t->second = BCD_TO_DEC(t->second);
  }
}


read_register(reg_addr)
char reg_addr;
{
  int val;

  if (port_out(CLK_ELE, reg_addr) < 0 || port_in(CLK_IO, &val) < 0) {
	printf("-q\n");
	exit(1);
  }
  return(val);
}
