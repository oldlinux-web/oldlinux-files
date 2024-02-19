/*
**	constants used for configuring amoeba transactions
*/
#define MINLOCCNT	   5	/* locate message sent every dsec */
#define MAXLOCCNT	 100	/* locate message sent every MAXLOCCNT dsec */

#define RETRANSTIME	   5	/* retransmission time in dsec */
#define CRASHTIME	 100	/* crash timer in dsec */
#define CLIENTCRASH	 500	/* client must probe within this time */

#define MAXRETRANS	  10	/* max. number of transmissions */
#define MINCRASH	   5	/* enquiry sent MINCRASH times during recv */
#define MAXCRASH	  10	/* enquiry sent MAXCRASH times during serv */

#define	NPORTS		  16	/* # ports in portcache */
