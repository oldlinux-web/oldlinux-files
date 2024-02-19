/***********************************************************************
		Copyright 1994 Comtrol Corporation.
			All Rights Reserved.

The following source code is subject to Comtrol Corporation's
Developer's License Agreement.

This source code is protected by United States copyright law and 
international copyright treaties.

This source code may only be used to develop software products that
will operate with Comtrol brand hardware.

You may not reproduce nor distribute this source code in its original
form but must produce a derivative work which includes portions of
this source code only.

The portions of this source code which you use in your derivative
work must bear Comtrol's copyright notice:

		Copyright 1994 Comtrol Corporation.

***********************************************************************/

#include "rcktldef.h"

#define ROCKET_C
#include "rckt.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

static Byte_t RData[RDATASIZE] =
{
   0x00, 0x09, 0xf6, 0x82,
   0x02, 0x09, 0x86, 0xfb,
   0x04, 0x09, 0x00, 0x0a,
   0x06, 0x09, 0x01, 0x0a,
   0x08, 0x09, 0x8a, 0x13,
   0x0a, 0x09, 0xc5, 0x11,
   0x0c, 0x09, 0x86, 0x85,
   0x0e, 0x09, 0x20, 0x0a,
   0x10, 0x09, 0x21, 0x0a,
   0x12, 0x09, 0x41, 0xff,
   0x14, 0x09, 0x82, 0x00,
   0x16, 0x09, 0x82, 0x7b,
   0x18, 0x09, 0x8a, 0x7d,
   0x1a, 0x09, 0x88, 0x81,
   0x1c, 0x09, 0x86, 0x7a,
   0x1e, 0x09, 0x84, 0x81,
   0x20, 0x09, 0x82, 0x7c,
   0x22, 0x09, 0x0a, 0x0a 
};

static Byte_t RRegData[RREGDATASIZE]=
{
   0x00, 0x09, 0xf6, 0x82,             /* 00: Stop Rx processor */
   0x08, 0x09, 0x8a, 0x13,             /* 04: Tx software flow control */
   0x0a, 0x09, 0xc5, 0x11,             /* 08: XON char */
   0x0c, 0x09, 0x86, 0x85,             /* 0c: XANY */
   0x12, 0x09, 0x41, 0xff,             /* 10: Rx mask char */
   0x14, 0x09, 0x82, 0x00,             /* 14: Compare/Ignore #0 */
   0x16, 0x09, 0x82, 0x7b,             /* 18: Compare #1 */
   0x18, 0x09, 0x8a, 0x7d,             /* 1c: Compare #2 */
   0x1a, 0x09, 0x88, 0x81,             /* 20: Interrupt #1 */
   0x1c, 0x09, 0x86, 0x7a,             /* 24: Ignore/Replace #1 */
   0x1e, 0x09, 0x84, 0x81,             /* 28: Interrupt #2 */
   0x20, 0x09, 0x82, 0x7c,             /* 2c: Ignore/Replace #2 */
   0x22, 0x09, 0x0a, 0x0a              /* 30: Rx FIFO Enable */
};

CONTROLLER_T sController[CTL_SIZE] =
{
   {-1,-1,0,0,0,0,0,0,0,0,0,{0,0,0,0},{0,0,0,0},{-1,-1,-1,-1},{0,0,0,0}},
   {-1,-1,0,0,0,0,0,0,0,0,0,{0,0,0,0},{0,0,0,0},{-1,-1,-1,-1},{0,0,0,0}},
   {-1,-1,0,0,0,0,0,0,0,0,0,{0,0,0,0},{0,0,0,0},{-1,-1,-1,-1},{0,0,0,0}},
   {-1,-1,0,0,0,0,0,0,0,0,0,{0,0,0,0},{0,0,0,0},{-1,-1,-1,-1},{0,0,0,0}}
};

#if 0
/* IRQ number to MUDBAC register 2 mapping */
Byte_t sIRQMap[16] =
{
   0,0,0,0x10,0x20,0x30,0,0,0,0x40,0x50,0x60,0x70,0,0,0x80
};
#endif

Byte_t sBitMapClrTbl[8] =
{
   0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f
};

Byte_t sBitMapSetTbl[8] =
{
   0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80
};

