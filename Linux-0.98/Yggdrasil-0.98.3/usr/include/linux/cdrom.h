/****************************************************************************************
 *											*
 * SCSI header library for linux							*
 * (C) 1992 David Giller rafetmad@oxy.edu						*
 *											*
 * <linux/cdrom.h> -- CD-ROM IOCTLs and structs		 				*
 *											*
 ****************************************************************************************/

#ifndef	_LINUX_CDROM_H
#define	_LINUX_CDROM_H

/*
 *
 * For IOCTL calls, we will commandeer byte 0x53, or 'S'.
 *
 */

/*
 * CD-ROM-specific SCSI command opcodes
 */

/*
 * Group 2 (10-byte).  All of these are called 'optional' by SCSI-II.
 */

#define	SCMD_READ_TOC		0x43		/* read table of contents	*/
#define	SCMD_PLAYAUDIO_MSF	0x47		/* play data at time offset	*/
#define	SCMD_PLAYAUDIO_TI	0x48		/* play data at track/index	*/
#define	SCMD_PAUSE_RESUME	0x4B		/* pause/resume audio		*/
#define	SCMD_READ_SUBCHANNEL	0x42		/* read SC info on playing disc */
#define	SCMD_PLAYAUDIO10	0x45		/* play data at logical block	*/
#define	SCMD_READ_HEADER	0x44		/* read TOC header		*/

/*
 * Group 5
 */

#define	SCMD_PLAYAUDIO12	0xA5 		/* play data at logical block	*/
#define	SCMD_PLAYTRACK_REL12	0xA9		/* play track at relative offset*/

/*
 * Group 6 Commands
 */

#define	SCMD_CD_PLAYBACK_CONTROL 0xC9		/* Sony vendor-specific audio	*/
#define	SCMD_CD_PLAYBACK_STATUS 0xC4		/* control opcodes. info please!*/

/*
 * CD-ROM capacity structure.
 */

struct scsi_capacity 
	{
	u_long	capacity;
	u_long	lbasize;
	};

/*
 * CD-ROM MODE_SENSE/MODE_SELECT parameters
 */

#define	ERR_RECOVERY_PARMS	0x01
#define	DISCO_RECO_PARMS	0x02
#define	FORMAT_PARMS		0x03
#define	GEOMETRY_PARMS		0x04
#define	CERTIFICATION_PARMS	0x06
#define	CACHE_PARMS		0x38

/*
 * standard mode-select header prepended to all mode-select commands
 */

struct ccs_modesel_head 
	{
	u_char	_r1;			/* reserved				*/
	u_char	medium;			/* device-specific medium type 		*/
	u_char 	_r2;			/* reserved				*/
	u_char	block_desc_length;	/* block descriptor length 		*/
	u_char	density;		/* device-specific density code		*/
	u_char	number_blocks_hi;	/* number of blocks in this block desc	*/
	u_char	number_blocks_med;
	u_char	number_blocks_lo;
	u_char	_r3;
	u_char	block_length_hi;	/* block length for blocks in this desc	*/
	u_short	block_length;
	};

/*
 * error recovery parameters
 */

struct ccs_err_recovery 
	{
	u_char	_r1		: 2;	/* reserved 				*/
	u_char	page_code	: 6;	/* page code 				*/
	u_char	page_length;		/* page length		 		*/
	u_char	awre		: 1;	/* auto write realloc enabled 		*/
	u_char	arre		: 1;	/* auto read realloc enabled 		*/
	u_char	tb		: 1;	/* transfer block 			*/
	u_char 	rc		: 1;	/* read continuous 			*/
	u_char	eec		: 1;	/* enable early correction 		*/
	u_char	per		: 1;	/* post error 				*/
	u_char	dte		: 1;	/* disable transfer on error 		*/
	u_char	dcr		: 1;	/* disable correction 			*/
	u_char	retry_count;		/* error retry count			*/
	u_char	correction_span;	/* largest recov. to be attempted, bits	*/
	u_char	head_offset_count;	/* head offset (2's C) for each retry	*/
	u_char	strobe_offset_count;	/* data strobe "			*/
	u_char	recovery_time_limit;	/* time limit on recovery attempts	*/
};

