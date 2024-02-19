/*
 *	seagate.h Copyright (C) 1992 Drew Eckhardt 
 *	low level scsi driver header for ST01/ST02 by
 *		Drew Eckhardt 
 *
 *	<drew@colorado.edu>
 */

#ifndef _SEAGATE_H
	#define SEAGATE_H
/*
	$Header
*/
#ifndef ASM
int seagate_st0x_detect(int);
int seagate_st0x_command(unsigned char target, const void *cmnd,  void *buff, 
	int bufflen);
int seagate_st0x_queue_command(unsigned char target, const void *cmnd,  
	void *buff,  int bufflen, void (*done)(int, int));

int seagate_st0x_abort(int);
char *seagate_st0x_info(void);
int seagate_st0x_reset(void); 

#ifndef NULL
	#define NULL 0
#endif

#define SEAGATE_ST0X  {"Seagate ST-01/ST-02", seagate_st0x_detect, 	\
			 seagate_st0x_info, seagate_st0x_command,  	\
			 seagate_st0x_queue_command, seagate_st0x_abort, \
			 seagate_st0x_reset, 1, 7, 0, 0}
#endif


/*
	defining PARITY causes parity data to be checked
*/

#define PARITY

/*
	defining ARBITRATE causes the arbitration sequence to be used.  And speed to drop by a 
	factor of ten.
*/

#undef ARBITRATE


/*
	Thanks to Brian Antoine for the example code in his Messy-Loss ST-01
		driver, and Mitsugu Suzuki for information on the ST-01
		SCSI host.
*/

/*
	CONTROL defines
*/

#define CMD_RST 		0x01
#define CMD_SEL 		0x02
#define CMD_BSY 		0x04
#define CMD_ATTN    		0x08
#define CMD_START_ARB		0x10
#define CMD_EN_PARITY		0x20
#define CMD_INTR		0x40
#define CMD_DRVR_ENABLE		0x80

/*
	STATUS
*/

#define STAT_BSY		0x01
#define STAT_MSG		0x02
#define STAT_IO			0x04
#define STAT_CD			0x08
#define STAT_REQ		0x10
#define STAT_SEL		0x20
#define STAT_PARITY		0x40
#define STAT_ARB_CMPL		0x80

/* 
	REQUESTS
*/

#define REQ_MASK (STAT_CD |  STAT_IO | STAT_MSG)
#define REQ_DATAOUT 0
#define REQ_DATAIN STAT_IO
#define REQ_CMDOUT STAT_CD
#define REQ_STATIN (STAT_CD | STAT_IO)
#define REQ_MSGOUT (STAT_MSG | STAT_CD)
#define REQ_MSGIN (STAT_MSG | STAT_CD | STAT_IO)

extern volatile int seagate_st0x_timeout;

#ifdef PARITY
	#define BASE_CMD CMD_EN_PARITY
#else
	#define BASE_CMD  0
#endif

/*
	Debugging code
*/

#define PHASE_BUS_FREE 1
#define PHASE_ARBITRATION 2
#define PHASE_SELECTION 4
#define PHASE_DATAIN 8 
#define PHASE_DATAOUT 0x10
#define PHASE_CMDOUT 0x20
#define PHASE_MSGIN 0x40
#define PHASE_MSGOUT 0x80
#define PHASE_STATUSIN 0x100
#define PHASE_ETC (PHASE_DATAIN | PHASE_DATA_OUT | PHASE_CMDOUT | PHASE_MSGIN | PHASE_MSGOUT | PHASE_STATUSIN)
#define PRINT_COMMAND 0x200
#define PHASE_EXIT 0x400
#define PHASE_RESELECT 0x800

/* 
 *	Control options - these are timeouts specified in .01 seconds.
 */

#define ST0X_BUS_FREE_DELAY 25
#define ST0X_SELECTION_DELAY 3

#define eoi() __asm__("push %%eax\nmovb $0x20, %%al\noutb %%al, $0x20\npop %%eax"::)
	

#endif