/***************************************************************************
Function: sInitController
Purpose:  Initialization of controller global registers and controller
          structure.
Call:     sInitController(CtlP,CtlNum,MudbacIO,AiopIOList,AiopIOListSize,
                          IRQNum,Frequency,PeriodicOnly)
          CONTROLLER_T *CtlP; Ptr to controller structure
          int CtlNum; Controller number
          ByteIO_t MudbacIO; Mudbac base I/O address.
          ByteIO_t *AiopIOList; List of I/O addresses for each AIOP.
             This list must be in the order the AIOPs will be found on the
             controller.  Once an AIOP in the list is not found, it is
             assumed that there are no more AIOPs on the controller.
          int AiopIOListSize; Number of addresses in AiopIOList
          int IRQNum; Interrupt Request number.  Can be any of the following:
                         0: Disable global interrupts
                         3: IRQ 3
                         4: IRQ 4
                         5: IRQ 5
                         9: IRQ 9
                         10: IRQ 10
                         11: IRQ 11
                         12: IRQ 12
                         15: IRQ 15
          Byte_t Frequency: A flag identifying the frequency
                   of the periodic interrupt, can be any one of the following:
                      FREQ_DIS - periodic interrupt disabled
                      FREQ_137HZ - 137 Hertz
                      FREQ_69HZ - 69 Hertz
                      FREQ_34HZ - 34 Hertz
                      FREQ_17HZ - 17 Hertz
                      FREQ_9HZ - 9 Hertz
                      FREQ_4HZ - 4 Hertz
                   If IRQNum is set to 0 the Frequency parameter is
                   overidden, it is forced to a value of FREQ_DIS.
          int PeriodicOnly: TRUE if all interrupts except the periodic
                               interrupt are to be blocked.
                            FALSE is both the periodic interrupt and
                               other channel interrupts are allowed.
                            If IRQNum is set to 0 the PeriodicOnly parameter is
                               overidden, it is forced to a value of FALSE.
Return:   int: Number of AIOPs on the controller, or CTLID_NULL if controller
               initialization failed.

Comments:
          If periodic interrupts are to be disabled but AIOP interrupts
          are allowed, set Frequency to FREQ_DIS and PeriodicOnly to FALSE.

          If interrupts are to be completely disabled set IRQNum to 0.

          Setting Frequency to FREQ_DIS and PeriodicOnly to TRUE is an
          invalid combination.

          This function performs initialization of global interrupt modes,
          but it does not actually enable global interrupts.  To enable
          and disable global interrupts use functions sEnGlobalInt() and
          sDisGlobalInt().  Enabling of global interrupts is normally not
          done until all other initializations are complete.

          Even if interrupts are globally enabled, they must also be
          individually enabled for each channel that is to generate
          interrupts.

Warnings: No range checking on any of the parameters is done.

          No context switches are allowed while executing this function.

          After this function all AIOPs on the controller are disabled,
          they can be enabled with sEnAiop().
*/
int sInitController(	CONTROLLER_T *CtlP,
			int CtlNum,
			ByteIO_t MudbacIO,
			ByteIO_t *AiopIOList,
			int AiopIOListSize,
			int IRQNum,
			Byte_t Frequency,
			int PeriodicOnly)
{
	int		i;
	ByteIO_t	io;

   CtlP->CtlNum = CtlNum;
   CtlP->CtlID = CTLID_0001;        /* controller release 1 */
   CtlP->BusType = isISA;     
   CtlP->MBaseIO = MudbacIO;
   CtlP->MReg1IO = MudbacIO + 1;
   CtlP->MReg2IO = MudbacIO + 2;
   CtlP->MReg3IO = MudbacIO + 3;
#if 1
   CtlP->MReg2 = 0;                 /* interrupt disable */
   CtlP->MReg3 = 0;                 /* no periodic interrupts */
#else
   if(sIRQMap[IRQNum] == 0)            /* interrupts globally disabled */
   {
      CtlP->MReg2 = 0;                 /* interrupt disable */
      CtlP->MReg3 = 0;                 /* no periodic interrupts */
   }
   else
   {
      CtlP->MReg2 = sIRQMap[IRQNum];   /* set IRQ number */
      CtlP->MReg3 = Frequency;         /* set frequency */
      if(PeriodicOnly)                 /* periodic interrupt only */
      {
         CtlP->MReg3 |= PERIODIC_ONLY;
      }
   }
#endif
   sOutB(CtlP->MReg2IO,CtlP->MReg2);
   sOutB(CtlP->MReg3IO,CtlP->MReg3);
   sControllerEOI(CtlP);               /* clear EOI if warm init */
   /* Init AIOPs */
   CtlP->NumAiop = 0;
   for(i=0; i < AiopIOListSize; i++)
   {
      io = AiopIOList[i];
      CtlP->AiopIO[i] = (WordIO_t)io;
      CtlP->AiopIntChanIO[i] = io + _INT_CHAN;
      sOutB(CtlP->MReg2IO,CtlP->MReg2 | (i & 0x03)); /* AIOP index */
      sOutB(MudbacIO,(Byte_t)(io >> 6));	/* set up AIOP I/O in MUDBAC */
      sEnAiop(CtlP,i);                         /* enable the AIOP */

      CtlP->AiopID[i] = sReadAiopID(io);       /* read AIOP ID */
      if(CtlP->AiopID[i] == AIOPID_NULL)       /* if AIOP does not exist */
      {
         sDisAiop(CtlP,i);                     /* disable AIOP */
         break;                                /* done looking for AIOPs */
      }

      CtlP->AiopNumChan[i] = sReadAiopNumChan((WordIO_t)io); /* num channels in AIOP */
      sOutW((WordIO_t)io + _INDX_ADDR,_CLK_PRE);      /* clock prescaler */
      sOutB(io + _INDX_DATA,CLOCK_PRESC);
      CtlP->NumAiop++;                         /* bump count of AIOPs */
      sDisAiop(CtlP,i);                        /* disable AIOP */
   }

   if(CtlP->NumAiop == 0)
      return(-1);
   else
      return(CtlP->NumAiop);
}

