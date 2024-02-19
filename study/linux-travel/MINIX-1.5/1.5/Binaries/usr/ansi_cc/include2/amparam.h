/*
**	parameters for trans, getreq and putrep are given to the kernel
**	in a Trpar struct
*/

typedef struct
{
	header *	p_hdr;	/* header pointer */
	bufptr		p_buf;	/* buffer pointer */
	unshort		p_cnt;	/* character count */
} Param;

typedef struct
{
	Param	tp_par[2];
	unshort tp_maxloc;	/* trans locate timeout */
} Trpar;