/*
 * disco/reco parameters
 */

struct ccs_disco_reco 
	{
	u_char	_r1		: 2;	/* reserved				*/
	u_char	page_code	: 6;	/* page code		 		*/
	u_char	page_length;		/* page length				*/
	u_char	buffer_full_ratio;	/* write buffer reconnect threshold	*/
	u_char	buffer_empty_ratio;	/* read "				*/
	u_short	bus_inactivity_limit;	/* limit on bus inactivity time		*/
	u_short	disconnect_time_limit;	/* minimum disconnect time		*/
	u_short	connect_time_limit;	/* minimum connect time			*/
	u_short	_r2;			/* reserved				*/
};

/*
 * drive geometry parameters
 */

struct ccs_geometry 
	{
	u_char	_r1		: 2;	/* reserved				*/
	u_char	page_code	: 6;	/* page code				*/
	u_char	page_length;		/* page length				*/
	u_char	cyl_ub;			/* #cyls				*/
	u_char	cyl_mb;
	u_char	cyl_lb;
	u_char	heads;			/* #heads				*/
	u_char	precomp_cyl_ub;		/* precomp start			*/
	u_char	precomp_cyl_mb;
	u_char	precomp_cyl_lb;
	u_char	current_cyl_ub;		/* reduced current start		*/
	u_char	current_cyl_mb;
	u_char	current_cyl_lb;
	u_short	step_rate;		/* stepping motor rate			*/
	u_char	landing_cyl_ub;		/* landing zone				*/
	u_char	landing_cyl_mb;
	u_char	landing_cyl_lb;
	u_char  _r2;
	u_char	_r3;
	u_char	_r4;
	};

/*
 * cache parameters
 */

struct ccs_cache 
	{
	u_char	_r1		: 2;	/* reserved				*/
	u_char	page_code	: 6;	/* page code				*/
	u_char	page_length;		/* page length				*/
	u_char	mode;			/* cache control byte			*/
	u_char	threshold;		/* prefetch threshold			*/
	u_char	max_prefetch;		/* maximum prefetch size		*/
	u_char	max_multiplier;		/* maximum prefetch multiplier 		*/
	u_char	min_prefetch;		/* minimum prefetch size		*/
	u_char	min_multiplier;		/* minimum prefetch multiplier 		*/
	u_char	_r2[8];
	};

/*
 * CDROM IOCTL structures
 */

struct cdrom_msf 
	{
	u_char	cdmsf_min0;		/* start minute 			*/
	u_char	cdmsf_sec0;		/* start second 			*/
	u_char	cdmsf_frame0;		/* start frame  			*/
	u_char	cdmsf_min1;		/* end minute   			*/
	u_char	cdmsf_sec1;		/* end second   			*/
	u_char	cdmsf_frame1;		/* end frame	   			*/
	};

struct cdrom_ti 
	{
	u_char	cdti_trk0;		/* start track 				*/
	u_char	cdti_ind0;		/* start index 				*/
	u_char	cdti_trk1;		/* end track 				*/
	u_char	cdti_ind1;		/* end index 				*/
	};

struct cdrom_tochdr 	
	{
	u_char	cdth_trk0;		/* start track 				*/
	u_char	cdth_trk1;		/* end track 				*/
	};

struct cdrom_tocentry 
	{
	u_char	cdte_track;
	u_char	cdte_adr	:4;
	u_char	cdte_ctrl	:4;
	u_char	cdte_format;
	union
		{
		struct
			{
			u_char	minute;
			u_char	second;
			u_char	frame;
			} msf;
		int	lba;
		} cdte_addr;
	u_char	cdte_datamode;
	};