/***************************************************************************
Function: sPCIInitController
Purpose:  Initialization of controller global registers and controller
          structure.
Call:     sPCIInitController(CtlP,CtlNum,AiopIOList,AiopIOListSize,
                          IRQNum,Frequency,PeriodicOnly)
          CONTROLLER_T *CtlP; Ptr to controller structure
          int CtlNum; Controller number
          ByteIO_t *AiopIOList; List of I/O addresses for each AIOP.
             This list must be in the order the AIOPs will be found on the
             controller.  Once an AIOP in the list is not found, it is
             assumed that there are no more AIOPs on the controller.
          int AiopIOListSize; Number of addresses in AiopIOList
          int IRQNum; Interrupt Request number.  Can be any of the following:
                         0: Disable global interrupts
                         3: IRQ 3
                         4: IRQ 4
                         5: IRQ 5
                         9: IRQ 9
                         10: IRQ 10
                         11: IRQ 11
                         12: IRQ 12
                         15: IRQ 15
          Byte_t Frequency: A flag identifying the frequency
                   of the periodic interrupt, can be any one of the following:
                      FREQ_DIS - periodic interrupt disabled
                      FREQ_137HZ - 137 Hertz
                      FREQ_69HZ - 69 Hertz
                      FREQ_34HZ - 34 Hertz
                      FREQ_17HZ - 17 Hertz
                      FREQ_9HZ - 9 Hertz
                      FREQ_4HZ - 4 Hertz
                   If IRQNum is set to 0 the Frequency parameter is
                   overidden, it is forced to a value of FREQ_DIS.
          int PeriodicOnly: TRUE if all interrupts except the periodic
                               interrupt are to be blocked.
                            FALSE is both the periodic interrupt and
                               other channel interrupts are allowed.
                            If IRQNum is set to 0 the PeriodicOnly parameter is
                               overidden, it is forced to a value of FALSE.
Return:   int: Number of AIOPs on the controller, or CTLID_NULL if controller
               initialization failed.

Comments:
          If periodic interrupts are to be disabled but AIOP interrupts
          are allowed, set Frequency to FREQ_DIS and PeriodicOnly to FALSE.

          If interrupts are to be completely disabled set IRQNum to 0.

          Setting Frequency to FREQ_DIS and PeriodicOnly to TRUE is an
          invalid combination.

          This function performs initialization of global interrupt modes,
          but it does not actually enable global interrupts.  To enable
          and disable global interrupts use functions sEnGlobalInt() and
          sDisGlobalInt().  Enabling of global interrupts is normally not
          done until all other initializations are complete.

          Even if interrupts are globally enabled, they must also be
          individually enabled for each channel that is to generate
          interrupts.

Warnings: No range checking on any of the parameters is done.

          No context switches are allowed while executing this function.

          After this function all AIOPs on the controller are disabled,
          they can be enabled with sEnAiop().
*/
int sPCIInitController(	CONTROLLER_T *CtlP,
			int CtlNum,
			ByteIO_t *AiopIOList,
			int AiopIOListSize,
			int IRQNum,
			Byte_t Frequency,
			int PeriodicOnly)
{
	int		i;
	ByteIO_t	io;

   CtlP->CtlNum = CtlNum;
   CtlP->CtlID = CTLID_0001;        /* controller release 1 */
   CtlP->BusType = isPCI;        /* controller release 1 */

   CtlP->PCIIO = (WordIO_t)((ByteIO_t)AiopIOList[0] + _PCI_INT_FUNC);

   sPCIControllerEOI(CtlP);               /* clear EOI if warm init */
   /* Init AIOPs */
   CtlP->NumAiop = 0;
   for(i=0; i < AiopIOListSize; i++)
   {
      io = AiopIOList[i];
      CtlP->AiopIO[i] = (WordIO_t)io;
      CtlP->AiopIntChanIO[i] = io + _INT_CHAN;

      CtlP->AiopID[i] = sReadAiopID(io);       /* read AIOP ID */
      if(CtlP->AiopID[i] == AIOPID_NULL)       /* if AIOP does not exist */
         break;                                /* done looking for AIOPs */

      CtlP->AiopNumChan[i] = sReadAiopNumChan((WordIO_t)io); /* num channels in AIOP */
      sOutW((WordIO_t)io + _INDX_ADDR,_CLK_PRE);      /* clock prescaler */
      sOutB(io + _INDX_DATA,CLOCK_PRESC);
      CtlP->NumAiop++;                         /* bump count of AIOPs */
   }

   if(CtlP->NumAiop == 0)
      return(-1);
   else
      return(CtlP->NumAiop);
}

/***************************************************************************
Function: sReadAiopID
Purpose:  Read the AIOP idenfication number directly from an AIOP.
Call:     sReadAiopID(io)
          ByteIO_t io: AIOP base I/O address
Return:   int: Flag AIOPID_XXXX if a valid AIOP is found, where X
                 is replace by an identifying number.
          Flag AIOPID_NULL if no valid AIOP is found
Warnings: No context switches are allowed while executing this function.

*/
int sReadAiopID(ByteIO_t io)
{
   Byte_t AiopID;               /* ID byte from AIOP */

   sOutB(io + _CMD_REG,RESET_ALL);     /* reset AIOP */
   sOutB(io + _CMD_REG,0x0);
   AiopID = sInB(io + _CHN_STAT0) & 0x07;
   if(AiopID == 0x06)
      return(1);
   else                                /* AIOP does not exist */
      return(-1);
}

/***************************************************************************
Function: sReadAiopNumChan
Purpose:  Read the number of channels available in an AIOP directly from
          an AIOP.
Call:     sReadAiopNumChan(io)
          WordIO_t io: AIOP base I/O address
Return:   int: The number of channels available
Comments: The number of channels is determined by write/reads from identical
          offsets within the SRAM address spaces for channels 0 and 4.
          If the channel 4 space is mirrored to channel 0 it is a 4 channel
          AIOP, otherwise it is an 8 channel.
Warnings: No context switches are allowed while executing this function.
*/
int sReadAiopNumChan(WordIO_t io)
{
   Word_t x;

   sOutDW((DWordIO_t)io + _INDX_ADDR,0x12340000L); /* write to chan 0 SRAM */
   sOutW(io + _INDX_ADDR,0);       /* read from SRAM, chan 0 */
   x = sInW(io + _INDX_DATA);
   sOutW(io + _INDX_ADDR,0x4000);  /* read from SRAM, chan 4 */
   if(x != sInW(io + _INDX_DATA))  /* if different must be 8 chan */
      return(8);
   else
      return(4);
}