/*
 * CD-ROM address types (cdrom_tocentry.cdte_format)
 */

#define	CDROM_LBA	0x01
#define	CDROM_MSF	0x02

/*
 * bit to tell whether track is data or audio
 */

#define	CDROM_DATA_TRACK	0x04

/*
 * The leadout track is always 0xAA, regardless of # of tracks on disc
 */

#define	CDROM_LEADOUT	0xAA

struct cdrom_subchnl 
	{
	u_char	cdsc_format;
	u_char	cdsc_audiostatus;
	u_char	cdsc_adr:	4;
	u_char	cdsc_ctrl:	4;
	u_char	cdsc_trk;
	u_char	cdsc_ind;
	union
		{
		struct 			
			{
			u_char	minute;
			u_char	second;
			u_char	frame;
			} msf;
		int	lba;
		} cdsc_absaddr;
	union 
		{
		struct 
			{
			u_char	minute;
			u_char	second;
			u_char	frame;
			} msf;
		int	lba;
		} cdsc_reladdr;
	};

/*
 * return value from READ SUBCHANNEL DATA
 */

#define	CDROM_AUDIO_INVALID	0x00	/* audio status not supported 		*/
#define	CDROM_AUDIO_PLAY	0x11	/* audio play operation in progress 	*/
#define	CDROM_AUDIO_PAUSED	0x12	/* audio play operation paused 		*/
#define	CDROM_AUDIO_COMPLETED	0x13	/* audio play successfully completed 	*/
#define	CDROM_AUDIO_ERROR	0x14	/* audio play stopped due to error 	*/
#define	CDROM_AUDIO_NO_STATUS	0x15	/* no current audio status to return 	*/

struct cdrom_volctrl
	{
	u_char	channel0;
	u_char	channel1;
	u_char	channel2;
	u_char	channel3;
	};

struct cdrom_read      
	{
	int	cdread_lba;
	caddr_t	cdread_bufaddr;
	int	cdread_buflen;
	};

#ifdef FIVETWELVE
#define	CDROM_MODE1_SIZE	512
#else
#define	CDROM_MODE1_SIZE	2048
#endif FIVETWELVE
#define	CDROM_MODE2_SIZE	2336

/*
 * CD-ROM IOCTL commands
 */

#define	CDROMPAUSE		0x5301		/* pause			*/
#define	CDROMRESUME		0x5302		/* resume			*/

#define	CDROMPLAYMSF		0x5303		/* (stuct cdrom_msf)		*/
                                                /* SCMD_PLAY_AUDIO_MSF		*/

#define	CDROMPLAYTRKIND		0x5304		/* (struct cdrom_ti)		*/
                                                /* SCMD_PLAY_AUDIO_TI		*/

#define	CDROMREADTOCHDR		0x5305		/* (struct cdrom_tochdr)	*/
                                                /* read the TOC header		*/
#define	CDROMREADTOCENTRY	0x5306		/* (struct cdrom_tocentry)	*/
                                                /* read a TOC entry		*/

#define	CDROMSTOP		0x5307		/* stop the drive motor		*/
#define	CDROMSTART		0x5308		/* turn the motor on 		*/

#define	CDROMEJECT		0x5309		/* eject CD-ROM media		*/

#define	CDROMVOLCTRL		0x530a		/* (struct cdrom_volctrl)	*/
                                                /* vlume control		*/

#define	CDROMSUBCHNL		0x530b		/* (struct cdrom_subchnl)	*/
                                                /* read sub-channel data 	*/

#define	CDROMREADMODE2		0x530c		/* (struct cdrom_read)		*/
                                                /* read type-2 data (not suppt)	*/

#define	CDROMREADMODE1		0x530d		/* (struct cdrom_read)		*/
                                                /* read type-1 data 		*/

#endif  _LINUX_CDROM_H