/***************************************************************************
Function: sInitChan
Purpose:  Initialization of a channel and channel structure
Call:     sInitChan(CtlP,ChP,AiopNum,ChanNum)
          CONTROLLER_T *CtlP; Ptr to controller structure
          CHANNEL_T *ChP; Ptr to channel structure
          int AiopNum; AIOP number within controller
          int ChanNum; Channel number within AIOP
Return:   int: TRUE if initialization succeeded, FALSE if it fails because channel
               number exceeds number of channels available in AIOP.
Comments: This function must be called before a channel can be used.
Warnings: No range checking on any of the parameters is done.

          No context switches are allowed while executing this function.
*/
int sInitChan(	CONTROLLER_T *CtlP,
		CHANNEL_T *ChP,
		int AiopNum,
		int ChanNum)
{
   int i;
   WordIO_t AiopIO;
   WordIO_t ChIOOff;
   Byte_t *ChR;
   Word_t ChOff;
   static Byte_t R[4];

   if(ChanNum >= CtlP->AiopNumChan[AiopNum])
      return(FALSE);                   /* exceeds num chans in AIOP */

   /* Channel, AIOP, and controller identifiers */
   ChP->CtlP = CtlP;
   ChP->ChanID = CtlP->AiopID[AiopNum];
   ChP->AiopNum = AiopNum;
   ChP->ChanNum = ChanNum;

   /* Global direct addresses */
   AiopIO = CtlP->AiopIO[AiopNum];
   ChP->Cmd = (ByteIO_t)AiopIO + _CMD_REG;
   ChP->IntChan = (ByteIO_t)AiopIO + _INT_CHAN;
   ChP->IntMask = (ByteIO_t)AiopIO + _INT_MASK;
   ChP->IndexAddr = (DWordIO_t)AiopIO + _INDX_ADDR;
   ChP->IndexData = AiopIO + _INDX_DATA;

   /* Channel direct addresses */
   ChIOOff = AiopIO + ChP->ChanNum * 2;
   ChP->TxRxData = ChIOOff + _TD0;
   ChP->ChanStat = ChIOOff + _CHN_STAT0;
   ChP->TxRxCount = ChIOOff + _FIFO_CNT0;
   ChP->IntID = (ByteIO_t)AiopIO + ChP->ChanNum + _INT_ID0;

   /* Initialize the channel from the RData array */
   for(i=0; i < RDATASIZE; i+=4)
   {
      R[0] = RData[i];
      R[1] = RData[i+1] + 0x10 * ChanNum;
      R[2] = RData[i+2];
      R[3] = RData[i+3];
      sOutDW(ChP->IndexAddr,*((DWord_t *)&R[0]));
   }

   ChR = ChP->R;
   for(i=0; i < RREGDATASIZE; i+=4)
   {
      ChR[i] = RRegData[i];
      ChR[i+1] = RRegData[i+1] + 0x10 * ChanNum;
      ChR[i+2] = RRegData[i+2];
      ChR[i+3] = RRegData[i+3];
   }

   /* Indexed registers */
   ChOff = (Word_t)ChanNum * 0x1000;

   ChP->BaudDiv[0] = (Byte_t)(ChOff + _BAUD);
   ChP->BaudDiv[1] = (Byte_t)((ChOff + _BAUD) >> 8);
   ChP->BaudDiv[2] = (Byte_t)BRD9600;
   ChP->BaudDiv[3] = (Byte_t)(BRD9600 >> 8);
   sOutDW(ChP->IndexAddr,*(DWord_t *)&ChP->BaudDiv[0]);

   ChP->TxControl[0] = (Byte_t)(ChOff + _TX_CTRL);
   ChP->TxControl[1] = (Byte_t)((ChOff + _TX_CTRL) >> 8);
   ChP->TxControl[2] = 0;
   ChP->TxControl[3] = 0;
   sOutDW(ChP->IndexAddr,*(DWord_t *)&ChP->TxControl[0]);

   ChP->RxControl[0] = (Byte_t)(ChOff + _RX_CTRL);
   ChP->RxControl[1] = (Byte_t)((ChOff + _RX_CTRL) >> 8);
   ChP->RxControl[2] = 0;
   ChP->RxControl[3] = 0;
   sOutDW(ChP->IndexAddr,*(DWord_t *)&ChP->RxControl[0]);

   ChP->TxEnables[0] = (Byte_t)(ChOff + _TX_ENBLS);
   ChP->TxEnables[1] = (Byte_t)((ChOff + _TX_ENBLS) >> 8);
   ChP->TxEnables[2] = 0;
   ChP->TxEnables[3] = 0;
   sOutDW(ChP->IndexAddr,*(DWord_t *)&ChP->TxEnables[0]);

   ChP->TxCompare[0] = (Byte_t)(ChOff + _TXCMP1);
   ChP->TxCompare[1] = (Byte_t)((ChOff + _TXCMP1) >> 8);
   ChP->TxCompare[2] = 0;
   ChP->TxCompare[3] = 0;
   sOutDW(ChP->IndexAddr,*(DWord_t *)&ChP->TxCompare[0]);

   ChP->TxReplace1[0] = (Byte_t)(ChOff + _TXREP1B1);
   ChP->TxReplace1[1] = (Byte_t)((ChOff + _TXREP1B1) >> 8);
   ChP->TxReplace1[2] = 0;
   ChP->TxReplace1[3] = 0;
   sOutDW(ChP->IndexAddr,*(DWord_t *)&ChP->TxReplace1[0]);

   ChP->TxReplace2[0] = (Byte_t)(ChOff + _TXREP2);
   ChP->TxReplace2[1] = (Byte_t)((ChOff + _TXREP2) >> 8);
   ChP->TxReplace2[2] = 0;
   ChP->TxReplace2[3] = 0;
   sOutDW(ChP->IndexAddr,*(DWord_t *)&ChP->TxReplace2[0]);

   ChP->TxFIFOPtrs = ChOff + _TXF_OUTP;
   ChP->TxFIFO = ChOff + _TX_FIFO;

   sOutB(ChP->Cmd,(Byte_t)ChanNum | RESTXFCNT); /* apply reset Tx FIFO count */
   sOutB(ChP->Cmd,(Byte_t)ChanNum);  /* remove reset Tx FIFO count */
   sOutW((WordIO_t)ChP->IndexAddr,ChP->TxFIFOPtrs); /* clear Tx in/out ptrs */
   sOutW(ChP->IndexData,0);
   ChP->RxFIFOPtrs = ChOff + _RXF_OUTP;
   ChP->RxFIFO = ChOff + _RX_FIFO;

   sOutB(ChP->Cmd,(Byte_t)ChanNum | RESRXFCNT); /* apply reset Rx FIFO count */
   sOutB(ChP->Cmd,(Byte_t)ChanNum);  /* remove reset Rx FIFO count */
   sOutW((WordIO_t)ChP->IndexAddr,ChP->RxFIFOPtrs); /* clear Rx out ptr */
   sOutW(ChP->IndexData,0);
   sOutW((WordIO_t)ChP->IndexAddr,ChP->RxFIFOPtrs + 2); /* clear Rx in ptr */
   sOutW(ChP->IndexData,0);
   ChP->TxPrioCnt = ChOff + _TXP_CNT;
   sOutW((WordIO_t)ChP->IndexAddr,ChP->TxPrioCnt);
   sOutB(ChP->IndexData,0);
   ChP->TxPrioPtr = ChOff + _TXP_PNTR;
   sOutW((WordIO_t)ChP->IndexAddr,ChP->TxPrioPtr);
   sOutB(ChP->IndexData,0);
   ChP->TxPrioBuf = ChOff + _TXP_BUF;
   sEnRxProcessor(ChP);                /* start the Rx processor */

   return(TRUE);
}

/***************************************************************************
Function: sStopRxProcessor
Purpose:  Stop the receive processor from processing a channel.
Call:     sStopRxProcessor(ChP)
          CHANNEL_T *ChP; Ptr to channel structure

Comments: The receive processor can be started again with sStartRxProcessor().
          This function causes the receive processor to skip over the
          stopped channel.  It does not stop it from processing other channels.

Warnings: No context switches are allowed while executing this function.

          Do not leave the receive processor stopped for more than one
          character time.

          After calling this function a delay of 4 uS is required to ensure
          that the receive processor is no longer processing this channel.
*/
void sStopRxProcessor(CHANNEL_T *ChP)
{
   Byte_t R[4];

   R[0] = ChP->R[0];
   R[1] = ChP->R[1];
   R[2] = 0x0a;
   R[3] = ChP->R[3];
   sOutDW(ChP->IndexAddr,*(DWord_t *)&R[0]);
}

/***************************************************************************
Function: sFlushRxFIFO
Purpose:  Flush the Rx FIFO
Call:     sFlushRxFIFO(ChP)
          CHANNEL_T *ChP; Ptr to channel structure
Return:   void
Comments: To prevent data from being enqueued or dequeued in the Tx FIFO
          while it is being flushed the receive processor is stopped
          and the transmitter is disabled.  After these operations a
          4 uS delay is done before clearing the pointers to allow
          the receive processor to stop.  These items are handled inside
          this function.
Warnings: No context switches are allowed while executing this function.
*/
void sFlushRxFIFO(CHANNEL_T *ChP)
{
   int i;
   Byte_t Ch;                   /* channel number within AIOP */
   int RxFIFOEnabled;                  /* TRUE if Rx FIFO enabled */

   if(sGetRxCnt(ChP) == 0)             /* Rx FIFO empty */
      return;                          /* don't need to flush */

   RxFIFOEnabled = FALSE;
   if(ChP->R[0x32] == 0x08) /* Rx FIFO is enabled */
   {
      RxFIFOEnabled = TRUE;
      sDisRxFIFO(ChP);                 /* disable it */
      for(i=0; i < 2000/200; i++)	/* delay 2 uS to allow proc to disable FIFO*/
         sInB(ChP->IntChan);		/* depends on bus i/o timing */
   }
   sGetChanStatus(ChP);          /* clear any pending Rx errors in chan stat */
   Ch = (Byte_t)sGetChanNum(ChP);
   sOutB(ChP->Cmd,Ch | RESRXFCNT);     /* apply reset Rx FIFO count */
   sOutB(ChP->Cmd,Ch);                 /* remove reset Rx FIFO count */
   sOutW((WordIO_t)ChP->IndexAddr,ChP->RxFIFOPtrs); /* clear Rx out ptr */
   sOutW(ChP->IndexData,0);
   sOutW((WordIO_t)ChP->IndexAddr,ChP->RxFIFOPtrs + 2); /* clear Rx in ptr */
   sOutW(ChP->IndexData,0);
   if(RxFIFOEnabled)
      sEnRxFIFO(ChP);                  /* enable Rx FIFO */
}

/***************************************************************************
Function: sFlushTxFIFO
Purpose:  Flush the Tx FIFO
Call:     sFlushTxFIFO(ChP)
          CHANNEL_T *ChP; Ptr to channel structure
Return:   void
Comments: To prevent data from being enqueued or dequeued in the Tx FIFO
          while it is being flushed the receive processor is stopped
          and the transmitter is disabled.  After these operations a
          4 uS delay is done before clearing the pointers to allow
          the receive processor to stop.  These items are handled inside
          this function.
Warnings: No context switches are allowed while executing this function.
*/
void sFlushTxFIFO(CHANNEL_T *ChP)
{
   int i;
   Byte_t Ch;                   /* channel number within AIOP */
   int TxEnabled;                      /* TRUE if transmitter enabled */

   if(sGetTxCnt(ChP) == 0)             /* Tx FIFO empty */
      return;                          /* don't need to flush */

   TxEnabled = FALSE;
   if(ChP->TxControl[3] & TX_ENABLE)
   {
      TxEnabled = TRUE;
      sDisTransmit(ChP);               /* disable transmitter */
   }
   sStopRxProcessor(ChP);              /* stop Rx processor */
   for(i = 0; i < 4000/200; i++)         /* delay 4 uS to allow proc to stop */
      sInB(ChP->IntChan);	/* depends on bus i/o timing */
   Ch = (Byte_t)sGetChanNum(ChP);
   sOutB(ChP->Cmd,Ch | RESTXFCNT);     /* apply reset Tx FIFO count */
   sOutB(ChP->Cmd,Ch);                 /* remove reset Tx FIFO count */
   sOutW((WordIO_t)ChP->IndexAddr,ChP->TxFIFOPtrs); /* clear Tx in/out ptrs */
   sOutW(ChP->IndexData,0);
   if(TxEnabled)
      sEnTransmit(ChP);                /* enable transmitter */
   sStartRxProcessor(ChP);             /* restart Rx processor */
}

/***************************************************************************
Function: sWriteTxPrioByte
Purpose:  Write a byte of priority transmit data to a channel
Call:     sWriteTxPrioByte(ChP,Data)
          CHANNEL_T *ChP; Ptr to channel structure
          Byte_t Data; The transmit data byte

Return:   int: 1 if the bytes is successfully written, otherwise 0.

Comments: The priority byte is transmitted before any data in the Tx FIFO.

Warnings: No context switches are allowed while executing this function.
*/
int sWriteTxPrioByte(CHANNEL_T *ChP, Byte_t Data)
{
   Byte_t DWBuf[4];             /* buffer for double word writes */
   Word_t *WordPtr;          /* must be far because Win SS != DS */
   register DWordIO_t IndexAddr;

   if(sGetTxCnt(ChP) > 1)              /* write it to Tx priority buffer */
   {
      IndexAddr = ChP->IndexAddr;
      sOutW((WordIO_t)IndexAddr,ChP->TxPrioCnt); /* get priority buffer status */
      if(sInB((ByteIO_t)ChP->IndexData) & PRI_PEND) /* priority buffer busy */
         return(0);                    /* nothing sent */

      WordPtr = (Word_t *)(&DWBuf[0]);
      *WordPtr = ChP->TxPrioBuf;       /* data byte address */

      DWBuf[2] = Data;                 /* data byte value */
      sOutDW(IndexAddr,*((DWord_t *)(&DWBuf[0]))); /* write it out */

      *WordPtr = ChP->TxPrioCnt;       /* Tx priority count address */

      DWBuf[2] = PRI_PEND + 1;         /* indicate 1 byte pending */
      DWBuf[3] = 0;                    /* priority buffer pointer */
      sOutDW(IndexAddr,*((DWord_t *)(&DWBuf[0]))); /* write it out */
   }
   else                                /* write it to Tx FIFO */
   {
      sWriteTxByte(sGetTxRxDataIO(ChP),Data);
   }
   return(1);                          /* 1 byte sent */
}

/***************************************************************************
Function: sEnInterrupts
Purpose:  Enable one or more interrupts for a channel
Call:     sEnInterrupts(ChP,Flags)
          CHANNEL_T *ChP; Ptr to channel structure
          Word_t Flags: Interrupt enable flags, can be any combination
             of the following flags:
                TXINT_EN:   Interrupt on Tx FIFO empty
                RXINT_EN:   Interrupt on Rx FIFO at trigger level (see
                            sSetRxTrigger())
                SRCINT_EN:  Interrupt on SRC (Special Rx Condition)
                MCINT_EN:   Interrupt on modem input change
                CHANINT_EN: Allow channel interrupt signal to the AIOP's
                            Interrupt Channel Register.
Return:   void
Comments: If an interrupt enable flag is set in Flags, that interrupt will be
          enabled.  If an interrupt enable flag is not set in Flags, that
          interrupt will not be changed.  Interrupts can be disabled with
          function sDisInterrupts().

          This function sets the appropriate bit for the channel in the AIOP's
          Interrupt Mask Register if the CHANINT_EN flag is set.  This allows
          this channel's bit to be set in the AIOP's Interrupt Channel Register.

          Interrupts must also be globally enabled before channel interrupts
          will be passed on to the host.  This is done with function
          sEnGlobalInt().

          In some cases it may be desirable to disable interrupts globally but
          enable channel interrupts.  This would allow the global interrupt
          status register to be used to determine which AIOPs need service.
*/
void sEnInterrupts(CHANNEL_T *ChP,Word_t Flags)
{
   Byte_t Mask;                 /* Interrupt Mask Register */

   ChP->RxControl[2] |=
      ((Byte_t)Flags & (RXINT_EN | SRCINT_EN | MCINT_EN));

   sOutDW(ChP->IndexAddr,*(DWord_t *)&ChP->RxControl[0]);

   ChP->TxControl[2] |= ((Byte_t)Flags & TXINT_EN);

   sOutDW(ChP->IndexAddr,*(DWord_t *)&ChP->TxControl[0]);

   if(Flags & CHANINT_EN)
   {
      Mask = sInB(ChP->IntMask) | sBitMapSetTbl[ChP->ChanNum];
      sOutB(ChP->IntMask,Mask);
   }
}

/***************************************************************************
Function: sDisInterrupts
Purpose:  Disable one or more interrupts for a channel
Call:     sDisInterrupts(ChP,Flags)
          CHANNEL_T *ChP; Ptr to channel structure
          Word_t Flags: Interrupt flags, can be any combination
             of the following flags:
                TXINT_EN:   Interrupt on Tx FIFO empty
                RXINT_EN:   Interrupt on Rx FIFO at trigger level (see
                            sSetRxTrigger())
                SRCINT_EN:  Interrupt on SRC (Special Rx Condition)
                MCINT_EN:   Interrupt on modem input change
                CHANINT_EN: Disable channel interrupt signal to the
                            AIOP's Interrupt Channel Register.
Return:   void
Comments: If an interrupt flag is set in Flags, that interrupt will be
          disabled.  If an interrupt flag is not set in Flags, that
          interrupt will not be changed.  Interrupts can be enabled with
          function sEnInterrupts().

          This function clears the appropriate bit for the channel in the AIOP's
          Interrupt Mask Register if the CHANINT_EN flag is set.  This blocks
          this channel's bit from being set in the AIOP's Interrupt Channel
          Register.
*/
void sDisInterrupts(CHANNEL_T *ChP,Word_t Flags)
{
   Byte_t Mask;                 /* Interrupt Mask Register */

   ChP->RxControl[2] &=
         ~((Byte_t)Flags & (RXINT_EN | SRCINT_EN | MCINT_EN));
   sOutDW(ChP->IndexAddr,*(DWord_t *)&ChP->RxControl[0]);
   ChP->TxControl[2] &= ~((Byte_t)Flags & TXINT_EN);
   sOutDW(ChP->IndexAddr,*(DWord_t *)&ChP->TxControl[0]);

   if(Flags & CHANINT_EN)
   {
      Mask = sInB(ChP->IntMask) & sBitMapClrTbl[ChP->ChanNum];
      sOutB(ChP->IntMask,Mask);
   }
}
