.fp 5 CW LucidaT   .\" To use a font other than Lucida, change 'LucidaT'
.po .9i
.lg 0
.nf
.ec `
.ps 7
.vs 9
.lt 5.25i
`f5
.nr Tb `w'0'
.nr Fp 0
.ta 9u*`n(Tbu 17u*`n(Tbu 25u*`n(Tbu 33u*`n(Tbu 41u*`n(Tbu 49u*`n(Tbu 57u*`n(Tbu 65u*`n(Tbu 73u*`n(Tbu 81u*`n(Tbu
.de Op 
.if ``n(Fp>0 .bp
.nr Fp 1
.sp 0.75i
.tl '``fR``s10MINIX SOURCE CODE``s0'``s11File: ``$2``s0``fP'``fB``s12``n%``s0``fP'
.sp 0.25i
..
.de Ep 
.if ``n(Fp>0 .bp
.sp 0.75i
.tl '``fB``s12``n%``s0``fP``fR'``s11File: ``$2'``s0``s10MINIX SOURCE CODE``s0``fP'
.nr Fp 1
.sp 0.25i
..
.Op 1 src/lib/ansi/ext_fmt.h

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/ext_fmt.h	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

00000	struct mantissa {
00001	        unsigned long h_32;
00002	        unsigned long l_32;
00003	};
00004	
00005	struct EXTEND {
00006	        short   sign;
00007	        short   exp;
00008	        struct mantissa mantissa;
00009	#define m1 mantissa.h_32
00010	#define m2 mantissa.l_32
00011	};
00012	        



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/loc_time.h	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

00100	/*
00101	 * loc_time.h - some local definitions
00102	 */
00103	/* $Header: loc_time.h,v 1.1 91/04/22 13:19:51 ceriel Exp $ */
00104	
00105	#define YEAR0           1900                    /* the first year */
00106	#define EPOCH_YR        1970            /* EPOCH = Jan 1 1970 00:00:00 */
00107	#define SECS_DAY        (24L * 60L * 60L)
00108	#define LEAPYEAR(year)  (!((year) % 4) && (((year) % 100) || !((year) % 400)))
00109	#define YEARSIZE(year)  (LEAPYEAR(year) ? 366 : 365)
00110	#define FIRSTSUNDAY(timp)       (((timp)->tm_yday - (timp)->tm_wday + 420) % 7)
00111	#define FIRSTDAYOF(timp)        (((timp)->tm_wday - (timp)->tm_yday + 420) % 7)
00112	#define TIME_MAX        ULONG_MAX
00113	#define ABB_LEN         3
00114	
00115	extern const int _ytab[2][12];
00116	extern const char *_days[];
00117	extern const char *_months[];
00118	
00119	void _tzset(void);
00120	unsigned _dstget(struct tm *timep);
00121	
00122	extern long _timezone;
00123	extern long _dst_off;
00124	extern int _daylight;
00125	extern char *_tzname[2];
.Ep 2 src/lib/ansi/abort.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/abort.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

00200	/*
00201	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
00202	 * See the copyright notice in the ACK home directory, in the file "Copyright".
00203	 */
00204	/* $Header: abort.c,v 1.3 90/11/22 13:59:37 eck Exp $ */
00205	
00206	#if     defined(_POSIX_SOURCE)
00207	#include        <sys/types.h>
00208	#endif
00209	#include        <signal.h>
00210	#include        <stdlib.h>
00211	
00212	extern void (*_clean)(void);
00213	
00214	void
00215	abort(void)
00216	{
00217	        if (_clean) _clean();           /* flush all output files */
00218	        raise(SIGABRT);
00219	}
	





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/abs.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

00300	/*
00301	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
00302	 * See the copyright notice in the ACK home directory, in the file "Copyright".
00303	 */
00304	/* $Header: abs.c,v 1.1 89/05/16 13:06:59 eck Exp $ */
00305	
00306	#include        <stdlib.h>
00307	
00308	int
00309	abs(register int i)
00310	{
00311	        return i >= 0 ? i : -i;
00312	}
.Op 3 src/lib/ansi/asctime.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/asctime.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

00400	/*
00401	 * asctime - print a date
00402	 */
00403	/* $Header: asctime.c,v 1.3 91/04/22 13:20:15 ceriel Exp $ */
00404	
00405	#include        <string.h>
00406	#include        <time.h>
00407	#include        "loc_time.h"
00408	
00409	#define DATE_STR        "??? ??? ?? ??:??:?? ????\n"
00410	
00411	static char *
00412	two_digits(register char *pb, int i, int nospace)
00413	{
00414	        *pb = (i / 10) % 10 + '0';
00415	        if (!nospace && *pb == '0') *pb = ' ';
00416	        pb++;
00417	        *pb++ = (i % 10) + '0';
00418	        return ++pb;
00419	}
	
00421	static char *
00422	four_digits(register char *pb, int i)
00423	{
00424	        i %= 10000;
00425	        *pb++ = (i / 1000) + '0';
00426	        i %= 1000;
00427	        *pb++ = (i / 100) + '0';
00428	        i %= 100;
00429	        *pb++ = (i / 10) + '0';
00430	        *pb++ = (i % 10) + '0';
00431	        return ++pb;
00432	}
	
00434	char *asctime(const struct tm *timeptr)
00435	{
00436	        static char buf[26];
00437	        register char *pb = buf;
00438	        register const char *ps;
00439	        register int n;
00440	
00441	        strcpy(pb, DATE_STR);
00442	        ps = _days[timeptr->tm_wday];
00443	        n = ABB_LEN;
00444	        while(--n >= 0) *pb++ = *ps++;
00445	        pb++;
00446	        ps = _months[timeptr->tm_mon];
00447	        n = ABB_LEN;
00448	        while(--n >= 0) *pb++ = *ps++;
00449	        pb++;
00450	        pb = two_digits(
00451	                    two_digits(
00452	                            two_digits(two_digits(pb, timeptr->tm_mday, 0)
00453	                                        , timeptr->tm_hour, 1)
00454	                            , timeptr->tm_min, 1)
.Ep 4 src/lib/ansi/asctime.c
00455	                    , timeptr->tm_sec, 1);
00456	
00457	        four_digits(pb, timeptr->tm_year + 1900);
00458	        return buf;
00459	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/assert.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

00500	/*
00501	 * assert.c - diagnostics
00502	 */
00503	/* $Header: assert.c,v 1.3 90/04/03 15:01:58 eck Exp $ */
00504	
00505	#include        <assert.h>
00506	#include        <stdio.h>
00507	#include        <stdlib.h>
00508	
00509	void __bad_assertion(const char *mess) {
00510	
00511	        fputs(mess, stderr);
00512	        abort();
00513	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/atexit.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

00600	/* $Header: atexit.c,v 1.1 89/12/18 15:11:09 eck Exp $ */
00601	
00602	#include        <stdlib.h>
00603	
00604	#define NEXITS  32
00605	
00606	extern void (*__functab[NEXITS])(void);
00607	extern int __funccnt;
00608	
00609	int
00610	atexit(void (*func)(void))
00611	{
00612	        if (__funccnt >= NEXITS)
00613	                return 1;
00614	        __functab[__funccnt++] = func;
00615	        return 0;
00616	}
.Op 5 src/lib/ansi/atof.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/atof.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

00700	/*
00701	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
00702	 * See the copyright notice in the ACK home directory, in the file "Copyright".
00703	 */
00704	/* $Header: atof.c,v 1.2 89/12/18 15:11:50 eck Exp $ */
00705	
00706	#include        <stdlib.h>
00707	#include        <errno.h>
00708	
00709	double
00710	atof(const char *nptr)
00711	{
00712	        double d;
00713	        int e = errno;
00714	
00715	        d = strtod(nptr, (char **) NULL);
00716	        errno = e;
00717	        return d;
00718	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/atoi.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

00800	/*
00801	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
00802	 * See the copyright notice in the ACK home directory, in the file "Copyright".
00803	 */
00804	/* $Header: atoi.c,v 1.4 90/05/22 12:22:25 ceriel Exp $ */
00805	
00806	#include        <ctype.h>
00807	#include        <stdlib.h>
00808	
00809	/* We do not use strtol here for backwards compatibility in behaviour on
00810	   overflow.
00811	*/
00812	int
00813	atoi(register const char *nptr)
00814	{
00815	        int total = 0;
00816	        int minus = 0;
00817	
00818	        while (isspace(*nptr)) nptr++;
00819	        if (*nptr == '+') nptr++;
00820	        else if (*nptr == '-') {
00821	                minus = 1;
00822	                nptr++;
00823	        }
00824	        while (isdigit(*nptr)) {
00825	                total *= 10;
00826	                total += (*nptr++ - '0');
00827	        }
00828	        return minus ? -total : total;
00829	}
.Ep 6 src/lib/ansi/atol.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/atol.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

00900	/*
00901	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
00902	 * See the copyright notice in the ACK home directory, in the file "Copyright".
00903	 */
00904	/* $Header: atol.c,v 1.3 90/05/22 10:48:12 ceriel Exp $ */
00905	
00906	#include        <ctype.h>
00907	#include        <stdlib.h>
00908	
00909	/* We do not use strtol here for backwards compatibility in behaviour on
00910	   overflow.
00911	*/
00912	long
00913	atol(register const char *nptr)
00914	{
00915	        long total = 0;
00916	        int minus = 0;
00917	
00918	        while (isspace(*nptr)) nptr++;
00919	        if (*nptr == '+') nptr++;
00920	        else if (*nptr == '-') {
00921	                minus = 1;
00922	                nptr++;
00923	        }
00924	        while (isdigit(*nptr)) {
00925	                total *= 10;
00926	                total += (*nptr++ - '0');
00927	        }
00928	        return minus ? -total : total;
00929	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/bsearch.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

01000	/*
01001	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
01002	 * See the copyright notice in the ACK home directory, in the file "Copyright".
01003	 */
01004	/* $Header: bsearch.c,v 1.2 89/12/18 15:12:21 eck Exp $ */
01005	
01006	#include        <stdlib.h>
01007	
01008	void *
01009	bsearch(register const void *key, register const void *base,
01010	        register size_t nmemb, register size_t size,
01011	        int (*compar)(const void *, const void *))
01012	{
01013	        register const void *mid_point;
01014	        register int  cmp;
.Op 7 src/lib/ansi/bsearch.c
01015	
01016	        while (nmemb > 0) {
01017	                mid_point = (char *)base + size * (nmemb >> 1);
01018	                if ((cmp = (*compar)(key, mid_point)) == 0)
01019	                        return (void *)mid_point;
01020	                if (cmp >= 0) {
01021	                        base  = (char *)mid_point + size;
01022	                        nmemb = (nmemb - 1) >> 1;
01023	                } else
01024	                        nmemb >>= 1;
01025	        }
01026	        return (void *)NULL;
01027	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/calloc.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

01100	/* $Header$ */
01101	#include        <stdlib.h>
01102	
01103	#define ALIGN(x)        (((x) + (sizeof(size_t) - 1)) & ~(sizeof(size_t) - 1))
01104	
01105	void *
01106	calloc(size_t nelem, size_t elsize)
01107	{
01108	        register char *p;
01109	        register size_t *q;
01110	        size_t size = ALIGN(nelem * elsize);
01111	
01112	        p = malloc(size);
01113	        if (p == NULL) return NULL;
01114	        q = (size_t *) (p + size);
01115	        while ((char *) q > p) *--q = 0;
01116	        return p;
01117	}
	


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/chartab.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

01200	#include        <ctype.h>
01201	
01202	char __ctype[] = {
01203	0,
01204	_C,
01205	_C,
01206	_C,
01207	_C,
01208	_C,
01209	_C,
01210	_C,
01211	_C,
01212	_C,
01213	_C|_S,
01214	_C|_S,
.Ep 8 src/lib/ansi/chartab.c
01215	_C|_S,
01216	_C|_S,
01217	_C|_S,
01218	_C,
01219	_C,
01220	_C,
01221	_C,
01222	_C,
01223	_C,
01224	_C,
01225	_C,
01226	_C,
01227	_C,
01228	_C,
01229	_C,
01230	_C,
01231	_C,
01232	_C,
01233	_C,
01234	_C,
01235	_C,
01236	_S,
01237	_P,
01238	_P,
01239	_P,
01240	_P,
01241	_P,
01242	_P,
01243	_P,
01244	_P,
01245	_P,
01246	_P,
01247	_P,
01248	_P,
01249	_P,
01250	_P,
01251	_P,
01252	_N,
01253	_N,
01254	_N,
01255	_N,
01256	_N,
01257	_N,
01258	_N,
01259	_N,
01260	_N,
01261	_N,
01262	_P,
01263	_P,
01264	_P,
01265	_P,
01266	_P,
01267	_P,
01268	_P,
01269	_U|_X,
01270	_U|_X,
01271	_U|_X,
01272	_U|_X,
01273	_U|_X,
01274	_U|_X,
.Op 9 src/lib/ansi/chartab.c
01275	_U,
01276	_U,
01277	_U,
01278	_U,
01279	_U,
01280	_U,
01281	_U,
01282	_U,
01283	_U,
01284	_U,
01285	_U,
01286	_U,
01287	_U,
01288	_U,
01289	_U,
01290	_U,
01291	_U,
01292	_U,
01293	_U,
01294	_U,
01295	_P,
01296	_P,
01297	_P,
01298	_P,
01299	_P,
01300	_P,
01301	_L|_X,
01302	_L|_X,
01303	_L|_X,
01304	_L|_X,
01305	_L|_X,
01306	_L|_X,
01307	_L,
01308	_L,
01309	_L,
01310	_L,
01311	_L,
01312	_L,
01313	_L,
01314	_L,
01315	_L,
01316	_L,
01317	_L,
01318	_L,
01319	_L,
01320	_L,
01321	_L,
01322	_L,
01323	_L,
01324	_L,
01325	_L,
01326	_L,
01327	_P,
01328	_P,
01329	_P,
01330	_P,
01331	_C,
01332	0,
01333	0,
01334	0,
.Ep 10 src/lib/ansi/chartab.c
01335	0,
01336	0,
01337	0,
01338	0,
01339	0,
01340	0,
01341	0,
01342	0,
01343	0,
01344	0,
01345	0,
01346	0,
01347	0,
01348	0,
01349	0,
01350	0,
01351	0,
01352	0,
01353	0,
01354	0,
01355	0,
01356	0,
01357	0,
01358	0,
01359	0,
01360	0,
01361	0,
01362	0,
01363	0,
01364	0,
01365	0,
01366	0,
01367	0,
01368	0,
01369	0,
01370	0,
01371	0,
01372	0,
01373	0,
01374	0,
01375	0,
01376	0,
01377	0,
01378	0,
01379	0,
01380	0,
01381	0,
01382	0,
01383	0,
01384	0,
01385	0,
01386	0,
01387	0,
01388	0,
01389	0,
01390	0,
01391	0,
01392	0,
01393	0,
01394	0,
.Op 11 src/lib/ansi/chartab.c
01395	0,
01396	0,
01397	0,
01398	0,
01399	0,
01400	0,
01401	0,
01402	0,
01403	0,
01404	0,
01405	0,
01406	0,
01407	0,
01408	0,
01409	0,
01410	0,
01411	0,
01412	0,
01413	0,
01414	0,
01415	0,
01416	0,
01417	0,
01418	0,
01419	0,
01420	0,
01421	0,
01422	0,
01423	0,
01424	0,
01425	0,
01426	0,
01427	0,
01428	0,
01429	0,
01430	0,
01431	0,
01432	0,
01433	0,
01434	0,
01435	0,
01436	0,
01437	0,
01438	0,
01439	0,
01440	0,
01441	0,
01442	0,
01443	0,
01444	0,
01445	0,
01446	0,
01447	0,
01448	0,
01449	0,
01450	0,
01451	0,
01452	0,
01453	0,
01454	0,
.Ep 12 src/lib/ansi/chartab.c
01455	0,
01456	0,
01457	0,
01458	0,
01459	0,
01460	};





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/clock.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

01500	/*
01501	 * clock - determine the processor time used
01502	 */
01503	/* $Header: clock.c,v 1.3 90/01/22 13:08:11 eck Exp $ */
01504	
01505	#include        <time.h>
01506	
01507	#if     defined(__BSD4_2)
01508	
01509	struct timeval {
01510	        long    tv_sec;                 /* seconds */
01511	        long    tv_usec;                /* and microseconds */
01512	};
01513	
01514	#define RUSAGE_SELF     0
01515	#define RUSAGE_CHILDREN -1
01516	
01517	struct rusage {
01518	        struct timeval ru_utime;        /* user time used */
01519	        struct timeval ru_stime;        /* system time used */
01520	        long    ru_maxrss;
01521	        long    ru_ixrss;               /* integral shared memory size */
01522	        long    ru_idrss;               /* integral unshared data size */
01523	        long    ru_isrss;               /* integral unshared stack size */
01524	        long    ru_minflt;              /* page reclaims */
01525	        long    ru_majflt;              /* page faults */
01526	        long    ru_nswap;               /* swaps */
01527	        long    ru_inblock;             /* block input operations */
01528	        long    ru_oublock;             /* block output operations */
01529	        long    ru_msgsnd;              /* messages sent */
01530	        long    ru_msgrcv;              /* messages received */
01531	        long    ru_nsignals;            /* signals received */
01532	        long    ru_nvcsw;               /* voluntary context switches */
01533	        long    ru_nivcsw;              /* involuntary context switches */
01534	};
01535	
01536	void _getrusage(int who, struct rusage *rusage);
01537	
01538	#elif   defined(_POSIX_SOURCE) || defined(__USG)
01539	
01540	struct tms {
01541	        time_t  tms_utime;              /* user time */
01542	        time_t  tms_stime;              /* system time */
01543	        time_t  tms_cutime;             /* user time, children */
01544	        time_t  tms_cstime;             /* system time, children */
.Op 13 src/lib/ansi/clock.c
01545	};
01546	
01547	clock_t _times(struct tms *buffer);
01548	
01549	#else                                   /* Version 7 UNIX */
01550	
01551	struct tbuffer {
01552	        long proc_user_time;
01553	        long proc_system_time;
01554	        long child_user_time;
01555	        long child_system_time;
01556	};
01557	
01558	clock_t _times(struct tbuffer *buffer);
01559	
01560	#endif
01561	
01562	clock_t
01563	clock(void)
01564	{
01565	#if     defined(__BSD4_2)
01566	        struct rusage rusage;
01567	
01568	        _getrusage(RUSAGE_SELF, &rusage);
01569	
01570	        return (((unsigned long)rusage.ru_utime.tv_sec * CLOCKS_PER_SEC)
01571	                + rusage.ru_utime.tv_usec);
01572	#elif   defined(_POSIX_SOURCE) || defined(__USG)
01573	        struct tms tms;
01574	
01575	        _times(&tms);
01576	        /* Assume that time_t can be converted to clock_t for Sys5 */
01577	        return tms.tms_utime;
01578	#else
01579	        struct tbuffer tbuffer;
01580	
01581	        _times(&tbuffer);
01582	        return tbuffer.proc_user_time;
01583	#endif
01584	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/ctime.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

01600	/*
01601	 * ctime - convers the calendar time to a string
01602	 */
01603	/* $Header: ctime.c,v 1.1 89/06/12 15:21:16 eck Exp $ */
01604	
01605	#include        <time.h>
01606	
01607	char *
01608	ctime(const time_t *timer)
01609	{
.Ep 14 src/lib/ansi/ctime.c
01610	        return asctime(localtime(timer));
01611	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/difftime.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

01700	/*
01701	 * difftime - compute the difference between two calendar times
01702	 */
01703	/* $Header: difftime.c,v 1.4 90/09/11 10:18:44 eck Exp $ */
01704	
01705	#include        <time.h>
01706	
01707	double
01708	difftime(time_t time1, time_t time0)
01709	{
01710	        /* be careful: time_t may be unsigned */
01711	        if ((time_t)-1 > 0 && time0 > time1) {
01712	                return - (double) (time0 - time1);
01713	        } else {
01714	                return (double)(time1 - time0);
01715	        }
01716	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/div.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

01800	/*
01801	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
01802	 * See the copyright notice in the ACK home directory, in the file "Copyright".
01803	 */
01804	/* $Header: div.c,v 1.3 90/03/05 13:47:53 eck Exp $ */
01805	
01806	#include        <stdlib.h>
01807	
01808	static int tmp = -1;
01809	
01810	div_t
01811	div(register int numer, register int denom)
01812	{
01813	        div_t r;
01814	
01815	        /* The assignment of tmp should not be optimized !! */
01816	        if (tmp == -1) {
01817	                tmp = (tmp / 2 == 0);
01818	        }
01819	        if (numer == 0) {
01820	                r.quot = numer / denom;         /* might trap if denom == 0 */
01821	                r.rem = numer % denom;
01822	        } else if ( !tmp && ((numer < 0) != (denom < 0))) {
01823	                r.quot = (numer / denom) + 1;
01824	                r.rem = numer - (numer / denom + 1) * denom;
.Op 15 src/lib/ansi/div.c
01825	        } else {
01826	                r.quot = numer / denom;
01827	                r.rem = numer % denom;
01828	        }
01829	        return r;
01830	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/errlist.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

01900	/*
01901	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
01902	 * See the copyright notice in the ACK home directory, in the file "Copyright".
01903	 */
01904	/* $Header: errlist.c,v 1.1 89/05/10 16:22:20 eck Exp $ */
01905	
01906	#include        <errno.h>
01907	
01908	static const char unknown[] = "Unknown error";
01909	
01910	const char *_sys_errlist[] = {
01911	        "Error 0",                      /* EGENERIC */
01912	        "Not owner",                    /* EPERM */
01913	        "No such file or directory",    /* ENOENT */
01914	        "No such process",              /* ESRCH */
01915	        "Interrupted system call",      /* EINTR */
01916	        "I/O error",                    /* EIO */
01917	        "No such device or address",    /* ENXIO */
01918	        "Arg list too long",            /* E2BIG */
01919	        "Exec format error",            /* ENOEXEC */
01920	        "Bad file number",              /* EBADF */
01921	        "No children",                  /* ECHILD */
01922	        "No more processes",            /* EAGAIN */
01923	        "Not enough core",              /* ENOMEM */
01924	        "Permission denied",            /* EACCES */
01925	        "Bad address",                  /* EFAULT */
01926	        "Block device required",        /* ENOTBLK */
01927	        "Resource busy",                /* EBUSY */
01928	        "File exists",                  /* EEXIST */
01929	        "Cross-device link",            /* EXDEV */
01930	        "No such device",               /* ENODEV */
01931	        "Not a directory",              /* ENOTDIR */
01932	        "Is a directory",               /* EISDIR */
01933	        "Invalid argument",             /* EINVAL */
01934	        "File table overflow",          /* ENFILE */
01935	        "Too many open files",          /* EMFILE */
01936	        "Not a typewriter",             /* ENOTTY */
01937	        "Text file busy",               /* ETXTBSY */
01938	        "File too large",               /* EFBIG */
01939	        "No space left on device",      /* ENOSPC */
01940	        "Illegal seek",                 /* ESPIPE */
01941	        "Read-only file system",        /* EROFS */
01942	        "Too many links",               /* EMLINK */
01943	        "Broken pipe",                  /* EPIPE */
01944	        "Math argument",                /* EDOM */
.Ep 16 src/lib/ansi/errlist.c
01945	        "Result too large",             /* ERANGE */
01946	        "Resource deadlock avoided",    /* EDEADLK */
01947	        "File name too long",           /* ENAMETOOLONG */
01948	        "No locks available",           /* ENOLCK */
01949	        "Function not implemented",     /* ENOSYS */
01950	        "Directory not empty",          /* ENOTEMPTY */
01951	        unknown,                        /* 40 */
01952	        unknown,                        /* 41 */
01953	        unknown,                        /* 42 */
01954	        unknown,                        /* 43 */
01955	        unknown,                        /* 44 */
01956	        unknown,                        /* 45 */
01957	        unknown,                        /* 46 */
01958	        unknown,                        /* 47 */
01959	        unknown,                        /* 48 */
01960	        unknown,                        /* 49 */
01961	        "Invalid packet size",          /* EPACKSIZE */
01962	        "Not enough buffers left",      /* EOUTOFBUFS */
01963	        "Illegal ioctl for device",     /* EBADIOCTL */
01964	        "Bad mode for ioctl",           /* EBADMODE */
01965	        "Would block",                  /* EWOULDBLOCK */
01966	        "Bad destination address",      /* EBADDEST */
01967	        "Destination not reachable",    /* EDSTNOTRCH */
01968	        "Already connected",            /* EISCONN */
01969	        "Address in use",               /* EADDRINUSE */
01970	        "Connection refused",           /* ECONNREFUSED */
01971	        "Connection reset",             /* ECONNRESET */
01972	        "Connection timed out",         /* ETIMEDOUT */
01973	        "Urgent data present",          /* EURG */
01974	        "No urgent data present",       /* ENOURG */
01975	        "No connection",                /* ENOTCONN */
01976	        "Already shutdown",             /* ESHUTDOWN */
01977	        "No such connection",           /* ENOCONN */
01978	};
01979	
01980	const int _sys_nerr = sizeof(_sys_errlist) / sizeof(_sys_errlist[0]);





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/exit.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

02000	/*
02001	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
02002	 * See the copyright notice in the ACK home directory, in the file "Copyright".
02003	 */
02004	/* $Header: exit.c,v 1.3 90/01/22 13:00:04 eck Exp $ */
02005	
02006	#include        <stdio.h>
02007	#include        <stdlib.h>
02008	
02009	#define NEXITS  32
02010	
02011	void (*__functab[NEXITS])(void);
02012	int __funccnt = 0;
02013	
02014	extern void _exit(int);
.Op 17 src/lib/ansi/exit.c
02015	
02016	/* only flush output buffers when necessary */
02017	int (*_clean)(void) = NULL;
02018	
02019	static void
02020	_calls(void)
02021	{
02022	        register int i = __funccnt;
02023	        
02024	        /* "Called in reversed order of their registration" */
02025	        while (--i >= 0)
02026	                (*__functab[i])();
02027	}
	
02029	void
02030	exit(int status)
02031	{
02032	        _calls();
02033	        if (_clean) _clean();
02034	        _exit(status) ;
02035	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/ext_comp.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

02100	/*
02101	  (c) copyright 1989 by the Vrije Universiteit, Amsterdam, The Netherlands.
02102	  See the copyright notice in the ACK home directory, in the file "Copyright".
02103	*/
02104	
02105	/* $Id: ext_comp.c,v 1.10 1994/06/24 11:53:36 ceriel Exp $ */
02106	
02107	/* extended precision arithmetic for the strtod() and cvt() routines */
02108	
02109	/* This may require some more work when long doubles get bigger than 8
02110	   bytes. In this case, these routines may become obsolete. ???
02111	*/
02112	
02113	#include        "ext_fmt.h"
02114	#include        <float.h>
02115	#include        <errno.h>
02116	#include        <ctype.h>
02117	
02118	static int b64_add(struct mantissa *e1, struct mantissa *e2);
02119	static b64_sft(struct mantissa *e1, int n);
02120	
02121	static
02122	mul_ext(struct EXTEND *e1, struct EXTEND *e2, struct EXTEND *e3)
02123	{
02124	        /*      Multiply the extended numbers e1 and e2, and put the
02125	                result in e3.
02126	        */
02127	        register int    i,j;            /* loop control */
02128	        unsigned short  mp[4];
02129	        unsigned short  mc[4];
.Ep 18 src/lib/ansi/ext_comp.c
02130	        unsigned short  result[8];      /* result */
02131	
02132	        register unsigned short *pres;
02133	
02134	        /* first save the sign (XOR)                    */
02135	        e3->sign = e1->sign ^ e2->sign;
02136	
02137	        /* compute new exponent */
02138	        e3->exp = e1->exp + e2->exp + 1;
02139	
02140	        /* check for overflow/underflow ??? */
02141	
02142	        /* 128 bit multiply of mantissas        */
02143	
02144	        /* assign unknown long formats          */
02145	        /* to known unsigned word formats       */
02146	        mp[0] = e1->m1 >> 16;
02147	        mp[1] = (unsigned short) e1->m1;
02148	        mp[2] = e1->m2 >> 16;
02149	        mp[3] = (unsigned short) e1->m2;
02150	        mc[0] = e2->m1 >> 16;
02151	        mc[1] = (unsigned short) e2->m1;
02152	        mc[2] = e2->m2 >> 16;
02153	        mc[3] = (unsigned short) e2->m2;
02154	        for (i = 8; i--;) {
02155	                result[i] = 0;
02156	        }
02157	        /*
02158	         *      fill registers with their components
02159	         */
02160	        for(i=4, pres = &result[4];i--;pres--) if (mp[i]) {
02161	                unsigned short k = 0;
02162	                unsigned long mpi = mp[i];
02163	                for(j=4;j--;) {
02164	                        unsigned long tmp = (unsigned long)pres[j] + k;
02165	                        if (mc[j]) tmp += mpi * mc[j];
02166	                        pres[j] = tmp;
02167	                        k = tmp >> 16;
02168	                }
02169	                pres[-1] = k;
02170	        }
02171	
02172	        if (! (result[0] & 0x8000)) {
02173	                e3->exp--;
02174	                for (i = 0; i <= 3; i++) {
02175	                        result[i] <<= 1;
02176	                        if (result[i+1]&0x8000) result[i] |= 1;
02177	                }
02178	                result[4] <<= 1;
02179	        }       
02180	        /*
02181	         *      combine the registers to a total
02182	         */
02183	        e3->m1 = ((unsigned long)(result[0]) << 16) + result[1];
02184	        e3->m2 = ((unsigned long)(result[2]) << 16) + result[3];
02185	        if (result[4] & 0x8000) {
02186	                if (++e3->m2 == 0) {
02187	                        if (++e3->m1 == 0) {
02188	                                e3->m1 = 0x80000000;
02189	                                e3->exp++;
.Op 19 src/lib/ansi/ext_comp.c
02190	                        }
02191	                }
02192	        }
02193	}
	
02195	static
02196	add_ext(struct EXTEND *e1, struct EXTEND *e2, struct EXTEND *e3)
02197	{
02198	        /*      Add two extended numbers e1 and e2, and put the result
02199	                in e3
02200	        */
02201	        struct EXTEND ce2;
02202	        int diff;
02203	
02204	        if ((e2->m1 | e2->m2) == 0L) {
02205	                *e3 = *e1;
02206	                return;
02207	        }
02208	        if ((e1->m1 | e1->m2) == 0L) {
02209	                *e3 = *e2;
02210	                return;
02211	        }
02212	        ce2 = *e2;
02213	        *e3 = *e1;
02214	        e1 = &ce2;
02215	
02216	        /* adjust mantissas to equal power */
02217	        diff = e3->exp - e1->exp;
02218	        if (diff < 0) {
02219	                diff = -diff;
02220	                e3->exp += diff;
02221	                b64_sft(&(e3->mantissa), diff);
02222	        }
02223	        else if (diff > 0) {
02224	                e1->exp += diff;
02225	                b64_sft(&(e1->mantissa), diff);
02226	        }
02227	        if (e1->sign != e3->sign) {
02228	                /* e3 + e1 = e3 - (-e1) */
02229	                if (e1->m1 > e3->m1 ||
02230	                    (e1->m1 == e3->m1 && e1->m2 > e3->m2)) {
02231	                        /*      abs(e1) > abs(e3) */
02232	                        if (e3->m2 > e1->m2) {
02233	                                e1->m1 -= 1;    /* carry in */
02234	                        }
02235	                        e1->m1 -= e3->m1;
02236	                        e1->m2 -= e3->m2;
02237	                        *e3 = *e1;
02238	                }
02239	                else {
02240	                        if (e1->m2 > e3->m2)
02241	                                e3->m1 -= 1;    /* carry in */
02242	                        e3->m1 -= e1->m1;
02243	                        e3->m2 -= e1->m2;
02244	                }
02245	        }
02246	        else {
02247	                if (b64_add(&e3->mantissa,&e1->mantissa)) {/* addition carry */
02248	                        b64_sft(&e3->mantissa,1);/* shift mantissa one bit RIGHT */
02249	                        e3->m1 |= 0x80000000L;  /* set max bit  */
.Ep 20 src/lib/ansi/ext_comp.c
02250	                        e3->exp++;              /* increase the exponent */
02251	                }
02252	        }
02253	        if ((e3->m2 | e3->m1) != 0L) {
02254	                /* normalize */
02255	                if (e3->m1 == 0L) {
02256	                        e3->m1 = e3->m2; e3->m2 = 0L; e3->exp -= 32;
02257	                }
02258	                if (!(e3->m1 & 0x80000000)) {
02259	                        unsigned long l = 0x40000000;
02260	                        int cnt = -1;
02261	
02262	                        while (! (l & e3->m1)) {
02263	                                l >>= 1; cnt--;
02264	                        }
02265	                        e3->exp += cnt;
02266	                        b64_sft(&(e3->mantissa), cnt);
02267	                }
02268	        }
02269	}
	
02271	static int
02272	cmp_ext(struct EXTEND *e1, struct EXTEND *e2)
02273	{
02274	        struct EXTEND tmp; 
02275	         
02276	        e2->sign = ! e2->sign; 
02277	        add_ext(e1, e2, &tmp);  
02278	        e2->sign = ! e2->sign;
02279	        if (tmp.m1 == 0 && tmp.m2 == 0) return 0; 
02280	        if (tmp.sign) return -1;
02281	        return 1;
02282	}
	
02284	static
02285	b64_sft(struct mantissa *e1, int n)
02286	{
02287	        if (n > 0) {
02288	                if (n > 63) {
02289	                        e1->l_32 = 0;
02290	                        e1->h_32 = 0;
02291	                        return;
02292	                }
02293	                if (n >= 32) {
02294	                        e1->l_32 = e1->h_32;
02295	                        e1->h_32 = 0;
02296	                        n -= 32;
02297	                }
02298	                if (n > 0) {
02299	                        e1->l_32 >>= n;
02300	                        if (e1->h_32 != 0) {
02301	                                e1->l_32 |= (e1->h_32 << (32 - n));
02302	                                e1->h_32 >>= n;
02303	                        }
02304	                }
02305	                return;
02306	        }
02307	        n = -n;
02308	        if (n > 0) {
02309	                if (n > 63) {
.Op 21 src/lib/ansi/ext_comp.c
02310	                        e1->l_32 = 0;
02311	                        e1->h_32 = 0;
02312	                        return;
02313	                }
02314	                if (n >= 32) {
02315	                        e1->h_32 = e1->l_32;
02316	                        e1->l_32 = 0;
02317	                        n -= 32;
02318	                }
02319	                if (n > 0) {
02320	                        e1->h_32 <<= n;
02321	                        if (e1->l_32 != 0) {
02322	                                e1->h_32 |= (e1->l_32 >> (32 - n));
02323	                                e1->l_32 <<= n;
02324	                        }
02325	                }
02326	        }
02327	}
	
02329	static int
02330	b64_add(struct mantissa *e1, struct mantissa *e2)
02331	                /*
02332	                 * pointers to 64 bit 'registers'
02333	                 */
02334	{
02335	        register int    overflow;
02336	        int             carry;
02337	
02338	                        /* add higher pair of 32 bits */
02339	        overflow = ((unsigned long) 0xFFFFFFFF - e1->h_32 < e2->h_32);
02340	        e1->h_32 += e2->h_32;
02341	
02342	                        /* add lower pair of 32 bits */
02343	        carry = ((unsigned long) 0xFFFFFFFF - e1->l_32 < e2->l_32);
02344	        e1->l_32 += e2->l_32;
02345	        if ((carry) && (++e1->h_32 == 0))
02346	                return(1);              /* had a 64 bit overflow */
02347	        else
02348	                return(overflow);       /* return status from higher add */
02349	}
	
02351	/* The following tables can be computed with the following bc(1)
02352	   program:
02353	
02354	obase=16
02355	scale=0
02356	define t(x){
02357	        auto a, b, c
02358	        a=2;b=1;c=2^32;n=1
02359	        while(a<x) {
02360	                b=a;n+=n;a*=a
02361	        }
02362	        n/=2
02363	        a=b
02364	        while(b<x) {
02365	                a=b;b*=c;n+=32
02366	        }
02367	        n-=32
02368	        b=a
02369	        while(a<x) {
.Ep 22 src/lib/ansi/ext_comp.c
02370	                b=a;a+=a;n+=1
02371	        }
02372	        n-=1
02373	        x*=16^16
02374	        b=x%a
02375	        x/=a
02376	        if(a<=(2*b)) x+=1
02377	        obase=10
02378	        n
02379	        obase=16
02380	        return(x)
02381	}
02382	for (i=1;i<28;i++) {
02383	        t(10^i)
02384	}
02385	0
02386	for (i=1;i<20;i++) {
02387	        t(10^(28*i))
02388	}
02389	0
02390	define r(x){
02391	        auto a, b, c
02392	        a=2;b=1;c=2^32;n=1
02393	        while(a<x) {
02394	                b=a;n+=n;a*=a
02395	        }
02396	        n/=2
02397	        a=b
02398	        while(b<x) {
02399	                a=b;b*=c;n+=32
02400	        }
02401	        n-=32
02402	        b=a
02403	        while(a<x) {
02404	                b=a;a+=a;n+=1
02405	        }
02406	        a=b
02407	        a*=16^16
02408	        b=a%x
02409	        a/=x
02410	        if(x<=(2*b)) a+=1
02411	        obase=10
02412	        -n
02413	        obase=16
02414	        return(a)
02415	}
02416	for (i=1;i<28;i++) {
02417	        r(10^i)
02418	}
02419	0
02420	for (i=1;i<20;i++) {
02421	        r(10^(28*i))
02422	}
02423	0
02424	
02425	*/
02426	static struct EXTEND ten_powers[] = {   /* representation of 10 ** i */
02427	        { 0,    0,      0x80000000,     0 },
02428	        { 0,    3,      0xA0000000,     0 },
02429	        { 0,    6,      0xC8000000,     0 },
.Op 23 src/lib/ansi/ext_comp.c
02430	        { 0,    9,      0xFA000000,     0 },
02431	        { 0,    13,     0x9C400000,     0 },
02432	        { 0,    16,     0xC3500000,     0 },
02433	        { 0,    19,     0xF4240000,     0 },
02434	        { 0,    23,     0x98968000,     0 },
02435	        { 0,    26,     0xBEBC2000,     0 },
02436	        { 0,    29,     0xEE6B2800,     0 },
02437	        { 0,    33,     0x9502F900,     0 },
02438	        { 0,    36,     0xBA43B740,     0 },
02439	        { 0,    39,     0xE8D4A510,     0 },
02440	        { 0,    43,     0x9184E72A,     0 },
02441	        { 0,    46,     0xB5E620F4,     0x80000000 },
02442	        { 0,    49,     0xE35FA931,     0xA0000000 },
02443	        { 0,    53,     0x8E1BC9BF,     0x04000000 },
02444	        { 0,    56,     0xB1A2BC2E,     0xC5000000 },
02445	        { 0,    59,     0xDE0B6B3A,     0x76400000 },
02446	        { 0,    63,     0x8AC72304,     0x89E80000 },
02447	        { 0,    66,     0xAD78EBC5,     0xAC620000 },
02448	        { 0,    69,     0xD8D726B7,     0x177A8000 },
02449	        { 0,    73,     0x87867832,     0x6EAC9000 },
02450	        { 0,    76,     0xA968163F,     0x0A57B400 },
02451	        { 0,    79,     0xD3C21BCE,     0xCCEDA100 },
02452	        { 0,    83,     0x84595161,     0x401484A0 },
02453	        { 0,    86,     0xA56FA5B9,     0x9019A5C8 },
02454	        { 0,    89,     0xCECB8F27,     0xF4200F3A }
02455	};
02456	static struct EXTEND big_ten_powers[] = {  /* representation of 10 ** (28*i) */
02457	        { 0,    0,      0x80000000,     0 },
02458	        { 0,    93,     0x813F3978,     0xF8940984 },
02459	        { 0,    186,    0x82818F12,     0x81ED44A0 },
02460	        { 0,    279,    0x83C7088E,     0x1AAB65DB },
02461	        { 0,    372,    0x850FADC0,     0x9923329E },
02462	        { 0,    465,    0x865B8692,     0x5B9BC5C2 },
02463	        { 0,    558,    0x87AA9AFF,     0x79042287 },
02464	        { 0,    651,    0x88FCF317,     0xF22241E2 },
02465	        { 0,    744,    0x8A5296FF,     0xE33CC930 },
02466	        { 0,    837,    0x8BAB8EEF,     0xB6409C1A },
02467	        { 0,    930,    0x8D07E334,     0x55637EB3 },
02468	        { 0,    1023,   0x8E679C2F,     0x5E44FF8F },
02469	        { 0,    1116,   0x8FCAC257,     0x558EE4E6 },
02470	        { 0,    1209,   0x91315E37,     0xDB165AA9 },
02471	        { 0,    1302,   0x929B7871,     0xDE7F22B9 },
02472	        { 0,    1395,   0x940919BB,     0xD4620B6D },
02473	        { 0,    1488,   0x957A4AE1,     0xEBF7F3D4 },
02474	        { 0,    1581,   0x96EF14C6,     0x454AA840 },
02475	        { 0,    1674,   0x98678061,     0x27ECE4F5 },
02476	        { 0,    1767,   0x99E396C1,     0x3A3ACFF2 }
02477	};
02478	
02479	static struct EXTEND r_ten_powers[] = { /* representation of 10 ** -i */
02480	        { 0,    0,      0x80000000,     0 },
02481	        { 0,    -4,     0xCCCCCCCC,     0xCCCCCCCD },
02482	        { 0,    -7,     0xA3D70A3D,     0x70A3D70A },
02483	        { 0,    -10,    0x83126E97,     0x8D4FDF3B },
02484	        { 0,    -14,    0xD1B71758,     0xE219652C },
02485	        { 0,    -17,    0xA7C5AC47,     0x1B478423 },
02486	        { 0,    -20,    0x8637BD05,     0xAF6C69B6 },
02487	        { 0,    -24,    0xD6BF94D5,     0xE57A42BC },
02488	        { 0,    -27,    0xABCC7711,     0x8461CEFD },
02489	        { 0,    -30,    0x89705F41,     0x36B4A597 },
.Ep 24 src/lib/ansi/ext_comp.c
02490	        { 0,    -34,    0xDBE6FECE,     0xBDEDD5BF },
02491	        { 0,    -37,    0xAFEBFF0B,     0xCB24AAFF },
02492	        { 0,    -40,    0x8CBCCC09,     0x6F5088CC },
02493	        { 0,    -44,    0xE12E1342,     0x4BB40E13 },
02494	        { 0,    -47,    0xB424DC35,     0x095CD80F },
02495	        { 0,    -50,    0x901D7CF7,     0x3AB0ACD9 },
02496	        { 0,    -54,    0xE69594BE,     0xC44DE15B },
02497	        { 0,    -57,    0xB877AA32,     0x36A4B449 },
02498	        { 0,    -60,    0x9392EE8E,     0x921D5D07 },
02499	        { 0,    -64,    0xEC1E4A7D,     0xB69561A5 },
02500	        { 0,    -67,    0xBCE50864,     0x92111AEB },
02501	        { 0,    -70,    0x971DA050,     0x74DA7BEF },
02502	        { 0,    -74,    0xF1C90080,     0xBAF72CB1 },
02503	        { 0,    -77,    0xC16D9A00,     0x95928A27 },
02504	        { 0,    -80,    0x9ABE14CD,     0x44753B53 },
02505	        { 0,    -84,    0xF79687AE,     0xD3EEC551 },
02506	        { 0,    -87,    0xC6120625,     0x76589DDB },
02507	        { 0,    -90,    0x9E74D1B7,     0x91E07E48 }
02508	};
02509	
02510	static struct EXTEND r_big_ten_powers[] = { /* representation of 10 ** -(28*i) */
02511	        { 0,    0,      0x80000000,     0 },
02512	        { 0,    -94,    0xFD87B5F2,     0x8300CA0E },
02513	        { 0,    -187,   0xFB158592,     0xBE068D2F },
02514	        { 0,    -280,   0xF8A95FCF,     0x88747D94 },
02515	        { 0,    -373,   0xF64335BC,     0xF065D37D },
02516	        { 0,    -466,   0xF3E2F893,     0xDEC3F126 },
02517	        { 0,    -559,   0xF18899B1,     0xBC3F8CA2 },
02518	        { 0,    -652,   0xEF340A98,     0x172AACE5 },
02519	        { 0,    -745,   0xECE53CEC,     0x4A314EBE },
02520	        { 0,    -838,   0xEA9C2277,     0x23EE8BCB },
02521	        { 0,    -931,   0xE858AD24,     0x8F5C22CA },
02522	        { 0,    -1024,  0xE61ACF03,     0x3D1A45DF },
02523	        { 0,    -1117,  0xE3E27A44,     0x4D8D98B8 },
02524	        { 0,    -1210,  0xE1AFA13A,     0xFBD14D6E },
02525	        { 0,    -1303,  0xDF82365C,     0x497B5454 },
02526	        { 0,    -1396,  0xDD5A2C3E,     0xAB3097CC },
02527	        { 0,    -1489,  0xDB377599,     0xB6074245 },
02528	        { 0,    -1582,  0xD91A0545,     0xCDB51186 },
02529	        { 0,    -1675,  0xD701CE3B,     0xD387BF48 },
02530	        { 0,    -1768,  0xD4EEC394,     0xD6258BF8 }
02531	};
02532	
02533	#define TP      (int)(sizeof(ten_powers)/sizeof(ten_powers[0]))
02534	#define BTP     (int)(sizeof(big_ten_powers)/sizeof(big_ten_powers[0]))
02535	#define MAX_EXP (TP * BTP - 1)
02536	
02537	static
02538	add_exponent(struct EXTEND *e, int exp)
02539	{
02540	        int neg = exp < 0;
02541	        int divsz, modsz;
02542	        struct EXTEND x;
02543	
02544	        if (neg) exp = -exp;
02545	        divsz = exp / TP;
02546	        modsz = exp % TP;
02547	        if (neg) {
02548	                mul_ext(e, &r_ten_powers[modsz], &x);
02549	                mul_ext(&x, &r_big_ten_powers[divsz], e);
.Op 25 src/lib/ansi/ext_comp.c
02550	        }
02551	        else {
02552	                mul_ext(e, &ten_powers[modsz], &x);
02553	                mul_ext(&x, &big_ten_powers[divsz], e);
02554	        }
02555	}
	
02557	_str_ext_cvt(const char *s, char **ss, struct EXTEND *e)
02558	{
02559	        /*      Like strtod, but for extended precision */
02560	        register int    c;
02561	        int             dotseen = 0;
02562	        int             digitseen = 0;
02563	        int             exp = 0;
02564	
02565	        if (ss) *ss = (char *)s;
02566	        while (isspace(*s)) s++;
02567	
02568	        e->sign = 0;
02569	        e->exp = 0;
02570	        e->m1 = e->m2 = 0;
02571	
02572	        c = *s;
02573	        switch(c) {
02574	        case '-':
02575	                e->sign = 1;
02576	        case '+':
02577	                s++;
02578	        }
02579	        while (c = *s++, isdigit(c) || (c == '.' && ! dotseen++)) {
02580	                if (c == '.') continue;
02581	                digitseen = 1;
02582	                if (e->m1 <= (unsigned long)(0xFFFFFFFF)/10) {
02583	                        struct mantissa a1;
02584	
02585	                        a1 = e->mantissa;
02586	                        b64_sft(&(e->mantissa), -3);
02587	                        b64_sft(&a1, -1);
02588	                        b64_add(&(e->mantissa), &a1);
02589	                        a1.h_32 = 0;
02590	                        a1.l_32 = c - '0';
02591	                        b64_add(&(e->mantissa), &a1);
02592	                }
02593	                else exp++;
02594	                if (dotseen) exp--;
02595	        }
02596	        if (! digitseen) return;
02597	
02598	        if (ss) *ss = (char *)s - 1;
02599	
02600	        if (c == 'E' || c == 'e') {
02601	                int     exp1 = 0;
02602	                int     sign = 1;
02603	                int     exp_overflow = 0;
02604	
02605	                switch(*s) {
02606	                case '-':
02607	                        sign = -1;
02608	                case '+':
02609	                        s++;
.Ep 26 src/lib/ansi/ext_comp.c
02610	                }
02611	                if (c = *s, isdigit(c)) {
02612	                        do {
02613	                                int tmp;
02614	
02615	                                exp1 = 10 * exp1 + (c - '0');
02616	                                if ((tmp = sign * exp1 + exp) > MAX_EXP ||
02617	                                     tmp < -MAX_EXP) {
02618	                                        exp_overflow = 1;
02619	                                }
02620	                        } while (c = *++s, isdigit(c));
02621	                        if (ss) *ss = (char *)s;
02622	                }
02623	                exp += sign * exp1;
02624	                if (exp_overflow) {
02625	                        exp = sign * MAX_EXP;
02626	                        if (e->m1 != 0 || e->m2 != 0) errno = ERANGE;
02627	                }
02628	        }
02629	        if (e->m1 == 0 && e->m2 == 0) return;
02630	        e->exp = 63;
02631	        while (! (e->m1 & 0x80000000)) {
02632	                b64_sft(&(e->mantissa),-1);
02633	                e->exp--;
02634	        }
02635	        add_exponent(e, exp);
02636	}
	
02638	#include        <math.h>
02639	
02640	static
02641	ten_mult(struct EXTEND *e)
02642	{
02643	        struct EXTEND e1 = *e;
02644	
02645	        e1.exp++;
02646	        e->exp += 3;
02647	        add_ext(e, &e1, e);
02648	}
	
02650	#define NDIGITS 128
02651	#define NSIGNIFICANT 19
02652	
02653	char *
02654	_ext_str_cvt(struct EXTEND *e, int ndigit, int *decpt, int *sign, int ecvtflag)
02655	{
02656	        /*      Like cvt(), but for extended precision */
02657	
02658	        static char buf[NDIGITS+1];
02659	        struct EXTEND m;
02660	        register char *p = buf;
02661	        register char *pe;
02662	        int findex = 0;
02663	
02664	        if (ndigit < 0) ndigit = 0;
02665	        if (ndigit > NDIGITS) ndigit = NDIGITS;
02666	        pe = &buf[ndigit];
02667	        buf[0] = '\0';
02668	
02669	        *sign = 0;
.Op 27 src/lib/ansi/ext_comp.c
02670	        if (e->sign) {
02671	                *sign = 1;
02672	                e->sign = 0;
02673	        }
02674	
02675	        *decpt = 0;
02676	        if (e->m1 != 0) {
02677	                register struct EXTEND *pp = &big_ten_powers[1];
02678	
02679	                while(cmp_ext(e,pp) >= 0) {
02680	                        pp++;
02681	                        findex = pp - big_ten_powers;
02682	                        if (findex >= BTP) break;
02683	                }
02684	                pp--;
02685	                findex = pp - big_ten_powers;
02686	                mul_ext(e,&r_big_ten_powers[findex],e);
02687	                *decpt += findex * TP;
02688	                pp = &ten_powers[1];
02689	                while(pp < &ten_powers[TP] && cmp_ext(e, pp) >= 0) pp++;
02690	                pp--;
02691	                findex = pp - ten_powers;
02692	                *decpt += findex;
02693	
02694	                if (cmp_ext(e, &ten_powers[0]) < 0) {
02695	                        pp = &r_big_ten_powers[1];
02696	                        while(cmp_ext(e,pp) < 0) pp++;
02697	                        pp--;
02698	                        findex = pp - r_big_ten_powers;
02699	                        mul_ext(e, &big_ten_powers[findex], e);
02700	                        *decpt -= findex * TP;
02701	                        /* here, value >= 10 ** -28 */
02702	                        ten_mult(e);
02703	                        (*decpt)--;
02704	                        pp = &r_ten_powers[0];
02705	                        while(cmp_ext(e, pp) < 0) pp++;
02706	                        findex = pp - r_ten_powers;
02707	                        mul_ext(e, &ten_powers[findex], e);
02708	                        *decpt -= findex;
02709	                        findex = 0;
02710	                }
02711	                (*decpt)++;     /* because now value in [1.0, 10.0) */
02712	        }
02713	        if (! ecvtflag) {
02714	                /* for fcvt() we need ndigit digits behind the dot */
02715	                pe += *decpt;
02716	                if (pe > &buf[NDIGITS]) pe = &buf[NDIGITS];
02717	        }
02718	        m.exp = -62;
02719	        m.sign = 0;
02720	        m.m1 = 0xA0000000;
02721	        m.m2 = 0;
02722	        while (p <= pe) {
02723	                struct EXTEND oneminm;
02724	
02725	                if (p - pe > NSIGNIFICANT) {
02726	                        findex = 0;
02727	                        e->m1 = 0;
02728	                }
02729	                if (findex) {
.Ep 28 src/lib/ansi/ext_comp.c
02730	                        struct EXTEND tc, oldtc;
02731	                        int count = 0;
02732	
02733	                        oldtc.exp = 0;
02734	                        oldtc.sign = 0;
02735	                        oldtc.m1 = 0;
02736	                        oldtc.m2 = 0;
02737	                        tc = ten_powers[findex];
02738	                        while (cmp_ext(e, &tc) >= 0) {
02739	                                oldtc = tc;
02740	                                add_ext(&tc, &ten_powers[findex], &tc);
02741	                                count++;
02742	                        }
02743	                        *p++ = count + '0';
02744	                        oldtc.sign = 1;
02745	                        add_ext(e, &oldtc, e);
02746	                        findex--;
02747	                        continue;
02748	                }
02749	                if (e->m1) {
02750	                        m.sign = 1;
02751	                        add_ext(&ten_powers[0], &m, &oneminm);
02752	                        m.sign = 0;
02753	                        if (e->exp >= 0) {
02754	                                struct EXTEND x;
02755	
02756	                                x.m2 = 0; x.exp = e->exp;
02757	                                x.sign = 1;
02758	                                x.m1 = e->m1>>(31-e->exp);
02759	                                *p++ = (x.m1) + '0';
02760	                                x.m1 = x.m1 << (31-e->exp);
02761	                                add_ext(e, &x, e);
02762	                        }
02763	                        else *p++ = '0';
02764	                        /* Check that remainder is still significant */
02765	                        if (cmp_ext(&m, e) > 0 || cmp_ext(e, &oneminm) > 0) {
02766	                                if (e->m1 && e->exp >= -1) *(p-1) += 1;
02767	                                e->m1 = 0;
02768	                                continue;
02769	                        }
02770	                        ten_mult(&m);
02771	                        ten_mult(e);
02772	                }
02773	                else *p++ = '0';
02774	        }
02775	        if (pe >= buf) {
02776	                p = pe;
02777	                *p += 5;        /* round of at the end */
02778	                while (*p > '9') {
02779	                        *p = '0';
02780	                        if (p > buf) ++*--p;
02781	                        else {
02782	                                *p = '1';
02783	                                ++*decpt;
02784	                                if (! ecvtflag) {
02785	                                        /* maybe add another digit at the end,
02786	                                           because the point was shifted right
02787	                                        */
02788	                                        if (pe > buf) *pe = '0';
02789	                                        pe++;
.Op 29 src/lib/ansi/ext_comp.c
02790	                                }
02791	                        }
02792	                }
02793	                *pe = '\0';
02794	        }
02795	        return buf;
02796	}
	
02798	_dbl_ext_cvt(double value, struct EXTEND *e)
02799	{
02800	        /*      Convert double to extended
02801	        */
02802	        int exponent;
02803	
02804	        value = frexp(value, &exponent);
02805	        e->sign = value < 0.0;
02806	        if (e->sign) value = -value;
02807	        e->exp = exponent - 1;
02808	        value *= 4294967296.0;
02809	        e->m1 = value;
02810	        value -= e->m1;
02811	        value *= 4294967296.0;
02812	        e->m2 = value;
02813	}
	
02815	static struct EXTEND max_d;
02816	
02817	double
02818	_ext_dbl_cvt(struct EXTEND *e)
02819	{
02820	        /*      Convert extended to double
02821	        */
02822	        double f;
02823	        int sign = e->sign;
02824	
02825	        e->sign = 0;
02826	        if (e->m1 == 0 && e->m2 == 0) {
02827	                return 0.0;
02828	        }
02829	        if (max_d.exp == 0) {
02830	                _dbl_ext_cvt(DBL_MAX, &max_d);
02831	        }
02832	        if (cmp_ext(&max_d, e) < 0) {
02833	                f = HUGE_VAL;
02834	                errno = ERANGE;
02835	        }
02836	        else    f = ldexp((double)e->m1*4294967296.0 + (double)e->m2, e->exp-63);
02837	        if (sign) f = -f;
02838	        if (f == 0.0 && (e->m1 != 0 || e->m2 != 0)) {
02839	                errno = ERANGE;
02840	        }
02841	        return f;
02842	}
.Ep 30 src/lib/ansi/getenv.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/getenv.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

02900	/*
02901	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
02902	 * See the copyright notice in the ACK home directory, in the file "Copyright".
02903	 */
02904	/* $Header: getenv.c,v 1.4 91/04/24 12:18:03 ceriel Exp $ */
02905	
02906	#include        <stdlib.h>
02907	
02908	extern const char **_penvp;
02909	
02910	char *
02911	getenv(const char *name)
02912	{
02913	        register const char **v = _penvp;
02914	        register const char *p, *q;
02915	
02916	        if (v == NULL || name == NULL)
02917	                return (char *)NULL;
02918	        while ((p = *v++) != NULL) {
02919	                q = name;
02920	                while (*q && (*q == *p++))
02921	                        q++;
02922	                if (*q || (*p != '='))
02923	                        continue;
02924	                return (char *)p + 1;
02925	        }
02926	        return (char *)NULL;
02927	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/gmtime.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

03000	/*
03001	 * gmtime - convert the calendar time into broken down time
03002	 */
03003	/* $Header: gmtime.c,v 1.4 91/04/22 13:20:27 ceriel Exp $ */
03004	
03005	#include        <time.h>
03006	#include        <limits.h>
03007	#include        "loc_time.h"
03008	
03009	struct tm *
03010	gmtime(register const time_t *timer)
03011	{
03012	        static struct tm br_time;
03013	        register struct tm *timep = &br_time;
03014	        time_t time = *timer;
03015	        register unsigned long dayclock, dayno;
03016	        int year = EPOCH_YR;
03017	
03018	        dayclock = (unsigned long)time % SECS_DAY;
03019	        dayno = (unsigned long)time / SECS_DAY;
.Op 31 src/lib/ansi/gmtime.c
03020	
03021	        timep->tm_sec = dayclock % 60;
03022	        timep->tm_min = (dayclock % 3600) / 60;
03023	        timep->tm_hour = dayclock / 3600;
03024	        timep->tm_wday = (dayno + 4) % 7;       /* day 0 was a thursday */
03025	        while (dayno >= YEARSIZE(year)) {
03026	                dayno -= YEARSIZE(year);
03027	                year++;
03028	        }
03029	        timep->tm_year = year - YEAR0;
03030	        timep->tm_yday = dayno;
03031	        timep->tm_mon = 0;
03032	        while (dayno >= _ytab[LEAPYEAR(year)][timep->tm_mon]) {
03033	                dayno -= _ytab[LEAPYEAR(year)][timep->tm_mon];
03034	                timep->tm_mon++;
03035	        }
03036	        timep->tm_mday = dayno + 1;
03037	        timep->tm_isdst = 0;
03038	
03039	        return timep;
03040	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/isalnum.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

03100	#include        <ctype.h>
03101	
03102	int (isalnum)(int c) {
03103	        return isalnum(c);
03104	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/isalpha.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

03200	#include        <ctype.h>
03201	
03202	int (isalpha)(int c) {
03203	        return isalpha(c);
03204	}
.Ep 32 src/lib/ansi/isascii.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/isascii.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

03300	#include        <ctype.h>
03301	
03302	int (isascii)(int c) {
03303	        return isascii(c);
03304	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/iscntrl.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

03400	#include        <ctype.h>
03401	
03402	int (iscntrl)(int c) {
03403	        return iscntrl(c);
03404	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/isdigit.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

03500	#include        <ctype.h>
03501	
03502	int (isdigit)(int c) {
03503	        return isdigit(c);
03504	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/isgraph.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

03600	#include        <ctype.h>
03601	
03602	int (isgraph)(int c) {
03603	        return isgraph(c);
03604	}
.Op 33 src/lib/ansi/islower.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/islower.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

03700	#include        <ctype.h>
03701	
03702	int (islower)(int c) {
03703	        return islower(c);
03704	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/isprint.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

03800	#include        <ctype.h>
03801	
03802	int (isprint)(int c) {
03803	        return isprint(c);
03804	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/ispunct.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

03900	#include        <ctype.h>
03901	
03902	int (ispunct)(int c) {
03903	        return ispunct(c);
03904	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/isspace.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

04000	#include        <ctype.h>
04001	
04002	int (isspace)(int c) {
04003	        return isspace(c);
04004	}
.Ep 34 src/lib/ansi/isupper.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/isupper.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

04100	#include        <ctype.h>
04101	
04102	int (isupper)(int c) {
04103	        return isupper(c);
04104	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/isxdigit.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

04200	#include        <ctype.h>
04201	
04202	int (isxdigit)(int c) {
04203	        return isxdigit(c);
04204	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/labs.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

04300	/*
04301	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
04302	 * See the copyright notice in the ACK home directory, in the file "Copyright".
04303	 */
04304	/* $Header: labs.c,v 1.1 89/05/16 13:08:11 eck Exp $ */
04305	
04306	#include        <stdlib.h>
04307	
04308	long
04309	labs(register long l)
04310	{
04311	        return l >= 0 ? l : -l;
04312	}
.Op 35 src/lib/ansi/ldiv.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/ldiv.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

04400	/*
04401	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
04402	 * See the copyright notice in the ACK home directory, in the file "Copyright".
04403	 */
04404	/* $Header: ldiv.c,v 1.3 90/03/05 13:48:03 eck Exp $ */
04405	
04406	#include        <stdlib.h>
04407	
04408	static long tmp = -1;
04409	
04410	ldiv_t
04411	ldiv(register long numer, register long denom)
04412	{
04413	        ldiv_t r;
04414	
04415	        /* The assignment of tmp should not be optimized !! */
04416	        if (tmp == -1) {
04417	                tmp = (tmp / 2 == 0);
04418	        }
04419	        if (numer == 0) {
04420	                r.quot = numer / denom;         /* might trap if denom == 0 */
04421	                r.rem = numer % denom;
04422	        } else if ( !tmp && ((numer < 0) != (denom < 0))) {
04423	                r.quot = (numer / denom) + 1;
04424	                r.rem = numer - (numer / denom + 1) * denom;
04425	        } else {
04426	                r.quot = numer / denom;
04427	                r.rem = numer % denom;
04428	        }
04429	        return r;
04430	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/localeconv.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

04500	/*
04501	 * localeconv - set components of a struct according to current locale
04502	 */
04503	/* $Header: localeconv.c,v 1.2 89/12/18 15:48:58 eck Exp $ */
04504	
04505	#include        <limits.h>
04506	#include        <locale.h>
04507	
04508	extern struct lconv _lc;
04509	
04510	struct lconv *
04511	localeconv(void)
04512	{
04513	        register struct lconv *lcp = &_lc;
04514	
.Ep 36 src/lib/ansi/localeconv.c
04515	        lcp->decimal_point = ".";
04516	        lcp->thousands_sep = "";
04517	        lcp->grouping = "";
04518	        lcp->int_curr_symbol = "";
04519	        lcp->currency_symbol = "";
04520	        lcp->mon_decimal_point = "";
04521	        lcp->mon_thousands_sep = "";
04522	        lcp->mon_grouping = "";
04523	        lcp->positive_sign = "";
04524	        lcp->negative_sign = "";
04525	        lcp->int_frac_digits = CHAR_MAX;
04526	        lcp->frac_digits = CHAR_MAX;
04527	        lcp->p_cs_precedes = CHAR_MAX;
04528	        lcp->p_sep_by_space = CHAR_MAX;
04529	        lcp->n_cs_precedes = CHAR_MAX;
04530	        lcp->n_sep_by_space = CHAR_MAX;
04531	        lcp->p_sign_posn = CHAR_MAX;
04532	        lcp->n_sign_posn = CHAR_MAX;
04533	
04534	        return lcp;
04535	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/localtime.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

04600	/*
04601	 * localtime - convert a calendar time into broken down time
04602	 */
04603	/* $Header: localtime.c,v 1.3 91/04/22 13:20:36 ceriel Exp $ */
04604	
04605	#include        <time.h>
04606	#include        "loc_time.h"
04607	
04608	/* We must be careful, since an int can't represent all the seconds in a day.
04609	 * Hence the adjustment of minutes when adding timezone and dst information.
04610	 * This assumes that both must be expressable in multiples of a minute.
04611	 * Furthermore, it is assumed that both fit into an integer when expressed as
04612	 * minutes (this is about 22 days, so this should not cause any problems). 
04613	 */
04614	struct tm *
04615	localtime(const time_t *timer)
04616	{
04617	        struct tm *timep;
04618	        unsigned dst;
04619	
04620	        _tzset();
04621	        timep = gmtime(timer);                  /* tm->tm_isdst == 0 */
04622	        timep->tm_min -= _timezone / 60;
04623	        timep->tm_sec -= _timezone % 60;
04624	        mktime(timep);
04625	
04626	        dst = _dstget(timep);
04627	        if (dst) {
04628	                timep->tm_min += dst / 60;
04629	                timep->tm_sec += dst % 60;
.Op 37 src/lib/ansi/localtime.c
04630	                mktime(timep);
04631	        }
04632	        return timep;
04633	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/malloc.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

04700	/* $Header$ */
04701	
04702	/* replace undef by define */
04703	#undef   DEBUG          /* check assertions */
04704	#undef   SLOWDEBUG      /* some extra test loops (requires DEBUG) */
04705	
04706	#include        <stdlib.h>
04707	#include        <string.h>
04708	#include        <errno.h>
04709	
04710	#ifdef DEBUG
04711	#define ASSERT(b)       if (!(b)) assert_failed();
04712	#else
04713	#define ASSERT(b)       /* empty */
04714	#endif
04715	
04716	#if _EM_WSIZE == _EM_PSIZE
04717	#define ptrint          int
04718	#else
04719	#define ptrint          long
04720	#endif
04721	
04722	#if     _EM_PSIZE == 2
04723	#define BRKSIZE         1024
04724	#else
04725	#define BRKSIZE         4096
04726	#endif
04727	#define PTRSIZE         ((int) sizeof(void *))
04728	#define Align(x,a)      (((x) + (a - 1)) & ~(a - 1))
04729	#define NextSlot(p)     (* (void **) ((p) - PTRSIZE))
04730	#define NextFree(p)     (* (void **) (p))
04731	
04732	/*
04733	 * A short explanation of the data structure and algorithms.
04734	 * An area returned by malloc() is called a slot. Each slot
04735	 * contains the number of bytes requested, but preceeded by
04736	 * an extra pointer to the next the slot in memory.
04737	 * '_bottom' and '_top' point to the first/last slot.
04738	 * More memory is asked for using brk() and appended to top.
04739	 * The list of free slots is maintained to keep malloc() fast.
04740	 * '_empty' points the the first free slot. Free slots are
04741	 * linked together by a pointer at the start of the
04742	 * user visable part, so just after the next-slot pointer.
04743	 * Free slots are merged together by free().
04744	 */
04745	
04746	extern void *_sbrk(int);
04747	extern int _brk(void *);
04748	static void *_bottom, *_top, *_empty;
04749	
.Ep 38 src/lib/ansi/malloc.c
04750	static int grow(size_t len)
04751	{
04752	  register char *p;
04753	
04754	  ASSERT(NextSlot((char *)_top) == 0);
04755	  errno = ENOMEM;
04756	  if ((char *) _top + len < (char *) _top
04757	      || (p = (char *)Align((ptrint)_top + len, BRKSIZE)) < (char *) _top 
04758	      || _brk(p) != 0)
04759	        return(0);
04760	  NextSlot((char *)_top) = p;
04761	  NextSlot(p) = 0;
04762	  free(_top);
04763	  _top = p;
04764	  return 1;
04765	}
	
04767	void *
04768	malloc(size_t size)
04769	{
04770	  register char *prev, *p, *next, *new;
04771	  register unsigned len, ntries;
04772	
04773	  if (size == 0) return NULL;
04774	  errno = ENOMEM;
04775	  for (ntries = 0; ntries < 2; ntries++) {
04776	        if ((len = Align(size, PTRSIZE) + PTRSIZE) < 2 * PTRSIZE)
04777	                return NULL;
04778	        if (_bottom == 0) {
04779	                if ((p = _sbrk(2 * PTRSIZE)) == (char *) -1)
04780	                        return NULL;
04781	                p = (char *) Align((ptrint)p, PTRSIZE);
04782	                p += PTRSIZE;
04783	                _top = _bottom = p;
04784	                NextSlot(p) = 0;
04785	        }
04786	#ifdef SLOWDEBUG
04787	        for (p = _bottom; (next = NextSlot(p)) != 0; p = next)
04788	                ASSERT(next > p);
04789	        ASSERT(p == _top);
04790	#endif
04791	        for (prev = 0, p = _empty; p != 0; prev = p, p = NextFree(p)) {
04792	                next = NextSlot(p);
04793	                new = p + len;  /* easily overflows!! */
04794	                if (new > next || new <= p)
04795	                        continue;               /* too small */
04796	                if (new + PTRSIZE < next) {     /* too big, so split */
04797	                        /* + PTRSIZE avoids tiny slots on free list */
04798	                        NextSlot(new) = next;
04799	                        NextSlot(p) = new;
04800	                        NextFree(new) = NextFree(p);
04801	                        NextFree(p) = new;
04802	                }
04803	                if (prev)
04804	                        NextFree(prev) = NextFree(p);
04805	                else
04806	                        _empty = NextFree(p);
04807	                return p;
04808	        }
04809	        if (grow(len) == 0)
.Op 39 src/lib/ansi/malloc.c
04810	                break;
04811	  }
04812	  ASSERT(ntries != 2);
04813	  return NULL;
04814	}
	
04816	void *
04817	realloc(void *oldp, size_t size)
04818	{
04819	  register char *prev, *p, *next, *new;
04820	  char *old = oldp;
04821	  register size_t len, n;
04822	
04823	  if (!old) return malloc(size);
04824	  else if (!size) {
04825	        free(oldp);
04826	        return NULL;
04827	  }
04828	  len = Align(size, PTRSIZE) + PTRSIZE;
04829	  next = NextSlot(old);
04830	  n = (int)(next - old);                        /* old length */
04831	  /*
04832	   * extend old if there is any free space just behind it
04833	   */
04834	  for (prev = 0, p = _empty; p != 0; prev = p, p = NextFree(p)) {
04835	        if (p > next)
04836	                break;
04837	        if (p == next) {        /* 'next' is a free slot: merge */
04838	                NextSlot(old) = NextSlot(p);
04839	                if (prev)
04840	                        NextFree(prev) = NextFree(p);
04841	                else
04842	                        _empty = NextFree(p);
04843	                next = NextSlot(old);
04844	                break;
04845	        }
04846	  }
04847	  new = old + len;
04848	  /*
04849	   * Can we use the old, possibly extended slot?
04850	   */
04851	  if (new <= next && new >= old) {              /* it does fit */
04852	        if (new + PTRSIZE < next) {             /* too big, so split */
04853	                /* + PTRSIZE avoids tiny slots on free list */
04854	                NextSlot(new) = next;
04855	                NextSlot(old) = new;
04856	                free(new);
04857	        }
04858	        return old;
04859	  }
04860	  if ((new = malloc(size)) == NULL)             /* it didn't fit */
04861	        return NULL;
04862	  memcpy(new, old, n);                          /* n < size */
04863	  free(old);
04864	  return new;
04865	}
	
04867	void
04868	free(void *ptr)
04869	{
.Ep 40 src/lib/ansi/malloc.c
04870	  register char *prev, *next;
04871	  char *p = ptr;
04872	
04873	  if (!p) return;
04874	
04875	  ASSERT(NextSlot(p) > p);
04876	  for (prev = 0, next = _empty; next != 0; prev = next, next = NextFree(next))
04877	        if (p < next)
04878	                break;
04879	  NextFree(p) = next;
04880	  if (prev)
04881	        NextFree(prev) = p;
04882	  else
04883	        _empty = p;
04884	  if (next) {
04885	        ASSERT(NextSlot(p) <= next);
04886	        if (NextSlot(p) == next) {              /* merge p and next */
04887	                NextSlot(p) = NextSlot(next);
04888	                NextFree(p) = NextFree(next);
04889	        }
04890	  }
04891	  if (prev) {
04892	        ASSERT(NextSlot(prev) <= p);
04893	        if (NextSlot(prev) == p) {              /* merge prev and p */
04894	                NextSlot(prev) = NextSlot(p);
04895	                NextFree(prev) = NextFree(p);
04896	        }
04897	  }
04898	}
	
04900	#ifdef DEBUG
04901	static assert_failed()
04902	{
04903	        write(2, "assert failed in lib/malloc.c\n", 30);
04904	        abort();
04905	}
04906	#endif




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/mblen.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

05000	/*
05001	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
05002	 * See the copyright notice in the ACK home directory, in the file "Copyright".
05003	 */
05004	/* $Header: mblen.c,v 1.2 89/12/18 15:12:50 eck Exp $ */
05005	
05006	#include        <stdlib.h>
05007	#include        <limits.h>
05008	
05009	#define CHAR_SHIFT      8
05010	
05011	int
05012	mblen(const char *s, size_t n)
05013	{
05014	        if (s == (const char *)NULL) return 0;  /* no state dependent codings */
.Op 41 src/lib/ansi/mblen.c
05015	        if (n <= 0) return 0;
05016	        return (*s != 0);
05017	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/mbstowcs.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

05100	/*
05101	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
05102	 * See the copyright notice in the ACK home directory, in the file "Copyright".
05103	 */
05104	/* $Header: mbstowcs.c,v 1.2 89/12/18 15:12:59 eck Exp $ */
05105	
05106	#include        <stdlib.h>
05107	
05108	size_t
05109	mbstowcs(register wchar_t *pwcs, register const char *s, size_t n)
05110	{
05111	        register int i = n;
05112	
05113	        while (--i >= 0) {
05114	                if (!(*pwcs++ = *s++))
05115	                        return n - i - 1;
05116	        }
05117	        return n - i;
05118	}
	






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/mbtowc.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

05200	/*
05201	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
05202	 * See the copyright notice in the ACK home directory, in the file "Copyright".
05203	 */
05204	/* $Header: mbtowc.c,v 1.3 90/03/28 16:36:45 eck Exp $ */
05205	
05206	#include        <stdlib.h>
05207	#include        <limits.h>
05208	
05209	int
05210	mbtowc(wchar_t *pwc, register const char *s, size_t n)
05211	{
05212	        if (s == (const char *)NULL) return 0;
05213	        if (n <= 0) return 0;
05214	        if (pwc) *pwc = *s;
05215	        return (*s != 0);
05216	}
.Ep 42 src/lib/ansi/memchr.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/memchr.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

05300	/*
05301	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
05302	 * See the copyright notice in the ACK home directory, in the file "Copyright".
05303	 */
05304	/* $Header: memchr.c,v 1.3 90/08/28 13:52:11 eck Exp $ */
05305	
05306	#include        <string.h>
05307	
05308	void *
05309	memchr(const void *s, register int c, register size_t n)
05310	{
05311	        register const unsigned char *s1 = s;
05312	
05313	        c = (unsigned char) c;
05314	        if (n) {
05315	                n++;
05316	                while (--n > 0) {
05317	                        if (*s1++ != c) continue;
05318	                        return (void *) --s1;
05319	                }
05320	        }
05321	        return NULL;
05322	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/memcmp.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

05400	/*
05401	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
05402	 * See the copyright notice in the ACK home directory, in the file "Copyright".
05403	 */
05404	/* $Id: memcmp.c,v 1.6 1994/06/24 11:56:26 ceriel Exp $ */
05405	
05406	#include        <string.h>
05407	
05408	int
05409	memcmp(const void *s1, const void *s2, size_t n)
05410	{
05411	        register const unsigned char *p1 = s1, *p2 = s2;
05412	
05413	        if (n) {
05414	                n++;
05415	                while (--n > 0) {
05416	                        if (*p1++ == *p2++) continue;
05417	                        return *--p1 - *--p2;
05418	                }
05419	        }
05420	        return 0;
05421	}
.Op 43 src/lib/ansi/memcpy.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/memcpy.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

05500	/*
05501	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
05502	 * See the copyright notice in the ACK home directory, in the file "Copyright".
05503	 */
05504	/* $Header: memcpy.c,v 1.4 90/08/28 13:52:31 eck Exp $ */
05505	
05506	#include        <string.h>
05507	
05508	void *
05509	memcpy(void *s1, const void *s2, register size_t n)
05510	{
05511	        register char *p1 = s1;
05512	        register const char *p2 = s2;
05513	
05514	
05515	        if (n) {
05516	                n++;
05517	                while (--n > 0) {
05518	                        *p1++ = *p2++;
05519	                }
05520	        }
05521	        return s1;
05522	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/memmove.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

05600	/*
05601	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
05602	 * See the copyright notice in the ACK home directory, in the file "Copyright".
05603	 */
05604	/* $Header: memmove.c,v 1.3 90/08/28 13:52:42 eck Exp $ */
05605	
05606	#include        <string.h>
05607	
05608	void *
05609	memmove(void *s1, const void *s2, register size_t n)
05610	{
05611	        register char *p1 = s1;
05612	        register const char *p2 = s2;
05613	
05614	        if (n>0) {
05615	                if (p2 <= p1 && p2 + n > p1) {
05616	                        /* overlap, copy backwards */
05617	                        p1 += n;
05618	                        p2 += n;
05619	                        n++;
05620	                        while (--n > 0) {
05621	                                *--p1 = *--p2;
05622	                        }
05623	                } else {
05624	                        n++;
.Ep 44 src/lib/ansi/memmove.c
05625	                        while (--n > 0) {
05626	                                *p1++ = *p2++;
05627	                        }
05628	                }
05629	        }
05630	        return s1;
05631	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/memset.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

05700	/*
05701	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
05702	 * See the copyright notice in the ACK home directory, in the file "Copyright".
05703	 */
05704	/* $Header: memset.c,v 1.3 90/08/28 13:52:54 eck Exp $ */
05705	
05706	#include        <string.h>
05707	
05708	void *
05709	memset(void *s, register int c, register size_t n)
05710	{
05711	        register char *s1 = s;
05712	
05713	        if (n>0) {
05714	                n++;
05715	                while (--n > 0) {
05716	                        *s1++ = c;
05717	                }
05718	        }
05719	        return s;
05720	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/misc.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

05800	/*
05801	 * misc - data and miscellaneous routines
05802	 */
05803	/* $Header: misc.c,v 1.11 91/04/22 13:20:45 ceriel Exp $ */
05804	
05805	#include        <ctype.h>
05806	#include        <time.h>
05807	#include        <stdlib.h>
05808	#include        <string.h>
05809	
05810	#if     defined(__BSD4_2)
05811	
05812	struct timeval {
05813	        long    tv_sec;         /* seconds */
05814	        long    tv_usec;        /* and microseconds */
.Op 45 src/lib/ansi/misc.c
05815	};
05816	
05817	struct timezone {
05818	        int     tz_minuteswest; /* minutes west of Greenwich */
05819	        int     tz_dsttime;     /* type of dst correction */
05820	};
05821	
05822	int _gettimeofday(struct timeval *tp, struct timezone *tzp);
05823	
05824	#elif   !defined(_POSIX_SOURCE) && !defined(__USG)
05825	#if     !defined(_MINIX)                /* MINIX has no ftime() */
05826	struct timeb {
05827	        long    time;
05828	        unsigned short millitm;
05829	        short   timezone;
05830	        short   dstflag;
05831	};
05832	void _ftime(struct timeb *bp);
05833	#endif
05834	#endif
05835	
05836	#include        "loc_time.h"
05837	
05838	#define RULE_LEN        120
05839	#define TZ_LEN          10
05840	
05841	/* Make sure that the strings do not end up in ROM.
05842	 * These strings probably contain the wrong value, and we cannot obtain the
05843	 * right value from the system. TZ is the only help.
05844	 */
05845	static char ntstr[TZ_LEN + 1] = "GMT";  /* string for normal time */
05846	static char dststr[TZ_LEN + 1] = "GDT"; /* string for daylight saving */
05847	
05848	long    _timezone = 0;
05849	long    _dst_off = 60 * 60;
05850	int     _daylight = 0;
05851	char    *_tzname[2] = {ntstr, dststr};
05852	
05853	#if     defined(__USG) || defined(_POSIX_SOURCE)
05854	char    *tzname[2] = {ntstr, dststr};
05855	
05856	#if     defined(__USG)
05857	long    timezone = 0;
05858	int     daylight = 0;
05859	#endif
05860	#endif
05861	
05862	static struct dsttype {
05863	        char ds_type;           /* Unknown, Julian, Zero-based or M */
05864	        int ds_date[3];         /* months, weeks, days */
05865	        long ds_sec;            /* usually 02:00:00 */
05866	}       dststart = { 'U', { 0, 0, 0 }, 2 * 60 * 60 }
05867	        , dstend = { 'U', { 0, 0, 0 }, 2 * 60 * 60 };
05868	
05869	const char *_days[] = {
05870	                        "Sunday", "Monday", "Tuesday", "Wednesday",
05871	                        "Thursday", "Friday", "Saturday"
05872	                };
05873	
05874	const char *_months[] = {
.Ep 46 src/lib/ansi/misc.c
05875	                        "January", "February", "March",
05876	                        "April", "May", "June",
05877	                        "July", "August", "September",
05878	                        "October", "November", "December"
05879	                };
05880	
05881	const int _ytab[2][12] = {
05882	                { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
05883	                { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
05884	        };
05885	
05886	#if     !defined(_POSIX_SOURCE) && !defined(__USG)
05887	#define USE_TABLE       1
05888	#endif
05889	
05890	#if     USE_TABLE
05891	static int usetable = 1;
05892	
05893	typedef struct table {
05894	        const char *tz_name;
05895	        const int daylight;
05896	        const long zoneoffset;
05897	} TABLE;
05898	
05899	#define HOUR(x) ((x) * 60*60)
05900	
05901	static TABLE TimezoneTable[] = {
05902	        {"GMT", 0,      HOUR(0) },      /* Greenwich Mean */
05903	        {"BST", 60*60,  HOUR(0) },      /* British Summer */
05904	        {"WAT", 0,      HOUR(1) },      /* West Africa */
05905	        {"AT", 0,       HOUR(2) },      /* Azores */
05906	        {"BST", 0,      HOUR(3) },      /* Brazil Standard */
05907	        {"NFT", 0,      HOUR(3.5) },    /* Newfoundland */
05908	        {"NDT", 60*60,  HOUR(3.5) },    /* Newfoundland Daylight */
05909	        {"AST", 0,      HOUR(4) },      /* Atlantic Standard */
05910	        {"ADT", 60*60,  HOUR(4) },      /* Atlantic Daylight */
05911	        {"EST", 0,      HOUR(5) },      /* Eastern Standard */
05912	        {"EDT", 60*60,  HOUR(5) },      /* Eastern Daylight */
05913	        {"CST", 0,      HOUR(6) },      /* Central Standard */
05914	        {"CDT", 60*60,  HOUR(6) },      /* Central Daylight */
05915	        {"MST", 0,      HOUR(7) },      /* Mountain Standard */
05916	        {"MDT", 60*60,  HOUR(7) },      /* Mountain Daylight */
05917	        {"PST", 0,      HOUR(8) },      /* Pacific Standard */
05918	        {"PDT", 60*60,  HOUR(8) },      /* Pacific Daylight */
05919	        {"YST", 0,      HOUR(9) },      /* Yukon Standard */
05920	        {"YDT", 60*60,  HOUR(9) },      /* Yukon Daylight */
05921	        {"HST", 0,      HOUR(10) },     /* Hawaii Standard */
05922	        {"HDT", 60*60,  HOUR(10) },     /* Hawaii Daylight */
05923	        {"NT", 0,       HOUR(11) },     /* Nome */
05924	        {"IDLW", 0,     HOUR(12) },     /* International Date Line West */
05925	        {"MET", 0,      -HOUR(1) },     /* Middle European */
05926	        {"MDT", 60*60,  -HOUR(1) },     /* Middle European Summer */
05927	        {"EET", 0,      -HOUR(2) },     /* Eastern Europe, USSR Zone 1 */
05928	        {"BT", 0,       -HOUR(3) },     /* Baghdad, USSR Zone 2 */
05929	        {"IT", 0,       -HOUR(3.5) },   /* Iran */
05930	        {"ZP4", 0,      -HOUR(4) },     /* USSR Zone 3 */
05931	        {"ZP5", 0,      -HOUR(5) },     /* USSR Zone 4 */
05932	        {"IST", 0,      -HOUR(5.5) },   /* Indian Standard */
05933	        {"ZP6", 0,      -HOUR(6) },     /* USSR Zone 5 */
05934	        {"NST", 0,      -HOUR(6.5) },   /* North Sumatra */
.Op 47 src/lib/ansi/misc.c
05935	        {"SST", 0,      -HOUR(7) },     /* South Sumatra, USSR Zone 6 */
05936	        {"WAST", 0,     -HOUR(7) },     /* West Australian Standard */
05937	        {"WADT", 60*60, -HOUR(7) },     /* West Australian Daylight */
05938	        {"JT", 0,       -HOUR(7.5) },   /* Java (3pm in Cronusland!) */
05939	        {"CCT", 0,      -HOUR(8) },     /* China Coast, USSR Zone 7 */
05940	        {"JST", 0,      -HOUR(9) },     /* Japan Standard, USSR Zone 8 */
05941	        {"CAST", 0,     -HOUR(9.5) },   /* Central Australian Standard */
05942	        {"CADT", 60*60, -HOUR(9.5) },   /* Central Australian Daylight */
05943	        {"EAST", 0,     -HOUR(10) },    /* Eastern Australian Standard */
05944	        {"EADT", 60*60, -HOUR(10) },    /* Eastern Australian Daylight */
05945	        {"NZT", 0,      -HOUR(12) },    /* New Zealand */
05946	        {"NZDT", 60*60, -HOUR(12) },    /* New Zealand Daylight */
05947	        {  NULL, 0, 0  }
05948	};
05949	
05950	/*
05951	 * The function ZoneFromTable() searches the table for the current
05952	 * timezone.  It saves the last one found in ntstr or dststr, depending on
05953	 * wheter the name is for daylight-saving-time or not.
05954	 * Both ntstr and dststr are TZ_LEN + 1 chars.
05955	 */
05956	static void
05957	ZoneFromTable(long timezone)
05958	{
05959	        register TABLE *tptr = TimezoneTable;
05960	
05961	        while (tptr->tz_name != NULL) {
05962	                if (tptr->zoneoffset == timezone) {
05963	                        if (tptr->daylight == 0) {
05964	                                strncpy(ntstr,tptr->tz_name, TZ_LEN);
05965	                                ntstr[TZ_LEN] = '\0';
05966	                        } else {
05967	                                strncpy(dststr,tptr->tz_name, TZ_LEN);
05968	                                dststr[TZ_LEN] = '\0';
05969	                        }
05970	                }
05971	                tptr++;
05972	        }
05973	}
05974	#endif  /* USE_TABLE */
05975	
05976	static const char *
05977	parseZoneName(register char *buf, register const char *p)
05978	{
05979	        register int n = 0;
05980	
05981	        if (*p == ':') return NULL;
05982	        while (*p && !isdigit(*p) && *p != ',' && *p != '-' && *p != '+') {
05983	                if (n < TZ_LEN)
05984	                        *buf++ = *p;
05985	                p++;
05986	                n++;
05987	        }
05988	        if (n < 3) return NULL;                         /* error */
05989	        *buf = '\0';
05990	        return p;
05991	}
	
05993	static const char *
05994	parseTime(register long *tm, const char *p, register struct dsttype *dst)
.Ep 48 src/lib/ansi/misc.c
05995	{
05996	        register int n = 0;
05997	        register const char *q = p;
05998	        char ds_type = (dst ? dst->ds_type : '\0');
05999	
06000	        if (dst) dst->ds_type = 'U';
06001	
06002	        *tm = 0;
06003	        while(*p >= '0' && *p <= '9') {
06004	                n = 10 * n + (*p++ - '0');
06005	        }
06006	        if (q == p) return NULL;        /* "The hour shall be required" */
06007	        if (n < 0 || n >= 24)   return NULL;
06008	        *tm = n * 60 * 60;
06009	        if (*p == ':') {
06010	                p++;
06011	                n = 0;
06012	                while(*p >= '0' && *p <= '9') {
06013	                        n = 10 * n + (*p++ - '0');
06014	                }
06015	                if (q == p) return NULL;        /* format error */
06016	                if (n < 0 || n >= 60)   return NULL;
06017	                *tm += n * 60;
06018	                if (*p == ':') {
06019	                        p++;
06020	                        n = 0;
06021	                        while(*p >= '0' && *p <= '9') {
06022	                                n = 10 * n + (*p++ - '0');
06023	                        }
06024	                        if (q == p) return NULL;        /* format error */
06025	                        if (n < 0 || n >= 60)   return NULL;
06026	                        *tm += n;
06027	                }
06028	        }
06029	        if (dst) {
06030	                dst->ds_type = ds_type;
06031	                dst->ds_sec = *tm;
06032	        }
06033	        return p;
06034	}
	
06036	static const char *
06037	parseDate(register char *buf, register const char *p, struct dsttype *dstinfo)
06038	{
06039	        register const char *q;
06040	        register int n = 0;
06041	        int cnt = 0;
06042	        const int bnds[3][2] =  {       { 1, 12 },
06043	                                        { 1, 5 },
06044	                                        { 0, 6}
06045	                                 };
06046	        char ds_type;
06047	
06048	        if (*p != 'M') {
06049	                if (*p == 'J') {
06050	                        *buf++ = *p++;
06051	                        ds_type = 'J';
06052	                }
06053	                else    ds_type = 'Z';
06054	                q = p;
.Op 49 src/lib/ansi/misc.c
06055	                while(*p >= '0' && *p <= '9') {
06056	                        n = 10 * n + (*p - '0');
06057	                        *buf++ = *p++;
06058	                }
06059	                if (q == p) return NULL;        /* format error */
06060	                if (n < (ds_type == 'J') || n > 365) return NULL;
06061	                dstinfo->ds_type = ds_type;
06062	                dstinfo->ds_date[0] = n;
06063	                return p;
06064	        }
06065	        ds_type = 'M';
06066	        do {
06067	                *buf++ = *p++;
06068	                q = p;
06069	                n = 0;
06070	                while(*p >= '0' && *p <= '9') {
06071	                        n = 10 * n + (*p - '0');
06072	                        *buf++ = *p++;
06073	                }
06074	                if (q == p) return NULL;        /* format error */
06075	                if (n < bnds[cnt][0] || n > bnds[cnt][1]) return NULL;
06076	                dstinfo->ds_date[cnt] = n;
06077	                cnt++;
06078	        } while (cnt < 3 && *p == '.');
06079	        if (cnt != 3) return NULL;
06080	        *buf = '\0';
06081	        dstinfo->ds_type = ds_type;
06082	        return p;
06083	}
	
06085	static const char *
06086	parseRule(register char *buf, register const char *p)
06087	{
06088	        long time;
06089	        register const char *q;
06090	
06091	        if (!(p = parseDate(buf, p, &dststart))) return NULL;
06092	        buf += strlen(buf);
06093	        if (*p == '/') {
06094	                q = ++p;
06095	                if (!(p = parseTime(&time, p, &dststart))) return NULL;
06096	                while( p != q) *buf++ = *q++;
06097	        }
06098	        if (*p != ',') return NULL;
06099	        p++;
06100	        if (!(p = parseDate(buf, p, &dstend))) return NULL;
06101	        buf += strlen(buf);
06102	        if (*p == '/') {
06103	                q = ++p;
06104	                if (!(p = parseTime(&time, p, &dstend))) return NULL;
06105	                while(*buf++ = *q++);
06106	        }
06107	        if (*p) return NULL;
06108	        return p;
06109	}
	
06111	/* The following routine parses timezone information in POSIX-format. For
06112	 * the requirements, see IEEE Std 1003.1-1988 section 8.1.1.
06113	 * The function returns as soon as it spots an error.
06114	 */
.Ep 50 src/lib/ansi/misc.c
06115	static void
06116	parseTZ(const char *p)
06117	{
06118	        long tz, dst = 60 * 60, sign = 1;
06119	        static char lastTZ[2 * RULE_LEN];
06120	        static char buffer[RULE_LEN];
06121	
06122	        if (!p) return;
06123	
06124	#if     USE_TABLE
06125	        usetable = 0;
06126	#endif
06127	        if (*p == ':') {
06128	                /*
06129	                 * According to POSIX, this is implementation defined.
06130	                 * Since it depends on the particular operating system, we
06131	                 * can do nothing.
06132	                 */
06133	                return;
06134	        }
06135	
06136	        if (!strcmp(lastTZ, p)) return;         /* nothing changed */
06137	
06138	        *_tzname[0] = '\0';
06139	        *_tzname[1] = '\0';
06140	        dststart.ds_type = 'U';
06141	        dststart.ds_sec = 2 * 60 * 60;
06142	        dstend.ds_type = 'U';
06143	        dstend.ds_sec = 2 * 60 * 60;
06144	
06145	        if (strlen(p) > 2 * RULE_LEN) return;
06146	        strcpy(lastTZ, p);
06147	
06148	        if (!(p = parseZoneName(buffer, p))) return;
06149	
06150	        if (*p == '-') {
06151	                sign = -1;
06152	                p++;
06153	        } else if (*p == '+') p++;
06154	
06155	        if (!(p = parseTime(&tz, p, NULL))) return;
06156	        tz *= sign;
06157	        _timezone = tz;
06158	        strncpy(_tzname[0], buffer, TZ_LEN);
06159	
06160	        if (!(_daylight = (*p != '\0'))) return;
06161	
06162	        buffer[0] = '\0';
06163	        if (!(p = parseZoneName(buffer, p))) return;
06164	        strncpy(_tzname[1], buffer, TZ_LEN);
06165	
06166	        buffer[0] = '\0';
06167	        if (*p && (*p != ','))
06168	                if (!(p = parseTime(&dst, p, NULL))) return;
06169	        _dst_off = dst;                 /* dst was initialized to 1 hour */
06170	        if (*p) {
06171	                if (*p != ',') return;
06172	                p++;
06173	                if (strlen(p) > RULE_LEN) return;
06174	                if (!(p = parseRule(buffer, p))) return;
.Op 51 src/lib/ansi/misc.c
06175	        }
06176	}
	
06178	void
06179	_tzset(void)
06180	{
06181	#if     defined(__BSD4_2)
06182	
06183	        struct timeval tv;
06184	        struct timezone tz;
06185	
06186	        _gettimeofday(&tv, &tz);
06187	        _daylight = tz.tz_dsttime;
06188	        _timezone = tz.tz_minuteswest * 60L;
06189	
06190	#elif   !defined(_POSIX_SOURCE) && !defined(__USG)
06191	
06192	#if     !defined(_MINIX)                /* MINIX has no ftime() */
06193	        struct timeb time;
06194	
06195	        _ftime(&time);
06196	        _timezone = time.timezone * 60L;
06197	        _daylight = time.dstflag;
06198	#endif
06199	
06200	#endif  /* !_POSIX_SOURCE && !__USG */
06201	
06202	        parseTZ(getenv("TZ"));          /* should go inside #if */
06203	
06204	#if     defined(__USG) || defined(_POSIX_SOURCE)
06205	        tzname[0] = _tzname[0];
06206	        tzname[1] = _tzname[1];
06207	#if     defined(__USG)
06208	        timezone = _timezone;
06209	        daylight = _daylight;
06210	#endif
06211	#endif  /* __USG || _POSIX_SOURCE */
06212	}
	
06214	static int
06215	last_sunday(register int day, register struct tm *timep)
06216	{
06217	        int first = FIRSTSUNDAY(timep);
06218	
06219	        if (day >= 58 && LEAPYEAR(YEAR0 + timep->tm_year)) day++;
06220	        if (day < first) return first;
06221	        return day - (day - first) % 7;
06222	}
	
06224	static int
06225	date_of(register struct dsttype *dst, struct tm *timep)
06226	{
06227	        int leap = LEAPYEAR(YEAR0 + timep->tm_year);
06228	        int firstday, tmpday;
06229	        register int day, month;
06230	
06231	        if (dst->ds_type != 'M') {
06232	                return dst->ds_date[0] -
06233	                            (dst->ds_type == 'J'
06234	                                && leap
.Ep 52 src/lib/ansi/misc.c
06235	                                && dst->ds_date[0] < 58);
06236	        }
06237	        day = 0;
06238	        month = 1;
06239	        while (month < dst->ds_date[0]) {
06240	                day += _ytab[leap][month - 1];
06241	                month++;
06242	        }
06243	        firstday = (day + FIRSTDAYOF(timep)) % 7;
06244	        tmpday = day;
06245	        day += (dst->ds_date[2] - firstday + 7) % 7
06246	                + 7 * (dst->ds_date[1] - 1);
06247	        if (day >= tmpday + _ytab[leap][month]) day -= 7;
06248	        return day;
06249	}
	
06251	/*
06252	 * The default dst transitions are those for Western Europe (except Great
06253	 * Britain). 
06254	 */
06255	unsigned
06256	_dstget(register struct tm *timep)
06257	{
06258	        int begindst, enddst;
06259	        register struct dsttype *dsts = &dststart, *dste = &dstend;
06260	        int do_dst = 0;
06261	
06262	        if (_daylight == -1)
06263	                _tzset();
06264	
06265	        timep->tm_isdst = _daylight;
06266	        if (!_daylight) return 0;
06267	
06268	        if (dsts->ds_type != 'U')
06269	                begindst = date_of(dsts, timep);
06270	        else begindst = last_sunday(89, timep); /* last Sun before Apr */
06271	        if (dste->ds_type != 'U')
06272	                enddst = date_of(dste, timep);
06273	        else enddst = last_sunday(272, timep);  /* last Sun in Sep */
06274	
06275	        /* assume begindst != enddst (otherwise it would be no use) */
06276	        if (begindst < enddst) {                /* northern hemisphere */
06277	                if (timep->tm_yday > begindst && timep->tm_yday < enddst)
06278	                        do_dst = 1;
06279	        } else {                                /* southern hemisphere */
06280	                if (timep->tm_yday > begindst || timep->tm_yday < enddst)
06281	                        do_dst = 1;
06282	        }
06283	
06284	        if (!do_dst
06285	            && (timep->tm_yday == begindst || timep->tm_yday == enddst)) {
06286	                long dsttranssec;       /* transition when day is this old */
06287	                long cursec;
06288	
06289	                if (timep->tm_yday == begindst)
06290	                        dsttranssec = dsts->ds_sec;
06291	                else    dsttranssec = dste->ds_sec;
06292	                cursec = ((timep->tm_hour * 60) + timep->tm_min) * 60L
06293	                            + timep->tm_sec;
06294	
.Op 53 src/lib/ansi/misc.c
06295	                if ((timep->tm_yday == begindst && cursec >= dsttranssec)
06296	                    || (timep->tm_yday == enddst && cursec < dsttranssec))
06297	                        do_dst = 1;
06298	        }
06299	#if USE_TABLE
06300	        if (usetable) ZoneFromTable(_timezone);
06301	#endif
06302	        if (do_dst) return _dst_off;
06303	        timep->tm_isdst = 0;
06304	        return 0;
06305	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/mktime.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

06400	/*
06401	 * mktime - convert local time into calendar time
06402	 */
06403	/* $Header: mktime.c,v 1.5 91/04/22 13:20:54 ceriel Exp $ */
06404	
06405	/* Michael A. Temari <temari@ix.netcom.com>   03/01/96    */
06406	/*   -  fixed bug is structure fixup code                 */
06407	
06408	#include        <time.h>
06409	#include        <limits.h>
06410	#include        "loc_time.h"
06411	
06412	/* The code assumes that unsigned long can be converted to time_t.
06413	 * A time_t should not be wider than unsigned long, since this would mean
06414	 * that the check for overflow at the end could fail.
06415	 */
06416	time_t
06417	mktime(register struct tm *timep)
06418	{
06419	        register long day, year;
06420	        register int tm_year;
06421	        int yday, month;
06422	        register unsigned long seconds;
06423	        int overflow;
06424	        unsigned dst;
06425	
06426	        timep->tm_min += timep->tm_sec / 60;
06427	        timep->tm_sec %= 60;
06428	        if (timep->tm_sec < 0) {
06429	                timep->tm_sec += 60;
06430	                timep->tm_min--;
06431	        }
06432	        timep->tm_hour += timep->tm_min / 60;
06433	        timep->tm_min = timep->tm_min % 60;
06434	        if (timep->tm_min < 0) {
06435	                timep->tm_min += 60;
06436	                timep->tm_hour--;
06437	        }
06438	        day = timep->tm_hour / 24;
06439	        timep->tm_hour= timep->tm_hour % 24;
.Ep 54 src/lib/ansi/mktime.c
06440	        if (timep->tm_hour < 0) {
06441	                timep->tm_hour += 24;
06442	                day--;
06443	        }
06444	        timep->tm_year += timep->tm_mon / 12;
06445	        timep->tm_mon %= 12;
06446	        if (timep->tm_mon < 0) {
06447	                timep->tm_mon += 12;
06448	                timep->tm_year--;
06449	        }
06450	        day += (timep->tm_mday - 1);
06451	        while (day < 0) {
06452	                if(--timep->tm_mon < 0) {
06453	                        timep->tm_year--;
06454	                        timep->tm_mon = 11;
06455	                }
06456	                day += _ytab[LEAPYEAR(YEAR0 + timep->tm_year)][timep->tm_mon];
06457	        }
06458	        while (day >= _ytab[LEAPYEAR(YEAR0 + timep->tm_year)][timep->tm_mon]) {
06459	                day -= _ytab[LEAPYEAR(YEAR0 + timep->tm_year)][timep->tm_mon];
06460	                if (++(timep->tm_mon) == 12) {
06461	                        timep->tm_mon = 0;
06462	                        timep->tm_year++;
06463	                }
06464	        }
06465	        timep->tm_mday = day + 1;
06466	        _tzset();                       /* set timezone and dst info  */
06467	        year = EPOCH_YR;
06468	        if (timep->tm_year < year - YEAR0) return (time_t)-1;
06469	        seconds = 0;
06470	        day = 0;                        /* means days since day 0 now */
06471	        overflow = 0;
06472	
06473	        /* Assume that when day becomes negative, there will certainly
06474	         * be overflow on seconds.
06475	         * The check for overflow needs not to be done for leapyears
06476	         * divisible by 400.
06477	         * The code only works when year (1970) is not a leapyear.
06478	         */
06479	#if     EPOCH_YR != 1970
06480	#error  EPOCH_YR != 1970
06481	#endif
06482	        tm_year = timep->tm_year + YEAR0;
06483	
06484	        if (LONG_MAX / 365 < tm_year - year) overflow++;
06485	        day = (tm_year - year) * 365;
06486	        if (LONG_MAX - day < (tm_year - year) / 4 + 1) overflow++;
06487	        day += (tm_year - year) / 4
06488	                + ((tm_year % 4) && tm_year % 4 < year % 4);
06489	        day -= (tm_year - year) / 100
06490	                + ((tm_year % 100) && tm_year % 100 < year % 100);
06491	        day += (tm_year - year) / 400
06492	                + ((tm_year % 400) && tm_year % 400 < year % 400);
06493	
06494	        yday = month = 0;
06495	        while (month < timep->tm_mon) {
06496	                yday += _ytab[LEAPYEAR(tm_year)][month];
06497	                month++;
06498	        }
06499	        yday += (timep->tm_mday - 1);
.Op 55 src/lib/ansi/mktime.c
06500	        if (day + yday < 0) overflow++;
06501	        day += yday;
06502	
06503	        timep->tm_yday = yday;
06504	        timep->tm_wday = (day + 4) % 7;         /* day 0 was thursday (4) */
06505	
06506	        seconds = ((timep->tm_hour * 60L) + timep->tm_min) * 60L + timep->tm_sec;
06507	
06508	        if ((TIME_MAX - seconds) / SECS_DAY < day) overflow++;
06509	        seconds += day * SECS_DAY;
06510	
06511	        /* Now adjust according to timezone and daylight saving time */
06512	
06513	        if (((_timezone > 0) && (TIME_MAX - _timezone < seconds))
06514	            || ((_timezone < 0) && (seconds < -_timezone)))
06515	                overflow++;
06516	        seconds += _timezone;
06517	
06518	        if (timep->tm_isdst < 0)
06519	                dst = _dstget(timep);
06520	        else if (timep->tm_isdst)
06521	                dst = _dst_off;
06522	        else dst = 0;
06523	
06524	        if (dst > seconds) overflow++;  /* dst is always non-negative */
06525	        seconds -= dst;
06526	
06527	        if (overflow) return (time_t)-1;
06528	
06529	        if ((time_t)seconds != seconds) return (time_t)-1;
06530	        return (time_t)seconds;
06531	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/qsort.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

06600	/*
06601	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
06602	 * See the copyright notice in the ACK home directory, in the file "Copyright".
06603	 */
06604	/* $Header: qsort.c,v 1.3 90/08/28 14:03:24 eck Exp $ */
06605	
06606	#include        <stdlib.h>
06607	
06608	static  void qsort1(char *, char *, size_t);
06609	static  int (*qcompar)(const char *, const char *);
06610	static  void qexchange(char *, char *, size_t);
06611	static  void q3exchange(char *, char *, char *, size_t);
06612	
06613	void
06614	qsort(void *base, size_t nel, size_t width,
06615	      int (*compar)(const void *, const void *))
06616	{
06617	        /* when nel is 0, the expression '(nel - 1) * width' is wrong */
06618	        if (!nel) return;
06619	        qcompar = (int (*)(const char *, const char *)) compar;
.Ep 56 src/lib/ansi/qsort.c
06620	        qsort1(base, (char *)base + (nel - 1) * width, width);
06621	}
	
06623	static void
06624	qsort1(char *a1, char *a2, register size_t width)
06625	{
06626	        register char *left, *right;
06627	        register char *lefteq, *righteq;
06628	        int cmp;
06629	
06630	        for (;;) {
06631	                if (a2 <= a1) return;
06632	                left = a1;
06633	                right = a2;
06634	                lefteq = righteq = a1 + width * (((a2-a1)+width)/(2*width));
06635	                /*
06636	                   Pick an element in the middle of the array.
06637	                   We will collect the equals around it.
06638	                   "lefteq" and "righteq" indicate the left and right
06639	                   bounds of the equals respectively.
06640	                   Smaller elements end up left of it, larger elements end
06641	                   up right of it.
06642	                */
06643	again:
06644	                while (left < lefteq && (cmp = (*qcompar)(left, lefteq)) <= 0) {
06645	                        if (cmp < 0) {
06646	                                /* leave it where it is */
06647	                                left += width;
06648	                        }
06649	                        else {
06650	                                /* equal, so exchange with the element to
06651	                                   the left of the "equal"-interval.
06652	                                */
06653	                                lefteq -= width;
06654	                                qexchange(left, lefteq, width);
06655	                        }
06656	                }
06657	                while (right > righteq) {
06658	                        if ((cmp = (*qcompar)(right, righteq)) < 0) {
06659	                                /* smaller, should go to left part
06660	                                */
06661	                                if (left < lefteq) {
06662	                                        /* yes, we had a larger one at the
06663	                                           left, so we can just exchange
06664	                                        */
06665	                                        qexchange(left, right, width);
06666	                                        left += width;
06667	                                        right -= width;
06668	                                        goto again;
06669	                                }
06670	                                /* no more room at the left part, so we
06671	                                   move the "equal-interval" one place to the
06672	                                   right, and the smaller element to the
06673	                                   left of it.
06674	                                   This is best expressed as a three-way
06675	                                   exchange.
06676	                                */
06677	                                righteq += width;
06678	                                q3exchange(left, righteq, right, width);
06679	                                lefteq += width;
.Op 57 src/lib/ansi/qsort.c
06680	                                left = lefteq;
06681	                        }
06682	                        else if (cmp == 0) {
06683	                                /* equal, so exchange with the element to
06684	                                   the right of the "equal-interval"
06685	                                */
06686	                                righteq += width;
06687	                                qexchange(right, righteq, width);
06688	                        }
06689	                        else    /* just leave it */ right -= width;
06690	                }
06691	                if (left < lefteq) {
06692	                        /* larger element to the left, but no more room,
06693	                           so move the "equal-interval" one place to the
06694	                           left, and the larger element to the right
06695	                           of it.
06696	                        */
06697	                        lefteq -= width;
06698	                        q3exchange(right, lefteq, left, width);
06699	                        righteq -= width;
06700	                        right = righteq;
06701	                        goto again;
06702	                }
06703	                /* now sort the "smaller" part */
06704	                qsort1(a1, lefteq - width, width);
06705	                /* and now the larger, saving a subroutine call
06706	                   because of the for(;;)
06707	                */
06708	                a1 = righteq + width;
06709	        }
06710	        /*NOTREACHED*/
06711	}
	
06713	static void
06714	qexchange(register char *p, register char *q,
06715	          register size_t n)
06716	{
06717	        register int c;
06718	
06719	        while (n-- > 0) {
06720	                c = *p;
06721	                *p++ = *q;
06722	                *q++ = c;
06723	        }
06724	}
	
06726	static void
06727	q3exchange(register char *p, register char *q, register char *r,
06728	           register size_t n)
06729	{
06730	        register int c;
06731	
06732	        while (n-- > 0) {
06733	                c = *p;
06734	                *p++ = *r;
06735	                *r++ = *q;
06736	                *q++ = c;
06737	        }
06738	}
.Ep 58 src/lib/ansi/raise.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/raise.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

06800	/*
06801	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
06802	 * See the copyright notice in the ACK home directory, in the file "Copyright".
06803	 */
06804	/* $Header: raise.c,v 1.3 90/11/22 13:55:50 eck Exp $ */
06805	
06806	#if     defined(_POSIX_SOURCE)
06807	#include        <sys/types.h>
06808	#endif
06809	#include        <signal.h>
06810	
06811	int _kill(int pid, int sig);
06812	pid_t _getpid(void);
06813	
06814	int
06815	raise(int sig)
06816	{
06817	        if (sig < 0 || sig > _NSIG)
06818	                return -1;
06819	        return _kill(_getpid(), sig);
06820	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/rand.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

06900	/*
06901	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
06902	 * See the copyright notice in the ACK home directory, in the file "Copyright".
06903	 */
06904	/* $Header: rand.c,v 1.3 90/06/30 20:02:45 ceriel Exp $ */
06905	
06906	#include        <stdlib.h>
06907	
06908	static unsigned long int next = 1;
06909	
06910	int rand(void)
06911	{
06912	        next = next * 1103515245 + 12345;
06913	        return (unsigned int)(next/(2 * (RAND_MAX +1L)) % (RAND_MAX+1L));
06914	}
	
06916	void srand(unsigned int seed)
06917	{
06918	        next = seed;
06919	}
.Op 59 src/lib/ansi/setlocale.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/setlocale.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

07000	/*
07001	 * setlocale - set the programs locale
07002	 */
07003	/* $Header: setlocale.c,v 1.2 89/12/18 15:49:11 eck Exp $ */
07004	
07005	#include        <locale.h>
07006	#include        <string.h>
07007	
07008	struct lconv _lc;
07009	
07010	char *
07011	setlocale(int category, const char *locale)
07012	{
07013	        if (!locale) return "C";
07014	        if (*locale && strcmp(locale, "C")) return (char *)NULL;
07015	        
07016	        switch(category) {
07017	        case LC_ALL:
07018	        case LC_CTYPE:
07019	        case LC_COLLATE:
07020	        case LC_TIME:
07021	        case LC_NUMERIC:
07022	        case LC_MONETARY:
07023	                return *locale ? (char *)locale : "C";
07024	        default:
07025	                return (char *)NULL;
07026	        }
07027	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/sigmisc.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

07100	/*
07101	 * sigmisc.c - used to get a signal mask
07102	 */
07103	/* $Header: sigmisc.c,v 1.2 90/10/15 11:23:08 eck Exp $ */
07104	
07105	#if     defined(_POSIX_SOURCE)
07106	
07107	/* This can't be done in setjmp.e, since SIG_SETMASK is defined in
07108	 * <signal.h>. This is a C-file, which can't be included.
07109	 */
07110	
07111	#include        <sys/types.h>
07112	#include        <signal.h>
07113	#include        <stddef.h>
07114	
07115	int _sigprocmask(int, sigset_t *, sigset_t *);
07116	
07117	static void
07118	__testsigset(void) {
07119	        /* This switch compiles when a sigset_t has the right size. */
.Ep 60 src/lib/ansi/sigmisc.c
07120	        switch(0) {
07121	        case 0: 
07122	        case sizeof(sigset_t) <= sizeof(long): break;
07123	        }
07124	}
	
07126	void
07127	__newsigset(sigset_t *p)
07128	{
07129	        /* The SIG_SETMASK is not significant */
07130	        _sigprocmask(SIG_SETMASK, NULL, p);
07131	}
	
07133	void
07134	__oldsigset(sigset_t *p)
07135	{
07136	        _sigprocmask(SIG_SETMASK, p, NULL);
07137	}
07138	#endif  /* _POSIX_SOURCE */


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/signal.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

07200	/* SYSVR4 and ANSI compatible signal(2). */
07201	
07202	#include <lib.h>
07203	#define sigaction       _sigaction
07204	#define sigemptyset     _sigemptyset
07205	#include <signal.h>
07206	
07207	PUBLIC sighandler_t signal(sig, disp)
07208	int sig;                        /* signal number */
07209	sighandler_t disp;              /* signal handler, or SIG_DFL, or SIG_IGN */
07210	{
07211	  struct sigaction sa, osa;
07212	
07213	  if (sig <= 0 || sig > _NSIG || sig == SIGKILL) {
07214	        errno = EINVAL;
07215	        return(SIG_ERR);
07216	  }
07217	  sigemptyset(&sa.sa_mask);
07218	
07219	#ifdef WANT_UNRELIABLE_SIGNALS
07220	  /* Allow the signal being handled to interrupt the signal handler. */
07221	  sa.sa_flags = SA_NODEFER;
07222	
07223	  /* When signal is caught, reset signal handler to SIG_DFL for all but
07224	   * SIGILL and SIGTRAP.
07225	   */
07226	  if (sig != SIGILL && sig != SIGTRAP) sa.sa_flags |= SA_RESETHAND;
07227	#else
07228	  sa.sa_flags = 0;
07229	#endif
07230	
07231	  sa.sa_handler = disp;
07232	  if (sigaction(sig, &sa, &osa) < 0) return(SIG_ERR);
07233	  return(osa.sa_handler);
07234	}
.Op 61 src/lib/ansi/strcat.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/strcat.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

07300	/*
07301	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
07302	 * See the copyright notice in the ACK home directory, in the file "Copyright".
07303	 */
07304	/* $Header: strcat.c,v 1.2 90/05/31 18:33:06 ceriel Exp $ */
07305	
07306	#include        <string.h>
07307	
07308	char *
07309	strcat(char *ret, register const char *s2)
07310	{
07311	        register char *s1 = ret;
07312	
07313	        while (*s1++ != '\0')
07314	                /* EMPTY */ ;
07315	        s1--;
07316	        while (*s1++ = *s2++)
07317	                /* EMPTY */ ;
07318	        return ret;
07319	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/strchr.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

07400	/*
07401	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
07402	 * See the copyright notice in the ACK home directory, in the file "Copyright".
07403	 */
07404	/* $Header: strchr.c,v 1.3 90/08/28 13:53:00 eck Exp $ */
07405	
07406	#include        <string.h>
07407	
07408	char *
07409	strchr(register const char *s, register int c)
07410	{
07411	        c = (char) c;
07412	
07413	        while (c != *s) {
07414	                if (*s++ == '\0') return NULL;
07415	        }
07416	        return (char *)s;
07417	}
.Ep 62 src/lib/ansi/strcmp.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/strcmp.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

07500	/*
07501	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
07502	 * See the copyright notice in the ACK home directory, in the file "Copyright".
07503	 */
07504	/* $Id: strcmp.c,v 1.4 1994/06/24 11:56:43 ceriel Exp $ */
07505	
07506	#include        <string.h>
07507	
07508	int
07509	strcmp(register const char *s1, register const char *s2)
07510	{
07511	        while (*s1 == *s2++) {
07512	                if (*s1++ == '\0') {
07513	                        return 0;
07514	                }
07515	        }
07516	        if (*s1 == '\0') return -1;
07517	        if (*--s2 == '\0') return 1;
07518	        return (unsigned char) *s1 - (unsigned char) *s2;
07519	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/strcoll.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

07600	/*
07601	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
07602	 * See the copyright notice in the ACK home directory, in the file "Copyright".
07603	 */
07604	/* $Header: strcoll.c,v 1.2 90/08/28 13:53:23 eck Exp $ */
07605	
07606	#include        <string.h>
07607	#include        <locale.h>
07608	
07609	int
07610	strcoll(register const char *s1, register const char *s2)
07611	{
07612	        while (*s1 == *s2++) {
07613	                if (*s1++ == '\0') {
07614	                        return 0;
07615	                }
07616	        }
07617	        return *s1 - *--s2;
07618	}
.Op 63 src/lib/ansi/strcpy.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/strcpy.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

07700	/*
07701	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
07702	 * See the copyright notice in the ACK home directory, in the file "Copyright".
07703	 */
07704	/* $Header: strcpy.c,v 1.2 90/05/31 18:33:13 ceriel Exp $ */
07705	
07706	#include        <string.h>
07707	
07708	char *
07709	strcpy(char *ret, register const char *s2)
07710	{
07711	        register char *s1 = ret;
07712	
07713	        while (*s1++ = *s2++)
07714	                /* EMPTY */ ;
07715	
07716	        return ret;
07717	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/strcspn.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

07800	/*
07801	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
07802	 * See the copyright notice in the ACK home directory, in the file "Copyright".
07803	 */
07804	/* $Header: strcspn.c,v 1.2 89/12/18 16:01:42 eck Exp $ */
07805	
07806	#include        <string.h>
07807	
07808	size_t
07809	strcspn(const char *string, const char *notin)
07810	{
07811	        register const char *s1, *s2;
07812	
07813	        for (s1 = string; *s1; s1++) {
07814	                for(s2 = notin; *s2 != *s1 && *s2; s2++)
07815	                        /* EMPTY */ ;
07816	                if (*s2)
07817	                        break;
07818	        }
07819	        return s1 - string;
07820	}
.Ep 64 src/lib/ansi/strerror.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/strerror.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

07900	/*
07901	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
07902	 * See the copyright notice in the ACK home directory, in the file "Copyright".
07903	 */
07904	/* $Header: strerror.c,v 1.3 90/08/28 13:53:31 eck Exp $ */
07905	
07906	#include        <string.h>
07907	
07908	/*
07909	 * I don't know why, but X3J11 says that strerror() should be in declared
07910	 * in <string.h>.  That is why the function is defined here.
07911	 */
07912	char *
07913	strerror(register int errnum)
07914	{
07915	        extern const char *_sys_errlist[];
07916	        extern const int _sys_nerr;
07917	
07918	        if (errnum < 0 || errnum >= _sys_nerr)
07919	                return "unknown error";
07920	        return (char *)_sys_errlist[errnum];
07921	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/strftime.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

08000	/*
08001	 * strftime - convert a structure to a string, controlled by an argument
08002	 */
08003	/* $Header: strftime.c,v 1.3 91/04/22 13:21:03 ceriel Exp $ */
08004	
08005	#include        <time.h>
08006	#include        "loc_time.h"
08007	
08008	/* The width can be -1 in both s_prnt() as in u_prnt(). This
08009	 * indicates that as many characters as needed should be printed.
08010	 */
08011	static char *
08012	s_prnt(char *s, size_t maxsize, const char *str, int width)
08013	{
08014	        while (width > 0 || (width < 0 && *str)) {
08015	                if (!maxsize) break;
08016	                *s++ = *str++;
08017	                maxsize--;
08018	                width--;
08019	        }
08020	        return s;
08021	}
	
08023	static char *
08024	u_prnt(char *s, size_t maxsize, unsigned val, int width)
.Op 65 src/lib/ansi/strftime.c
08025	{
08026	        int c;
08027	
08028	        c = val % 10;
08029	        val = val / 10;
08030	        if (--width > 0 || (width < 0 && val != 0))
08031	                s = u_prnt(s, (maxsize ? maxsize - 1 : 0), val, width);
08032	        if (maxsize) *s++ = c + '0';
08033	        return s;
08034	}
	
08036	size_t
08037	strftime(char *s, size_t maxsize,
08038	                const char *format, const struct tm *timeptr)
08039	{
08040	        size_t n;
08041	        char *firsts, *olds;
08042	
08043	        if (!format) return 0;
08044	
08045	        _tzset();       /* for %Z conversion */
08046	        firsts = s;
08047	        while (maxsize && *format) {
08048	                while (maxsize && *format && *format != '%') {
08049	                        *s++ = *format++;
08050	                        maxsize--;
08051	                }
08052	                if (!maxsize || !*format) break;
08053	                format++;
08054	
08055	                olds = s;
08056	                switch (*format++) {
08057	                case 'a':
08058	                        s = s_prnt(s, maxsize,
08059	                                        _days[timeptr->tm_wday], ABB_LEN);
08060	                        maxsize -= s - olds;
08061	                        break;
08062	                case 'A':
08063	                        s = s_prnt(s, maxsize, _days[timeptr->tm_wday], -1);
08064	                        maxsize -= s - olds;
08065	                        break;
08066	                case 'b':
08067	                        s = s_prnt(s, maxsize,
08068	                                        _months[timeptr->tm_mon], ABB_LEN);
08069	                        maxsize -= s - olds;
08070	                        break;
08071	                case 'B':
08072	                        s = s_prnt(s, maxsize, _months[timeptr->tm_mon], -1);
08073	                        maxsize -= s - olds;
08074	                        break;
08075	                case 'c':
08076	                        n = strftime(s, maxsize,
08077	                                        "%a %b %d %H:%M:%S %Y", timeptr);
08078	                        if (n) maxsize -= n;
08079	                        else maxsize = 0;
08080	                        s += n;
08081	                        break;
08082	                case 'd':
08083	                        s = u_prnt(s, maxsize, timeptr->tm_mday, 2);
08084	                        maxsize -= s - olds;
.Ep 66 src/lib/ansi/strftime.c
08085	                        break;
08086	                case 'H':
08087	                        s = u_prnt(s, maxsize, timeptr->tm_hour, 2);
08088	                        maxsize -= s - olds;
08089	                        break;
08090	                case 'I':
08091	                        s = u_prnt(s, maxsize,
08092	                                        (timeptr->tm_hour + 11) % 12 + 1, 2);
08093	                        maxsize -= s - olds;
08094	                        break;
08095	                case 'j':
08096	                        s = u_prnt(s, maxsize, timeptr->tm_yday + 1, 3);
08097	                        maxsize -= s - olds;
08098	                        break;
08099	                case 'm':
08100	                        s = u_prnt(s, maxsize, timeptr->tm_mon + 1, 2);
08101	                        maxsize -= s - olds;
08102	                        break;
08103	                case 'M':
08104	                        s = u_prnt(s, maxsize, timeptr->tm_min, 2);
08105	                        maxsize -= s - olds;
08106	                        break;
08107	                case 'p':
08108	                        s = s_prnt(s, maxsize,
08109	                                    (timeptr->tm_hour < 12) ? "AM" : "PM", 2);
08110	                        maxsize -= s - olds;
08111	                        break;
08112	                case 'S':
08113	                        s = u_prnt(s, maxsize, timeptr->tm_sec, 2);
08114	                        maxsize -= s - olds;
08115	                        break;
08116	                case 'U':
08117	                        s = u_prnt(s, maxsize,          /* ??? */
08118	                            (timeptr->tm_yday + 7 - timeptr->tm_wday) / 7, 2);
08119	                        maxsize -= s - olds;
08120	                        break;
08121	                case 'w':
08122	                        s = u_prnt(s, maxsize, timeptr->tm_wday, 1);
08123	                        maxsize -= s - olds;
08124	                        break;
08125	                case 'W':
08126	                        s = u_prnt(s, maxsize,          /* ??? */
08127	                            (timeptr->tm_yday+7-(timeptr->tm_wday+6)%7)/7,2);
08128	                        maxsize -= s - olds;
08129	                        break;
08130	                case 'x':
08131	                        n = strftime(s, maxsize, "%a %b %d %Y", timeptr);
08132	                        if (n) maxsize -= n;
08133	                        else maxsize = 0;
08134	                        s += n;
08135	                        break;
08136	                case 'X':
08137	                        n = strftime(s, maxsize, "%H:%M:%S", timeptr);
08138	                        if (n) maxsize -= n;
08139	                        else maxsize = 0;
08140	                        s += n;
08141	                        break;
08142	                case 'y':
08143	                        s = u_prnt(s, maxsize, timeptr->tm_year % 100, 2);
08144	                        maxsize -= s - olds;
.Op 67 src/lib/ansi/strftime.c
08145	                        break;
08146	                case 'Y':
08147	                        s = u_prnt(s, maxsize, timeptr->tm_year + YEAR0, -1);
08148	                        maxsize -= s - olds;
08149	                        break;
08150	                case 'Z':
08151	                        s = s_prnt(s, maxsize,
08152	                                        _tzname[(timeptr->tm_isdst > 0)], -1);
08153	                        maxsize -= s - olds;
08154	                        break;
08155	                case '%':
08156	                        *s++ = '%';
08157	                        maxsize--;
08158	                        break;
08159	                default:
08160	                        /* A conversion error. Leave the loop. */
08161	                        while (*format) format++;
08162	                        break;
08163	                }
08164	
08165	        }
08166	        if (maxsize) {
08167	                *s = '\0';
08168	                return s - firsts;
08169	        }
08170	        return 0;       /* The buffer is full */
08171	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/strlen.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

08200	/*
08201	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
08202	 * See the copyright notice in the ACK home directory, in the file "Copyright".
08203	 */
08204	/* $Header: strlen.c,v 1.4 90/08/28 13:53:42 eck Exp $ */
08205	
08206	#include        <string.h>
08207	
08208	size_t
08209	strlen(const char *org)
08210	{
08211	        register const char *s = org;
08212	
08213	        while (*s++)
08214	                /* EMPTY */ ;
08215	
08216	        return --s - org;
08217	}
.Ep 68 src/lib/ansi/strncat.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/strncat.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

08300	/*
08301	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
08302	 * See the copyright notice in the ACK home directory, in the file "Copyright".
08303	 */
08304	/* $Header: strncat.c,v 1.3 90/08/28 13:53:54 eck Exp $ */
08305	
08306	#include        <string.h>
08307	
08308	char *
08309	strncat(char *ret, register const char *s2, size_t n)
08310	{
08311	        register char *s1 = ret;
08312	
08313	        if (n > 0) {
08314	                while (*s1++)
08315	                        /* EMPTY */ ;
08316	                s1--;
08317	                while (*s1++ = *s2++)  {
08318	                        if (--n > 0) continue;
08319	                        *s1 = '\0';
08320	                        break;
08321	                }
08322	                return ret;
08323	        } else return s1;
08324	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/strncmp.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

08400	/*
08401	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
08402	 * See the copyright notice in the ACK home directory, in the file "Copyright".
08403	 */
08404	/* $Id: strncmp.c,v 1.4 1994/06/24 11:57:04 ceriel Exp $ */
08405	
08406	#include        <string.h>
08407	
08408	int
08409	strncmp(register const char *s1, register const char *s2, register size_t n)
08410	{
08411	        if (n) {
08412	                do {
08413	                        if (*s1 != *s2++)
08414	                                break;
08415	                        if (*s1++ == '\0')
08416	                                return 0;
08417	                } while (--n > 0);
08418	                if (n > 0) {
08419	                        if (*s1 == '\0') return -1;
.Op 69 src/lib/ansi/strncmp.c
08420	                        if (*--s2 == '\0') return 1;
08421	                        return (unsigned char) *s1 - (unsigned char) *s2;
08422	                }
08423	        }
08424	        return 0;
08425	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/strncpy.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

08500	/*
08501	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
08502	 * See the copyright notice in the ACK home directory, in the file "Copyright".
08503	 */
08504	/* $Header: strncpy.c,v 1.3 90/08/28 13:54:11 eck Exp $ */
08505	
08506	#include        <string.h>
08507	
08508	char *
08509	strncpy(char *ret, register const char *s2, register size_t n)
08510	{
08511	        register char *s1 = ret;
08512	
08513	        if (n>0) {
08514	                while((*s1++ = *s2++) && --n > 0)
08515	                        /* EMPTY */ ;
08516	                if ((*--s2 == '\0') && --n > 0) {
08517	                        do {
08518	                                *s1++ = '\0';
08519	                        } while(--n > 0);
08520	                }
08521	        }
08522	        return ret;
08523	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/strpbrk.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

08600	/*
08601	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
08602	 * See the copyright notice in the ACK home directory, in the file "Copyright".
08603	 */
08604	/* $Header: strpbrk.c,v 1.2 89/12/18 16:02:21 eck Exp $ */
08605	
08606	#include        <string.h>
08607	
08608	char *
08609	strpbrk(register const char *string, register const char *brk)
08610	{
08611	        register const char *s1;
08612	
08613	        while (*string) {
08614	                for (s1 = brk; *s1 && *s1 != *string; s1++)
.Ep 70 src/lib/ansi/strpbrk.c
08615	                        /* EMPTY */ ;
08616	                if (*s1)
08617	                        return (char *)string;
08618	                string++;
08619	        }
08620	        return (char *)NULL;
08621	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/strrchr.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

08700	/*
08701	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
08702	 * See the copyright notice in the ACK home directory, in the file "Copyright".
08703	 */
08704	/* $Header: strrchr.c,v 1.3 90/08/28 13:54:21 eck Exp $ */
08705	
08706	#include        <string.h>
08707	
08708	char *
08709	strrchr(register const char *s, int c)
08710	{
08711	        register const char *result = NULL;
08712	
08713	        c = (char) c;
08714	
08715	        do {
08716	                if (c == *s)
08717	                        result = s;
08718	        } while (*s++ != '\0');
08719	
08720	        return (char *)result;
08721	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/strspn.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

08800	/*
08801	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
08802	 * See the copyright notice in the ACK home directory, in the file "Copyright".
08803	 */
08804	/* $Header: strspn.c,v 1.1 89/05/11 10:09:09 eck Exp $ */
08805	
08806	#include        <string.h>
08807	
08808	size_t
08809	strspn(const char *string, const char *in)
08810	{
08811	        register const char *s1, *s2;
08812	
08813	        for (s1 = string; *s1; s1++) {
08814	                for (s2 = in; *s2 && *s2 != *s1; s2++)
.Op 71 src/lib/ansi/strspn.c
08815	                        /* EMPTY */ ;
08816	                if (*s2 == '\0')
08817	                        break;
08818	        }
08819	        return s1 - string;
08820	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/strstr.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

08900	/*
08901	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
08902	 * See the copyright notice in the ACK home directory, in the file "Copyright".
08903	 */
08904	/* $Header: strstr.c,v 1.3 90/08/28 13:54:28 eck Exp $ */
08905	
08906	#include        <string.h>
08907	
08908	char *
08909	strstr(register const char *s, register const char *wanted)
08910	{
08911	        register const size_t len = strlen(wanted);
08912	
08913	        if (len == 0) return (char *)s;
08914	        while (*s != *wanted || strncmp(s, wanted, len))
08915	                if (*s++ == '\0')
08916	                        return (char *)NULL;
08917	        return (char *)s;
08918	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/strtok.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

09000	/*
09001	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
09002	 * See the copyright notice in the ACK home directory, in the file "Copyright".
09003	 */
09004	/* $Header: strtok.c,v 1.2 90/08/28 13:54:38 eck Exp $ */
09005	
09006	#include        <string.h>
09007	
09008	char *
09009	strtok(register char *string, const char *separators)
09010	{
09011	        register char *s1, *s2;
09012	        static char *savestring;
09013	
09014	        if (string == NULL) {
09015	                string = savestring;
09016	                if (string == NULL) return (char *)NULL;
09017	        }
09018	
09019	        s1 = string + strspn(string, separators);
.Ep 72 src/lib/ansi/strtok.c
09020	        if (*s1 == '\0') {
09021	                savestring = NULL;
09022	                return (char *)NULL;
09023	        }
09024	
09025	        s2 = strpbrk(s1, separators);
09026	        if (s2 != NULL)
09027	                *s2++ = '\0';
09028	        savestring = s2;
09029	        return s1;
09030	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/strtol.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

09100	/*
09101	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
09102	 * See the copyright notice in the ACK home directory, in the file "Copyright".
09103	 */
09104	/* $Header: strtol.c,v 1.4 90/05/11 15:22:19 eck Exp $ */
09105	
09106	#include        <ctype.h>
09107	#include        <errno.h>
09108	#include        <limits.h>
09109	#include        <stdlib.h>
09110	
09111	static unsigned long
09112	string2long(register const char *nptr, char **endptr,
09113	                        int base, int is_signed);
09114	
09115	long int
09116	strtol(register const char *nptr, char **endptr, int base)
09117	{
09118	        return (signed long)string2long(nptr, endptr, base, 1);
09119	}
	
09121	unsigned long int
09122	strtoul(register const char *nptr, char **endptr, int base)
09123	{
09124	        return (unsigned long)string2long(nptr, endptr, base, 0);
09125	}
	
09127	#define between(a, c, z)  ((unsigned) ((c) - (a)) <= (unsigned) ((z) - (a)))
09128	
09129	static unsigned long
09130	string2long(register const char *nptr, char ** const endptr,
09131	                        int base, int is_signed)
09132	{
09133	        register unsigned int v;
09134	        register unsigned long val = 0;
09135	        register int c;
09136	        int ovfl = 0, sign = 1;
09137	        const char *startnptr = nptr, *nrstart;
09138	
09139	        if (endptr) *endptr = (char *)nptr;
.Op 73 src/lib/ansi/strtol.c
09140	        while (isspace(*nptr)) nptr++;
09141	        c = *nptr;
09142	
09143	        if (c == '-' || c == '+') {
09144	                if (c == '-') sign = -1;
09145	                nptr++;
09146	        }
09147	        nrstart = nptr;                 /* start of the number */
09148	
09149	        /* When base is 0, the syntax determines the actual base */
09150	        if (base == 0)
09151	                if (*nptr == '0')
09152	                        if (*++nptr == 'x' || *nptr == 'X') {
09153	                                base = 16;
09154	                                nptr++;
09155	                        }
09156	                        else    base = 8;
09157	                else    base = 10;
09158	        else if (base==16 && *nptr=='0' && (*++nptr =='x' || *nptr =='X'))
09159	                nptr++;
09160	
09161	        for (;;) {
09162	                c = *nptr;
09163	                if (between('0', c, '9')) {
09164	                        v = c - '0';
09165	                } else
09166	                if (between('a', c, 'z')) {
09167	                        v = c - 'a' + 0xa;
09168	                } else
09169	                if (between('A', c, 'Z')) {
09170	                        v = c - 'A' + 0xA;
09171	                } else {
09172	                        break;
09173	                }
09174	                if (v >= base) break;
09175	                if (val > (ULONG_MAX - v) / base) ovfl++;
09176	                val = (val * base) + v;
09177	                nptr++;
09178	        }
09179	        if (endptr) {
09180	                if (nrstart == nptr) *endptr = (char *)startnptr;
09181	                else *endptr = (char *)nptr;
09182	        }
09183	
09184	        if (!ovfl) {
09185	                /* Overflow is only possible when converting a signed long. */
09186	                if (is_signed
09187	                    && (   (sign < 0 && val > -(unsigned long)LONG_MIN)
09188	                        || (sign > 0 && val > LONG_MAX)))
09189	                    ovfl++;
09190	        }
09191	
09192	        if (ovfl) {
09193	                errno = ERANGE;
09194	                if (is_signed)
09195	                        if (sign < 0) return LONG_MIN;
09196	                        else return LONG_MAX;
09197	                else return ULONG_MAX;
09198	        }
09199	        return (long) sign * val;
.Ep 74 src/lib/ansi/strtol.c
09200	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/strxfrm.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

09300	/*
09301	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
09302	 * See the copyright notice in the ACK home directory, in the file "Copyright".
09303	 */
09304	/* $Header: strxfrm.c,v 1.4 90/08/28 13:54:46 eck Exp $ */
09305	
09306	#include        <string.h>
09307	
09308	size_t
09309	strxfrm(register char *s1, register const char *save, register size_t n)
09310	{
09311	        register const char *s2 = save;
09312	
09313	        while (*s2) {
09314	                if (n > 1) {
09315	                        n--;
09316	                        *s1++ = *s2++;
09317	                } else
09318	                        s2++;
09319	        }
09320	        if (n > 0)
09321	                *s1++ = '\0';
09322	        return s2 - save;
09323	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/system.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

09400	/*
09401	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
09402	 * See the copyright notice in the ACK home directory, in the file "Copyright".
09403	 */
09404	/* $Header: system.c,v 1.4 90/11/22 13:59:54 eck Exp $ */
09405	
09406	#if     defined(_POSIX_SOURCE)
09407	#include        <sys/types.h>
09408	#endif
09409	#include        <stdlib.h>
09410	#include        <signal.h>
09411	
09412	extern pid_t _fork(void);
09413	extern pid_t _wait(int *);
09414	extern void _exit(int);
09415	extern void _execve(const char *path, const char ** argv, const char ** envp);
09416	extern int _close(int);
09417	
09418	#define FAIL    127
09419	
.Op 75 src/lib/ansi/system.c
09420	extern const char **_penvp;
09421	static const char *exec_tab[] = {
09422	        "sh",                   /* argv[0] */
09423	        "-c",                   /* argument to the shell */
09424	        NULL,                   /* to be filled with user command */
09425	        NULL                    /* terminating NULL */
09426	        };
09427	
09428	int
09429	system(const char *str)
09430	{
09431	        int pid, exitstatus, waitval;
09432	        int i;
09433	
09434	        if ((pid = _fork()) < 0) return str ? -1 : 0;
09435	
09436	        if (pid == 0) {
09437	                for (i = 3; i <= 20; i++)
09438	                        _close(i);
09439	                if (!str) str = "cd .";         /* just testing for a shell */
09440	                exec_tab[2] = str;              /* fill in command */
09441	                _execve("/bin/sh", exec_tab, _penvp);
09442	                /* get here if execve fails ... */
09443	                _exit(FAIL);    /* see manual page */
09444	        }
09445	        while ((waitval = _wait(&exitstatus)) != pid) {
09446	                if (waitval == -1) break;
09447	        }
09448	        if (waitval == -1) {
09449	                /* no child ??? or maybe interrupted ??? */
09450	                exitstatus = -1;
09451	        }
09452	        if (!str) {
09453	                if (exitstatus == FAIL << 8)            /* execve() failed */
09454	                        exitstatus = 0;
09455	                else exitstatus = 1;                    /* /bin/sh exists */
09456	        }
09457	        return exitstatus;
09458	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/tolower.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

09500	#include        <ctype.h>
09501	
09502	int tolower(int c) {
09503	        return isupper(c) ? c - 'A' + 'a' : c ;
09504	}
.Ep 76 src/lib/ansi/toupper.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/toupper.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

09600	#include        <ctype.h>
09601	
09602	int toupper(int c) {
09603	        return islower(c) ? c - 'a' + 'A' : c ;
09604	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/tzset.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

09700	/*
09701	 * tzset - set timezone information
09702	 */
09703	/* $Header: tzset.c,v 1.3 91/04/22 13:21:11 ceriel Exp $ */
09704	
09705	/* This function is present for System V && POSIX */
09706	
09707	#include        <time.h>
09708	#include        "loc_time.h"
09709	
09710	void
09711	tzset(void)
09712	{
09713	        _tzset();       /* does the job */
09714	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/wcstombs.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

09800	/*
09801	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
09802	 * See the copyright notice in the ACK home directory, in the file "Copyright".
09803	 */
09804	/* $Header: wcstombs.c,v 1.3 90/03/28 16:37:07 eck Exp $ */
09805	
09806	#include        <stdlib.h>
09807	#include        <locale.h>
09808	#include        <limits.h>
09809	
09810	size_t
09811	wcstombs(register char *s, register const wchar_t *pwcs, size_t n)
09812	{
09813	        register int i = n;
09814	
.Op 77 src/lib/ansi/wcstombs.c
09815	        while (--i >= 0) {
09816	                if (!(*s++ = *pwcs++))
09817	                        break;
09818	        }
09819	        return n - i - 1;
09820	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ansi/wctomb.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

09900	/*
09901	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
09902	 * See the copyright notice in the ACK home directory, in the file "Copyright".
09903	 */
09904	/* $Header: wctomb.c,v 1.4 91/01/15 11:55:33 ceriel Exp $ */
09905	
09906	#include        <stdlib.h>
09907	#include        <limits.h>
09908	
09909	int
09910	/* was: wctomb(char *s, wchar_t wchar) 
09911	 * This conflicts with prototype, so it was changed to:
09912	 */
09913	wctomb(char *s, wchar_t wchar)
09914	{
09915	        if (!s) return 0;               /* no state dependent codings */
09916	
09917	        *s = wchar;
09918	        return 1;
09919	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/curspriv.h	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

10000	/* Constants */
10001	#define _SUBWIN         1               /* window is a subwindow */
10002	#define _ENDLINE        2               /* last winline is last screen line */
10003	#define _FULLWIN        4               /* window fills screen */
10004	#define _SCROLLWIN      8               /* window lwr rgt is screen lwr rgt */
10005	
10006	#define _NO_CHANGE      -1              /* flags line edge unchanged */
10007	#define _BREAKCHAR      0x03            /* ^C character */
10008	#define _DCCHAR         0x08            /* Delete Char char (BS) */
10009	#define _DLCHAR         0x1b            /* Delete Line char (ESC) */
10010	#define _GOCHAR         0x11            /* ^Q character */
10011	#define _PRINTCHAR      0x10            /* ^P character */
10012	#define _STOPCHAR       0x13            /* ^S character */
10013	#define  NUNGETCH       10              /* max # chars to ungetch() */
10014	
.Ep 78 src/lib/curses/curspriv.h
10015	#define max(a,b) (((a) > (b)) ? (a) : (b))
10016	#define min(a,b) (((a) < (b)) ? (a) : (b))
10017	
10018	/* Character mask definitions. */
10019	#define CHR_MSK ((int) 0x00ff)          /* ASCIIZ character mask */
10020	#define ATR_MSK ((int) 0xff00)          /* attribute mask */
10021	#define ATR_NRM ((int) 0x0000)          /* no special attributes */
10022	
10023	/* Type declarations. */
10024	
10025	typedef struct {
10026	  WINDOW  *tmpwin;                      /* window used for updates */
10027	  int      cursrow;                     /* position of physical cursor */
10028	  int      curscol;
10029	  bool     rawmode;
10030	  bool     cbrkmode;
10031	  bool     echoit;
10032	} cursv;
10033	
10034	/* External variables */
10035	extern  cursv   _cursvar;               /* curses variables */





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/beep.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

10100	#include <curses.h>
10101	#include "curspriv.h"
10102	#include <termcap.h>
10103	
10104	extern char *bl, *vb;
10105	
10106	/* Beep() sounds the terminal bell. */
10107	void beep()
10108	{
10109	  if (bl)
10110	        tputs(bl, 1, outc);
10111	  else if (vb)
10112	        tputs(vb, 1, outc);
10113	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/charpick.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

10200	#include <curses.h>
10201	#include "curspriv.h"
10202	
10203	/****************************************************************/
10204	/* Winch(win) returns the character at the current position in  */
10205	/* Window 'win'.                                                */
10206	/****************************************************************/
10207	
10208	int winch(win)
10209	WINDOW *win;
.Op 79 src/lib/curses/charpick.c
10210	{
10211	  return((win->_line[win->_cury][win->_curx]) & 0xff);
10212	}                               /* winch */
10213	
10214	/****************************************************************/
10215	/* Mvinch() moves the stdscr cursor to a new position, then     */
10216	/* Returns the character at that position.                      */
10217	/****************************************************************/
10218	
10219	int mvinch(y, x)
10220	int y;
10221	int x;
10222	{
10223	  if (wmove(stdscr, y, x) == ERR) return(ERR);
10224	  return((stdscr->_line[stdscr->_cury][stdscr->_curx]) & 0xff);
10225	}
	
10227	/****************************************************************/
10228	/* Mvwinch() moves the cursor of window 'win' to a new posi-    */
10229	/* Tion, then returns the character at that position.           */
10230	/****************************************************************/
10231	
10232	int mvwinch(win, y, x)
10233	WINDOW *win;
10234	int y;
10235	int x;
10236	{
10237	  if (wmove(win, y, x) == ERR) return(ERR);
10238	  return((win->_line[win->_cury][win->_curx]) & 0xff);
10239	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/curs_set.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

10300	#include <curses.h>
10301	#include "curspriv.h"
10302	#include <termcap.h>
10303	
10304	extern char *vi, *ve, *vs;
10305	
10306	/* Sets cursor visibility to unvisible=0; normal visible=1 or very good
10307	 * visible=2. 
10308	*/
10309	void curs_set(visibility)
10310	int visibility;
10311	{
10312	  switch (visibility) {
10313	      case 0:
10314	        if (vi) tputs(vi, 1, outc);
10315	        break;
10316	      case 1:
10317	        if (ve) tputs(ve, 1, outc);
10318	        break;
10319	      case 2:
.Ep 80 src/lib/curses/curs_set.c
10320	        if (vs)
10321	                tputs(vs, 1, outc);
10322	        else if (ve)
10323	                tputs(ve, 1, outc);
10324	  }
10325	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/cursesio.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

10400	#include <stdlib.h>
10401	#include <termcap.h>
10402	#include <sys/types.h>
10403	#include <sys/ioctl.h>
10404	#include <curses.h>
10405	#include "curspriv.h"
10406	
10407	struct termios _orig_tty, _tty;
10408	cursv _cursvar;
10409	
10410	WINDOW *stdscr, *curscr;
10411	int LINES, COLS;
10412	bool NONL;
10413	
10414	char termcap[1024];             /* termcap buffer */
10415	char tc[200];                   /* area to hold string capabilities */
10416	char *ttytype;                  /* terminal type from env */
10417	char *arp;                      /* pointer for use in tgetstr */
10418	char *cp;                       /* character pointer */
10419	
10420	char *cl;                       /* clear screen capability */
10421	char *cm;                       /* cursor motion capability */
10422	char *so;                       /* start standout capability */
10423	char *se;                       /* end standout capability */
10424	char *mr;                       /* start of reverse */
10425	char *me;                       /* revert to normal */
10426	char *mb;                       /* start of blink */
10427	char *md;                       /* start of bold */
10428	char *us;                       /* start of underscore */
10429	char *ue;                       /* end of underscore */
10430	char *vi;                       /* cursor invisible */
10431	char *ve;                       /* cursor normal */
10432	char *vs;                       /* cursor good visible */
10433	char *as;                       /* alternative charset start */
10434	char *ae;                       /* alternative charset end */
10435	char *bl;                       /* ring the bell */
10436	char *vb;                       /* visual bell */
10437	
10438	/* fatal - report error and die. Never returns */
10439	void fatal(s)
10440	char *s;
10441	{
10442	  (void) fprintf(stderr, "curses: %s\n", s);
10443	  exit(1);
10444	}
.Op 81 src/lib/curses/cursesio.c
	
10446	/* Outc - call putchar, necessary because putchar is a macro. */
10447	void outc(c)
10448	int c;
10449	{
10450	  putchar(c);
10451	}
	
10453	/* Move cursor to r,c */
10454	void poscur(r, c)
10455	int r, c;
10456	{
10457	  tputs(tgoto(cm, c, r), 1, outc);
10458	}
	
10460	/* Clear the screen */
10461	void clrscr()
10462	{
10463	  tputs(cl, 1, outc);
10464	}
	
10466	/* This are terminal independent characters which can be used in curses */
10467	
10468	unsigned int ACS_ULCORNER;
10469	unsigned int ACS_LLCORNER;
10470	unsigned int ACS_URCORNER;
10471	unsigned int ACS_LRCORNER;
10472	unsigned int ACS_RTEE;
10473	unsigned int ACS_LTEE;
10474	unsigned int ACS_BTEE;
10475	unsigned int ACS_TTEE;
10476	unsigned int ACS_HLINE;
10477	unsigned int ACS_VLINE;
10478	unsigned int ACS_PLUS;
10479	unsigned int ACS_S1;
10480	unsigned int ACS_S9;
10481	unsigned int ACS_DIAMOND;
10482	unsigned int ACS_CKBOARD;
10483	unsigned int ACS_DEGREE;
10484	unsigned int ACS_PLMINUS;
10485	unsigned int ACS_BULLET;
10486	unsigned int ACS_LARROW;
10487	unsigned int ACS_RARROW;
10488	unsigned int ACS_DARROW;
10489	unsigned int ACS_UARROW;
10490	unsigned int ACS_BOARD;
10491	unsigned int ACS_LANTERN;
10492	unsigned int ACS_BLOCK;
10493	
10494	/* These defines describe the full set of grafic block characters which
10495	 * can be defined via termcap.
10496	 */
10497	
10498	#define RIGHTARROW  0
10499	#define LEFTARROW   1
10500	#define DOWNARROW   2
10501	#define UPARROW     3
10502	#define FULLSQUARE  4
10503	#define GREYSQUARE  5
10504	#define EMPTYSQUARE 6
.Ep 82 src/lib/curses/cursesio.c
10505	#define LATERN      7
10506	#define DIAMOND     8
10507	#define DEGREE      9
10508	#define PLUSMINUS  10
10509	#define DOWNRIGHT  11
10510	#define UPRIGHT    12
10511	#define UPLEFT     13
10512	#define DOWNLEFT   14
10513	#define CROSS      15
10514	#define UPLINE     16
10515	#define UPMIDLINE  17
10516	#define MIDLINE    18
10517	#define DOMIDLINE  19
10518	#define DOWNLINE   20
10519	#define TEELEFT    21
10520	#define TEERIGHT   22
10521	#define TEEHEAD    23
10522	#define TEENORMAL  24
10523	#define VERTLINE   25
10524	#define PARAGRAPH  26
10525	
10526	unsigned int _cursgraftable[27] =
10527	{
10528	 '>', '<', 'v', '^', '#', ':', ' ', '#', '+', '\'', '#', '+', '+',
10529	 '+', '+', '+', '-', ' ', '-', ' ', '_', '+', '+', '+', '+', '|'
10530	};
10531	char _cursident[28] = "+,.-0ahI'fgjklmnopqrstuvwx~";
10532	
10533	int setterm(type)
10534	char *type;
10535	{
10536	  unsigned char *ac;
10537	  int i;
10538	#ifdef TIOCGWINSZ
10539	  struct winsize wsize;
10540	#endif
10541	
10542	  if (tgetent(termcap, type) != 1) return ERR;
10543	
10544	#ifdef TIOCGWINSZ
10545	  if (ioctl(0, TIOCGWINSZ, &wsize) == 0) {
10546	        LINES = wsize.ws_row != 0 ? wsize.ws_row : tgetnum("li");
10547	        COLS = wsize.ws_col != 0 ? wsize.ws_col : tgetnum("co");
10548	  } else {
10549	#endif
10550	        LINES = tgetnum("li");
10551	        COLS = tgetnum("co");
10552	#ifdef TIOCGWINSZ
10553	  }
10554	#endif
10555	  arp = tc;
10556	  cl = tgetstr("cl", &arp);
10557	  so = tgetstr("so", &arp);
10558	  se = tgetstr("se", &arp);
10559	  cm = tgetstr("cm", &arp);
10560	  mr = tgetstr("mr", &arp);
10561	  me = tgetstr("me", &arp);
10562	  mb = tgetstr("mb", &arp);
10563	  md = tgetstr("md", &arp);
10564	  us = tgetstr("us", &arp);
.Op 83 src/lib/curses/cursesio.c
10565	  ue = tgetstr("ue", &arp);
10566	  vi = tgetstr("vi", &arp);
10567	  ve = tgetstr("ve", &arp);
10568	  vs = tgetstr("vs", &arp);
10569	  as = tgetstr("as", &arp);
10570	  ae = tgetstr("ae", &arp);
10571	  ac = (unsigned char *) tgetstr("ac", &arp);
10572	  bl = tgetstr("bl", &arp);
10573	  vb = tgetstr("vb", &arp);
10574	
10575	  if (ac) {
10576	        while (*ac) {
10577	                i = 0;
10578	                while (*ac != _cursident[i]) i++;
10579	                _cursgraftable[i] = *++ac | A_ALTCHARSET;
10580	                ac++;
10581	        }
10582	  }
10583	
10584	  ACS_ULCORNER = _cursgraftable[UPLEFT];
10585	  ACS_LLCORNER = _cursgraftable[DOWNLEFT];
10586	  ACS_URCORNER = _cursgraftable[UPRIGHT];
10587	  ACS_LRCORNER = _cursgraftable[DOWNRIGHT];
10588	  ACS_RTEE = _cursgraftable[TEERIGHT];
10589	  ACS_LTEE = _cursgraftable[TEELEFT];
10590	  ACS_BTEE = _cursgraftable[TEEHEAD];
10591	  ACS_TTEE = _cursgraftable[TEENORMAL];
10592	  ACS_HLINE = _cursgraftable[MIDLINE];
10593	  ACS_VLINE = _cursgraftable[VERTLINE];
10594	  ACS_PLUS = _cursgraftable[CROSS];
10595	  ACS_S1 = _cursgraftable[UPLINE];
10596	  ACS_S9 = _cursgraftable[DOWNLINE];
10597	  ACS_DIAMOND = _cursgraftable[DIAMOND];
10598	  ACS_CKBOARD = _cursgraftable[GREYSQUARE];
10599	  ACS_DEGREE = _cursgraftable[DEGREE];
10600	  ACS_PLMINUS = _cursgraftable[PLUSMINUS];
10601	  ACS_BULLET = 'o';             /* where the hell is a bullet defined in
10602	                         * termcap ??? */
10603	  ACS_LARROW = _cursgraftable[LEFTARROW];
10604	  ACS_RARROW = _cursgraftable[RIGHTARROW];
10605	  ACS_DARROW = _cursgraftable[DOWNARROW];
10606	  ACS_UARROW = _cursgraftable[UPARROW];
10607	  ACS_BOARD = _cursgraftable[EMPTYSQUARE];
10608	  ACS_LANTERN = _cursgraftable[LATERN];
10609	  ACS_BLOCK = _cursgraftable[FULLSQUARE];
10610	  /* Wow, I got it! */
10611	  return OK;
10612	}
	
10614	void gettmode()
10615	{
10616	  tcgetattr(0, &_orig_tty);
10617	  tcgetattr(0, &_tty);
10618	  _cursvar.echoit = (_tty.c_lflag & ECHO) != 0;
10619	  _cursvar.rawmode = (_tty.c_lflag & (ICANON|ISIG)) == 0;
10620	  _cursvar.cbrkmode = (_tty.c_lflag & (ICANON|ISIG)) == ISIG;
10621	  NONL = (_tty.c_iflag & ICRNL) != 0;
10622	}
.Ep 84 src/lib/curses/endwin.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/endwin.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

10700	#include <curses.h>
10701	#include "curspriv.h"
10702	#include <termcap.h>
10703	
10704	int endwin()
10705	{
10706	  extern char *me;
10707	
10708	  curs_set(1);
10709	  poscur(LINES - 1, 0);
10710	  refresh();
10711	  tputs(me, 1, outc);
10712	  delwin(stdscr);
10713	  delwin(curscr);
10714	  delwin(_cursvar.tmpwin);
10715	  resetty();
10716	  return(OK);
10717	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/flash.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

10800	#include <curses.h>
10801	#include "curspriv.h"
10802	#include <termcap.h>
10803	
10804	extern char *bl, *vb;
10805	
10806	/* Flash() flashes the terminal screen. */
10807	void flash()
10808	{
10809	  if (vb)
10810	        tputs(vb, 1, outc);
10811	  else if (bl)
10812	        tputs(bl, 1, outc);
10813	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/initscr.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

10900	/* initscr.c - initialize the curses library */
10901	
10902	#include <stdlib.h>
10903	#include <curses.h>
10904	#include "curspriv.h"
10905	
10906	WINDOW *initscr()
10907	{
10908	  char *term;
10909	
.Op 85 src/lib/curses/initscr.c
10910	  if ((term = getenv("TERM")) == NULL) return NULL;
10911	  setterm(term);
10912	  gettmode();
10913	  if ((_cursvar.tmpwin = newwin(LINES, COLS, 0, 0)) == NULL) return NULL;
10914	  if ((curscr = newwin(LINES, COLS, 0, 0)) == NULL) return NULL;
10915	  if ((stdscr = newwin(LINES, COLS, 0, 0)) == NULL) return NULL;
10916	  clearok(curscr, TRUE);
10917	  return(stdscr);
10918	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/longname.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

11000	#include <curses.h>
11001	#include "curspriv.h"
11002	
11003	/****************************************************************/
11004	/* Longname() returns a pointer to a string describing the      */
11005	/* User terminal.                                               */
11006	/****************************************************************/
11007	
11008	char *longname()
11009	{
11010	  return("not implemented");
11011	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/move.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

11100	#include <curses.h>
11101	#include "curspriv.h"
11102	
11103	/****************************************************************/
11104	/* Wmove() moves the cursor in window 'win' to position (x,y).  */
11105	/****************************************************************/
11106	
11107	int wmove(win, y, x)
11108	WINDOW *win;
11109	int y;
11110	int x;
11111	{
11112	  if ((x<0) || (x>win->_maxx) || (y<win->_regtop) || (y>win->_regbottom)) 
11113	        return(ERR);
11114	  win->_curx = x;
11115	  win->_cury = y;
11116	  return(OK);
11117	}
.Ep 86 src/lib/curses/mvcursor.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/mvcursor.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

11200	#include <curses.h>
11201	#include "curspriv.h"
11202	
11203	/****************************************************************/
11204	/* Mvcur(oldy,oldx,newy,newx) the display cursor to <newy,newx> */
11205	/****************************************************************/
11206	
11207	int mvcur(oldy, oldx, newy, newx)
11208	int oldy;
11209	int oldx;
11210	int newy;
11211	int newx;
11212	{
11213	  if ((newy >= LINES) || (newx >= COLS) || (newy < 0) || (newx < 0))
11214	        return(ERR);
11215	  poscur(newy, newx);
11216	  _cursvar.cursrow = newy;
11217	  _cursvar.curscol = newx;
11218	  return(OK);
11219	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/newwin.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

11300	#include <stdlib.h>
11301	#include <curses.h>
11302	#include "curspriv.h"
11303	
11304	/****************************************************************/
11305	/* Makenew() allocates all data for a new window except the     */
11306	/* Actual lines themselves.                                     */
11307	/****************************************************************/
11308	
11309	_PROTOTYPE(static WINDOW *makenew, (int nlines, int ncols, int begy,int begx));
11310	
11311	static WINDOW *makenew(num_lines, num_columns, begy, begx)
11312	int num_lines, num_columns, begy, begx;
11313	{
11314	  int i;
11315	  WINDOW *win;
11316	
11317	  /* Allocate the window structure itself */
11318	  if ((win = (WINDOW *) malloc(sizeof(WINDOW))) == NULL) 
11319	        return((WINDOW *) ERR);
11320	
11321	  /* Allocate the line pointer array */
11322	  if ((win->_line = (int **) calloc(num_lines, sizeof(int *))) == NULL) {
11323	        free(win);
11324	        return((WINDOW *) ERR);
.Op 87 src/lib/curses/newwin.c
11325	  }
11326	
11327	  /* Allocate the minchng and maxchng arrays */
11328	  if ((win->_minchng = (int *) calloc(num_lines, sizeof(int))) == NULL) {
11329	        free(win);
11330	        free(win->_line);
11331	        return((WINDOW *) ERR);
11332	  }
11333	  if ((win->_maxchng = (int *) calloc(num_lines, sizeof(int))) == NULL) {
11334	        free(win);
11335	        free(win->_line);
11336	        free(win->_minchng);
11337	        return((WINDOW *) ERR);
11338	  }
11339	
11340	  /* Initialize window variables */
11341	  win->_curx = 0;
11342	  win->_cury = 0;
11343	  win->_maxy = num_lines - 1;
11344	  win->_maxx = num_columns - 1;
11345	  win->_begy = begy;
11346	  win->_begx = begx;
11347	  win->_flags = 0;
11348	  win->_attrs = ATR_NRM;
11349	  win->_tabsize = 8;
11350	  win->_clear = FALSE;
11351	  win->_leave = FALSE;
11352	  win->_scroll = FALSE;
11353	  win->_nodelay = FALSE;
11354	  win->_keypad = FALSE;
11355	  win->_regtop = 0;
11356	  win->_regbottom = num_lines - 1;
11357	
11358	  /* Init to say window unchanged */
11359	  for (i = 0; i < num_lines; i++) {
11360	        win->_minchng[i] = 0;
11361	        win->_maxchng[i] = num_columns - 1;
11362	  }
11363	
11364	  /* Set flags for window properties */
11365	  if ((begy + num_lines) == LINES) {
11366	        win->_flags |= _ENDLINE;
11367	        if ((begx == 0) && (num_columns == COLS) && (begy == 0))
11368	                win->_flags |= _FULLWIN;
11369	  }                             /* if */
11370	  if (((begy + num_lines) == LINES) && ((begx + num_columns) == COLS))
11371	        win->_flags |= _SCROLLWIN;
11372	  return(win);
11373	}
	
	
11376	/****************************************************************/
11377	/* Newwin() creates a new window with size num_lines * num_co-  */
11378	/* Lumns, and origin begx,begy relative to the SCREEN. Special  */
11379	/* Case: if num_lines and/or num_columns is 0, the remainder of */
11380	/* The screen is used.                                          */
11381	/****************************************************************/
11382	WINDOW *newwin(num_lines, num_columns, begy, begx)
11383	int num_lines, num_columns, begy, begx;
11384	{
.Ep 88 src/lib/curses/newwin.c
11385	  WINDOW *win;
11386	  int *ptr;
11387	  int i, j;
11388	
11389	  if (num_lines == 0) num_lines = LINES - begy;
11390	  if (num_columns == 0) num_columns = COLS - begx;
11391	  if ((win = makenew(num_lines, num_columns, begy, begx)) == (WINDOW *) ERR)
11392	        return((WINDOW *) ERR);
11393	  for (i = 0; i < num_lines; i++) {     /* make and clear the lines */
11394	        if ((win->_line[i] = (int *)calloc(num_columns, sizeof(int))) == NULL){
11395	                for (j = 0; j < i; j++) /* if error, free all the data */
11396	                        free(win->_line[j]);
11397	                free(win->_minchng);
11398	                free(win->_maxchng);
11399	                free(win->_line);
11400	                free(win);
11401	                return((WINDOW *) ERR);
11402	        } else {
11403	                for (ptr = win->_line[i]; ptr < win->_line[i] + num_columns;)
11404	                        *ptr++ = ' ' | ATR_NRM;
11405	        }
11406	  }
11407	  return(win);
11408	}
	
	
11411	/****************************************************************/
11412	/* Subwin() creates a sub-window in the 'orig' window, with     */
11413	/* Size num_lines * num_columns, and with origin begx, begy     */
11414	/* Relative to the SCREEN. Special case: if num_lines and/or    */
11415	/* Num_columns is 0, the remainder of the original window is    */
11416	/* Used. The subwindow uses the original window's line buffers  */
11417	/* To store it's own lines.                                     */
11418	/****************************************************************/
11419	WINDOW *subwin(orig, num_lines, num_columns, begy, begx)
11420	WINDOW *orig;
11421	int num_lines, num_columns, begy, begx;
11422	{
11423	  WINDOW *win;
11424	  int i, j, k;
11425	
11426	  /* Make sure window fits inside the original one */
11427	  if (begy < orig->_begy || begx < orig->_begx ||
11428	                      (begy + num_lines) > (orig->_begy + orig->_maxy) ||
11429	                      (begx + num_columns) > (orig->_begx + orig->_maxx) )
11430	        return((WINDOW *) ERR);
11431	
11432	  if (num_lines == 0) num_lines = orig->_maxy - (begy - orig->_begy);
11433	  if (num_columns == 0) num_columns = orig->_maxx - (begx - orig->_begx);
11434	  if ((win = makenew(num_lines, num_columns, begy, begx)) == (WINDOW *) ERR)
11435	        return((WINDOW *) ERR);
11436	
11437	  /* Set line pointers the same as in the original window */
11438	  j = begy - orig->_begy;
11439	  k = begx - orig->_begx;
11440	  for (i = 0; i < num_lines; i++) win->_line[i] = (orig->_line[j++]) + k;
11441	  win->_flags |= _SUBWIN;
11442	  return(win);
11443	}
.Op 89 src/lib/curses/options.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/options.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

11500	#include <curses.h>
11501	#include "curspriv.h"
11502	
11503	static bool hasold = FALSE;     /* for remembering old cursor type */
11504	static int oldmode;
11505	
11506	/****************************************************************/
11507	/* Idlok() is used to set  flag for using the terminal insert/  */
11508	/* Delete line capabilities. This is not relevant for the PC    */
11509	/* Version of curses, and thus nothing is done.                 */
11510	/****************************************************************/
11511	void idlok(win, flag)
11512	WINDOW *win;
11513	bool flag;
11514	{
11515	}
	
11517	/****************************************************************/
11518	/* Clearok() marks window 'win' to cause screen clearing and    */
11519	/* Redraw the next time a refresh is done.                      */
11520	/****************************************************************/
11521	void clearok(win, flag)
11522	WINDOW *win;
11523	bool flag;
11524	{
11525	  if (win == curscr)
11526	        _cursvar.tmpwin->_clear = flag;
11527	  else
11528	        win->_clear = flag;
11529	}
	
11531	/****************************************************************/
11532	/* Leaveok() marks window 'win' to allow the update routines    */
11533	/* To leave the hardware cursor where it happens to be at the   */
11534	/* End of update. Usually used in combination with cursoff().   */
11535	/****************************************************************/
11536	
11537	void leaveok(win, flag)
11538	WINDOW *win;
11539	bool flag;
11540	{
11541	  win->_leave = flag;
11542	}
	
11544	/****************************************************************/
11545	/* Scrollok() marks window 'win' to allow the scrolling region  */
11546	/* Of it to actually scroll.                                    */
11547	/****************************************************************/
11548	void scrollok(win, flag)
11549	WINDOW *win;
11550	bool flag;
11551	{
11552	  win->_scroll = flag;
11553	}
	
.Ep 90 src/lib/curses/options.c
11555	/****************************************************************/
11556	/* Nodelay() marks the window to make character input non-      */
11557	/* Waiting, i.e. if there is no character to get, -1 will be    */
11558	/* Returned.                                                    */
11559	/****************************************************************/
11560	void nodelay(win, flag)
11561	WINDOW *win;
11562	bool flag;
11563	{
11564	  win->_nodelay = flag;
11565	}
	
11567	/****************************************************************/
11568	/* Keypad() marks window 'win' to use the special keypad mode.  */
11569	/****************************************************************/
11570	void keypad(win, flag)
11571	WINDOW *win;
11572	bool flag;
11573	{
11574	  win->_keypad = flag;
11575	}
	
11577	/****************************************************************/
11578	/* Meta() allows use of any alternate character set allowed by  */
11579	/* The terminal. We always allow this on the PC, so this one    */
11580	/* Does nothing.                                                */
11581	/****************************************************************/
11582	void meta(win, flag)
11583	WINDOW *win;
11584	bool flag;
11585	{
11586	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/overlay.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

11600	/****************************************************************/
11601	/* Overlay() and overwrite() functions of the PCcurses package  */
11602	/*                                                              */
11603	/****************************************************************/
11604	/* This version of curses is based on ncurses, a curses version */
11605	/* Originally written by Pavel Curtis at Cornell University.    */
11606	/* I have made substantial changes to make it run on IBM PC's,  */
11607	/* And therefore consider myself free to make it public domain. */
11608	/*              Bjorn Larsson (...mcvax!enea!infovax!bl)        */
11609	/****************************************************************/
11610	/* 1.0: Release:                                        870515  */
11611	/****************************************************************/
11612	/* Modified to run under the MINIX operating system by Don Cope */
11613	/* These changes are also released into the public domain.      */
11614	/*                                                      900906  */
11615	/****************************************************************/
11616	
11617	#include <curses.h>
11618	#include "curspriv.h"
11619	
.Op 91 src/lib/curses/overlay.c
11620	/****************************************************************/
11621	/* Overlay() overwrites 'win1' upon 'win2', with origins alig-  */
11622	/* Ned. Overlay is transparent; blanks from 'win1' are not      */
11623	/* Copied to 'win2'.                                            */
11624	/****************************************************************/
11625	void overlay(win1, win2)
11626	WINDOW *win1, *win2;
11627	{
11628	  int *minchng;
11629	  int *maxchng;
11630	  int *w1ptr;
11631	  int *w2ptr;
11632	  int attrs;
11633	  int col;
11634	  int line;
11635	  int last_line;
11636	  int last_col;
11637	
11638	  last_col = min(win1->_maxx, win2->_maxx);
11639	  last_line = min(win1->_maxy, win2->_maxy);
11640	  attrs = win2->_attrs & ATR_MSK;
11641	  minchng = win2->_minchng;
11642	  maxchng = win2->_maxchng;
11643	
11644	  for (line = 0; line <= last_line; line++) {
11645	        register short fc, lc = 0;
11646	        w1ptr = win1->_line[line];
11647	        w2ptr = win2->_line[line];
11648	        fc = _NO_CHANGE;
11649	        for (col = 0; col <= last_col; col++) {
11650	                if ((*w1ptr & CHR_MSK) != ' ') {
11651	                        *w2ptr = (*w1ptr & CHR_MSK) | attrs;
11652	                        if (fc == _NO_CHANGE) fc = col;
11653	                        lc = col;
11654	                }
11655	                w1ptr++;
11656	                w2ptr++;
11657	        }
11658	
11659	        if (*minchng == _NO_CHANGE) {
11660	                *minchng = fc;
11661	                *maxchng = lc;
11662	        } else if (fc != _NO_CHANGE) {
11663	                if (fc < *minchng) *minchng = fc;
11664	                if (lc > *maxchng) *maxchng = lc;
11665	        }
11666	        minchng++;
11667	        maxchng++;
11668	  }                             /* for */
11669	}                               /* overlay */
11670	
11671	/****************************************************************/
11672	/* Overwrite() overwrites 'win1' upon 'win2', with origins      */
11673	/* Aligned. Overwrite is non-transparent; blanks from 'win1'    */
11674	/* Are copied to 'win2'.                                        */
11675	/****************************************************************/
11676	void overwrite(win1, win2)
11677	WINDOW *win1, *win2;
11678	{
11679	  int *minchng;
.Ep 92 src/lib/curses/overlay.c
11680	  int *maxchng;
11681	  int *w1ptr;
11682	  int *w2ptr;
11683	  int attrs;
11684	  int col;
11685	  int line;
11686	  int last_line;
11687	  int last_col;
11688	
11689	  last_col = min(win1->_maxx, win2->_maxx);
11690	  last_line = min(win1->_maxy, win2->_maxy);
11691	  attrs = win2->_attrs & ATR_MSK;
11692	  minchng = win2->_minchng;
11693	  maxchng = win2->_maxchng;
11694	
11695	  for (line = 0; line <= last_line; line++) {
11696	        register short fc, lc = 0;
11697	
11698	        w1ptr = win1->_line[line];
11699	        w2ptr = win2->_line[line];
11700	        fc = _NO_CHANGE;
11701	
11702	        for (col = 0; col <= last_col; col++) {
11703	                if ((*w1ptr & CHR_MSK) != (*w2ptr & CHR_MSK)) {
11704	                        *w2ptr = (*w1ptr & CHR_MSK) | attrs;
11705	
11706	                        if (fc == _NO_CHANGE) fc = col;
11707	                        lc = col;
11708	                }
11709	                w1ptr++;
11710	                w2ptr++;
11711	        }                       /* for */
11712	
11713	        if (*minchng == _NO_CHANGE) {
11714	                *minchng = fc;
11715	                *maxchng = lc;
11716	        } else if (fc != _NO_CHANGE) {
11717	                if (fc < *minchng) *minchng = fc;
11718	                if (lc > *maxchng) *maxchng = lc;
11719	        }
11720	        minchng++;
11721	        maxchng++;
11722	  }
11723	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/prntscan.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

11800	#include <string.h>
11801	#include <curses.h>
11802	#include "curspriv.h"
11803	
11804	static char printscanbuf[513];  /* buffer used during I/O */
11805	
11806	/****************************************************************/
11807	/* Wprintw(win,fmt,args) does a printf() in window 'win'.       */
11808	/****************************************************************/
11809	int wprintw(WINDOW *win, const char *fmt, ...)
.Op 93 src/lib/curses/prntscan.c
11810	{
11811	  va_list args;
11812	
11813	  va_start(args, fmt);
11814	  vsprintf(printscanbuf, fmt, args);
11815	  if (waddstr(win, printscanbuf) == ERR) return(ERR);
11816	  return(strlen(printscanbuf));
11817	}
	
11819	/****************************************************************/
11820	/* Printw(fmt,args) does a printf() in stdscr.                  */
11821	/****************************************************************/
11822	int printw(const char *fmt, ...)
11823	{
11824	  va_list args;
11825	
11826	  va_start(args, fmt);
11827	  vsprintf(printscanbuf, fmt, args);
11828	  if (waddstr(stdscr, printscanbuf) == ERR) return(ERR);
11829	  return(strlen(printscanbuf));
11830	}                               /* printw */
11831	
11832	/****************************************************************/
11833	/* Mvprintw(fmt,args) moves the stdscr cursor to a new posi-    */
11834	/* tion, then does a printf() in stdscr.                        */
11835	/****************************************************************/
11836	int mvprintw(int y, int x, const char *fmt, ...)
11837	{
11838	  va_list args;
11839	
11840	  va_start(args, fmt);
11841	  if (wmove(stdscr, y, x) == ERR) return(ERR);
11842	  vsprintf(printscanbuf, fmt, args);
11843	  if (waddstr(stdscr, printscanbuf) == ERR) return(ERR);
11844	  return(strlen(printscanbuf));
11845	}
	
11847	/****************************************************************/
11848	/* Mvwprintw(win,fmt,args) moves the window 'win's cursor to    */
11849	/* A new position, then does a printf() in window 'win'.        */
11850	/****************************************************************/
11851	int mvwprintw(WINDOW *win, int y, int x, const char *fmt, ...)
11852	{
11853	  va_list args;
11854	
11855	  va_start(args, fmt);
11856	  if (wmove(win, y, x) == ERR) return(ERR);
11857	  vsprintf(printscanbuf, fmt, args);
11858	  if (waddstr(win, printscanbuf) == ERR) return(ERR);
11859	  return(strlen(printscanbuf));
11860	}                               /* mvwprintw */
11861	
11862	/****************************************************************/
11863	/* Wscanw(win,fmt,args) gets a string via window 'win', then    */
11864	/* Scans the string using format 'fmt' to extract the values    */
11865	/* And put them in the variables pointed to the arguments.      */
11866	/****************************************************************/
11867	int wscanw(WINDOW *win, const char *fmt, ...)
11868	{
11869	  va_list args;
.Ep 94 src/lib/curses/prntscan.c
11870	
11871	  va_start(args, fmt);
11872	  wrefresh(win);                /* set cursor */
11873	  if (wgetstr(win, printscanbuf) == ERR)        /* get string */
11874	        return(ERR);
11875	  return(vsscanf(printscanbuf, fmt, args));
11876	}                               /* wscanw */
11877	
11878	/****************************************************************/
11879	/* Scanw(fmt,args) gets a string via stdscr, then scans the     */
11880	/* String using format 'fmt' to extract the values and put them */
11881	/* In the variables pointed to the arguments.                   */
11882	/****************************************************************/
11883	int scanw(const char *fmt, ...)
11884	{
11885	  va_list args;
11886	
11887	  va_start(args, fmt);
11888	  wrefresh(stdscr);             /* set cursor */
11889	  if (wgetstr(stdscr, printscanbuf) == ERR)     /* get string */
11890	        return(ERR);
11891	  return(vsscanf(printscanbuf, fmt, args));
11892	}                               /* scanw */
11893	
11894	/****************************************************************/
11895	/* Mvscanw(y,x,fmt,args) moves stdscr's cursor to a new posi-   */
11896	/* Tion, then gets a string via stdscr and scans the string     */
11897	/* Using format 'fmt' to extract the values and put them in the */
11898	/* Variables pointed to the arguments.                          */
11899	/****************************************************************/
11900	int mvscanw(int y, int x, const char *fmt, ...)
11901	{
11902	  va_list args;
11903	
11904	  va_start(args, fmt);
11905	  if (wmove(stdscr, y, x) == ERR) return(ERR);
11906	  wrefresh(stdscr);             /* set cursor */
11907	  if (wgetstr(stdscr, printscanbuf) == ERR)     /* get string */
11908	        return(ERR);
11909	  return(vsscanf(printscanbuf, fmt, args));
11910	}                               /* mvscanw */
11911	
11912	/****************************************************************/
11913	/* Mvwscanw(win,y,x,fmt,args) moves window 'win's cursor to a   */
11914	/* New position, then gets a string via 'win' and scans the     */
11915	/* String using format 'fmt' to extract the values and put them */
11916	/* In the variables pointed to the arguments.                   */
11917	/****************************************************************/
11918	int mvwscanw(WINDOW *win, int y, int x, const char *fmt, ...)
11919	{
11920	  va_list args;
11921	
11922	  va_start(args, fmt);
11923	  if (wmove(win, y, x) == ERR) return(ERR);
11924	  wrefresh(win);                /* set cursor */
11925	  if (wgetstr(win, printscanbuf) == ERR)        /* get string */
11926	        return(ERR);
11927	  return(vsscanf(printscanbuf, fmt, args));
11928	}                               /* mvwscanw */
.Op 95 src/lib/curses/refresh.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/refresh.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

12000	/* refresh.c */
12001	
12002	#include <curses.h>
12003	#include "curspriv.h"
12004	
12005	/* Wrefresh() updates window win's area of the physical screen. */
12006	void wrefresh(win)
12007	WINDOW *win;
12008	{
12009	  if (win == curscr)
12010	        curscr->_clear = TRUE;
12011	  else
12012	        wnoutrefresh(win);
12013	  doupdate();
12014	}
	
12016	/****************************************************************/
12017	/* Wnoutrefresh() updates the image of the desired screen,      */
12018	/* Without doing physical update (copies window win's image to  */
12019	/* The _cursvar.tmpwin window, which is hidden from the user).  */
12020	/****************************************************************/
12021	
12022	void wnoutrefresh(win)
12023	register WINDOW *win;
12024	{
12025	  register int *dst;            /* start destination in temp window */
12026	  register int *end;            /* end destination in temp window */
12027	  register int *src;            /* source in user window */
12028	  register int first;           /* first changed char on line */
12029	  register int last;            /* last changed char on line */
12030	  WINDOW *nscr;
12031	  int begy;                     /* window's place on screen */
12032	  int begx;
12033	  int i;
12034	  int j;
12035	
12036	  nscr = _cursvar.tmpwin;
12037	  begy = win->_begy;
12038	  begx = win->_begx;
12039	
12040	  for (i = 0, j = begy; i <= win->_maxy; i++, j++) {
12041	        if (win->_minchng[i] != _NO_CHANGE) {
12042	                first = win->_minchng[i];
12043	                last = win->_maxchng[i];
12044	                dst = &(nscr->_line[j][begx + first]);
12045	                end = &(nscr->_line[j][begx + last]);
12046	                src = &(win->_line[i][first]);
12047	
12048	                while (dst <= end)      /* copy user line to temp window */
12049	                        *dst++ = *src++;
12050	
12051	                first += begx;  /* nscr's min/max change positions */
12052	                last += begx;
12053	
12054	                if ((nscr->_minchng[j] == _NO_CHANGE) || (nscr->_minchng[j] > first))
.Ep 96 src/lib/curses/refresh.c
12055	                        nscr->_minchng[j] = first;
12056	                if (last > nscr->_maxchng[j]) nscr->_maxchng[j] = last;
12057	
12058	                win->_minchng[i] = _NO_CHANGE;  /* updated now */
12059	        }                       /* if */
12060	        win->_maxchng[i] = _NO_CHANGE;  /* updated now */
12061	  }                             /* for */
12062	
12063	  if (win->_clear) {
12064	        win->_clear = FALSE;
12065	        nscr->_clear = TRUE;
12066	  }                             /* if */
12067	  if (!win->_leave) {
12068	        nscr->_cury = win->_cury + begy;
12069	        nscr->_curx = win->_curx + begx;
12070	  }                             /* if */
12071	}                               /* wnoutrefresh */




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/scrreg.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

12100	/****************************************************************/
12101	/* Wsetscrreg() routine of the PCcurses package                 */
12102	/*                                                              */
12103	/****************************************************************/
12104	/* This version of curses is based on ncurses, a curses version */
12105	/* Originally written by Pavel Curtis at Cornell University.    */
12106	/* I have made substantial changes to make it run on IBM PC's,  */
12107	/* And therefore consider myself free to make it public domain. */
12108	/*              Bjorn Larsson (...mcvax!enea!infovax!bl)        */
12109	/****************************************************************/
12110	/* 1.0: Release:                                        870515  */
12111	/****************************************************************/
12112	/* Modified to run under the MINIX operating system by Don Cope */
12113	/* These changes are also released into the public domain.      */
12114	/*                                                      900906  */
12115	/****************************************************************/
12116	
12117	#include <curses.h>
12118	#include "curspriv.h"
12119	
12120	/****************************************************************/
12121	/* Wsetscrreg() set the scrolling region of window 'win' to in- */
12122	/* Clude all lines between 'top' and 'bottom'.                  */
12123	/****************************************************************/
12124	
12125	int wsetscrreg(win, top, bottom)
12126	WINDOW *win;
12127	int top;
12128	int bottom;
12129	{
12130	  if ((0 <= top) &&
12131	      (top <= win->_cury)
12132	      &&
12133	      (win->_cury <= bottom)
12134	      &&
.Op 97 src/lib/curses/scrreg.c
12135	      (bottom <= win->_maxy)
12136	        ) {
12137	        win->_regtop = top;
12138	        win->_regbottom = bottom;
12139	        return(OK);
12140	  }
12141	
12142	   /* If */ 
12143	  else
12144	        return(ERR);
12145	}                               /* wsetscrreg */
12146	
12147	/****************************************************************/
12148	/* Setscrreg() set the scrolling region of stdscr to include    */
12149	/* All lines between 'top' and 'bottom'.                        */
12150	/****************************************************************/
12151	
12152	int setscrreg(top, bottom)
12153	int top;
12154	int bottom;
12155	{
12156	  return(wsetscrreg(stdscr, top, bottom));
12157	}                               /* setscrreg */



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/setterm.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

12200	#include <curses.h>
12201	#include "curspriv.h"
12202	
12203	_PROTOTYPE( static void ttysetflags, (void) );
12204	
12205	static void ttysetflags()
12206	{
12207	  _tty.c_iflag |= ICRNL | IXON;
12208	  _tty.c_oflag |= OPOST | ONLCR;
12209	  _tty.c_lflag |= ECHO | ICANON | IEXTEN | ISIG;
12210	
12211	  if (_cursvar.rawmode) {
12212	        _tty.c_iflag &= ~(ICRNL | IXON);
12213	        _tty.c_oflag &= ~(OPOST);
12214	        _tty.c_lflag &= ~(ICANON | IEXTEN | ISIG);
12215	  }
12216	  if (_cursvar.cbrkmode) {
12217	        _tty.c_lflag &= ~(ICANON);
12218	  }
12219	  if (!_cursvar.echoit) {
12220	        _tty.c_lflag &= ~(ECHO | ECHONL);
12221	  }
12222	  if (NONL) {
12223	        _tty.c_iflag &= ~(ICRNL);
12224	        _tty.c_oflag &= ~(ONLCR);
12225	  }
12226	  tcsetattr(0, TCSANOW, &_tty);
12227	}                               /* ttysetflags */
12228	
12229	void raw()
.Ep 98 src/lib/curses/setterm.c
12230	{
12231	  _cursvar.rawmode = TRUE;
12232	  ttysetflags();
12233	}                               /* raw */
12234	
12235	void noraw()
12236	{
12237	  _cursvar.rawmode = FALSE;
12238	  ttysetflags();
12239	}                               /* noraw */
12240	
12241	void echo()
12242	{
12243	  _cursvar.echoit = TRUE;
12244	  ttysetflags();
12245	}
	
12247	void noecho()
12248	{
12249	  _cursvar.echoit = FALSE;
12250	  ttysetflags();
12251	}
	
12253	void nl()
12254	{
12255	  NONL = FALSE;
12256	  ttysetflags();
12257	}                               /* nl */
12258	
12259	void nonl()
12260	{
12261	  NONL = TRUE;
12262	  ttysetflags();
12263	}                               /* nonl */
12264	
12265	void cbreak()
12266	{
12267	  _cursvar.cbrkmode = TRUE;
12268	  ttysetflags();
12269	}                               /* cbreak */
12270	
12271	void nocbreak()
12272	{
12273	  _cursvar.cbrkmode = FALSE;
12274	  ttysetflags();
12275	}                               /* nocbreak */
.Op 99 src/lib/curses/tabsize.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/tabsize.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

12300	/****************************************************************/
12301	/* Tabsize() routines of the PCcurses package                   */
12302	/*                                                              */
12303	/****************************************************************/
12304	/* This version of curses is based on ncurses, a curses version */
12305	/* Originally written by Pavel Curtis at Cornell University.    */
12306	/* I have made substantial changes to make it run on IBM PC's,  */
12307	/* And therefore consider myself free to make it public domain. */
12308	/*              Bjorn Larsson (...mcvax!enea!infovax!bl)        */
12309	/****************************************************************/
12310	/* 1.0: Release:                                        870515  */
12311	/****************************************************************/
12312	/* Modified to run under the MINIX operating system by Don Cope */
12313	/* These changes are also released into the public domain.      */
12314	/*                                                      900906  */
12315	/****************************************************************/
12316	
12317	#include <curses.h>
12318	#include "curspriv.h"
12319	
12320	/****************************************************************/
12321	/* Wtabsize(win,ts) sets the tabsize of window 'win' to 'ts',   */
12322	/* And returns the original value.                              */
12323	/****************************************************************/
12324	
12325	int wtabsize(win, ts)
12326	WINDOW *win;
12327	int ts;
12328	{
12329	  int origval;
12330	
12331	  origval = win->_tabsize;
12332	  win->_tabsize = ts;
12333	  return(origval);
12334	}                               /* wtabsize */
12335	
12336	/****************************************************************/
12337	/* Tabsize(ts) sets the tabsize of stdscr to 'ts', and returns  */
12338	/* The original value.                                          */
12339	/****************************************************************/
12340	
12341	int tabsize(ts)
12342	int ts;
12343	{
12344	  int origval;
12345	
12346	  origval = stdscr->_tabsize;
12347	  stdscr->_tabsize = ts;
12348	  return(origval);
12349	}                               /* tabsize */
.Ep 100 src/lib/curses/termmisc.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/termmisc.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

12400	#include <curses.h>
12401	#include "curspriv.h"
12402	
12403	/* Static variables or saving terminal modes */
12404	
12405	int fixterm()
12406	{
12407	  return(OK);
12408	}                               /* fixterm */
12409	
12410	int resetterm()
12411	{
12412	  return(OK);
12413	}
	
12415	int saveoldterm()
12416	{
12417	  return(OK);
12418	}                               /* saveoldterm */
12419	
12420	int saveterm()
12421	{
12422	  return(OK);
12423	}                               /* saveterm */
12424	
12425	int baudrate()
12426	{
12427	  return(19200);
12428	}                               /* baudrate */
12429	
12430	/****************************************************************/
12431	/* Erasechar(), killchar() returns std MSDOS erase chars.       */
12432	/****************************************************************/
12433	
12434	int erasechar()
12435	{
12436	  return(_DCCHAR);              /* character delete char */
12437	}                               /* erasechar */
12438	
12439	int killchar()
12440	{
12441	  return(_DLCHAR);              /* line delete char */
12442	}                               /* killchar */
12443	
12444	/****************************************************************/
12445	/* Savetty() and resetty() saves and restores the terminal I/O  */
12446	/* Settings.                                                    */
12447	/****************************************************************/
12448	
12449	int savetty()
12450	{
12451	  return(OK);
12452	}                               /* savetty */
12453	
12454	/****************************************************************/
.Op 101 src/lib/curses/termmisc.c
12455	/* Setupterm() sets up the terminal. On a PC, it is always suc- */
12456	/* Cessful, and returns 1.                                      */
12457	/****************************************************************/
12458	
12459	int setupterm()
12460	{
12461	  return(1);
12462	}                               /* setupterm */



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/unctrl.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

12500	/****************************************************************/
12501	/* Unctrl() routines of the PCcurses package                    */
12502	/*                                                              */
12503	/****************************************************************/
12504	/* This version of curses is based on ncurses, a curses version */
12505	/* Originally written by Pavel Curtis at Cornell University.    */
12506	/* I have made substantial changes to make it run on IBM PC's,  */
12507	/* And therefore consider myself free to make it public domain. */
12508	/*              Bjorn Larsson (...mcvax!enea!infovax!bl)        */
12509	/****************************************************************/
12510	/* 1.0: Release:                                        870515  */
12511	/****************************************************************/
12512	/* Modified to run under the MINIX operating system by Don Cope */
12513	/* These changes are also released into the public domain.      */
12514	/*                                                      900906  */
12515	/****************************************************************/
12516	
12517	#include <curses.h>
12518	#include "curspriv.h"
12519	
12520	static char strbuf[3] = {0, 0, 0};
12521	
12522	/****************************************************************/
12523	/* Unctrl() returns a char pointer to a string corresponding to */
12524	/* Argument character 'c'.                                      */
12525	/****************************************************************/
12526	
12527	char *unctrl(c)
12528	char c;
12529	{
12530	  int ic = c;
12531	  ic &= 0xff;
12532	
12533	  if ((ic >= ' ') && (ic != 0x7f)) {    /* normal characters */
12534	        strbuf[0] = ic;
12535	        strbuf[1] = '\0';
12536	        return(strbuf);
12537	  }                             /* if */
12538	  strbuf[0] = '^';              /* '^' prefix */
12539	  if (c == 0x7f)                /* DEL */
12540	        strbuf[1] = '?';
12541	  else                          /* other control */
12542	        strbuf[1] = ic + '@';
12543	  return(strbuf);
12544	}                               /* unctrl */
.Ep 102 src/lib/curses/update.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/update.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

12600	#include <curses.h>
12601	#include "curspriv.h"
12602	#include <termcap.h>
12603	
12604	static WINDOW *twin;            /* used by many routines */
12605	
12606	/****************************************************************/
12607	/* Gotoxy() moves the physical cursor to the desired address on */
12608	/* The screen. We don't optimize here - on a PC, it takes more  */
12609	/* Time to optimize than to do things directly.                 */
12610	/****************************************************************/
12611	
12612	_PROTOTYPE(static void gotoxy, (int row, int col ));
12613	_PROTOTYPE(static void newattr, (int ch ));
12614	_PROTOTYPE(static void Putchar, (int ch ));
12615	_PROTOTYPE(static void clrupdate, (WINDOW *scr ));
12616	_PROTOTYPE(static void transformline, (int lineno ));
12617	
12618	static void gotoxy(row, col)
12619	int row, col;
12620	{
12621	  poscur(row, col);
12622	  _cursvar.cursrow = row;
12623	  _cursvar.curscol = col;
12624	}
	
12626	/* Update attributes */
12627	static void newattr(ch)
12628	int ch;
12629	{
12630	  extern char *me, *as, *ae, *mb, *md, *mr, *so, *us;
12631	  static int lastattr = 0;
12632	
12633	  if (lastattr != (ch &= ATR_MSK)) {
12634	        lastattr = ch;
12635	
12636	        tputs(me, 1, outc);
12637	        if (ae) tputs(ae, 1, outc);
12638	
12639	        if (ch & A_ALTCHARSET)
12640	                if (as) tputs(as, 1, outc);
12641	        if (ch & A_BLINK) tputs(mb, 1, outc);
12642	        if (ch & A_BOLD) tputs(md, 1, outc);
12643	        if (ch & A_REVERSE) tputs(mr, 1, outc);
12644	        if (ch & A_STANDOUT) tputs(so, 1, outc);
12645	        if (ch & A_UNDERLINE) tputs(us, 1, outc);
12646	  }
12647	}
	
12649	/* Putchar() writes a character, with attributes, to the physical
12650	   screen, but avoids writing to the lower right screen position.
12651	   Should it care about am?
12652	*/
12653	
12654	/* Output char with attribute */
.Op 103 src/lib/curses/update.c
12655	static void Putchar(ch)
12656	int ch;
12657	{
12658	  if ((_cursvar.cursrow < LINES) || (_cursvar.curscol < COLS)) {
12659	        newattr(ch);
12660	        putchar(ch);
12661	  }
12662	}
	
12664	/****************************************************************/
12665	/* Clrupdate(scr) updates the screen by clearing it and then    */
12666	/* Redraw it in it's entirety.                                  */
12667	/****************************************************************/
12668	
12669	static void clrupdate(scr)
12670	WINDOW *scr;
12671	{
12672	  register int *src;
12673	  register int *dst;
12674	  register int i;
12675	  register int j;
12676	  WINDOW *w;
12677	
12678	  w = curscr;
12679	
12680	  if (scr != w) {               /* copy scr to curscr */
12681	        for (i = 0; i < LINES; i++) {
12682	                src = scr->_line[i];
12683	                dst = w->_line[i];
12684	                for (j = 0; j < COLS; j++) *dst++ = *src++;
12685	        }                       /* for */
12686	  }                             /* if */
12687	  newattr(scr->_attrs);
12688	  clrscr();
12689	  scr->_clear = FALSE;
12690	  for (i = 0; i < LINES; i++) { /* update physical screen */
12691	        src = w->_line[i];
12692	        j = 0;
12693	        while (j < COLS) {
12694	                if (*src != (' ' | ATR_NRM)) {
12695	                        gotoxy(i, j);
12696	                        while (j < COLS && (*src != (' ' | ATR_NRM))) {
12697	                                Putchar(*src++);
12698	                                j++;
12699	                        }
12700	                } else {
12701	                        src++;
12702	                        j++;
12703	                }
12704	        }                       /* for */
12705	  }                             /* for */
12706	  fflush(stdout);
12707	}                               /* clrupdate */
12708	
12709	/****************************************************************/
12710	/* Transformline() updates the given physical line to look      */
12711	/* Like the corresponding line in _cursvar.tmpwin.              */
12712	/****************************************************************/
12713	
12714	static void transformline(lineno)
.Ep 104 src/lib/curses/update.c
12715	register int lineno;
12716	{
12717	  register int *dstp;
12718	  register int *srcp;
12719	  register int dstc;
12720	  register int srcc;
12721	  int x;
12722	  int endx;
12723	
12724	  x = twin->_minchng[lineno];
12725	  endx = twin->_maxchng[lineno];
12726	  dstp = curscr->_line[lineno] + x;
12727	  srcp = twin->_line[lineno] + x;
12728	
12729	  while (x <= endx) {
12730	        if ((*dstp != *srcp) || (dstc != srcc)) {
12731	                gotoxy(lineno, x);
12732	                while (x <= endx && ((*dstp != *srcp) || (dstc != srcc))) {
12733	                        Putchar(*srcp);
12734	                        *dstp++ = *srcp++;
12735	                        x++;
12736	                }
12737	        } else {
12738	                *dstp++ = *srcp++;
12739	                x++;
12740	        }
12741	  }                             /* for */
12742	  twin->_minchng[lineno] = _NO_CHANGE;
12743	  twin->_maxchng[lineno] = _NO_CHANGE;
12744	}                               /* transformline */
12745	
12746	/****************************************************************/
12747	/* Doupdate() updates the physical screen to look like _curs-   */
12748	/* Var.tmpwin if curscr is not 'Clear-marked'. Otherwise it     */
12749	/* Updates the screen to look like curscr.                      */
12750	/****************************************************************/
12751	
12752	void doupdate()
12753	{
12754	  int i;
12755	
12756	  twin = _cursvar.tmpwin;
12757	  if (curscr->_clear)
12758	        clrupdate(curscr);
12759	  else {
12760	        if (twin->_clear)
12761	                clrupdate(twin);
12762	        else {
12763	                for (i = 0; i < LINES; i++)
12764	                        if (twin->_minchng[i] != _NO_CHANGE)
12765	                                transformline(i);
12766	        }
12767	  }
12768	  curscr->_curx = twin->_curx;
12769	  curscr->_cury = twin->_cury;
12770	  gotoxy(curscr->_cury, curscr->_curx);
12771	  fflush(stdout);
12772	}                               /* doupdate */
.Op 105 src/lib/curses/waddch.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/waddch.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

12800	#include <curses.h>
12801	#include "curspriv.h"
12802	
12803	/****************************************************************/
12804	/* Newline() does line advance and returns the new cursor line. */
12805	/* If error, return -1.                                         */
12806	/****************************************************************/
12807	
12808	_PROTOTYPE( static short newline, (WINDOW *win, int lin));
12809	
12810	static short newline(win, lin)
12811	WINDOW *win;
12812	int lin;
12813	{
12814	  if (++lin > win->_regbottom) {
12815	        lin--;
12816	        if (win->_scroll)
12817	                scroll(win);
12818	        else
12819	                return(-1);
12820	  }                             /* if */
12821	  return(lin);
12822	}                               /* newline */
12823	
12824	/****************************************************************/
12825	/* Waddch() inserts character 'c' at the current cursor posi-   */
12826	/* Tion in window 'win', and takes any actions as dictated by   */
12827	/* The character.                                               */
12828	/****************************************************************/
12829	
12830	int waddch(win, c)
12831	WINDOW *win;
12832	int c;
12833	{
12834	  int x = win->_curx;
12835	  int y = win->_cury;
12836	  int newx;
12837	  int ch = c;
12838	  int ts = win->_tabsize;
12839	
12840	  ch &= (A_ALTCHARSET | 0xff);
12841	  if (y > win->_maxy || x > win->_maxx || y < 0 || x < 0) return(ERR);
12842	  switch (ch) {
12843	      case '\t':
12844	        for (newx = ((x / ts) + 1) * ts; x < newx; x++) {
12845	                if (waddch(win, ' ') == ERR) return(ERR);
12846	                if (win->_curx == 0)    /* if tab to next line */
12847	                        return(OK);     /* exit the loop */
12848	        }
12849	        return(OK);
12850	
12851	      case '\n':
12852	        if (NONL) x = 0;
12853	        if ((y = newline(win, y)) < 0) return (ERR);
12854	        break;
.Ep 106 src/lib/curses/waddch.c
12855	
12856	      case '\r':        x = 0;  break;
12857	
12858	      case '\b':
12859	        if (--x < 0)            /* no back over left margin */
12860	                x = 0;
12861	        break;
12862	
12863	      case 0x7f:
12864	        {
12865	                if (waddch(win, '^') == ERR) return(ERR);
12866	                return(waddch(win, '?'));
12867	        }
12868	
12869	      default:
12870	        if (ch < ' ') {         /* handle control chars */
12871	                if (waddch(win, '^') == ERR) return(ERR);
12872	                return(waddch(win, c + '@'));
12873	        }
12874	        ch |= (win->_attrs & ATR_MSK);
12875	        if (win->_line[y][x] != ch) {   /* only if data change */
12876	                if (win->_minchng[y] == _NO_CHANGE)
12877	                        win->_minchng[y] = win->_maxchng[y] = x;
12878	                else if (x < win->_minchng[y])
12879	                        win->_minchng[y] = x;
12880	                else if (x > win->_maxchng[y])
12881	                        win->_maxchng[y] = x;
12882	        }                       /* if */
12883	        win->_line[y][x++] = ch;
12884	        if (x > win->_maxx) {   /* wrap around test */
12885	                x = 0;
12886	                if ((y = newline(win, y)) < 0) return(ERR);
12887	        }
12888	        break;
12889	
12890	  }                             /* switch */
12891	  win->_curx = x;
12892	  win->_cury = y;
12893	  return(OK);
12894	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/waddstr.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

12900	#include <curses.h>
12901	#include "curspriv.h"
12902	
12903	/****************************************************************/
12904	/* Waddstr() inserts string 'str' at the current cursor posi-   */
12905	/* Tion in window 'win', and takes any actions as dictated by   */
12906	/* The characters.                                              */
12907	/****************************************************************/
12908	
12909	int waddstr(win, str)
.Op 107 src/lib/curses/waddstr.c
12910	WINDOW *win;
12911	char *str;
12912	{
12913	  while (*str) {
12914	        if (waddch(win, *str++) == ERR) return(ERR);
12915	  }
12916	  return(OK);
12917	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/wbox.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

13000	#include <curses.h>
13001	#include "curspriv.h"
13002	
13003	/****************************************************************/
13004	/* Wbox(win,ymin,xmin,ymax,xmax,v,h) draws a box in window      */
13005	/* 'win', enclosing the area xmin-xmax and ymin-xmax. If        */
13006	/* Xmax and/or ymax is 0, the window max value is used. 'v' and */
13007	/* 'h' are the vertical and horizontal characters to use. If    */
13008	/* 'v' and 'h' are 0, wbox will use the alternate character set */
13009	/* In a pretty way.                                             */
13010	/****************************************************************/
13011	
13012	int wbox(win, ymin, xmin, ymax, xmax, v, h)
13013	WINDOW *win;
13014	int ymin, xmin, ymax, xmax;
13015	unsigned int v;
13016	unsigned int h;
13017	{
13018	  unsigned int vc, hc, ulc, urc, llc, lrc;      /* corner chars */
13019	  int i;
13020	
13021	  if (ymax == 0) ymax = win->_maxy;
13022	  if (xmax == 0) xmax = win->_maxx;
13023	
13024	  if (ymin >= win->_maxy || ymax > win->_maxy ||
13025	      xmin >= win->_maxx || xmax > win->_maxx ||
13026	      ymin >= ymax || xmin >= xmax)
13027	        return(ERR);
13028	
13029	  vc = v;
13030	  hc = h;
13031	  ulc = urc = llc = lrc = vc;   /* default same as vertical */
13032	
13033	  if (v == 0 && h == 0) {
13034	        ulc = ACS_ULCORNER;
13035	        urc = ACS_URCORNER;
13036	        llc = ACS_LLCORNER;
13037	        lrc = ACS_LRCORNER;
13038	        hc = ACS_HLINE;
13039	        vc = ACS_VLINE;
13040	  }
13041	  for (i = xmin + 1; i <= xmax - 1; i++) {
13042	        win->_line[ymin][i] = hc | win->_attrs;
13043	        win->_line[ymax][i] = hc | win->_attrs;
13044	  }
.Ep 108 src/lib/curses/wbox.c
13045	  for (i = ymin + 1; i <= ymax - 1; i++) {
13046	        win->_line[i][xmin] = vc | win->_attrs;
13047	        win->_line[i][xmax] = vc | win->_attrs;
13048	  }
13049	  win->_line[ymin][xmin] = ulc | win->_attrs;
13050	  win->_line[ymin][xmax] = urc | win->_attrs;
13051	  win->_line[ymax][xmin] = llc | win->_attrs;
13052	  win->_line[ymax][xmax] = lrc | win->_attrs;
13053	
13054	  for (i = ymin; i <= ymax; i++) {
13055	        if (win->_minchng[i] == _NO_CHANGE) {
13056	                win->_minchng[i] = xmin;
13057	                win->_maxchng[i] = xmax;
13058	        } else {
13059	                win->_minchng[i] = min(win->_minchng[i], xmin);
13060	                win->_maxchng[i] = max(win->_maxchng[i], xmax);
13061	        }
13062	  }
13063	  return(OK);
13064	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/wclear.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

13100	#include <curses.h>
13101	#include "curspriv.h"
13102	
13103	/****************************************************************/
13104	/* Wclear() fills all lines of window 'win' with blanks, and    */
13105	/* Marks the window to be cleared at next refresh operation.    */
13106	/****************************************************************/
13107	
13108	void wclear(win)
13109	WINDOW *win;
13110	{
13111	  werase(win);
13112	  win->_clear = TRUE;
13113	}                               /* wclear */


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/wclrtobot.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

13200	#include <curses.h>
13201	#include "curspriv.h"
13202	
13203	/****************************************************************/
13204	/* Wclrtobot() fills the right half of the cursor line of       */
13205	/* Window 'win', and all lines below it with blanks.            */
13206	/****************************************************************/
13207	
13208	int wclrtobot(win)
13209	WINDOW *win;
.Op 109 src/lib/curses/wclrtobot.c
13210	{
13211	  int y, minx, startx, *ptr, *end, *maxx, blank;
13212	
13213	  blank = ' ' | (win->_attrs & ATR_MSK);
13214	  startx = win->_curx;
13215	  for (y = win->_cury; y <= win->_regbottom; y++) {
13216	        minx = _NO_CHANGE;
13217	        end = &win->_line[y][win->_maxx];
13218	        for (ptr = &win->_line[y][startx]; ptr <= end; ptr++) {
13219	                if (*ptr != blank) {
13220	                        maxx = ptr;
13221	                        if (minx == _NO_CHANGE) minx = ptr - win->_line[y];
13222	                        *ptr = blank;
13223	                }               /* if */
13224	        }                       /* for */
13225	        if (minx != _NO_CHANGE) {
13226	                if ((win->_minchng[y] > minx) || (win->_minchng[y] == _NO_CHANGE))
13227	                        win->_minchng[y] = minx;
13228	                if (win->_maxchng[y] < maxx - win->_line[y])
13229	                        win->_maxchng[y] = maxx - win->_line[y];
13230	        }                       /* if */
13231	        startx = 0;
13232	  }
13233	  return(OK);
13234	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/wclrtoeol.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

13300	#include <curses.h>
13301	#include "curspriv.h"
13302	
13303	/****************************************************************/
13304	/* Wclrtoeol() fills the half of the cursor line to the right   */
13305	/* Of the cursor in window 'win' with blanks.                   */
13306	/****************************************************************/
13307	
13308	int wclrtoeol(win)
13309	WINDOW *win;
13310	{
13311	  int *maxx, *ptr, *end, y, x, minx, blank;
13312	
13313	  y = win->_cury;
13314	  x = win->_curx;
13315	  blank = ' ' | (win->_attrs & ATR_MSK);
13316	
13317	  end = &win->_line[y][win->_maxx];
13318	  minx = _NO_CHANGE;
13319	  maxx = &win->_line[y][x];
13320	  for (ptr = maxx; ptr <= end; ptr++) {
13321	        if (*ptr != blank) {
13322	                maxx = ptr;
13323	                if (minx == _NO_CHANGE) minx = ptr - win->_line[y];
13324	                *ptr = blank;
.Ep 110 src/lib/curses/wclrtoeol.c
13325	        }                       /* if */
13326	  }                             /* for */
13327	
13328	  if (minx != _NO_CHANGE) {
13329	        if (win->_minchng[y] > minx || win->_minchng[y] == _NO_CHANGE)
13330	                win->_minchng[y] = minx;
13331	        if (win->_maxchng[y] < maxx - win->_line[y])
13332	                win->_maxchng[y] = maxx - win->_line[y];
13333	  }
13334	  return(OK);
13335	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/wdelch.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

13400	#include <curses.h>
13401	#include "curspriv.h"
13402	
13403	/* Wdelch() deletes the character at the window cursor, and the
13404	   characters to the right of it are shifted left, inserting a
13405	   space at the last position of the line.
13406	*/
13407	
13408	int wdelch(win)
13409	WINDOW *win;
13410	{
13411	  int *temp1;
13412	  int *temp2;
13413	  int *end;
13414	  int y = win->_cury;
13415	  int x = win->_curx;
13416	  int maxx = win->_maxx;
13417	
13418	  end = &win->_line[y][maxx];
13419	  temp1 = &win->_line[y][x];
13420	  temp2 = temp1 + 1;
13421	  while (temp1 < end) *temp1++ = *temp2++;
13422	  *temp1 = ' ' | (win->_attrs & ATR_MSK);
13423	  win->_maxchng[y] = maxx;
13424	  if (win->_minchng[y] == _NO_CHANGE || win->_minchng[y] > x)
13425	        win->_minchng[y] = x;
13426	  return(OK);
13427	}
.Op 111 src/lib/curses/wdeleteln.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/wdeleteln.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

13500	#include <curses.h>
13501	#include "curspriv.h"
13502	
13503	/****************************************************************/
13504	/* Wdeleteln() deletes the line at the window cursor, and the   */
13505	/* Lines below it are shifted up, inserting a blank line at     */
13506	/* The bottom of the window.                                    */
13507	/****************************************************************/
13508	
13509	int wdeleteln(win)
13510	WINDOW *win;
13511	{
13512	  int *end, *temp, y, blank;
13513	
13514	  blank = ' ' | (win->_attrs & ATR_MSK);
13515	
13516	  temp = win->_line[win->_cury];
13517	  for (y = win->_cury; y < win->_regbottom; y++) {
13518	        win->_line[y] = win->_line[y + 1];
13519	        win->_minchng[y] = 0;
13520	        win->_maxchng[y] = win->_maxx;
13521	  }
13522	  win->_minchng[y] = 0;
13523	  win->_maxchng[y] = win->_maxx;
13524	  win->_line[win->_regbottom] = temp;
13525	  for (end = &(temp[win->_maxx]); temp <= end;) *temp++ = blank;
13526	  return(OK);
13527	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/werase.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

13600	#include <curses.h>
13601	#include "curspriv.h"
13602	
13603	/****************************************************************/
13604	/* Werase() fills all lines of window 'win' with blanks and po- */
13605	/* Sitions the cursor at home in the scroll region.             */
13606	/****************************************************************/
13607	
13608	void werase(win)
13609	WINDOW *win;
13610	{
13611	  int *end, *start, y, blank;
13612	
13613	  blank = ' ' | (win->_attrs & ATR_MSK);
13614	
13615	  for (y = win->_regtop; y <= win->_regbottom; y++) {   /* clear all lines */
13616	        start = win->_line[y];
13617	        end = &start[win->_maxx];
13618	        while (start <= end)    /* clear all line */
13619	                *start++ = blank;
.Ep 112 src/lib/curses/werase.c
13620	        win->_minchng[y] = 0;
13621	        win->_maxchng[y] = win->_maxx;
13622	  }
13623	  win->_cury = win->_regtop;    /* cursor home */
13624	  win->_curx = 0;
13625	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/wgetch.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

13700	#include <curses.h>
13701	#include <stdio.h>
13702	#include "curspriv.h"
13703	
13704	int wgetch(win)
13705	WINDOW *win;
13706	{
13707	  bool weset = FALSE;
13708	  char inp;
13709	
13710	  if (!win->_scroll && (win->_flags & _FULLWIN)
13711	      && win->_curx == win->_maxx - 1 && win->_cury == win->_maxy - 1)
13712	        return ERR;
13713	  if (_cursvar.echoit && !_cursvar.rawmode) {
13714	        cbreak();
13715	        weset++;
13716	  }
13717	  inp = getchar();
13718	  if (_cursvar.echoit) {
13719	        mvwaddch(curscr, win->_cury + win->_begy,
13720	                 win->_curx + win->_begx, inp);
13721	        waddch(win, inp);
13722	  }
13723	  if (weset) nocbreak();
13724	  return inp;
13725	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/wgetstr.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

13800	#include <curses.h>
13801	#include "curspriv.h"
13802	
13803	/****************************************************************/
13804	/* Wgetstr(win,str) reads in a string (terminated by \n or \r)  */
13805	/* To the buffer pointed to by 'str', and displays the input    */
13806	/* In window 'win'. The user's erase and kill characters are    */
13807	/* Active.                                                      */
13808	/****************************************************************/
13809	
.Op 113 src/lib/curses/wgetstr.c
13810	int wgetstr(win, str)
13811	WINDOW *win;
13812	char *str;
13813	{
13814	  while ((*str = wgetch(win)) != ERR && *str != '\n') str++;
13815	  if (*str == ERR) {
13816	        *str = '\0';
13817	        return ERR;
13818	  }
13819	  *str = '\0';
13820	  return OK;
13821	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/windel.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

13900	/****************************************************************/
13901	/* Delwin() routine of the PCcurses package.                    */
13902	/*                                                              */
13903	/****************************************************************/
13904	/* This version of curses is based on ncurses, a curses version */
13905	/* Originally written by Pavel Curtis at Cornell University.    */
13906	/* I have made substantial changes to make it run on IBM PC's,  */
13907	/* And therefore consider myself free to make it public domain. */
13908	/*              Bjorn Larsson (...mcvax!enea!infovax!bl)        */
13909	/****************************************************************/
13910	/* 1.0: Release:                                        870515  */
13911	/****************************************************************/
13912	/* Modified to run under the MINIX operating system by Don Cope */
13913	/* These changes are also released into the public domain.      */
13914	/*                                                      900906  */
13915	/****************************************************************/
13916	
13917	#include <stdlib.h>
13918	#include <curses.h>
13919	#include "curspriv.h"
13920	
13921	/****************************************************************/
13922	/* Delwin() deallocates all data allocated by 'win'. If 'win'   */
13923	/* Is a subwindow, it uses the original window's lines for sto- */
13924	/* Rage, and thus the line arrays are not deallocated.          */
13925	/****************************************************************/
13926	
13927	void delwin(win)
13928	WINDOW *win;
13929	{
13930	  int i;
13931	
13932	  if (!(win->_flags & _SUBWIN)) {       /* subwindow uses 'parent's' lines */
13933	        for (i = 0; i <= win->_maxy && win->_line[i]; i++)
13934	                free(win->_line[i]);
13935	  }
13936	  free(win->_minchng);
13937	  free(win->_maxchng);
13938	  free(win->_line);
13939	  free(win);
.Ep 114 src/lib/curses/windel.c
13940	}                               /* delwin */





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/winmove.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

14000	/****************************************************************/
14001	/* Mvwin() routine of the PCcurses package                      */
14002	/*                                                              */
14003	/****************************************************************/
14004	/* This version of curses is based on ncurses, a curses version */
14005	/* Originally written by Pavel Curtis at Cornell University.    */
14006	/* I have made substantial changes to make it run on IBM PC's,  */
14007	/* And therefore consider myself free to make it public domain. */
14008	/*              Bjorn Larsson (...mcvax!enea!infovax!bl)        */
14009	/****************************************************************/
14010	/* 1.0: Release:                                        870515  */
14011	/****************************************************************/
14012	/* Modified to run under the MINIX operating system by Don Cope */
14013	/* These changes are also released into the public domain.      */
14014	/*                                                      900906  */
14015	/****************************************************************/
14016	
14017	#include <curses.h>
14018	#include "curspriv.h"
14019	
14020	/****************************************************************/
14021	/* Mvwin() moves window 'win' to position (begx, begy) on the   */
14022	/* Screen.                                                      */
14023	/****************************************************************/
14024	
14025	int mvwin(win, begy, begx)
14026	WINDOW *win;
14027	int begy, begx;
14028	{
14029	  if ((begy + win->_maxy) > (LINES - 1) || (begx + win->_maxx) > (COLS - 1))
14030	        return(ERR);
14031	  win->_begy = begy;
14032	  win->_begx = begx;
14033	  touchwin(win);
14034	  return(OK);
14035	}                               /* mvwin */
.Op 115 src/lib/curses/winsch.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/winsch.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

14100	#include <curses.h>
14101	#include "curspriv.h"
14102	
14103	/* Winsch() inserts character 'c' at the cursor position in
14104	   window 'win'. The cursor is advanced.
14105	*/
14106	
14107	int winsch(win, c)
14108	WINDOW *win;
14109	char c;
14110	{
14111	  int *temp1;
14112	  int *temp2;
14113	  int *end;
14114	  int x = win->_curx;
14115	  int y = win->_cury;
14116	  int maxx = win->_maxx;
14117	
14118	  if ((c < ' ') && (c == '\n' || c == '\r' || c == '\t' || c == '\b'))
14119	        return(waddch(win, c));
14120	  end = &win->_line[y][x];
14121	  temp1 = &win->_line[y][maxx];
14122	  temp2 = temp1 - 1;
14123	  if (c < ' ')                  /* if CTRL-char make space for 2 */
14124	        temp2--;
14125	  while (temp1 > end) *temp1-- = *temp2--;
14126	  win->_maxchng[y] = maxx;
14127	  if ((win->_minchng[y] == _NO_CHANGE) || (win->_minchng[y] > x))
14128	        win->_minchng[y] = x;
14129	  return(waddch(win, c));       /* fixes CTRL-chars too */
14130	}                               /* winsch */





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/winscrol.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

14200	/****************************************************************/
14201	/* Scroll() routine of the PCcurses package                     */
14202	/*                                                              */
14203	/****************************************************************/
14204	/* This version of curses is based on ncurses, a curses version */
14205	/* Originally written by Pavel Curtis at Cornell University.    */
14206	/* I have made substantial changes to make it run on IBM PC's,  */
14207	/* And therefore consider myself free to make it public domain. */
14208	/*              Bjorn Larsson (...mcvax!enea!infovax!bl)        */
14209	/****************************************************************/
14210	/* 1.0: Release:                                        870515  */
14211	/****************************************************************/
14212	/* Modified to run under the MINIX operating system by Don Cope */
14213	/* These changes are also released into the public domain.      */
14214	/*                                                      900906  */
.Ep 116 src/lib/curses/winscrol.c
14215	/****************************************************************/
14216	
14217	#include <curses.h>
14218	#include "curspriv.h"
14219	
14220	/****************************************************************/
14221	/* Scroll() scrolls the scrolling region of 'win', but only if  */
14222	/* Scrolling is allowed and if the cursor is inside the scrol-  */
14223	/* Ling region.                                                 */
14224	/****************************************************************/
14225	
14226	void scroll(win)
14227	WINDOW *win;
14228	{
14229	  int i;
14230	  int *ptr;
14231	  int *temp;
14232	  static int blank;
14233	
14234	  blank = ' ' | (win->_attrs & ATR_MSK);
14235	  if ((!win->_scroll)           /* check if window scrolls */
14236	      ||(win->_cury < win->_regtop)     /* and cursor in region */
14237	      ||(win->_cury > win->_regbottom)
14238	        )
14239	        return;
14240	
14241	  temp = win->_line[win->_regtop];
14242	  for (i = win->_regtop; i < win->_regbottom; i++) {
14243	        win->_line[i] = win->_line[i + 1];      /* re-arrange line pointers */
14244	        win->_minchng[i] = 0;
14245	        win->_maxchng[i] = win->_maxx;
14246	  }
14247	  for (ptr = temp; ptr - temp <= win->_maxx; ptr++)
14248	        *ptr = blank;           /* make a blank line */
14249	  win->_line[win->_regbottom] = temp;
14250	  if (win->_cury > win->_regtop)/* if not on top line */
14251	        win->_cury--;           /* cursor scrolls too */
14252	  win->_minchng[win->_regbottom] = 0;
14253	  win->_maxchng[win->_regbottom] = win->_maxx;
14254	}                               /* scroll */






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/winsertln.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

14300	#include <curses.h>
14301	#include "curspriv.h"
14302	
14303	/****************************************************************/
14304	/* Winsertln() inserts a blank line instead of the cursor line  */
14305	/* In window 'win' and pushes other lines down.                 */
14306	/****************************************************************/
14307	
14308	int winsertln(win)
14309	WINDOW *win;
.Op 117 src/lib/curses/winsertln.c
14310	{
14311	  int *temp, *end, y, blank;
14312	
14313	  blank = ' ' | (win->_attrs & ATR_MSK);
14314	  temp = win->_line[win->_regbottom];
14315	  for (y = win->_regbottom; y > win->_cury; y--) {
14316	        win->_line[y] = win->_line[y - 1];
14317	        win->_minchng[y] = 0;
14318	        win->_maxchng[y] = win->_maxx;
14319	  }
14320	  win->_line[win->_cury] = temp;
14321	  for (end = &temp[win->_maxx]; temp <= end; temp++) *temp = blank;
14322	  win->_minchng[win->_cury] = 0;
14323	  win->_maxchng[win->_cury] = win->_maxx;
14324	  return(OK);
14325	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/curses/wintouch.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

14400	/****************************************************************/
14401	/* Touchwin() routine of the PCcurses package                   */
14402	/*                                                              */
14403	/****************************************************************/
14404	/* This version of curses is based on ncurses, a curses version */
14405	/* Originally written by Pavel Curtis at Cornell University.    */
14406	/* I have made substantial changes to make it run on IBM PC's,  */
14407	/* And therefore consider myself free to make it public domain. */
14408	/*              Bjorn Larsson (...mcvax!enea!infovax!bl)        */
14409	/****************************************************************/
14410	/* 1.0: Release:                                        870515  */
14411	/****************************************************************/
14412	/* Modified to run under the MINIX operating system by Don Cope */
14413	/* These changes are also released into the public domain.      */
14414	/*                                                      900906  */
14415	/****************************************************************/
14416	
14417	#include <curses.h>
14418	#include "curspriv.h"
14419	
14420	/****************************************************************/
14421	/* Touchwin() marks all lines of window 'win' as changed, from  */
14422	/* The first to the last character on the line.                 */
14423	/****************************************************************/
14424	
14425	void touchwin(win)
14426	WINDOW *win;
14427	{
14428	  int y;
14429	  int maxy;
14430	  int maxx;
14431	
14432	  maxy = win->_maxy;
14433	  maxx = win->_maxx;
14434	
.Ep 118 src/lib/curses/wintouch.c
14435	  for (y = 0; y <= maxy; y++) {
14436	        win->_minchng[y] = 0;
14437	        win->_maxchng[y] = maxx;
14438	  }                             /* for */
14439	}                               /* touchwin */






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/editline/editline.h	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

14500	/*  $Revision: 1.3 $
14501	**
14502	**  Internal header file for editline library.
14503	*/
14504	#include <stdio.h>
14505	#if     defined(HAVE_STDLIB)
14506	#include <stdlib.h>
14507	#include <string.h>
14508	#endif  /* defined(HAVE_STDLIB) */
14509	#if     defined(SYS_UNIX)
14510	#include "unix.h"
14511	#endif  /* defined(SYS_UNIX) */
14512	#if     defined(SYS_OS9)
14513	#include "os9.h"
14514	#endif  /* defined(SYS_OS9) */
14515	
14516	#if     !defined(SIZE_T)
14517	#define SIZE_T  unsigned int
14518	#endif  /* !defined(SIZE_T) */
14519	
14520	typedef unsigned char   CHAR;
14521	
14522	#if     defined(HIDE)
14523	#define STATIC  static
14524	#else
14525	#define STATIC  /* NULL */
14526	#endif  /* !defined(HIDE) */
14527	
14528	#if     !defined(CONST)
14529	#if     defined(__STDC__)
14530	#define CONST   const
14531	#else
14532	#define CONST
14533	#endif  /* defined(__STDC__) */
14534	#endif  /* !defined(CONST) */
14535	
14536	
14537	#define MEM_INC         64
14538	#define SCREEN_INC      256
14539	
14540	#define DISPOSE(p)      free((char *)(p))
14541	#define NEW(T, c)       \
14542	        ((T *)malloc((unsigned int)(sizeof (T) * (c))))
14543	#define RENEW(p, T, c)  \
14544	        (p = (T *)realloc((char *)(p), (unsigned int)(sizeof (T) * (c))))
.Op 119 src/lib/editline/editline.h
14545	#define COPYFROMTO(new, p, len) \
14546	        (void)memcpy((char *)(new), (char *)(p), (int)(len))
14547	
14548	
14549	/*
14550	**  Variables and routines internal to this package.
14551	*/
14552	extern int      rl_eof;
14553	extern int      rl_erase;
14554	extern int      rl_intr;
14555	extern int      rl_kill;
14556	extern int      rl_quit;
14557	extern char     *rl_complete();
14558	extern int      rl_list_possib();
14559	extern void     rl_ttyset();
14560	extern void     rl_add_slash();
14561	
14562	#if     !defined(HAVE_STDLIB)
14563	extern char     *getenv();
14564	extern char     *malloc();
14565	extern char     *realloc();
14566	extern char     *memcpy();
14567	extern char     *strcat();
14568	extern char     *strchr();
14569	extern char     *strrchr();
14570	extern char     *strcpy();
14571	extern int      strcmp();
14572	extern int      strlen();
14573	extern int      strncmp();
14574	#endif  /* !defined(HAVE_STDLIB) */
14575	
14576	#if     defined(NEED_STRDUP)
14577	extern char     *strdup();
14578	#endif


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/editline/os9.h	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

14600	/*  $Revision: 1.1 $
14601	**
14602	**  Editline system header file for OS-9 (on 68k).
14603	*/
14604	
14605	#define CRLF            "\r\l"
14606	#define FORWARD         extern
14607	
14608	#include <dir.h>
14609	typedef struct direct   DIRENTRY;
.Ep 120 src/lib/editline/unix.h

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/editline/unix.h	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

14700	/*  $Revision: 1.1 $
14701	**
14702	**  Editline system header file for Unix.
14703	*/
14704	
14705	#define CRLF            "\r\n"
14706	#define FORWARD         STATIC
14707	
14708	#include <sys/types.h>
14709	#include <sys/stat.h>
14710	
14711	#if     defined(USE_DIRENT)
14712	#include <dirent.h>
14713	typedef struct dirent   DIRENTRY;
14714	#else
14715	#include <sys/dir.h>
14716	typedef struct direct   DIRENTRY;
14717	#endif  /* defined(USE_DIRENT) */
14718	
14719	#if     !defined(S_ISDIR)
14720	#define S_ISDIR(m)              (((m) & S_IFMT) == S_IFDIR)
14721	#endif  /* !defined(S_ISDIR) */




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/editline/complete.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

14800	/*  $Revision: 1.4 $
14801	**
14802	**  History and file completion functions for editline library.
14803	*/
14804	#include "editline.h"
14805	
14806	
14807	#if     defined(NEED_STRDUP)
14808	/*
14809	**  Return an allocated copy of a string.
14810	*/
14811	char *
14812	strdup(p)
14813	    char        *p;
14814	{
14815	    char        *new;
14816	
14817	    if ((new = NEW(char, strlen(p) + 1)) != NULL)
14818	        (void)strcpy(new, p);
14819	    return new;
14820	}
14821	#endif  /* defined(NEED_STRDUP) */
14822	
14823	/*
14824	**  strcmp-like sorting predicate for qsort.
.Op 121 src/lib/editline/complete.c
14825	*/
14826	STATIC int
14827	compare(p1, p2)
14828	    CONST void  *p1;
14829	    CONST void  *p2;
14830	{
14831	    CONST char  **v1;
14832	    CONST char  **v2;
14833	
14834	    v1 = (CONST char **)p1;
14835	    v2 = (CONST char **)p2;
14836	    return strcmp(*v1, *v2);
14837	}
	
14839	/*
14840	**  Fill in *avp with an array of names that match file, up to its length.
14841	**  Ignore . and .. .
14842	*/
14843	STATIC int
14844	FindMatches(dir, file, avp)
14845	    char        *dir;
14846	    char        *file;
14847	    char        ***avp;
14848	{
14849	    char        **av;
14850	    char        **new;
14851	    char        *p;
14852	    DIR         *dp;
14853	    DIRENTRY    *ep;
14854	    SIZE_T      ac;
14855	    SIZE_T      len;
14856	    SIZE_T      choices;
14857	    SIZE_T      total;
14858	
14859	    if ((dp = opendir(dir)) == NULL)
14860	        return 0;
14861	
14862	    av = NULL;
14863	    ac = 0;
14864	    len = strlen(file);
14865	    choices = 0;
14866	    total = 0;
14867	    while ((ep = readdir(dp)) != NULL) {
14868	        p = ep->d_name;
14869	        if (p[0] == '.' && (p[1] == '\0' || (p[1] == '.' && p[2] == '\0')))
14870	            continue;
14871	        if (len && strncmp(p, file, len) != 0)
14872	            continue;
14873	
14874	        choices++;
14875	        if ((total += strlen(p)) > 1024) {
14876	            /* This is a bit too much. */
14877	            while (ac > 0) DISPOSE(av[--ac]);
14878	            continue;
14879	        }
14880	
14881	        if ((ac % MEM_INC) == 0) {
14882	            if ((new = NEW(char*, ac + MEM_INC)) == NULL) {
14883	                total = 0;
14884	                break;
.Ep 122 src/lib/editline/complete.c
14885	            }
14886	            if (ac) {
14887	                COPYFROMTO(new, av, ac * sizeof (char **));
14888	                DISPOSE(av);
14889	            }
14890	            *avp = av = new;
14891	        }
14892	
14893	        if ((av[ac] = strdup(p)) == NULL) {
14894	            if (ac == 0)
14895	                DISPOSE(av);
14896	            total = 0;
14897	            break;
14898	        }
14899	        ac++;
14900	    }
14901	
14902	    /* Clean up and return. */
14903	    (void)closedir(dp);
14904	    if (total > 1024) {
14905	        char many[sizeof(total) * 3];
14906	        p = many + sizeof(many);
14907	        *--p = '\0';
14908	        while (choices > 0) {
14909	           *--p = '0' + choices % 10;
14910	           choices /= 10;
14911	        }
14912	        while (p > many + sizeof(many) - 8) *--p = ' ';
14913	        if ((p = strdup(p)) != NULL) av[ac++] = p;
14914	        if ((p = strdup("choices")) != NULL) av[ac++] = p;
14915	    } else {
14916	        if (ac)
14917	            qsort(av, ac, sizeof (char **), compare);
14918	    }
14919	    return ac;
14920	}
	
14922	/*
14923	**  Split a pathname into allocated directory and trailing filename parts.
14924	*/
14925	STATIC int
14926	SplitPath(path, dirpart, filepart)
14927	    char        *path;
14928	    char        **dirpart;
14929	    char        **filepart;
14930	{
14931	    static char DOT[] = ".";
14932	    char        *dpart;
14933	    char        *fpart;
14934	
14935	    if ((fpart = strrchr(path, '/')) == NULL) {
14936	        if ((dpart = strdup(DOT)) == NULL)
14937	            return -1;
14938	        if ((fpart = strdup(path)) == NULL) {
14939	            DISPOSE(dpart);
14940	            return -1;
14941	        }
14942	    }
14943	    else {
14944	        if ((dpart = strdup(path)) == NULL)
.Op 123 src/lib/editline/complete.c
14945	            return -1;
14946	        dpart[fpart - path + 1] = '\0';
14947	        if ((fpart = strdup(++fpart)) == NULL) {
14948	            DISPOSE(dpart);
14949	            return -1;
14950	        }
14951	    }
14952	    *dirpart = dpart;
14953	    *filepart = fpart;
14954	    return 0;
14955	}
	
14957	/*
14958	**  Attempt to complete the pathname, returning an allocated copy.
14959	**  Fill in *unique if we completed it, or set it to 0 if ambiguous.
14960	*/
14961	char *
14962	rl_complete(pathname, unique)
14963	    char        *pathname;
14964	    int         *unique;
14965	{
14966	    char        **av;
14967	    char        *dir;
14968	    char        *file;
14969	    char        *new;
14970	    char        *p;
14971	    SIZE_T      ac;
14972	    SIZE_T      end;
14973	    SIZE_T      i;
14974	    SIZE_T      j;
14975	    SIZE_T      len;
14976	
14977	    if (SplitPath(pathname, &dir, &file) < 0)
14978	        return NULL;
14979	    if ((ac = FindMatches(dir, file, &av)) == 0) {
14980	        DISPOSE(dir);
14981	        DISPOSE(file);
14982	        return NULL;
14983	    }
14984	
14985	    p = NULL;
14986	    len = strlen(file);
14987	    if (ac == 1) {
14988	        /* Exactly one match -- finish it off. */
14989	        *unique = 1;
14990	        j = strlen(av[0]) - len + 2;
14991	        if ((p = NEW(char, j + 1)) != NULL) {
14992	            COPYFROMTO(p, av[0] + len, j);
14993	            if ((new = NEW(char, strlen(dir) + strlen(av[0]) + 2)) != NULL) {
14994	                (void)strcpy(new, dir);
14995	                (void)strcat(new, "/");
14996	                (void)strcat(new, av[0]);
14997	                rl_add_slash(new, p);
14998	                DISPOSE(new);
14999	            }
15000	        }
15001	    }
15002	    else {
15003	        *unique = 0;
15004	        if (len) {
.Ep 124 src/lib/editline/complete.c
15005	            /* Find largest matching substring. */
15006	            for (i = len, end = strlen(av[0]); i < end; i++)
15007	                for (j = 1; j < ac; j++)
15008	                    if (av[0][i] != av[j][i])
15009	                        goto breakout;
15010	  breakout:
15011	            if (i > len) {
15012	                j = i - len + 1;
15013	                if ((p = NEW(char, j)) != NULL) {
15014	                    COPYFROMTO(p, av[0] + len, j);
15015	                    p[j - 1] = '\0';
15016	                }
15017	            }
15018	        }
15019	    }
15020	
15021	    /* Clean up and return. */
15022	    DISPOSE(dir);
15023	    DISPOSE(file);
15024	    for (i = 0; i < ac; i++)
15025	        DISPOSE(av[i]);
15026	    DISPOSE(av);
15027	    return p;
15028	}
	
15030	/*
15031	**  Return all possible completions.
15032	*/
15033	int
15034	rl_list_possib(pathname, avp)
15035	    char        *pathname;
15036	    char        ***avp;
15037	{
15038	    char        *dir;
15039	    char        *file;
15040	    int         ac;
15041	
15042	    if (SplitPath(pathname, &dir, &file) < 0)
15043	        return 0;
15044	    ac = FindMatches(dir, file, avp);
15045	    DISPOSE(dir);
15046	    DISPOSE(file);
15047	    return ac;
15048	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/editline/editline.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

15100	/*  $Revision: 1.7 $
15101	**
15102	**  Main editing routines for editline library.
15103	*/
15104	#include "editline.h"
15105	#include <signal.h>
15106	#include <ctype.h>
15107	
15108	/*
15109	**  Manifest constants.
.Op 125 src/lib/editline/editline.c
15110	*/
15111	#define SCREEN_WIDTH    80
15112	#define SCREEN_ROWS     24
15113	#define NO_ARG          (-1)
15114	#define DEL             127
15115	#define CTL(x)          ((x) & 0x1F)
15116	#define ISCTL(x)        ((x) && (x) < ' ')
15117	#define UNCTL(x)        ((x) + 64)
15118	#define META(x)         ((x) | 0x80)
15119	#define ISMETA(x)       ((x) & 0x80)
15120	#define UNMETA(x)       ((x) & 0x7F)
15121	#if     !defined(HIST_SIZE)
15122	#define HIST_SIZE       20
15123	#endif  /* !defined(HIST_SIZE) */
15124	
15125	/*
15126	**  Command status codes.
15127	*/
15128	typedef enum _STATUS {
15129	    CSdone, CSeof, CSmove, CSdispatch, CSstay, CSsignal
15130	} STATUS;
15131	
15132	/*
15133	**  The type of case-changing to perform.
15134	*/
15135	typedef enum _CASE {
15136	    TOupper, TOlower
15137	} CASE;
15138	
15139	/*
15140	**  Key to command mapping.
15141	*/
15142	typedef struct _KEYMAP {
15143	    CHAR        Key;
15144	    STATUS      (*Function)();
15145	} KEYMAP;
15146	
15147	/*
15148	**  Command history structure.
15149	*/
15150	typedef struct _HISTORY {
15151	    int         Size;
15152	    int         Pos;
15153	    CHAR        *Lines[HIST_SIZE];
15154	} HISTORY;
15155	
15156	/*
15157	**  Globals.
15158	*/
15159	int             rl_eof;
15160	int             rl_erase;
15161	int             rl_intr;
15162	int             rl_kill;
15163	int             rl_quit;
15164	
15165	STATIC CHAR             NIL[] = "";
15166	STATIC CONST CHAR       *Input = NIL;
15167	STATIC CHAR             *Line;
15168	STATIC CONST char       *Prompt;
15169	STATIC CHAR             *Yanked;
.Ep 126 src/lib/editline/editline.c
15170	STATIC char             *Screen;
15171	STATIC char             NEWLINE[]= CRLF;
15172	STATIC HISTORY          H;
15173	STATIC int              Repeat;
15174	STATIC int              End;
15175	STATIC int              Mark;
15176	STATIC int              OldPoint;
15177	STATIC int              Point;
15178	STATIC int              PushBack;
15179	STATIC int              Pushed;
15180	STATIC int              Signal;
15181	FORWARD KEYMAP          Map[33];
15182	FORWARD KEYMAP          MetaMap[17];
15183	STATIC SIZE_T           Length;
15184	STATIC SIZE_T           ScreenCount;
15185	STATIC SIZE_T           ScreenSize;
15186	STATIC char             *backspace;
15187	STATIC int              TTYwidth;
15188	STATIC int              TTYrows;
15189	
15190	/* Display print 8-bit chars as `M-x' or as the actual 8-bit char? */
15191	int             rl_meta_chars = 0;
15192	
15193	/*
15194	**  Declarations.
15195	*/
15196	STATIC CHAR     *editinput();
15197	extern int      read();
15198	extern int      write();
15199	#if     defined(USE_TERMCAP)
15200	extern char     *getenv();
15201	extern char     *tgetstr();
15202	extern int      tgetent();
15203	#endif  /* defined(USE_TERMCAP) */
15204	
15205	/*
15206	**  TTY input/output functions.
15207	*/
15208	
15209	STATIC void
15210	TTYflush()
15211	{
15212	    if (ScreenCount) {
15213	        (void)write(1, Screen, ScreenCount);
15214	        ScreenCount = 0;
15215	    }
15216	}
	
15218	STATIC void
15219	TTYput(c)
15220	    CHAR        c;
15221	{
15222	    Screen[ScreenCount] = c;
15223	    if (++ScreenCount >= ScreenSize - 1) {
15224	        ScreenSize += SCREEN_INC;
15225	        RENEW(Screen, char, ScreenSize);
15226	    }
15227	}
	
15229	STATIC void
.Op 127 src/lib/editline/editline.c
15230	TTYputs(p)
15231	    CHAR        *p;
15232	{
15233	    while (*p)
15234	        TTYput(*p++);
15235	}
	
15237	STATIC void
15238	TTYshow(c)
15239	    CHAR        c;
15240	{
15241	    if (c == DEL) {
15242	        TTYput('^');
15243	        TTYput('?');
15244	    }
15245	    else if (ISCTL(c)) {
15246	        TTYput('^');
15247	        TTYput(UNCTL(c));
15248	    }
15249	    else if (rl_meta_chars && ISMETA(c)) {
15250	        TTYput('M');
15251	        TTYput('-');
15252	        TTYput(UNMETA(c));
15253	    }
15254	    else
15255	        TTYput(c);
15256	}
	
15258	STATIC void
15259	TTYstring(p)
15260	    CHAR        *p;
15261	{
15262	    while (*p)
15263	        TTYshow(*p++);
15264	}
	
15266	STATIC unsigned int
15267	TTYget()
15268	{
15269	    CHAR        c;
15270	
15271	    TTYflush();
15272	    if (Pushed) {
15273	        Pushed = 0;
15274	        return PushBack;
15275	    }
15276	    if (*Input)
15277	        return *Input++;
15278	    return read(0, &c, (SIZE_T)1) == 1 ? c : EOF;
15279	}
	
15281	#define TTYback()       (backspace ? TTYputs((CHAR *)backspace) : TTYput('\b'))
15282	
15283	STATIC void
15284	TTYbackn(n)
15285	    int         n;
15286	{
15287	    while (--n >= 0)
15288	        TTYback();
15289	}
.Ep 128 src/lib/editline/editline.c
	
15291	STATIC void
15292	TTYinfo()
15293	{
15294	    static int          init;
15295	#if     defined(USE_TERMCAP)
15296	    char                *term;
15297	    char                buff[2048];
15298	    char                *bp;
15299	#endif  /* defined(USE_TERMCAP) */
15300	#if     defined(TIOCGWINSZ)
15301	    struct winsize      W;
15302	#endif  /* defined(TIOCGWINSZ) */
15303	
15304	    if (init) {
15305	#if     defined(TIOCGWINSZ)
15306	        /* Perhaps we got resized. */
15307	        if (ioctl(0, TIOCGWINSZ, &W) >= 0
15308	         && W.ws_col > 0 && W.ws_row > 0) {
15309	            TTYwidth = (int)W.ws_col;
15310	            TTYrows = (int)W.ws_row;
15311	        }
15312	#endif  /* defined(TIOCGWINSZ) */
15313	        return;
15314	    }
15315	    init++;
15316	
15317	    TTYwidth = TTYrows = 0;
15318	#if     defined(USE_TERMCAP)
15319	    bp = &buff[0];
15320	    if ((term = getenv("TERM")) == NULL)
15321	        term = "dumb";
15322	    if (tgetent(buff, term) < 0) {
15323	       TTYwidth = SCREEN_WIDTH;
15324	       TTYrows = SCREEN_ROWS;
15325	       return;
15326	    }
15327	    if ((backspace = tgetstr("le", &bp)) != NULL)
15328	        backspace = strdup(backspace);
15329	    TTYwidth = tgetnum("co");
15330	    TTYrows = tgetnum("li");
15331	#endif  /* defined(USE_TERMCAP) */
15332	
15333	#if     defined(TIOCGWINSZ)
15334	    if (ioctl(0, TIOCGWINSZ, &W) >= 0) {
15335	        TTYwidth = (int)W.ws_col;
15336	        TTYrows = (int)W.ws_row;
15337	    }
15338	#endif  /* defined(TIOCGWINSZ) */
15339	
15340	    if (TTYwidth <= 0 || TTYrows <= 0) {
15341	        TTYwidth = SCREEN_WIDTH;
15342	        TTYrows = SCREEN_ROWS;
15343	    }
15344	}
15345	
15346	
15347	/*
15348	**  Print an array of words in columns.
15349	*/
.Op 129 src/lib/editline/editline.c
15350	STATIC void
15351	columns(ac, av)
15352	    int         ac;
15353	    CHAR        **av;
15354	{
15355	    CHAR        *p;
15356	    int         i;
15357	    int         j;
15358	    int         k;
15359	    int         len;
15360	    int         skip;
15361	    int         longest;
15362	    int         cols;
15363	
15364	    /* Find longest name, determine column count from that. */
15365	    for (longest = 0, i = 0; i < ac; i++)
15366	        if ((j = strlen((char *)av[i])) > longest)
15367	            longest = j;
15368	    cols = TTYwidth / (longest + 3);
15369	
15370	    TTYputs((CHAR *)NEWLINE);
15371	    for (skip = ac / cols + 1, i = 0; i < skip; i++) {
15372	        for (j = i; j < ac; j += skip) {
15373	            for (p = av[j], len = strlen((char *)p), k = len; --k >= 0; p++)
15374	                TTYput(*p);
15375	            if (j + skip < ac)
15376	                while (++len < longest + 3)
15377	                    TTYput(' ');
15378	        }
15379	        TTYputs((CHAR *)NEWLINE);
15380	    }
15381	}
	
15383	STATIC void
15384	reposition()
15385	{
15386	    int         i;
15387	    CHAR        *p;
15388	
15389	    TTYput('\r');
15390	    TTYputs((CONST CHAR *)Prompt);
15391	    for (i = Point, p = Line; --i >= 0; p++)
15392	        TTYshow(*p);
15393	}
	
15395	STATIC void
15396	left(Change)
15397	    STATUS      Change;
15398	{
15399	    TTYback();
15400	    if (Point) {
15401	        if (ISCTL(Line[Point - 1]))
15402	            TTYback();
15403	        else if (rl_meta_chars && ISMETA(Line[Point - 1])) {
15404	            TTYback();
15405	            TTYback();
15406	        }
15407	    }
15408	    if (Change == CSmove)
15409	        Point--;
.Ep 130 src/lib/editline/editline.c
15410	}
	
15412	STATIC void
15413	right(Change)
15414	    STATUS      Change;
15415	{
15416	    TTYshow(Line[Point]);
15417	    if (Change == CSmove)
15418	        Point++;
15419	}
	
15421	STATIC STATUS
15422	ring_bell()
15423	{
15424	    TTYput('\07');
15425	    TTYflush();
15426	    return CSstay;
15427	}
	
15429	STATIC STATUS
15430	do_macro(c)
15431	    unsigned int        c;
15432	{
15433	    CHAR                name[4];
15434	
15435	    name[0] = '_';
15436	    name[1] = c;
15437	    name[2] = '_';
15438	    name[3] = '\0';
15439	
15440	    if ((Input = (CHAR *)getenv((char *)name)) == NULL) {
15441	        Input = NIL;
15442	        return ring_bell();
15443	    }
15444	    return CSstay;
15445	}
	
15447	STATIC STATUS
15448	do_forward(move)
15449	    STATUS      move;
15450	{
15451	    int         i;
15452	    CHAR        *p;
15453	
15454	    i = 0;
15455	    do {
15456	        p = &Line[Point];
15457	        for ( ; Point < End && (*p == ' ' || !isalnum(*p)); Point++, p++)
15458	            if (move == CSmove)
15459	                right(CSstay);
15460	
15461	        for (; Point < End && isalnum(*p); Point++, p++)
15462	            if (move == CSmove)
15463	                right(CSstay);
15464	
15465	        if (Point == End)
15466	            break;
15467	    } while (++i < Repeat);
15468	
15469	    return CSstay;
.Op 131 src/lib/editline/editline.c
15470	}
	
15472	STATIC STATUS
15473	do_case(type)
15474	    CASE        type;
15475	{
15476	    int         i;
15477	    int         end;
15478	    int         count;
15479	    CHAR        *p;
15480	
15481	    (void)do_forward(CSstay);
15482	    if (OldPoint != Point) {
15483	        if ((count = Point - OldPoint) < 0)
15484	            count = -count;
15485	        Point = OldPoint;
15486	        if ((end = Point + count) > End)
15487	            end = End;
15488	        for (i = Point, p = &Line[i]; i < end; i++, p++) {
15489	            if (type == TOupper) {
15490	                if (islower(*p))
15491	                    *p = toupper(*p);
15492	            }
15493	            else if (isupper(*p))
15494	                *p = tolower(*p);
15495	            right(CSmove);
15496	        }
15497	    }
15498	    return CSstay;
15499	}
	
15501	STATIC STATUS
15502	case_down_word()
15503	{
15504	    return do_case(TOlower);
15505	}
	
15507	STATIC STATUS
15508	case_up_word()
15509	{
15510	    return do_case(TOupper);
15511	}
	
15513	STATIC void
15514	ceol()
15515	{
15516	    int         extras;
15517	    int         i;
15518	    CHAR        *p;
15519	
15520	    for (extras = 0, i = Point, p = &Line[i]; i <= End; i++, p++) {
15521	        TTYput(' ');
15522	        if (ISCTL(*p)) {
15523	            TTYput(' ');
15524	            extras++;
15525	        }
15526	        else if (rl_meta_chars && ISMETA(*p)) {
15527	            TTYput(' ');
15528	            TTYput(' ');
15529	            extras += 2;
.Ep 132 src/lib/editline/editline.c
15530	        }
15531	    }
15532	
15533	    for (i += extras; i > Point; i--)
15534	        TTYback();
15535	}
	
15537	STATIC void
15538	clear_line()
15539	{
15540	    Point = -strlen(Prompt);
15541	    TTYput('\r');
15542	    ceol();
15543	    Point = 0;
15544	    End = 0;
15545	    Line[0] = '\0';
15546	}
	
15548	STATIC STATUS
15549	insert_string(p)
15550	    CHAR        *p;
15551	{
15552	    SIZE_T      len;
15553	    int         i;
15554	    CHAR        *new;
15555	    CHAR        *q;
15556	
15557	    len = strlen((char *)p);
15558	    if (End + len >= Length) {
15559	        if ((new = NEW(CHAR, Length + len + MEM_INC)) == NULL)
15560	            return CSstay;
15561	        if (Length) {
15562	            COPYFROMTO(new, Line, Length);
15563	            DISPOSE(Line);
15564	        }
15565	        Line = new;
15566	        Length += len + MEM_INC;
15567	    }
15568	
15569	    for (q = &Line[Point], i = End - Point; --i >= 0; )
15570	        q[len + i] = q[i];
15571	    COPYFROMTO(&Line[Point], p, len);
15572	    End += len;
15573	    Line[End] = '\0';
15574	    TTYstring(&Line[Point]);
15575	    Point += len;
15576	
15577	    return Point == End ? CSstay : CSmove;
15578	}
	
15580	STATIC STATUS
15581	redisplay()
15582	{
15583	    TTYputs((CONST CHAR *)NEWLINE);
15584	    TTYputs((CONST CHAR *)Prompt);
15585	    TTYstring(Line);
15586	    return CSmove;
15587	}
	
15589	STATIC STATUS
.Op 133 src/lib/editline/editline.c
15590	toggle_meta_mode()
15591	{
15592	    rl_meta_chars = ! rl_meta_chars;
15593	    return redisplay();
15594	}
15595	
15596	
15597	STATIC CHAR *
15598	next_hist()
15599	{
15600	    return H.Pos >= H.Size - 1 ? NULL : H.Lines[++H.Pos];
15601	}
	
15603	STATIC CHAR *
15604	prev_hist()
15605	{
15606	    return H.Pos == 0 ? NULL : H.Lines[--H.Pos];
15607	}
	
15609	STATIC STATUS
15610	do_insert_hist(p)
15611	    CHAR        *p;
15612	{
15613	    if (p == NULL)
15614	        return ring_bell();
15615	    Point = 0;
15616	    reposition();
15617	    ceol();
15618	    End = 0;
15619	    return insert_string(p);
15620	}
	
15622	STATIC STATUS
15623	do_hist(move)
15624	    CHAR        *(*move)();
15625	{
15626	    CHAR        *p;
15627	    int         i;
15628	
15629	    i = 0;
15630	    do {
15631	        if ((p = (*move)()) == NULL)
15632	            return ring_bell();
15633	    } while (++i < Repeat);
15634	    return do_insert_hist(p);
15635	}
	
15637	STATIC STATUS
15638	h_next()
15639	{
15640	    return do_hist(next_hist);
15641	}
	
15643	STATIC STATUS
15644	h_prev()
15645	{
15646	    return do_hist(prev_hist);
15647	}
	
15649	STATIC STATUS
.Ep 134 src/lib/editline/editline.c
15650	h_first()
15651	{
15652	    return do_insert_hist(H.Lines[H.Pos = 0]);
15653	}
	
15655	STATIC STATUS
15656	h_last()
15657	{
15658	    return do_insert_hist(H.Lines[H.Pos = H.Size - 1]);
15659	}
	
15661	/*
15662	**  Return zero if pat appears as a substring in text.
15663	*/
15664	STATIC int
15665	substrcmp(text, pat, len)
15666	    char        *text;
15667	    char        *pat;
15668	    int         len;
15669	{
15670	    char        c;
15671	
15672	    if ((c = *pat) == '\0')
15673	        return *text == '\0';
15674	    for ( ; *text; text++)
15675	        if (*text == c && strncmp(text, pat, len) == 0)
15676	            return 0;
15677	    return 1;
15678	}
	
15680	STATIC CHAR *
15681	search_hist(search, move)
15682	    CHAR        *search;
15683	    CHAR        *(*move)();
15684	{
15685	    static CHAR *old_search;
15686	    int         len;
15687	    int         pos;
15688	    int         (*match)();
15689	    char        *pat;
15690	
15691	    /* Save or get remembered search pattern. */
15692	    if (search && *search) {
15693	        if (old_search)
15694	            DISPOSE(old_search);
15695	        old_search = (CHAR *)strdup((char *)search);
15696	    }
15697	    else {
15698	        if (old_search == NULL || *old_search == '\0')
15699	            return NULL;
15700	        search = old_search;
15701	    }
15702	
15703	    /* Set up pattern-finder. */
15704	    if (*search == '^') {
15705	        match = strncmp;
15706	        pat = (char *)(search + 1);
15707	    }
15708	    else {
15709	        match = substrcmp;
.Op 135 src/lib/editline/editline.c
15710	        pat = (char *)search;
15711	    }
15712	    len = strlen(pat);
15713	
15714	    for (pos = H.Pos; (*move)() != NULL; )
15715	        if ((*match)((char *)H.Lines[H.Pos], pat, len) == 0)
15716	            return H.Lines[H.Pos];
15717	    H.Pos = pos;
15718	    return NULL;
15719	}
	
15721	STATIC STATUS
15722	h_search()
15723	{
15724	    static int  Searching;
15725	    CONST char  *old_prompt;
15726	    CHAR        *(*move)();
15727	    CHAR        *p;
15728	
15729	    if (Searching)
15730	        return ring_bell();
15731	    Searching = 1;
15732	
15733	    clear_line();
15734	    old_prompt = Prompt;
15735	    Prompt = "Search: ";
15736	    TTYputs((CONST CHAR *)Prompt);
15737	    move = Repeat == NO_ARG ? prev_hist : next_hist;
15738	    p = editinput();
15739	    Prompt = old_prompt;
15740	    Searching = 0;
15741	    TTYputs((CONST CHAR *)Prompt);
15742	    if (p == NULL && Signal > 0) {
15743	        Signal = 0;
15744	        clear_line();
15745	        return redisplay();
15746	    }
15747	    p = search_hist(p, move);
15748	    clear_line();
15749	    if (p == NULL) {
15750	        (void)ring_bell();
15751	        return redisplay();
15752	    }
15753	    return do_insert_hist(p);
15754	}
	
15756	STATIC STATUS
15757	fd_char()
15758	{
15759	    int         i;
15760	
15761	    i = 0;
15762	    do {
15763	        if (Point >= End)
15764	            break;
15765	        right(CSmove);
15766	    } while (++i < Repeat);
15767	    return CSstay;
15768	}
	
.Ep 136 src/lib/editline/editline.c
15770	STATIC void
15771	save_yank(begin, i)
15772	    int         begin;
15773	    int         i;
15774	{
15775	    if (Yanked) {
15776	        DISPOSE(Yanked);
15777	        Yanked = NULL;
15778	    }
15779	
15780	    if (i < 1)
15781	        return;
15782	
15783	    if ((Yanked = NEW(CHAR, (SIZE_T)i + 1)) != NULL) {
15784	        COPYFROMTO(Yanked, &Line[begin], i);
15785	        Yanked[i] = '\0';
15786	    }
15787	}
	
15789	STATIC STATUS
15790	delete_string(count)
15791	    int         count;
15792	{
15793	    int         i;
15794	    CHAR        *p;
15795	
15796	    if (count <= 0 || End == Point)
15797	        return ring_bell();
15798	
15799	    if (count == 1 && Point == End - 1) {
15800	        /* Optimize common case of delete at end of line. */
15801	        End--;
15802	        p = &Line[Point];
15803	        i = 1;
15804	        TTYput(' ');
15805	        if (ISCTL(*p)) {
15806	            i = 2;
15807	            TTYput(' ');
15808	        }
15809	        else if (rl_meta_chars && ISMETA(*p)) {
15810	            i = 3;
15811	            TTYput(' ');
15812	            TTYput(' ');
15813	        }
15814	        TTYbackn(i);
15815	        *p = '\0';
15816	        return CSmove;
15817	    }
15818	    if (Point + count > End && (count = End - Point) <= 0)
15819	        return CSstay;
15820	
15821	    if (count > 1)
15822	        save_yank(Point, count);
15823	
15824	    for (p = &Line[Point], i = End - (Point + count) + 1; --i >= 0; p++)
15825	        p[0] = p[count];
15826	    ceol();
15827	    End -= count;
15828	    TTYstring(&Line[Point]);
15829	    return CSmove;
.Op 137 src/lib/editline/editline.c
15830	}
	
15832	STATIC STATUS
15833	bk_char()
15834	{
15835	    int         i;
15836	
15837	    i = 0;
15838	    do {
15839	        if (Point == 0)
15840	            break;
15841	        left(CSmove);
15842	    } while (++i < Repeat);
15843	
15844	    return CSstay;
15845	}
	
15847	STATIC STATUS
15848	bk_del_char()
15849	{
15850	    int         i;
15851	
15852	    i = 0;
15853	    do {
15854	        if (Point == 0)
15855	            break;
15856	        left(CSmove);
15857	    } while (++i < Repeat);
15858	
15859	    return delete_string(i);
15860	}
	
15862	STATIC STATUS
15863	kill_line()
15864	{
15865	    int         i;
15866	
15867	    if (Repeat != NO_ARG) {
15868	        if (Repeat < Point) {
15869	            i = Point;
15870	            Point = Repeat;
15871	            reposition();
15872	            (void)delete_string(i - Point);
15873	        }
15874	        else if (Repeat > Point) {
15875	            right(CSmove);
15876	            (void)delete_string(Repeat - Point - 1);
15877	        }
15878	        return CSmove;
15879	    }
15880	
15881	    save_yank(Point, End - Point);
15882	    Line[Point] = '\0';
15883	    ceol();
15884	    End = Point;
15885	    return CSstay;
15886	}
	
15888	STATIC STATUS
15889	insert_char(c)
.Ep 138 src/lib/editline/editline.c
15890	    int         c;
15891	{
15892	    STATUS      s;
15893	    CHAR        buff[2];
15894	    CHAR        *p;
15895	    CHAR        *q;
15896	    int         i;
15897	
15898	    if (Repeat == NO_ARG || Repeat < 2) {
15899	        buff[0] = c;
15900	        buff[1] = '\0';
15901	        return insert_string(buff);
15902	    }
15903	
15904	    if ((p = NEW(CHAR, Repeat + 1)) == NULL)
15905	        return CSstay;
15906	    for (i = Repeat, q = p; --i >= 0; )
15907	        *q++ = c;
15908	    *q = '\0';
15909	    Repeat = 0;
15910	    s = insert_string(p);
15911	    DISPOSE(p);
15912	    return s;
15913	}
	
15915	STATIC STATUS
15916	meta()
15917	{
15918	    unsigned int        c;
15919	    KEYMAP              *kp;
15920	
15921	    if ((c = TTYget()) == EOF)
15922	        return CSeof;
15923	#if     defined(ANSI_ARROWS)
15924	    /* Also include VT-100 arrows. */
15925	    if (c == '[' || c == 'O')
15926	        switch (c = TTYget()) {
15927	        default:        return ring_bell();
15928	        case EOF:       return CSeof;
15929	        case 'A':       return h_prev();
15930	        case 'B':       return h_next();
15931	        case 'C':       return fd_char();
15932	        case 'D':       return bk_char();
15933	        }
15934	#endif  /* defined(ANSI_ARROWS) */
15935	
15936	    if (isdigit(c)) {
15937	        for (Repeat = c - '0'; (c = TTYget()) != EOF && isdigit(c); )
15938	            Repeat = Repeat * 10 + c - '0';
15939	        Pushed = 1;
15940	        PushBack = c;
15941	        return CSstay;
15942	    }
15943	
15944	    if (isupper(c))
15945	        return do_macro(c);
15946	    for (kp = MetaMap; kp->Function; kp++)
15947	        if (kp->Key == c)
15948	            return (*kp->Function)();
15949	
.Op 139 src/lib/editline/editline.c
15950	    return ring_bell();
15951	}
	
15953	STATIC STATUS
15954	emacs(c)
15955	    unsigned int        c;
15956	{
15957	    STATUS              s;
15958	    KEYMAP              *kp;
15959	
15960	    OldPoint = Point;
15961	    if (rl_meta_chars && ISMETA(c)) {
15962	        Pushed = 1;
15963	        PushBack = UNMETA(c);
15964	        return meta();
15965	    }
15966	    for (kp = Map; kp->Function; kp++)
15967	        if (kp->Key == c)
15968	            break;
15969	    s = kp->Function ? (*kp->Function)() : insert_char((int)c);
15970	    if (!Pushed)
15971	        /* No pushback means no repeat count; hacky, but true. */
15972	        Repeat = NO_ARG;
15973	    return s;
15974	}
	
15976	STATIC STATUS
15977	TTYspecial(c)
15978	    unsigned int        c;
15979	{
15980	    if (ISMETA(c))
15981	        return CSdispatch;
15982	
15983	    if (c == rl_erase || c == DEL)
15984	        return bk_del_char();
15985	    if (c == rl_kill) {
15986	        if (Point != 0) {
15987	            Point = 0;
15988	            reposition();
15989	        }
15990	        Repeat = NO_ARG;
15991	        return kill_line();
15992	    }
15993	    if (c == rl_eof && Point == 0 && End == 0)
15994	        return CSeof;
15995	    if (c == rl_intr) {
15996	        Signal = SIGINT;
15997	        return CSsignal;
15998	    }
15999	    if (c == rl_quit) {
16000	        Signal = SIGQUIT;
16001	        return CSeof;
16002	    }
16003	
16004	    return CSdispatch;
16005	}
	
16007	STATIC CHAR *
16008	editinput()
16009	{
.Ep 140 src/lib/editline/editline.c
16010	    unsigned int        c;
16011	
16012	    Repeat = NO_ARG;
16013	    OldPoint = Point = Mark = End = 0;
16014	    Line[0] = '\0';
16015	
16016	    Signal = -1;
16017	    while ((c = TTYget()) != EOF)
16018	        switch (TTYspecial(c)) {
16019	        case CSdone:
16020	            return Line;
16021	        case CSeof:
16022	            return NULL;
16023	        case CSsignal:
16024	            return (CHAR *)"";
16025	        case CSmove:
16026	            reposition();
16027	            break;
16028	        case CSdispatch:
16029	            switch (emacs(c)) {
16030	            case CSdone:
16031	                return Line;
16032	            case CSeof:
16033	                return NULL;
16034	            case CSsignal:
16035	                return (CHAR *)"";
16036	            case CSmove:
16037	                reposition();
16038	                break;
16039	            case CSdispatch:
16040	            case CSstay:
16041	                break;
16042	            }
16043	            break;
16044	        case CSstay:
16045	            break;
16046	        }
16047	    return NULL;
16048	}
	
16050	STATIC void
16051	hist_add(p)
16052	    CHAR        *p;
16053	{
16054	    int         i;
16055	
16056	    if ((p = (CHAR *)strdup((char *)p)) == NULL)
16057	        return;
16058	    if (H.Size < HIST_SIZE)
16059	        H.Lines[H.Size++] = p;
16060	    else {
16061	        DISPOSE(H.Lines[0]);
16062	        for (i = 0; i < HIST_SIZE - 1; i++)
16063	            H.Lines[i] = H.Lines[i + 1];
16064	        H.Lines[i] = p;
16065	    }
16066	    H.Pos = H.Size - 1;
16067	}
	
16069	/*
.Op 141 src/lib/editline/editline.c
16070	**  For compatibility with FSF readline.
16071	*/
16072	/* ARGSUSED0 */
16073	void
16074	rl_reset_terminal(p)
16075	    char        *p;
16076	{
16077	}
	
16079	void
16080	rl_initialize()
16081	{
16082	}
	
16084	char *
16085	readline(prompt)
16086	    CONST char  *prompt;
16087	{
16088	    CHAR        *line;
16089	    int         s;
16090	
16091	    if (Line == NULL) {
16092	        Length = MEM_INC;
16093	        if ((Line = NEW(CHAR, Length)) == NULL)
16094	            return NULL;
16095	    }
16096	
16097	    TTYinfo();
16098	    rl_ttyset(0);
16099	    hist_add(NIL);
16100	    ScreenSize = SCREEN_INC;
16101	    Screen = NEW(char, ScreenSize);
16102	    Prompt = prompt ? prompt : (char *)NIL;
16103	    TTYputs((CONST CHAR *)Prompt);
16104	    if ((line = editinput()) != NULL) {
16105	        line = (CHAR *)strdup((char *)line);
16106	        TTYputs((CHAR *)NEWLINE);
16107	        TTYflush();
16108	    }
16109	    rl_ttyset(1);
16110	    DISPOSE(Screen);
16111	    DISPOSE(H.Lines[--H.Size]);
16112	    if (Signal > 0) {
16113	        s = Signal;
16114	        Signal = 0;
16115	        (void)kill(getpid(), s);
16116	    }
16117	    return (char *)line;
16118	}
	
16120	void
16121	add_history(p)
16122	    char        *p;
16123	{
16124	    if (p == NULL || *p == '\0')
16125	        return;
16126	
16127	#if     defined(UNIQUE_HISTORY)
16128	    if (H.Pos && strcmp(p, (char *) H.Lines[H.Pos - 1]) == 0)
16129	        return;
.Ep 142 src/lib/editline/editline.c
16130	#endif  /* defined(UNIQUE_HISTORY) */
16131	    if (H.Size && strcmp(p, (char *) H.Lines[H.Size - 1]) == 0)
16132	        return;
16133	    hist_add((CHAR *)p);
16134	}
16135	
16136	
16137	STATIC STATUS
16138	beg_line()
16139	{
16140	    if (Point) {
16141	        Point = 0;
16142	        return CSmove;
16143	    }
16144	    return CSstay;
16145	}
	
16147	STATIC STATUS
16148	del_char()
16149	{
16150	    return delete_string(Repeat == NO_ARG ? 1 : Repeat);
16151	}
	
16153	STATIC STATUS
16154	end_line()
16155	{
16156	    if (Point != End) {
16157	        Point = End;
16158	        return CSmove;
16159	    }
16160	    return CSstay;
16161	}
	
16163	/*
16164	**  Move back to the beginning of the current word and return an
16165	**  allocated copy of it.
16166	*/
16167	STATIC CHAR *
16168	find_word()
16169	{
16170	    static char SEPS[] = "#:;&|^$=`'{}()<>\n\t ";
16171	    CHAR        *p;
16172	    CHAR        *new;
16173	    SIZE_T      len;
16174	
16175	    for (p = &Line[Point]; p > Line && strchr(SEPS, (char)p[-1]) == NULL; p--)
16176	        continue;
16177	    len = Point - (p - Line) + 1;
16178	    if ((new = NEW(CHAR, len)) == NULL)
16179	        return NULL;
16180	    COPYFROMTO(new, p, len);
16181	    new[len - 1] = '\0';
16182	    return new;
16183	}
	
16185	STATIC STATUS
16186	c_possible()
16187	{
16188	    CHAR        **av;
16189	    CHAR        *word;
.Op 143 src/lib/editline/editline.c
16190	    int         ac;
16191	
16192	    word = find_word();
16193	    ac = rl_list_possib((char *)word, (char ***)&av);
16194	    if (word)
16195	        DISPOSE(word);
16196	    if (ac) {
16197	        columns(ac, av);
16198	        while (--ac >= 0)
16199	            DISPOSE(av[ac]);
16200	        DISPOSE(av);
16201	        return CSmove;
16202	    }
16203	    return ring_bell();
16204	}
	
16206	STATIC STATUS
16207	c_complete()
16208	{
16209	    CHAR        *p;
16210	    CHAR        *word;
16211	    int         unique;
16212	    STATUS      s;
16213	
16214	    word = find_word();
16215	    p = (CHAR *)rl_complete((char *)word, &unique);
16216	    if (word)
16217	        DISPOSE(word);
16218	    if (p && *p) {
16219	        s = insert_string(p);
16220	#if ANNOYING_NOISE
16221	        if (!unique)
16222	            (void)ring_bell();
16223	#endif
16224	        DISPOSE(p);
16225	        return s;
16226	    }
16227	    return c_possible();
16228	}
	
16230	STATIC STATUS
16231	accept_line()
16232	{
16233	    Line[End] = '\0';
16234	    return CSdone;
16235	}
	
16237	STATIC STATUS
16238	transpose()
16239	{
16240	    CHAR        c;
16241	
16242	    if (Point) {
16243	        if (Point == End)
16244	            left(CSmove);
16245	        c = Line[Point - 1];
16246	        left(CSstay);
16247	        Line[Point - 1] = Line[Point];
16248	        TTYshow(Line[Point - 1]);
16249	        Line[Point++] = c;
.Ep 144 src/lib/editline/editline.c
16250	        TTYshow(c);
16251	    }
16252	    return CSstay;
16253	}
	
16255	STATIC STATUS
16256	quote()
16257	{
16258	    unsigned int        c;
16259	
16260	    return (c = TTYget()) == EOF ? CSeof : insert_char((int)c);
16261	}
	
16263	STATIC STATUS
16264	wipe()
16265	{
16266	    int         i;
16267	
16268	    if (Mark > End)
16269	        return ring_bell();
16270	
16271	    if (Point > Mark) {
16272	        i = Point;
16273	        Point = Mark;
16274	        Mark = i;
16275	        reposition();
16276	    }
16277	
16278	    return delete_string(Mark - Point);
16279	}
	
16281	STATIC STATUS
16282	mk_set()
16283	{
16284	    Mark = Point;
16285	    return CSstay;
16286	}
	
16288	STATIC STATUS
16289	exchange()
16290	{
16291	    unsigned int        c;
16292	
16293	    if ((c = TTYget()) != CTL('X'))
16294	        return c == EOF ? CSeof : ring_bell();
16295	
16296	    if ((c = Mark) <= End) {
16297	        Mark = Point;
16298	        Point = c;
16299	        return CSmove;
16300	    }
16301	    return CSstay;
16302	}
	
16304	STATIC STATUS
16305	yank()
16306	{
16307	    if (Yanked && *Yanked)
16308	        return insert_string(Yanked);
16309	    return CSstay;
.Op 145 src/lib/editline/editline.c
16310	}
	
16312	STATIC STATUS
16313	copy_region()
16314	{
16315	    if (Mark > End)
16316	        return ring_bell();
16317	
16318	    if (Point > Mark)
16319	        save_yank(Mark, Point - Mark);
16320	    else
16321	        save_yank(Point, Mark - Point);
16322	
16323	    return CSstay;
16324	}
	
16326	STATIC STATUS
16327	move_to_char()
16328	{
16329	    unsigned int        c;
16330	    int                 i;
16331	    CHAR                *p;
16332	
16333	    if ((c = TTYget()) == EOF)
16334	        return CSeof;
16335	    for (i = Point + 1, p = &Line[i]; i < End; i++, p++)
16336	        if (*p == c) {
16337	            Point = i;
16338	            return CSmove;
16339	        }
16340	    return CSstay;
16341	}
	
16343	STATIC STATUS
16344	fd_word()
16345	{
16346	    return do_forward(CSmove);
16347	}
	
16349	STATIC STATUS
16350	fd_kill_word()
16351	{
16352	    int         i;
16353	
16354	    (void)do_forward(CSstay);
16355	    if (OldPoint != Point) {
16356	        i = Point - OldPoint;
16357	        Point = OldPoint;
16358	        return delete_string(i);
16359	    }
16360	    return CSstay;
16361	}
	
16363	STATIC STATUS
16364	bk_word()
16365	{
16366	    int         i;
16367	    CHAR        *p;
16368	
16369	    i = 0;
.Ep 146 src/lib/editline/editline.c
16370	    do {
16371	        for (p = &Line[Point]; p > Line && !isalnum(p[-1]); p--)
16372	            left(CSmove);
16373	
16374	        for (; p > Line && p[-1] != ' ' && isalnum(p[-1]); p--)
16375	            left(CSmove);
16376	
16377	        if (Point == 0)
16378	            break;
16379	    } while (++i < Repeat);
16380	
16381	    return CSstay;
16382	}
	
16384	STATIC STATUS
16385	bk_kill_word()
16386	{
16387	    (void)bk_word();
16388	    if (OldPoint != Point)
16389	        return delete_string(OldPoint - Point);
16390	    return CSstay;
16391	}
	
16393	STATIC int
16394	argify(line, avp)
16395	    CHAR        *line;
16396	    CHAR        ***avp;
16397	{
16398	    CHAR        *c;
16399	    CHAR        **p;
16400	    CHAR        **new;
16401	    int         ac;
16402	    int         i;
16403	
16404	    i = MEM_INC;
16405	    if ((*avp = p = NEW(CHAR*, i))== NULL)
16406	         return 0;
16407	
16408	    for (c = line; isspace(*c); c++)
16409	        continue;
16410	    if (*c == '\n' || *c == '\0')
16411	        return 0;
16412	
16413	    for (ac = 0, p[ac++] = c; *c && *c != '\n'; ) {
16414	        if (isspace(*c)) {
16415	            *c++ = '\0';
16416	            if (*c && *c != '\n') {
16417	                if (ac + 1 == i) {
16418	                    new = NEW(CHAR*, i + MEM_INC);
16419	                    if (new == NULL) {
16420	                        p[ac] = NULL;
16421	                        return ac;
16422	                    }
16423	                    COPYFROMTO(new, p, i * sizeof (char **));
16424	                    i += MEM_INC;
16425	                    DISPOSE(p);
16426	                    *avp = p = new;
16427	                }
16428	                p[ac++] = c;
16429	            }
.Op 147 src/lib/editline/editline.c
16430	        }
16431	        else
16432	            c++;
16433	    }
16434	    *c = '\0';
16435	    p[ac] = NULL;
16436	    return ac;
16437	}
	
16439	STATIC STATUS
16440	last_argument()
16441	{
16442	    CHAR        **av;
16443	    CHAR        *p;
16444	    STATUS      s;
16445	    int         ac;
16446	
16447	    if (H.Size == 1 || (p = H.Lines[H.Size - 2]) == NULL)
16448	        return ring_bell();
16449	
16450	    if ((p = (CHAR *)strdup((char *)p)) == NULL)
16451	        return CSstay;
16452	    ac = argify(p, &av);
16453	
16454	    if (Repeat != NO_ARG)
16455	        s = Repeat < ac ? insert_string(av[Repeat]) : ring_bell();
16456	    else
16457	        s = ac ? insert_string(av[ac - 1]) : CSstay;
16458	
16459	    if (ac)
16460	        DISPOSE(av);
16461	    DISPOSE(p);
16462	    return s;
16463	}
	
16465	STATIC KEYMAP   Map[33] = {
16466	    {   CTL('@'),       mk_set          },
16467	    {   CTL('A'),       beg_line        },
16468	    {   CTL('B'),       bk_char         },
16469	    {   CTL('D'),       del_char        },
16470	    {   CTL('E'),       end_line        },
16471	    {   CTL('F'),       fd_char         },
16472	    {   CTL('G'),       ring_bell       },
16473	    {   CTL('H'),       bk_del_char     },
16474	    {   CTL('I'),       c_complete      },
16475	    {   CTL('J'),       accept_line     },
16476	    {   CTL('K'),       kill_line       },
16477	    {   CTL('L'),       redisplay       },
16478	    {   CTL('M'),       accept_line     },
16479	    {   CTL('N'),       h_next          },
16480	    {   CTL('O'),       ring_bell       },
16481	    {   CTL('P'),       h_prev          },
16482	    {   CTL('Q'),       ring_bell       },
16483	    {   CTL('R'),       h_search        },
16484	    {   CTL('S'),       ring_bell       },
16485	    {   CTL('T'),       transpose       },
16486	    {   CTL('U'),       ring_bell       },
16487	    {   CTL('V'),       quote           },
16488	    {   CTL('W'),       bk_kill_word    },
16489	    {   CTL('X'),       exchange        },
.Ep 148 src/lib/editline/editline.c
16490	    {   CTL('Y'),       yank            },
16491	    {   CTL('Z'),       end_line        },
16492	    {   CTL('['),       meta            },
16493	    {   CTL(']'),       move_to_char    },
16494	    {   CTL('^'),       ring_bell       },
16495	    {   CTL('_'),       ring_bell       },
16496	    {   0,              NULL            }
16497	};
16498	
16499	STATIC KEYMAP   MetaMap[17]= {
16500	    {   CTL('H'),       wipe            },
16501	    {   DEL,            wipe            },
16502	    {   ' ',            mk_set          },
16503	    {   '.',            last_argument   },
16504	    {   '<',            h_first         },
16505	    {   '>',            h_last          },
16506	    {   '?',            c_possible      },
16507	    {   'b',            bk_word         },
16508	    {   'd',            fd_kill_word    },
16509	    {   'f',            fd_word         },
16510	    {   'l',            case_down_word  },
16511	    {   'm',            toggle_meta_mode },
16512	    {   'u',            case_up_word    },
16513	    {   'y',            yank            },
16514	    {   'w',            copy_region     },
16515	    {   0,              NULL            }
16516	};




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/editline/sysos9.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

16600	/*  $Revision: 1.1 $
16601	**
16602	**  OS-9 system-dependant routines for editline library.
16603	*/
16604	#include "editline.h"
16605	#include <sgstat.h>
16606	#include <modes.h>
16607	
16608	
16609	void
16610	rl_ttyset(Reset)
16611	    int                 Reset;
16612	{
16613	    static struct sgbuf old;
16614	    struct sgbuf        new;
16615	
16616	
16617	    if (Reset == 0) {
16618	        _gs_opt(0, &old);
16619	        _gs_opt(0, &new);
16620	        new.sg_backsp = 0;      new.sg_delete = 0;      new.sg_echo = 0;
16621	        new.sg_alf = 0;         new.sg_nulls = 0;       new.sg_pause = 0;
16622	        new.sg_page = 0;        new.sg_bspch = 0;       new.sg_dlnch = 0;
16623	        new.sg_eorch = 0;       new.sg_eofch = 0;       new.sg_rlnch = 0;
16624	        new.sg_dulnch = 0;      new.sg_psch = 0;        new.sg_kbich = 0;
.Op 149 src/lib/editline/sysos9.c
16625	        new.sg_kbach = 0;       new.sg_bsech = 0;       new.sg_bellch = 0;
16626	        new.sg_xon = 0;         new.sg_xoff = 0;        new.sg_tabcr = 0;
16627	        new.sg_tabsiz = 0;
16628	        _ss_opt(0, &new);
16629	        rl_erase = old.sg_bspch;
16630	        rl_kill = old.sg_dlnch;
16631	        rl_eof = old.sg_eofch;
16632	        rl_intr = old.sg_kbich;
16633	        rl_quit = -1;
16634	    }
16635	    else
16636	        _ss_opt(0, &old);
16637	}
	
16639	void
16640	rl_add_slash(path, p)
16641	    char        *path;
16642	    char        *p;
16643	{
16644	    (void)strcat(p, access(path, S_IREAD | S_IFDIR) ? " " : "/");
16645	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/editline/sysunix.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

16700	/*  $Revision: 1.2 $
16701	**
16702	**  Unix system-dependant routines for editline library.
16703	*/
16704	#include "editline.h"
16705	
16706	#if     defined(HAVE_TCGETATTR)
16707	#include <termios.h>
16708	
16709	void
16710	rl_ttyset(Reset)
16711	    int                         Reset;
16712	{
16713	    static struct termios       old;
16714	    struct termios              new;
16715	
16716	    if (Reset == 0) {
16717	        (void)tcgetattr(0, &old);
16718	        rl_erase = old.c_cc[VERASE];
16719	        rl_kill = old.c_cc[VKILL];
16720	        rl_eof = old.c_cc[VEOF];
16721	        rl_intr = old.c_cc[VINTR];
16722	        rl_quit = old.c_cc[VQUIT];
16723	
16724	        new = old;
16725	        new.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
16726	        new.c_cc[VMIN] = 1;
16727	        new.c_cc[VTIME] = 0;
16728	        (void)tcsetattr(0, TCSADRAIN, &new);
16729	    }
.Ep 150 src/lib/editline/sysunix.c
16730	    else
16731	        (void)tcsetattr(0, TCSADRAIN, &old);
16732	}
	
16734	#else
16735	#if     defined(HAVE_TERMIO)
16736	#include <termio.h>
16737	
16738	void
16739	rl_ttyset(Reset)
16740	    int                         Reset;
16741	{
16742	    static struct termio        old;
16743	    struct termio               new;
16744	
16745	    if (Reset == 0) {
16746	        (void)ioctl(0, TCGETA, &old);
16747	        rl_erase = old.c_cc[VERASE];
16748	        rl_kill = old.c_cc[VKILL];
16749	        rl_eof = old.c_cc[VEOF];
16750	        rl_intr = old.c_cc[VINTR];
16751	        rl_quit = old.c_cc[VQUIT];
16752	
16753	        new = old;
16754	        new.c_cc[VINTR] = -1;
16755	        new.c_cc[VQUIT] = -1;
16756	        new.c_lflag &= ~(ECHO | ICANON);
16757	        new.c_cc[VMIN] = 1;
16758	        new.c_cc[VTIME] = 0;
16759	        (void)ioctl(0, TCSETAW, &new);
16760	    }
16761	    else
16762	        (void)ioctl(0, TCSETAW, &old);
16763	}
	
16765	#else
16766	#include <sgtty.h>
16767	
16768	void
16769	rl_ttyset(Reset)
16770	    int                         Reset;
16771	{
16772	    static struct sgttyb        old_sgttyb;
16773	    static struct tchars        old_tchars;
16774	    struct sgttyb               new_sgttyb;
16775	    struct tchars               new_tchars;
16776	
16777	    if (Reset == 0) {
16778	        (void)ioctl(0, TIOCGETP, &old_sgttyb);
16779	        rl_erase = old_sgttyb.sg_erase;
16780	        rl_kill = old_sgttyb.sg_kill;
16781	
16782	        (void)ioctl(0, TIOCGETC, &old_tchars);
16783	        rl_eof = old_tchars.t_eofc;
16784	        rl_intr = old_tchars.t_intrc;
16785	        rl_quit = old_tchars.t_quitc;
16786	
16787	        new_sgttyb = old_sgttyb;
16788	        new_sgttyb.sg_flags &= ~ECHO;
16789	        new_sgttyb.sg_flags |= RAW;
.Op 151 src/lib/editline/sysunix.c
16790	        (void)ioctl(0, TIOCSETP, &new_sgttyb);
16791	
16792	        new_tchars = old_tchars;
16793	        new_tchars.t_intrc = -1;
16794	        new_tchars.t_quitc = -1;
16795	        (void)ioctl(0, TIOCSETC, &new_tchars);
16796	    }
16797	    else {
16798	        (void)ioctl(0, TIOCSETP, &old_sgttyb);
16799	        (void)ioctl(0, TIOCSETC, &old_tchars);
16800	    }
16801	}
16802	#endif  /* defined(HAVE_TERMIO) */
16803	#endif  /* defined(HAVE_TCGETATTR) */
16804	
16805	void
16806	rl_add_slash(path, p)
16807	    char        *path;
16808	    char        *p;
16809	{
16810	    struct stat Sb;
16811	
16812	    if (stat(path, &Sb) >= 0)
16813	        (void)strcat(p, S_ISDIR(Sb.st_mode) ? "/" : " ");
16814	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/editline/testit.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

16900	/*  $Revision: 1.3 $
16901	**
16902	**  A "micro-shell" to test editline library.
16903	**  If given any arguments, commands aren't executed.
16904	*/
16905	#include <stdio.h>
16906	#if     defined(HAVE_STDLIB)
16907	#include <stdlib.h>
16908	#endif  /* defined(HAVE_STDLIB) */
16909	
16910	extern char     *readline();
16911	extern void     add_history();
16912	
16913	#if     !defined(HAVE_STDLIB)
16914	extern int      chdir();
16915	extern int      free();
16916	extern int      strncmp();
16917	extern int      system();
16918	extern void     exit();
16919	extern char     *getenv();
16920	#endif  /* !defined(HAVE_STDLIB) */
16921	
16922	
16923	#if     defined(NEED_PERROR)
16924	void
.Ep 152 src/lib/editline/testit.c
16925	perror(s)
16926	    char        *s;
16927	{
16928	    extern int  errno;
16929	
16930	    (voidf)printf(stderr, "%s: error %d\n", s, errno);
16931	}
16932	#endif  /* defined(NEED_PERROR) */
16933	
16934	
16935	/* ARGSUSED1 */
16936	int
16937	main(ac, av)
16938	    int         ac;
16939	    char        *av[];
16940	{
16941	    char        *prompt;
16942	    char        *p;
16943	    int         doit;
16944	
16945	    doit = ac == 1;
16946	    if ((prompt = getenv("TESTPROMPT")) == NULL)
16947	        prompt = "testit>  ";
16948	
16949	    while ((p = readline(prompt)) != NULL) {
16950	        (void)printf("\t\t\t|%s|\n", p);
16951	        if (doit)
16952	            if (strncmp(p, "cd ", 3) == 0) {
16953	                if (chdir(&p[3]) < 0)
16954	                    perror(&p[3]);
16955	            }
16956	            else if (system(p) != 0)
16957	                perror(p);
16958	        add_history(p);
16959	        free(p);
16960	    }
16961	    exit(0);
16962	    /* NOTREACHED */
16963	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/end/edata.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

17000	#
17001	.sect .text
17002	.sect .rom
17003	.sect .data
17004	.sect .bss
17005	.define _edata
17006	.sect .data
17007	        .align _EM_WSIZE
17008	_edata:
.Op 153 src/lib/end/em_end.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/end/em_end.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

17100	#
17101	.sect .text
17102	.sect .rom
17103	.sect .data
17104	.sect .bss
17105	.define endtext,enddata,endbss,__end
17106	.sect .text
17107	        .align _EM_WSIZE
17108	.sect .rom
17109	        .align _EM_WSIZE
17110	.sect .data
17111	        .align _EM_WSIZE
17112	.sect .bss
17113	        .align _EM_WSIZE
17114	.sect .end ! only for declaration of _end, __end and endbss.
17115	
17116	        .sect .text
17117	endtext:
17118	        .sect .rom
17119	endrom:
17120	        .sect .data
17121	enddata:
17122	        .sect .end
17123	__end:
17124	endbss:






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/end/end.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

17200	.sect .text
17201	.sect .rom
17202	.sect .data
17203	.sect .bss
17204	.define _end
17205	.sect .end ! only for declaration of _end, __end and endbss.
17206	_end:
.Ep 154 src/lib/end/etext.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/end/etext.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

17300	#
17301	.sect .text
17302	.sect .rom
17303	.sect .data
17304	.sect .bss
17305	.define _etext
17306	.sect .text
17307	        .align _EM_WSIZE
17308	        .sect .text
17309	_etext:






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/FP_bias.h	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

17400	/*
17401	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
17402	  See the copyright notice in the ACK home directory, in the file "Copyright".
17403	*/
17404	
17405	/* $Header: FP_bias.h,v 1.4 89/07/25 14:16:55 ceriel Exp $ */
17406	
17407	/*
17408	        include file for floating point package
17409	*/
17410	
17411	                /*      FLOAT FORMAT EXPONENT BIAS      */
17412	
17413	#define SGL_BIAS         127    /* excess  128 notation used    */
17414	#define DBL_BIAS        1023    /* excess 1024 notation used    */
17415	#define EXT_BIAS           0    /* 2s-complement notation used  */
17416	                                /* this is possible because the */
17417	                                /* sign is in a seperate word   */
17418	                
17419	                /*      VARIOUS MAX AND MIN VALUES      */
17420	                /*      1) FOR THE DIFFERENT FORMATS    */
17421	
17422	#define SGL_MAX            254  /*      standard definition     */
17423	#define SGL_MIN              1  /*      standard definition     */
17424	#define DBL_MAX           2046  /*      standard definition     */
17425	#define DBL_MIN              1  /*      standard definition     */
17426	#define EXT_MAX          16383  /*      standard minimum        */
17427	#define EXT_MIN         -16382  /*      standard minimum        */
.Op 155 src/lib/float/FP_shift.h

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/FP_shift.h	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

17500	/*
17501	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
17502	  See the copyright notice in the ACK home directory, in the file "Copyright".
17503	*/
17504	
17505	/* $Header: FP_shift.h,v 1.3 89/07/25 14:17:04 ceriel Exp $ */
17506	
17507	/*
17508	        include file for floating point package
17509	*/
17510	
17511	# define        CARRYBIT        0x80000000L
17512	# define        NORMBIT         0x80000000L
17513	# define        EXP_STORE       16
17514	
17515	
17516	                                /* parameters for Single Precision */
17517	#define SGL_EXPSHIFT    7
17518	#define SGL_M1LEFT      8
17519	#define SGL_ZERO        0xffffff80L
17520	#define SGL_EXACT       0xff
17521	#define SGL_RUNPACK     SGL_M1LEFT
17522	
17523	#define SGL_ROUNDUP     0x80
17524	#define SGL_CARRYOUT    0x01000000L
17525	#define SGL_MASK        0x007fffffL
17526	
17527	                                /* parameters for Double Precision */
17528	                                /* used in extend.c */
17529	
17530	#define DBL_EXPSHIFT    4
17531	
17532	#define DBL_M1LEFT      11
17533	
17534	#define DBL_RPACK       (32-DBL_M1LEFT)
17535	#define DBL_LPACK       DBL_M1LEFT
17536	
17537	                                /* used in compact.c */
17538	
17539	#define DBL_ZERO        0xfffffd00L
17540	
17541	#define DBL_EXACT       0x7ff
17542	
17543	#define DBL_RUNPACK     DBL_M1LEFT
17544	#define DBL_LUNPACK     (32-DBL_RUNPACK)
17545	
17546	#define DBL_ROUNDUP     0x400
17547	#define DBL_CARRYOUT    0x00200000L
17548	#define DBL_MASK        0x000fffffL
.Ep 156 src/lib/float/FP_trap.h

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/FP_trap.h	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

17600	/*
17601	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
17602	  See the copyright notice in the ACK home directory, in the file "Copyright".
17603	*/
17604	
17605	/* $Header: FP_trap.h,v 1.2 88/04/07 11:33:06 ceriel Exp $ */
17606	
17607	/*
17608	        include file for floating point package
17609	*/
17610	
17611	                        /*      EM TRAPS        */
17612	
17613	#define EIOVFL  3       /* Integer  Overflow            */
17614	#define EFOVFL  4       /* Floating Overflow            */
17615	#define EFUNFL  5       /* Floating Underflow           */
17616	#define EIDIVZ  6       /* Integer  Divide by 0         */
17617	#define EFDIVZ  7       /* Floating Divide by 0.0       */
17618	#define EIUND   8       /* Integer  Undefined Number    */
17619	#define EFUND   9       /* Floating Undefined Number    */
17620	#define ECONV   10      /* Conversion Error             */
17621	# define trap(x) _fptrp(x)




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/FP_types.h	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

17700	/*
17701	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
17702	  See the copyright notice in the ACK home directory, in the file "Copyright".
17703	*/
17704	
17705	/* $Header: FP_types.h,v 1.4 93/01/05 12:03:05 ceriel Exp $ */
17706	
17707	/********************************************************/
17708	/*
17709	        Type definitions for C Floating Point Package
17710	        include file for floating point package
17711	*/
17712	/********************************************************/
17713	/*
17714	        THESE STRUCTURES ARE USED TO ADDRESS THE INDIVIDUAL
17715	        PARTS OF THE FLOATING POINT NUMBER REPRESENTATIONS.
17716	
17717	        THREE STRUCTURES ARE DEFINED:
17718	                SINGLE: single precision floating format
17719	                DOUBLE: double precision floating format
17720	                EXTEND: double precision extended format
17721	*/
17722	/********************************************************/
17723	
17724	#ifndef __FPTYPES
.Op 157 src/lib/float/FP_types.h
17725	#define __FPTYPES
17726	
17727	typedef struct  {
17728	        unsigned long   h_32;   /* higher 32 bits of 64 */
17729	        unsigned long   l_32;   /* lower  32 bits of 64 */
17730	}       B64;
17731	
17732	typedef unsigned long   SINGLE;
17733	
17734	typedef struct  {
17735	        unsigned long   d[2];
17736	}       DOUBLE;
17737	
17738	typedef struct  {       /* expanded float format        */
17739	        short   sign;
17740	        short   exp;
17741	        B64     mantissa;
17742	#define m1 mantissa.h_32
17743	#define m2 mantissa.l_32
17744	} EXTEND;
17745	
17746	struct  fef4_returns {
17747	        int     e;
17748	        SINGLE  f;
17749	};
17750	
17751	struct  fef8_returns {
17752	        int     e;
17753	        DOUBLE  f;
17754	};
17755	
17756	struct fif4_returns {
17757	        SINGLE ipart;
17758	        SINGLE fpart;
17759	};
17760	
17761	struct fif8_returns {
17762	        DOUBLE ipart;
17763	        DOUBLE fpart;
17764	};
17765	
17766	#if __STDC__
17767	#define _PROTOTYPE(function, params)    function params
17768	#else
17769	#define _PROTOTYPE(function, params)    function()
17770	#endif
17771	_PROTOTYPE( void add_ext, (EXTEND *e1, EXTEND *e2));
17772	_PROTOTYPE( void mul_ext, (EXTEND *e1, EXTEND *e2));
17773	_PROTOTYPE( void div_ext, (EXTEND *e1, EXTEND *e2));
17774	_PROTOTYPE( void sub_ext, (EXTEND *e1, EXTEND *e2));
17775	_PROTOTYPE( void sft_ext, (EXTEND *e1, EXTEND *e2));
17776	_PROTOTYPE( void nrm_ext, (EXTEND *e1));
17777	_PROTOTYPE( void zrf_ext, (EXTEND *e1));
17778	_PROTOTYPE( void extend, (unsigned long *from, EXTEND *to, int size));
17779	_PROTOTYPE( void compact, (EXTEND *from, unsigned long *to, int size));
17780	_PROTOTYPE( void _fptrp, (int));
17781	_PROTOTYPE( void adf4, (SINGLE s2, SINGLE s1));
17782	_PROTOTYPE( void adf8, (DOUBLE s2, DOUBLE s1));
17783	_PROTOTYPE( void sbf4, (SINGLE s2, SINGLE s1));
17784	_PROTOTYPE( void sbf8, (DOUBLE s2, DOUBLE s1));
.Ep 158 src/lib/float/FP_types.h
17785	_PROTOTYPE( void dvf4, (SINGLE s2, SINGLE s1));
17786	_PROTOTYPE( void dvf8, (DOUBLE s2, DOUBLE s1));
17787	_PROTOTYPE( void mlf4, (SINGLE s2, SINGLE s1));
17788	_PROTOTYPE( void mlf8, (DOUBLE s2, DOUBLE s1));
17789	_PROTOTYPE( void ngf4, (SINGLE f));
17790	_PROTOTYPE( void ngf8, (DOUBLE f));
17791	_PROTOTYPE( void zrf4, (SINGLE *l));
17792	_PROTOTYPE( void zrf8, (DOUBLE *z));
17793	_PROTOTYPE( void cff4, (DOUBLE src));
17794	_PROTOTYPE( void cff8, (SINGLE src));
17795	_PROTOTYPE( void cif4, (int ss, long src));
17796	_PROTOTYPE( void cif8, (int ss, long src));
17797	_PROTOTYPE( void cuf4, (int ss, long src));
17798	_PROTOTYPE( void cuf8, (int ss, long src));
17799	_PROTOTYPE( long cfu, (int ds, int ss, DOUBLE src));
17800	_PROTOTYPE( long cfi, (int ds, int ss, DOUBLE src));
17801	_PROTOTYPE( int cmf4, (SINGLE s2, SINGLE s1));
17802	_PROTOTYPE( int cmf8, (DOUBLE d1, DOUBLE d2));
17803	_PROTOTYPE( void fef4, (struct fef4_returns *r, SINGLE s1));
17804	_PROTOTYPE( void fef8, (struct fef8_returns *r, DOUBLE s1));
17805	_PROTOTYPE( void fif4, (struct fif4_returns *p, SINGLE x, SINGLE y));
17806	_PROTOTYPE( void fif8, (struct fif8_returns *p, DOUBLE x, DOUBLE y));
17807	
17808	_PROTOTYPE( void b64_sft, (B64 *, int));
17809	_PROTOTYPE( void b64_lsft, (B64 *));
17810	_PROTOTYPE( void b64_rsft, (B64 *));
17811	_PROTOTYPE( int b64_add, (B64 *, B64 *));
17812	#endif



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/adder.h	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

17900	/*
17901	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
17902	  See the copyright notice in the ACK home directory, in the file "Copyright".
17903	*/
17904	
17905	/* $Header: adder.h,v 1.2 92/02/20 18:20:25 philip Exp $ */
17906	
17907	/*
17908	 *      include file for 32 & 64 bit addition
17909	 */
17910	
17911	typedef struct  B64 {
17912	        unsigned long   h_32;   /* higher 32 bits of 64 */
17913	        unsigned long   l_32;   /* lower  32 bits of 64 */
17914	}       B64;
.Op 159 src/lib/float/byte_order.h

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/byte_order.h	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

18000	#define CHAR_UNSIGNED   0
18001	#define MSB_AT_LOW_ADDRESS      0
18002	#define MSW_AT_LOW_ADDRESS      0
18003	#define FL_MSB_AT_LOW_ADDRESS   0
18004	#define FL_MSW_AT_LOW_ADDRESS   0
18005	#define FL_MSL_AT_LOW_ADDRESS   0





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/get_put.h	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

18100	/*
18101	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
18102	  See the copyright notice in the ACK home directory, in the file "Copyright".
18103	*/
18104	
18105	/* $Header: get_put.h,v 1.3 89/10/25 17:15:01 ceriel Exp $ */
18106	
18107	#include <byte_order.h>
18108	
18109	#if CHAR_UNSIGNED
18110	#define Xchar(ch)       (ch)
18111	#else
18112	#define Xchar(ch)       ((ch) & 0377)
18113	#endif
18114	
18115	#define BYTES_REVERSED (MSB_AT_LOW_ADDRESS != FL_MSB_AT_LOW_ADDRESS)
18116	#define WORDS_REVERSED (MSW_AT_LOW_ADDRESS != FL_MSW_AT_LOW_ADDRESS)
18117	#define LONGS_REVERSED (FL_MSL_AT_LOW_ADDRESS)
18118	
18119	#if BYTES_REVERSED
18120	#define uget2(c)        (Xchar((c)[1]) | ((unsigned) Xchar((c)[0]) << 8))
18121	#define Xput2(i, c)     (((c)[1] = (i)), ((c)[0] = (i) >> 8))
18122	#define put2(i, c)      { register int j = (i); Xput2(j, c); }
18123	#else
18124	#define uget2(c)        (* ((unsigned short *) (c)))
18125	#define Xput2(i, c)     (* ((short *) (c)) = (i))
18126	#define put2(i, c)      Xput2(i, c)
18127	#endif
18128	
18129	#define get2(c)         ((short) uget2(c))
18130	
18131	#if WORDS_REVERSED || BYTES_REVERSED
18132	#define get4(c)         (uget2((c)+2) | ((long) uget2(c) << 16))
18133	#define put4(l, c)      { register long x=(l); \
18134	                          Xput2((int)x,(c)+2); \
18135	                          Xput2((int)(x>>16),(c)); \
18136	                        }
18137	#else
18138	#define get4(c)         (* ((long *) (c)))
18139	#define put4(l, c)      (* ((long *) (c)) = (l))
.Ep 160 src/lib/float/get_put.h
18140	#endif





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/add_ext.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

18200	/*
18201	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
18202	  See the copyright notice in the ACK home directory, in the file "Copyright".
18203	*/
18204	
18205	/* $Header: add_ext.c,v 1.7 93/01/05 12:03:11 ceriel Exp $ */
18206	
18207	/*
18208	        ADD TWO EXTENDED FORMAT NUMBERS
18209	*/
18210	
18211	#include "FP_types.h"
18212	
18213	void
18214	add_ext(e1,e2)
18215	register EXTEND *e1,*e2;
18216	{
18217	        if ((e2->m1 | e2->m2) == 0L) {
18218	                return;
18219	        }
18220	        if ((e1->m1 | e1->m2) == 0L) {
18221	                *e1 = *e2;
18222	                return;
18223	        }
18224	        sft_ext(e1, e2);        /* adjust mantissas to equal powers */
18225	        if (e1->sign != e2->sign) {
18226	                /* e1 + e2 = e1 - (-e2) */
18227	                if (e2->m1 > e1->m1 ||
18228	                    (e2->m1 == e1->m1 && e2->m2 > e1->m2)) {
18229	                        /*      abs(e2) > abs(e1) */
18230	                        EXTEND x;
18231	
18232	                        x = *e1;
18233	                        *e1 = *e2;
18234	                        if (x.m2 > e1->m2) {
18235	                                e1->m1 -= 1;    /* carry in */
18236	                        }
18237	                        e1->m1 -= x.m1;
18238	                        e1->m2 -= x.m2;
18239	                }
18240	                else {
18241	                        if (e2->m2 > e1->m2)
18242	                                e1->m1 -= 1;    /* carry in */
18243	                        e1->m1 -= e2->m1;
18244	                        e1->m2 -= e2->m2;
18245	                }
18246	        }
18247	        else {
18248	                if (b64_add(&e1->mantissa,&e2->mantissa)) {     /* addition carry */
18249	                        b64_rsft(&e1->mantissa);        /* shift mantissa one bit RIGHT */
.Op 161 src/lib/float/add_ext.c
18250	                        e1->m1 |= 0x80000000L;  /* set max bit  */
18251	                        e1->exp++;              /* increase the exponent */
18252	                }
18253	        }
18254	        nrm_ext(e1);
18255	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/adder.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

18300	/*
18301	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
18302	  See the copyright notice in the ACK home directory, in the file "Copyright".
18303	*/
18304	
18305	/* $Header: adder.c,v 1.6 93/01/05 12:03:17 ceriel Exp $ */
18306	
18307	/*
18308	 *      these are the routines the routines to do 32 and  64-bit addition
18309	 */
18310	
18311	# ifdef EXT_DEBUG
18312	# include <stdio.h>
18313	# endif
18314	
18315	# include "FP_types.h"
18316	# define        UNKNOWN -1
18317	# define        TRUE     1
18318	# define        FALSE    0
18319	# define        MAXBIT  0x80000000L
18320	
18321	        /*
18322	         *      add 64 bits
18323	         */
18324	int
18325	b64_add(e1,e2)
18326	                /*
18327	                 * pointers to 64 bit 'registers'
18328	                 */
18329	register        B64     *e1,*e2;
18330	{
18331	                register        int     overflow;
18332	                                int     carry;
18333	
18334	                        /* add higher pair of 32 bits */
18335	        overflow = ((unsigned long) 0xFFFFFFFF - e1->h_32 < e2->h_32);
18336	        e1->h_32 += e2->h_32;
18337	
18338	                        /* add lower pair of 32 bits */
18339	        carry = ((unsigned long) 0xFFFFFFFF - e1->l_32 < e2->l_32);
18340	        e1->l_32 += e2->l_32;
18341	# ifdef EXT_DEBUG
18342	        printf("\t\t\t\t\tb64_add: overflow (%d); internal carry(%d)\n",
18343	                                        overflow,carry);
18344	        fflush(stdout);
.Ep 162 src/lib/float/adder.c
18345	# endif
18346	        if ((carry) && (++e1->h_32 == 0))
18347	                return(TRUE);           /* had a 64 bit overflow */
18348	        return(overflow);               /* return status from higher add */
18349	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/adf4.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

18400	/*
18401	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
18402	  See the copyright notice in the ACK home directory, in the file "Copyright".
18403	*/
18404	
18405	/* $Header: adf4.c,v 1.7 93/01/05 12:03:23 ceriel Exp $ */
18406	
18407	/*
18408	        ADD TWO FLOATS - SINGLE (ADF 4)
18409	*/
18410	
18411	#include        "FP_types.h"
18412	
18413	void
18414	adf4(s2,s1)
18415	SINGLE  s1,s2;
18416	{
18417	        EXTEND  e1,e2;
18418	        int     swap = 0;
18419	
18420	        if (s1 == (SINGLE) 0) {
18421	                s1 = s2;
18422	                return;
18423	        }
18424	        if (s2 == (SINGLE) 0) {
18425	                return;
18426	        }
18427	        extend(&s1,&e1,sizeof(SINGLE));
18428	        extend(&s2,&e2,sizeof(SINGLE));
18429	        add_ext(&e1,&e2);
18430	        compact(&e1,&s1,sizeof(SINGLE));
18431	}
.Op 163 src/lib/float/adf8.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/adf8.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

18500	/*
18501	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
18502	  See the copyright notice in the ACK home directory, in the file "Copyright".
18503	*/
18504	
18505	/* $Header: adf8.c,v 1.7 93/01/05 12:03:29 ceriel Exp $ */
18506	
18507	/*
18508	        ADD TWO FLOATS - DOUBLE (ADF 8)
18509	*/
18510	
18511	#include        "FP_types.h"
18512	
18513	void
18514	adf8(s2,s1)
18515	DOUBLE  s1,s2;
18516	{
18517	        EXTEND  e1,e2;
18518	
18519	        if (s1.d[0] == 0 && s1.d[1] == 0) {
18520	                s1 = s2;
18521	                return;
18522	        }
18523	        if (s2.d[0] == 0 && s2.d[1] == 0) {
18524	                return;
18525	        }
18526	
18527	        extend(&s1.d[0],&e1,sizeof(DOUBLE));
18528	        extend(&s2.d[0],&e2,sizeof(DOUBLE));
18529	        add_ext(&e1,&e2);
18530	        compact(&e1,&s1.d[0],sizeof(DOUBLE));
18531	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/cff4.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

18600	/*
18601	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
18602	  See the copyright notice in the ACK home directory, in the file "Copyright".
18603	*/
18604	
18605	/* $Header: cff4.c,v 1.5 93/01/05 12:03:36 ceriel Exp $ */
18606	
18607	/*
18608	                CONVERT DOUBLE TO SINGLE (CFF 8 4)
18609	
18610	        This routine works quite simply. A floating point
18611	        of size 08 is converted to extended format.
18612	        This extended variable is converted back to
18613	        a floating point of size 04.
18614	
.Ep 164 src/lib/float/cff4.c
18615	*/
18616	
18617	#include        "FP_types.h"
18618	
18619	void
18620	cff4(src)
18621	DOUBLE  src;    /* the source itself -  THIS TIME it's DOUBLE */
18622	{
18623	        EXTEND  buf;
18624	
18625	        extend(&src.d[0],&buf,sizeof(DOUBLE));  /* no matter what */
18626	        compact(&buf,&(src.d[1]),sizeof(SINGLE));
18627	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/cff8.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

18700	/*
18701	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
18702	  See the copyright notice in the ACK home directory, in the file "Copyright".
18703	*/
18704	
18705	/* $Header: cff8.c,v 1.5 93/01/05 12:03:41 ceriel Exp $ */
18706	
18707	/*
18708	                CONVERT SINGLE TO DOUBLE (CFF 4 8)
18709	
18710	        This routine works quite simply. A floating point
18711	        of size 04 is converted to extended format.
18712	        This extended variable is converted back to
18713	        a floating point of size 08.
18714	
18715	*/
18716	
18717	#include "FP_types.h"
18718	
18719	void
18720	cff8(src)
18721	SINGLE  src;
18722	{
18723	        EXTEND  buf;
18724	
18725	        extend(&src,&buf,sizeof(SINGLE));       /* no matter what */
18726	        compact(&buf, &src,sizeof(DOUBLE));
18727	}
.Op 165 src/lib/float/cfi.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/cfi.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

18800	/*
18801	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
18802	  See the copyright notice in the ACK home directory, in the file "Copyright".
18803	*/
18804	
18805	/* $Header: cfi.c,v 1.5 93/01/05 12:03:48 ceriel Exp $ */
18806	
18807	/*
18808	                CONVERT FLOAT TO SIGNED (CFI m n)
18809	
18810	                N.B. The caller must know what it is getting.
18811	                     A LONG is always returned. If it is an
18812	                     integer the high byte is cleared first.
18813	*/
18814	
18815	#include "FP_trap.h"
18816	#include "FP_types.h"
18817	#include "FP_shift.h"
18818	
18819	long
18820	cfi(ds,ss,src)
18821	int     ds;     /* destination size (2 or 4) */
18822	int     ss;     /* source size      (4 or 8) */
18823	DOUBLE  src;    /* assume worst case */
18824	{
18825	        EXTEND  buf;
18826	        long    new;
18827	        short   max_exp;
18828	
18829	        extend(&src.d[0],&buf,ss);      /* get extended format */
18830	        if (buf.exp < 0) {      /* no conversion needed */
18831	                src.d[ss == 8] = 0L;
18832	                return(0L);
18833	        }
18834	        max_exp = (ds << 3) - 2;        /* signed numbers */
18835	                                /* have more limited max_exp */
18836	        if (buf.exp > max_exp) {
18837	                if (buf.exp == max_exp+1 && buf.sign && buf.m1 == NORMBIT &&
18838	                    buf.m2 == 0L) {
18839	                }
18840	                else {
18841	                        trap(EIOVFL);   /* integer overflow     */
18842	                        buf.exp %= max_exp; /* truncate */
18843	                }
18844	        }
18845	        new = buf.m1 >> (31-buf.exp);
18846	        if (buf.sign)
18847	                new = -new;
18848	done:
18849	        src.d[ss == 8] = new;
18850	        return(new);
18851	}
.Ep 166 src/lib/float/cfu.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/cfu.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

18900	/*
18901	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
18902	  See the copyright notice in the ACK home directory, in the file "Copyright".
18903	*/
18904	
18905	/* $Header: cfu.c,v 1.5 93/01/05 12:03:55 ceriel Exp $ */
18906	
18907	/*
18908	                CONVERT FLOAT TO UNSIGNED (CFU m n)
18909	
18910	                N.B. The caller must know what it is getting.
18911	                     A LONG is always returned. If it is an
18912	                     integer the high byte is cleared first.
18913	*/
18914	
18915	#include "FP_trap.h"
18916	#include "FP_types.h"
18917	
18918	long
18919	cfu(ds,ss,src)
18920	int     ds;     /* destination size (2 or 4) */
18921	int     ss;     /* source size      (4 or 8) */
18922	DOUBLE  src;    /* assume worst case */
18923	{
18924	        EXTEND  buf;
18925	        long    new;
18926	        short   newint, max_exp;
18927	
18928	        extend(&src.d[0],&buf,ss);      /* get extended format  */
18929	        if (buf.exp < 0) {      /* no conversion needed */
18930	                src.d[ss == 8] = 0L;
18931	                return(0L);
18932	        }
18933	        max_exp = (ds << 3) - 1;
18934	        if (buf.exp > max_exp) {
18935	                trap(EIOVFL);   /* integer overflow     */
18936	                buf.exp %= max_exp;
18937	        }
18938	        new = buf.m1 >> (31-buf.exp);
18939	done:
18940	        src.d[ss == 8] = new;
18941	        return(new);
18942	}
.Op 167 src/lib/float/cif4.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/cif4.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

19000	/*
19001	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
19002	  See the copyright notice in the ACK home directory, in the file "Copyright".
19003	*/
19004	
19005	/* $Header: cif4.c,v 1.5 93/01/05 12:04:01 ceriel Exp $ */
19006	
19007	/*
19008	        CONVERT INTEGER TO SINGLE (CIF n 4)
19009	
19010	        THIS ROUTINE WORKS BY FILLING AN EXTENDED
19011	        WITH THE INTEGER VALUE IN EXTENDED FORMAT
19012	        AND USES COMPACT() TO PUT IT INTO THE PROPER
19013	        FLOATING POINT PRECISION.
19014	*/
19015	
19016	#include "FP_types.h"
19017	
19018	void
19019	cif4(ss,src)
19020	int     ss;     /* source size */
19021	long    src;    /* largest possible integer to convert */
19022	{
19023	        EXTEND  buf;
19024	        short   *ipt;
19025	        long    i_src;
19026	        SINGLE  *result;
19027	
19028	        zrf_ext(&buf);
19029	        if (ss == sizeof(long)) {
19030	                buf.exp = 31;
19031	                i_src = src;
19032	                result = (SINGLE *) &src;
19033	        }
19034	        else    {
19035	                ipt = (short *) &src;
19036	                i_src = (long) *ipt;
19037	                buf.exp = 15;
19038	                result = (SINGLE *) &ss;
19039	        }
19040	        if (i_src == 0) {
19041	                *result = (SINGLE) 0L;
19042	                return;
19043	        }
19044	                        /* ESTABLISHED THAT src != 0    */
19045	                        /* adjust exponent field        */
19046	        buf.sign = (i_src < 0) ? 0x8000 : 0;
19047	                        /* clear sign bit of integer    */
19048	                        /* move to mantissa field       */
19049	        buf.m1 = (i_src < 0) ? -i_src : i_src;
19050	                        /* adjust mantissa field        */
19051	        if (ss != sizeof(long))
19052	                buf.m1 <<= 16;
19053	        nrm_ext(&buf);          /* adjust mantissa field        */
19054	        compact(&buf, result,sizeof(SINGLE));   /* put on stack */
.Ep 168 src/lib/float/cif4.c
19055	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/cif8.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

19100	/*
19101	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
19102	  See the copyright notice in the ACK home directory, in the file "Copyright".
19103	*/
19104	
19105	/* $Header: cif8.c,v 1.5 93/01/05 12:04:07 ceriel Exp $ */
19106	
19107	/*
19108	        CONVERT INTEGER TO FLOAT (CIF n 8)
19109	
19110	        THIS ROUTINE WORKS BY FILLING AN EXTENDED
19111	        WITH THE INTEGER VALUE IN EXTENDED FORMAT
19112	        AND USES COMPACT() TO PUT IT INTO THE PROPER
19113	        FLOATING POINT PRECISION.
19114	*/
19115	
19116	#include "FP_types.h"
19117	
19118	void
19119	cif8(ss,src)
19120	int     ss;     /* source size */
19121	long    src;    /* largest possible integer to convert */
19122	{
19123	        EXTEND  buf;
19124	        DOUBLE  *result;        /* for return value */
19125	        short   *ipt;
19126	        long    i_src;
19127	
19128	        result = (DOUBLE *) ((void *) &ss);     /* always */
19129	        zrf_ext(&buf);
19130	        if (ss == sizeof(long)) {
19131	                buf.exp = 31;
19132	                i_src = src;
19133	        }
19134	        else    {
19135	                ipt = (short *) &src;
19136	                i_src = (long) *ipt;
19137	                buf.exp = 15;
19138	        }
19139	        if (i_src == 0) {
19140	                zrf8(result);
19141	                return;
19142	        }
19143	                        /* ESTABLISHED THAT src != 0    */
19144	                        /* adjust exponent field        */
19145	        buf.sign = (i_src < 0) ? 0x8000 : 0;
19146	                        /* clear sign bit of integer    */
19147	                        /* move to mantissa field       */
19148	        buf.m1 = (i_src < 0) ? -i_src : i_src;
19149	                        /* adjust mantissa field        */
.Op 169 src/lib/float/cif8.c
19150	        if (ss != sizeof(long))
19151	                buf.m1 <<= 16;
19152	        nrm_ext(&buf);
19153	        compact(&buf,&result->d[0],8);
19154	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/cmf4.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

19200	/*
19201	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
19202	  See the copyright notice in the ACK home directory, in the file "Copyright".
19203	*/
19204	
19205	/* $Header: cmf4.c,v 1.6 93/01/05 12:04:14 ceriel Exp $ */
19206	
19207	/*
19208	        COMPARE SINGLES (CMF 4)
19209	*/
19210	
19211	#include        "FP_types.h"
19212	#include        "get_put.h"
19213	
19214	int
19215	cmf4(f1,f2)
19216	SINGLE  f1,f2;
19217	{
19218	                /*
19219	                 * return ((f1 < f2) ? 1 : (f1 - f2))
19220	                 */
19221	#define SIGN(x) (((x) < 0) ? -1 : 1)
19222	        int     sign1,sign2;
19223	        long    l1,l2;
19224	
19225	        l1 = get4((char *) &f1);
19226	        l2 = get4((char *) &f2);
19227	
19228	        if (l1 == l2) return 0;
19229	
19230	        sign1 = SIGN(l1);
19231	        sign2 = SIGN(l2);
19232	        if (sign1 != sign2) {
19233	                if ((l1 & 0x7fffffff) == 0 &&
19234	                    (l2 & 0x7fffffff) == 0) return 0;
19235	                return ((sign1 > 0) ? -1 : 1);
19236	        }
19237	
19238	        return (sign1 * ((l1 < l2) ? 1 : -1));
19239	}
.Ep 170 src/lib/float/cmf8.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/cmf8.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

19300	/*
19301	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
19302	  See the copyright notice in the ACK home directory, in the file "Copyright".
19303	*/
19304	
19305	/* $Header: cmf8.c,v 1.9 93/01/05 12:04:22 ceriel Exp $ */
19306	
19307	/*
19308	        COMPARE DOUBLES (CMF 8)
19309	*/
19310	
19311	#include        "FP_types.h"
19312	#include        "get_put.h"
19313	
19314	int
19315	cmf8(d1,d2)
19316	DOUBLE  d1,d2;
19317	{
19318	#define SIGN(x) (((x) < 0) ? -1 : 1)
19319	                /*
19320	                 * return ((d1 < d2) ? 1 : (d1 > d2) ? -1 : 0))
19321	                 */
19322	        long    l1,l2;
19323	        int     sign1,sign2;
19324	        int     rv;
19325	
19326	#if FL_MSL_AT_LOW_ADDRESS
19327	        l1 = get4((char *)&d1);
19328	        l2 = get4((char *)&d2);
19329	#else
19330	        l1 = get4(((char *)&d1+4));
19331	        l2 = get4(((char *)&d2+4));
19332	#endif
19333	        sign1 = SIGN(l1);
19334	        sign2 = SIGN(l2);
19335	        if (sign1 != sign2) {
19336	                l1 &= 0x7fffffff;
19337	                l2 &= 0x7fffffff;
19338	                if (l1 != 0 || l2 != 0) {
19339	                        return ((sign1 > 0) ? -1 : 1);
19340	                }
19341	        }
19342	        if (l1 != l2)   {       /* we can decide here */
19343	                rv = l1 < l2 ? 1 : -1;
19344	        }
19345	        else    {               /* decide in 2nd half */
19346	                unsigned long u1, u2;
19347	#if FL_MSL_AT_LOW_ADDRESS
19348	                u1 = get4(((char *)&d1 + 4));
19349	                u2 = get4(((char *)&d2 + 4));
19350	#else
19351	                u1 = get4((char *)&d1);
19352	                u2 = get4((char *)&d2);
19353	#endif
19354	                if (u1 == u2)
.Op 171 src/lib/float/cmf8.c
19355	                        return(0);
19356	                if (u1 < u2) rv = 1;
19357	                else rv = -1;
19358	        }
19359	        return sign1 * rv;
19360	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/compact.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

19400	/*
19401	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
19402	  See the copyright notice in the ACK home directory, in the file "Copyright".
19403	*/
19404	
19405	/* $Header: compact.c,v 1.13 93/01/05 12:04:28 ceriel Exp $ */
19406	
19407	/*
19408	        COMPACT EXTEND FORMAT INTO FLOAT OF PROPER SIZE
19409	*/
19410	
19411	# include "FP_bias.h"
19412	# include "FP_shift.h"
19413	# include "FP_trap.h"
19414	# include "FP_types.h"
19415	# include "get_put.h"
19416	
19417	void
19418	compact(f,to,size)
19419	EXTEND  *f;
19420	unsigned long   *to;
19421	int     size;
19422	{
19423	        int     error = 0;
19424	
19425	        if (size == sizeof(DOUBLE)) {
19426	        /*
19427	         * COMPACT EXTENDED INTO DOUBLE
19428	         */
19429	                DOUBLE *DBL = (DOUBLE *) (void *) to;
19430	
19431	                if ((f->m1|(f->m2 & DBL_ZERO)) == 0L)   {
19432	                        zrf8(DBL);
19433	                        return;
19434	                }
19435	                f->exp += DBL_BIAS;     /* restore proper bias  */
19436	                if (f->exp > DBL_MAX)   {
19437	dbl_over:                       trap(EFOVFL);
19438	                        f->exp = DBL_MAX+1;
19439	                        f->m1 = 0;
19440	                        f->m2 = 0;
19441	                        if (error++)
19442	                                return;
19443	                }
19444	                else if (f->exp < DBL_MIN)      {
.Ep 172 src/lib/float/compact.c
19445	                        b64_rsft(&(f->mantissa));
19446	                        if (f->exp < 0) {
19447	                                b64_sft(&(f->mantissa), -f->exp);
19448	                                f->exp = 0;
19449	                        }
19450	                        /* underflow ??? */
19451	                }
19452	                        
19453	                /* local CAST conversion                */
19454	
19455	                /* because of special format shift only 10 bits */
19456	                /* bit shift mantissa 10 bits           */
19457	
19458	                /* first align within words, then do store operation */
19459	
19460	                DBL->d[0] = f->m1 >> DBL_RUNPACK;   /* plus 22 == 32 */
19461	                DBL->d[1] = f->m2 >> DBL_RUNPACK;   /* plus 22 == 32 */
19462	                DBL->d[1] |= (f->m1 << DBL_LUNPACK); /* plus 10 == 32 */
19463	
19464	                /* if not exact then round to nearest   */
19465	                /* on a tie, round to even */
19466	
19467	#ifdef EXCEPTION_INEXACT
19468	                if ((f->m2 & DBL_EXACT) != 0) {
19469	                    INEXACT();
19470	#endif
19471	                    if (((f->m2 & DBL_EXACT) > DBL_ROUNDUP)
19472	                        || ((f->m2 & DBL_EXACT) == DBL_ROUNDUP
19473	                            && (f->m2 & (DBL_ROUNDUP << 1)))) {
19474	                        DBL->d[1]++;    /* rounding up  */
19475	                        if (DBL->d[1] == 0L) { /* carry out     */
19476	                            DBL->d[0]++;
19477	
19478	                            if (f->exp == 0 && (DBL->d[0] & ~DBL_MASK)) {
19479	                                        f->exp++;
19480	                                }
19481	                            if (DBL->d[0] & DBL_CARRYOUT) { /* carry out */
19482	                                if (DBL->d[0] & 01)
19483	                                    DBL->d[1] = CARRYBIT;
19484	                                DBL->d[0] >>= 1;
19485	                                f->exp++;
19486	                            }
19487	                        }
19488	                        /*      check for overflow                      */
19489	                        if (f->exp > DBL_MAX)
19490	                                goto dbl_over;
19491	                    }
19492	#ifdef EXCEPTION_INEXACT
19493	                }
19494	#endif
19495	
19496	                /*
19497	                 * STORE EXPONENT AND SIGN:
19498	                 *
19499	                 * 1) clear leading bits (B4-B15)
19500	                 * 2) shift and store exponent
19501	                 */
19502	
19503	                DBL->d[0] &= DBL_MASK;
19504	                DBL->d[0] |= 
.Op 173 src/lib/float/compact.c
19505	                        ((long) (f->exp << DBL_EXPSHIFT) << EXP_STORE);
19506	                if (f->sign)
19507	                        DBL->d[0] |= CARRYBIT;
19508	
19509	                /*
19510	                 * STORE MANTISSA
19511	                 */
19512	
19513	#if FL_MSL_AT_LOW_ADDRESS
19514	                put4(DBL->d[0], (char *) &DBL->d[0]);
19515	                put4(DBL->d[1], (char *) &DBL->d[1]);
19516	#else
19517	                { unsigned long l;
19518	                  put4(DBL->d[1], (char *) &l);
19519	                  put4(DBL->d[0], (char *) &DBL->d[1]);
19520	                  DBL->d[0] = l;
19521	                }
19522	#endif
19523	        }
19524	        else {
19525	                /*
19526	                 * COMPACT EXTENDED INTO FLOAT
19527	                 */
19528	                SINGLE  *SGL;
19529	
19530	                /* local CAST conversion                */
19531	                SGL = (SINGLE *) (void *) to;
19532	                if ((f->m1 & SGL_ZERO) == 0L)   {
19533	                        *SGL = 0L;
19534	                        return;
19535	                }
19536	                f->exp += SGL_BIAS;     /* restore bias */
19537	                if (f->exp > SGL_MAX)   {
19538	sgl_over:                       trap(EFOVFL);
19539	                        f->exp = SGL_MAX+1;
19540	                        f->m1 = 0L;
19541	                        f->m2 = 0L;
19542	                        if (error++)
19543	                                return;
19544	                }
19545	                else if (f->exp < SGL_MIN)      {
19546	                        b64_rsft(&(f->mantissa));
19547	                        if (f->exp < 0) {
19548	                                b64_sft(&(f->mantissa), -f->exp);
19549	                                f->exp = 0;
19550	                        }
19551	                        /* underflow ??? */
19552	                }
19553	
19554	                /* shift mantissa and store     */
19555	                *SGL = (f->m1 >> SGL_RUNPACK);
19556	
19557	                /* check for rounding to nearest        */
19558	                /* on a tie, round to even              */
19559	#ifdef EXCEPTION_INEXACT
19560	                if (f->m2 != 0 ||
19561	                    (f->m1 & SGL_EXACT) != 0L) {
19562	                        INEXACT();
19563	#endif
19564	                        if (((f->m1 & SGL_EXACT) > SGL_ROUNDUP)
.Ep 174 src/lib/float/compact.c
19565	                            || ((f->m1 & SGL_EXACT) == SGL_ROUNDUP
19566	                                && (f->m1 & (SGL_ROUNDUP << 1)))) {
19567	                                (*SGL)++;
19568	                                if (f->exp == 0 && (*SGL & ~SGL_MASK)) {
19569	                                        f->exp++;
19570	                                }
19571	                        /* check normal */
19572	                                if (*SGL & SGL_CARRYOUT)        {
19573	                                        *SGL >>= 1;
19574	                                        f->exp++;
19575	                                }
19576	                                if (f->exp > SGL_MAX)
19577	                                        goto sgl_over;
19578	                        }
19579	#ifdef EXCEPTION_INEXACT
19580	                }
19581	#endif
19582	
19583	                /*
19584	                 * STORE EXPONENT AND SIGN:
19585	                 *
19586	                 * 1) clear leading bit of fraction
19587	                 * 2) shift and store exponent
19588	                 */
19589	
19590	                *SGL &= SGL_MASK; /* B23-B31 are 0 */
19591	                *SGL |= ((long) (f->exp << SGL_EXPSHIFT) << EXP_STORE);
19592	                if (f->sign)
19593	                        *SGL |= CARRYBIT;
19594	
19595	                /*
19596	                 * STORE MANTISSA
19597	                 */
19598	
19599	                put4(*SGL, (char *) &SGL);
19600	        }
19601	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/cuf4.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

19700	/*
19701	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
19702	  See the copyright notice in the ACK home directory, in the file "Copyright".
19703	*/
19704	
19705	/* $Header: cuf4.c,v 1.6 93/01/05 12:04:35 ceriel Exp $ */
19706	
19707	/*
19708	        CONVERT INTEGER TO SINGLE (CUF n 4)
19709	
19710	        THIS ROUTINE WORKS BY FILLING AN EXTENDED
19711	        WITH THE INTEGER VALUE IN EXTENDED FORMAT
19712	        AND USES COMPACT() TO PUT IT INTO THE PROPER
19713	        FLOATING POINT PRECISION.
19714	*/
.Op 175 src/lib/float/cuf4.c
19715	
19716	#include "FP_types.h"
19717	
19718	void
19719	cuf4(ss,src)
19720	int     ss;     /* source size */
19721	long    src;    /* largest possible integer to convert */
19722	{
19723	        EXTEND  buf;
19724	        short   *ipt;
19725	        SINGLE  *result;
19726	        long    i_src;
19727	
19728	        zrf_ext(&buf);
19729	        if (ss == sizeof(long)) {
19730	                buf.exp = 31;
19731	                i_src = src;
19732	                result = (SINGLE *) &src;
19733	        }
19734	        else    {
19735	                ipt = (short *) &src;
19736	                i_src = (long) *ipt;
19737	                buf.exp = 15;
19738	                result = (SINGLE *) ((void *) &ss);
19739	        }
19740	        if (i_src == 0) {
19741	                *result = (SINGLE) 0L;
19742	                return;
19743	        }
19744	                        /* ESTABLISHED THAT src != 0    */
19745	
19746	                        /* adjust exponent field        */
19747	        if (ss != sizeof(long))
19748	                i_src <<= 16;
19749	
19750	                        /* move to mantissa field       */
19751	        buf.m1 = i_src;
19752	
19753	                        /* adjust mantissa field        */
19754	        nrm_ext(&buf);
19755	        compact(&buf,result,4);
19756	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/cuf8.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

19800	/*
19801	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
19802	  See the copyright notice in the ACK home directory, in the file "Copyright".
19803	*/
19804	
19805	/* $Header: cuf8.c,v 1.6 93/01/05 12:04:41 ceriel Exp $ */
19806	
19807	/*
19808	        CONVERT INTEGER TO FLOAT (CUF n 8)
19809	
.Ep 176 src/lib/float/cuf8.c
19810	        THIS ROUTINE WORKS BY FILLING AN EXTENDED
19811	        WITH THE INTEGER VALUE IN EXTENDED FORMAT
19812	        AND USES COMPACT() TO PUT IT INTO THE PROPER
19813	        FLOATING POINT PRECISION.
19814	*/
19815	
19816	#include "FP_types.h"
19817	
19818	void
19819	cuf8(ss,src)
19820	int     ss;     /* source size */
19821	long    src;    /* largest possible integer to convert */
19822	{
19823	        EXTEND  buf;
19824	        short   *ipt;
19825	        long    i_src;
19826	
19827	        zrf_ext(&buf);
19828	        if (ss == sizeof(long)) {
19829	                buf.exp = 31;
19830	                i_src = src;
19831	        }
19832	        else    {
19833	                ipt = (short *) &src;
19834	                i_src = (long) *ipt;
19835	                buf.exp = 15;
19836	        }
19837	        if (i_src == 0) {
19838	                zrf8((DOUBLE *)((void *)&ss));
19839	                return;
19840	        }
19841	                        /* ESTABLISHED THAT src != 0    */
19842	
19843	                        /* adjust exponent field        */
19844	        if (ss != sizeof(long))
19845	                i_src <<= 16;
19846	
19847	                        /* move to mantissa field       */
19848	        buf.m1 = i_src;
19849	
19850	                        /* adjust mantissa field        */
19851	        nrm_ext(&buf);
19852	        compact(&buf,(unsigned long *) (void *)&ss,8);
19853	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/div_ext.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

19900	/*
19901	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
19902	  See the copyright notice in the ACK home directory, in the file "Copyright".
19903	*/
19904	
19905	/* $Header: div_ext.c,v 1.10 93/01/05 12:04:47 ceriel Exp $ */
19906	
19907	/*
19908	        DIVIDE EXTENDED FORMAT
19909	*/
.Op 177 src/lib/float/div_ext.c
19910	
19911	#include "FP_bias.h"
19912	#include "FP_trap.h"
19913	#include "FP_types.h"
19914	
19915	/*
19916	        November 15, 1984
19917	
19918	        This is a routine to do the work.
19919	        There are two versions: 
19920	        One is based on the partial products method
19921	        and makes no use possible machine instructions
19922	        to divide (hardware dividers).
19923	        The other is used when USE_DIVIDE is defined. It is much faster on
19924	        machines with fast 4 byte operations.
19925	*/
19926	/********************************************************/
19927	
19928	void
19929	div_ext(e1,e2)
19930	EXTEND  *e1,*e2;
19931	{
19932	        short   error = 0;
19933	        B64             result;
19934	        register        unsigned long   *lp;
19935	#ifndef USE_DIVIDE
19936	        short   count;
19937	#else
19938	        unsigned short u[9], v[5];
19939	        register int j;
19940	        register unsigned short *u_p = u;
19941	        int maxv = 4;
19942	#endif
19943	
19944	        if ((e2->m1 | e2->m2) == 0) {
19945	                /*
19946	                 * Exception 8.2 - Divide by zero
19947	                 */
19948	                trap(EFDIVZ);
19949	                e1->m1 = e1->m2 = 0L;
19950	                e1->exp = EXT_MAX;
19951	                return;
19952	        }
19953	        if ((e1->m1 | e1->m2) == 0) {   /* 0 / anything == 0 */
19954	                e1->exp = 0;    /* make sure */
19955	                return;
19956	        }
19957	#ifndef USE_DIVIDE
19958	        /*
19959	         * numbers are right shifted one bit to make sure
19960	         * that m1 is quaranteed to be larger if its
19961	         * maximum bit is set
19962	         */
19963	        b64_rsft(&e1->mantissa);        /* 64 bit shift right */
19964	        b64_rsft(&e2->mantissa);        /* 64 bit shift right */
19965	        e1->exp++;
19966	        e2->exp++;
19967	#endif
19968	        /*      check for underflow, divide by zero, etc        */
19969	        e1->sign ^= e2->sign;
.Ep 178 src/lib/float/div_ext.c
19970	        e1->exp -= e2->exp;
19971	
19972	#ifndef USE_DIVIDE
19973	                /* do division of mantissas     */
19974	                /* uses partial product method  */
19975	                /* init control variables       */
19976	
19977	        count = 64;
19978	        result.h_32 = 0L;
19979	        result.l_32 = 0L;
19980	
19981	                /* partial product division loop */
19982	
19983	        while (count--) {
19984	                /* first left shift result 1 bit        */
19985	                /* this is ALWAYS done                  */
19986	
19987	                b64_lsft(&result);
19988	
19989	                /* compare dividend and divisor         */
19990	                /* if dividend >= divisor add a bit     */
19991	                /* and subtract divisior from dividend  */
19992	
19993	                if ( (e1->m1 < e2->m1) ||
19994	                        ((e1->m1 == e2->m1) && (e1->m2 < e2->m2) ))
19995	                        ;       /* null statement */
19996	                                /* i.e., don't add or subtract */
19997	                else    {
19998	                        result.l_32++;  /* ADD  */
19999	                        if (e2->m2 > e1->m2)
20000	                                e1->m1 -= 1;    /* carry in */
20001	                        e1->m1 -= e2->m1;       /* do SUBTRACTION */
20002	                        e1->m2 -= e2->m2;       /*    SUBTRACTION */
20003	                }
20004	
20005	                /*      shift dividend left one bit OR  */
20006	                /*      IF it equals ZERO we can break out      */
20007	                /*      of the loop, but still must shift       */
20008	                /*      the quotient the remaining count bits   */
20009	                /* NB   save the results of this test in error  */
20010	                /*      if not zero, then the result is inexact. */
20011	                /*      this would be reported in IEEE standard */
20012	
20013	                /*      lp points to dividend                   */
20014	                lp = &e1->m1;
20015	
20016	                error = ((*lp | *(lp+1)) != 0L) ? 1 : 0;
20017	                if (error)      {       /* more work */
20018	                        /*      assume max bit == 0 (see above) */
20019	                        b64_lsft(&e1->mantissa);
20020	                        continue;
20021	                }
20022	                else
20023	                        break;  /* leave loop   */
20024	        }       /* end of divide by subtraction loop    */
20025	
20026	        if (count > 0)  {
20027	                lp = &result.h_32;
20028	                if (count > 31) {       /* move to higher word */
20029	                        *lp = *(lp+1);
.Op 179 src/lib/float/div_ext.c
20030	                        count -= 32;
20031	                        *(lp+1) = 0L;   /* clear low word       */
20032	                }
20033	                if (*lp)
20034	                        *lp <<= count;  /* shift rest of way    */
20035	                lp++;   /*  == &result.l_32     */
20036	                if (*lp) {
20037	                        result.h_32 |= (*lp >> 32-count);
20038	                        *lp <<= count;
20039	                }
20040	        }
20041	#else /* USE_DIVIDE */
20042	
20043	        u[4] = (e1->m2 & 1) << 15;
20044	        b64_rsft(&(e1->mantissa));
20045	        u[0] = e1->m1 >> 16;
20046	        u[1] = e1->m1;
20047	        u[2] = e1->m2 >> 16;
20048	        u[3] = e1->m2;
20049	        u[5] = 0; u[6] = 0; u[7] = 0;
20050	        v[1] = e2->m1 >> 16;
20051	        v[2] = e2->m1;
20052	        v[3] = e2->m2 >> 16;
20053	        v[4] = e2->m2;
20054	        while (! v[maxv]) maxv--;
20055	        result.h_32 = 0;
20056	        result.l_32 = 0;
20057	        lp = &result.h_32;
20058	
20059	        /*
20060	         * Use an algorithm of Knuth (The art of programming, Seminumerical
20061	         * algorithms), to divide u by v. u and v are both seen as numbers
20062	         * with base 65536. 
20063	         */
20064	        for (j = 0; j <= 3; j++, u_p++) {
20065	                unsigned long q_est, temp;
20066	
20067	                if (j == 2) lp++;
20068	                if (u_p[0] == 0 && u_p[1] < v[1]) continue;
20069	                temp = ((unsigned long)u_p[0] << 16) + u_p[1];
20070	                if (u_p[0] >= v[1]) {
20071	                        q_est = 0x0000FFFFL;
20072	                }
20073	                else {
20074	                        q_est = temp / v[1];
20075	                }
20076	                temp -= q_est * v[1];
20077	                while (temp < 0x10000 && v[2]*q_est > ((temp<<16)+u_p[2])) {
20078	                        q_est--;
20079	                        temp += v[1];
20080	                }
20081	                /*      Now, according to Knuth, we have an estimate of the
20082	                        quotient, that is either correct or one too big, but
20083	                        almost always correct.
20084	                */
20085	                if (q_est != 0)  {
20086	                        int i;
20087	                        unsigned long k = 0;
20088	                        int borrow = 0;
20089	
.Ep 180 src/lib/float/div_ext.c
20090	                        for (i = maxv; i > 0; i--) {
20091	                                unsigned long tmp = q_est * v[i] + k + borrow;
20092	                                unsigned short md = tmp;
20093	
20094	                                borrow = (md > u_p[i]);
20095	                                u_p[i] -= md;
20096	                                k = tmp >> 16;
20097	                        }
20098	                        k += borrow;
20099	                        borrow = u_p[0] < k;
20100	                        u_p[0] -= k;
20101	
20102	                        if (borrow) {
20103	                                /* So, this does not happen often; the estimate
20104	                                   was one too big; correct this
20105	                                */
20106	                                *lp |= (j & 1) ? (q_est - 1) : ((q_est-1)<<16);
20107	                                borrow = 0;
20108	                                for (i = maxv; i > 0; i--) {
20109	                                        unsigned long tmp 
20110	                                            = v[i]+(unsigned long)u_p[i]+borrow;
20111	                                        
20112	                                        u_p[i] = tmp;
20113	                                        borrow = tmp >> 16;
20114	                                }
20115	                                u_p[0] += borrow;
20116	                        }
20117	                        else *lp |= (j & 1) ? q_est : (q_est<<16);
20118	                }
20119	        }
20120	#ifdef  EXCEPTION_INEXACT
20121	        u_p = &u[0];
20122	        for (j = 7; j >= 0; j--) {
20123	                if (*u_p++) {
20124	                        error = 1;
20125	                        break;
20126	                }
20127	        }
20128	#endif
20129	#endif
20130	
20131	#ifdef  EXCEPTION_INEXACT
20132	        if (error)      {
20133	                /*
20134	                 * report here exception 8.5 - Inexact
20135	                 * from Draft 8.0 of IEEE P754:
20136	                 * In the absence of an invalid operation exception,
20137	                 * if the rounded result of an operation is not exact or if
20138	                 * it overflows without a trap, then the inexact exception
20139	                 * shall be assigned. The rounded or overflowed result
20140	                 * shall be delivered to the destination.
20141	                 */
20142	                INEXACT();
20143	#endif
20144	        e1->mantissa = result;
20145	
20146	        nrm_ext(e1);
20147	        if (e1->exp < EXT_MIN)  {
20148	                /*
20149	                 * Exception 8.4 - Underflow
.Op 181 src/lib/float/div_ext.c
20150	                 */
20151	                trap(EFUNFL);   /* underflow */
20152	                e1->exp = EXT_MIN;
20153	                e1->m1 = e1->m2 = 0L;
20154	                return;
20155	        }
20156	        if (e1->exp >= EXT_MAX) {
20157	                /*
20158	                 * Exception 8.3 - Overflow
20159	                 */
20160	                trap(EFOVFL);   /* overflow */
20161	                e1->exp = EXT_MAX;
20162	                e1->m1 = e1->m2 = 0L;
20163	                return;
20164	        }
20165	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/dvf4.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

20200	/*
20201	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
20202	  See the copyright notice in the ACK home directory, in the file "Copyright".
20203	*/
20204	
20205	/* $Header: dvf4.c,v 1.5 93/01/05 12:04:53 ceriel Exp $ */
20206	
20207	/*
20208	        DIVIDE TWO SINGLES - SINGLE Precision (dvf 4)
20209	*/
20210	
20211	#include        "FP_types.h"
20212	
20213	void
20214	dvf4(s2,s1)
20215	SINGLE  s1,s2;
20216	{
20217	        EXTEND  e1,e2;
20218	
20219	        extend(&s1,&e1,sizeof(SINGLE));
20220	        extend(&s2,&e2,sizeof(SINGLE));
20221	
20222	                /* do a divide */
20223	        div_ext(&e1,&e2);
20224	        compact(&e1,&s1,sizeof(SINGLE));
20225	}
.Ep 182 src/lib/float/dvf8.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/dvf8.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

20300	/*
20301	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
20302	  See the copyright notice in the ACK home directory, in the file "Copyright".
20303	*/
20304	
20305	/* $Header: dvf8.c,v 1.5 93/01/05 12:04:59 ceriel Exp $ */
20306	
20307	/*
20308	        DIVIDE TWO FLOATS - DOUBLE Precision (DVF 8)
20309	*/
20310	
20311	#include        "FP_types.h"
20312	
20313	void
20314	dvf8(s2,s1)
20315	DOUBLE  s1,s2;
20316	{
20317	        EXTEND  e1,e2;
20318	
20319	        extend(&s1.d[0],&e1,sizeof(DOUBLE));
20320	        extend(&s2.d[0],&e2,sizeof(DOUBLE));
20321	
20322	                /* do a divide */
20323	        div_ext(&e1,&e2);
20324	        compact(&e1,&s1.d[0],sizeof(DOUBLE));
20325	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/extend.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

20400	/*
20401	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
20402	  See the copyright notice in the ACK home directory, in the file "Copyright".
20403	*/
20404	
20405	/* $Header: extend.c,v 1.11 93/01/05 12:05:05 ceriel Exp $ */
20406	
20407	/*
20408	        CONVERTS FLOATING POINT TO EXTENDED FORMAT
20409	
20410	        Two sizes of FLOATING Point are known:
20411	                SINGLE and DOUBLE
20412	*/
20413	/********************************************************/
20414	/*
20415	        It is not required to normalize in extended
20416	        format, but it has been chosen to do so.
20417	        Extended Format is as follows (at exit):
20418	
20419	->sign  S000 0000 | 0000 0000           <SIGN>
.Op 183 src/lib/float/extend.c
20420	->exp   0EEE EEEE | EEEE EEEE           <EXPONENT>
20421	->m1    LFFF FFFF | FFFF FFFF           <L.Fraction>
20422	        FFFF FFFF | FFFF FFFF           <Fraction>
20423	->m2    FFFF FFFF | FFFF FFFF           <Fraction>
20424	        FFFF F000 | 0000 0000           <Fraction>
20425	*/
20426	/********************************************************/
20427	
20428	#include "FP_bias.h"
20429	#include "FP_shift.h"
20430	#include "FP_types.h"
20431	#include "get_put.h"
20432	/********************************************************/
20433	
20434	void
20435	extend(from,to,size)
20436	unsigned long   *from;
20437	EXTEND  *to;
20438	int     size;
20439	{
20440	        register char *cpt1;
20441	        unsigned long   tmp;
20442	        int     leadbit = 0;
20443	
20444	        cpt1 = (char *) from;
20445	
20446	#if FL_MSL_AT_LOW_ADDRESS
20447	#if FL_MSW_AT_LOW_ADDRESS
20448	        to->exp = uget2(cpt1);
20449	#else
20450	        to->exp = uget2(cpt1+2);
20451	#endif
20452	#else
20453	#if FL_MSW_AT_LOW_ADDRESS
20454	        to->exp = uget2(cpt1+(size == sizeof(DOUBLE) ? 4 : 0));
20455	#else
20456	        to->exp = uget2(cpt1+(size == sizeof(DOUBLE) ? 6 : 2));
20457	#endif
20458	#endif
20459	        to->sign = (to->exp & 0x8000);  /* set sign bit */
20460	        to->exp ^= to->sign;
20461	        if (size == sizeof(DOUBLE))
20462	                to->exp >>= DBL_EXPSHIFT;
20463	        else
20464	                to->exp >>= SGL_EXPSHIFT;
20465	        if (to->exp > 0)
20466	                leadbit++;      /* will set Lead bit later      */
20467	        else to->exp++;
20468	
20469	        if (size == sizeof(DOUBLE))     {
20470	#if FL_MSL_AT_LOW_ADDRESS
20471	                to->m1 = get4(cpt1);
20472	                cpt1 += 4;
20473	                tmp = get4(cpt1);
20474	#else
20475	                tmp = get4(cpt1);
20476	                cpt1 += 4;
20477	                to->m1 = get4(cpt1);
20478	#endif
20479	                if (to->exp == 1 && to->m1 == 0 && tmp == 0) {
.Ep 184 src/lib/float/extend.c
20480	                        to->exp = 0;
20481	                        to->sign = 0;
20482	                        to->m1 = 0;
20483	                        to->m2 = 0;
20484	                        return;
20485	                }
20486	                to->m1 <<= DBL_M1LEFT;          /* shift        */
20487	                to->exp -= DBL_BIAS;            /* remove bias  */
20488	                to->m1 |= (tmp>>DBL_RPACK);     /* plus 10 == 32        */
20489	                to->m2 = (tmp<<DBL_LPACK);      /* plus 22 == 32        */
20490	        }
20491	        else    {       /* size == sizeof(SINGLE)               */
20492	                to->m1 = get4(cpt1);
20493	                to->m1  <<= SGL_M1LEFT; /* shift        */
20494	                if (to->exp == 1 && to->m1 == 0) {
20495	                        to->exp = 0;
20496	                        to->sign = 0;
20497	                        to->m1 = 0;
20498	                        to->m2 = 0;
20499	                        return;
20500	                }
20501	                to->exp -= SGL_BIAS;            /* remove bias  */
20502	                to->m2 = 0L;
20503	        }
20504	
20505	        to->m1 |= NORMBIT;                              /* set bit L    */
20506	        if (leadbit == 0) {             /* set or clear Leading Bit     */
20507	                to->m1 &= ~NORMBIT;                     /* clear bit L  */
20508	                nrm_ext(to);                            /* and normalize */
20509	        }
20510	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/fef4.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

20600	/*
20601	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
20602	  See the copyright notice in the ACK home directory, in the file "Copyright".
20603	*/
20604	
20605	/* $Header: fef4.c,v 1.7 93/01/05 12:05:12 ceriel Exp $ */
20606	
20607	/*
20608	        SEPERATE INTO EXPONENT AND FRACTION (FEF 4)
20609	*/
20610	
20611	#include        "FP_types.h"
20612	
20613	void
20614	fef4(r,s1)
20615	SINGLE  s1;
20616	struct fef4_returns     *r;
20617	{
20618	        EXTEND  buf;
20619	        register struct fef4_returns    *p = r; /* make copy; r might refer
.Op 185 src/lib/float/fef4.c
20620	                                                   to itself (see table)
20621	                                                */
20622	
20623	        extend(&s1,&buf,sizeof(SINGLE));
20624	        if (buf.exp == 0 && buf.m1 == 0 && buf.m2 == 0) {
20625	                p->e = 0;
20626	        }
20627	        else {
20628	                p->e = buf.exp+1;
20629	                buf.exp = -1;
20630	        }
20631	        compact(&buf,&p->f,sizeof(SINGLE));
20632	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/fef8.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

20700	/*
20701	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
20702	  See the copyright notice in the ACK home directory, in the file "Copyright".
20703	*/
20704	
20705	/* $Header: fef8.c,v 1.7 93/01/05 12:05:18 ceriel Exp $ */
20706	
20707	/*
20708	        SEPERATE DOUBLE INTO EXPONENT AND FRACTION (FEF 8)
20709	*/
20710	
20711	#include        "FP_types.h"
20712	
20713	void
20714	fef8(r, s1)
20715	DOUBLE  s1;
20716	struct fef8_returns *r;
20717	{
20718	        EXTEND  buf;
20719	        register struct fef8_returns *p = r;    /* make copy, r might refer
20720	                                                   to itself (see table)
20721	                                                */
20722	
20723	        extend(&s1.d[0],&buf,sizeof(DOUBLE));
20724	        if (buf.exp == 0 && buf.m1 == 0 && buf.m2 == 0) {
20725	                p->e = 0;
20726	        }
20727	        else {
20728	                p->e = buf.exp + 1;
20729	                buf.exp = -1;
20730	        }
20731	        compact(&buf,&p->f.d[0],sizeof(DOUBLE));
20732	}
.Ep 186 src/lib/float/fif4.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/fif4.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

20800	/*
20801	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
20802	  See the copyright notice in the ACK home directory, in the file "Copyright".
20803	*/
20804	
20805	/* $Header: fif4.c,v 1.7 93/01/05 12:05:24 ceriel Exp $ */
20806	
20807	/*
20808	        MULTIPLY AND DISMEMBER PARTS (FIF 4)
20809	*/
20810	
20811	#include "FP_types.h"
20812	#include "FP_shift.h"
20813	
20814	void
20815	fif4(p,x,y)
20816	SINGLE  x,y;
20817	struct fif4_returns *p;
20818	{
20819	
20820	        EXTEND  e1,e2;
20821	
20822	        extend(&y,&e1,sizeof(SINGLE));
20823	        extend(&x,&e2,sizeof(SINGLE));
20824	                /* do a multiply */
20825	        mul_ext(&e1,&e2);
20826	        e2 = e1;
20827	        compact(&e2,&y,sizeof(SINGLE));
20828	        if (e1.exp < 0) {
20829	                p->ipart = 0;
20830	                p->fpart = y;
20831	                return;
20832	        }
20833	        if (e1.exp > 30 - SGL_M1LEFT) {
20834	                p->ipart = y;
20835	                p->fpart = 0;
20836	                return;
20837	        }
20838	        b64_sft(&e1.mantissa, 63 - e1.exp);
20839	        b64_sft(&e1.mantissa, e1.exp - 63);     /* "loose" low order bits */
20840	        compact(&e1,&(p->ipart),sizeof(SINGLE));
20841	        extend(&(p->ipart), &e2, sizeof(SINGLE));
20842	        extend(&y, &e1, sizeof(SINGLE));
20843	        sub_ext(&e1, &e2);
20844	        compact(&e1, &(p->fpart), sizeof(SINGLE));
20845	}
.Op 187 src/lib/float/fif8.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/fif8.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

20900	/*
20901	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
20902	  See the copyright notice in the ACK home directory, in the file "Copyright".
20903	*/
20904	
20905	/* $Header: fif8.c,v 1.7 93/01/05 12:05:30 ceriel Exp $ */
20906	
20907	/*
20908	        MULTIPLY AND DISMEMBER PARTS (FIF 8)
20909	*/
20910	
20911	#include "FP_types.h"
20912	#include "FP_shift.h"
20913	
20914	void
20915	fif8(p,x,y)
20916	DOUBLE  x,y;
20917	struct fif8_returns *p;
20918	{
20919	
20920	        EXTEND  e1,e2;
20921	
20922	        extend(&y.d[0],&e1,sizeof(DOUBLE));
20923	        extend(&x.d[0],&e2,sizeof(DOUBLE));
20924	                /* do a multiply */
20925	        mul_ext(&e1,&e2);
20926	        e2 = e1;
20927	        compact(&e2, &y.d[0], sizeof(DOUBLE));
20928	        if (e1.exp < 0) {
20929	                p->ipart.d[0] = 0;
20930	                p->ipart.d[1] = 0;
20931	                p->fpart = y;
20932	                return;
20933	        }
20934	        if (e1.exp > 62 - DBL_M1LEFT) {
20935	                p->ipart = y;
20936	                p->fpart.d[0] = 0;
20937	                p->fpart.d[1] = 0;
20938	                return;
20939	        }
20940	        b64_sft(&e1.mantissa, 63 - e1.exp);
20941	        b64_sft(&e1.mantissa, e1.exp - 63);     /* "loose" low order bits */
20942	        compact(&e1, &(p->ipart.d[0]), sizeof(DOUBLE));
20943	        extend(&(p->ipart.d[0]), &e2, sizeof(DOUBLE));
20944	        extend(&y.d[0], &e1, sizeof(DOUBLE));
20945	        sub_ext(&e1, &e2);
20946	        compact(&e1, &(p->fpart.d[0]), sizeof(DOUBLE));
20947	}
.Ep 188 src/lib/float/mlf4.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/mlf4.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

21000	/*
21001	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
21002	  See the copyright notice in the ACK home directory, in the file "Copyright".
21003	*/
21004	
21005	/* $Header: mlf4.c,v 1.4 93/01/05 12:05:37 ceriel Exp $ */
21006	
21007	/*
21008	 * Multiply Single Precesion Float (MLF 4)
21009	 */
21010	
21011	#include        "FP_types.h"
21012	
21013	void
21014	mlf4(s2,s1)
21015	SINGLE  s1,s2;
21016	{
21017	        EXTEND  e1,e2;
21018	
21019	        extend(&s1,&e1,sizeof(SINGLE));
21020	        extend(&s2,&e2,sizeof(SINGLE));
21021	                /* do a multiply */
21022	        mul_ext(&e1,&e2);
21023	        compact(&e1,&s1,sizeof(SINGLE));
21024	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/mlf8.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

21100	/*
21101	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
21102	  See the copyright notice in the ACK home directory, in the file "Copyright".
21103	*/
21104	
21105	/* $Header: mlf8.c,v 1.4 93/01/05 12:05:44 ceriel Exp $ */
21106	
21107	/*
21108	 * Multiply Double Precision Float (MLF 8)
21109	 */
21110	
21111	#include        "FP_types.h"
21112	
21113	void
21114	mlf8(s2,s1)
21115	DOUBLE  s1,s2;
21116	{
21117	        EXTEND  e1,e2;
21118	
21119	        extend(&s1.d[0],&e1,sizeof(DOUBLE));
.Op 189 src/lib/float/mlf8.c
21120	        extend(&s2.d[0],&e2,sizeof(DOUBLE));
21121	                /* do a multiply */
21122	        mul_ext(&e1,&e2);
21123	        compact(&e1,&s1.d[0],sizeof(DOUBLE));
21124	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/mul_ext.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

21200	/*
21201	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
21202	  See the copyright notice in the ACK home directory, in the file "Copyright".
21203	*/
21204	
21205	/* $Header: mul_ext.c,v 1.6 93/01/05 12:05:51 ceriel Exp $ */
21206	
21207	/*
21208	        ROUTINE TO MULTIPLY TWO EXTENDED FORMAT NUMBERS
21209	*/
21210	
21211	# include "FP_bias.h"
21212	# include "FP_trap.h"
21213	# include "FP_types.h"
21214	# include "FP_shift.h"
21215	
21216	void
21217	mul_ext(e1,e2)
21218	EXTEND  *e1,*e2;
21219	{
21220	        register int    i,j;            /* loop control */
21221	        unsigned short  mp[4];          /* multiplier */
21222	        unsigned short  mc[4];          /* multipcand */
21223	        unsigned short  result[8];      /* result */
21224	        register unsigned short *pres;
21225	
21226	        /* first save the sign (XOR)                    */
21227	        e1->sign ^= e2->sign;
21228	
21229	        /* compute new exponent */
21230	        e1->exp += e2->exp + 1;
21231	        /* 128 bit multiply of mantissas                        */
21232	
21233	                /* assign unknown long formats          */
21234	                /* to known unsigned word formats       */
21235	        mp[0] = e1->m1 >> 16;
21236	        mp[1] = (unsigned short) e1->m1;
21237	        mp[2] = e1->m2 >> 16;
21238	        mp[3] = (unsigned short) e1->m2;
21239	        mc[0] = e2->m1 >> 16;
21240	        mc[1] = (unsigned short) e2->m1;
21241	        mc[2] = e2->m2 >> 16;
21242	        mc[3] = (unsigned short) e2->m2;
21243	        for (i = 8; i--;) {
21244	                result[i] = 0;
.Ep 190 src/lib/float/mul_ext.c
21245	        }
21246	        /*
21247	         *      fill registers with their components
21248	         */
21249	        for(i=4, pres = &result[4];i--;pres--) if (mp[i]) {
21250	                unsigned short k = 0;
21251	                unsigned long mpi = mp[i];
21252	                for(j=4;j--;) {
21253	                        unsigned long tmp = (unsigned long)pres[j] + k;
21254	                        if (mc[j]) tmp += mpi * mc[j];
21255	                        pres[j] = tmp;
21256	                        k = tmp >> 16;
21257	                }
21258	                pres[-1] = k;
21259	        }
21260	        if (! (result[0] & 0x8000)) {
21261	                e1->exp--;
21262	                for (i = 0; i <= 3; i++) {
21263	                        result[i] <<= 1;
21264	                        if (result[i+1]&0x8000) result[i] |= 1;
21265	                }
21266	                result[4] <<= 1;
21267	        }
21268	
21269	        /*
21270	         *      combine the registers to a total
21271	         */
21272	        e1->m1 = ((unsigned long)(result[0]) << 16) + result[1];
21273	        e1->m2 = ((unsigned long)(result[2]) << 16) + result[3];
21274	        if (result[4] & 0x8000) {
21275	                if (++e1->m2 == 0)
21276	                        if (++e1->m1 == 0) {
21277	                                e1->m1 = NORMBIT;
21278	                                e1->exp++;
21279	                        }
21280	        }
21281	
21282	                                        /* check for overflow   */
21283	        if (e1->exp >= EXT_MAX) {
21284	                trap(EFOVFL);
21285	                        /* if caught                    */
21286	                        /* return signed infinity       */
21287	                e1->exp = EXT_MAX;
21288	infinity:       e1->m1 = e1->m2 =0L;
21289	                return;
21290	        }
21291	                                /* check for underflow  */
21292	        if (e1->exp < EXT_MIN)  {
21293	                trap(EFUNFL);
21294	                e1->exp = EXT_MIN;
21295	                goto infinity;
21296	        }
21297	}
.Op 191 src/lib/float/ngf4.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/ngf4.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

21300	/*
21301	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
21302	  See the copyright notice in the ACK home directory, in the file "Copyright".
21303	*/
21304	
21305	/* $Header: ngf4.c,v 1.7 93/01/05 12:05:57 ceriel Exp $ */
21306	
21307	/*
21308	                NEGATE A FLOATING POINT (NGF 4)
21309	*/
21310	/********************************************************/
21311	
21312	#include "FP_types.h"
21313	#include "get_put.h"
21314	
21315	#define OFF ((FL_MSW_AT_LOW_ADDRESS ? 0 : 2) + (FL_MSB_AT_LOW_ADDRESS ? 0 : 1))
21316	void
21317	ngf4(f)
21318	SINGLE  f;
21319	{
21320	        unsigned char *p;
21321	
21322	        if (f != (SINGLE) 0) {
21323	                p = (unsigned char *) &f + OFF;
21324	                *p ^= 0x80;
21325	        }
21326	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/ngf8.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

21400	/*
21401	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
21402	  See the copyright notice in the ACK home directory, in the file "Copyright".
21403	*/
21404	
21405	/* $Header: ngf8.c,v 1.8 93/01/05 12:06:05 ceriel Exp $ */
21406	
21407	/*
21408	                NEGATE A FLOATING POINT (NGF 8)
21409	*/
21410	/********************************************************/
21411	
21412	#include "FP_types.h"
21413	#include "get_put.h"
21414	
21415	#define OFF ((FL_MSL_AT_LOW_ADDRESS ? 0 : 4) + (FL_MSW_AT_LOW_ADDRESS ? 0 : 2) + (FL_MSB_AT_LOW_ADDRESS ? 0 : 1))
21416	
21417	void
21418	ngf8(f)
21419	DOUBLE  f;
.Ep 192 src/lib/float/ngf8.c
21420	{
21421	        unsigned char   *p;
21422	
21423	        if (f.d[0] != 0 || f.d[1] != 0) {
21424	                p = (unsigned char *) &f + OFF;
21425	                *p ^= 0x80;
21426	        }
21427	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/nrm_ext.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

21500	/*
21501	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
21502	  See the copyright notice in the ACK home directory, in the file "Copyright".
21503	*/
21504	
21505	/* $Header: nrm_ext.c,v 1.5 93/01/05 12:06:11 ceriel Exp $ */
21506	
21507	/********************************************************/
21508	/*
21509	        NORMALIZE an EXTENDED FORMAT NUMBER
21510	*/
21511	/********************************************************/
21512	
21513	#include "FP_shift.h"
21514	#include "FP_types.h"
21515	
21516	void
21517	nrm_ext(e1)
21518	EXTEND  *e1;
21519	{
21520	                /* we assume that the mantissa != 0     */
21521	                /* if it is then just return            */
21522	                /* to let it be a problem elsewhere     */
21523	                /* THAT IS, The exponent is not set to  */
21524	                /* zero. If we don't test here an       */
21525	                /* infinite loop is generated when      */
21526	                /* mantissa is zero                     */
21527	
21528	        if ((e1->m1 | e1->m2) == 0L)
21529	                return;
21530	
21531	                /* if top word is zero mov low word     */
21532	                /* to top word, adjust exponent value   */
21533	        if (e1->m1 == 0L)       {
21534	                e1->m1 = e1->m2;
21535	                e1->m2 = 0L;
21536	                e1->exp -= 32;
21537	        }
21538	        if ((e1->m1 & NORMBIT) == 0) {
21539	                unsigned long l = ((unsigned long)NORMBIT >> 1);
21540	                int cnt = -1;
21541	
21542	                while (! (l & e1->m1)) {
21543	                        l >>= 1;
21544	                        cnt--;
.Op 193 src/lib/float/nrm_ext.c
21545	                }
21546	                e1->exp += cnt;
21547	                b64_sft(&(e1->mantissa), cnt);
21548	        }
21549	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/sbf4.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

21600	/*
21601	 (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
21602	  See the copyright notice in the ACK home directory, in the file "Copyright".
21603	*/
21604	
21605	/* $Header: sbf4.c,v 1.8 93/01/05 12:06:16 ceriel Exp $ */
21606	
21607	/*
21608	        SUBTRACT TWO FLOATS - SINGLE Precision (SBF 4)
21609	*/
21610	
21611	#include        "FP_types.h"
21612	
21613	void
21614	sbf4(s2,s1)
21615	SINGLE  s1,s2;
21616	{
21617	        EXTEND e1,e2;
21618	
21619	        if (s2 == (SINGLE) 0) {
21620	                return;
21621	        }
21622	        extend(&s1,&e1,sizeof(SINGLE));
21623	        extend(&s2,&e2,sizeof(SINGLE));
21624	        sub_ext(&e1,&e2);
21625	        compact(&e1,&s1,sizeof(SINGLE));
21626	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/sbf8.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

21700	/*
21701	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
21702	  See the copyright notice in the ACK home directory, in the file "Copyright".
21703	*/
21704	
21705	/* $Header: sbf8.c,v 1.8 93/01/05 12:06:22 ceriel Exp $ */
21706	
21707	/*
21708	        SUBTRACT TWO FLOATS - DOUBLE Precision (SBF 8)
21709	*/
.Ep 194 src/lib/float/sbf8.c
21710	
21711	#include        "FP_types.h"
21712	
21713	void
21714	sbf8(s2,s1)
21715	DOUBLE  s1,s2;
21716	{
21717	        EXTEND e1, e2;
21718	
21719	        if (s2.d[0] == 0 && s2.d[1] == 0) {
21720	                return;
21721	        }
21722	        extend(&s1.d[0],&e1,sizeof(DOUBLE));
21723	        extend(&s2.d[0],&e2,sizeof(DOUBLE));
21724	        sub_ext(&e1,&e2);
21725	        compact(&e1,&s1.d[0],sizeof(DOUBLE));
21726	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/sft_ext.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

21800	/*
21801	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
21802	  See the copyright notice in the ACK home directory, in the file "Copyright".
21803	*/
21804	
21805	/* $Header: sft_ext.c,v 1.5 93/01/05 12:06:28 ceriel Exp $ */
21806	
21807	/*
21808	        SHIFT TWO EXTENDED NUMBERS INTO PROPER
21809	        ALIGNMENT FOR ADDITION (exponents are equal)
21810	        Numbers should not be zero on entry.
21811	*/
21812	
21813	#include "FP_types.h"
21814	
21815	void
21816	sft_ext(e1,e2)
21817	EXTEND  *e1,*e2;
21818	{
21819	        register        EXTEND  *s;
21820	        register        int     diff;
21821	
21822	        diff = e1->exp - e2->exp;
21823	
21824	        if (!diff)
21825	                return; /* exponents are equal  */
21826	
21827	        if (diff < 0)   { /* e2 is larger       */
21828	                        /* shift e1             */
21829	                diff = -diff;
21830	                s = e1;
21831	        }
21832	        else            /* e1 is larger         */
21833	                        /* shift e2             */
21834	                s = e2;
.Op 195 src/lib/float/sft_ext.c
21835	
21836	        s->exp += diff;
21837	        b64_sft(&(s->mantissa), diff);
21838	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/shifter.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

21900	/*
21901	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
21902	  See the copyright notice in the ACK home directory, in the file "Copyright".
21903	*/
21904	
21905	/* $Header: shifter.c,v 1.6 93/01/05 12:06:34 ceriel Exp $ */
21906	
21907	# include "FP_types.h"
21908	
21909	void
21910	b64_sft(e1,n)
21911	B64     *e1;
21912	int     n;
21913	{
21914	        if (n > 0) {
21915	                if (n > 63) {
21916	                        e1->l_32 = 0;
21917	                        e1->h_32 = 0;
21918	                        return;
21919	                }
21920	                if (n >= 32) {
21921	                        e1->l_32 = e1->h_32;
21922	                        e1->h_32 = 0;
21923	                        n -= 32;
21924	                }
21925	                if (n > 0) {
21926	                        e1->l_32 >>= n;
21927	                        if (e1->h_32 != 0) {
21928	                                e1->l_32 |= (e1->h_32 << (32 - n));
21929	                                e1->h_32 >>= n;
21930	                        }
21931	                }
21932	                return;
21933	        }
21934	        n = -n;
21935	        if (n > 0) {
21936	                if (n > 63) {
21937	                        e1->l_32 = 0;
21938	                        e1->h_32 = 0;
21939	                        return;
21940	                }
21941	                if (n >= 32) {
21942	                        e1->h_32 = e1->l_32;
21943	                        e1->l_32 = 0;
21944	                        n -= 32;
21945	                }
21946	                if (n > 0) {
21947	                        e1->h_32 <<= n;
21948	                        if (e1->l_32 != 0) {
21949	                                e1->h_32 |= (e1->l_32 >> (32 - n));
.Ep 196 src/lib/float/shifter.c
21950	                                e1->l_32 <<= n;
21951	                        }
21952	                }
21953	        }
21954	}
	
21956	void
21957	b64_lsft(e1)
21958	B64     *e1;
21959	{
21960	        /*      shift left 1 bit */
21961	        e1->h_32 <<= 1;
21962	        if (e1->l_32 & 0x80000000L) e1->h_32 |= 1;
21963	        e1->l_32 <<= 1;
21964	}
	
21966	void
21967	b64_rsft(e1)
21968	B64     *e1;
21969	{
21970	        /*      shift right 1 bit */
21971	        e1->l_32 >>= 1;
21972	        if (e1->h_32 & 1) e1->l_32 |= 0x80000000L;
21973	        e1->h_32 >>= 1;
21974	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/sub_ext.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

22000	/*
22001	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
22002	  See the copyright notice in the ACK home directory, in the file "Copyright".
22003	*/
22004	
22005	/* $Header: sub_ext.c,v 1.6 93/01/05 12:06:40 ceriel Exp $ */
22006	
22007	/*
22008	        SUBTRACT 2 EXTENDED FORMAT NUMBERS
22009	*/
22010	
22011	#include "FP_types.h"
22012	
22013	void
22014	sub_ext(e1,e2)
22015	EXTEND  *e1,*e2;
22016	{
22017	        if ((e2->m1 | e2->m2) == 0L) {
22018	                return;
22019	        }
22020	        if ((e1->m1 | e1->m2) == 0L) {
22021	                *e1 = *e2;
22022	                e1->sign = e2->sign ? 0 : 1;
22023	                return;
22024	        }
.Op 197 src/lib/float/sub_ext.c
22025	        sft_ext(e1, e2);
22026	        if (e1->sign != e2->sign) {
22027	                /* e1 - e2 = e1 + (-e2) */
22028	                if (b64_add(&e1->mantissa,&e2->mantissa)) { /* addition carry */
22029	                        b64_rsft(&e1->mantissa);      /* shift mantissa one bit RIGHT */
22030	                        e1->m1 |= 0x80000000L;  /* set max bit  */
22031	                        e1->exp++;              /* increase the exponent */
22032	                }
22033	        }
22034	        else if (e2->m1 > e1->m1 ||
22035	                 (e2->m1 == e1->m1 && e2->m2 > e1->m2)) {
22036	                /*      abs(e2) > abs(e1) */
22037	                if (e1->m2 > e2->m2) {
22038	                        e2->m1 -= 1;    /* carry in */
22039	                }
22040	                e2->m1 -= e1->m1;
22041	                e2->m2 -= e1->m2;
22042	                *e1 = *e2;
22043	                e1->sign = e2->sign ? 0 : 1;
22044	        }
22045	        else {
22046	                if (e2->m2 > e1->m2)
22047	                        e1->m1 -= 1;    /* carry in */
22048	                e1->m1 -= e2->m1;
22049	                e1->m2 -= e2->m2;
22050	        }
22051	        nrm_ext(e1);
22052	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/zrf4.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

22100	/*
22101	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
22102	  See the copyright notice in the ACK home directory, in the file "Copyright".
22103	*/
22104	
22105	/* $Header: zrf4.c,v 1.5 93/01/05 12:06:46 ceriel Exp $ */
22106	
22107	/*
22108	        return a zero float (ZRF 4)
22109	*/
22110	
22111	#include "FP_types.h"
22112	
22113	void
22114	zrf4(l)
22115	SINGLE  *l;
22116	{
22117	        *l = 0L;
22118	}
.Ep 198 src/lib/float/zrf8.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/zrf8.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

22200	/*
22201	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
22202	  See the copyright notice in the ACK home directory, in the file "Copyright".
22203	*/
22204	
22205	/* $Header: zrf8.c,v 1.4 93/01/05 12:06:52 ceriel Exp $ */
22206	
22207	/*
22208	        return a zero double (ZRF 8)
22209	*/
22210	
22211	#include "FP_types.h"
22212	
22213	void
22214	zrf8(z)
22215	DOUBLE  *z;
22216	{
22217	
22218	        z->d[0] = 0L;
22219	        z->d[1] = 0L;
22220	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/zrf_ext.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

22300	/*
22301	  (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
22302	  See the copyright notice in the ACK home directory, in the file "Copyright".
22303	*/
22304	
22305	/* $Header: zrf_ext.c,v 1.5 93/01/05 12:06:58 ceriel Exp $ */
22306	
22307	/*
22308	        ZERO and return EXTEND FORMAT FLOAT
22309	*/
22310	
22311	#include "FP_types.h"
22312	
22313	void
22314	zrf_ext(e)
22315	EXTEND  *e;
22316	{
22317	        e->m1 = 0;
22318	        e->m2 = 0;
22319	        e->exp = 0;
22320	        e->sign = 0;
22321	}
.Op 199 src/lib/float/fptrp.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/float/fptrp.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

22400	#
22401	.sect .text; .sect .rom; .sect .data; .sect .bss
22402	.define __fptrp
22403	.sect .text
22404	__fptrp:
22405	#if __i386
22406	        push    ebp
22407	        mov     ebp, esp
22408	        mov     eax, 8(bp)
22409	        call    .Xtrp
22410	        leave
22411	        ret
22412	#else /* i86 */
22413	        push    bp
22414	        mov     bp, sp
22415	        mov     ax, 4(bp)
22416	        call    .Xtrp
22417	        jmp     .cret
22418	#endif


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/fphook/fltpr.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

22500	#include        <stdio.h>
22501	#include        <stdlib.h>
22502	#include        "../stdio/loc_incl.h"
22503	
22504	int _fp_hook = 1;
22505	
22506	char *
22507	_f_print(va_list *ap, int flags, char *s, char c, int precision)
22508	{
22509	        fprintf(stderr,"cannot print floating point\n");
22510	        exit(EXIT_FAILURE);
22511	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/fphook/fphook.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

22600	/*
22601	 * fltpr.c - print floating point numbers
22602	 */
22603	/* $Header: fltpr.c,v 1.4 90/02/27 16:47:40 eck Exp $ */
22604	
22605	#ifndef NOFLOAT
22606	#include        <string.h>
22607	#include        <stdarg.h>
22608	#include        "../stdio/loc_incl.h"
22609	int _fp_hook = 1;
.Ep 200 src/lib/fphook/fphook.c
22610	
22611	static char *
22612	_pfloat(long double r, register char *s, int n, int flags)
22613	{
22614	        register char *s1;
22615	        int sign, dp;
22616	        register int i;
22617	
22618	        s1 = _fcvt(r, n, &dp, &sign);
22619	        if (sign)
22620	                *s++ = '-';
22621	        else if (flags & FL_SIGN)
22622	                *s++ = '+';
22623	        else if (flags & FL_SPACE)
22624	                *s++ = ' ';
22625	
22626	        if (dp<=0)
22627	                *s++ = '0';
22628	        for (i=dp; i>0; i--)
22629	                if (*s1) *s++ = *s1++;
22630	                else *s++ = '0';
22631	        if (((i=n) > 0) || (flags & FL_ALT))
22632	                *s++ = '.';
22633	        while (++dp <= 0) {
22634	                if (--i<0)
22635	                        break;
22636	                *s++ = '0';
22637	        }
22638	        while (--i >= 0)
22639	                if (*s1) *s++ = *s1++;
22640	                else *s++ = '0';
22641	        return s;
22642	}
	
22644	static char *
22645	_pscien(long double r, register char *s, int n, int flags)
22646	{
22647	        int sign, dp; 
22648	        register char *s1;
22649	
22650	        s1 = _ecvt(r, n + 1, &dp, &sign);
22651	        if (sign)
22652	                *s++ = '-';
22653	        else if (flags & FL_SIGN)
22654	                *s++ = '+';
22655	        else if (flags & FL_SPACE)
22656	                *s++ = ' ';
22657	
22658	        *s++ = *s1++;
22659	        if ((n > 0) || (flags & FL_ALT))
22660	                *s++ = '.';
22661	        while (--n >= 0)
22662	                if (*s1) *s++ = *s1++;
22663	                else *s++ = '0';
22664	        *s++ = 'e';
22665	        if ( r != 0 ) --dp ;
22666	        if ( dp<0 ) {
22667	                *s++ = '-' ; dp= -dp ;
22668	        } else {
22669	                *s++ = '+' ;
.Op 201 src/lib/fphook/fphook.c
22670	        }
22671	        if (dp >= 100) {
22672	                *s++ = '0' + (dp / 100);
22673	                dp %= 100;
22674	        }
22675	        *s++ = '0' + (dp/10);
22676	        *s++ = '0' + (dp%10);
22677	        return s;
22678	}
	
22680	#define NDIGINEXP(exp)          (((exp) >= 100 || (exp) <= -100) ? 3 : 2)
22681	#define LOW_EXP                 -4
22682	#define USE_EXP(exp, ndigits)   (((exp) < LOW_EXP + 1) || (exp >= ndigits + 1))
22683	
22684	static char *
22685	_gcvt(long double value, int ndigit, char *s, int flags)
22686	{
22687	        int sign, dp;
22688	        register char *s1, *s2;
22689	        register int i;
22690	        register int nndigit = ndigit;
22691	
22692	        s1 = _ecvt(value, ndigit, &dp, &sign);
22693	        s2 = s;
22694	        if (sign) *s2++ = '-';
22695	        else if (flags & FL_SIGN)
22696	                *s2++ = '+';
22697	        else if (flags & FL_SPACE)
22698	                *s2++ = ' ';
22699	
22700	        if (!(flags & FL_ALT))
22701	                for (i = nndigit - 1; i > 0 && s1[i] == '0'; i--)
22702	                        nndigit--;
22703	
22704	        if (USE_EXP(dp,ndigit)) {
22705	                /* Use E format */
22706	                dp--;
22707	                *s2++ = *s1++;
22708	                if ((nndigit > 1) || (flags & FL_ALT)) *s2++ = '.';
22709	                while (--nndigit > 0) *s2++ = *s1++;
22710	                *s2++ = 'e';
22711	                if (dp < 0) {
22712	                        *s2++ = '-';
22713	                        dp = -dp;
22714	                }
22715	                else     *s2++ = '+';
22716	                s2 += NDIGINEXP(dp);
22717	                *s2 = 0;
22718	                for (i = NDIGINEXP(dp); i > 0; i--) {
22719	                        *--s2 = dp % 10 + '0';
22720	                        dp /= 10;
22721	                }
22722	                return s;
22723	        }
22724	        /* Use f format */
22725	        if (dp <= 0) {
22726	                if (*s1 != '0') {
22727	                        /* otherwise the whole number is 0 */
22728	                        *s2++ = '0';
22729	                        *s2++ = '.';
.Ep 202 src/lib/fphook/fphook.c
22730	                }
22731	                while (dp < 0) {
22732	                        dp++;
22733	                        *s2++ = '0';
22734	                }
22735	        }
22736	        for (i = 1; i <= nndigit; i++) {
22737	                *s2++ = *s1++;
22738	                if (i == dp) *s2++ = '.';
22739	        }
22740	        if (i <= dp) {
22741	                while (i++ <= dp) *s2++ = '0';
22742	                *s2++ = '.';
22743	        }
22744	        if ((s2[-1]=='.') && !(flags & FL_ALT)) s2--;
22745	        *s2 = '\0';
22746	        return s;
22747	}
	
22749	char *
22750	_f_print(va_list *ap, int flags, char *s, char c, int precision)
22751	{
22752	        register char *old_s = s;
22753	        long double ld_val;
22754	
22755	        if (flags & FL_LONGDOUBLE) ld_val = va_arg(*ap, long double);
22756	        else ld_val = (long double) va_arg(*ap, double);
22757	
22758	        switch(c) {
22759	        case 'f':
22760	                s = _pfloat(ld_val, s, precision, flags);
22761	                break;
22762	        case 'e':
22763	        case 'E':
22764	                s = _pscien(ld_val, s, precision , flags);
22765	                break;
22766	        case 'g':
22767	        case 'G':
22768	                s = _gcvt(ld_val, precision, s, flags);
22769	                s += strlen(s);
22770	                break;
22771	        }
22772	        if ( c == 'E' || c == 'G') {
22773	                while (*old_s && *old_s != 'e') old_s++;
22774	                if (*old_s == 'e') *old_s = 'E';
22775	        }
22776	        return s;
22777	}
22778	#endif  /* NOFLOAT */
22779	/* $Header: strtod.c,v 1.3 90/09/07 11:00:24 eck Exp $ */
22780	
22781	#include <stdlib.h>
22782	#include "../ansi/ext_fmt.h"
22783	
22784	void _str_ext_cvt(const char *s, char **ss, struct EXTEND *e);
22785	double _ext_dbl_cvt(struct EXTEND *e);
22786	
22787	double
22788	strtod(const char *p, char **pp)
22789	{
.Op 203 src/lib/fphook/fphook.c
22790	        struct EXTEND e;
22791	
22792	        _str_ext_cvt(p, pp, &e);
22793	        return _ext_dbl_cvt(&e);
22794	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/fphook/strtod.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

22800	#include        <stdio.h>
22801	#include        <stdlib.h>
22802	
22803	double
22804	strtod(const char *p, char **pp)
22805	{
22806	        fprintf(stderr,"cannot print floating point\n");
22807	        exit(EXIT_FAILURE);
22808	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/ether.h	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

22900	/* $Id: ether.h,v 2.2 89/10/23 15:42:19 dupuy Exp $ */
22901	
22902	/* Interface definitions for ethernet access library */
22903	
22904	typedef union etheraddr
22905	{
22906	    unsigned char bytes[6];             /* byteorder safe initialization */
22907	    unsigned short shorts[3];           /* force 2-byte alignment */
22908	}
22909	          ether_addr;
22910	
22911	typedef struct etherpacket
22912	{
22913	    ether_addr dest;
22914	    ether_addr src;
22915	    unsigned char type[2];              /* in network byte order! */
22916	    unsigned short pktlen;              /* length of pktbuf ONLY */
22917	    char *pktbuf;
22918	}
22919	            ether_packet;
22920	
22921	typedef struct ethervec
22922	{
22923	    ether_addr dest;
22924	    ether_addr src;
22925	    unsigned char type[2];              /* in network byte order! */
22926	    unsigned short iovcnt;              /* number of iovec to use */
22927	    struct iovec *iov;                  /* ptr to array of iovec */
22928	}
22929	         ether_vec;
.Ep 204 src/lib/ip/ether.h
22930	
22931	#ifndef __ETHER_BCAST_ADDR__
22932	extern ether_addr ether_bcast_addr;
22933	#endif
22934	
22935	#ifdef __STDC__
22936	
22937	int ether_open (char *name, unsigned type, ether_addr * address);
22938	
22939	ether_addr *ether_address (int fd, ether_addr * address);
22940	
22941	ether_addr *ether_intfaddr (char *intf, ether_addr * address);
22942	
22943	char **ether_interfaces (void);
22944	
22945	int ether_write (int fd, ether_packet * packet);
22946	
22947	int ether_writev (int fd, ether_vec * packet);
22948	
22949	int ether_read (int fd, ether_packet * packet);
22950	
22951	int ether_readv (int fd, ether_vec * packet);
22952	
22953	int ether_blocking (int fd, int state);
22954	
22955	int ether_send_self (int fd);
22956	
22957	int ether_mcast_self (int fd);
22958	
22959	int ether_bcast_self (int fd);
22960	
22961	char *ether_ntoa (ether_addr *);
22962	
22963	ether_addr *ether_aton (char *);
22964	
22965	#ifdef __GNUC__
22966	
22967	/*
22968	 * Avoid stupid warnings if structs aren't defined
22969	 */
22970	
22971	typedef struct in_addr *_ether_NoNsEnSe;
22972	typedef struct hostent *_ether_nOnSeNsE;
22973	
22974	#endif
22975	
22976	char *ether_e2a (ether_addr *, char *);
22977	
22978	ether_addr *ether_a2e (char *, ether_addr *);
22979	
22980	struct in_addr *ether_e2ip (ether_addr *, struct in_addr *);
22981	
22982	ether_addr *ether_ip2e (struct in_addr *, ether_addr *);
22983	
22984	char *ether_e2host (ether_addr *, char *);
22985	
22986	ether_addr *ether_host2e (char *, ether_addr *);
22987	
22988	ether_addr *ether_hostent2e (struct hostent *, ether_addr *);
22989	
.Op 205 src/lib/ip/ether.h
22990	#else
22991	
22992	int ether_open ();
22993	ether_addr *ether_address ();
22994	ether_addr *ether_intfaddr ();
22995	char **ether_interfaces ();
22996	int ether_write ();
22997	int ether_writev ();
22998	int ether_read ();
22999	int ether_readv ();
23000	int ether_blocking ();
23001	int ether_send_self ();
23002	int ether_mcast_self ();
23003	int ether_bcast_self ();
23004	
23005	char *ether_ntoa ();
23006	ether_addr *ether_aton ();
23007	char *ether_e2a ();
23008	ether_addr *ether_a2e ();
23009	struct in_addr *ether_e2ip ();
23010	ether_addr *ether_ip2e ();
23011	char *ether_e2host ();
23012	ether_addr *ether_host2e ();
23013	ether_addr *ether_hostent2e ();
23014	
23015	#endif
23016	
23017	#undef ether_cmp                        /* lose def from netinet/if_ether.h */
23018	
23019	#define ether_cmp(addr1,addr2) \
23020	 ((addr1)->shorts[0] != (addr2)->shorts[0] \
23021	  || (addr1)->shorts[1] != (addr2)->shorts[1] \
23022	  || (addr1)->shorts[2] != (addr2)->shorts[2])
23023	
23024	#define ETHERSTRLEN 18                  /* max length of "xx:xx:xx:xx:xx:xx" */
23025	
23026	#ifdef NOFILE                           /* i.e. we have included sys/param.h */
23027	#ifndef MAXHOSTNAMELEN                  /* but MAXHOSTNAMELEN still isnt set */
23028	#define MAXHOSTNAMELEN 64
23029	#endif
23030	#endif
23031	
23032	/* should be defined in terms of ether_packet struct; need offsetof() macro */
23033	
23034	#define ETHER_DST       0
23035	#define ETHER_SRC       6
23036	#define ETHER_TYPE      12
23037	#define ETHER_PKT       14
23038	#define ETHER_MIN       46
23039	#define ETHER_MAX       1500
23040	
23041	#define ETHER_MINTYPE   0x5DD           /* lowest protocol not valid IEEE802 */
23042	#define ETHER_MAXTYPE   0xFFFF          /* largest possible protocol */
23043	
23044	#define ETHER_MCAST(addr) (((unsigned char *) (addr))[0] & 0x01)
23045	
23046	#ifdef NT_ALLTYPES
23047	#define ETHER_ALLTYPES NT_ALLTYPES
23048	#else
23049	#define ETHER_ALLTYPES ((unsigned) -1)
.Ep 206 src/lib/ip/ether.h
23050	#endif





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/domainname.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

23100	/*
23101	domainname.c
23102	*/
23103	
23104	#include <stdio.h>
23105	#include <string.h>
23106	#include <net/netlib.h>
23107	
23108	int getdomainname(domain, size)
23109	char *domain;
23110	size_t size;
23111	{
23112	        FILE *domainfile;
23113	        char *line;
23114	
23115	        domainfile= fopen("/etc/domainname", "r");
23116	        if (!domainfile)
23117	        {
23118	                return -1;
23119	        }
23120	
23121	        line= fgets(domain, size, domainfile);
23122	        fclose(domainfile);
23123	        if (!line)
23124	                return -1;
23125	        line= strchr(domain, '\n');
23126	        if (line)
23127	                *line= '\0';
23128	        return 0;
23129	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/ether_line.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

23200	/*
23201	**  ETHER_LINE
23202	**
23203	**      This routine parses the array pointed to by "line" (which should be
23204	**      from a file in the format of /etc/ethers) and returns in "eaddr" the
23205	**      ethernet address at the start of the line and the corresponding host
23206	**      name in "hostname".  It assumes either tabs or spaces separate the
23207	**      two.  The buffer pointed to by "hostname" must be big enough to hold
23208	**      the host name plus a NULL byte.
23209	**      The function returns 0 on success and 1 on failure.
.Op 207 src/lib/ip/ether_line.c
23210	**      Arguments are assumed sensible.  Null pointers will probably cause
23211	**      exceptions.
23212	**      Author: Gregory J. Sharp, July 1990
23213	**      Adapted to MINIX: Philip Homburg, May 1992
23214	*/
23215	
23216	#include <sys/types.h>
23217	#include <ctype.h>
23218	#include <stdlib.h>
23219	#include <net/gen/ether.h>
23220	#include <net/gen/if_ether.h>
23221	
23222	int
23223	ether_line(line, eaddr, hostname)
23224	char *                  line;
23225	struct ether_addr *     eaddr;
23226	char *                  hostname;
23227	{
23228	    register int i;
23229	    register unsigned long val;
23230	
23231	/* skip leading white space */
23232	    while (*line != '\n' && (*line == ' ' || *line == '\t'))
23233	        line++;
23234	
23235	/* read the ethernet address */
23236	    for (i = 0; i < 5; i++)
23237	    {
23238	        val = (unsigned long) strtol(line, &line, 16);
23239	        if (val > 255 || *line++ != ':')
23240	            return 1;
23241	        eaddr->ea_addr[i] = val & 0xff;
23242	    }
23243	    val = (unsigned long) strtol(line, &line, 16);
23244	    if (val > 255 || (*line != ' ' && *line != '\t'))
23245	        return 1;
23246	    eaddr->ea_addr[i] = val & 0xff;
23247	
23248	/* skip leading white space */
23249	    while (*line != '\n' && (*line == ' ' || *line == '\t'))
23250	        line++;
23251	
23252	/* read in the hostname */
23253	    while (!isspace(*line))
23254	        *hostname++ = *line++;
23255	    *hostname = '\0';
23256	    return 0;
23257	}
.Ep 208 src/lib/ip/ethera2n.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/ethera2n.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

23300	/*
23301	ethera2n.c
23302	
23303	Convert an ASCII string with an ethernet address into a struct ether_addr.
23304	
23305	Created:        Nov 17, 1992 by Philip Homburg
23306	*/
23307	
23308	#include <sys/types.h>
23309	#include <stdlib.h>
23310	#include <net/netlib.h>
23311	#include <net/gen/ether.h>
23312	#include <net/gen/if_ether.h>
23313	
23314	struct ether_addr *ether_aton(s)
23315	char *s;
23316	{
23317	        static struct ether_addr ea;
23318	
23319	        int i;
23320	        long v;
23321	        char *check;
23322	
23323	        if (s == NULL)
23324	                return NULL;
23325	
23326	        for (i=0; i<6; i++)
23327	        {
23328	                v= strtol(s, &check, 16);
23329	                if (v<0 || v>255)
23330	                        return NULL;
23331	                if ((i<5 && check[0] != ':') || (i == 5 && check[0] != '\0'))
23332	                        return NULL;
23333	                ea.ea_addr[i]= v;
23334	                s= check+1;
23335	        }
23336	        return &ea;
23337	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/ethere2a.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

23400	/* $Id: ethere2a.c,v 2.1 89/10/23 15:42:28 dupuy Exp $ */
23401	/* This file was part of the etherlib package. */
23402	
23403	#include <stdio.h>
23404	
23405	#ifdef _MINIX
23406	#include <sys/types.h>
23407	#include <stdlib.h>
23408	
23409	#include <net/gen/ether.h>
.Op 209 src/lib/ip/ethere2a.c
23410	#include <net/gen/if_ether.h>
23411	
23412	#define ETHERSTRLEN 18                  /* max length of "xx:xx:xx:xx:xx:xx" */
23413	#define ether_addr      ether_addr_t
23414	#define bytes           ea_addr
23415	char *ether_e2a _ARGS(( ether_addr_t *a, char *e ));
23416	#else
23417	#include "libether.h"
23418	#endif
23419	
23420	char *
23421	ether_e2a (addr, estring)
23422	ether_addr *addr;
23423	char *estring;
23424	{
23425	#ifdef lint
23426	    char *sprintf ();
23427	#endif
23428	    if (estring == NULL)
23429	        estring = (char *) malloc (ETHERSTRLEN);
23430	
23431	    if (estring != NULL)
23432	        (void) sprintf (estring, "%x:%x:%x:%x:%x:%x",
23433	                        addr->bytes[0], addr->bytes[1], addr->bytes[2],
23434	                        addr->bytes[3], addr->bytes[4], addr->bytes[5]);
23435	    return (estring);
23436	}
	
23438	#ifndef ETHERDB
23439	
23440	char *
23441	ether_ntoa (addr)
23442	ether_addr *addr;
23443	{
23444	    static char estring[ETHERSTRLEN];
23445	
23446	    return (ether_e2a (addr, estring));
23447	}
	
23449	#endif






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/etherh2n.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

23500	/*
23501	etherh2n.c
23502	
23503	Created:        May 20, 1992 by Philip Homburg
23504	*/
23505	
23506	#include <stdio.h>
23507	#include <string.h>
23508	#include <net/gen/if_ether.h>
23509	
.Ep 210 src/lib/ip/etherh2n.c
23510	int
23511	ether_hostton(hostname, e)
23512	char *hostname;
23513	struct ether_addr *e;
23514	{
23515	        FILE *etherf;
23516	        char b[256], hn[256];
23517	
23518	        etherf= fopen(_PATH_ETHERS, "r");
23519	        if (etherf == NULL)
23520	                return 1;
23521	
23522	        while(fgets(b, sizeof(b), etherf) != NULL)
23523	        {
23524	                if (ether_line(b, e, hn) == 0 && strcmp(hn, hostname) == 0)
23525	                {
23526	                        fclose(etherf);
23527	                        return 0;
23528	                }
23529	        }
23530	        fclose(etherf);
23531	        return 1;
23532	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/ethern2h.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

23600	/*
23601	ethern2h.c
23602	
23603	Created:        Nov 12, 1992 by Philip Homburg
23604	*/
23605	
23606	#include <sys/types.h>
23607	#include <stdio.h>
23608	#include <string.h>
23609	#include <net/gen/ether.h>
23610	#include <net/gen/if_ether.h>
23611	
23612	int
23613	ether_ntohost(hostname, e)
23614	char *hostname;
23615	struct ether_addr *e;
23616	{
23617	        FILE *etherf;
23618	        char b[256];
23619	        struct ether_addr e_tmp;
23620	
23621	        etherf= fopen(_PATH_ETHERS, "r");
23622	        if (etherf == NULL)
23623	                return 1;
23624	
23625	        while(fgets(b, sizeof(b), etherf) != NULL)
23626	        {
23627	                if (ether_line(b, &e_tmp, hostname) == 0 && 
23628	                memcmp(&e_tmp, e, sizeof(e_tmp)) == 0)
23629	                {
.Op 211 src/lib/ip/ethern2h.c
23630	                        fclose(etherf);
23631	                        return 0;
23632	                }
23633	        }
23634	        fclose(etherf);
23635	        return 1;
23636	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/getdomain.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

23700	/*      getdomainname()                                 Author: Kees J. Bot
23701	 *                                                              2 Dec 1994
23702	 */
23703	#define nil 0
23704	#include <sys/types.h>
23705	#include <sys/utsname.h>
23706	#include <unistd.h>
23707	#include <string.h>
23708	
23709	int getdomainname(char *domain, size_t size)
23710	{
23711	        char nodename[256];
23712	        char *dot;
23713	
23714	        if (gethostname(nodename, sizeof(nodename)) < 0)
23715	                return -1;
23716	        nodename[sizeof(nodename)-1]= 0;
23717	        if ((dot= strchr(nodename, '.')) == nil) dot= ".";
23718	
23719	        strncpy(domain, dot+1, size);
23720	        if (size > 0) domain[size-1]= 0;
23721	        return 0;
23722	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/gethnmadr.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

23800	/*
23801	 * Copyright (c) 1985, 1988 Regents of the University of California.
23802	 * All rights reserved.
23803	 *
23804	 * Redistribution and use in source and binary forms are permitted
23805	 * provided that: (1) source distributions retain this entire copyright
23806	 * notice and comment, and (2) distributions including binaries display
23807	 * the following acknowledgement:  ``This product includes software
23808	 * developed by the University of California, Berkeley and its contributors''
23809	 * in the documentation or other materials provided with the distribution
23810	 * and in all advertising materials mentioning features or use of this
23811	 * software. Neither the name of the University nor the names of its
23812	 * contributors may be used to endorse or promote products derived
23813	 * from this software without specific prior written permission.
23814	 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
.Ep 212 src/lib/ip/gethnmadr.c
23815	 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
23816	 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
23817	 */
23818	
23819	#if defined(LIBC_SCCS) && !defined(lint)
23820	static char sccsid[] = "@(#)gethostnamadr.c     6.41 (Berkeley) 6/1/90";
23821	#endif /* LIBC_SCCS and not lint */
23822	
23823	#ifdef _MINIX
23824	#include <sys/types.h>
23825	#include <ctype.h>
23826	#include <errno.h>
23827	#include <stdio.h>
23828	#include <string.h>
23829	
23830	#include <net/hton.h>
23831	#include <net/gen/nameser.h>
23832	#include <net/gen/netdb.h>
23833	#include <net/gen/in.h>
23834	#include <net/gen/inet.h>
23835	#include <net/gen/resolv.h>
23836	#include <net/gen/socket.h>
23837	#else
23838	#include <sys/param.h>
23839	#include <sys/socket.h>
23840	#include <netinet/in.h>
23841	#include <ctype.h>
23842	#include <netdb.h>
23843	#include <stdio.h>
23844	#include <errno.h>
23845	#include <arpa/inet.h>
23846	#include <arpa/nameser.h>
23847	#include <resolv.h>
23848	#endif /* AMOEABA */
23849	
23850	#define MAXALIASES      35
23851	#define MAXADDRS        35
23852	
23853	static char *h_addr_ptrs[MAXADDRS + 1];
23854	
23855	#ifdef _MINIX
23856	struct in_addr
23857	{
23858	        ipaddr_t s_addr;
23859	};
23860	typedef u32_t u_long;
23861	typedef u16_t u_short;
23862	typedef u8_t u_char;
23863	union querybuf;
23864	
23865	extern int dn_skipname _ARGS(( const u_char *comp_dn, const u_char *eom ));
23866	#define getshort _getshort
23867	static struct hostent *getanswer _ARGS(( union querybuf *answer, int anslen, 
23868	        int iquery ));
23869	#define bcmp memcmp
23870	#define bcopy(s, d, l) memcpy(d, s, l)
23871	#endif /* _MINIX */
23872	
23873	static struct hostent host;
23874	static char *host_aliases[MAXALIASES];
.Op 213 src/lib/ip/gethnmadr.c
23875	static char hostbuf[BUFSIZ+1];
23876	static struct in_addr host_addr;
23877	
23878	#ifndef _MINIX
23879	char *strpbrk();
23880	#endif /* !_MINIX */
23881	
23882	#if PACKETSZ > 1024
23883	#define MAXPACKET       PACKETSZ
23884	#else
23885	#define MAXPACKET       1024
23886	#endif
23887	
23888	typedef union querybuf
23889	{
23890	        dns_hdr_t hdr;
23891	        u_char buf[MAXPACKET];
23892	} querybuf_t;
23893	
23894	typedef union align {
23895	    long al;
23896	    char ac;
23897	} align_t;
23898	
23899	static struct hostent *
23900	getanswer(answer, anslen, iquery)
23901	        querybuf_t *answer;
23902	        int anslen;
23903	        int iquery;
23904	{
23905	        register dns_hdr_t *hp;
23906	        register u_char *cp;
23907	        register int n;
23908	        u_char *eom;
23909	        char *bp, **ap;
23910	        int type, class, buflen, ancount, qdcount;
23911	        int haveanswer, getclass = C_ANY;
23912	        char **hap;
23913	
23914	        eom = answer->buf + anslen;
23915	        /*
23916	         * find first satisfactory answer
23917	         */
23918	        hp = &answer->hdr;
23919	        ancount = ntohs(hp->dh_ancount);
23920	        qdcount = ntohs(hp->dh_qdcount);
23921	        bp = hostbuf;
23922	        buflen = sizeof(hostbuf);
23923	        cp = answer->buf + sizeof(dns_hdr_t);
23924	        if (qdcount) {
23925	                if (iquery) {
23926	                        if ((n = dn_expand((u_char *)answer->buf, eom,
23927	                             cp, (u_char *)bp, buflen)) < 0) {
23928	                                h_errno = NO_RECOVERY;
23929	                                return ((struct hostent *) NULL);
23930	                        }
23931	                        cp += n + QFIXEDSZ;
23932	                        host.h_name = bp;
23933	                        n = strlen(bp) + 1;
23934	                        bp += n;
.Ep 214 src/lib/ip/gethnmadr.c
23935	                        buflen -= n;
23936	                } else
23937	                        cp += dn_skipname(cp, eom) + QFIXEDSZ;
23938	                while (--qdcount > 0)
23939	                        cp += dn_skipname(cp, eom) + QFIXEDSZ;
23940	        } else if (iquery) {
23941	                if (hp->dh_flag1 & DHF_AA)
23942	                        h_errno = HOST_NOT_FOUND;
23943	                else
23944	                        h_errno = TRY_AGAIN;
23945	                return ((struct hostent *) NULL);
23946	        }
23947	        ap = host_aliases;
23948	        *ap = NULL;
23949	        host.h_aliases = host_aliases;
23950	        hap = h_addr_ptrs;
23951	        *hap = NULL;
23952	#if BSD >= 43 || defined(h_addr)        /* new-style hostent structure */
23953	        host.h_addr_list = h_addr_ptrs;
23954	#endif
23955	        haveanswer = 0;
23956	        while (--ancount >= 0 && cp < eom) {
23957	                if ((n = dn_expand((u_char *)answer->buf, eom, cp, (u_char *)bp,
23958	                        buflen)) < 0)
23959	                        break;
23960	                cp += n;
23961	                type = getshort(cp);
23962	                cp += sizeof(u_short);
23963	                class = getshort(cp);
23964	                cp += sizeof(u_short) + sizeof(u_long);
23965	                n = getshort(cp);
23966	                cp += sizeof(u_short);
23967	                if (type == T_CNAME) {
23968	                        cp += n;
23969	                        if (ap >= &host_aliases[MAXALIASES-1])
23970	                                continue;
23971	                        *ap++ = bp;
23972	                        n = strlen(bp) + 1;
23973	                        bp += n;
23974	                        buflen -= n;
23975	                        continue;
23976	                }
23977	                if (iquery && type == T_PTR) {
23978	                        if ((n = dn_expand((u8_t *)answer->buf, eom,
23979	                            cp, (u8_t *)bp, buflen)) < 0) {
23980	                                cp += n;
23981	                                continue;
23982	                        }
23983	                        cp += n;
23984	                        host.h_name = bp;
23985	                        return(&host);
23986	                }
23987	                if (iquery || type != T_A)  {
23988	#ifdef DEBUG
23989	                        if (_res.options & RES_DEBUG)
23990	                                printf("unexpected answer type %d, size %d\n",
23991	                                        type, n);
23992	#endif
23993	                        cp += n;
23994	                        continue;
.Op 215 src/lib/ip/gethnmadr.c
23995	                }
23996	                if (haveanswer) {
23997	                        if (n != host.h_length) {
23998	                                cp += n;
23999	                                continue;
24000	                        }
24001	                        if (class != getclass) {
24002	                                cp += n;
24003	                                continue;
24004	                        }
24005	                } else {
24006	                        host.h_length = n;
24007	                        getclass = class;
24008	                        host.h_addrtype = (class == C_IN) ? AF_INET : AF_UNSPEC;
24009	                        if (!iquery) {
24010	                                host.h_name = bp;
24011	                                bp += strlen(bp) + 1;
24012	                        }
24013	                }
24014	
24015	                bp += (size_t)(sizeof(align_t) - 
24016	                                                ((u_long)bp % sizeof(align_t)));
24017	
24018	                if (bp + n >= &hostbuf[sizeof(hostbuf)]) {
24019	#ifdef DEBUG
24020	                        if (_res.options & RES_DEBUG)
24021	                                printf("size (%d) too big\n", n);
24022	#endif
24023	                        break;
24024	                }
24025	                bcopy(cp, *hap++ = bp, n);
24026	                bp +=n;
24027	                cp += n;
24028	                haveanswer++;
24029	        }
24030	        if (haveanswer) {
24031	                *ap = NULL;
24032	#if BSD >= 43 || defined(h_addr)        /* new-style hostent structure */
24033	                *hap = NULL;
24034	#else
24035	                host.h_addr = h_addr_ptrs[0];
24036	#endif
24037	                return (&host);
24038	        } else {
24039	                h_errno = TRY_AGAIN;
24040	                return ((struct hostent *) NULL);
24041	        }
24042	}
	
24044	struct hostent *
24045	gethostbyname(name)
24046	        char *name;
24047	{
24048	        querybuf_t buf;
24049	        register char *cp;
24050	        int n;
24051	
24052	        /*
24053	         * disallow names consisting only of digits/dots, unless
24054	         * they end in a dot.
.Ep 216 src/lib/ip/gethnmadr.c
24055	         */
24056	        if (isdigit(name[0]))
24057	                for (cp = name;; ++cp) {
24058	                        if (!*cp) {
24059	                                if (*--cp == '.')
24060	                                        break;
24061	                                /*
24062	                                 * All-numeric, no dot at the end.
24063	                                 * Fake up a hostent as if we'd actually
24064	                                 * done a lookup.  What if someone types
24065	                                 * 255.255.255.255?  The test below will
24066	                                 * succeed spuriously... ???
24067	                                 */
24068	                                if ((host_addr.s_addr = inet_addr(name)) == -1) {
24069	                                        h_errno = HOST_NOT_FOUND;
24070	                                        return((struct hostent *) NULL);
24071	                                }
24072	                                host.h_name = name;
24073	                                host.h_aliases = host_aliases;
24074	                                host_aliases[0] = NULL;
24075	                                host.h_addrtype = AF_INET;
24076	                                host.h_length = sizeof(u_long);
24077	                                h_addr_ptrs[0] = (char *)&host_addr;
24078	                                h_addr_ptrs[1] = (char *)0;
24079	#if BSD >= 43 || defined(h_addr)        /* new-style hostent structure */
24080	                                host.h_addr_list = h_addr_ptrs;
24081	#else
24082	                                host.h_addr = h_addr_ptrs[0];
24083	#endif
24084	                                return (&host);
24085	                        }
24086	                        if (!isdigit(*cp) && *cp != '.') 
24087	                                break;
24088	                }
24089	
24090	        if ((n = res_search(name, C_IN, T_A, buf.buf, sizeof(buf))) < 0) {
24091	#ifdef DEBUG
24092	                if (_res.options & RES_DEBUG)
24093	                        printf("res_search failed\n");
24094	#endif
24095	                return ((struct hostent *) NULL);
24096	        }
24097	        return (getanswer(&buf, n, 0));
24098	}
	
24100	struct hostent *
24101	gethostbyaddr(addr, len, type)
24102	        const char *addr;
24103	        int len, type;
24104	{
24105	        int n;
24106	        querybuf_t buf;
24107	        register struct hostent *hp;
24108	        char qbuf[MAXDNAME];
24109	        
24110	        if (type != AF_INET)
24111	                return ((struct hostent *) NULL);
24112	        (void)sprintf(qbuf, "%u.%u.%u.%u.in-addr.arpa",
24113	                ((unsigned)addr[3] & 0xff),
24114	                ((unsigned)addr[2] & 0xff),
.Op 217 src/lib/ip/gethnmadr.c
24115	                ((unsigned)addr[1] & 0xff),
24116	                ((unsigned)addr[0] & 0xff));
24117	        n = res_query(qbuf, C_IN, T_PTR, (u8_t *)&buf, sizeof(buf));
24118	        if (n < 0) {
24119	#ifdef DEBUG
24120	                if (_res.options & RES_DEBUG)
24121	                        printf("res_query failed\n");
24122	#endif
24123	                return ((struct hostent *) NULL);
24124	        }
24125	        hp = getanswer(&buf, n, 1);
24126	        if (hp == NULL)
24127	                return ((struct hostent *) NULL);
24128	        hp->h_addrtype = type;
24129	        hp->h_length = len;
24130	        h_addr_ptrs[0] = (char *)&host_addr;
24131	        h_addr_ptrs[1] = (char *)0;
24132	        host_addr = *(struct in_addr *)addr;
24133	#if BSD < 43 && !defined(h_addr)        /* new-style hostent structure */
24134	        hp->h_addr = h_addr_ptrs[0];
24135	#endif
24136	        return(hp);
24137	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/gethostent.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

24200	/*
24201	 * Copyright (c) 1985, 1988 Regents of the University of California.
24202	 * All rights reserved.
24203	 *
24204	 * Redistribution and use in source and binary forms are permitted
24205	 * provided that: (1) source distributions retain this entire copyright
24206	 * notice and comment, and (2) distributions including binaries display
24207	 * the following acknowledgement:  ``This product includes software
24208	 * developed by the University of California, Berkeley and its contributors''
24209	 * in the documentation or other materials provided with the distribution
24210	 * and in all advertising materials mentioning features or use of this
24211	 * software. Neither the name of the University nor the names of its
24212	 * contributors may be used to endorse or promote products derived
24213	 * from this software without specific prior written permission.
24214	 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
24215	 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
24216	 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
24217	 */
24218	
24219	#if defined(LIBC_SCCS) && !defined(lint)
24220	static char sccsid[] = "@(#)gethostnamadr.c     6.41 (Berkeley) 6/1/90";
24221	#endif /* LIBC_SCCS and not lint */
24222	
24223	/* Prefix the functions defined here with underscores to distinguish them
24224	 * from the newer replacements in the resolver library.
24225	 */
24226	#define sethostent      _sethostent
24227	#define endhostent      _endhostent
24228	#define gethostent      _gethostent
24229	#define gethostbyname   _gethostbyname
.Ep 218 src/lib/ip/gethostent.c
24230	#define gethostbyaddr   _gethostbyaddr
24231	
24232	#ifdef _MINIX
24233	#include <sys/types.h>
24234	#include <ctype.h>
24235	#include <errno.h>
24236	#include <stdio.h>
24237	#include <string.h>
24238	
24239	#include <net/hton.h>
24240	#include <net/gen/nameser.h>
24241	#include <net/gen/netdb.h>
24242	#include <net/gen/in.h>
24243	#include <net/gen/inet.h>
24244	#include <net/gen/resolv.h>
24245	#include <net/gen/socket.h>
24246	#else
24247	#include <sys/param.h>
24248	#include <sys/socket.h>
24249	#include <netinet/in.h>
24250	#include <ctype.h>
24251	#include <netdb.h>
24252	#include <stdio.h>
24253	#include <errno.h>
24254	#include <arpa/inet.h>
24255	#include <arpa/nameser.h>
24256	#include <resolv.h>
24257	#endif /* !_MINIX */
24258	
24259	#define MAXALIASES      35
24260	#define MAXADDRS        35
24261	
24262	#ifdef _MINIX
24263	typedef u32_t u_long;
24264	typedef u16_t u_short;
24265	typedef u8_t u_char;
24266	
24267	#define bcmp    memcmp
24268	#endif /* _MINIX */
24269	
24270	static struct hostent host;
24271	static char *host_aliases[MAXALIASES];
24272	static char hostbuf[BUFSIZ+1];
24273	static FILE *hostf = NULL;
24274	static u_long hostaddr[(MAXADDRS+sizeof(u_long)-1)/sizeof(u_long)];
24275	static char *host_addrs[2];
24276	static int stayopen = 0;
24277	
24278	#ifndef _MINIX
24279	char *strpbrk();
24280	#endif /* !_MINIX */
24281	
24282	void
24283	sethostent(f)
24284	        int f;
24285	{
24286	        if (hostf == NULL)
24287	                hostf = fopen(_PATH_HOSTS, "r" );
24288	        else
24289	                rewind(hostf);
.Op 219 src/lib/ip/gethostent.c
24290	        stayopen |= f;
24291	}
	
24293	void
24294	endhostent()
24295	{
24296	        if (hostf && !stayopen) {
24297	                (void) fclose(hostf);
24298	                hostf = NULL;
24299	        }
24300	}
	
24302	struct hostent *
24303	gethostent()
24304	{
24305	        char *p;
24306	        register char *cp, **q;
24307	
24308	        if (hostf == NULL && (hostf = fopen(_PATH_HOSTS, "r" )) == NULL)
24309	                return (NULL);
24310	again:
24311	        if ((p = fgets(hostbuf, BUFSIZ, hostf)) == NULL)
24312	                return (NULL);
24313	        if (*p == '#')
24314	                goto again;
24315	        cp = strpbrk(p, "#\n");
24316	        if (cp == NULL)
24317	                goto again;
24318	        *cp = '\0';
24319	        cp = strpbrk(p, " \t");
24320	        if (cp == NULL)
24321	                goto again;
24322	        *cp++ = '\0';
24323	        /* THIS STUFF IS INTERNET SPECIFIC */
24324	#if BSD >= 43 || defined(h_addr)        /* new-style hostent structure */
24325	        host.h_addr_list = host_addrs;
24326	#endif
24327	        host.h_addr = (char *) hostaddr;
24328	        *((u_long *)host.h_addr) = inet_addr(p);
24329	        host.h_length = sizeof (u_long);
24330	        host.h_addrtype = AF_INET;
24331	        while (*cp == ' ' || *cp == '\t')
24332	                cp++;
24333	        host.h_name = cp;
24334	        q = host.h_aliases = host_aliases;
24335	        cp = strpbrk(cp, " \t");
24336	        if (cp != NULL) 
24337	                *cp++ = '\0';
24338	        while (cp && *cp) {
24339	                if (*cp == ' ' || *cp == '\t') {
24340	                        cp++;
24341	                        continue;
24342	                }
24343	                if (q < &host_aliases[MAXALIASES - 1])
24344	                        *q++ = cp;
24345	                cp = strpbrk(cp, " \t");
24346	                if (cp != NULL)
24347	                        *cp++ = '\0';
24348	        }
24349	        *q = NULL;
.Ep 220 src/lib/ip/gethostent.c
24350	        return (&host);
24351	}
	
24353	struct hostent *
24354	gethostbyname(name)
24355	        char *name;
24356	{
24357	        register struct hostent *p;
24358	        register char **cp;
24359	        
24360	        sethostent(0);
24361	        while (p = gethostent()) {
24362	                if (strcasecmp(p->h_name, name) == 0)
24363	                        break;
24364	                for (cp = p->h_aliases; *cp != 0; cp++)
24365	                        if (strcasecmp(*cp, name) == 0)
24366	                                goto found;
24367	        }
24368	found:
24369	        endhostent();
24370	        return (p);
24371	}
	
24373	struct hostent *
24374	gethostbyaddr(addr, len, type)
24375	        const char *addr;
24376	        int len, type;
24377	{
24378	        register struct hostent *p;
24379	
24380	        sethostent(0);
24381	        while (p = gethostent())
24382	                if (p->h_addrtype == type && !bcmp(p->h_addr, addr, len))
24383	                        break;
24384	        endhostent();
24385	        return (p);
24386	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/gethostname.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

24400	/* gethostname(2) system call emulation */
24401	
24402	#include <sys/types.h>
24403	#include <fcntl.h>
24404	#include <stdlib.h>
24405	#include <string.h>
24406	#include <unistd.h>
24407	#include <net/gen/netdb.h>
24408	
24409	#define HOSTNAME_FILE "/etc/hostname.file"
24410	
24411	int gethostname(char *buf, size_t len)
24412	{
24413	        int fd;
24414	        int r;
.Op 221 src/lib/ip/gethostname.c
24415	        char *nl;
24416	
24417	        if ((fd= open(HOSTNAME_FILE, O_RDONLY)) < 0) return -1;
24418	
24419	        r= read(fd, buf, len);
24420	        close(fd);
24421	        if (r == -1) return -1;
24422	
24423	        buf[len-1]= '\0';
24424	        if ((nl= strchr(buf, '\n')) != NULL) *nl= '\0';
24425	        return 0;
24426	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/getproto.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

24500	/*
24501	 * Copyright (c) 1983 Regents of the University of California.
24502	 * All rights reserved.
24503	 *
24504	 * Redistribution and use in source and binary forms are permitted
24505	 * provided that: (1) source distributions retain this entire copyright
24506	 * notice and comment, and (2) distributions including binaries display
24507	 * the following acknowledgement:  ``This product includes software
24508	 * developed by the University of California, Berkeley and its contributors''
24509	 * in the documentation or other materials provided with the distribution
24510	 * and in all advertising materials mentioning features or use of this
24511	 * software. Neither the name of the University nor the names of its
24512	 * contributors may be used to endorse or promote products derived
24513	 * from this software without specific prior written permission.
24514	 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
24515	 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
24516	 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
24517	 */
24518	
24519	#if defined(LIBC_SCCS) && !defined(lint)
24520	static char sccsid[] = "@(#)getproto.c  5.6 (Berkeley) 6/1/90";
24521	#endif /* LIBC_SCCS and not lint */
24522	
24523	#include <stddef.h>
24524	
24525	#ifdef _MINIX
24526	#include <ansi.h>
24527	#include <net/gen/netdb.h>
24528	#endif
24529	
24530	extern int _proto_stayopen;
24531	
24532	struct protoent *
24533	getprotobynumber(proto)
24534	        register int proto;
24535	{
24536	        register struct protoent *p;
24537	
24538	        setprotoent(_proto_stayopen);
24539	        while (p = getprotoent())
.Ep 222 src/lib/ip/getproto.c
24540	                if (p->p_proto == proto)
24541	                        break;
24542	        if (!_proto_stayopen)
24543	                endprotoent();
24544	        return (p);
24545	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/getprotoent.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

24600	/*
24601	 * Copyright (c) 1983 Regents of the University of California.
24602	 * All rights reserved.
24603	 *
24604	 * Redistribution and use in source and binary forms are permitted
24605	 * provided that: (1) source distributions retain this entire copyright
24606	 * notice and comment, and (2) distributions including binaries display
24607	 * the following acknowledgement:  ``This product includes software
24608	 * developed by the University of California, Berkeley and its contributors''
24609	 * in the documentation or other materials provided with the distribution
24610	 * and in all advertising materials mentioning features or use of this
24611	 * software. Neither the name of the University nor the names of its
24612	 * contributors may be used to endorse or promote products derived
24613	 * from this software without specific prior written permission.
24614	 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
24615	 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
24616	 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
24617	 */
24618	
24619	#if defined(LIBC_SCCS) && !defined(lint)
24620	static char sccsid[] = "@(#)getprotoent.c       5.7 (Berkeley) 6/1/90";
24621	#endif /* LIBC_SCCS and not lint */
24622	
24623	#include <ctype.h>
24624	#include <stdio.h>
24625	#include <stdlib.h>
24626	
24627	#ifdef _MINIX
24628	#include <net/gen/netdb.h>
24629	
24630	static char *any _ARGS(( char *cp, char *match ));
24631	#endif
24632	
24633	#define MAXALIASES      35
24634	
24635	static FILE *protof = NULL;
24636	static char line[BUFSIZ+1];
24637	static struct protoent proto;
24638	static char *proto_aliases[MAXALIASES];
24639	int _proto_stayopen;
24640	
24641	void
24642	setprotoent(f)
24643	        int f;
24644	{
.Op 223 src/lib/ip/getprotoent.c
24645	        if (protof == NULL)
24646	                protof = fopen(_PATH_PROTOCOLS, "r" );
24647	        else
24648	                rewind(protof);
24649	        _proto_stayopen |= f;
24650	}
	
24652	void
24653	endprotoent()
24654	{
24655	        if (protof) {
24656	                fclose(protof);
24657	                protof = NULL;
24658	        }
24659	        _proto_stayopen = 0;
24660	}
	
24662	struct protoent *
24663	getprotoent()
24664	{
24665	        char *p;
24666	        register char *cp, **q;
24667	
24668	        if (protof == NULL && (protof = fopen(_PATH_PROTOCOLS, "r" )) == NULL)
24669	                return (NULL);
24670	again:
24671	        if ((p = fgets(line, BUFSIZ, protof)) == NULL)
24672	                return (NULL);
24673	        if (*p == '#')
24674	                goto again;
24675	        cp = any(p, "#\n");
24676	        if (cp == NULL)
24677	                goto again;
24678	        *cp = '\0';
24679	        proto.p_name = p;
24680	        cp = any(p, " \t");
24681	        if (cp == NULL)
24682	                goto again;
24683	        *cp++ = '\0';
24684	        while (*cp == ' ' || *cp == '\t')
24685	                cp++;
24686	        p = any(cp, " \t");
24687	        if (p != NULL)
24688	                *p++ = '\0';
24689	        proto.p_proto = atoi(cp);
24690	        q = proto.p_aliases = proto_aliases;
24691	        if (p != NULL) {
24692	                cp = p;
24693	                while (cp && *cp) {
24694	                        if (*cp == ' ' || *cp == '\t') {
24695	                                cp++;
24696	                                continue;
24697	                        }
24698	                        if (q < &proto_aliases[MAXALIASES - 1])
24699	                                *q++ = cp;
24700	                        cp = any(cp, " \t");
24701	                        if (cp != NULL)
24702	                                *cp++ = '\0';
24703	                }
24704	        }
.Ep 224 src/lib/ip/getprotoent.c
24705	        *q = NULL;
24706	        return (&proto);
24707	}
	
24709	static char *
24710	any(cp, match)
24711	        register char *cp;
24712	        char *match;
24713	{
24714	        register char *mp, c;
24715	
24716	        while (c = *cp) {
24717	                for (mp = match; *mp; mp++)
24718	                        if (*mp == c)
24719	                                return (cp);
24720	                cp++;
24721	        }
24722	        return ((char *)0);
24723	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/getservent.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

24800	/*
24801	 * Copyright (c) 1983 Regents of the University of California.
24802	 * All rights reserved.
24803	 *
24804	 * Redistribution and use in source and binary forms are permitted
24805	 * provided that: (1) source distributions retain this entire copyright
24806	 * notice and comment, and (2) distributions including binaries display
24807	 * the following acknowledgement:  ``This product includes software
24808	 * developed by the University of California, Berkeley and its contributors''
24809	 * in the documentation or other materials provided with the distribution
24810	 * and in all advertising materials mentioning features or use of this
24811	 * software. Neither the name of the University nor the names of its
24812	 * contributors may be used to endorse or promote products derived
24813	 * from this software without specific prior written permission.
24814	 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
24815	 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
24816	 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
24817	 */
24818	
24819	#if defined(LIBC_SCCS) && !defined(lint)
24820	static char sccsid[] = "@(#)getservent.c        5.8 (Berkeley) 6/1/90";
24821	#endif /* LIBC_SCCS and not lint */
24822	
24823	#include <sys/types.h>
24824	#include <ctype.h>
24825	#include <stdio.h>
24826	#include <stdlib.h>
24827	
24828	#include <net/hton.h>
24829	#include <net/gen/netdb.h>
24830	
24831	#define MAXALIASES      35
24832	
24833	static FILE *servf = NULL;
24834	static char line[BUFSIZ+1];
.Op 225 src/lib/ip/getservent.c
24835	static struct servent serv;
24836	static char *serv_aliases[MAXALIASES];
24837	int _serv_stayopen;
24838	
24839	static char *any _ARGS(( char *cp, char *match ));
24840	
24841	void
24842	setservent(f)
24843	        int f;
24844	{
24845	        if (servf == NULL)
24846	                servf = fopen(_PATH_SERVICES, "r" );
24847	        else
24848	                rewind(servf);
24849	        _serv_stayopen |= f;
24850	}
	
24852	void
24853	endservent()
24854	{
24855	        if (servf) {
24856	                fclose(servf);
24857	                servf = NULL;
24858	        }
24859	        _serv_stayopen = 0;
24860	}
	
24862	struct servent *
24863	getservent()
24864	{
24865	        char *p;
24866	        register char *cp, **q;
24867	
24868	        if (servf == NULL && (servf = fopen(_PATH_SERVICES, "r" )) == NULL)
24869	                return (NULL);
24870	again:
24871	        if ((p = fgets(line, BUFSIZ, servf)) == NULL)
24872	                return (NULL);
24873	        if (*p == '#')
24874	                goto again;
24875	        cp = any(p, "#\n");
24876	        if (cp == NULL)
24877	                goto again;
24878	        *cp = '\0';
24879	        serv.s_name = p;
24880	        p = any(p, " \t");
24881	        if (p == NULL)
24882	                goto again;
24883	        *p++ = '\0';
24884	        while (*p == ' ' || *p == '\t')
24885	                p++;
24886	        cp = any(p, ",/");
24887	        if (cp == NULL)
24888	                goto again;
24889	        *cp++ = '\0';
24890	        serv.s_port = htons((u16_t)atoi(p));
24891	        serv.s_proto = cp;
24892	        q = serv.s_aliases = serv_aliases;
24893	        cp = any(cp, " \t");
24894	        if (cp != NULL)
.Ep 226 src/lib/ip/getservent.c
24895	                *cp++ = '\0';
24896	        while (cp && *cp) {
24897	                if (*cp == ' ' || *cp == '\t') {
24898	                        cp++;
24899	                        continue;
24900	                }
24901	                if (q < &serv_aliases[MAXALIASES - 1])
24902	                        *q++ = cp;
24903	                cp = any(cp, " \t");
24904	                if (cp != NULL)
24905	                        *cp++ = '\0';
24906	        }
24907	        *q = NULL;
24908	        return (&serv);
24909	}
	
24911	static char *
24912	any(cp, match)
24913	        register char *cp;
24914	        char *match;
24915	{
24916	        register char *mp, c;
24917	
24918	        while (c = *cp) {
24919	                for (mp = match; *mp; mp++)
24920	                        if (*mp == c)
24921	                                return (cp);
24922	                cp++;
24923	        }
24924	        return ((char *)0);
24925	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/getsrvbyname.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

25000	/*
25001	 * Copyright (c) 1983 Regents of the University of California.
25002	 * All rights reserved.
25003	 *
25004	 * Redistribution and use in source and binary forms are permitted
25005	 * provided that: (1) source distributions retain this entire copyright
25006	 * notice and comment, and (2) distributions including binaries display
25007	 * the following acknowledgement:  ``This product includes software
25008	 * developed by the University of California, Berkeley and its contributors''
25009	 * in the documentation or other materials provided with the distribution
25010	 * and in all advertising materials mentioning features or use of this
25011	 * software. Neither the name of the University nor the names of its
25012	 * contributors may be used to endorse or promote products derived
25013	 * from this software without specific prior written permission.
25014	 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
25015	 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
25016	 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
25017	 */
25018	
25019	#if defined(LIBC_SCCS) && !defined(lint)
.Op 227 src/lib/ip/getsrvbyname.c
25020	static char sccsid[] = "@(#)getservbyname.c     5.6 (Berkeley) 6/1/90";
25021	#endif /* LIBC_SCCS and not lint */
25022	
25023	#include <string.h>
25024	
25025	#include <net/gen/netdb.h>
25026	
25027	extern int _serv_stayopen;
25028	
25029	struct servent *
25030	getservbyname(name, proto)
25031	        const char *name, *proto;
25032	{
25033	        register struct servent *p;
25034	        register char **cp;
25035	
25036	        setservent(_serv_stayopen);
25037	        while (p = getservent()) {
25038	                if (strcmp(name, p->s_name) == 0)
25039	                        goto gotname;
25040	                for (cp = p->s_aliases; *cp; cp++)
25041	                        if (strcmp(name, *cp) == 0)
25042	                                goto gotname;
25043	                continue;
25044	gotname:
25045	                if (proto == 0 || strcmp(p->s_proto, proto) == 0)
25046	                        break;
25047	        }
25048	        if (!_serv_stayopen)
25049	                endservent();
25050	        return (p);
25051	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/getsrvbyport.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

25100	/*
25101	 * Copyright (c) 1983 Regents of the University of California.
25102	 * All rights reserved.
25103	 *
25104	 * Redistribution and use in source and binary forms are permitted
25105	 * provided that: (1) source distributions retain this entire copyright
25106	 * notice and comment, and (2) distributions including binaries display
25107	 * the following acknowledgement:  ``This product includes software
25108	 * developed by the University of California, Berkeley and its contributors''
25109	 * in the documentation or other materials provided with the distribution
25110	 * and in all advertising materials mentioning features or use of this
25111	 * software. Neither the name of the University nor the names of its
25112	 * contributors may be used to endorse or promote products derived
25113	 * from this software without specific prior written permission.
25114	 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
25115	 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
25116	 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
25117	 */
25118	
25119	#if defined(LIBC_SCCS) && !defined(lint)
.Ep 228 src/lib/ip/getsrvbyport.c
25120	static char sccsid[] = "@(#)getservbyport.c     5.6 (Berkeley) 6/1/90";
25121	#endif /* LIBC_SCCS and not lint */
25122	
25123	#include <stddef.h>
25124	#include <string.h>
25125	
25126	#ifdef _MINIX
25127	#include <net/gen/netdb.h>
25128	#endif
25129	
25130	extern int _serv_stayopen;
25131	
25132	struct servent *
25133	getservbyport(port, proto)
25134	        int port;
25135	        const char *proto;
25136	{
25137	        register struct servent *p;
25138	
25139	        setservent(_serv_stayopen);
25140	        while (p = getservent()) {
25141	                if (p->s_port != port)
25142	                        continue;
25143	                if (proto == 0 || strcmp(p->s_proto, proto) == 0)
25144	                        break;
25145	        }
25146	        if (!_serv_stayopen)
25147	                endservent();
25148	        return (p);
25149	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/hton.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

25200	/*
25201	hton.c
25202	*/
25203	
25204	#include <sys/types.h>
25205	#include <minix/config.h>
25206	#include <net/hton.h>
25207	
25208	u16_t _tmp;
25209	u32_t _tmp_l;
.Op 229 src/lib/ip/inet_addr.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/inet_addr.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

25300	/*
25301	 * Copyright (c) 1983, 1990 Regents of the University of California.
25302	 * All rights reserved.
25303	 *
25304	 * Redistribution and use in source and binary forms are permitted provided
25305	 * that: (1) source distributions retain this entire copyright notice and
25306	 * comment, and (2) distributions including binaries display the following
25307	 * acknowledgement:  ``This product includes software developed by the
25308	 * University of California, Berkeley and its contributors'' in the
25309	 * documentation or other materials provided with the distribution and in
25310	 * all advertising materials mentioning features or use of this software.
25311	 * Neither the name of the University nor the names of its contributors may
25312	 * be used to endorse or promote products derived from this software without
25313	 * specific prior written permission.
25314	 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
25315	 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
25316	 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
25317	 */
25318	
25319	#if defined(LIBC_SCCS) && !defined(lint)
25320	static char sccsid[] = "@(#)inet_addr.c 5.8 (Berkeley) 6/23/90";
25321	#endif /* LIBC_SCCS and not lint */
25322	
25323	#if _MINIX
25324	#include <sys/types.h>
25325	#include <ctype.h>
25326	#include <errno.h>
25327	
25328	#include <net/hton.h>
25329	#include <net/gen/in.h>
25330	#include <net/gen/inet.h>
25331	#endif
25332	
25333	#ifdef __STDC__
25334	#define _CONST  const
25335	#else
25336	#define _CONST
25337	#endif
25338	
25339	/*
25340	 * Ascii internet address interpretation routine.
25341	 * The value returned is in network order.
25342	 */
25343	ipaddr_t
25344	inet_addr(cp)
25345	        register _CONST char *cp;
25346	{
25347	        ipaddr_t val;
25348	
25349	        if (inet_aton(cp, &val))
25350	                return (val);
25351	        errno= EINVAL;
25352	        return (ipaddr_t)-1;
25353	}
	
.Ep 230 src/lib/ip/inet_addr.c
25355	/* 
25356	 * Check whether "cp" is a valid ascii representation
25357	 * of an Internet address and convert to a binary address.
25358	 * Returns 1 if the address is valid, 0 if not.
25359	 * This replaces inet_addr, the return value from which
25360	 * cannot distinguish between failure and a local broadcast address.
25361	 */
25362	
25363	int
25364	inet_aton(cp, addr)
25365	        register _CONST char *cp;
25366	        ipaddr_t *addr;
25367	{
25368	        register u32_t val, base, n;
25369	        register char c;
25370	        u32_t parts[4], *pp = parts;
25371	
25372	        for (;;) {
25373	                /*
25374	                 * Collect number up to ``.''.
25375	                 * Values are specified as for C:
25376	                 * 0x=hex, 0=octal, other=decimal.
25377	                 */
25378	                val = 0; base = 10;
25379	                if (*cp == '0') {
25380	                        if (*++cp == 'x' || *cp == 'X')
25381	                                base = 16, cp++;
25382	                        else
25383	                                base = 8;
25384	                }
25385	                while ((c = *cp) != '\0') {
25386	                        if (isascii(c) && isdigit(c)) {
25387	                                val = (val * base) + (c - '0');
25388	                                cp++;
25389	                                continue;
25390	                        }
25391	                        if (base == 16 && isascii(c) && isxdigit(c)) {
25392	                                val = (val << 4) + 
25393	                                        (c + 10 - (islower(c) ? 'a' : 'A'));
25394	                                cp++;
25395	                                continue;
25396	                        }
25397	                        break;
25398	                }
25399	                if (*cp == '.') {
25400	                        /*
25401	                         * Internet format:
25402	                         *      a.b.c.d
25403	                         *      a.b.c   (with c treated as 16-bits)
25404	                         *      a.b     (with b treated as 24 bits)
25405	                         */
25406	                        if (pp >= parts + 3 || val > 0xff)
25407	                                return (0);
25408	                        *pp++ = val, cp++;
25409	                } else
25410	                        break;
25411	        }
25412	        /*
25413	         * Check for trailing characters.
25414	         */
.Op 231 src/lib/ip/inet_addr.c
25415	        if (*cp && (!isascii(*cp) || !isspace(*cp)))
25416	                return (0);
25417	        /*
25418	         * Concoct the address according to
25419	         * the number of parts specified.
25420	         */
25421	        n = pp - parts + 1;
25422	        switch (n) {
25423	
25424	        case 1:                         /* a -- 32 bits */
25425	                break;
25426	
25427	        case 2:                         /* a.b -- 8.24 bits */
25428	                if (val > 0xffffff)
25429	                        return (0);
25430	                val |= parts[0] << 24;
25431	                break;
25432	
25433	        case 3:                         /* a.b.c -- 8.8.16 bits */
25434	                if (val > 0xffff)
25435	                        return (0);
25436	                val |= (parts[0] << 24) | (parts[1] << 16);
25437	                break;
25438	
25439	        case 4:                         /* a.b.c.d -- 8.8.8.8 bits */
25440	                if (val > 0xff)
25441	                        return (0);
25442	                val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
25443	                break;
25444	        }
25445	        if (addr)
25446	                *addr = htonl(val);
25447	        return (1);
25448	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/inet_ntoa.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

25500	/*
25501	 * Copyright (c) 1983 Regents of the University of California.
25502	 * All rights reserved.
25503	 *
25504	 * Redistribution and use in source and binary forms are permitted
25505	 * provided that: (1) source distributions retain this entire copyright
25506	 * notice and comment, and (2) distributions including binaries display
25507	 * the following acknowledgement:  ``This product includes software
25508	 * developed by the University of California, Berkeley and its contributors''
25509	 * in the documentation or other materials provided with the distribution
25510	 * and in all advertising materials mentioning features or use of this
25511	 * software. Neither the name of the University nor the names of its
25512	 * contributors may be used to endorse or promote products derived
25513	 * from this software without specific prior written permission.
25514	 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
25515	 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
25516	 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
25517	 */
25518	
25519	#if defined(LIBC_SCCS) && !defined(lint)
.Ep 232 src/lib/ip/inet_ntoa.c
25520	static char sccsid[] = "@(#)inet_ntoa.c 5.5 (Berkeley) 6/1/90";
25521	#endif /* LIBC_SCCS and not lint */
25522	
25523	/*
25524	 * Convert network-format internet address
25525	 * to base 256 d.d.d.d representation.
25526	 */
25527	
25528	#include <sys/types.h>
25529	#include <stdio.h>
25530	
25531	#include <net/gen/in.h>
25532	#include <net/gen/inet.h>
25533	
25534	char *
25535	inet_ntoa(in)
25536	        ipaddr_t in;
25537	{
25538	        static char b[18];
25539	        register u8_t *p;
25540	
25541	        p = (u8_t *)&in;
25542	        sprintf(b, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
25543	        return (b);
25544	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/memcspn.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

25600	/*
25601	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
25602	 * See the copyright notice in the ACK home directory, in the file "Copyright".
25603	 */
25604	/* $Header: strcspn.c,v 1.1 91/12/19 13:20:40 philip Exp $ */
25605	
25606	#include        <string.h>
25607	
25608	size_t
25609	memcspn(const char *string, size_t strlen, const char *notin, size_t notinlen)
25610	{
25611	        register const char *s1, *s2;
25612	        int i,j;
25613	
25614	        for (s1 = string, i = 0; i<strlen; s1++, i++) {
25615	                for(s2 = notin, j = 0; *s2 != *s1 && j < notinlen; s2++, j++)
25616	                        /* EMPTY */ ;
25617	                if (j != notinlen)
25618	                        break;
25619	        }
25620	        return s1 - string;
25621	}
.Op 233 src/lib/ip/oneC_sum.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/oneC_sum.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

25700	/*      oneC_sum() - One complement's checksum          Author: Kees J. Bot
25701	 *                                                              8 May 1995
25702	 * See RFC 1071, "Computing the Internet checksum"
25703	 */
25704	
25705	#include <sys/types.h>
25706	#include <net/gen/oneCsum.h>
25707	
25708	u16_t oneC_sum(U16_t prev, void *data, size_t size)
25709	{
25710	        u8_t *dptr;
25711	        size_t n;
25712	        u16_t word;
25713	        u32_t sum;
25714	        int swap= 0;
25715	
25716	        sum= prev;
25717	        dptr= data;
25718	        n= size;
25719	
25720	        swap= ((size_t) dptr & 1);
25721	        if (swap) {
25722	                sum= ((sum & 0xFF) << 8) | ((sum & 0xFF00) >> 8);
25723	                if (n > 0) {
25724	                        ((u8_t *) &word)[0]= 0;
25725	                        ((u8_t *) &word)[1]= dptr[0];
25726	                        sum+= (u32_t) word;
25727	                        dptr+= 1;
25728	                        n-= 1;
25729	                }
25730	        }
25731	
25732	        while (n >= 8) {
25733	                sum+= (u32_t) ((u16_t *) dptr)[0]
25734	                    + (u32_t) ((u16_t *) dptr)[1]
25735	                    + (u32_t) ((u16_t *) dptr)[2]
25736	                    + (u32_t) ((u16_t *) dptr)[3];
25737	                dptr+= 8;
25738	                n-= 8;
25739	        }
25740	
25741	        while (n >= 2) {
25742	                sum+= (u32_t) ((u16_t *) dptr)[0];
25743	                dptr+= 2;
25744	                n-= 2;
25745	        }
25746	
25747	        if (n > 0) {
25748	                ((u8_t *) &word)[0]= dptr[0];
25749	                ((u8_t *) &word)[1]= 0;
25750	                sum+= (u32_t) word;
25751	        }
25752	
25753	        sum= (sum & 0xFFFF) + (sum >> 16);
25754	        if (sum > 0xFFFF) sum++;
.Ep 234 src/lib/ip/oneC_sum.c
25755	
25756	        if (swap) {
25757	                sum= ((sum & 0xFF) << 8) | ((sum & 0xFF00) >> 8);
25758	        }
25759	        return sum;
25760	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/rcmd.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

25800	/*
25801	 * Copyright (c) 1983 Regents of the University of California.
25802	 * All rights reserved.
25803	 *
25804	 * Redistribution and use in source and binary forms are permitted
25805	 * provided that: (1) source distributions retain this entire copyright
25806	 * notice and comment, and (2) distributions including binaries display
25807	 * the following acknowledgement:  ``This product includes software
25808	 * developed by the University of California, Berkeley and its contributors''
25809	 * in the documentation or other materials provided with the distribution
25810	 * and in all advertising materials mentioning features or use of this
25811	 * software. Neither the name of the University nor the names of its
25812	 * contributors may be used to endorse or promote products derived
25813	 * from this software without specific prior written permission.
25814	 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
25815	 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
25816	 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
25817	 */
25818	
25819	#if defined(LIBC_SCCS) && !defined(lint)
25820	static char sccsid[] = "@(#)rcmd.c      5.22 (Berkeley) 6/1/90";
25821	#endif /* LIBC_SCCS and not lint */
25822	
25823	#if _MINIX
25824	#include <sys/types.h>
25825	#include <sys/stat.h>
25826	#include <sys/ioctl.h>
25827	#include <ctype.h>
25828	#include <errno.h>
25829	#include <fcntl.h>
25830	#include <limits.h>
25831	#include <pwd.h>
25832	#include <stdio.h>
25833	#include <stdlib.h>
25834	#include <signal.h>
25835	#include <string.h>
25836	#include <unistd.h>
25837	#include <net/gen/netdb.h>
25838	#include <net/gen/in.h>
25839	#include <net/gen/tcp.h>
25840	#include <net/gen/tcp_io.h>
25841	#include <net/hton.h>
25842	#include <net/netlib.h>
25843	
25844	#define MAXHOSTNAMELEN  256
.Op 235 src/lib/ip/rcmd.c
25845	#define MAXPATHLEN PATH_MAX
25846	#else
25847	#include <stdio.h>
25848	#include <ctype.h>
25849	#include <pwd.h>
25850	#include <sys/param.h>
25851	#include <sys/file.h>
25852	#include <sys/signal.h>
25853	#include <sys/socket.h>
25854	#include <sys/stat.h>
25855	
25856	#include <netinet/in.h>
25857	
25858	#include <netdb.h>
25859	#include <errno.h>
25860	#endif
25861	
25862	#ifdef __STDC__
25863	#define CONST   const
25864	#else
25865	#define CONST
25866	#endif
25867	
25868	extern  errno;
25869	#if _MINIX
25870	int _validuser _ARGS(( FILE *hostf, const char *rhost, const char *luser,
25871	                                        const char *ruser, int baselen ));
25872	static int _checkhost _ARGS(( const char *rhost, const char *lhost, int len ));
25873	#else
25874	char    *index();
25875	#endif
25876	
25877	#if _MINIX
25878	int rcmd(ahost, rport, locuser, remuser, cmd, fd2p)
25879	char **ahost;
25880	int rport;
25881	CONST char *locuser, *remuser, *cmd;
25882	int *fd2p;
25883	{
25884	        int fd, fd2, result;
25885	        struct hostent *hp;
25886	        tcpport_t lport;
25887	        nwio_tcpconf_t tcpconf;
25888	        nwio_tcpcl_t tcpconnopt;
25889	        pid_t pid;
25890	        char num[8];
25891	        char c;
25892	        char *tcp_device;
25893	
25894	        fd= -1;
25895	        fd2= -1;
25896	
25897	        tcp_device= getenv("TCP_DEVICE");
25898	        if (tcp_device == NULL)
25899	                tcp_device= TCP_DEVICE;
25900	        hp= gethostbyname(*ahost);
25901	        if (!hp)
25902	        {
25903	                fprintf(stderr, "%s: unknown host\n", *ahost);
25904	                return -1;
.Ep 236 src/lib/ip/rcmd.c
25905	        }
25906	        *ahost= hp->h_name;
25907	        for (lport= TCPPORT_RESERVED-1; lport >= TCPPORT_RESERVED/2; lport--)
25908	        {
25909	                fd= open (tcp_device, O_RDWR);
25910	                if (fd<0)
25911	                {
25912	                        fprintf(stderr, "unable to open %s: %s\n",
25913	                                tcp_device, strerror(errno));
25914	                        goto bad;
25915	                }
25916	                tcpconf.nwtc_flags= NWTC_LP_SET | NWTC_SET_RA | NWTC_SET_RP |
25917	                        NWTC_EXCL;
25918	                tcpconf.nwtc_locport= htons(lport);
25919	                tcpconf.nwtc_remport= rport;
25920	                tcpconf.nwtc_remaddr= *(ipaddr_t *)hp->h_addr;
25921	
25922	                result= ioctl(fd, NWIOSTCPCONF, &tcpconf);
25923	                if (result<0)
25924	                {
25925	                        if (errno == EADDRINUSE)
25926	                        {
25927	                                close(fd);
25928	                                continue;
25929	                        }
25930	                        fprintf(stderr, "unable to ioctl(NWIOSTCPCONF): %s\n",
25931	                                strerror(errno));
25932	                        goto bad;
25933	                }
25934	                tcpconf.nwtc_flags= NWTC_SHARED;
25935	                result= ioctl(fd, NWIOSTCPCONF, &tcpconf);
25936	                if (result<0)
25937	                {
25938	                        fprintf(stderr, "unable to ioctl(NWIOSTCPCONF): %s\n",
25939	                                strerror(errno));
25940	                        goto bad;
25941	                }
25942	                tcpconnopt.nwtcl_flags= 0;
25943	
25944	                do
25945	                {
25946	                        result= ioctl (fd, NWIOTCPCONN, &tcpconnopt);
25947	                        if (result<0 && errno == EAGAIN)
25948	                        {
25949	                                sleep(2);
25950	                        }
25951	                } while (result<0 && errno == EAGAIN);
25952	                if (result<0 && errno != EADDRINUSE)
25953	                {
25954	                        fprintf(stderr,
25955	                                "unable to ioctl(NWIOTCPCONN): %s\n",
25956	                                strerror(errno));
25957	                        goto bad;
25958	                }
25959	                if (result>=0)
25960	                        break;
25961	        }
25962	        if (lport<TCPPORT_RESERVED/2)
25963	        {
25964	                fprintf(stderr, "can't get port\n");
.Op 237 src/lib/ip/rcmd.c
25965	                return -1;
25966	        }
25967	        if (!fd2p)
25968	        {
25969	                if (write(fd, "", 1) != 1)
25970	                {
25971	                        fprintf(stderr, "unable to write: %s", strerror(errno));
25972	                        goto bad;
25973	                }
25974	        }
25975	        else
25976	        {
25977	                fd2= open (tcp_device, O_RDWR);
25978	                if (fd2<0)
25979	                {
25980	                        fprintf(stderr, "unable to open %s: %s\n",
25981	                                tcp_device, strerror(errno));
25982	                        goto bad;
25983	                }
25984	                tcpconf.nwtc_flags= NWTC_LP_SET | NWTC_UNSET_RA | 
25985	                        NWTC_UNSET_RP | NWTC_SHARED;
25986	                tcpconf.nwtc_locport= htons(lport);
25987	
25988	                result= ioctl(fd2, NWIOSTCPCONF, &tcpconf);
25989	                if (result<0)
25990	                {
25991	                        fprintf(stderr,
25992	                                "unable to ioctl(NWIOSTCPCONF): %s\n",
25993	                                strerror(errno));
25994	                        goto bad;
25995	                }
25996	                pid= fork();
25997	                if (pid<0)
25998	                {
25999	                        fprintf(stderr, "unable to fork: %s\n",
26000	                                strerror(errno));
26001	                        goto bad;
26002	                }
26003	                if (!pid)
26004	                {
26005	                        alarm(0);
26006	                        signal(SIGALRM, SIG_DFL);
26007	                        alarm(30); /* give up after half a minute */
26008	                        tcpconnopt.nwtcl_flags= 0;
26009	
26010	                        do
26011	                        {
26012	                                result= ioctl (fd2, NWIOTCPLISTEN,
26013	                                        &tcpconnopt);
26014	                                if (result<0 && errno == EAGAIN)
26015	                                {
26016	                                        sleep(2);
26017	                                }
26018	                        } while (result<0 && errno == EAGAIN);
26019	                        if (result<0 && errno != EADDRINUSE)
26020	                        {
26021	                                fprintf(stderr,
26022	                                        "unable to ioctl(NWIOTCPLISTEN): %s\n",
26023	                                        strerror(errno));
26024	                                exit(1);
.Ep 238 src/lib/ip/rcmd.c
26025	                        }
26026	                        if (result>=0)
26027	                                exit(0);
26028	                        else
26029	                                exit(1);
26030	                }
26031	                /*
26032	                 * This sleep is a HACK.  The command that we are starting
26033	                 * will try to connect to the fd2 port.  It seems that for
26034	                 * this to succeed the child process must have already made
26035	                 * the call to ioctl above (the NWIOTCPLISTEN) call.
26036	                 * The sleep gives the child a chance to make the call
26037	                 * before the parent sends the port number to the
26038	                 * command being started.
26039	                 */
26040	                sleep(1);
26041	
26042	                sprintf(num, "%d", lport);
26043	                if (write(fd, num, strlen(num)+1) != strlen(num)+1)
26044	                {
26045	                        fprintf(stderr, "unable to write: %s\n",
26046	                                strerror(errno));
26047	                        goto bad;
26048	                }
26049	
26050	        }
26051	        write (fd, locuser, strlen(locuser)+1);
26052	        write (fd, remuser, strlen(remuser)+1);
26053	        write (fd, cmd, strlen(cmd)+1);
26054	        if (read(fd, &c, 1) != 1)
26055	        {
26056	                fprintf(stderr, "unable to read: %s\n", strerror(errno) );
26057	                goto bad;
26058	        }
26059	        if (c != 0)
26060	        {
26061	                while (read(fd, &c, 1) == 1)
26062	                {
26063	                        write(2, &c, 1);
26064	                        if (c == '\n')
26065	                                break;
26066	                }
26067	                goto bad;
26068	        }
26069	        if (fd2p)
26070	        {
26071	                *fd2p= fd2;
26072	                result= ioctl(fd2, NWIOGTCPCONF, &tcpconf);
26073	                if (result<0)
26074	                {
26075	                        fprintf(stderr, "unable to ioctl(NWIOGTCPCONF): %s\n",
26076	                                strerror(errno) );
26077	                        goto bad;
26078	                }
26079	                if (ntohs(tcpconf.nwtc_remport) >= TCPPORT_RESERVED)
26080	                {
26081	                        fprintf(stderr, "unable to setup 2nd channel\n");
26082	                        goto bad;
26083	                }
26084	        }
.Op 239 src/lib/ip/rcmd.c
26085	        return fd;
26086	
26087	bad:
26088	        if (fd>=0)
26089	                close(fd);
26090	        if (fd2>=0)
26091	                close(fd2);
26092	        return -1;
26093	}
26094	#else /* _MINIX */
26095	rcmd(ahost, rport, locuser, remuser, cmd, fd2p)
26096	        char **ahost;
26097	        u_short rport;
26098	        char *locuser, *remuser, *cmd;
26099	        int *fd2p;
26100	{
26101	        int s, timo = 1, pid;
26102	        long oldmask;
26103	        struct sockaddr_in sin, sin2, from;
26104	        char c;
26105	        int lport = IPPORT_RESERVED - 1;
26106	        struct hostent *hp;
26107	        fd_set reads;
26108	
26109	        pid = getpid();
26110	        hp = gethostbyname(*ahost);
26111	        if (hp == 0) {
26112	                herror(*ahost);
26113	                return (-1);
26114	        }
26115	        *ahost = hp->h_name;
26116	        oldmask = sigblock(sigmask(SIGURG));
26117	        for (;;) {
26118	                s = rresvport(&lport);
26119	                if (s < 0) {
26120	                        if (errno == EAGAIN)
26121	                                fprintf(stderr, "socket: All ports in use\n");
26122	                        else
26123	                                perror("rcmd: socket");
26124	                        sigsetmask(oldmask);
26125	                        return (-1);
26126	                }
26127	                fcntl(s, F_SETOWN, pid);
26128	                sin.sin_family = hp->h_addrtype;
26129	                bcopy(hp->h_addr_list[0], (caddr_t)&sin.sin_addr, hp->h_length);
26130	                sin.sin_port = rport;
26131	                if (connect(s, (caddr_t)&sin, sizeof (sin), 0) >= 0)
26132	                        break;
26133	                (void) close(s);
26134	                if (errno == EADDRINUSE) {
26135	                        lport--;
26136	                        continue;
26137	                }
26138	                if (errno == ECONNREFUSED && timo <= 16) {
26139	                        sleep(timo);
26140	                        timo *= 2;
26141	                        continue;
26142	                }
26143	                if (hp->h_addr_list[1] != NULL) {
26144	                        int oerrno = errno;
.Ep 240 src/lib/ip/rcmd.c
26145	
26146	                        fprintf(stderr,
26147	                            "connect to address %s: ", inet_ntoa(sin.sin_addr));
26148	                        errno = oerrno;
26149	                        perror(0);
26150	                        hp->h_addr_list++;
26151	                        bcopy(hp->h_addr_list[0], (caddr_t)&sin.sin_addr,
26152	                            hp->h_length);
26153	                        fprintf(stderr, "Trying %s...\n",
26154	                                inet_ntoa(sin.sin_addr));
26155	                        continue;
26156	                }
26157	                perror(hp->h_name);
26158	                sigsetmask(oldmask);
26159	                return (-1);
26160	        }
26161	        lport--;
26162	        if (fd2p == 0) {
26163	                write(s, "", 1);
26164	                lport = 0;
26165	        } else {
26166	                char num[8];
26167	                int s2 = rresvport(&lport), s3;
26168	                int len = sizeof (from);
26169	
26170	                if (s2 < 0)
26171	                        goto bad;
26172	                listen(s2, 1);
26173	                (void) sprintf(num, "%d", lport);
26174	                if (write(s, num, strlen(num)+1) != strlen(num)+1) {
26175	                        perror("write: setting up stderr");
26176	                        (void) close(s2);
26177	                        goto bad;
26178	                }
26179	                FD_ZERO(&reads);
26180	                FD_SET(s, &reads);
26181	                FD_SET(s2, &reads);
26182	                errno = 0;
26183	                if (select(32, &reads, 0, 0, 0) < 1 ||
26184	                    !FD_ISSET(s2, &reads)) {
26185	                        if (errno != 0)
26186	                                perror("select: setting up stderr");
26187	                        else
26188	                            fprintf(stderr,
26189	                                "select: protocol failure in circuit setup.\n");
26190	                        (void) close(s2);
26191	                        goto bad;
26192	                }
26193	                s3 = accept(s2, &from, &len, 0);
26194	                (void) close(s2);
26195	                if (s3 < 0) {
26196	                        perror("accept");
26197	                        lport = 0;
26198	                        goto bad;
26199	                }
26200	                *fd2p = s3;
26201	                from.sin_port = ntohs((u_short)from.sin_port);
26202	                if (from.sin_family != AF_INET ||
26203	                    from.sin_port >= IPPORT_RESERVED ||
26204	                    from.sin_port < IPPORT_RESERVED / 2) {
.Op 241 src/lib/ip/rcmd.c
26205	                        fprintf(stderr,
26206	                            "socket: protocol failure in circuit setup.\n");
26207	                        goto bad2;
26208	                }
26209	        }
26210	        (void) write(s, locuser, strlen(locuser)+1);
26211	        (void) write(s, remuser, strlen(remuser)+1);
26212	        (void) write(s, cmd, strlen(cmd)+1);
26213	        if (read(s, &c, 1) != 1) {
26214	                perror(*ahost);
26215	                goto bad2;
26216	        }
26217	        if (c != 0) {
26218	                while (read(s, &c, 1) == 1) {
26219	                        (void) write(2, &c, 1);
26220	                        if (c == '\n')
26221	                                break;
26222	                }
26223	                goto bad2;
26224	        }
26225	        sigsetmask(oldmask);
26226	        return (s);
26227	bad2:
26228	        if (lport)
26229	                (void) close(*fd2p);
26230	bad:
26231	        (void) close(s);
26232	        sigsetmask(oldmask);
26233	        return (-1);
26234	}
	
26236	rresvport(alport)
26237	        int *alport;
26238	{
26239	        struct sockaddr_in sin;
26240	        int s;
26241	
26242	        sin.sin_family = AF_INET;
26243	        sin.sin_addr.s_addr = INADDR_ANY;
26244	        s = socket(AF_INET, SOCK_STREAM, 0);
26245	        if (s < 0)
26246	                return (-1);
26247	        for (;;) {
26248	                sin.sin_port = htons((u_short)*alport);
26249	                if (bind(s, (caddr_t)&sin, sizeof (sin)) >= 0)
26250	                        return (s);
26251	                if (errno != EADDRINUSE) {
26252	                        (void) close(s);
26253	                        return (-1);
26254	                }
26255	                (*alport)--;
26256	                if (*alport == IPPORT_RESERVED/2) {
26257	                        (void) close(s);
26258	                        errno = EAGAIN;         /* close */
26259	                        return (-1);
26260	                }
26261	        }
26262	}
26263	#endif /* _MINIX */
26264	
.Ep 242 src/lib/ip/rcmd.c
26265	int     _check_rhosts_file = 1;
26266	
26267	ruserok(rhost, superuser, ruser, luser)
26268	        CONST char *rhost;
26269	        int superuser;
26270	        CONST char *ruser, *luser;
26271	{
26272	        FILE *hostf;
26273	        char fhost[MAXHOSTNAMELEN];
26274	        int first = 1;
26275	        register CONST char *sp;
26276	        register char *p;
26277	        int baselen = -1;
26278	
26279	        sp = rhost;
26280	        p = fhost;
26281	        while (*sp) {
26282	                if (*sp == '.') {
26283	                        if (baselen == -1)
26284	                                baselen = sp - rhost;
26285	                        *p++ = *sp++;
26286	                } else {
26287	                        *p++ = isupper(*sp) ? tolower(*sp++) : *sp++;
26288	                }
26289	        }
26290	        *p = '\0';
26291	        hostf = superuser ? (FILE *)0 : fopen(_PATH_HEQUIV, "r");
26292	again:
26293	        if (hostf) {
26294	                if (!_validuser(hostf, fhost, luser, ruser, baselen)) {
26295	                        (void) fclose(hostf);
26296	                        return(0);
26297	                }
26298	                (void) fclose(hostf);
26299	        }
26300	        if (first == 1 && (_check_rhosts_file || superuser)) {
26301	                struct stat sbuf;
26302	                struct passwd *pwd;
26303	                char pbuf[MAXPATHLEN];
26304	
26305	                first = 0;
26306	                if ((pwd = getpwnam(luser)) == NULL)
26307	                        return(-1);
26308	                (void)strcpy(pbuf, pwd->pw_dir);
26309	                (void)strcat(pbuf, "/.rhosts");
26310	                if ((hostf = fopen(pbuf, "r")) == NULL)
26311	                        return(-1);
26312	                /*
26313	                 * if owned by someone other than user or root or if
26314	                 * writeable by anyone but the owner, quit
26315	                 */
26316	                if (fstat(fileno(hostf), &sbuf) ||
26317	                    sbuf.st_uid && sbuf.st_uid != pwd->pw_uid ||
26318	                    sbuf.st_mode&022) {
26319	                        fclose(hostf);
26320	                        return(-1);
26321	                }
26322	                goto again;
26323	        }
26324	        return (-1);
.Op 243 src/lib/ip/rcmd.c
26325	}
	
26327	/* don't make static, used by lpd(8) */
26328	int _validuser(hostf, rhost, luser, ruser, baselen)
26329	        FILE *hostf;
26330	        CONST char *rhost, *luser, *ruser;
26331	        int baselen;
26332	{
26333	        char *user;
26334	        char ahost[MAXHOSTNAMELEN];
26335	        register char *p;
26336	
26337	        while (fgets(ahost, sizeof (ahost), hostf)) {
26338	                p = ahost;
26339	                while (*p != '\n' && *p != ' ' && *p != '\t' && *p != '\0') {
26340	                        *p = isupper(*p) ? tolower(*p) : *p;
26341	                        p++;
26342	                }
26343	                if (*p == ' ' || *p == '\t') {
26344	                        *p++ = '\0';
26345	                        while (*p == ' ' || *p == '\t')
26346	                                p++;
26347	                        user = p;
26348	                        while (*p != '\n' && *p != ' ' && *p != '\t' && *p != '\0')
26349	                                p++;
26350	                } else
26351	                        user = p;
26352	                *p = '\0';
26353	                if (_checkhost(rhost, ahost, baselen) &&
26354	                    !strcmp(ruser, *user ? user : luser)) {
26355	                        return (0);
26356	                }
26357	        }
26358	        return (-1);
26359	}
	
26361	static int
26362	_checkhost(rhost, lhost, len)
26363	        CONST char *rhost, *lhost;
26364	        int len;
26365	{
26366	        static char ldomain[MAXHOSTNAMELEN + 1];
26367	        static char *domainp = NULL;
26368	        static int nodomain = 0;
26369	        register char *cp;
26370	
26371	        if (len == -1)
26372	                return(!strcmp(rhost, lhost));
26373	        if (strncmp(rhost, lhost, len))
26374	                return(0);
26375	        if (!strcmp(rhost, lhost))
26376	                return(1);
26377	        if (*(lhost + len) != '\0')
26378	                return(0);
26379	        if (nodomain)
26380	                return(0);
26381	        if (!domainp) {
26382	                if (gethostname(ldomain, sizeof(ldomain)) == -1) {
26383	                        nodomain = 1;
26384	                        return(0);
.Ep 244 src/lib/ip/rcmd.c
26385	                }
26386	                ldomain[MAXHOSTNAMELEN] = 0;
26387	                if ((domainp = index(ldomain, '.')) == (char *)NULL) {
26388	                        nodomain = 1;
26389	                        return(0);
26390	                }
26391	                for (cp = ++domainp; *cp; ++cp)
26392	                        if (isupper(*cp))
26393	                                *cp = tolower(*cp);
26394	        }
26395	        return(!strcmp(domainp, rhost + len +1));
26396	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/res_comp.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

26400	/*
26401	 * Copyright (c) 1985 Regents of the University of California.
26402	 * All rights reserved.
26403	 *
26404	 * Redistribution and use in source and binary forms are permitted provided
26405	 * that: (1) source distributions retain this entire copyright notice and
26406	 * comment, and (2) distributions including binaries display the following
26407	 * acknowledgement:  ``This product includes software developed by the
26408	 * University of California, Berkeley and its contributors'' in the
26409	 * documentation or other materials provided with the distribution and in
26410	 * all advertising materials mentioning features or use of this software.
26411	 * Neither the name of the University nor the names of its contributors may
26412	 * be used to endorse or promote products derived from this software without
26413	 * specific prior written permission.
26414	 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
26415	 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
26416	 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
26417	 */
26418	
26419	#if defined(LIBC_SCCS) && !defined(lint)
26420	static char sccsid[] = "@(#)res_comp.c  6.18 (Berkeley) 6/27/90";
26421	#endif /* LIBC_SCCS and not lint */
26422	
26423	#if _MINIX
26424	#include <sys/types.h>
26425	#include <stdlib.h>
26426	
26427	#include <net/gen/in.h>
26428	#include <net/gen/nameser.h>
26429	#include <net/gen/resolv.h>
26430	
26431	typedef u8_t u_char;
26432	typedef u16_t u_short;
26433	typedef u32_t u_long;
26434	
26435	static int dn_find _ARGS(( const u_char *exp_dn, const u_char *msg,
26436	        u_char **dnptrs, u_char **lastdnptr ));
26437	int dn_skipname _ARGS(( const u_char *comp_dn, const u_char *eom ));
26438	
26439	#define getshort _getshort
.Op 245 src/lib/ip/res_comp.c
26440	#define getlong _getlong
26441	#define putshort __putshort
26442	#define putlong __putlong
26443	#else
26444	#include <sys/types.h>
26445	#include <stdio.h>
26446	#include <arpa/nameser.h>
26447	
26448	static dn_find();
26449	#endif
26450	
26451	#ifdef __STDC__
26452	#define CONST   const
26453	#else
26454	#define CONST
26455	#endif
26456	
26457	/*
26458	 * Expand compressed domain name 'comp_dn' to full domain name.
26459	 * 'msg' is a pointer to the begining of the message,
26460	 * 'eomorig' points to the first location after the message,
26461	 * 'exp_dn' is a pointer to a buffer of size 'length' for the result.
26462	 * Return size of compressed name or -1 if there was an error.
26463	 */
26464	dn_expand(msg, eomorig, comp_dn, exp_dn, length)
26465	        CONST u_char *msg, *eomorig, *comp_dn;
26466	        u_char *exp_dn;
26467	        int length;
26468	{
26469	        register CONST u_char *cp;
26470	        register u_char *dn;
26471	        register int n, c;
26472	        CONST u_char *eom;
26473	        int len = -1, checked = 0;
26474	
26475	        dn = exp_dn;
26476	        cp = comp_dn;
26477	        eom = exp_dn + length;
26478	        /*
26479	         * fetch next label in domain name
26480	         */
26481	        while (n = *cp++) {
26482	                /*
26483	                 * Check for indirection
26484	                 */
26485	                switch (n & INDIR_MASK) {
26486	                case 0:
26487	                        if (dn != exp_dn) {
26488	                                if (dn >= eom)
26489	                                        return (-1);
26490	                                *dn++ = '.';
26491	                        }
26492	                        if (dn+n >= eom)
26493	                                return (-1);
26494	                        checked += n + 1;
26495	                        while (--n >= 0) {
26496	                                if ((c = *cp++) == '.') {
26497	                                        if (dn + n + 2 >= eom)
26498	                                                return (-1);
26499	                                        *dn++ = '\\';
.Ep 246 src/lib/ip/res_comp.c
26500	                                }
26501	                                *dn++ = c;
26502	                                if (cp >= eomorig)      /* out of range */
26503	                                        return(-1);
26504	                        }
26505	                        break;
26506	
26507	                case INDIR_MASK:
26508	                        if (len < 0)
26509	                                len = cp - comp_dn + 1;
26510	                        cp = msg + (((n & 0x3f) << 8) | (*cp & 0xff));
26511	                        if (cp < msg || cp >= eomorig)  /* out of range */
26512	                                return(-1);
26513	                        checked += 2;
26514	                        /*
26515	                         * Check for loops in the compressed name;
26516	                         * if we've looked at the whole message,
26517	                         * there must be a loop.
26518	                         */
26519	                        if (checked >= eomorig - msg)
26520	                                return (-1);
26521	                        break;
26522	
26523	                default:
26524	                        return (-1);                    /* flag error */
26525	                }
26526	        }
26527	        *dn = '\0';
26528	        if (len < 0)
26529	                len = cp - comp_dn;
26530	        return (len);
26531	}
	
26533	/*
26534	 * Compress domain name 'exp_dn' into 'comp_dn'.
26535	 * Return the size of the compressed name or -1.
26536	 * 'length' is the size of the array pointed to by 'comp_dn'.
26537	 * 'dnptrs' is a list of pointers to previous compressed names. dnptrs[0]
26538	 * is a pointer to the beginning of the message. The list ends with NULL.
26539	 * 'lastdnptr' is a pointer to the end of the arrary pointed to
26540	 * by 'dnptrs'. Side effect is to update the list of pointers for
26541	 * labels inserted into the message as we compress the name.
26542	 * If 'dnptr' is NULL, we don't try to compress names. If 'lastdnptr'
26543	 * is NULL, we don't update the list.
26544	 */
26545	int
26546	dn_comp(exp_dn, comp_dn, length, dnptrs, lastdnptr)
26547	        CONST u_char *exp_dn;
26548	        u_char *comp_dn;
26549	        int length;
26550	        u_char **dnptrs, **lastdnptr;
26551	{
26552	        register u_char *cp;
26553	        register CONST u_char *dn;
26554	        register int c, l;
26555	        u_char **cpp, **lpp, *sp, *eob;
26556	        u_char *msg;
26557	
26558	        dn = exp_dn;
26559	        cp = comp_dn;
.Op 247 src/lib/ip/res_comp.c
26560	        eob = cp + length;
26561	        if (dnptrs != NULL) {
26562	                if ((msg = *dnptrs++) != NULL) {
26563	                        for (cpp = dnptrs; *cpp != NULL; cpp++)
26564	                                ;
26565	                        lpp = cpp;      /* end of list to search */
26566	                }
26567	        } else
26568	                msg = NULL;
26569	        for (c = *dn++; c != '\0'; ) {
26570	                /* look to see if we can use pointers */
26571	                if (msg != NULL) {
26572	                        if ((l = dn_find(dn-1, msg, dnptrs, lpp)) >= 0) {
26573	                                if (cp+1 >= eob)
26574	                                        return (-1);
26575	                                *cp++ = (l >> 8) | INDIR_MASK;
26576	                                *cp++ = l % 256;
26577	                                return (cp - comp_dn);
26578	                        }
26579	                        /* not found, save it */
26580	                        if (lastdnptr != NULL && cpp < lastdnptr-1) {
26581	                                *cpp++ = cp;
26582	                                *cpp = NULL;
26583	                        }
26584	                }
26585	                sp = cp++;      /* save ptr to length byte */
26586	                do {
26587	                        if (c == '.') {
26588	                                c = *dn++;
26589	                                break;
26590	                        }
26591	                        if (c == '\\') {
26592	                                if ((c = *dn++) == '\0')
26593	                                        break;
26594	                        }
26595	                        if (cp >= eob) {
26596	                                if (msg != NULL)
26597	                                        *lpp = NULL;
26598	                                return (-1);
26599	                        }
26600	                        *cp++ = c;
26601	                } while ((c = *dn++) != '\0');
26602	                /* catch trailing '.'s but not '..' */
26603	                if ((l = cp - sp - 1) == 0 && c == '\0') {
26604	                        cp--;
26605	                        break;
26606	                }
26607	                if (l <= 0 || l > MAXLABEL) {
26608	                        if (msg != NULL)
26609	                                *lpp = NULL;
26610	                        return (-1);
26611	                }
26612	                *sp = l;
26613	        }
26614	        if (cp >= eob) {
26615	                if (msg != NULL)
26616	                        *lpp = NULL;
26617	                return (-1);
26618	        }
26619	        *cp++ = '\0';
.Ep 248 src/lib/ip/res_comp.c
26620	        return (cp - comp_dn);
26621	}
	
26623	/*
26624	 * Skip over a compressed domain name. Return the size or -1.
26625	 */
26626	dn_skipname(comp_dn, eom)
26627	        CONST u_char *comp_dn, *eom;
26628	{
26629	        register CONST u_char *cp;
26630	        register int n;
26631	
26632	        cp = comp_dn;
26633	        while (cp < eom && (n = *cp++)) {
26634	                /*
26635	                 * check for indirection
26636	                 */
26637	                switch (n & INDIR_MASK) {
26638	                case 0:         /* normal case, n == len */
26639	                        cp += n;
26640	                        continue;
26641	                default:        /* illegal type */
26642	                        return (-1);
26643	                case INDIR_MASK:        /* indirection */
26644	                        cp++;
26645	                }
26646	                break;
26647	        }
26648	        return (cp - comp_dn);
26649	}
	
26651	/*
26652	 * Search for expanded name from a list of previously compressed names.
26653	 * Return the offset from msg if found or -1.
26654	 * dnptrs is the pointer to the first name on the list,
26655	 * not the pointer to the start of the message.
26656	 */
26657	static int
26658	dn_find(exp_dn, msg, dnptrs, lastdnptr)
26659	        CONST u_char *exp_dn, *msg;
26660	        u_char **dnptrs, **lastdnptr;
26661	{
26662	        CONST register u_char *dn, *cp;
26663	        register u_char **cpp;
26664	        register int n;
26665	        CONST u_char *sp;
26666	
26667	        for (cpp = dnptrs; cpp < lastdnptr; cpp++) {
26668	                dn = exp_dn;
26669	                sp = cp = *cpp;
26670	                while (n = *cp++) {
26671	                        /*
26672	                         * check for indirection
26673	                         */
26674	                        switch (n & INDIR_MASK) {
26675	                        case 0:         /* normal case, n == len */
26676	                                while (--n >= 0) {
26677	                                        if (*dn == '.')
26678	                                                goto next;
26679	                                        if (*dn == '\\')
.Op 249 src/lib/ip/res_comp.c
26680	                                                dn++;
26681	                                        if (*dn++ != *cp++)
26682	                                                goto next;
26683	                                }
26684	                                if ((n = *dn++) == '\0' && *cp == '\0')
26685	                                        return (sp - msg);
26686	                                if (n == '.')
26687	                                        continue;
26688	                                goto next;
26689	
26690	                        default:        /* illegal type */
26691	                                return (-1);
26692	
26693	                        case INDIR_MASK:        /* indirection */
26694	                                cp = msg + (((n & 0x3f) << 8) | *cp);
26695	                        }
26696	                }
26697	                if (*dn == '\0')
26698	                        return (sp - msg);
26699	        next:   ;
26700	        }
26701	        return (-1);
26702	}
	
26704	/*
26705	 * Routines to insert/extract short/long's. Must account for byte
26706	 * order and non-alignment problems. This code at least has the
26707	 * advantage of being portable.
26708	 *
26709	 * used by sendmail.
26710	 */
26711	
26712	u16_t
26713	getshort(msgp)
26714	        CONST u8_t *msgp;
26715	{
26716	        return ((msgp[0] << 8) | (msgp[1] << 0));
26717	}
	
26719	u32_t
26720	getlong(msgp)
26721	        CONST u8_t *msgp;
26722	{
26723	        return (  ((u32_t) msgp[0] << 24)
26724	                | ((u32_t) msgp[1] << 16)
26725	                | ((u32_t) msgp[2] <<  8)
26726	                | ((u32_t) msgp[3] <<  0));
26727	}
	
	
26730	void
26731	putshort(s, msgp)
26732	        register U16_t s;
26733	        register u8_t *msgp;
26734	{
26735	
26736	        msgp[1] = s;
26737	        msgp[0] = s >> 8;
26738	}
	
.Ep 250 src/lib/ip/res_comp.c
26740	void
26741	putlong(l, msgp)
26742	        register u32_t l;
26743	        register u8_t *msgp;
26744	{
26745	
26746	        msgp[3] = l;
26747	        msgp[2] = (l >>= 8);
26748	        msgp[1] = (l >>= 8);
26749	        msgp[0] = l >> 8;
26750	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/res_init.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

26800	/*-
26801	 * Copyright (c) 1985, 1989 Regents of the University of California.
26802	 * All rights reserved.
26803	 *
26804	 * Redistribution and use in source and binary forms are permitted provided
26805	 * that: (1) source distributions retain this entire copyright notice and
26806	 * comment, and (2) distributions including binaries display the following
26807	 * acknowledgement:  ``This product includes software developed by the
26808	 * University of California, Berkeley and its contributors'' in the
26809	 * documentation or other materials provided with the distribution and in
26810	 * all advertising materials mentioning features or use of this software.
26811	 * Neither the name of the University nor the names of its contributors may
26812	 * be used to endorse or promote products derived from this software without
26813	 * specific prior written permission.
26814	 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
26815	 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
26816	 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
26817	 */
26818	
26819	#if defined(LIBC_SCCS) && !defined(lint)
26820	static char sccsid[] = "@(#)res_init.c  6.14 (Berkeley) 6/27/90";
26821	#endif /* LIBC_SCCS and not lint */
26822	
26823	#if _MINIX
26824	#include <sys/types.h>
26825	#include <stdio.h>
26826	#include <stdlib.h>
26827	#include <string.h>
26828	#include <unistd.h>
26829	
26830	#include <net/hton.h>
26831	#include <net/gen/in.h>
26832	#include <net/gen/inet.h>
26833	#include <net/gen/nameser.h>
26834	#include <net/gen/netdb.h>
26835	#include <net/gen/resolv.h>
26836	#include <net/gen/socket.h>
26837	
26838	#define index(s,c) strchr(s,c)
26839	#else
.Op 251 src/lib/ip/res_init.c
26840	#include <sys/types.h>
26841	#include <sys/socket.h>
26842	#include <netinet/in.h>
26843	#include <stdio.h>
26844	#include <arpa/nameser.h>
26845	#include <resolv.h>
26846	#endif
26847	
26848	/*
26849	 * Resolver state
26850	 */
26851	struct state _res;
26852	
26853	/*
26854	 * Set up default settings.  If the configuration file exist, the values
26855	 * there will have precedence.  Otherwise, the server address is set to
26856	 * 127.0.0.1 (localhost) and the default domain name comes from gethostname().
26857	 *
26858	 * The configuration file should only be used if you want to redefine your
26859	 * domain or run without a server on your machine.
26860	 *
26861	 * Return 0 if completes successfully, -1 on error
26862	 */
26863	int
26864	res_init()
26865	{
26866	        register FILE *fp;
26867	        register char *cp, **pp;
26868	        register int n;
26869	        char buf[BUFSIZ];
26870	        int haveenv = 0;
26871	        int havesearch = 0;
26872	        struct servent* servent;
26873	        u16_t nameserver_port;
26874	
26875	        /* Resolver state default settings */
26876	        _res.retrans = RES_TIMEOUT;     /* retransmition time interval */
26877	        _res.retry = 4;                 /* number of times to retransmit */
26878	        _res.options = RES_DEFAULT;     /* options flags */
26879	        _res.nscount = 0;               /* number of name servers */
26880	
26881	        servent= getservbyname("domain", NULL);
26882	        if (!servent)
26883	        {
26884	                h_errno= NO_RECOVERY;
26885	                return -1;
26886	        }
26887	        nameserver_port= servent->s_port;
26888	
26889	        /* Allow user to override the local domain definition */
26890	        if ((cp = getenv("LOCALDOMAIN")) != NULL) {
26891	                (void)strncpy(_res.defdname, cp, sizeof(_res.defdname));
26892	                haveenv++;
26893	        }
26894	
26895	        if ((fp = fopen(_PATH_RESCONF, "r")) != NULL) {
26896	            /* read the config file */
26897	            while (fgets(buf, sizeof(buf), fp) != NULL) {
26898	                /* read default domain name */
26899	                if (!strncmp(buf, "domain", sizeof("domain") - 1)) {
.Ep 252 src/lib/ip/res_init.c
26900	                    if (haveenv)        /* skip if have from environ */
26901	                            continue;
26902	                    cp = buf + sizeof("domain") - 1;
26903	                    while (*cp == ' ' || *cp == '\t')
26904	                            cp++;
26905	                    if ((*cp == '\0') || (*cp == '\n'))
26906	                            continue;
26907	                    (void)strncpy(_res.defdname, cp, sizeof(_res.defdname) - 1);
26908	                    if ((cp = index(_res.defdname, '\n')) != NULL)
26909	                            *cp = '\0';
26910	                    havesearch = 0;
26911	                    continue;
26912	                }
26913	                /* set search list */
26914	                if (!strncmp(buf, "search", sizeof("search") - 1)) {
26915	                    if (haveenv)        /* skip if have from environ */
26916	                            continue;
26917	                    cp = buf + sizeof("search") - 1;
26918	                    while (*cp == ' ' || *cp == '\t')
26919	                            cp++;
26920	                    if ((*cp == '\0') || (*cp == '\n'))
26921	                            continue;
26922	                    (void)strncpy(_res.defdname, cp, sizeof(_res.defdname) - 1);
26923	                    if ((cp = index(_res.defdname, '\n')) != NULL)
26924	                            *cp = '\0';
26925	                    /*
26926	                     * Set search list to be blank-separated strings
26927	                     * on rest of line.
26928	                     */
26929	                    cp = _res.defdname;
26930	                    pp = _res.dnsrch;
26931	                    *pp++ = cp;
26932	                    for (n = 0; *cp && pp < _res.dnsrch + MAXDNSRCH; cp++) {
26933	                            if (*cp == ' ' || *cp == '\t') {
26934	                                    *cp = 0;
26935	                                    n = 1;
26936	                            } else if (n) {
26937	                                    *pp++ = cp;
26938	                                    n = 0;
26939	                            }
26940	                    }
26941	                    /* null terminate last domain if there are excess */
26942	                    while (*cp != '\0' && *cp != ' ' && *cp != '\t')
26943	                            cp++;
26944	                    *cp = '\0';
26945	                    *pp++ = 0;
26946	                    havesearch = 1;
26947	                    continue;
26948	                }
26949	                /* read nameservers to query */
26950	                if (!strncmp(buf, "nameserver", sizeof("nameserver") - 1) &&
26951	                   _res.nscount < MAXNS) {
26952	                    cp = buf + sizeof("nameserver") - 1;
26953	                    while (*cp == ' ' || *cp == '\t')
26954	                            cp++;
26955	                    if ((*cp == '\0') || (*cp == '\n'))
26956	                            continue;
26957	                    if (!inet_aton(cp, &_res.nsaddr_list[_res.nscount]))
26958	                            continue;
26959	                    _res.nsport_list[_res.nscount]= nameserver_port;
.Op 253 src/lib/ip/res_init.c
26960	                    _res.nscount++;
26961	                    continue;
26962	                }
26963	            }
26964	            (void) fclose(fp);
26965	        }
26966	        if (_res.nscount == 0) {
26967	                /* "localhost" is the default nameserver. */
26968	                _res.nsaddr_list[0]= HTONL(0x7F000001);
26969	                _res.nsport_list[0]= nameserver_port;
26970	                _res.nscount= 1;
26971	        }
26972	        if (_res.defdname[0] == 0) {
26973	                if (gethostname(buf, sizeof(_res.defdname)) == 0 &&
26974	                   (cp = index(buf, '.')))
26975	                        (void)strcpy(_res.defdname, cp + 1);
26976	        }
26977	
26978	        /* find components of local domain that might be searched */
26979	        if (havesearch == 0) {
26980	                pp = _res.dnsrch;
26981	                *pp++ = _res.defdname;
26982	                for (cp = _res.defdname, n = 0; *cp; cp++)
26983	                        if (*cp == '.')
26984	                                n++;
26985	                cp = _res.defdname;
26986	                for (; n >= LOCALDOMAINPARTS && pp < _res.dnsrch + MAXDFLSRCH;
26987	                    n--) {
26988	                        cp = index(cp, '.');
26989	                        *pp++ = ++cp;
26990	                }
26991	                *pp++ = 0;
26992	        }
26993	        _res.options |= RES_INIT;
26994	        return (0);
26995	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/res_mkquery.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

27000	/*
27001	 * Copyright (c) 1985 Regents of the University of California.
27002	 * All rights reserved.
27003	 *
27004	 * Redistribution and use in source and binary forms are permitted
27005	 * provided that: (1) source distributions retain this entire copyright
27006	 * notice and comment, and (2) distributions including binaries display
27007	 * the following acknowledgement:  ``This product includes software
27008	 * developed by the University of California, Berkeley and its contributors''
27009	 * in the documentation or other materials provided with the distribution
27010	 * and in all advertising materials mentioning features or use of this
27011	 * software. Neither the name of the University nor the names of its
27012	 * contributors may be used to endorse or promote products derived
27013	 * from this software without specific prior written permission.
27014	 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
.Ep 254 src/lib/ip/res_mkquery.c
27015	 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
27016	 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
27017	 */
27018	
27019	#if defined(LIBC_SCCS) && !defined(lint)
27020	static char sccsid[] = "@(#)res_mkquery.c       6.12 (Berkeley) 6/1/90";
27021	#endif /* LIBC_SCCS and not lint */
27022	
27023	#if _MINIX
27024	#include <sys/types.h>
27025	#include <stdio.h>
27026	#include <stdlib.h>
27027	#include <string.h>
27028	
27029	#include <net/hton.h>
27030	#include <net/gen/in.h>
27031	#include <net/gen/nameser.h>
27032	#include <net/gen/resolv.h>
27033	
27034	typedef u16_t u_short;
27035	typedef unsigned u_int;
27036	typedef u32_t u_long;
27037	
27038	#define bzero(b,l) memset(b,0,l)
27039	#define bcopy(s,d,l) memcpy(d,s,l)
27040	
27041	#define putshort __putshort
27042	#define putlong __putlong
27043	#else
27044	#include <stdio.h>
27045	#include <sys/types.h>
27046	#include <netinet/in.h>
27047	#include <arpa/nameser.h>
27048	#include <resolv.h>
27049	#endif
27050	
27051	#ifdef __STDC__
27052	#define _CONST  const
27053	#else
27054	#define _CONST
27055	#endif
27056	
27057	/*
27058	 * Form all types of queries.
27059	 * Returns the size of the result or -1.
27060	 */
27061	res_mkquery(op, dname, class, type, data, datalen, newrr, buf, buflen)
27062	        int op;                 /* opcode of query */
27063	        _CONST char *dname;     /* domain name */
27064	        int class, type;        /* class and type of query */
27065	        _CONST char *data;      /* resource record data */
27066	        int datalen;            /* length of data */
27067	        _CONST struct rrec *newrr; /* new rr for modify or append */
27068	        char *buf;              /* buffer to put query */
27069	        int buflen;             /* size of buffer */
27070	{
27071	        register dns_hdr_t *hp;
27072	        register char *cp;
27073	        register int n;
27074	        char *dnptrs[10], **dpp, **lastdnptr;
.Op 255 src/lib/ip/res_mkquery.c
27075	
27076	#ifdef DEBUG
27077	        if (_res.options & RES_DEBUG)
27078	                printf("res_mkquery(%d, %s, %d, %d)\n", op, dname, class, type);
27079	#endif /* DEBUG */
27080	        /*
27081	         * Initialize header fields.
27082	         */
27083	        if ((buf == NULL) || (buflen < sizeof(dns_hdr_t)))
27084	                return(-1);
27085	        bzero(buf, sizeof(dns_hdr_t));
27086	        hp = (dns_hdr_t *) buf;
27087	        hp->dh_id = htons(++_res.id);
27088	        hp->dh_flag1= 0;
27089	        hp->dh_flag2= 0;
27090	        hp->dh_flag1 |= (op << 3) & DHF_OPCODE;
27091	        hp->dh_flag2 |= ((_res.options & RES_PRIMARY) != 0 ? 1 : 0) << 6;
27092	        hp->dh_flag1 |= (_res.options & RES_RECURSE) != 0 ? 1 : 0;
27093	        hp->dh_flag2 |= NOERROR & DHF_RCODE;
27094	        cp = buf + sizeof(dns_hdr_t);
27095	        buflen -= sizeof(dns_hdr_t);
27096	        dpp = dnptrs;
27097	        *dpp++ = buf;
27098	        *dpp++ = NULL;
27099	        lastdnptr = dnptrs + sizeof(dnptrs)/sizeof(dnptrs[0]);
27100	        /*
27101	         * perform opcode specific processing
27102	         */
27103	        switch (op) {
27104	        case QUERY:
27105	                if ((buflen -= QFIXEDSZ) < 0)
27106	                        return(-1);
27107	                if ((n = dn_comp((u8_t *)dname, (u8_t *)cp, buflen, 
27108	                        (u8_t **)dnptrs, (u8_t **)lastdnptr)) < 0)
27109	                        return (-1);
27110	                cp += n;
27111	                buflen -= n;
27112	                putshort(type, (u8_t *)cp);
27113	                cp += sizeof(u_short);
27114	                putshort(class, (u8_t *)cp);
27115	                cp += sizeof(u_short);
27116	                hp->dh_qdcount = htons(1);
27117	                if (op == QUERY || data == NULL)
27118	                        break;
27119	                /*
27120	                 * Make an additional record for completion domain.
27121	                 */
27122	                buflen -= RRFIXEDSZ;
27123	                if ((n = dn_comp((u8_t *)data, (u8_t *)cp, buflen, 
27124	                        (u8_t **)dnptrs, (u8_t **)lastdnptr)) < 0)
27125	                        return (-1);
27126	                cp += n;
27127	                buflen -= n;
27128	                putshort(T_NULL, (u8_t *)cp);
27129	                cp += sizeof(u_short);
27130	                putshort(class, (u8_t *)cp);
27131	                cp += sizeof(u_short);
27132	                putlong(0, (u8_t *)cp);
27133	                cp += sizeof(u_long);
27134	                putshort(0, (u8_t *)cp);
.Ep 256 src/lib/ip/res_mkquery.c
27135	                cp += sizeof(u_short);
27136	                hp->dh_arcount = htons(1);
27137	                break;
27138	
27139	        case IQUERY:
27140	                /*
27141	                 * Initialize answer section
27142	                 */
27143	                if (buflen < 1 + RRFIXEDSZ + datalen)
27144	                        return (-1);
27145	                *cp++ = '\0';   /* no domain name */
27146	                putshort(type, (u8_t *)cp);
27147	                cp += sizeof(u_short);
27148	                putshort(class, (u8_t *)cp);
27149	                cp += sizeof(u_short);
27150	                putlong(0, (u8_t *)cp);
27151	                cp += sizeof(u_long);
27152	                putshort(datalen, (u8_t *)cp);
27153	                cp += sizeof(u_short);
27154	                if (datalen) {
27155	                        bcopy(data, cp, datalen);
27156	                        cp += datalen;
27157	                }
27158	                hp->dh_ancount = htons(1);
27159	                break;
27160	
27161	#ifdef ALLOW_UPDATES
27162	        /*
27163	         * For UPDATEM/UPDATEMA, do UPDATED/UPDATEDA followed by UPDATEA
27164	         * (Record to be modified is followed by its replacement in msg.)
27165	         */
27166	        case UPDATEM:
27167	        case UPDATEMA:
27168	
27169	        case UPDATED:
27170	                /*
27171	                 * The res code for UPDATED and UPDATEDA is the same; user
27172	                 * calls them differently: specifies data for UPDATED; server
27173	                 * ignores data if specified for UPDATEDA.
27174	                 */
27175	        case UPDATEDA:
27176	                buflen -= RRFIXEDSZ + datalen;
27177	                if ((n = dn_comp(dname, cp, buflen, dnptrs, lastdnptr)) < 0)
27178	                        return (-1);
27179	                cp += n;
27180	                putshort(type, cp);
27181	                cp += sizeof(u_short);
27182	                putshort(class, cp);
27183	                cp += sizeof(u_short);
27184	                putlong(0, cp);
27185	                cp += sizeof(u_long);
27186	                putshort(datalen, cp);
27187	                cp += sizeof(u_short);
27188	                if (datalen) {
27189	                        bcopy(data, cp, datalen);
27190	                        cp += datalen;
27191	                }
27192	                if ( (op == UPDATED) || (op == UPDATEDA) ) {
27193	                        hp->ancount = htons(0);
27194	                        break;
.Op 257 src/lib/ip/res_mkquery.c
27195	                }
27196	                /* Else UPDATEM/UPDATEMA, so drop into code for UPDATEA */
27197	
27198	        case UPDATEA:   /* Add new resource record */
27199	                buflen -= RRFIXEDSZ + datalen;
27200	                if ((n = dn_comp(dname, cp, buflen, dnptrs, lastdnptr)) < 0)
27201	                        return (-1);
27202	                cp += n;
27203	                putshort(newrr->r_type, cp);
27204	                cp += sizeof(u_short);
27205	                putshort(newrr->r_class, cp);
27206	                cp += sizeof(u_short);
27207	                putlong(0, cp);
27208	                cp += sizeof(u_long);
27209	                putshort(newrr->r_size, cp);
27210	                cp += sizeof(u_short);
27211	                if (newrr->r_size) {
27212	                        bcopy(newrr->r_data, cp, newrr->r_size);
27213	                        cp += newrr->r_size;
27214	                }
27215	                hp->ancount = htons(0);
27216	                break;
27217	
27218	#endif /* ALLOW_UPDATES */
27219	        }
27220	        return (cp - buf);
27221	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/res_query.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

27300	/*
27301	 * Copyright (c) 1988 Regents of the University of California.
27302	 * All rights reserved.
27303	 *
27304	 * Redistribution and use in source and binary forms are permitted
27305	 * provided that: (1) source distributions retain this entire copyright
27306	 * notice and comment, and (2) distributions including binaries display
27307	 * the following acknowledgement:  ``This product includes software
27308	 * developed by the University of California, Berkeley and its contributors''
27309	 * in the documentation or other materials provided with the distribution
27310	 * and in all advertising materials mentioning features or use of this
27311	 * software. Neither the name of the University nor the names of its
27312	 * contributors may be used to endorse or promote products derived
27313	 * from this software without specific prior written permission.
27314	 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
27315	 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
27316	 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
27317	 */
27318	
27319	#if defined(LIBC_SCCS) && !defined(lint)
27320	static char sccsid[] = "@(#)res_query.c 5.7 (Berkeley) 6/1/90";
27321	#endif /* LIBC_SCCS and not lint */
27322	
27323	#if _MINIX
27324	#include <sys/types.h>
.Ep 258 src/lib/ip/res_query.c
27325	#include <ctype.h>
27326	#include <errno.h>
27327	#include <stdio.h>
27328	#include <stdlib.h>
27329	#include <string.h>
27330	
27331	#include <net/hton.h>
27332	#include <net/gen/in.h>
27333	#include <net/gen/nameser.h>
27334	#include <net/gen/netdb.h>
27335	#include <net/gen/resolv.h>
27336	
27337	typedef u8_t u_char;
27338	
27339	#define bcopy(s,d,l) memcpy(d,s,l)
27340	
27341	#define hostalias __hostalias
27342	#else
27343	#include <sys/param.h>
27344	#include <sys/socket.h>
27345	#include <netinet/in.h>
27346	#include <ctype.h>
27347	#include <netdb.h>
27348	#include <stdio.h>
27349	#include <errno.h>
27350	#include <string.h>
27351	#include <arpa/inet.h>
27352	#include <arpa/nameser.h>
27353	#include <resolv.h>
27354	
27355	extern int errno;
27356	#endif
27357	
27358	#if __STDC__
27359	#define CONST   const
27360	#else
27361	#define CONST
27362	#endif
27363	
27364	#if PACKETSZ > 1024
27365	#define MAXPACKET       PACKETSZ
27366	#else
27367	#define MAXPACKET       1024
27368	#endif
27369	
27370	int h_errno;
27371	
27372	/*
27373	 * Formulate a normal query, send, and await answer.
27374	 * Returned answer is placed in supplied buffer "answer".
27375	 * Perform preliminary check of answer, returning success only
27376	 * if no error is indicated and the answer count is nonzero.
27377	 * Return the size of the response on success, -1 on error.
27378	 * Error number is left in h_errno.
27379	 * Caller must parse answer and determine whether it answers the question.
27380	 */
27381	int
27382	res_query(name, class, type, answer, anslen)
27383	        char *name;             /* domain name */
27384	        int class, type;        /* class and type of query */
.Op 259 src/lib/ip/res_query.c
27385	        u_char *answer;         /* buffer to put answer */
27386	        int anslen;             /* size of answer buffer */
27387	{
27388	        char buf[MAXPACKET];
27389	        dns_hdr_t *hp;
27390	        int n;
27391	
27392	        if ((_res.options & RES_INIT) == 0 && res_init() == -1)
27393	                return (-1);
27394	#ifdef DEBUG
27395	        if (_res.options & RES_DEBUG)
27396	                printf("res_query(%s, %d, %d)\n", name, class, type);
27397	#endif
27398	        n = res_mkquery(QUERY, name, class, type, (char *)NULL, 0, NULL,
27399	            buf, sizeof(buf));
27400	
27401	        if (n <= 0) {
27402	#ifdef DEBUG
27403	                if (_res.options & RES_DEBUG)
27404	                        printf("res_query: mkquery failed\n");
27405	#endif
27406	                h_errno = NO_RECOVERY;
27407	                return (n);
27408	        }
27409	        n = res_send(buf, n, (char *)answer, anslen);
27410	        if (n < 0) {
27411	#ifdef DEBUG
27412	                if (_res.options & RES_DEBUG)
27413	                        printf("res_query: send error(%d)\n", errno);
27414	#endif
27415	                h_errno = TRY_AGAIN;
27416	                return(n);
27417	        }
27418	
27419	        hp = (dns_hdr_t *) answer;
27420	        if ((hp->dh_flag2 & DHF_RCODE) != NOERROR || 
27421	                ntohs(hp->dh_ancount) == 0) {
27422	#ifdef DEBUG
27423	                if (_res.options & RES_DEBUG)
27424	                        printf("rcode = %d, ancount=%d\n", 
27425	                                hp->dh_flag2 & DHF_RCODE,
27426	                            ntohs(hp->dh_ancount));
27427	#endif
27428	                switch (hp->dh_flag2 & DHF_RCODE) {
27429	                        case NXDOMAIN:
27430	                                h_errno = HOST_NOT_FOUND;
27431	                                break;
27432	                        case SERVFAIL:
27433	                                h_errno = TRY_AGAIN;
27434	                                break;
27435	                        case NOERROR:
27436	                                h_errno = NO_DATA;
27437	                                break;
27438	                        case FORMERR:
27439	                        case NOTIMP:
27440	                        case REFUSED:
27441	                        default:
27442	                                h_errno = NO_RECOVERY;
27443	                                break;
27444	                }
.Ep 260 src/lib/ip/res_query.c
27445	                return (-1);
27446	        }
27447	        return(n);
27448	}
	
27450	/*
27451	 * Formulate a normal query, send, and retrieve answer in supplied buffer.
27452	 * Return the size of the response on success, -1 on error.
27453	 * If enabled, implement search rules until answer or unrecoverable failure
27454	 * is detected.  Error number is left in h_errno.
27455	 * Only useful for queries in the same name hierarchy as the local host
27456	 * (not, for example, for host address-to-name lookups in domain in-addr.arpa).
27457	 */
27458	res_search(name, class, type, answer, anslen)
27459	        char *name;             /* domain name */
27460	        int class, type;        /* class and type of query */
27461	        u_char *answer;         /* buffer to put answer */
27462	        int anslen;             /* size of answer */
27463	{
27464	        register char *cp, **domain;
27465	        int n, ret, got_nodata = 0;
27466	
27467	        if ((_res.options & RES_INIT) == 0 && res_init() == -1)
27468	                return (-1);
27469	
27470	        errno = 0;
27471	        h_errno = HOST_NOT_FOUND;               /* default, if we never query */
27472	        for (cp = name, n = 0; *cp; cp++)
27473	                if (*cp == '.')
27474	                        n++;
27475	        if (n == 0 && (cp = hostalias(name)))
27476	                return (res_query(cp, class, type, answer, anslen));
27477	
27478	        /*
27479	         * We do at least one level of search if
27480	         *      - there is no dot and RES_DEFNAME is set, or
27481	         *      - there is at least one dot, there is no trailing dot,
27482	         *        and RES_DNSRCH is set.
27483	         */
27484	        if ((n == 0 && _res.options & RES_DEFNAMES) ||
27485	           (n != 0 && *--cp != '.' && _res.options & RES_DNSRCH))
27486	             for (domain = _res.dnsrch; *domain; domain++) {
27487	                ret = res_querydomain(name, *domain, class, type,
27488	                    answer, anslen);
27489	                if (ret > 0)
27490	                        return (ret);
27491	                /*
27492	                 * If no server present, give up.
27493	                 * If name isn't found in this domain,
27494	                 * keep trying higher domains in the search list
27495	                 * (if that's enabled).
27496	                 * On a NO_DATA error, keep trying, otherwise
27497	                 * a wildcard entry of another type could keep us
27498	                 * from finding this entry higher in the domain.
27499	                 * If we get some other error (negative answer or
27500	                 * server failure), then stop searching up,
27501	                 * but try the input name below in case it's fully-qualified.
27502	                 */
27503	                if (errno == ECONNREFUSED) {
27504	                        h_errno = TRY_AGAIN;
.Op 261 src/lib/ip/res_query.c
27505	                        return (-1);
27506	                }
27507	                if (h_errno == NO_DATA)
27508	                        got_nodata++;
27509	                if ((h_errno != HOST_NOT_FOUND && h_errno != NO_DATA) ||
27510	                    (_res.options & RES_DNSRCH) == 0)
27511	                        break;
27512	        }
27513	        /*
27514	         * If the search/default failed, try the name as fully-qualified,
27515	         * but only if it contained at least one dot (even trailing).
27516	         * This is purely a heuristic; we assume that any reasonable query
27517	         * about a top-level domain (for servers, SOA, etc) will not use
27518	         * res_search.
27519	         */
27520	        if (n && (ret = res_querydomain(name, (char *)NULL, class, type,
27521	            answer, anslen)) > 0)
27522	                return (ret);
27523	        if (got_nodata)
27524	                h_errno = NO_DATA;
27525	        return (-1);
27526	}
	
27528	/*
27529	 * Perform a call on res_query on the concatenation of name and domain,
27530	 * removing a trailing dot from name if domain is NULL.
27531	 */
27532	int
27533	res_querydomain(name, domain, class, type, answer, anslen)
27534	        char *name, *domain;
27535	        int class, type;        /* class and type of query */
27536	        u_char *answer;         /* buffer to put answer */
27537	        int anslen;             /* size of answer */
27538	{
27539	        char nbuf[2*MAXDNAME+2];
27540	        char *longname = nbuf;
27541	        int n;
27542	
27543	#ifdef DEBUG
27544	        if (_res.options & RES_DEBUG)
27545	                printf("res_querydomain(%s, %s, %d, %d)\n",
27546	                    name, domain, class, type);
27547	#endif
27548	        if (domain == NULL) {
27549	                /*
27550	                 * Check for trailing '.';
27551	                 * copy without '.' if present.
27552	                 */
27553	                n = strlen(name) - 1;
27554	                if (name[n] == '.' && n < sizeof(nbuf) - 1) {
27555	                        bcopy(name, nbuf, n);
27556	                        nbuf[n] = '\0';
27557	                } else
27558	                        longname = name;
27559	        } else
27560	                (void)sprintf(nbuf, "%.*s.%.*s",
27561	                    MAXDNAME, name, MAXDNAME, domain);
27562	
27563	        return (res_query(longname, class, type, answer, anslen));
27564	}
.Ep 262 src/lib/ip/res_query.c
	
27566	char *
27567	hostalias(name)
27568	        register CONST char *name;
27569	{
27570	        register char *C1, *C2;
27571	        FILE *fp;
27572	        char *file;
27573	        char buf[BUFSIZ];
27574	        static char abuf[MAXDNAME];
27575	
27576	        file = getenv("HOSTALIASES");
27577	        if (file == NULL || (fp = fopen(file, "r")) == NULL)
27578	                return (NULL);
27579	        buf[sizeof(buf) - 1] = '\0';
27580	        while (fgets(buf, sizeof(buf), fp)) {
27581	                for (C1 = buf; *C1 && !isspace(*C1); ++C1);
27582	                if (!*C1)
27583	                        break;
27584	                *C1 = '\0';
27585	                if (!strcasecmp(buf, name)) {
27586	                        while (isspace(*++C1));
27587	                        if (!*C1)
27588	                                break;
27589	                        for (C2 = C1 + 1; *C2 && !isspace(*C2); ++C2);
27590	                        abuf[sizeof(abuf) - 1] = *C2 = '\0';
27591	                        (void)strncpy(abuf, C1, sizeof(abuf) - 1);
27592	                        fclose(fp);
27593	                        return (abuf);
27594	                }
27595	        }
27596	        fclose(fp);
27597	        return (NULL);
27598	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/res_send.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

27600	/*
27601	 * Copyright (c) 1985, 1989 Regents of the University of California.
27602	 * All rights reserved.
27603	 *
27604	 * Redistribution and use in source and binary forms, with or without
27605	 * modification, are permitted provided that the following conditions
27606	 * are met:
27607	 * 1. Redistributions of source code must retain the above copyright
27608	 *    notice, this list of conditions and the following disclaimer.
27609	 * 2. Redistributions in binary form must reproduce the above copyright
27610	 *    notice, this list of conditions and the following disclaimer in the
27611	 *    documentation and/or other materials provided with the distribution.
27612	 * 3. All advertising materials mentioning features or use of this software
27613	 *    must display the following acknowledgement:
27614	 *      This product includes software developed by the University of
27615	 *      California, Berkeley and its contributors.
27616	 * 4. Neither the name of the University nor the names of its contributors
27617	 *    may be used to endorse or promote products derived from this software
27618	 *    without specific prior written permission.
27619	 *
.Op 263 src/lib/ip/res_send.c
27620	 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
27621	 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
27622	 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
27623	 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
27624	 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
27625	 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
27626	 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
27627	 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
27628	 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
27629	 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
27630	 * SUCH DAMAGE.
27631	 */
27632	
27633	#if defined(LIBC_SCCS) && !defined(lint)
27634	static char sccsid[] = "@(#)res_send.c  6.27 (Berkeley) 2/24/91";
27635	#endif /* LIBC_SCCS and not lint */
27636	
27637	/*
27638	 * Send query to name server and wait for reply.
27639	 */
27640	
27641	#if !_MINIX
27642	#include <sys/param.h>
27643	#include <sys/time.h>
27644	#include <sys/socket.h>
27645	#include <sys/uio.h>
27646	#include <netinet/in.h>
27647	#include <arpa/nameser.h>
27648	#include <stdio.h>
27649	#include <errno.h>
27650	#include <resolv.h>
27651	#include <unistd.h>
27652	#include <string.h>
27653	
27654	#else /* _MINIX */
27655	
27656	#include <sys/types.h>
27657	#include <sys/ioctl.h>
27658	#include <sys/stat.h>
27659	#include <assert.h>
27660	#include <errno.h>
27661	#include <fcntl.h>
27662	#include <signal.h>
27663	#include <stdio.h>
27664	#include <stdlib.h>
27665	#include <string.h>
27666	#include <unistd.h>
27667	
27668	#include <net/hton.h>
27669	
27670	#include <net/netlib.h>
27671	#include <net/gen/in.h>
27672	#include <net/gen/inet.h>
27673	#include <net/gen/netdb.h>
27674	#include <net/gen/nameser.h>
27675	#include <net/gen/resolv.h>
27676	#include <net/gen/tcp.h>
27677	#include <net/gen/tcp_io.h>
27678	#include <net/gen/udp.h>
27679	#include <net/gen/udp_hdr.h>
.Ep 264 src/lib/ip/res_send.c
27680	#include <net/gen/udp_io.h>
27681	
27682	typedef u16_t u_short;
27683	
27684	static int tcp_connect _ARGS(( ipaddr_t host, Tcpport_t port, int *terrno ));
27685	static int tcpip_writeall _ARGS(( int fd, const char *buf, size_t siz ));
27686	static int udp_connect _ARGS(( void ));
27687	static int udp_sendto _ARGS(( int fd, const char *buf, unsigned buflen,
27688	                                ipaddr_t addr, Udpport_t port ));
27689	static int udp_receive _ARGS(( int fd, char *buf, unsigned buflen,
27690	                                time_t timeout ));
27691	static void alarm_handler _ARGS(( int sig ));
27692	
27693	#endif /* !_MINIX */
27694	
27695	static int s = -1;      /* socket used for communications */
27696	#if !_MINIX
27697	static struct sockaddr no_addr;
27698	
27699	#ifndef FD_SET
27700	#define NFDBITS         32
27701	#define FD_SETSIZE      32
27702	#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
27703	#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
27704	#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
27705	#define FD_ZERO(p)      bzero((char *)(p), sizeof(*(p)))
27706	#endif /* FD_SET */
27707	#endif /* _MINIX */
27708	
27709	res_send(buf, buflen, answer, anslen)
27710	        const char *buf;
27711	        int buflen;
27712	        char *answer;
27713	        int anslen;
27714	{
27715	        register int n;
27716	        int try, v_circuit, resplen, ns;
27717	        int gotsomewhere = 0, connected = 0;
27718	        int connreset = 0;
27719	#if !_MINIX
27720	        u_short id, len;
27721	#else /* _MINIX */
27722	        u16_t id, len;
27723	#endif /* !_MINIX */
27724	        char *cp;
27725	#if !_MINIX
27726	        fd_set dsmask;
27727	        struct timeval timeout;
27728	        HEADER *hp = (HEADER *) buf;
27729	        HEADER *anhp = (HEADER *) answer;
27730	        struct iovec iov[2];
27731	#else /* _MINIX */
27732	        time_t timeout;
27733	        dns_hdr_t *hp = (dns_hdr_t *) buf;
27734	        dns_hdr_t *anhp = (dns_hdr_t *) answer;
27735	#endif /* !_MINIX */
27736	        int terrno = ETIMEDOUT;
27737	        char junk[512];
27738	
27739	#ifdef DEBUG
.Op 265 src/lib/ip/res_send.c
27740	        if (_res.options & RES_DEBUG) {
27741	                printf("res_send()\n");
27742	                __p_query(buf);
27743	        }
27744	#endif /* DEBUG */
27745	        if (!(_res.options & RES_INIT))
27746	                if (res_init() == -1) {
27747	                        return(-1);
27748	                }
27749	
27750	        v_circuit = (_res.options & RES_USEVC) || buflen > PACKETSZ;
27751	#if !_MINIX
27752	        id = hp->id;
27753	#else /* _MINIX */
27754	        id = hp->dh_id;
27755	#endif /* !_MINIX */
27756	        /*
27757	         * Send request, RETRY times, or until successful
27758	         */
27759	        for (try = 0; try < _res.retry; try++) {
27760	           for (ns = 0; ns < _res.nscount; ns++) {
27761	#ifdef DEBUG
27762	#if !_MINIX
27763	                if (_res.options & RES_DEBUG)
27764	                        printf("Querying server (# %d) address = %s\n", ns+1,
27765	                              inet_ntoa(_res.nsaddr_list[ns].sin_addr));
27766	#else /* _MINIX */
27767	                if (_res.options & RES_DEBUG)
27768	                        printf("Querying server (# %d) address = %s\n", ns+1,
27769	                              inet_ntoa(_res.nsaddr_list[ns]));
27770	#endif /* !_MINIX */
27771	#endif /* DEBUG */
27772	        usevc:
27773	                if (v_circuit) {
27774	#if !_MINIX
27775	                        int truncated = 0;
27776	
27777	                        /*
27778	                         * Use virtual circuit;
27779	                         * at most one attempt per server.
27780	                         */
27781	                        try = _res.retry;
27782	                        if (s < 0) {
27783	                                s = socket(AF_INET, SOCK_STREAM, 0);
27784	                                if (s < 0) {
27785	                                        terrno = errno;
27786	#ifdef DEBUG
27787	                                        if (_res.options & RES_DEBUG)
27788	                                            perror("socket (vc) failed");
27789	#endif /* DEBUG */
27790	                                        continue;
27791	                                }
27792	                                if (connect(s,
27793	                                    (struct sockaddr *)&(_res.nsaddr_list[ns]),
27794	                                    sizeof(struct sockaddr)) < 0) {
27795	                                        terrno = errno;
27796	#ifdef DEBUG
27797	                                        if (_res.options & RES_DEBUG)
27798	                                            perror("connect failed");
27799	#endif /* DEBUG */
.Ep 266 src/lib/ip/res_send.c
27800	                                        (void) close(s);
27801	                                        s = -1;
27802	                                        continue;
27803	                                }
27804	                        }
27805	                        /*
27806	                         * Send length & message
27807	                         */
27808	                        len = htons((u_short)buflen);
27809	                        iov[0].iov_base = (caddr_t)&len;
27810	                        iov[0].iov_len = sizeof(len);
27811	                        iov[1].iov_base = (char *)buf;
27812	                        iov[1].iov_len = buflen;
27813	                        if (writev(s, iov, 2) != sizeof(len) + buflen) {
27814	                                terrno = errno;
27815	#ifdef DEBUG
27816	                                if (_res.options & RES_DEBUG)
27817	                                        perror("write failed");
27818	#endif /* DEBUG */
27819	                                (void) close(s);
27820	                                s = -1;
27821	                                continue;
27822	                        }
27823	                        /*
27824	                         * Receive length & response
27825	                         */
27826	                        cp = answer;
27827	                        len = sizeof(short);
27828	                        while (len != 0 &&
27829	                            (n = read(s, (char *)cp, (int)len)) > 0) {
27830	                                cp += n;
27831	                                len -= n;
27832	                        }
27833	                        if (n <= 0) {
27834	                                terrno = errno;
27835	#ifdef DEBUG
27836	                                if (_res.options & RES_DEBUG)
27837	                                        perror("read failed");
27838	#endif /* DEBUG */
27839	                                (void) close(s);
27840	                                s = -1;
27841	                                /*
27842	                                 * A long running process might get its TCP
27843	                                 * connection reset if the remote server was
27844	                                 * restarted.  Requery the server instead of
27845	                                 * trying a new one.  When there is only one
27846	                                 * server, this means that a query might work
27847	                                 * instead of failing.  We only allow one reset
27848	                                 * per query to prevent looping.
27849	                                 */
27850	                                if (terrno == ECONNRESET && !connreset) {
27851	                                        connreset = 1;
27852	                                        ns--;
27853	                                }
27854	                                continue;
27855	                        }
27856	                        cp = answer;
27857	                        if ((resplen = ntohs(*(u_short *)cp)) > anslen) {
27858	#ifdef DEBUG
27859	                                if (_res.options & RES_DEBUG)
.Op 267 src/lib/ip/res_send.c
27860	                                        fprintf(stderr, "response truncated\n");
27861	#endif /* DEBUG */
27862	                                len = anslen;
27863	                                truncated = 1;
27864	                        } else
27865	                                len = resplen;
27866	                        while (len != 0 &&
27867	                           (n = read(s, (char *)cp, (int)len)) > 0) {
27868	                                cp += n;
27869	                                len -= n;
27870	                        }
27871	                        if (n <= 0) {
27872	                                terrno = errno;
27873	#ifdef DEBUG
27874	                                if (_res.options & RES_DEBUG)
27875	                                        perror("read failed");
27876	#endif /* DEBUG */
27877	                                (void) close(s);
27878	                                s = -1;
27879	                                continue;
27880	                        }
27881	                        if (truncated) {
27882	                                /*
27883	                                 * Flush rest of answer
27884	                                 * so connection stays in synch.
27885	                                 */
27886	                                anhp->tc = 1;
27887	                                len = resplen - anslen;
27888	                                while (len != 0) {
27889	                                        n = (len > sizeof(junk) ?
27890	                                            sizeof(junk) : len);
27891	                                        if ((n = read(s, junk, n)) > 0)
27892	                                                len -= n;
27893	                                        else
27894	                                                break;
27895	                                }
27896	                        }
27897	#else /* _MINIX */
27898	                        int truncated = 0;
27899	                        int nbytes;
27900	
27901	                        /*
27902	                         * Use virtual circuit;
27903	                         * at most one attempt per server.
27904	                         */
27905	                        try = _res.retry;
27906	                        if (s < 0) 
27907	                        {
27908	                                s= tcp_connect(_res.nsaddr_list[ns],
27909	                                        _res.nsport_list[ns], &terrno);
27910	                                if (s == -1)
27911	                                        continue;
27912	                        }
27913	                        /*
27914	                         * Send length & message
27915	                         */
27916	                        len = htons((u_short)buflen);
27917	                        nbytes= tcpip_writeall(s, (char *)&len, 
27918	                                sizeof(len));
27919	                        if (nbytes != sizeof(len))
.Ep 268 src/lib/ip/res_send.c
27920	                        {
27921	                                terrno= errno;
27922	#ifdef DEBUG
27923	                                if (_res.options & RES_DEBUG)
27924	                                        fprintf(stderr, "write failed: %s\n",
27925	                                        strerror(terrno));
27926	#endif /* DEBUG */
27927	                                close(s);
27928	                                s= -1;
27929	                                continue;
27930	                        }
27931	                        nbytes= tcpip_writeall(s, buf, buflen);
27932	                        if (nbytes != buflen)
27933	                        {
27934	                                terrno= errno;
27935	#ifdef DEBUG
27936	                                if (_res.options & RES_DEBUG)
27937	                                        fprintf(stderr, "write failed: %s\n",
27938	                                        strerror(terrno));
27939	#endif /* DEBUG */
27940	                                close(s);
27941	                                s= -1;
27942	                                continue;
27943	                        }
27944	                        /*
27945	                         * Receive length & response
27946	                         */
27947	                        cp = answer;
27948	                        len = sizeof(short);
27949	                        while (len != 0)
27950	                        {
27951	                                n = read(s, (char *)cp, (int)len);
27952	                                if (n <= 0)
27953	                                        break;
27954	                                cp += n;
27955	                                assert(len >= n);
27956	                                len -= n;
27957	                        }
27958	                        if (len) {
27959	                                terrno = errno;
27960	#ifdef DEBUG
27961	                                if (_res.options & RES_DEBUG)
27962	                                        fprintf(stderr, "read failed: %s\n",
27963	                                                strerror(terrno));
27964	#endif /* DEBUG */
27965	                                close(s);
27966	                                s= -1;
27967	                                /*
27968	                                 * A long running process might get its TCP
27969	                                 * connection reset if the remote server was
27970	                                 * restarted.  Requery the server instead of
27971	                                 * trying a new one.  When there is only one
27972	                                 * server, this means that a query might work
27973	                                 * instead of failing.  We only allow one reset
27974	                                 * per query to prevent looping.
27975	                                 */
27976	                                if (terrno == ECONNRESET && !connreset) {
27977	                                        connreset = 1;
27978	                                        ns--;
27979	                                }
.Op 269 src/lib/ip/res_send.c
27980	                                continue;
27981	                        }
27982	                        cp = answer;
27983	                        if ((resplen = ntohs(*(u_short *)cp)) > anslen) {
27984	#ifdef DEBUG
27985	                                if (_res.options & RES_DEBUG)
27986	                                        fprintf(stderr, "response truncated\n");
27987	#endif /* DEBUG */
27988	                                len = anslen;
27989	                                truncated = 1;
27990	                        } else
27991	                                len = resplen;
27992	                        while (len != 0)
27993	                        {
27994	                                n= read(s, (char *)cp, (int)len);
27995	                                if (n <= 0)
27996	                                        break;
27997	                                cp += n;
27998	                                assert(len >= n);
27999	                                len -= n;
28000	                        }
28001	                        if (len) {
28002	                                terrno = errno;
28003	#ifdef DEBUG
28004	                                if (_res.options & RES_DEBUG)
28005	                                        fprintf(stderr, "read failed: %s\n",
28006	                                                strerror(terrno));
28007	#endif /* DEBUG */
28008	                                close(s);
28009	                                s= -1;
28010	                                continue;
28011	                        }
28012	                        if (truncated) {
28013	                                /*
28014	                                 * Flush rest of answer
28015	                                 * so connection stays in synch.
28016	                                 */
28017	                                anhp->dh_flag1 |= DHF_TC;
28018	                                len = resplen - anslen;
28019	                                while (len != 0) {
28020	                                        n = (len > sizeof(junk) ?
28021	                                            sizeof(junk) : len);
28022	                                        n = read(s, junk, n);
28023	                                        if (n <= 0)
28024	                                        {
28025	                                                assert(len >= n);
28026	                                                len -= n;
28027	                                        }
28028	                                        else
28029	                                                break;
28030	                                }
28031	                        }
28032	#endif /* _MINIX */
28033	                } else {
28034	#if !_MINIX
28035	                        /*
28036	                         * Use datagrams.
28037	                         */
28038	                        if (s < 0) {
28039	                                s = socket(AF_INET, SOCK_DGRAM, 0);
.Ep 270 src/lib/ip/res_send.c
28040	                                if (s < 0) {
28041	                                        terrno = errno;
28042	#ifdef DEBUG
28043	                                        if (_res.options & RES_DEBUG)
28044	                                            perror("socket (dg) failed");
28045	#endif /* DEBUG */
28046	                                        continue;
28047	                                }
28048	                        }
28049	#if     BSD >= 43
28050	                        /*
28051	                         * I'm tired of answering this question, so:
28052	                         * On a 4.3BSD+ machine (client and server,
28053	                         * actually), sending to a nameserver datagram
28054	                         * port with no nameserver will cause an
28055	                         * ICMP port unreachable message to be returned.
28056	                         * If our datagram socket is "connected" to the
28057	                         * server, we get an ECONNREFUSED error on the next
28058	                         * socket operation, and select returns if the
28059	                         * error message is received.  We can thus detect
28060	                         * the absence of a nameserver without timing out.
28061	                         * If we have sent queries to at least two servers,
28062	                         * however, we don't want to remain connected,
28063	                         * as we wish to receive answers from the first
28064	                         * server to respond.
28065	                         */
28066	                        if (_res.nscount == 1 || (try == 0 && ns == 0)) {
28067	                                /*
28068	                                 * Don't use connect if we might
28069	                                 * still receive a response
28070	                                 * from another server.
28071	                                 */
28072	                                if (connected == 0) {
28073	                        if (connect(s, (struct sockaddr *)&_res.nsaddr_list[ns],
28074	                                            sizeof(struct sockaddr)) < 0) {
28075	#ifdef DEBUG
28076	                                                if (_res.options & RES_DEBUG)
28077	                                                        perror("connect");
28078	#endif /* DEBUG */
28079	                                                continue;
28080	                                        }
28081	                                        connected = 1;
28082	                                }
28083	                                if (send(s, buf, buflen, 0) != buflen) {
28084	#ifdef DEBUG
28085	                                        if (_res.options & RES_DEBUG)
28086	                                                perror("send");
28087	#endif /* DEBUG */
28088	                                        continue;
28089	                                }
28090	                        } else {
28091	                                /*
28092	                                 * Disconnect if we want to listen
28093	                                 * for responses from more than one server.
28094	                                 */
28095	                                if (connected) {
28096	                                        (void) connect(s, &no_addr,
28097	                                            sizeof(no_addr));
28098	                                        connected = 0;
28099	                                }
.Op 271 src/lib/ip/res_send.c
28100	#endif /* BSD */
28101	                                if (sendto(s, buf, buflen, 0,
28102	                                    (struct sockaddr *)&_res.nsaddr_list[ns],
28103	                                    sizeof(struct sockaddr)) != buflen) {
28104	#ifdef DEBUG
28105	                                        if (_res.options & RES_DEBUG)
28106	                                                perror("sendto");
28107	#endif /* DEBUG */
28108	                                        continue;
28109	                                }
28110	#if     BSD >= 43
28111	                        }
28112	#endif /* BSD */
28113	
28114	                        /*
28115	                         * Wait for reply
28116	                         */
28117	                        timeout.tv_sec = (_res.retrans << try);
28118	                        if (try > 0)
28119	                                timeout.tv_sec /= _res.nscount;
28120	                        if (timeout.tv_sec <= 0)
28121	                                timeout.tv_sec = 1;
28122	                        timeout.tv_usec = 0;
28123	wait:
28124	                        FD_ZERO(&dsmask);
28125	                        FD_SET(s, &dsmask);
28126	                        n = select(s+1, &dsmask, (fd_set *)NULL,
28127	                                (fd_set *)NULL, &timeout);
28128	                        if (n < 0) {
28129	#ifdef DEBUG
28130	                                if (_res.options & RES_DEBUG)
28131	                                        perror("select");
28132	#endif /* DEBUG */
28133	                                continue;
28134	                        }
28135	                        if (n == 0) {
28136	                                /*
28137	                                 * timeout
28138	                                 */
28139	#ifdef DEBUG
28140	                                if (_res.options & RES_DEBUG)
28141	                                        printf("timeout\n");
28142	#endif /* DEBUG */
28143	#if BSD >= 43
28144	                                gotsomewhere = 1;
28145	#endif
28146	                                continue;
28147	                        }
28148	                        if ((resplen = recv(s, answer, anslen, 0)) <= 0) {
28149	#ifdef DEBUG
28150	                                if (_res.options & RES_DEBUG)
28151	                                        perror("recvfrom");
28152	#endif /* DEBUG */
28153	                                continue;
28154	                        }
28155	                        gotsomewhere = 1;
28156	                        if (id != anhp->id) {
28157	                                /*
28158	                                 * response from old query, ignore it
28159	                                 */
.Ep 272 src/lib/ip/res_send.c
28160	#ifdef DEBUG
28161	                                if (_res.options & RES_DEBUG) {
28162	                                        printf("old answer:\n");
28163	                                        __p_query(answer);
28164	                                }
28165	#endif /* DEBUG */
28166	                                goto wait;
28167	                        }
28168	                        if (!(_res.options & RES_IGNTC) && anhp->tc) {
28169	                                /*
28170	                                 * get rest of answer;
28171	                                 * use TCP with same server.
28172	                                 */
28173	#ifdef DEBUG
28174	                                if (_res.options & RES_DEBUG)
28175	                                        printf("truncated answer\n");
28176	#endif /* DEBUG */
28177	                                (void) close(s);
28178	                                s = -1;
28179	                                v_circuit = 1;
28180	                                goto usevc;
28181	                        }
28182	#else /* _MINIX */
28183	                        /*
28184	                         * Use datagrams.
28185	                         */
28186	                        if (s < 0) {
28187	                                s = udp_connect();
28188	                                if (s < 0) {
28189	                                        terrno = errno;
28190	#ifdef DEBUG
28191	                                        if (_res.options & RES_DEBUG)
28192	                                            perror("udp_connect failed");
28193	#endif /* DEBUG */
28194	                                        continue;
28195	                                }
28196	                        }
28197	                        if (udp_sendto(s, buf, buflen, _res.nsaddr_list[ns],
28198	                                _res.nsport_list[ns]) != buflen) {
28199	#ifdef DEBUG
28200	                                if (_res.options & RES_DEBUG)
28201	                                        perror("sendto");
28202	#endif /* DEBUG */
28203	                                continue;
28204	                        }
28205	
28206	                        /*
28207	                         * Wait for reply
28208	                         */
28209	                        timeout= (_res.retrans << try);
28210	                        if (try > 0)
28211	                                timeout /= _res.nscount;
28212	                        if (timeout <= 0)
28213	                                timeout= 1;
28214	wait:
28215	                        if ((resplen= udp_receive(s, answer, anslen, timeout))
28216	                                == -1)
28217	                        {
28218	                                if (errno == EINTR)
28219	                                {
.Op 273 src/lib/ip/res_send.c
28220	                                /*
28221	                                 * timeout
28222	                                 */
28223	#ifdef DEBUG
28224	                                        if (_res.options & RES_DEBUG)
28225	                                                printf("timeout\n");
28226	#endif /* DEBUG */
28227	                                        gotsomewhere = 1;
28228	                                }
28229	                                else
28230	                                {
28231	#ifdef DEBUG
28232	                                if (_res.options & RES_DEBUG)
28233	                                        perror("udp_receive");
28234	#endif /* DEBUG */
28235	                                }
28236	                                continue;
28237	                        }
28238	                        gotsomewhere = 1;
28239	                        if (id != anhp->dh_id) {
28240	                                /*
28241	                                 * response from old query, ignore it
28242	                                 */
28243	#ifdef DEBUG
28244	                                if (_res.options & RES_DEBUG) {
28245	                                        printf("old answer:\n");
28246	                                        __p_query(answer);
28247	                                }
28248	#endif /* DEBUG */
28249	                                goto wait;
28250	                        }
28251	                        if (!(_res.options & RES_IGNTC) &&
28252	                                (anhp->dh_flag1 & DHF_TC)) {
28253	                                /*
28254	                                 * get rest of answer;
28255	                                 * use TCP with same server.
28256	                                 */
28257	#ifdef DEBUG
28258	                                if (_res.options & RES_DEBUG)
28259	                                        printf("truncated answer\n");
28260	#endif /* DEBUG */
28261	                                (void) close(s);
28262	                                s = -1;
28263	                                v_circuit = 1;
28264	                                goto usevc;
28265	                        }
28266	#endif /* !_MINIX */
28267	                }
28268	#ifdef DEBUG
28269	                if (_res.options & RES_DEBUG) {
28270	                        printf("got answer:\n");
28271	                        __p_query(answer);
28272	                }
28273	#endif /* DEBUG */
28274	                /*
28275	                 * If using virtual circuits, we assume that the first server
28276	                 * is preferred * over the rest (i.e. it is on the local
28277	                 * machine) and only keep that one open.
28278	                 * If we have temporarily opened a virtual circuit,
28279	                 * or if we haven't been asked to keep a socket open,
.Ep 274 src/lib/ip/res_send.c
28280	                 * close the socket.
28281	                 */
28282	                if ((v_circuit &&
28283	                    ((_res.options & RES_USEVC) == 0 || ns != 0)) ||
28284	                    (_res.options & RES_STAYOPEN) == 0) {
28285	                        (void) close(s);
28286	                        s = -1;
28287	                }
28288	                return (resplen);
28289	           }
28290	        }
28291	        if (s >= 0) {
28292	                (void) close(s);
28293	                s = -1;
28294	        }
28295	        if (v_circuit == 0)
28296	                if (gotsomewhere == 0)
28297	                        errno = ECONNREFUSED;   /* no nameservers found */
28298	                else
28299	                        errno = ETIMEDOUT;      /* no answer obtained */
28300	        else
28301	                errno = terrno;
28302	        return (-1);
28303	}
	
28305	/*
28306	 * This routine is for closing the socket if a virtual circuit is used and
28307	 * the program wants to close it.  This provides support for endhostent()
28308	 * which expects to close the socket.
28309	 *
28310	 * This routine is not expected to be user visible.
28311	 */
28312	void
28313	_res_close()
28314	{
28315	        if (s != -1) {
28316	                (void) close(s);
28317	                s = -1;
28318	        }
28319	}
	
28321	#if _MINIX
28322	static int tcp_connect(host, port, terrno)
28323	ipaddr_t host;
28324	tcpport_t port;
28325	int *terrno;
28326	{
28327	        char *dev_name;
28328	        int fd;
28329	        int error;
28330	        nwio_tcpconf_t tcpconf;
28331	        nwio_tcpcl_t clopt;
28332	
28333	        dev_name= getenv("TCP_DEVICE");
28334	        if (!dev_name)
28335	                dev_name= TCP_DEVICE;
28336	        fd= open(dev_name, O_RDWR);
28337	        if (fd == -1)
28338	        {
28339	                *terrno= errno;
.Op 275 src/lib/ip/res_send.c
28340	                return -1;
28341	        }
28342	        tcpconf.nwtc_flags= NWTC_EXCL | NWTC_LP_SEL | NWTC_SET_RA | NWTC_SET_RP;
28343	        tcpconf.nwtc_remaddr= host;
28344	        tcpconf.nwtc_remport= port;
28345	        error= ioctl(fd, NWIOSTCPCONF, &tcpconf);
28346	        if (error == -1)
28347	        {
28348	                *terrno= errno;
28349	                close(fd);
28350	                return -1;
28351	        }
28352	        clopt.nwtcl_flags= 0;
28353	        error= ioctl(fd, NWIOTCPCONN, &clopt);
28354	        if (error == -1)
28355	        {
28356	                *terrno= errno;
28357	                close(fd);
28358	                return -1;
28359	        }
28360	        *terrno= 0;
28361	        return fd;
28362	}
	
28364	static int tcpip_writeall(fd, buf, siz)
28365	int fd;
28366	const char *buf;
28367	size_t siz;
28368	{
28369	        size_t siz_org;
28370	        int nbytes;
28371	
28372	        siz_org= siz;
28373	
28374	        while (siz)
28375	        {
28376	                nbytes= write(fd, buf, siz);
28377	                if (nbytes <= 0)
28378	                        return siz_org-siz;
28379	                assert(siz >= nbytes);
28380	                buf += nbytes;
28381	                siz -= nbytes;
28382	        }
28383	        return siz_org;
28384	}
	
	
28387	static int udp_connect()
28388	{
28389	        nwio_udpopt_t udpopt;
28390	        char *dev_name;
28391	        int fd, r, terrno;
28392	
28393	        dev_name= getenv("UDP_DEVICE");
28394	        if (!dev_name)
28395	                dev_name= UDP_DEVICE;
28396	        fd= open(dev_name, O_RDWR);
28397	        if (fd == -1)
28398	                return -1;
28399	
.Ep 276 src/lib/ip/res_send.c
28400	        udpopt.nwuo_flags= NWUO_COPY | NWUO_LP_SEL | NWUO_EN_LOC |
28401	                NWUO_EN_BROAD | NWUO_RP_ANY | NWUO_RA_ANY | NWUO_RWDATALL |
28402	                NWUO_DI_IPOPT;
28403	        r= ioctl(fd, NWIOSUDPOPT, &udpopt);
28404	        if (r == -1)
28405	        {
28406	                terrno= errno;
28407	                close(fd);
28408	                errno= terrno;
28409	                return -1;
28410	        }
28411	        return fd;
28412	}
	
28414	static int udp_sendto(fd, buf, buflen, addr, port)
28415	int fd;
28416	const char *buf;
28417	unsigned buflen;
28418	ipaddr_t addr;
28419	udpport_t port;
28420	{
28421	        char *newbuf;
28422	        udp_io_hdr_t *udp_io_hdr;
28423	        int r, terrno;
28424	
28425	        newbuf= malloc(sizeof(*udp_io_hdr) + buflen);
28426	        if (newbuf == NULL)
28427	        {
28428	                errno= ENOMEM;
28429	                return -1;
28430	        }
28431	        udp_io_hdr= (udp_io_hdr_t *)newbuf;
28432	        udp_io_hdr->uih_dst_addr= addr;
28433	        udp_io_hdr->uih_dst_port= port;
28434	        udp_io_hdr->uih_ip_opt_len= 0;
28435	        udp_io_hdr->uih_data_len= buflen;
28436	
28437	        memcpy(newbuf + sizeof(*udp_io_hdr), buf, buflen);
28438	        r= write(fd, newbuf, sizeof(*udp_io_hdr) + buflen);
28439	        terrno= errno;
28440	        free(newbuf);
28441	        if (r >= sizeof(*udp_io_hdr))
28442	                r -= sizeof(*udp_io_hdr);
28443	        errno= terrno;
28444	        return r;
28445	}
	
28447	static void alarm_handler(sig)
28448	int sig;
28449	{
28450	        signal(SIGALRM, alarm_handler);
28451	        alarm(1);
28452	}
	
28454	static int udp_receive(fd, buf, buflen, timeout)
28455	int fd;
28456	char *buf;
28457	unsigned buflen;
28458	time_t timeout;
28459	{
.Op 277 src/lib/ip/res_send.c
28460	        char *newbuf;
28461	        udp_io_hdr_t *udp_io_hdr;
28462	        int r, terrno;
28463	        void (*u_handler) _ARGS(( int sig ));
28464	        time_t u_timeout;
28465	
28466	        newbuf= malloc(sizeof(*udp_io_hdr) + buflen);
28467	        if (newbuf == NULL)
28468	        {
28469	                errno= ENOMEM;
28470	                return -1;
28471	        }
28472	
28473	        u_handler= signal(SIGALRM, alarm_handler);
28474	        u_timeout= alarm(timeout);
28475	
28476	        r= read(fd, newbuf, sizeof(*udp_io_hdr) + buflen);
28477	        terrno= errno;
28478	
28479	        if (r < 0 || r <= sizeof(*udp_io_hdr))
28480	        {
28481	                if (r > 0)
28482	                        r= 0;
28483	                free(newbuf);
28484	
28485	
28486	                alarm(0);
28487	                signal(SIGALRM, u_handler);
28488	                alarm(u_timeout);
28489	
28490	                errno= terrno;
28491	                return r;
28492	        }
28493	
28494	        memcpy(buf, newbuf + sizeof(*udp_io_hdr), r - sizeof(*udp_io_hdr));
28495	        free(newbuf);
28496	
28497	        alarm(0);
28498	        signal(SIGALRM, u_handler);
28499	        alarm(u_timeout);
28500	
28501	        return r-sizeof(*udp_io_hdr);
28502	}
	
28504	#endif
.Ep 278 src/lib/ip/strcasecmp.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/ip/strcasecmp.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

28600	/*
28601	strcasecmp.c
28602	
28603	Created Oct 14, 1991 by Philip Homburg
28604	*/
28605	
28606	#include <ctype.h>
28607	#include <string.h>
28608	
28609	#ifdef __STDC__
28610	#define _CONST  const
28611	#else
28612	#define _CONST
28613	#endif
28614	
28615	int
28616	strcasecmp(s1, s2)
28617	_CONST char *s1, *s2;
28618	{
28619	        int c1, c2;
28620	        while (c1= toupper(*s1++), c2= toupper(*s2++), c1 == c2 && (c1 & c2))
28621	                ;
28622	        if (c1 & c2)
28623	                return c1 < c2 ? -1 : 1;
28624	        return c1 ? 1 : (c2 ? -1 : 0);
28625	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/liby/main.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

28700	/*-
28701	 * Copyright (c) 1990 The Regents of the University of California.
28702	 * All rights reserved.
28703	 *
28704	 * Redistribution and use in source and binary forms, with or without
28705	 * modification, are permitted provided that the following conditions
28706	 * are met:
28707	 * 1. Redistributions of source code must retain the above copyright
28708	 *    notice, this list of conditions and the following disclaimer.
28709	 * 2. Redistributions in binary form must reproduce the above copyright
28710	 *    notice, this list of conditions and the following disclaimer in the
28711	 *    documentation and/or other materials provided with the distribution.
28712	 * 3. All advertising materials mentioning features or use of this software
28713	 *    must display the following acknowledgement:
28714	 *      This product includes software developed by the University of
28715	 *      California, Berkeley and its contributors.
28716	 * 4. Neither the name of the University nor the names of its contributors
28717	 *    may be used to endorse or promote products derived from this software
28718	 *    without specific prior written permission.
28719	 *
.Op 279 src/lib/liby/main.c
28720	 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
28721	 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
28722	 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
28723	 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
28724	 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
28725	 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
28726	 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
28727	 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
28728	 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
28729	 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
28730	 * SUCH DAMAGE.
28731	 */
28732	
28733	#if defined(LIBC_SCCS) && !defined(lint)
28734	static char sccsid[] = "@(#)main.c      5.3 (Berkeley) 1/13/91";
28735	#endif /* not lint */
28736	
28737	main()
28738	{
28739	        exit(yyparse());
28740	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/liby/yyerror.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

28800	/*-
28801	 * Copyright (c) 1990 The Regents of the University of California.
28802	 * All rights reserved.
28803	 *
28804	 * Redistribution and use in source and binary forms, with or without
28805	 * modification, are permitted provided that the following conditions
28806	 * are met:
28807	 * 1. Redistributions of source code must retain the above copyright
28808	 *    notice, this list of conditions and the following disclaimer.
28809	 * 2. Redistributions in binary form must reproduce the above copyright
28810	 *    notice, this list of conditions and the following disclaimer in the
28811	 *    documentation and/or other materials provided with the distribution.
28812	 * 3. All advertising materials mentioning features or use of this software
28813	 *    must display the following acknowledgement:
28814	 *      This product includes software developed by the University of
28815	 *      California, Berkeley and its contributors.
28816	 * 4. Neither the name of the University nor the names of its contributors
28817	 *    may be used to endorse or promote products derived from this software
28818	 *    without specific prior written permission.
28819	 *
28820	 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
28821	 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
28822	 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
28823	 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
28824	 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
28825	 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
28826	 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
28827	 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
28828	 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
28829	 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
.Ep 280 src/lib/liby/yyerror.c
28830	 * SUCH DAMAGE.
28831	 */
28832	
28833	#if defined(LIBC_SCCS) && !defined(lint)
28834	static char sccsid[] = "@(#)yyerror.c   5.2 (Berkeley) 5/15/90";
28835	#endif /* not lint */
28836	
28837	#include <stdio.h>
28838	
28839	yyerror(msg)
28840	char *msg;
28841	{
28842	        (void)fprintf(stderr, "%s\n", msg);
28843	        return(0);
28844	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/math/localmath.h	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

28900	/*
28901	 * localmath.h - This header is used by the mathematical library.
28902	 */
28903	/* $Header: localmath.h,v 1.2 89/12/18 15:43:05 eck Exp $ */
28904	
28905	/* some constants (Hart & Cheney) */
28906	#define M_PI            3.14159265358979323846264338327950288
28907	#define M_2PI           6.28318530717958647692528676655900576
28908	#define M_3PI_4         2.35619449019234492884698253745962716
28909	#define M_PI_2          1.57079632679489661923132169163975144
28910	#define M_3PI_8         1.17809724509617246442349126872981358
28911	#define M_PI_4          0.78539816339744830961566084581987572
28912	#define M_PI_8          0.39269908169872415480783042290993786
28913	#define M_1_PI          0.31830988618379067153776752674502872
28914	#define M_2_PI          0.63661977236758134307553505349005744
28915	#define M_4_PI          1.27323954473516268615107010698011488
28916	#define M_E             2.71828182845904523536028747135266250
28917	#define M_LOG2E         1.44269504088896340735992468100189213
28918	#define M_LOG10E        0.43429448190325182765112891891660508
28919	#define M_LN2           0.69314718055994530941723212145817657
28920	#define M_LN10          2.30258509299404568401799145468436421
28921	#define M_SQRT2         1.41421356237309504880168872420969808
28922	#define M_1_SQRT2       0.70710678118654752440084436210484904
28923	#define M_EULER         0.57721566490153286060651209008240243
28924	
28925	/* macros for constructing polynomials */
28926	#define POLYNOM1(x, a)  ((a)[1]*(x)+(a)[0])
28927	#define POLYNOM2(x, a)  (POLYNOM1((x),(a)+1)*(x)+(a)[0])
28928	#define POLYNOM3(x, a)  (POLYNOM2((x),(a)+1)*(x)+(a)[0])
28929	#define POLYNOM4(x, a)  (POLYNOM3((x),(a)+1)*(x)+(a)[0])
28930	#define POLYNOM5(x, a)  (POLYNOM4((x),(a)+1)*(x)+(a)[0])
28931	#define POLYNOM6(x, a)  (POLYNOM5((x),(a)+1)*(x)+(a)[0])
28932	#define POLYNOM7(x, a)  (POLYNOM6((x),(a)+1)*(x)+(a)[0])
28933	#define POLYNOM8(x, a)  (POLYNOM7((x),(a)+1)*(x)+(a)[0])
28934	#define POLYNOM9(x, a)  (POLYNOM8((x),(a)+1)*(x)+(a)[0])
.Op 281 src/lib/math/localmath.h
28935	#define POLYNOM10(x, a) (POLYNOM9((x),(a)+1)*(x)+(a)[0])
28936	#define POLYNOM11(x, a) (POLYNOM10((x),(a)+1)*(x)+(a)[0])
28937	#define POLYNOM12(x, a) (POLYNOM11((x),(a)+1)*(x)+(a)[0])
28938	#define POLYNOM13(x, a) (POLYNOM12((x),(a)+1)*(x)+(a)[0])
28939	
28940	#define M_LN_MAX_D      (M_LN2 * DBL_MAX_EXP)
28941	#define M_LN_MIN_D      (M_LN2 * (DBL_MIN_EXP - 1))




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/math/asin.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

29000	/*
29001	 * (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
29002	 * See the copyright notice in the ACK home directory, in the file "Copyright".
29003	 *
29004	 * Author: Ceriel J.H. Jacobs
29005	 */
29006	/* $Header: asin.c,v 1.3 91/03/19 16:38:12 ceriel Exp $ */
29007	
29008	#include        <math.h>
29009	#include        <errno.h>
29010	#include        "localmath.h"
29011	
29012	static double
29013	asin_acos(double x, int cosfl)
29014	{
29015	        int negative = x < 0;
29016	        int     i;
29017	        double  g;
29018	        static double p[] = {
29019	                -0.27368494524164255994e+2,
29020	                 0.57208227877891731407e+2,
29021	                -0.39688862997540877339e+2,
29022	                 0.10152522233806463645e+2,
29023	                -0.69674573447350646411e+0
29024	        };
29025	        static double q[] = {
29026	                -0.16421096714498560795e+3,
29027	                 0.41714430248260412556e+3,
29028	                -0.38186303361750149284e+3,
29029	                 0.15095270841030604719e+3,
29030	                -0.23823859153670238830e+2,
29031	                 1.0
29032	        };
29033	
29034	        if (__IsNan(x)) {
29035	                errno = EDOM;
29036	                return x;
29037	        }
29038	
29039	        if (negative) {
29040	                x = -x;
29041	        }
29042	        if (x > 0.5) {
29043	                i = 1;
29044	                if (x > 1) {
.Ep 282 src/lib/math/asin.c
29045	                        errno = EDOM;
29046	                        return 0;
29047	                }
29048	                g = 0.5 - 0.5 * x;
29049	                x = - sqrt(g);
29050	                x += x;
29051	        }
29052	        else {
29053	                /* ??? avoid underflow ??? */
29054	                i = 0;
29055	                g = x * x;
29056	        }
29057	        x += x * g * POLYNOM4(g, p) / POLYNOM5(g, q);
29058	        if (cosfl) {
29059	                if (! negative) x = -x;
29060	        }
29061	        if ((cosfl == 0) == (i == 1)) {
29062	                x = (x + M_PI_4) + M_PI_4;
29063	        }
29064	        else if (cosfl && negative && i == 1) {
29065	                x = (x + M_PI_2) + M_PI_2;
29066	        }
29067	        if (! cosfl && negative) x = -x;
29068	        return x;
29069	}
	
29071	double
29072	asin(double x)
29073	{
29074	        return asin_acos(x, 0);
29075	}
	
29077	double
29078	acos(double x)
29079	{
29080	        return asin_acos(x, 1);
29081	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/math/atan.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

29100	/*
29101	 * (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
29102	 * See the copyright notice in the ACK home directory, in the file "Copyright".
29103	 *
29104	 * Author: Ceriel J.H. Jacobs
29105	 */
29106	/* $Header: atan.c,v 1.3 91/03/19 16:38:21 ceriel Exp $ */
29107	
29108	#include        <float.h>
29109	#include        <math.h>
29110	#include        <errno.h>
29111	#include        "localmath.h"
29112	
29113	double
29114	atan(double x)
.Op 283 src/lib/math/atan.c
29115	{
29116	        /*      Algorithm and coefficients from:
29117	                        "Software manual for the elementary functions"
29118	                        by W.J. Cody and W. Waite, Prentice-Hall, 1980
29119	        */
29120	
29121	        static double p[] = {
29122	                -0.13688768894191926929e+2,
29123	                -0.20505855195861651981e+2,
29124	                -0.84946240351320683534e+1,
29125	                -0.83758299368150059274e+0
29126	        };
29127	        static double q[] = {
29128	                 0.41066306682575781263e+2,
29129	                 0.86157349597130242515e+2,
29130	                 0.59578436142597344465e+2,
29131	                 0.15024001160028576121e+2,
29132	                 1.0
29133	        };
29134	        static double a[] = {
29135	                0.0,
29136	                0.52359877559829887307710723554658381,  /* pi/6 */
29137	                M_PI_2,
29138	                1.04719755119659774615421446109316763   /* pi/3 */
29139	        };
29140	
29141	        int     neg = x < 0;
29142	        int     n;
29143	        double  g;
29144	
29145	        if (__IsNan(x)) {
29146	                errno = EDOM;
29147	                return x;
29148	        }
29149	        if (neg) {
29150	                x = -x;
29151	        }
29152	        if (x > 1.0) {
29153	                x = 1.0/x;
29154	                n = 2;
29155	        }
29156	        else    n = 0;
29157	
29158	        if (x > 0.26794919243112270647) {       /* 2-sqtr(3) */
29159	                n = n + 1;
29160	                x = (((0.73205080756887729353*x-0.5)-0.5)+x)/
29161	                        (1.73205080756887729353+x);
29162	        }
29163	
29164	        /* ??? avoid underflow ??? */
29165	
29166	        g = x * x;
29167	        x += x * g * POLYNOM3(g, p) / POLYNOM4(g, q);
29168	        if (n > 1) x = -x;
29169	        x += a[n];
29170	        return neg ? -x : x;
29171	}
.Ep 284 src/lib/math/atan2.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/math/atan2.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

29200	/*
29201	 * (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
29202	 * See the copyright notice in the ACK home directory, in the file "Copyright".
29203	 *
29204	 * Author: Ceriel J.H. Jacobs
29205	 */
29206	/* $Header: atan2.c,v 1.2 89/12/18 15:42:35 eck Exp $ */
29207	
29208	#include        <math.h>
29209	#include        <errno.h>
29210	#include        "localmath.h"
29211	
29212	double
29213	atan2(double y, double x)
29214	{
29215	        double absx, absy, val;
29216	
29217	        if (x == 0 && y == 0) {
29218	                errno = EDOM;
29219	                return 0;
29220	        }
29221	        absy = y < 0 ? -y : y;
29222	        absx = x < 0 ? -x : x;
29223	        if (absy - absx == absy) {
29224	                /* x negligible compared to y */
29225	                return y < 0 ? -M_PI_2 : M_PI_2;
29226	        }
29227	        if (absx - absy == absx) {
29228	                /* y negligible compared to x */
29229	                val = 0.0;
29230	        }
29231	        else    val = atan(y/x);
29232	        if (x > 0) {
29233	                /* first or fourth quadrant; already correct */
29234	                return val;
29235	        }
29236	        if (y < 0) {
29237	                /* third quadrant */
29238	                return val - M_PI;
29239	        }
29240	        return val + M_PI;
29241	}
.Op 285 src/lib/math/ceil.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/math/ceil.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

29300	/*
29301	 * (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
29302	 * See the copyright notice in the ACK home directory, in the file "Copyright".
29303	 *
29304	 * Author: Ceriel J.H. Jacobs
29305	 */
29306	/* $Header: ceil.c,v 1.1 89/05/10 16:00:46 eck Exp $ */
29307	
29308	#include        <math.h>
29309	
29310	double
29311	ceil(double x)
29312	{
29313	        double val;
29314	
29315	        return modf(x, &val) > 0 ? val + 1.0 : val ;
29316	        /*      this also works if modf always returns a positive
29317	                fractional part
29318	        */
29319	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/math/exp.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

29400	/*
29401	 * (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
29402	 * See the copyright notice in the ACK home directory, in the file "Copyright".
29403	 *
29404	 * Author: Ceriel J.H. Jacobs
29405	 */
29406	/* $Header: exp.c,v 1.5 91/03/19 16:38:29 ceriel Exp $ */
29407	
29408	#include        <math.h>
29409	#include        <float.h>
29410	#include        <errno.h>
29411	#include        "localmath.h"
29412	
29413	
29414	double
29415	exp(double x)
29416	{
29417	        /*      Algorithm and coefficients from:
29418	                        "Software manual for the elementary functions"
29419	                        by W.J. Cody and W. Waite, Prentice-Hall, 1980
29420	        */
29421	
29422	        static double p[] = {
29423	                0.25000000000000000000e+0,
29424	                0.75753180159422776666e-2,
.Ep 286 src/lib/math/exp.c
29425	                0.31555192765684646356e-4
29426	        };
29427	
29428	        static double q[] = {
29429	                0.50000000000000000000e+0,
29430	                0.56817302698551221787e-1,
29431	                0.63121894374398503557e-3,
29432	                0.75104028399870046114e-6
29433	        };
29434	        double  xn, g;
29435	        int     n;
29436	        int     negative = x < 0;
29437	
29438	        if (__IsNan(x)) {
29439	                errno = EDOM;
29440	                return x;
29441	        }
29442	        if (x < M_LN_MIN_D) {
29443	                errno = ERANGE;
29444	                return 0.0;
29445	        }
29446	        if (x > M_LN_MAX_D) {
29447	                errno = ERANGE;
29448	                return HUGE_VAL;
29449	        }
29450	
29451	        if (negative) x = -x;
29452	 
29453	        /* ??? avoid underflow ??? */
29454	
29455	        n = x * M_LOG2E + 0.5;  /* 1/ln(2) = log2(e), 0.5 added for rounding */
29456	        xn = n;
29457	        {
29458	                double  x1 = (long) x;
29459	                double  x2 = x - x1;
29460	
29461	                g = ((x1-xn*0.693359375)+x2) - xn*(-2.1219444005469058277e-4);
29462	        }
29463	        if (negative) {
29464	                g = -g;
29465	                n = -n;
29466	        }
29467	        xn = g * g;
29468	        x = g * POLYNOM2(xn, p);
29469	        n += 1;
29470	        return (ldexp(0.5 + x/(POLYNOM3(xn, q) - x), n));
29471	}
.Op 287 src/lib/math/fabs.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/math/fabs.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

29500	/*
29501	 * (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
29502	 * See the copyright notice in the ACK home directory, in the file "Copyright".
29503	 *
29504	 * Author: Ceriel J.H. Jacobs
29505	 */
29506	/* $Header: fabs.c,v 1.1 89/05/10 16:01:17 eck Exp $ */
29507	
29508	double
29509	fabs(double x)
29510	{
29511	        return  x < 0 ? -x : x;
29512	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/math/floor.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

29600	/*
29601	 * (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
29602	 * See the copyright notice in the ACK home directory, in the file "Copyright".
29603	 *
29604	 * Author: Ceriel J.H. Jacobs
29605	 */
29606	/* $Header: floor.c,v 1.1 89/05/10 16:01:29 eck Exp $ */
29607	
29608	#include        <math.h>
29609	
29610	double
29611	floor(double x)
29612	{
29613	        double val;
29614	
29615	        return modf(x, &val) < 0 ? val - 1.0 : val ;
29616	        /*      this also works if modf always returns a positive
29617	                fractional part
29618	        */
29619	}
.Ep 288 src/lib/math/fmod.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/math/fmod.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

29700	/*
29701	 * (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
29702	 * See the copyright notice in the ACK home directory, in the file "Copyright".
29703	 *
29704	 * Author: Hans van Eck
29705	 */
29706	/* $Header: fmod.c,v 1.1 89/12/18 15:41:53 eck Exp $ */
29707	
29708	#include        <math.h>
29709	#include        <errno.h>
29710	
29711	double
29712	fmod(double x, double y)
29713	{
29714	        long    i;
29715	        double val;
29716	        double frac;
29717	
29718	        if (y == 0) {
29719	                errno = EDOM;
29720	                return 0;
29721	        }
29722	        frac = modf( x / y, &val);
29723	
29724	        return frac * y;
29725	
29726	/*
29727	        val = x / y;
29728	        if (val > LONG_MIN && val < LONG_MAX) {
29729	                i = val;
29730	                return x - i * y;
29731	        }
29732	*/
29733	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/math/frexp.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

29800	#
29801	.sect .text; .sect .rom; .sect .data; .sect .bss
29802	.extern _frexp
29803	.sect .text
29804	_frexp:
29805	#if __i386
29806	        push    ebp
29807	        mov     ebp, esp
29808	        push    12(ebp)
29809	        push    8(ebp)
29810	        mov     eax, esp
29811	        add     eax, -4
29812	        push    eax
29813	        call    .fef8
29814	        mov     eax, 16(ebp)
.Op 289 src/lib/math/frexp.s
29815	        pop     (eax)
29816	        pop     eax
29817	        pop     edx
29818	        leave
29819	        ret
29820	#else /* i86 */
29821	        push    bp
29822	        mov     bp, sp
29823	        lea     bx, 4(bp)
29824	        mov     cx, #8
29825	        call    .loi
29826	        mov     ax, sp
29827	        add     ax, #-2
29828	        push    ax
29829	        call    .fef8
29830	        mov     bx, 12(bp)
29831	        pop     (bx)
29832	        call    .ret8
29833	        jmp     .cret
29834	#endif






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/math/hugeval.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

29900	/*
29901	 * (c) copyright 1990 by the Vrije Universiteit, Amsterdam, The Netherlands.
29902	 * See the copyright notice in the ACK home directory, in the file "Copyright".
29903	 *
29904	 * Author: Hans van Eck
29905	 */
29906	/* $Header: hugeval.c,v 1.1 90/10/24 14:29:42 eck Exp $ */
29907	#include        <math.h>
29908	
29909	double
29910	__huge_val(void)
29911	{
29912	        return 1.0e+1000;       /* This will generate a warning */
29913	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/math/isnan.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

30000	int __IsNan(double d)
30001	{
30002	#if defined(vax) || defined(pdp)
30003	#else
30004	        float f = d;
30005	
30006	        if ((*((long *) &f) & 0x7f800000) == 0x7f800000 &&
30007	            (*((long *) &f) & 0x007fffff) != 0) return 1;
30008	#endif
30009	        return 0;
.Ep 290 src/lib/math/isnan.c
30010	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/math/ldexp.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

30100	/*
30101	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
30102	 * See the copyright notice in the ACK home directory, in the file "Copyright".
30103	 */
30104	/* $Header: ldexp.c,v 1.5 91/03/19 16:38:37 ceriel Exp $ */
30105	
30106	#include        <math.h>
30107	#include        <float.h>
30108	#include        <errno.h>
30109	
30110	double
30111	ldexp(double fl, int exp)
30112	{
30113	        int sign = 1;
30114	        int currexp;
30115	
30116	        if (__IsNan(fl)) {
30117	                errno = EDOM;
30118	                return fl;
30119	        }
30120	        if (fl == 0.0) return 0.0;
30121	        if (fl<0) {
30122	                fl = -fl;
30123	                sign = -1;
30124	        }
30125	        if (fl > DBL_MAX) {             /* for infinity */
30126	                errno = ERANGE;
30127	                return sign * fl;
30128	        }
30129	        fl = frexp(fl,&currexp);
30130	        exp += currexp;
30131	        if (exp > 0) {
30132	                if (exp > DBL_MAX_EXP) {
30133	                        errno = ERANGE;
30134	                        return sign * HUGE_VAL;
30135	                }
30136	                while (exp>30) {
30137	                        fl *= (double) (1L << 30);
30138	                        exp -= 30;
30139	                }
30140	                fl *= (double) (1L << exp);
30141	        }
30142	        else    {
30143	                /* number need not be normalized */
30144	                if (exp < DBL_MIN_EXP - DBL_MANT_DIG) {
30145	                        return 0.0;
30146	                }
30147	                while (exp<-30) {
30148	                        fl /= (double) (1L << 30);
30149	                        exp += 30;
.Op 291 src/lib/math/ldexp.c
30150	                }
30151	                fl /= (double) (1L << -exp);
30152	        }
30153	        return sign * fl;
30154	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/math/log.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

30200	/*
30201	 * (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
30202	 * See the copyright notice in the ACK home directory, in the file "Copyright".
30203	 *
30204	 * Author: Ceriel J.H. Jacobs
30205	 */
30206	/* $Header: log.c,v 1.3 91/03/19 16:38:45 ceriel Exp $ */
30207	
30208	#include        <math.h>
30209	#include        <float.h>
30210	#include        <errno.h>
30211	#include        "localmath.h"
30212	
30213	double
30214	log(double x)
30215	{
30216	        /*      Algorithm and coefficients from:
30217	                        "Software manual for the elementary functions"
30218	                        by W.J. Cody and W. Waite, Prentice-Hall, 1980
30219	        */
30220	        static double a[] = {
30221	                -0.64124943423745581147e2,
30222	                 0.16383943563021534222e2,
30223	                -0.78956112887491257267e0
30224	        };
30225	        static double b[] = {
30226	                -0.76949932108494879777e3,
30227	                 0.31203222091924532844e3,
30228	                -0.35667977739034646171e2,
30229	                 1.0
30230	        };
30231	
30232	        double  znum, zden, z, w;
30233	        int     exponent;
30234	
30235	        if (__IsNan(x)) {
30236	                errno = EDOM;
30237	                return x;
30238	        }
30239	        if (x < 0) {
30240	                errno = EDOM;
30241	                return -HUGE_VAL;
30242	        }
30243	        else if (x == 0) {
30244	                errno = ERANGE;
.Ep 292 src/lib/math/log.c
30245	                return -HUGE_VAL;
30246	        }
30247	
30248	        if (x <= DBL_MAX) {
30249	        }
30250	        else return x;  /* for infinity and Nan */
30251	        x = frexp(x, &exponent);
30252	        if (x > M_1_SQRT2) {
30253	                znum = (x - 0.5) - 0.5;
30254	                zden = x * 0.5 + 0.5;
30255	        }
30256	        else {
30257	                znum = x - 0.5;
30258	                zden = znum * 0.5 + 0.5;
30259	                exponent--;
30260	        }
30261	        z = znum/zden; w = z * z;
30262	        x = z + z * w * (POLYNOM2(w,a)/POLYNOM3(w,b));
30263	        z = exponent;
30264	        x += z * (-2.121944400546905827679e-4);
30265	        return x + z * 0.693359375;
30266	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/math/log10.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

30300	/*
30301	 * (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
30302	 * See the copyright notice in the ACK home directory, in the file "Copyright".
30303	 *
30304	 * Author: Ceriel J.H. Jacobs
30305	 */
30306	/* $Header: log10.c,v 1.3 91/03/19 16:38:55 ceriel Exp $ */
30307	
30308	#include        <math.h>
30309	#include        <errno.h>
30310	#include        "localmath.h"
30311	
30312	double
30313	log10(double x)
30314	{
30315	        if (__IsNan(x)) {
30316	                errno = EDOM;
30317	                return x;
30318	        }
30319	        if (x < 0) {
30320	                errno = EDOM;
30321	                return -HUGE_VAL;
30322	        }
30323	        else if (x == 0) {
30324	                errno = ERANGE;
30325	                return -HUGE_VAL;
30326	        }
30327	
30328	        return log(x) / M_LN10;
30329	}
.Op 293 src/lib/math/modf.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/math/modf.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

30400	#
30401	.sect .text; .sect .rom; .sect .data; .sect .bss
30402	.extern _modf
30403	.sect .text
30404	_modf:
30405	#if __i386
30406	        push    ebp
30407	        mov     ebp, esp
30408	        push    12(ebp)
30409	        push    8(ebp)
30410	        push    1
30411	        push    4
30412	        call    .cif8
30413	        mov     eax, esp
30414	        push    eax
30415	        call    .fif8
30416	        pop     ecx
30417	        mov     edx, 16(ebp)
30418	        pop     ecx
30419	        pop     ebx
30420	        mov     0(edx), ecx
30421	        mov     4(edx), ebx
30422	        pop     eax
30423	        pop     edx
30424	        leave
30425	        ret
30426	#else /* i86 */
30427	        push    bp
30428	        mov     bp, sp
30429	        lea     bx, 4(bp)
30430	        mov     cx, #8
30431	        call    .loi
30432	        mov     dx, #1
30433	        push    dx
30434	        push    dx
30435	        push    dx
30436	        mov     ax, #2
30437	        push    ax
30438	        call    .cif8
30439	        mov     ax, sp
30440	        push    ax
30441	        call    .fif8
30442	        pop     bx
30443	        mov     bx, 12(bp)
30444	        mov     cx, #8
30445	        call    .sti
30446	        call    .ret8
30447	        jmp     .cret
30448	#endif
.Ep 294 src/lib/math/pow.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/math/pow.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

30500	/*
30501	 * (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
30502	 * See the copyright notice in the ACK home directory, in the file "Copyright".
30503	 *
30504	 * Author: Ceriel J.H. Jacobs
30505	 */
30506	/* $Header: pow.c,v 1.3 90/08/28 13:59:36 eck Exp $ */
30507	
30508	#include        <math.h>
30509	#include        <float.h>
30510	#include        <errno.h>
30511	#include        "localmath.h"
30512	
30513	double
30514	pow(double x, double y)
30515	{
30516	        /*      Simple version for now. The Cody and Waite book has
30517	                a very complicated, much more precise version, but
30518	                this version has machine-dependent arrays A1 and A2,
30519	                and I don't know yet how to solve this ???
30520	        */
30521	        double dummy;
30522	        int     result_neg = 0;
30523	
30524	        if ((x == 0 && y == 0) ||
30525	            (x < 0 && modf(y, &dummy) != 0)) {
30526	                errno = EDOM;
30527	                return 0;
30528	        }
30529	
30530	        if (x == 0) return x;
30531	
30532	        if (x < 0) {
30533	                if (modf(y/2.0, &dummy) != 0) {
30534	                        /* y was odd */
30535	                        result_neg = 1;
30536	                }
30537	                x = -x;
30538	        }
30539	        x = log(x);
30540	
30541	        if (x < 0) {
30542	                x = -x;
30543	                y = -y;
30544	        }
30545	        /* Beware of overflow in the multiplication */
30546	        if (x > 1.0 && y > DBL_MAX/x) {
30547	                errno = ERANGE;
30548	                return result_neg ? -HUGE_VAL : HUGE_VAL;
30549	        }
30550	
30551	        x = exp(x * y);
30552	        return result_neg ? -x : x;
30553	}
.Op 295 src/lib/math/sin.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/math/sin.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

30600	/*
30601	 * (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
30602	 * See the copyright notice in the ACK home directory, in the file "Copyright".
30603	 *
30604	 * Author: Ceriel J.H. Jacobs
30605	 */
30606	/* $Header: sin.c,v 1.3 91/03/19 16:39:04 ceriel Exp $ */
30607	
30608	#include        <math.h>
30609	#include        <float.h>
30610	#include        <errno.h>
30611	#include        "localmath.h"
30612	
30613	static double
30614	sinus(double x, int cos_flag)
30615	{
30616	        /*      Algorithm and coefficients from:
30617	                        "Software manual for the elementary functions"
30618	                        by W.J. Cody and W. Waite, Prentice-Hall, 1980
30619	        */
30620	
30621	        static double r[] = {
30622	                -0.16666666666666665052e+0,
30623	                 0.83333333333331650314e-2,
30624	                -0.19841269841201840457e-3,
30625	                 0.27557319210152756119e-5,
30626	                -0.25052106798274584544e-7,
30627	                 0.16058936490371589114e-9,
30628	                -0.76429178068910467734e-12,
30629	                 0.27204790957888846175e-14
30630	        };
30631	
30632	        double  y;
30633	        int     neg = 1;
30634	
30635	        if (__IsNan(x)) {
30636	                errno = EDOM;
30637	                return x;
30638	        }
30639	        if (x < 0) {
30640	                x = -x;
30641	                neg = -1;
30642	        }
30643	        if (cos_flag) {
30644	                neg = 1;
30645	                y = M_PI_2 + x;
30646	        }
30647	        else    y = x;
30648	
30649	        /* ??? avoid loss of significance, if y is too large, error ??? */
30650	
30651	        y = y * M_1_PI + 0.5;
30652	
30653	        if (y >= DBL_MAX/M_PI) return 0.0;
30654	
.Ep 296 src/lib/math/sin.c
30655	        /*      Use extended precision to calculate reduced argument.
30656	                Here we used 12 bits of the mantissa for a1.
30657	                Also split x in integer part x1 and fraction part x2.
30658	        */
30659	#define A1 3.1416015625
30660	#define A2 -8.908910206761537356617e-6
30661	        {
30662	                double x1, x2;
30663	
30664	                modf(y, &y);
30665	                if (modf(0.5*y, &x1)) neg = -neg;
30666	                if (cos_flag) y -= 0.5;
30667	                x2 = modf(x, &x1);
30668	                x = x1 - y * A1;
30669	                x += x2;
30670	                x -= y * A2;
30671	#undef A1
30672	#undef A2
30673	        }
30674	 
30675	        if (x < 0) {
30676	                neg = -neg;
30677	                x = -x;
30678	        }
30679	
30680	        /* ??? avoid underflow ??? */
30681	
30682	        y = x * x;
30683	        x += x * y * POLYNOM7(y, r);
30684	        return neg==-1 ? -x : x;
30685	}
	
30687	double
30688	sin(double x)
30689	{
30690	        return sinus(x, 0);
30691	}
	
30693	double
30694	cos(double x)
30695	{
30696	        if (x < 0) x = -x;
30697	        return sinus(x, 1);
30698	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/math/sinh.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

30700	/*
30701	 * (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
30702	 * See the copyright notice in the ACK home directory, in the file "Copyright".
30703	 *
30704	 * Author: Ceriel J.H. Jacobs
30705	 */
30706	/* $Header: sinh.c,v 1.3 91/03/19 16:39:12 ceriel Exp $ */
30707	
30708	#include        <math.h>
30709	#include        <float.h>
.Op 297 src/lib/math/sinh.c
30710	#include        <errno.h>
30711	#include        "localmath.h"
30712	
30713	static double
30714	sinh_cosh(double x, int cosh_flag)
30715	{
30716	        /*      Algorithm and coefficients from:
30717	                        "Software manual for the elementary functions"
30718	                        by W.J. Cody and W. Waite, Prentice-Hall, 1980
30719	        */
30720	
30721	        static double p[] = {
30722	                -0.35181283430177117881e+6,
30723	                -0.11563521196851768270e+5,
30724	                -0.16375798202630751372e+3,
30725	                -0.78966127417357099479e+0
30726	        };
30727	        static double q[] = {
30728	                -0.21108770058106271242e+7,
30729	                 0.36162723109421836460e+5,
30730	                -0.27773523119650701167e+3,
30731	                 1.0
30732	        };
30733	        int     negative = x < 0;
30734	        double  y = negative ? -x : x;
30735	
30736	        if (__IsNan(x)) {
30737	                errno = EDOM;
30738	                return x;
30739	        }
30740	        if (! cosh_flag && y <= 1.0) {
30741	                /* ??? check for underflow ??? */
30742	                y = y * y;
30743	                return x + x * y * POLYNOM3(y, p)/POLYNOM3(y,q);
30744	        }
30745	
30746	        if (y >= M_LN_MAX_D) {
30747	                /* exp(y) would cause overflow */
30748	#define LNV     0.69316101074218750000e+0
30749	#define VD2M1   0.52820835025874852469e-4
30750	                double  w = y - LNV;
30751	                
30752	                if (w < M_LN_MAX_D+M_LN2-LNV) {
30753	                        x = exp(w);
30754	                        x += VD2M1 * x;
30755	                }
30756	                else {
30757	                        errno = ERANGE;
30758	                        x = HUGE_VAL;
30759	                }
30760	        }
30761	        else {
30762	                double  z = exp(y);
30763	                
30764	                x = 0.5 * (z + (cosh_flag ? 1.0 : -1.0)/z);
30765	        }
30766	        return negative ? -x : x;
30767	}
	
30769	double
.Ep 298 src/lib/math/sinh.c
30770	sinh(double x)
30771	{
30772	        return sinh_cosh(x, 0);
30773	}
	
30775	double
30776	cosh(double x)
30777	{
30778	        if (x < 0) x = -x;
30779	        return sinh_cosh(x, 1);
30780	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/math/sqrt.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

30800	/*
30801	 * (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
30802	 * See the copyright notice in the ACK home directory, in the file "Copyright".
30803	 *
30804	 * Author: Ceriel J.H. Jacobs
30805	 */
30806	/* $Header: sqrt.c,v 1.3 91/03/19 16:39:21 ceriel Exp $ */
30807	
30808	#include        <math.h>
30809	#include        <float.h>
30810	#include        <errno.h>
30811	
30812	#define NITER   5
30813	
30814	double
30815	sqrt(double x)
30816	{
30817	        int exponent;
30818	        double val;
30819	
30820	        if (__IsNan(x)) {
30821	                errno = EDOM;
30822	                return x;
30823	        }
30824	        if (x <= 0) {
30825	                if (x < 0) errno = EDOM;
30826	                return 0;
30827	        }
30828	
30829	        if (x > DBL_MAX) return x;      /* for infinity */
30830	
30831	        val = frexp(x, &exponent);
30832	        if (exponent & 1) {
30833	                exponent--;
30834	                val *= 2;
30835	        }
30836	        val = ldexp(val + 1.0, exponent/2 - 1);
30837	        /* was: val = (val + 1.0)/2.0; val = ldexp(val, exponent/2); */
30838	        for (exponent = NITER - 1; exponent >= 0; exponent--) {
30839	                val = (val + x / val) / 2.0;
.Op 299 src/lib/math/sqrt.c
30840	        }
30841	        return val;
30842	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/math/tan.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

30900	/*
30901	 * (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
30902	 * See the copyright notice in the ACK home directory, in the file "Copyright".
30903	 *
30904	 * Author: Ceriel J.H. Jacobs
30905	 */
30906	/* $Header: tan.c,v 1.3 91/03/19 16:39:30 ceriel Exp $ */
30907	
30908	#include        <math.h>
30909	#include        <float.h>
30910	#include        <errno.h>
30911	#include        "localmath.h"
30912	
30913	double
30914	tan(double x)
30915	{
30916	        /*      Algorithm and coefficients from:
30917	                        "Software manual for the elementary functions"
30918	                        by W.J. Cody and W. Waite, Prentice-Hall, 1980
30919	        */
30920	
30921	        int negative = x < 0;
30922	        int invert = 0;
30923	        double  y;
30924	        static double   p[] = {
30925	                 1.0,
30926	                -0.13338350006421960681e+0,
30927	                 0.34248878235890589960e-2,
30928	                -0.17861707342254426711e-4
30929	        };
30930	        static double   q[] = {
30931	                 1.0,
30932	                -0.46671683339755294240e+0,
30933	                 0.25663832289440112864e-1,
30934	                -0.31181531907010027307e-3,
30935	                 0.49819433993786512270e-6
30936	        };
30937	
30938	        if (__IsNan(x)) {
30939	                errno = EDOM;
30940	                return x;
30941	        }
30942	        if (negative) x = -x;
30943	 
30944	        /* ??? avoid loss of significance, error if x is too large ??? */
30945	
30946	        y = x * M_2_PI + 0.5;
30947	
30948	        if (y >= DBL_MAX/M_PI_2) return 0.0;
30949	
.Ep 300 src/lib/math/tan.c
30950	        /*      Use extended precision to calculate reduced argument.
30951	                Here we used 12 bits of the mantissa for a1.
30952	                Also split x in integer part x1 and fraction part x2.
30953	        */
30954	    #define A1 1.57080078125
30955	    #define A2 -4.454455103380768678308e-6
30956	        {
30957	                double x1, x2;
30958	
30959	                modf(y, &y);
30960	                if (modf(0.5*y, &x1)) invert = 1;
30961	                x2 = modf(x, &x1);
30962	                x = x1 - y * A1;
30963	                x += x2;
30964	                x -= y * A2;
30965	    #undef A1
30966	    #undef A2
30967	        }
30968	
30969	        /* ??? avoid underflow ??? */
30970	        y = x * x;
30971	        x += x * y * POLYNOM2(y, p+1);
30972	        y = POLYNOM4(y, q);
30973	        if (negative) x = -x;
30974	        return invert ? -y/x : x/y;
30975	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/math/tanh.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

31000	/*
31001	 * (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
31002	 * See the copyright notice in the ACK home directory, in the file "Copyright".
31003	 *
31004	 * Author: Ceriel J.H. Jacobs
31005	 */
31006	/* $Header: tanh.c,v 1.3 91/03/19 16:39:40 ceriel Exp $ */
31007	
31008	#include        <float.h>
31009	#include        <math.h>
31010	#include        <errno.h>
31011	#include        "localmath.h"
31012	
31013	double
31014	tanh(double x)
31015	{
31016	        /*      Algorithm and coefficients from:
31017	                        "Software manual for the elementary functions"
31018	                        by W.J. Cody and W. Waite, Prentice-Hall, 1980
31019	        */
31020	
31021	        static double p[] = {
31022	                -0.16134119023996228053e+4,
31023	                -0.99225929672236083313e+2,
31024	                -0.96437492777225469787e+0
.Op 301 src/lib/math/tanh.c
31025	        };
31026	        static double q[] = {
31027	                 0.48402357071988688686e+4,
31028	                 0.22337720718962312926e+4,
31029	                 0.11274474380534949335e+3,
31030	                 1.0
31031	        };
31032	        int     negative = x < 0;
31033	
31034	        if (__IsNan(x)) {
31035	                errno = EDOM;
31036	                return x;
31037	        }
31038	        if (negative) x = -x;
31039	
31040	        if (x >= 0.5*M_LN_MAX_D) {
31041	                x = 1.0;
31042	        }
31043	#define LN3D2   0.54930614433405484570e+0       /* ln(3)/2 */
31044	        else if (x > LN3D2) {
31045	                x = 0.5 - 1.0/(exp(x+x)+1.0);
31046	                x += x;
31047	        }
31048	        else {
31049	                /* ??? avoid underflow ??? */
31050	                double g = x*x;
31051	                x += x * g * POLYNOM2(g, p)/POLYNOM3(g, q);
31052	        }
31053	        return negative ? -x : x;
31054	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/_brk.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

31100	#include <lib.h>
31101	#define brk     _brk
31102	#define sbrk    _sbrk
31103	#include <unistd.h>
31104	
31105	extern char *_brksize;
31106	
31107	/* Both OSF/1 and SYSVR4 man pages specify that brk(2) returns int.
31108	 * However, BSD4.3 specifies that brk() returns char*.  POSIX omits
31109	 * brk() on the grounds that it imposes a memory model on an architecture.
31110	 * For this reason, brk() and sbrk() are not in the lib/posix directory.
31111	 * On the other hand, they are so crucial to correct operation of so many
31112	 * parts of the system, that we have chosen to hide the name brk using _brk,
31113	 * as with system calls.  In this way, if a user inadvertently defines a
31114	 * procedure brk, MINIX may continue to work because the true call is _brk.
31115	 */
31116	PUBLIC int brk(addr)
31117	char *addr;
31118	{
31119	  message m;
.Ep 302 src/lib/other/_brk.c
31120	
31121	  m.m1_p1 = addr;
31122	  if (_syscall(MM, BRK, &m) < 0) return(-1);
31123	  _brksize = m.m2_p1;
31124	  return(0);
31125	}
	
	
31128	PUBLIC char *sbrk(incr)
31129	int incr;
31130	{
31131	  char *newsize, *oldsize;
31132	
31133	  oldsize = _brksize;
31134	  newsize = _brksize + incr;
31135	  if (incr > 0 && newsize < oldsize || incr < 0 && newsize > oldsize)
31136	        return( (char *) -1);
31137	  if (brk(newsize) == 0)
31138	        return(oldsize);
31139	  else
31140	        return( (char *) -1);
31141	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/_longjerr.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

31200	#include <lib.h>
31201	#define longjerr        _longjerr
31202	#define write           _write
31203	#include <minix/minlib.h>
31204	#include <stdlib.h>
31205	#include <string.h>
31206	#include <unistd.h>
31207	
31208	
31209	_PROTOTYPE( void longjerr, (void));
31210	
31211	PUBLIC void longjerr()
31212	{
31213	  static char errmsg[] = "longj error\n";
31214	
31215	  write(2, errmsg, strlen(errmsg));     /* hope it's stderr */
31216	  while(1) abort();             /* XXX - maybe just exit */
31217	}
.Op 303 src/lib/other/_reboot.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/_reboot.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

31300	/* reboot.c - Systemcall interface to mm/signal.c::do_reboot()
31301	
31302	   author: Edvard Tuinder  v892231@si.hhs.NL
31303	 */
31304	
31305	#include <lib.h>
31306	#define reboot  _reboot
31307	#include <unistd.h>
31308	#include <stdarg.h>
31309	
31310	int reboot(int how, ...)
31311	{
31312	  message m;
31313	  va_list ap;
31314	
31315	  va_start(ap, how);
31316	  if ((m.m1_i1 = how) == RBT_MONITOR) {
31317	        m.m1_p1 = va_arg(ap, char *);
31318	        m.m1_i2 = va_arg(ap, size_t);
31319	  }
31320	  va_end(ap);
31321	
31322	  return _syscall(MM, REBOOT, &m);
31323	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/_seekdir.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

31400	/*      seekdir()                                       Author: Kees J. Bot
31401	 *                                                              24 Apr 1989
31402	 */
31403	#define nil 0
31404	#include <lib.h>
31405	#define lseek   _lseek
31406	#define readdir _readdir
31407	#define seekdir _seekdir
31408	#include <sys/types.h>
31409	#include <dirent.h>
31410	#include <unistd.h>
31411	#include <errno.h>
31412	
31413	int seekdir(DIR *dp, off_t pos)
31414	/* Seek to position pos in a directory. */
31415	{
31416	        int off;
31417	
31418	        if (dp == nil) { errno= EBADF; return -1; }
31419	
31420	        dp->_count= 0;
31421	        dp->_ptr= dp->_buf;
31422	
31423	        off= pos & (sizeof(dp->_buf) - 1);
31424	        dp->_pos= pos - off;
.Ep 304 src/lib/other/_seekdir.c
31425	
31426	        if (lseek(dp->_fd, dp->_pos, SEEK_SET) == -1) return -1;
31427	
31428	        while (dp->_pos < pos && readdir(dp) != nil) {}
31429	
31430	        return 0;
31431	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/asynchio.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

31500	/*      asyn_init(), asyn_read(), asyn_write(), asyn_ioctl(),
31501	 *      asyn_wait(), asyn_synch(), asyn_close()
31502	 *                                                      Author: Kees J. Bot
31503	 *                                                              26 Jan 1995
31504	 * Thise are just stub routines that are call compatible with
31505	 * the asynchio(3) library of Minix-vmd.  See asynchio.h.
31506	 */
31507	#define nil 0
31508	#define alarm   _alarm
31509	#define ioctl   _ioctl
31510	#define read    _read
31511	#define sigaction _sigaction
31512	#define sigfillset _sigfillset
31513	#define time    _time
31514	#define write   _write
31515	#include <lib.h>
31516	#include <sys/ioctl.h>
31517	#include <sys/asynchio.h>
31518	#include <stdlib.h>
31519	#include <unistd.h>
31520	#include <string.h>
31521	#include <time.h>
31522	#include <signal.h>
31523	
31524	#define IDLE            0
31525	#define INPROGRESS      1
31526	#define RESULT          2
31527	
31528	#define OP_READ         0
31529	#define OP_WRITE        1
31530	#define OP_IOCTL        2
31531	
31532	static int *asyn_current;
31533	static int asyn_op;
31534	static int asyn_fd;
31535	static int asyn_req;
31536	static void *asyn_data;
31537	static ssize_t asyn_count;
31538	static int asyn_errno;
31539	
31540	void asyn_init(asynchio_t *asyn)
31541	{
31542	        *asyn= IDLE;
31543	}
	
.Op 305 src/lib/other/asynchio.c
31545	static ssize_t operation(int op, asynchio_t *asyn, int fd, int req,
31546	                                                void *data, ssize_t count)
31547	{
31548	        switch (*asyn) {
31549	        case INPROGRESS:
31550	                if (asyn_current != asyn && asyn_op != op) abort();
31551	                /*FALL THROUGH*/
31552	        case IDLE:
31553	                asyn_current= asyn;
31554	                asyn_op= op;
31555	                asyn_fd= fd;
31556	                asyn_req= req;
31557	                asyn_data= data;
31558	                asyn_count= count;
31559	                *asyn= INPROGRESS;
31560	                errno= EINPROGRESS;
31561	                return -1;
31562	        case RESULT:
31563	                if (asyn_current != asyn && asyn_op != op) abort();
31564	                *asyn= IDLE;
31565	                errno= asyn_errno;
31566	                return asyn_count;
31567	        }
31568	}
	
31570	ssize_t asyn_read(asynchio_t *asyn, int fd, void *buf, size_t len)
31571	{
31572	        return operation(OP_READ, asyn, fd, 0, buf, len);
31573	}
	
31575	ssize_t asyn_write(asynchio_t *asyn, int fd, const void *buf, size_t len)
31576	{
31577	        return operation(OP_WRITE, asyn, fd, 0, (void *) buf, len);
31578	}
	
31580	int asyn_ioctl(asynchio_t *asyn, int fd, unsigned long request, void *data)
31581	{
31582	        return operation(OP_IOCTL, asyn, fd, request, data, 0);
31583	}
	
31585	static void time_out(int sig)
31586	{
31587	        alarm(1);
31588	}
	
31590	int asyn_wait(asynchio_t *asyn, int flags, struct timeval *to)
31591	{
31592	        time_t now;
31593	        unsigned old_timer, new_timer;
31594	        struct sigaction old_sa, new_sa;
31595	
31596	        if (*asyn == IDLE) return 0;
31597	        if (asyn_current != asyn || *asyn != INPROGRESS) abort();
31598	        if (flags & ASYN_NONBLOCK) abort();
31599	
31600	        if (to != nil) {
31601	                now= time(nil);
31602	                if (to->tv_sec <= now) { errno= EINTR; return -1; }
31603	                old_timer= alarm(0);
31604	                new_sa.sa_handler= time_out;
.Ep 306 src/lib/other/asynchio.c
31605	                sigfillset(&new_sa.sa_mask);
31606	                new_sa.sa_flags= 0;
31607	                sigaction(SIGALRM, &new_sa, &old_sa);
31608	                new_timer= to->tv_sec - now;
31609	                if (new_timer < old_timer) {
31610	                        new_timer= old_timer;
31611	                }
31612	                alarm(new_timer);
31613	        }
31614	        switch (asyn_op) {
31615	        case OP_READ:
31616	                asyn_count= read(asyn_fd, asyn_data, asyn_count);
31617	                asyn_errno= errno;
31618	                break;
31619	        case OP_WRITE:
31620	                asyn_count= write(asyn_fd, asyn_data, asyn_count);
31621	                asyn_errno= errno;
31622	                break;
31623	        case OP_IOCTL:
31624	                asyn_count= ioctl(asyn_fd, asyn_req, asyn_data);
31625	                asyn_errno= errno;
31626	                break;
31627	        }
31628	        if (to != nil) {
31629	                alarm(0);
31630	                sigaction(SIGALRM, &old_sa, (struct sigaction *)0);
31631	                alarm(old_timer);
31632	        }
31633	
31634	        if (asyn_count == -1 && asyn_errno == EINTR) {
31635	                errno= EINTR;
31636	                return -1;
31637	        } else {
31638	                *asyn= RESULT;
31639	                return 0;
31640	        }
31641	}
	
31643	int asyn_synch(asynchio_t *asyn, int fd)
31644	{
31645	}
	
31647	int asyn_close(asynchio_t *asyn, int fd)
31648	{
31649	        *asyn= IDLE;
31650	}
.Op 307 src/lib/other/bcmp.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/bcmp.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

31700	#include <lib.h>
31701	/* bcmp - Berklix equivalent of memcmp  */
31702	
31703	#include <string.h>
31704	
31705	int bcmp(s1, s2, length)        /* == 0 or != 0 for equality and inequality */ 
31706	_CONST void *s1;
31707	_CONST void *s2;
31708	int length;
31709	{
31710	  return(memcmp((_CONST _VOIDSTAR) s1, (_CONST _VOIDSTAR) s2, (_SIZET) length));
31711	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/bcopy.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

31800	#include <lib.h>
31801	/* bcopy - Berklix equivalent of memcpy  */
31802	
31803	#include <string.h>
31804	
31805	void bcopy(src, dst, length)
31806	_CONST void *src;
31807	void *dst;
31808	int length;
31809	{
31810	  (void) memcpy((_VOIDSTAR) dst, (_CONST _VOIDSTAR) src, (_SIZET) length);
31811	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/bzero.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

31900	#include <lib.h>
31901	/* bzero - Berklix subset of memset  */
31902	
31903	#include <string.h>
31904	
31905	void bzero(dst, length)
31906	void *dst;
31907	int length;
31908	{
31909	  (void) memset((_VOIDSTAR) dst, 0, (_SIZET) length);
31910	}
.Ep 308 src/lib/other/crypt.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/crypt.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

32000	/*      crypt() - one-way password encryption function  Author: Kees J. Bot
32001	 *                                                              7 Feb 1994
32002	 * This routine does not encrypt anything, it uses the pwdauth
32003	 * program to do the hard work.
32004	 */
32005	#define nil 0
32006	#define pipe _pipe
32007	#define fork _fork
32008	#define close _close
32009	#define dup2 _dup2
32010	#define execl _execl
32011	#define read _read
32012	#define _exit __exit
32013	#define write _write
32014	#define waitpid _waitpid
32015	#include <sys/types.h>
32016	#include <unistd.h>
32017	#include <string.h>
32018	#include <stdio.h>
32019	#include <sys/wait.h>
32020	
32021	/* Set-uid root program to read /etc/shadow or encrypt passwords. */
32022	static char PWDAUTH[] = "/usr/lib/pwdauth";
32023	#define LEN     1024
32024	
32025	char *crypt(const char *key, const char *salt)
32026	{
32027	        pid_t pid;
32028	        int status;
32029	        int pfd[2];
32030	        static char pwdata[LEN];
32031	        char *p= pwdata;
32032	        const char *k= key;
32033	        const char *s= salt;
32034	        int n;
32035	
32036	        /* Fill pwdata[] with the key and salt. */
32037	        while ((*p++ = *k++) != 0) if (p == pwdata+LEN-1) goto fail;
32038	        while ((*p++ = *s++) != 0) if (p == pwdata+LEN-0) goto fail;
32039	
32040	        if (pipe(pfd) < 0) goto fail;
32041	
32042	        /* Prefill the pipe. */
32043	        (void) write(pfd[1], pwdata, p - pwdata);
32044	
32045	        switch ((pid= fork())) {
32046	        case -1:
32047	                close(pfd[0]);
32048	                close(pfd[1]);
32049	                goto fail;
32050	        case 0:
32051	                /* Connect both input and output to the pipe. */
32052	                if (pfd[0] != 0) {
32053	                        dup2(pfd[0], 0);
32054	                        close(pfd[0]);
.Op 309 src/lib/other/crypt.c
32055	                }
32056	                if (pfd[1] != 1) {
32057	                        dup2(pfd[1], 1);
32058	                        close(pfd[1]);
32059	                }
32060	
32061	                execl(PWDAUTH, PWDAUTH, (char *) nil);
32062	
32063	                /* No pwdauth?  Fail! */
32064	                (void) read(0, pwdata, LEN);
32065	                _exit(1);
32066	        }
32067	        close(pfd[1]);
32068	
32069	        if (waitpid(pid, &status, 0) < 0 || status != 0) {
32070	                close(pfd[0]);
32071	                goto fail;
32072	        }
32073	
32074	        /* Read and return the result.  Check if it contains exactly one
32075	         * string.
32076	         */
32077	        n= read(pfd[0], pwdata, LEN);
32078	        close(pfd[0]);
32079	        if (n < 0) goto fail;
32080	        p = pwdata + n;
32081	        n = 0;
32082	        while (p > pwdata) if (*--p == 0) n++;
32083	        if (n != 1) goto fail;
32084	        return pwdata;
32085	
32086	fail:
32087	        pwdata[0] = salt[0] ^ 1;                /* make result != salt */
32088	        pwdata[1] = 0;
32089	        return pwdata;
32090	}
	
32092	/*
32093	 * $PchHeader: /mount/hd2/minix/lib/misc/RCS/crypt.c,v 1.3 1994/12/22 13:51:49 philip Exp $
32094	 */






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/ctermid.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

32100	/*  ctermid(3)
32101	 *
32102	 *  Author: Terrence Holm          Aug. 1988
32103	 *
32104	 *
32105	 *  Ctermid(3) returns a pointer to a string naming the controlling
32106	 *  terminal. If <name_space> is NULL then local PRIVATE storage
32107	 *  is used, otherwise <name_space> must point to storage of at
32108	 *  least L_ctermid characters.
32109	 *
.Ep 310 src/lib/other/ctermid.c
32110	 *  Returns a pointer to "/dev/tty".
32111	 */
32112	
32113	#include <lib.h>
32114	#include <string.h>
32115	#include <stdio.h>
32116	
32117	_PROTOTYPE( char *ctermid, (char *name_space));
32118	
32119	#ifndef L_ctermid
32120	#define L_ctermid  9
32121	#endif
32122	
32123	char *ctermid(name_space)
32124	char *name_space;
32125	{
32126	  PRIVATE char termid[L_ctermid];
32127	
32128	  if (name_space == (char *)NULL) name_space = termid;
32129	  strcpy(name_space, "/dev/tty");
32130	  return(name_space);
32131	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/cuserid.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

32200	/*  cuserid(3)
32201	 *
32202	 *  Author: Terrence W. Holm          Sept. 1987
32203	 */
32204	
32205	#include <lib.h>
32206	#include <pwd.h>
32207	#include <string.h>
32208	#include <unistd.h>
32209	#include <stdio.h>
32210	
32211	#ifndef  L_cuserid
32212	#define  L_cuserid   9
32213	#endif
32214	
32215	char *cuserid(user_name)
32216	char *user_name;
32217	{
32218	  PRIVATE char userid[L_cuserid];
32219	  struct passwd *pw_entry;
32220	
32221	  if (user_name == (char *)NULL) user_name = userid;
32222	
32223	  pw_entry = getpwuid(geteuid());
32224	
32225	  if (pw_entry == (struct passwd *)NULL) {
32226	        *user_name = '\0';
32227	        return((char *)NULL);
32228	  }
32229	  strcpy(user_name, pw_entry->pw_name);
.Op 311 src/lib/other/cuserid.c
32230	
32231	  return(user_name);
32232	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/environ.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

32300	/*
32301	 * environ.c - define the variable environ
32302	 */
32303	/* $Header: environ.c,v 1.1 90/09/27 13:39:50 eck Exp $ */
32304	/*
32305	 * This file defines the variable environ and initializes it with a magic
32306	 * value.  The C run-time start-off routine tests whether the variable
32307	 * environ is initialized with this value.  If it is not, it is assumed
32308	 * that it is defined by the user.  Only two bytes are tested, since we
32309	 * don't know the endian-ness and alignment restrictions of the machine.
32310	 * This means that the low-order two-bytes should be equal to the
32311	 * high-order two-bytes on machines with four-byte pointers.  In fact, all
32312	 * the bytes in the pointer are the same, just in case.
32313	 */
32314	
32315	#if _EM_PSIZE==2
32316	char **environ = (char **) 0x5353;
32317	#else
32318	char **environ = (char **) 0x53535353;
32319	#endif






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/errno.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

32400	#include <lib.h>
32401	/* errno.c - declare variable errno             Author: F. Meulenbroeks */
32402	
32403	int errno = 0;


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/execlp.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

32500	/* execlp(3) and execvp(3)
32501	 *
32502	 * Author: Terrence W. Holm      July 1988
32503	 */
32504	
32505	/* FIXES - Dec 1989 - Jan 1990 Bruce Evans.
32506	 *       - Don't use search path when file name contains a '/' *anywhere*.
32507	 *       - Invoke sh(1) on command files.
32508	 *       - Use PATH_MAX and check strings fit in buffer.
32509	 *       - Use stdargs, with the unjustified assumption that va_start() turns
.Ep 312 src/lib/other/execlp.c
32510	 *         the arg list into a char *[].  Strictly, the arg list should be
32511	 *         copied, "wasting" up to ARG_MAX bytes.
32512	 */
32513	
32514	/*  Execlp(3) and execvp(3) are like execl(3) and execv(3),
32515	 *  except that they use the environment variable $PATH as
32516	 *  a search list of possible locations for the executable
32517	 *  file, if <file> does not contain a '/', and they attempt
32518	 *  to run non-binary executable files using sh(1).
32519	 *
32520	 *  The path search list is a list of directory names separated
32521	 *  by ':'s. If a colon appears at the beginning or end of the
32522	 *  list, or two appear together, then an empty prefix is tried.
32523	 *  If $PATH is not in the environment, it defaults to "".
32524	 *
32525	 *  For example, if <file> is "ls", and the $PATH is
32526	 *  ":/usr/local/bin:/bin:/usr/bin", then  ./ls,
32527	 *  /usr/local/bin/ls, /bin/ls and /usr/bin/ls are tried until
32528	 *  an exectable one is found. If the direct attempt to exec it
32529	 *  fails, the arg list is modified to begin with "sh" and the
32530	 *  absolute name of <file>, and an exec of /bin/sh is tried.
32531	 *  If this fails, no further attempts are made.
32532	 *
32533	 *  This function only returns after an error.  It returns -1
32534	 *  and sets errno like execv().
32535	 */
32536	
32537	#include <sys/types.h>
32538	#include <lib.h>
32539	#include <errno.h>
32540	#include <limits.h>
32541	#include <stdlib.h>
32542	#include <string.h>
32543	#include <unistd.h>
32544	#include <stdarg.h>
32545	
32546	#undef NULL
32547	#define NULL 0                  /* kludge for ACK not understanding void * */
32548	
32549	#define MAX_NUM_ARGS 512        /* maximum number of arguments to execvp */
32550	
32551	extern char **environ;          /* environment pointer */
32552	
32553	#ifdef _ANSI
32554	int execlp(const char *file, const char *arg, ...)
32555	#else
32556	int execlp(file)
32557	char *file;
32558	#endif
32559	{
32560	  register va_list argp;
32561	  register int result;
32562	
32563	  va_start(argp, file);
32564	  result = execvp(file, (char **) argp);
32565	  va_end(argp);
32566	  return(result);
32567	}
	
32569	#ifdef _ANSI
.Op 313 src/lib/other/execlp.c
32570	int execvp(const char *file, char *const argv[])
32571	#else
32572	int execvp(file, argv)
32573	char *file;
32574	char **argv;
32575	#endif
32576	{
32577	  int i, best_errno;
32578	  char **envtop;
32579	  size_t flength;
32580	  char *searchpath;
32581	  size_t slength;
32582	  char *split;
32583	  char execpath[PATH_MAX + 1];
32584	  char *arg2[MAX_NUM_ARGS + 3]; /* place to copy argv */
32585	
32586	  /* POSIX requires argv to be immutable.  Unfortunately, we have to change it
32587	   * during execution.  To keep POSIX happy, a copy is made and the copy 
32588	   * changed.  The question arises: how big should the copy be?  Allocating
32589	   * space dynamically requires using malloc, which itself takes up a lot
32590	   * of space.  The solution chosen here is to limit the number of arguments
32591	   * to MAX_NUM_ARGS and set this value fairly high.  This solution is simpler
32592	   * and is probably adequate.  Only programs with huge numbers of very short
32593	   * arguments will get an error (if the arguments are large, ARG_MAX will
32594	   * be exceeded.
32595	   */
32596	
32597	  if (strchr(file, '/') != NULL || (searchpath = getenv("PATH")) == NULL)
32598	        searchpath = "";
32599	  flength = strlen(file);
32600	  best_errno = ENOENT;
32601	
32602	  while (1) {
32603	        split = strchr(searchpath, ':');
32604	        if (split == NULL)
32605	                slength = strlen(searchpath);
32606	        else
32607	                slength = split - searchpath;
32608	        if (slength + flength >= sizeof execpath - 2) {
32609	                errno = ENAMETOOLONG;   /* too bad if premature */
32610	                return(-1);
32611	        }
32612	        strncpy(execpath, searchpath, slength);
32613	        if (slength != 0) execpath[slength++] = '/';
32614	        strcpy(execpath + slength, file);
32615	
32616	        /* Don't try to avoid execv() for non-existent files, since the Minix
32617	         * shell doesn't, and it is not clear whether access() or stat() work
32618	         * right when this code is set-uid.
32619	         */
32620	        execv(execpath, argv);
32621	        switch (errno) {
32622	            case EACCES:
32623	                best_errno = errno;     /* more useful than ENOENT */
32624	            case ENOENT:
32625	                if (split == NULL) {
32626	                        /* No more path components. */
32627	                        errno = best_errno;
32628	                        return(-1);
32629	                }
.Ep 314 src/lib/other/execlp.c
32630	                searchpath = split + 1; /* try next in path */
32631	                break;
32632	            case ENOEXEC:
32633	                /* Assume a command file and invoke sh(1) on it.  Replace arg0
32634	                 * (which is usually a short name for the command) by the full
32635	                 * name of the command file.
32636	                 */
32637	
32638	                /* Copy the arg pointers from argv to arg2,  moving them up by
32639	                 * 1, overlaying the assumed NULL at the end, to make room 
32640	                 * for "sh" at the beginning.
32641	                 */
32642	                i = 0;
32643	                if (argv != NULL)
32644	                {
32645	                        while (argv[i] != 0) {
32646	                                if (i >= MAX_NUM_ARGS) {
32647	                                        /* Copy failed.  Not enough room. */
32648	                                        errno = ENOEXEC;
32649	                                        return(-1);
32650	                                }
32651	                                arg2[i + 1] = argv[i];
32652	                                i++;
32653	                        }
32654	                }
32655	                arg2[0] = "sh";         /* exec the shell */
32656	                arg2[1] = execpath;     /* full path */
32657	                arg2[i + 1] = NULL;     /* terminator */
32658	
32659	                /* Count the environment pointers. */
32660	                for (envtop = environ; *envtop != NULL; ) envtop++;
32661	
32662	                /* Try only /bin/sh, like the Minix shell.  Lose if the user
32663	                 * has a different shell or the command has #!another/shell.
32664	                 */
32665	                __execve("/bin/sh", arg2, environ, i + 1, (int)(envtop - environ));
32666	
32667	                /* Oops, no shell?   Give up. */
32668	                errno = ENOEXEC;
32669	                return(-1);
32670	            default:
32671	                return(-1);     /* probably  ENOMEM or E2BIG */
32672	        }
32673	  }
32674	}
.Op 315 src/lib/other/fdopen.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/fdopen.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

32700	/*
32701	 * fdopen - convert a (UNIX) file descriptor into a FILE pointer
32702	 */
32703	/* $Header: fdopen.c,v 1.4 91/02/22 16:32:05 ceriel Exp $ */
32704	
32705	#include        <stdlib.h>
32706	#include        "../stdio/loc_incl.h"
32707	#include        <stdio.h>
32708	
32709	FILE *
32710	fdopen(fd, mode)
32711	int fd;
32712	_CONST char *mode;
32713	{
32714	        register int i;
32715	        FILE *stream;
32716	        int flags = 0;
32717	
32718	        if (fd < 0) return (FILE *)NULL;
32719	        for (i = 0; __iotab[i] != 0 ; i++) 
32720	                if (i >= FOPEN_MAX-1)
32721	                        return (FILE *)NULL;
32722	
32723	        switch(*mode++) {
32724	        case 'r':
32725	                flags |= _IOREAD | _IOREADING;
32726	                break;
32727	        case 'a':
32728	                flags |= _IOAPPEND;
32729	        case 'w':
32730	                flags |= _IOWRITE | _IOWRITING;
32731	                break;
32732	        default:
32733	                return (FILE *)NULL;
32734	        }
32735	        while(*mode) {
32736	                switch(*mode++) {
32737	                case 'b':
32738	                        continue;
32739	                case '+':
32740	                        flags |= _IOREAD | _IOWRITE;
32741	                        continue;
32742	                /* The sequence may be followed by aditional characters */
32743	                default:
32744	                        break;
32745	                }
32746	                break;
32747	        }
32748	
32749	        if ((stream = (FILE *) malloc(sizeof(FILE))) == NULL) {
32750	                return (FILE *)NULL;
32751	        }
32752	
32753	        if ((flags & _IOREAD) && (flags & _IOWRITE))
32754	                flags &= ~(_IOREADING | _IOWRITING);
.Ep 316 src/lib/other/fdopen.c
32755	
32756	        stream->_count = 0;
32757	        stream->_fd = fd;
32758	        stream->_flags = flags;
32759	        stream->_buf = NULL;
32760	        __iotab[i] = stream;
32761	        return stream;
32762	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/ffs.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

32800	#include <lib.h>
32801	/*  ffs(3)
32802	 *
32803	 *  Author: Terrence W. Holm          Sep. 1988
32804	 */
32805	_PROTOTYPE( int ffs, (int word));
32806	
32807	int ffs(word)
32808	int word;
32809	{
32810	  int i;
32811	
32812	  if (word == 0) return(0);
32813	
32814	  for (i = 1;; ++i, word >>= 1)
32815	        if (word & 1) return(i);
32816	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/fslib.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

32900	/* fslib.c - routines needed by fs and fs utilities */
32901	
32902	#include <minix/config.h>       /* for unused stuff in <minix/type.h> :-( */
32903	#include <ansi.h>
32904	#include <sys/types.h>
32905	#include <minix/const.h>
32906	#include <minix/type.h>         /* for unshort :-( */
32907	#include "fs/const.h"           /* depends of -I flag in Makefile */
32908	#include "fs/type.h"            /* ditto */
32909	#include "fs/inode.h"           /* ditto */
32910	#include <minix/fslib.h>
32911	
32912	/* The next routine is copied from fsck.c and mkfs.c...  (Re)define some
32913	 * things for consistency.  Some things should be done better.  The shifts
32914	 * should be replaced by multiplications and divisions by MAP_BITS_PER_BLOCK
32915	 * since log2 of this is too painful to get right.
32916	 */
32917	#define BITMAPSHIFT      13     /* = log2(MAP_BITS_PER_BLOCK) */
32918	
32919	/* Convert from bit count to a block count. The usual expression
.Op 317 src/lib/other/fslib.c
32920	 *
32921	 *      (nr_bits + (1 << BITMAPSHIFT) - 1) >> BITMAPSHIFT
32922	 *
32923	 * doesn't work because of overflow.
32924	 *
32925	 * Other overflow bugs, such as the expression for N_ILIST overflowing when
32926	 * s_inodes is just over V*_INODES_PER_BLOCK less than the maximum+1, are not
32927	 * fixed yet, because that number of inodes is silly.
32928	 */
32929	/* The above comment doesn't all apply now bit_t is long.  Overflow is now
32930	 * unlikely, but negative bit counts are now possible (though unlikely)
32931	 * and give silly results.
32932	 */ 
32933	PUBLIC int bitmapsize(nr_bits)
32934	bit_t nr_bits;
32935	{
32936	  int nr_blocks;
32937	
32938	  nr_blocks = (int) (nr_bits >> BITMAPSHIFT);
32939	  if (((bit_t) nr_blocks << BITMAPSHIFT) < nr_bits) ++nr_blocks;
32940	  return(nr_blocks);
32941	}
	
	
32944	/*===========================================================================*
32945	 *                              conv2                                        *
32946	 *===========================================================================*/
32947	PUBLIC unsigned conv2(norm, w)
32948	int norm;                       /* TRUE if no swap, FALSE for byte swap */
32949	int w;                          /* promotion of 16-bit word to be swapped */
32950	{
32951	/* Possibly swap a 16-bit word between 8086 and 68000 byte order. */
32952	
32953	  if (norm) return( (unsigned) w & 0xFFFF);
32954	  return( ((w&BYTE) << 8) | ( (w>>8) & BYTE));
32955	}
	
	
32958	/*===========================================================================*
32959	 *                              conv4                                        *
32960	 *===========================================================================*/
32961	PUBLIC long conv4(norm, x)
32962	int norm;                       /* TRUE if no swap, FALSE for byte swap */
32963	long x;                         /* 32-bit long to be byte swapped */
32964	{
32965	/* Possibly swap a 32-bit long between 8086 and 68000 byte order. */
32966	
32967	  unsigned lo, hi;
32968	  long l;
32969	  
32970	  if (norm) return(x);                  /* byte order was already ok */
32971	  lo = conv2(FALSE, (int) x & 0xFFFF);  /* low-order half, byte swapped */
32972	  hi = conv2(FALSE, (int) (x>>16) & 0xFFFF);    /* high-order half, swapped */
32973	  l = ( (long) lo <<16) | hi;
32974	  return(l);
32975	}
	
	
32978	/*===========================================================================*
32979	 *                              conv_inode                                   *
.Ep 318 src/lib/other/fslib.c
32980	 *===========================================================================*/
32981	PUBLIC void conv_inode(rip, dip, dip2, rw_flag, magic)
32982	register struct inode *rip;     /* pointer to the in-core inode struct */
32983	register d1_inode *dip;         /* pointer to the V1 on-disk inode struct */
32984	register d2_inode *dip2;        /* pointer to the V2 on-disk inode struct */
32985	int rw_flag;                    /* READING or WRITING */
32986	int magic;                      /* magic number of file system */
32987	{ 
32988	/* Copy the inode from the disk block to the in-core table or vice versa.
32989	 * If the fourth parameter below is FALSE, the bytes are swapped.
32990	 */
32991	  switch (magic) {
32992	        case SUPER_MAGIC:       old_icopy(rip, dip,  rw_flag, TRUE);    break;
32993	        case SUPER_REV:         old_icopy(rip, dip,  rw_flag, FALSE);   break;
32994	        case SUPER_V2:          new_icopy(rip, dip2, rw_flag, TRUE);    break;
32995	        case SUPER_V2_REV:      new_icopy(rip, dip2, rw_flag, FALSE);   break;
32996	  } 
32997	}
	
	
33000	/*===========================================================================*
33001	 *                              old_icopy                                    *
33002	 *===========================================================================*/
33003	PUBLIC void old_icopy(rip, dip, direction, norm)
33004	register struct inode *rip;     /* pointer to the in-core inode struct */
33005	register d1_inode *dip;         /* pointer to the d1_inode inode struct */
33006	int direction;                  /* READING (from disk) or WRITING (to disk) */
33007	int norm;                       /* TRUE = do not swap bytes; FALSE = swap */
33008	
33009	{
33010	/* 4 different on-disk inode layouts are supported, one for each combination
33011	 * of V1.x/V2.x * bytes-swapped/not-swapped.  When an inode is read or written
33012	 * this routine handles the conversions so that the information in the inode
33013	 * table is independent of the disk structure from which the inode came.
33014	 * The old_icopy routine copies to and from V1 disks.
33015	 */
33016	
33017	  int i;
33018	
33019	  if (direction == READING) {
33020	        /* Copy V1.x inode to the in-core table, swapping bytes if need be. */
33021	        rip->i_mode    = conv2(norm, dip->d1_mode);
33022	        rip->i_uid     = conv2(norm,dip->d1_uid );
33023	        rip->i_size    = conv4(norm,dip->d1_size);
33024	        rip->i_mtime   = conv4(norm,dip->d1_mtime);
33025	        rip->i_atime   = 0;
33026	        rip->i_ctime   = 0;
33027	        rip->i_nlinks  = (nlink_t) dip->d1_nlinks;      /* 1 char */
33028	        rip->i_gid     = (gid_t) dip->d1_gid;           /* 1 char */
33029	        rip->i_ndzones = V1_NR_DZONES;
33030	        rip->i_nindirs = V1_INDIRECTS;
33031	        for (i = 0; i < V1_NR_TZONES; i++)
33032	                rip->i_zone[i] = conv2(norm, (int) dip->d1_zone[i]);
33033	  } else {
33034	        /* Copying V1.x inode to disk from the in-core table. */
33035	        dip->d1_mode   = conv2(norm,rip->i_mode);
33036	        dip->d1_uid    = conv2(norm,rip->i_uid );
33037	        dip->d1_size   = conv4(norm,rip->i_size);
33038	        dip->d1_mtime  = conv4(norm,rip->i_mtime);
33039	        dip->d1_nlinks = (nlink_t) rip->i_nlinks;       /* 1 char */
.Op 319 src/lib/other/fslib.c
33040	        dip->d1_gid    = (gid_t) rip->i_gid;            /* 1 char */
33041	        for (i = 0; i < V1_NR_TZONES; i++)
33042	                dip->d1_zone[i] = conv2(norm, (int) rip->i_zone[i]);
33043	  }
33044	}
	
	
33047	/*===========================================================================*
33048	 *                              new_icopy                                    *
33049	 *===========================================================================*/
33050	PUBLIC void new_icopy(rip, dip, direction, norm)
33051	register struct inode *rip;     /* pointer to the in-core inode struct */
33052	register d2_inode *dip; /* pointer to the d2_inode struct */
33053	int direction;                  /* READING (from disk) or WRITING (to disk) */
33054	int norm;                       /* TRUE = do not swap bytes; FALSE = swap */
33055	
33056	{
33057	/* Same as old_icopy, but to/from V2 disk layout. */
33058	
33059	  int i;
33060	
33061	  if (direction == READING) {
33062	        /* Copy V2.x inode to the in-core table, swapping bytes if need be. */
33063	        rip->i_mode    = conv2(norm,dip->d2_mode);
33064	        rip->i_uid     = conv2(norm,dip->d2_uid );
33065	        rip->i_nlinks  = conv2(norm,(int) dip->d2_nlinks);
33066	        rip->i_gid     = conv2(norm,(int) dip->d2_gid );
33067	        rip->i_size    = conv4(norm,dip->d2_size);
33068	        rip->i_atime   = conv4(norm,dip->d2_atime);
33069	        rip->i_ctime   = conv4(norm,dip->d2_ctime);
33070	        rip->i_mtime   = conv4(norm,dip->d2_mtime);
33071	        rip->i_ndzones = V2_NR_DZONES;
33072	        rip->i_nindirs = V2_INDIRECTS;
33073	        for (i = 0; i < V2_NR_TZONES; i++)
33074	                rip->i_zone[i] = conv4(norm, (long) dip->d2_zone[i]);
33075	  } else {
33076	        /* Copying V2.x inode to disk from the in-core table. */
33077	        dip->d2_mode   = conv2(norm,rip->i_mode);
33078	        dip->d2_uid    = conv2(norm,rip->i_uid );
33079	        dip->d2_nlinks = conv2(norm,rip->i_nlinks);
33080	        dip->d2_gid    = conv2(norm,rip->i_gid );
33081	        dip->d2_size   = conv4(norm,rip->i_size);
33082	        dip->d2_atime  = conv4(norm,rip->i_atime);
33083	        dip->d2_ctime  = conv4(norm,rip->i_ctime);
33084	        dip->d2_mtime  = conv4(norm,rip->i_mtime);
33085	        for (i = 0; i < V2_NR_TZONES; i++)
33086	                dip->d2_zone[i] = conv4(norm, (long) rip->i_zone[i]);
33087	  }
33088	}
.Ep 320 src/lib/other/fsversion.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/fsversion.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

33100	/* This procedure examines a file system and figures out whether it is
33101	 * version 1 or version 2.  It returns the result as an int.  If the
33102	 * file system is neither, it returns -1.  A typical call is:
33103	 *
33104	 *      n = fsversion("/dev/hd1", "df");
33105	 *
33106	 * The first argument is the special file for the file system. 
33107	 * The second is the program name, which is used in error messages.
33108	 */
33109	
33110	#include <sys/types.h>
33111	#include <minix/config.h>
33112	#include <minix/const.h>
33113	#include <minix/minlib.h>
33114	#include <minix/type.h>
33115	#include <fcntl.h>
33116	#include <unistd.h>
33117	#include <stdio.h>
33118	
33119	#include "../../fs/const.h"
33120	#include "../../fs/type.h"
33121	#include "../../fs/super.h"
33122	
33123	static struct super_block super, *sp;
33124	
33125	int fsversion(dev, prog)
33126	char *dev, *prog;
33127	{
33128	  int fd;
33129	
33130	  if ((fd = open(dev, O_RDONLY)) < 0) {
33131	        std_err(prog);
33132	        std_err(" cannot open ");
33133	        perror(dev);
33134	        return(-1);
33135	  }
33136	
33137	  lseek(fd, (off_t) BLOCK_SIZE, SEEK_SET);      /* skip boot block */
33138	  if (read(fd, (char *) &super, (unsigned) SUPER_SIZE) != SUPER_SIZE) {
33139	        std_err(prog);
33140	        std_err(" cannot read super block on ");
33141	        perror(dev);
33142	        close(fd);
33143	        return(-1);
33144	  }
33145	  close(fd);
33146	  sp = &super;
33147	  if (sp->s_magic == SUPER_MAGIC) return(1);
33148	  if (sp->s_magic == SUPER_V2) return(2);
33149	  return(-1);
33150	}
.Op 321 src/lib/other/getgrent.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/getgrent.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

33200	/*      getgrent(), getgrgid(), getgrnam() - group file routines
33201	 *
33202	 *                                                      Author: Kees J. Bot
33203	 *                                                              31 Jan 1994
33204	 */
33205	#define nil 0
33206	#define open _open
33207	#define fcntl _fcntl
33208	#define read _read
33209	#define close _close
33210	#include <sys/types.h>
33211	#include <grp.h>
33212	#include <string.h>
33213	#include <stdlib.h>
33214	#include <unistd.h>
33215	#include <fcntl.h>
33216	
33217	#define arraysize(a)    (sizeof(a) / sizeof((a)[0]))
33218	#define arraylimit(a)   ((a) + arraysize(a))
33219	
33220	static char GROUP[]= "/etc/group";      /* The group file. */
33221	static const char *grfile;              /* Current group file. */
33222	
33223	static char buf[1024];                  /* Read buffer. */
33224	static char grline[512];                /* One line from the group file. */
33225	static struct group entry;              /* Entry to fill and return. */
33226	static char *members[64];               /* Group members with the entry. */
33227	static int grfd= -1;                    /* Filedescriptor to the file. */
33228	static char *bufptr;                    /* Place in buf. */
33229	static ssize_t buflen= 0;               /* Remaining characters in buf. */
33230	static char *lineptr;                   /* Place in the line. */
33231	
33232	void endgrent(void)
33233	/* Close the group file. */
33234	{
33235	        if (grfd >= 0) {
33236	                (void) close(grfd);
33237	                grfd= -1;
33238	                buflen= 0;
33239	        }
33240	}
	
33242	int setgrent(void)
33243	/* Open the group file. */
33244	{
33245	        if (grfd >= 0) endgrent();
33246	
33247	        if (grfile == nil) grfile= GROUP;
33248	
33249	        if ((grfd= open(grfile, O_RDONLY)) < 0) return -1;
33250	        (void) fcntl(grfd, F_SETFD, fcntl(grfd, F_GETFD) | FD_CLOEXEC);
33251	        return 0;
33252	}
	
33254	void setgrfile(const char *file)
.Ep 322 src/lib/other/getgrent.c
33255	/* Prepare for reading an alternate group file. */
33256	{
33257	        endgrent();
33258	        grfile= file;
33259	}
	
33261	static int getline(void)
33262	/* Get one line from the group file, return 0 if bad or EOF. */
33263	{
33264	        lineptr= grline;
33265	
33266	        do {
33267	                if (buflen == 0) {
33268	                        if ((buflen= read(grfd, buf, sizeof(buf))) <= 0)
33269	                                return 0;
33270	                        bufptr= buf;
33271	                }
33272	
33273	                if (lineptr == arraylimit(grline)) return 0;
33274	                buflen--;
33275	        } while ((*lineptr++ = *bufptr++) != '\n');
33276	
33277	        lineptr= grline;
33278	        return 1;
33279	}
	
33281	static char *scan_punct(int punct)
33282	/* Scan for a field separator in a line, return the start of the field. */
33283	{
33284	        char *field= lineptr;
33285	        char *last;
33286	
33287	        for (;;) {
33288	                last= lineptr;
33289	                if (*lineptr == 0) return nil;
33290	                if (*lineptr == '\n') break;
33291	                if (*lineptr++ == punct) break;
33292	                if (lineptr[-1] == ':') return nil;     /* :::,,,:,,,? */
33293	        }
33294	        *last= 0;
33295	        return field;
33296	}
	
33298	struct group *getgrent(void)
33299	/* Read one entry from the group file. */
33300	{
33301	        char *p;
33302	        char **mem;
33303	
33304	        /* Open the file if not yet open. */
33305	        if (grfd < 0 && setgrent() < 0) return nil;
33306	
33307	        /* Until a good line is read. */
33308	        for (;;) {
33309	                if (!getline()) return nil;     /* EOF or corrupt. */
33310	
33311	                if ((entry.gr_name= scan_punct(':')) == nil) continue;
33312	                if ((entry.gr_passwd= scan_punct(':')) == nil) continue;
33313	                if ((p= scan_punct(':')) == nil) continue;
33314	                entry.gr_gid= strtol(p, nil, 0);
.Op 323 src/lib/other/getgrent.c
33315	
33316	                entry.gr_mem= mem= members;
33317	                if (*lineptr != '\n') {
33318	                        do {
33319	                                if ((*mem= scan_punct(',')) == nil) goto again;
33320	                                if (mem < arraylimit(members) - 1) mem++;
33321	                        } while (*lineptr != 0);
33322	                }
33323	                *mem= nil;
33324	                return &entry;
33325	        again:;
33326	        }
33327	}
	
33329	struct group *getgrgid(Gid_t gid)
33330	/* Return the group file entry belonging to the user-id. */
33331	{
33332	        struct group *gr;
33333	
33334	        endgrent();
33335	        while ((gr= getgrent()) != nil && gr->gr_gid != gid) {}
33336	        endgrent();
33337	        return gr;
33338	}
	
33340	struct group *getgrnam(const char *name)
33341	/* Return the group file entry belonging to the user name. */
33342	{
33343	        struct group *gr;
33344	
33345	        endgrent();
33346	        while ((gr= getgrent()) != nil && strcmp(gr->gr_name, name) != 0) {}
33347	        endgrent();
33348	        return gr;
33349	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/getlogin.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

33400	/*  getlogin(3)
33401	 *
33402	 *  Author: Terrence W. Holm          Aug. 1988
33403	 */
33404	
33405	#include <lib.h>
33406	#include <pwd.h>
33407	#include <unistd.h>
33408	#include <string.h>
33409	#include <stdio.h>
33410	
33411	#ifndef  L_cuserid
33412	#define  L_cuserid   9
33413	#endif
33414	
.Ep 324 src/lib/other/getlogin.c
33415	char *getlogin()
33416	{
33417	  PRIVATE char userid[L_cuserid];
33418	  struct passwd *pw_entry;
33419	
33420	  pw_entry = getpwuid(getuid());
33421	
33422	  if (pw_entry == (struct passwd *)NULL) return((char *)NULL);
33423	
33424	  strcpy(userid, pw_entry->pw_name);
33425	
33426	  return(userid);
33427	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/getopt.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

33500	/*
33501	 * getopt - parse command-line options
33502	 */
33503	/* $Header: getopt.c,v 1.1 89/12/18 14:39:31 eck Exp $ */
33504	
33505	#include        <stdlib.h>
33506	#include        <string.h>
33507	#include        <stdio.h>
33508	
33509	#define ERR(s, c)       if(opterr){\
33510	        fputs(argv[0], stderr);\
33511	        fputs(s, stderr);\
33512	        fputc(c, stderr);\
33513	        fputc('\n', stderr);}
33514	
33515	int     opterr = 1;
33516	int     optind = 1;
33517	int     optopt;
33518	char    *optarg;
33519	
33520	int
33521	getopt(argc, argv, opts)
33522	int argc;
33523	char **argv;
33524	char *opts;
33525	{
33526	        static int sp = 1;
33527	        register c;
33528	        register char *cp;
33529	
33530	        if (sp == 1)
33531	                if (optind >= argc ||
33532	                   argv[optind][0] != '-' || argv[optind][1] == '\0')
33533	                        return EOF;
33534	                else if (!strcmp(argv[optind], "--")) {
33535	                        optind++;
33536	                        return EOF;
33537	                }
33538	        optopt = c = argv[optind][sp];
33539	        if (c == ':' || (cp=strchr(opts, c)) == NULL) {
.Op 325 src/lib/other/getopt.c
33540	                ERR (": illegal option -- ", c);
33541	                if (argv[optind][++sp] == '\0') {
33542	                        optind++;
33543	                        sp = 1;
33544	                }
33545	                return '?';
33546	        }
33547	        if (*++cp == ':') {
33548	                if (argv[optind][sp+1] != '\0')
33549	                        optarg = &argv[optind++][sp+1];
33550	                else if (++optind >= argc) {
33551	                        ERR (": option requires an argument -- ", c);
33552	                        sp = 1;
33553	                        return '?';
33554	                } else
33555	                        optarg = argv[optind++];
33556	                sp = 1;
33557	        } else {
33558	                if (argv[optind][++sp] == '\0') {
33559	                        sp = 1;
33560	                        optind++;
33561	                }
33562	                optarg = NULL;
33563	        }
33564	        return c;
33565	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/getpass.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

33600	/*      getpass() - read a password             Author: Kees J. Bot
33601	 *                                                      Feb 16 1993
33602	 */
33603	#define open _open
33604	#define sigaction _sigaction
33605	#define sigemptyset _sigemptyset
33606	#define tcgetattr _tcgetattr
33607	#define tcsetattr _tcsetattr
33608	#define write _write
33609	#define read _read
33610	#define close _close
33611	#include <sys/types.h>
33612	#include <fcntl.h>
33613	#include <unistd.h>
33614	#include <signal.h>
33615	#include <termios.h>
33616	#include <string.h>
33617	
33618	static int intr;
33619	
33620	static void catch(int sig)
33621	{
33622	        intr= 1;
33623	}
	
.Ep 326 src/lib/other/getpass.c
33625	char *getpass(const char *prompt)
33626	{
33627	        struct sigaction osa, sa;
33628	        struct termios cooked, raw;
33629	        static char password[32+1];
33630	        int fd, n= 0;
33631	
33632	        /* Try to open the controlling terminal. */
33633	        if ((fd= open("/dev/tty", O_RDONLY)) < 0) return NULL;
33634	
33635	        /* Trap interrupts unless ignored. */
33636	        intr= 0;
33637	        sigaction(SIGINT, NULL, &osa);
33638	        if (osa.sa_handler != SIG_IGN) {
33639	                sigemptyset(&sa.sa_mask);
33640	                sa.sa_flags= 0;
33641	                sa.sa_handler= catch;
33642	                sigaction(SIGINT, &sa, &osa);
33643	        }
33644	
33645	        /* Set the terminal to non-echo mode. */
33646	        tcgetattr(fd, &cooked);
33647	        raw= cooked;
33648	        raw.c_iflag|= ICRNL;
33649	        raw.c_lflag&= ~ECHO;
33650	        raw.c_lflag|= ECHONL;
33651	        raw.c_oflag|= OPOST | ONLCR;
33652	        tcsetattr(fd, TCSANOW, &raw);
33653	
33654	        /* Print the prompt.  (After setting non-echo!) */
33655	        write(2, prompt, strlen(prompt));
33656	
33657	        /* Read the password, 32 characters max. */
33658	        while (read(fd, password+n, 1) > 0) {
33659	                if (password[n] == '\n') break;
33660	                if (n < 32) n++;
33661	        }
33662	        password[n]= 0;
33663	
33664	        /* Terminal back to cooked mode. */
33665	        tcsetattr(fd, TCSANOW, &cooked);
33666	
33667	        close(fd);
33668	
33669	        /* Interrupt? */
33670	        sigaction(SIGINT, &osa, NULL);
33671	        if (intr) raise(SIGINT);
33672	
33673	        return password;
33674	}
.Op 327 src/lib/other/getpwent.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/getpwent.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

33700	/*      getpwent(), getpwuid(), getpwnam() - password file routines
33701	 *
33702	 *                                                      Author: Kees J. Bot
33703	 *                                                              31 Jan 1994
33704	 */
33705	#define nil 0
33706	#define open _open
33707	#define fcntl _fcntl
33708	#define read _read
33709	#define close _close
33710	#include <sys/types.h>
33711	#include <pwd.h>
33712	#include <string.h>
33713	#include <stdlib.h>
33714	#include <unistd.h>
33715	#include <fcntl.h>
33716	
33717	#define arraysize(a)    (sizeof(a) / sizeof((a)[0]))
33718	#define arraylimit(a)   ((a) + arraysize(a))
33719	
33720	static char PASSWD[]= "/etc/passwd";    /* The password file. */
33721	static const char *pwfile;              /* Current password file. */
33722	
33723	static char buf[1024];                  /* Read buffer. */
33724	static char pwline[256];                /* One line from the password file. */
33725	static struct passwd entry;             /* Entry to fill and return. */
33726	static int pwfd= -1;                    /* Filedescriptor to the file. */
33727	static char *bufptr;                    /* Place in buf. */
33728	static ssize_t buflen= 0;               /* Remaining characters in buf. */
33729	static char *lineptr;                   /* Place in the line. */
33730	
33731	void endpwent(void)
33732	/* Close the password file. */
33733	{
33734	        if (pwfd >= 0) {
33735	                (void) close(pwfd);
33736	                pwfd= -1;
33737	                buflen= 0;
33738	        }
33739	}
	
33741	int setpwent(void)
33742	/* Open the password file. */
33743	{
33744	        if (pwfd >= 0) endpwent();
33745	
33746	        if (pwfile == nil) pwfile= PASSWD;
33747	
33748	        if ((pwfd= open(pwfile, O_RDONLY)) < 0) return -1;
33749	        (void) fcntl(pwfd, F_SETFD, fcntl(pwfd, F_GETFD) | FD_CLOEXEC);
33750	        return 0;
33751	}
	
33753	void setpwfile(const char *file)
33754	/* Prepare for reading an alternate password file. */
.Ep 328 src/lib/other/getpwent.c
33755	{
33756	        endpwent();
33757	        pwfile= file;
33758	}
	
33760	static int getline(void)
33761	/* Get one line from the password file, return 0 if bad or EOF. */
33762	{
33763	        lineptr= pwline;
33764	
33765	        do {
33766	                if (buflen == 0) {
33767	                        if ((buflen= read(pwfd, buf, sizeof(buf))) <= 0)
33768	                                return 0;
33769	                        bufptr= buf;
33770	                }
33771	
33772	                if (lineptr == arraylimit(pwline)) return 0;
33773	                buflen--;
33774	        } while ((*lineptr++ = *bufptr++) != '\n');
33775	
33776	        lineptr= pwline;
33777	        return 1;
33778	}
	
33780	static char *scan_colon(void)
33781	/* Scan for a field separator in a line, return the start of the field. */
33782	{
33783	        char *field= lineptr;
33784	        char *last;
33785	
33786	        for (;;) {
33787	                last= lineptr;
33788	                if (*lineptr == 0) return nil;
33789	                if (*lineptr == '\n') break;
33790	                if (*lineptr++ == ':') break;
33791	        }
33792	        *last= 0;
33793	        return field;
33794	}
	
33796	struct passwd *getpwent(void)
33797	/* Read one entry from the password file. */
33798	{
33799	        char *p;
33800	
33801	        /* Open the file if not yet open. */
33802	        if (pwfd < 0 && setpwent() < 0) return nil;
33803	
33804	        /* Until a good line is read. */
33805	        for (;;) {
33806	                if (!getline()) return nil;     /* EOF or corrupt. */
33807	
33808	                if ((entry.pw_name= scan_colon()) == nil) continue;
33809	                if ((entry.pw_passwd= scan_colon()) == nil) continue;
33810	                if ((p= scan_colon()) == nil) continue;
33811	                entry.pw_uid= strtol(p, nil, 0);
33812	                if ((p= scan_colon()) == nil) continue;
33813	                entry.pw_gid= strtol(p, nil, 0);
33814	                if ((entry.pw_gecos= scan_colon()) == nil) continue;
.Op 329 src/lib/other/getpwent.c
33815	                if ((entry.pw_dir= scan_colon()) == nil) continue;
33816	                if ((entry.pw_shell= scan_colon()) == nil) continue;
33817	
33818	                if (*lineptr == 0) return &entry;
33819	        }
33820	}
	
33822	struct passwd *getpwuid(Uid_t uid)
33823	/* Return the password file entry belonging to the user-id. */
33824	{
33825	        struct passwd *pw;
33826	
33827	        endpwent();
33828	        while ((pw= getpwent()) != nil && pw->pw_uid != uid) {}
33829	        endpwent();
33830	        return pw;
33831	}
	
33833	struct passwd *getpwnam(const char *name)
33834	/* Return the password file entry belonging to the user name. */
33835	{
33836	        struct passwd *pw;
33837	
33838	        endpwent();
33839	        while ((pw= getpwent()) != nil && strcmp(pw->pw_name, name) != 0) {}
33840	        endpwent();
33841	        return pw;
33842	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/getttyent.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

33900	/*      getttyent(3) - get a ttytab entry               Author: Kees J. Bot
33901	 *                                                              28 Oct 1995
33902	 */
33903	#define nil 0
33904	#define open _open
33905	#define close _close
33906	#define fcntl _fcntl
33907	#define read _read
33908	#include <string.h>
33909	#include <sys/types.h>
33910	#include <ttyent.h>
33911	#include <unistd.h>
33912	#include <fcntl.h>
33913	
33914	#define arraysize(a)    (sizeof(a) / sizeof((a)[0]))
33915	#define arraylimit(a)   ((a) + arraysize(a))
33916	
33917	static char TTYTAB[]= "/etc/ttytab";    /* The table of terminal devices. */
33918	
33919	static char buf[512];                   /* Read buffer. */
33920	static char ttline[256];                /* One line from the ttytab file. */
33921	static char *ttargv[32];                /* Compound arguments. */
33922	static struct ttyent entry;             /* Entry to fill and return. */
33923	static int ttfd= -1;                    /* Filedescriptor to the file. */
33924	static char *bufptr;                    /* Place in buf. */
.Ep 330 src/lib/other/getttyent.c
33925	static ssize_t buflen= 0;               /* Remaining characters in buf. */
33926	static char *lineptr;                   /* Place in the line. */
33927	static char **argvptr;                  /* Place in word lists. */
33928	
33929	void endttyent(void)
33930	/* Close the ttytab file. */
33931	{
33932	        if (ttfd >= 0) {
33933	                (void) close(ttfd);
33934	                ttfd= -1;
33935	                buflen= 0;
33936	        }
33937	}
	
33939	int setttyent(void)
33940	/* Open the ttytab file. */
33941	{
33942	        if (ttfd >= 0) endttyent();
33943	
33944	        if ((ttfd= open(TTYTAB, O_RDONLY)) < 0) return -1;
33945	        (void) fcntl(ttfd, F_SETFD, fcntl(ttfd, F_GETFD) | FD_CLOEXEC);
33946	        return 0;
33947	}
	
33949	static int getline(void)
33950	/* Get one line from the ttytab file, return 0 if bad or EOF. */
33951	{
33952	        lineptr= ttline;
33953	        argvptr= ttargv;
33954	
33955	        do {
33956	                if (buflen == 0) {
33957	                        if ((buflen= read(ttfd, buf, sizeof(buf))) <= 0)
33958	                                return 0;
33959	                        bufptr= buf;
33960	                }
33961	
33962	                if (lineptr == arraylimit(ttline)) return 0;
33963	                buflen--;
33964	        } while ((*lineptr++ = *bufptr++) != '\n');
33965	
33966	        lineptr= ttline;
33967	        return 1;
33968	}
	
33970	static int white(int c)
33971	/* Whitespace? */
33972	{
33973	        return c == ' ' || c == '\t';
33974	}
	
33976	static char *scan_white(int quoted)
33977	/* Scan for a field separator in a line, return the start of the field.
33978	 * "quoted" is set if we have to watch out for double quotes.
33979	 */
33980	{
33981	        char *field, *last;
33982	
33983	        while (white(*lineptr)) lineptr++;
33984	        if (!quoted && *lineptr == '#') return nil;
.Op 331 src/lib/other/getttyent.c
33985	
33986	        field= lineptr;
33987	        for (;;) {
33988	                last= lineptr;
33989	                if (*lineptr == 0) return nil;
33990	                if (*lineptr == '\n') break;
33991	                if (quoted && *lineptr == '"') return field;
33992	                if (white(*lineptr++)) break;
33993	        }
33994	        *last= 0;
33995	        return *field == 0 ? nil : field;
33996	}
	
33998	static char **scan_quoted(void)
33999	/* Read a field that may be a quoted list of words. */
34000	{
34001	        char *p, **field= argvptr;
34002	
34003	        while (white(*lineptr)) lineptr++;
34004	
34005	        if (*lineptr == '"') {
34006	                /* Quoted list of words. */
34007	                lineptr++;
34008	                while ((p= scan_white(1)) != nil && *p != '"') {
34009	                        if (argvptr == arraylimit(ttargv)) return nil;
34010	                        *argvptr++= p;
34011	                }
34012	                if (*lineptr == '"') *lineptr++= 0;
34013	        } else {
34014	                /* Just one word. */
34015	                if ((p= scan_white(0)) == nil) return nil;
34016	                if (argvptr == arraylimit(ttargv)) return nil;
34017	                *argvptr++= p;
34018	        }
34019	        if (argvptr == arraylimit(ttargv)) return nil;
34020	        *argvptr++= nil;
34021	        return field;
34022	}
	
34024	struct ttyent *getttyent(void)
34025	/* Read one entry from the ttytab file. */
34026	{
34027	        /* Open the file if not yet open. */
34028	        if (ttfd < 0 && setttyent() < 0) return nil;
34029	
34030	        /* Look for a line with something on it. */
34031	        for (;;) {
34032	                if (!getline()) return nil;     /* EOF or corrupt. */
34033	
34034	                if ((entry.ty_name= scan_white(0)) == nil) continue;
34035	                entry.ty_type= scan_white(0);
34036	                entry.ty_getty= scan_quoted();
34037	                entry.ty_init= scan_quoted();
34038	
34039	                return &entry;
34040	        }
34041	}
	
34043	struct ttyent *getttynam(const char *name)
34044	/* Return the ttytab file entry for a given tty. */
.Ep 332 src/lib/other/getttyent.c
34045	{
34046	        struct ttyent *tty;
34047	
34048	        endttyent();
34049	        while ((tty= getttyent()) != nil && strcmp(tty->ty_name, name) != 0) {}
34050	        endttyent();
34051	        return tty;
34052	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/getw.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

34100	/*
34101	 * getw - read a word from a stream
34102	 */
34103	/* $Header: getw.c,v 1.1 89/12/18 14:39:51 eck Exp $ */
34104	
34105	#include        <stdio.h>
34106	
34107	_PROTOTYPE(int getw, (FILE *stream ));
34108	
34109	int getw(stream)
34110	register FILE *stream;
34111	{
34112	        register int cnt = sizeof(int);
34113	        int w;
34114	        register char *p = (char *) &w;
34115	
34116	        while (cnt--) {
34117	                *p++ = getc(stream);
34118	        }
34119	        if (feof(stream) || ferror(stream)) return EOF;
34120	        return w;
34121	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/hypot.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

34200	/*
34201	 * (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
34202	 * See the copyright notice in the ACK home directory, in the file "Copyright".
34203	 *
34204	 * Author: Ceriel J.H. Jacobs
34205	 */
34206	
34207	#include <math.h>
34208	
34209	struct complex {
34210	        double r,i;
34211	};
34212	
34213	_PROTOTYPE(double hypot, (double x, double y ));
34214	_PROTOTYPE(double cabs, (struct complex p_compl ));
.Op 333 src/lib/other/hypot.c
34215	
34216	/* $Header: hypot.c,v 1.1 91/02/26 18:08:08 ceriel Exp $ */
34217	
34218	double
34219	hypot(x, y)
34220	double x, y;
34221	{
34222	        /*      Computes sqrt(x*x+y*y), avoiding overflow */
34223	
34224	        if (x < 0) x = -x;
34225	        if (y < 0) y = -y;
34226	        if (x > y) {
34227	                double t = y;
34228	                y = x;
34229	                x = t;
34230	        }
34231	        /* sqrt(x*x+y*y) = sqrt(y*y*(x*x/(y*y)+1.0)) = y*sqrt(x*x/(y*y)+1.0) */
34232	        if (y == 0.0) return 0.0;
34233	        x /= y;
34234	        return y*sqrt(x*x+1.0);
34235	}
	
34237	double
34238	cabs(p_compl)
34239	struct complex p_compl;
34240	{
34241	        return hypot(p_compl.r, p_compl.i);
34242	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/index.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

34300	#include <lib.h>
34301	/* index - find first occurrence of a character in a string */
34302	
34303	#include <string.h>
34304	
34305	char *index(s, charwanted)      /* found char, or NULL if none */
34306	_CONST char *s;
34307	char charwanted;
34308	{
34309	  return(strchr(s, charwanted));
34310	}
.Ep 334 src/lib/other/itoa.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/itoa.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

34400	#include <lib.h>
34401	/* Integer to ASCII for signed decimal integers. */
34402	
34403	PRIVATE int next;
34404	PRIVATE char qbuf[8];
34405	
34406	_PROTOTYPE( char *itoa, (int n));
34407	
34408	char *itoa(n)
34409	int n;
34410	{
34411	  register int r, k;
34412	  int flag = 0;
34413	
34414	  next = 0;
34415	  if (n < 0) {
34416	        qbuf[next++] = '-';
34417	        n = -n;
34418	  }
34419	  if (n == 0) {
34420	        qbuf[next++] = '0';
34421	  } else {
34422	        k = 10000;
34423	        while (k > 0) {
34424	                r = n / k;
34425	                if (flag || r > 0) {
34426	                        qbuf[next++] = '0' + r;
34427	                        flag = 1;
34428	                }
34429	                n -= r * k;
34430	                k = k / 10;
34431	        }
34432	  }
34433	  qbuf[next] = 0;
34434	  return(qbuf);
34435	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/loadname.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

34500	#include <lib.h>
34501	#include <string.h>
34502	
34503	PUBLIC void _loadname(name, msgptr)
34504	_CONST char *name;
34505	message *msgptr;
34506	{
34507	/* This function is used to load a string into a type m3 message. If the
34508	 * string fits in the message, it is copied there.  If not, a pointer to
34509	 * it is passed.
.Op 335 src/lib/other/loadname.c
34510	 */
34511	
34512	  register size_t k;
34513	
34514	  k = strlen(name) + 1;
34515	  msgptr->m3_i1 = k;
34516	  msgptr->m3_p1 = (char *) name;
34517	  if (k <= sizeof msgptr->m3_ca1) strcpy(msgptr->m3_ca1, name);
34518	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/lock.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

34600	#include <lib.h>
34601	#include <errno.h>
34602	#include <sys/types.h>
34603	#include <unistd.h>
34604	#include <string.h>
34605	#include <fcntl.h>
34606	#include <stdio.h>
34607	#if _ANSI
34608	#include <stdlib.h>
34609	#endif
34610	
34611	typedef enum {
34612	  False, True
34613	} BOOLEAN;
34614	
34615	#define LOCKDIR "/tmp/"         /* or /usr/tmp/ as the case may be */
34616	#define MAXTRIES 3
34617	#define NAPTIME (unsigned int)5
34618	
34619	PRIVATE _PROTOTYPE( char *lockpath, (char *name));
34620	_PROTOTYPE( void syserr, (char *errstring));
34621	_PROTOTYPE( BOOLEAN lock, (char *name));
34622	_PROTOTYPE( void unlock, (char *name));
34623	
34624	void
34625	syserr(errstring)
34626	char *errstring;
34627	{
34628	        fprintf(stderr,"couldn't %s\n", errstring);
34629	        exit(1);
34630	}
	
34632	BOOLEAN lock(name)              /* acquire lock */
34633	char *name;
34634	{
34635	  char *path;
34636	  int fd, tries;
34637	
34638	  path = lockpath(name);
34639	  tries = 0;
34640	  while ((fd = creat(path, 0)) == -1 && errno == EACCES) {
34641	        if (++tries >= MAXTRIES) return(False);
34642	        sleep(NAPTIME);
34643	  }
34644	  if (fd == -1 || close(fd) == -1) syserr("lock");
.Ep 336 src/lib/other/lock.c
34645	  return(True);
34646	}
	
34648	void unlock(name)               /* free lock */
34649	char *name;
34650	{
34651	  if (unlink(lockpath(name)) == -1) syserr("unlock");
34652	}
	
34654	PRIVATE char *lockpath(name)    /* generate lock file path */
34655	char *name;
34656	{
34657	  PRIVATE char path[20];
34658	
34659	  strcpy(path, LOCKDIR);
34660	  return(strcat(path, name));
34661	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/lrand.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

34700	/*  lrand(3)
34701	 *
34702	 *  Author: Terrence W. Holm          Nov. 1988
34703	 *
34704	 *
34705	 *  A prime modulus multiplicative linear congruential
34706	 *  generator (PMMLCG), or "Lehmer generator".
34707	 *  Implementation directly derived from the article:
34708	 *
34709	 *      S. K. Park and K. W. Miller
34710	 *      Random Number Generators: Good Ones are Hard to Find
34711	 *      CACM vol 31, #10. Oct. 1988. pp 1192-1201.
34712	 *
34713	 *
34714	 *  Using the following multiplier and modulus, we obtain a
34715	 *  generator which:
34716	 *
34717	 *      1)  Has a full period: 1 to 2^31 - 2.
34718	 *      2)  Is testably "random" (see the article).
34719	 *      3)  Has a known implementation by E. L. Schrage.
34720	 */
34721	
34722	#include <lib.h>
34723	
34724	_PROTOTYPE( long seed, (long lseed));
34725	_PROTOTYPE( long lrand, (void));
34726	
34727	#define  A        16807L        /* A "good" multiplier    */
34728	#define  M   2147483647L        /* Modulus: 2^31 - 1      */
34729	#define  Q       127773L        /* M / A                  */
34730	#define  R         2836L        /* M % A                  */
34731	
34732	PRIVATE long _lseed = 1L;
34733	
34734	long seed(lseed)
.Op 337 src/lib/other/lrand.c
34735	long lseed;
34736	{
34737	  long previous_seed = _lseed;
34738	
34739	  _lseed = lseed;
34740	
34741	  return(previous_seed);
34742	}
	
	
34745	long lrand()
34746	{
34747	  _lseed = A * (_lseed % Q) - R * (_lseed / Q);
34748	
34749	  if (_lseed < 0) _lseed += M;
34750	
34751	  return(_lseed);
34752	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/lsearch.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

34800	#include <lib.h>
34801	#include <string.h>
34802	/*  lsearch(3)  and  lfind(3)
34803	 *
34804	 *  Author: Terrence W. Holm          Sep. 1988
34805	 */
34806	
34807	#include <stddef.h>
34808	
34809	_PROTOTYPE( char *lsearch, (char *key, char *base,
34810	                            unsigned *count, unsigned width,
34811	                            int (*keycmp)(const void *, const void *)));
34812	_PROTOTYPE( char *lfind, (char *key, char *base,
34813	                            unsigned *count, unsigned width,
34814	                            int (*keycmp)(const void *, const void *)));
34815	
34816	char *lsearch(key, base, count, width, keycmp)
34817	char *key;
34818	char *base;
34819	unsigned *count;
34820	unsigned width;
34821	_PROTOTYPE( int (*keycmp), (const void *, const void *));
34822	{
34823	  char *entry;
34824	  char *last = base + *count * width;
34825	
34826	  for (entry = base; entry < last; entry += width)
34827	        if (keycmp(key, entry) == 0) return(entry);
34828	
34829	  bcopy(key, last, width);
34830	  *count += 1;
34831	  return(last);
34832	}
	
	
.Ep 338 src/lib/other/lsearch.c
34835	char *lfind(key, base, count, width, keycmp)
34836	char *key;
34837	char *base;
34838	unsigned *count;
34839	unsigned width;
34840	_PROTOTYPE( int (*keycmp), (const void *, const void *));
34841	{
34842	  char *entry;
34843	  char *last = base + *count * width;
34844	
34845	  for (entry = base; entry < last; entry += width)
34846	        if (keycmp(key, entry) == 0) return(entry);
34847	
34848	  return((char *)NULL);
34849	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/memccpy.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

34900	#include <lib.h>
34901	/* memccpy - copy bytes up to a certain char
34902	 *
34903	 * CHARBITS should be defined only if the compiler lacks "unsigned char".
34904	 * It should be a mask, e.g. 0377 for an 8-bit machine.
34905	 */
34906	
34907	#include <ansi.h>
34908	#include <stddef.h>
34909	
34910	_PROTOTYPE( void *memccpy, (void *dst, const void *src,
34911	                            int ucharstop, size_t size));
34912	#ifndef CHARBITS
34913	#       define  UNSCHAR(c)      ((unsigned char)(c))
34914	#else
34915	#       define  UNSCHAR(c)      ((c)&CHARBITS)
34916	#endif
34917	
34918	void *memccpy(dst, src, ucharstop, size)
34919	void * dst;
34920	_CONST void * src;
34921	int ucharstop;
34922	_SIZET size;
34923	{
34924	  register char *d;
34925	  register _CONST char *s;
34926	  register _SIZET n;
34927	  register int uc;
34928	
34929	  if (size <= 0) return( (void *) NULL);
34930	
34931	  s = (char *) src;
34932	  d = (char *) dst;
34933	  uc = UNSCHAR(ucharstop);
34934	  for (n = size; n > 0; n--)
.Op 339 src/lib/other/memccpy.c
34935	        if (UNSCHAR(*d++ = *s++) == (char) uc) return( (void *) d);
34936	
34937	  return( (void *) NULL);
34938	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/mtab.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

35000	/* This package consists of 4 routines for handling the /etc/mtab file.
35001	 * The /etc/mtab file contains information about the root and mounted file
35002	 * systems as a series of lines, each one with exactly four fields separated 
35003	 * by one space as follows:
35004	 *
35005	 *      special mounted_on version rw_flag
35006	 *
35007	 * where 
35008	 *      special is the name of the block special file
35009	 *      mounted_on is the directory on which it is mounted
35010	 *      version is either 1 or 2 for MINIX V1 and V2 file systems
35011	 *      rw_flag is rw or ro for read/write or read only
35012	 *
35013	 * An example /etc/mtab:
35014	 *
35015	 *      /dev/ram / 2 rw
35016	 *      /dev/hd1 /usr 2 rw
35017	 *      /dev/fd0 /user 1 ro
35018	 *
35019	 *
35020	 * The four routines for handling /etc/mtab are as follows.  They use two
35021	 * (hidden) internal buffers, mtab_in for input and mtab_out for output.
35022	 *
35023	 *      load_mtab(&prog_name)              - read /etc/mtab into mtab_in
35024	 *      get_mtab_entry(&s1, &s2, &s3, &s4) - arrays that are filled in
35025	 *      put_mtab_entry(&s1, &s2, &s3, &s4) - append a line to mtab_out
35026	 *      rewrite_mtab(&prog_name)           - write mtab_out to /etc/mtab
35027	 *
35028	 * If load_mtab and rewrite_mtab work, they return 0.  If they fail, they
35029	 * print their own error messages on stderr and return -1.  When get_mtab_entry
35030	 * runs out of entries to return, it sets the first pointer to NULL and returns
35031	 * -1 instead of 0.  Also, rewrite_mtab returns -1 if it fails.
35032	 */
35033	 
35034	#include <sys/types.h>
35035	#include <minix/minlib.h>
35036	#include <ctype.h>
35037	#include <fcntl.h>
35038	#include <stdlib.h>
35039	#include <string.h>
35040	#include <unistd.h>
35041	#include <stdio.h>
35042	
35043	#define BUF_SIZE   512            /* size of the /etc/mtab buffer */
35044	
35045	char *etc_mtab = "/etc/mtab";     /* name of the /etc/mtab file */
35046	static char mtab_in[BUF_SIZE+1];  /* holds /etc/mtab when it is read in */
35047	static char mtab_out[BUF_SIZE+1]; /* buf to build /etc/mtab for output later */
35048	static char *iptr = mtab_in;      /* pointer to next line to feed out. */
35049	static char *optr = mtab_out;     /* pointer to place where next line goes */
.Ep 340 src/lib/other/mtab.c
35050	
35051	_PROTOTYPE(int load_mtab, (char *prog_name ));
35052	_PROTOTYPE(int rewrite_mtab, (char *prog_name ));
35053	_PROTOTYPE(int get_mtab_entry, (char *special, char *mounted_on, 
35054	                                        char *version, char *rw_flag));
35055	_PROTOTYPE(int put_mtab_entry, (char *special, char *mounted_on, 
35056	                                        char *version, char *rw_flag));
35057	_PROTOTYPE(void err, (char *prog_name, char *str ));
35058	
35059	
35060	int load_mtab(prog_name)
35061	char *prog_name;
35062	{
35063	/* Read in /etc/mtab and store it in /etc/mtab. */
35064	
35065	  int fd, n;
35066	  char *ptr;
35067	
35068	  /* Open the file. */
35069	  fd = open(etc_mtab, O_RDONLY);
35070	  if (fd < 0) {
35071	        err(prog_name, ": cannot open ");
35072	        return(-1);
35073	  }
35074	
35075	  /* File opened.  Read it in. */
35076	  n = read(fd, mtab_in, BUF_SIZE);
35077	  if (n <= 0) {
35078	        /* Read failed. */
35079	        err(prog_name, ": cannot read ");
35080	        return(-1);
35081	  }
35082	  if (n == BUF_SIZE) {
35083	        /* Some nut has mounted 50 file systems or something like that. */
35084	        std_err(prog_name);
35085	        std_err(": file too large: ");
35086	        std_err(etc_mtab);
35087	        return(-1);
35088	  }
35089	
35090	  close(fd);
35091	
35092	  /* Replace all the whitespace by '\0'. */
35093	  ptr = mtab_in;
35094	  while (*ptr != '\0') {
35095	        if (isspace(*ptr)) *ptr = '\0';
35096	        ptr++;
35097	  }
35098	  return(0);
35099	}
	
	
35102	int rewrite_mtab(prog_name)
35103	char *prog_name;
35104	{
35105	/* Write mtab_out to /etc/mtab. */
35106	
35107	  int fd, n;
35108	
35109	  /* Do a creat to truncate the file. */
.Op 341 src/lib/other/mtab.c
35110	  fd = creat(etc_mtab, 0777);
35111	  if (fd < 0) {
35112	        err(prog_name, ": cannot overwrite ");
35113	        return(-1);
35114	  }
35115	
35116	  /* File created.  Write it. */
35117	  n = write(fd, mtab_out, (unsigned int)(optr - mtab_out));
35118	  if (n <= 0) {
35119	        /* Write failed. */
35120	        err(prog_name, " could not write ");
35121	        return(-1);
35122	  }
35123	
35124	  close(fd);
35125	  return(0);
35126	}
	
	
35129	int get_mtab_entry(special, mounted_on, version, rw_flag)
35130	char *special;
35131	char *mounted_on;
35132	char *version;
35133	char *rw_flag;
35134	{
35135	/* Return the next entry from mtab_in. */
35136	
35137	  if (iptr >= &mtab_in[BUF_SIZE]) {
35138	        special[0] = '\0';
35139	        return(-1);
35140	  }
35141	
35142	  strcpy(special, iptr);
35143	  while (isprint(*iptr)) iptr++;
35144	  while (*iptr == '\0'&& iptr < &mtab_in[BUF_SIZE]) iptr++;
35145	
35146	  strcpy(mounted_on, iptr);
35147	  while (isprint(*iptr)) iptr++;
35148	  while (*iptr == '\0'&& iptr < &mtab_in[BUF_SIZE]) iptr++;
35149	
35150	  strcpy(version, iptr);
35151	  while (isprint(*iptr)) iptr++;
35152	  while (*iptr == '\0'&& iptr < &mtab_in[BUF_SIZE]) iptr++;
35153	
35154	  strcpy(rw_flag, iptr);
35155	  while (isprint(*iptr)) iptr++;
35156	  while (*iptr == '\0'&& iptr < &mtab_in[BUF_SIZE]) iptr++;
35157	  return(0);
35158	}
	
	
35161	int put_mtab_entry(special, mounted_on, version, rw_flag)
35162	char *special;
35163	char *mounted_on;
35164	char *version;
35165	char *rw_flag;
35166	{
35167	/* Append an entry to the mtab_out buffer. */
35168	
35169	  int n1, n2, n3, n4;
.Ep 342 src/lib/other/mtab.c
35170	
35171	  n1 = strlen(special);
35172	  n2 = strlen(mounted_on);
35173	  n3 = strlen(version);
35174	  n4 = strlen(rw_flag);
35175	
35176	  if (optr + n1 + n2 + n3 + n4 + 5 >= &mtab_out[BUF_SIZE]) return(-1);
35177	  strcpy(optr, special);
35178	  optr += n1;
35179	  *optr++ = ' ';
35180	
35181	  strcpy(optr, mounted_on);
35182	  optr += n2;
35183	  *optr++ = ' ';
35184	
35185	  strcpy(optr, version);
35186	  optr += n3;
35187	  *optr++ = ' ';
35188	
35189	  strcpy(optr, rw_flag);
35190	  optr += n4;
35191	  *optr++ = '\n';
35192	  return(0);
35193	}
	
	
35196	void
35197	err(prog_name, str)
35198	char *prog_name, *str;
35199	{
35200	  std_err(prog_name); 
35201	  std_err(str);
35202	  std_err(etc_mtab);
35203	  perror(" ");
35204	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/nlist.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

35300	/*
35301	 * "nlist.c", Peter Valkenburg, january 1989.
35302	 */
35303	 
35304	#include <lib.h>
35305	#include <string.h>
35306	#include <a.out.h>
35307	#include <sys/types.h>
35308	#include <fcntl.h>
35309	#include <unistd.h>
35310	#include <stdio.h>
35311	
35312	#define fail(fp)        (fclose(fp), -1)        /* ret. exp. when nlist fails */
35313	
35314	_PROTOTYPE( int nlist, (char *file, struct nlist nl[]));
.Op 343 src/lib/other/nlist.c
35315	
35316	/*
35317	 * Nlist fills fields n_sclass and n_value of array nl with values found in
35318	 * non-stripped executable file.  Entries that are not found have their
35319	 * n_value/n_sclass fields set to 0.  Nl ends with a 0 or nul string n_name.
35320	 * The return value is -1 on failure, else the number of entries not found.
35321	 */
35322	int nlist(file, nl)
35323	char *file;
35324	struct nlist nl[];
35325	{
35326	        int nents, nsrch, nfound, i;
35327	        struct nlist nlent;
35328	        FILE *fp;
35329	        struct exec hd;
35330	
35331	        /* open executable with namelist */
35332	        if ((fp = fopen(file, "r")) == NULL)
35333	                return -1;
35334	                
35335	        /* get header and seek to start of namelist */  
35336	        if (fread((char *) &hd, sizeof(struct exec), 1, fp) != 1 ||
35337	            BADMAG(hd) || fseek(fp, A_SYMPOS(hd), SEEK_SET) != 0)
35338	                return fail(fp);
35339	        
35340	        /* determine number of entries searched for & reset fields */
35341	        nsrch = 0;
35342	        while (nl[nsrch].n_name != NULL && *(nl[nsrch].n_name) != '\0') {
35343	                nl[nsrch].n_sclass = 0;
35344	                nl[nsrch].n_value = 0;
35345	                nl[nsrch].n_type = 0;           /* for compatability */
35346	                nsrch++;
35347	        }
35348	
35349	        /* loop through namelist & fill in user array */
35350	        nfound = 0;
35351	        for (nents = (hd.a_syms & 0xFFFF) / sizeof(struct nlist);
35352	             nents > 0; nents--) {
35353	                if (nsrch == nfound)
35354	                        break;                  /* no need to look further */
35355	                if (fread((char *) &nlent, sizeof(struct nlist), 1, fp) != 1)
35356	                        return fail(fp);          
35357	                for (i = 0; i < nsrch; i++)
35358	                        if (nl[i].n_sclass == 0 &&
35359	                            strncmp(nl[i].n_name, nlent.n_name,
35360	                                    sizeof(nlent.n_name)) == 0) {
35361	                                nl[i] = nlent;
35362	                                nfound++;
35363	                                break;
35364	                        }
35365	        }
35366	
35367	        (void) fclose(fp);
35368	        
35369	        return nsrch - nfound;
35370	}
.Ep 344 src/lib/other/peekpoke.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/peekpoke.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

35400	/* Peek and poke using /dev/mem.
35401	 *
35402	 * Callers now ought to check the return values.
35403	 *
35404	 * Calling peek() requires read permission on /dev/mem, and consumes
35405	 * a file descriptor.  Calling poke() requires write permission, and
35406	 * consumes another file descriptor.
35407	 */
35408	
35409	#include <sys/types.h>
35410	#include <fcntl.h>
35411	#include <unistd.h>
35412	
35413	_PROTOTYPE( int peek, (unsigned segment, unsigned offset));
35414	_PROTOTYPE( int poke, (unsigned segment, unsigned offset, unsigned value));
35415	
35416	#define SEGSIZE 0x10
35417	
35418	int peek(segment, offset)
35419	unsigned segment;
35420	unsigned offset;
35421	{
35422	  unsigned char chvalue;
35423	  static int infd = -1;
35424	
35425	  if (infd < 0) infd = open("/dev/mem", O_RDONLY);
35426	  if (infd < 0 ||
35427	      lseek(infd, (unsigned long) segment * SEGSIZE + offset, SEEK_SET) < 0 ||
35428	      read(infd, (char *) &chvalue, (unsigned) 1) != 1)
35429	        return(-1);
35430	  return(chvalue);
35431	}
	
35433	int poke(segment, offset, value)
35434	unsigned segment;
35435	unsigned offset;
35436	unsigned value;
35437	{
35438	  unsigned char chvalue;
35439	  static int outfd = -1;
35440	
35441	  chvalue = value;
35442	  if (outfd < 0) outfd = open("/dev/mem", O_WRONLY);
35443	  if (outfd < 0 ||
35444	      lseek(outfd, (unsigned long) segment * SEGSIZE + offset, SEEK_SET) < 0 ||
35445	      write(outfd, (char *) &chvalue, (unsigned) 1) != 1)
35446	        return(-1);
35447	  return(chvalue);
35448	}
.Op 345 src/lib/other/popen.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/popen.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

35500	/*
35501	 * popen - open a pipe
35502	 */
35503	/* $Header: popen.c,v 1.3 90/08/28 14:53:34 eck Exp $ */
35504	
35505	#include        <sys/types.h>
35506	#include        <limits.h>
35507	#include        <errno.h>
35508	#include        <signal.h>
35509	#include        <stdio.h>
35510	
35511	#if     defined(__BSD4_2)
35512	union wait {
35513	        int     w_status;
35514	};
35515	typedef union wait wait_arg;
35516	#else
35517	typedef int wait_arg;
35518	#endif  /* __BSD4_2 */
35519	
35520	#include        "../stdio/loc_incl.h"
35521	
35522	#ifdef _ANSI
35523	int _close(int d);
35524	int _dup2(int oldd, int newd);          /* not present in System 5 */
35525	int _execl(const char *name, const char *_arg, ... );
35526	pid_t _fork(void);
35527	int _pipe(int fildes[2]);
35528	pid_t _wait(wait_arg *status);
35529	void _exit(int status);
35530	#endif
35531	
35532	static int pids[OPEN_MAX];
35533	
35534	FILE *
35535	popen(command, type)
35536	_CONST char *command;
35537	_CONST char *type;
35538	{
35539	        int piped[2];
35540	        int Xtype = *type == 'r' ? 0 : *type == 'w' ? 1 : 2;
35541	        int pid;
35542	
35543	        if (Xtype == 2 ||
35544	            _pipe(piped) < 0 ||
35545	            (pid = _fork()) < 0) return 0;
35546	        
35547	        if (pid == 0) {
35548	                /* child */
35549	                register int *p;
35550	
35551	                for (p = pids; p < &pids[OPEN_MAX]; p++) {
35552	                        if (*p) _close((int)(p - pids));
35553	                }
35554	                _close(piped[Xtype]);
.Ep 346 src/lib/other/popen.c
35555	                _dup2(piped[!Xtype], !Xtype);
35556	                _close(piped[!Xtype]);
35557	                _execl("/bin/sh", "sh", "-c", command, (char *) 0);
35558	                _exit(127);     /* like system() ??? */
35559	        }
35560	
35561	        pids[piped[Xtype]] = pid;
35562	        _close(piped[!Xtype]);
35563	        return fdopen(piped[Xtype], type);
35564	}
	
35566	#if     defined(__BSD4_2)
35567	#define ret_val status.w_status
35568	#else
35569	#define ret_val status
35570	#endif
35571	
35572	int
35573	pclose(stream)
35574	FILE *stream;
35575	{
35576	        int fd = fileno(stream);
35577	        wait_arg status;
35578	        int wret;
35579	
35580	#ifdef _ANSI
35581	        void (*intsave)(int) = signal(SIGINT, SIG_IGN);
35582	        void (*quitsave)(int) = signal(SIGQUIT, SIG_IGN);
35583	#else
35584	        void (*intsave)() = signal(SIGINT, SIG_IGN);
35585	        void (*quitsave)() = signal(SIGQUIT, SIG_IGN);
35586	#endif
35587	        fclose(stream);
35588	        while ((wret = _wait(&status)) != -1) {
35589	                if (wret == pids[fd]) break;
35590	        }
35591	        if (wret == -1) ret_val = -1;
35592	        signal(SIGINT, intsave);
35593	        signal(SIGQUIT, quitsave);
35594	        pids[fd] = 0;
35595	        return ret_val;
35596	}
	
35598	#if     defined(__USG)
35599	int _dup(int fildes);
35600	
35601	static int
35602	_dup2(oldd, newd)
35603	int oldd, newd;
35604	{
35605	        int i = 0, fd, tmp;
35606	        int fdbuf[_NFILES];
35607	
35608	        /* ignore the error on the close() */
35609	        tmp = errno; (void) _close(newd); errno = tmp;
35610	        while ((fd = _dup(oldd)) != newd) {
35611	                if (fd == -1) break;
35612	                fdbuf[i++] = fd;
35613	        }
35614	        tmp = errno;
.Op 347 src/lib/other/popen.c
35615	        while (--i >= 0) {
35616	                _close(fdbuf[i]);
35617	        }
35618	        errno = tmp;
35619	        return -(fd == -1);
35620	}
35621	#endif  /* __USG */




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/printk.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

35700	/*      printk() - kernel printf()                      Author: Kees J. Bot
35701	 *                                                              15 Jan 1994
35702	 */
35703	#define nil 0
35704	#include <stdarg.h>
35705	#include <limits.h>
35706	
35707	#define isdigit(c)      ((unsigned) ((c) - '0') <  (unsigned) 10)
35708	
35709	#if !__STDC__
35710	/* Classic C stuff, ignore. */
35711	void putk();
35712	void printk(fmt) char *fmt;
35713	#else
35714	
35715	/* Printk() uses putk() to print characters. */
35716	void putk(int c);
35717	
35718	void printk(const char *fmt, ...)
35719	#endif
35720	{
35721	        int c;
35722	        enum { LEFT, RIGHT } adjust;
35723	        enum { LONG, INT } intsize;
35724	        int fill;
35725	        int width, max, len, base;
35726	        static char X2C_tab[]= "0123456789ABCDEF";
35727	        static char x2c_tab[]= "0123456789abcdef";
35728	        char *x2c;
35729	        char *p;
35730	        long i;
35731	        unsigned long u;
35732	        char temp[8 * sizeof(long) / 3 + 2];
35733	
35734	        va_list argp;
35735	
35736	        va_start(argp, fmt);
35737	
35738	        while ((c= *fmt++) != 0) {
35739	                if (c != '%') {
35740	                        /* Ordinary character. */
35741	                        putk(c);
35742	                        continue;
35743	                }
35744	
.Ep 348 src/lib/other/printk.c
35745	                /* Format specifier of the form:
35746	                 *      %[adjust][fill][width][.max]keys
35747	                 */
35748	                c= *fmt++;
35749	
35750	                adjust= RIGHT;
35751	                if (c == '-') {
35752	                        adjust= LEFT;
35753	                        c= *fmt++;
35754	                }
35755	
35756	                fill= ' ';
35757	                if (c == '0') {
35758	                        fill= '0';
35759	                        c= *fmt++;
35760	                }
35761	
35762	                width= 0;
35763	                if (c == '*') {
35764	                        /* Width is specified as an argument, e.g. %*d. */
35765	                        width= va_arg(argp, int);
35766	                        c= *fmt++;
35767	                } else
35768	                if (isdigit(c)) {
35769	                        /* A number tells the width, e.g. %10d. */
35770	                        do {
35771	                                width= width * 10 + (c - '0');
35772	                        } while (isdigit(c= *fmt++));
35773	                }
35774	
35775	                max= INT_MAX;
35776	                if (c == '.') {
35777	                        /* Max field length coming up. */
35778	                        if ((c= *fmt++) == '*') {
35779	                                max= va_arg(argp, int);
35780	                                c= *fmt++;
35781	                        } else
35782	                        if (isdigit(c)) {
35783	                                max= 0;
35784	                                do {
35785	                                        max= max * 10 + (c - '0');
35786	                                } while (isdigit(c= *fmt++));
35787	                        }
35788	                }
35789	
35790	                /* Set a few flags to the default. */
35791	                x2c= x2c_tab;
35792	                i= 0;
35793	                base= 10;
35794	                intsize= INT;
35795	                if (c == 'l' || c == 'L') {
35796	                        /* "Long" key, e.g. %ld. */
35797	                        intsize= LONG;
35798	                        c= *fmt++;
35799	                }
35800	                if (c == 0) break;
35801	
35802	                switch (c) {
35803	                        /* Decimal.  Note that %D is treated as %ld. */
35804	                case 'D':
.Op 349 src/lib/other/printk.c
35805	                        intsize= LONG;
35806	                case 'd':
35807	                        i= intsize == LONG ? va_arg(argp, long)
35808	                                                : va_arg(argp, int);
35809	                        u= i < 0 ? -i : i;
35810	                        goto int2ascii;
35811	
35812	                        /* Octal. */
35813	                case 'O':
35814	                        intsize= LONG;
35815	                case 'o':
35816	                        base= 010;
35817	                        goto getint;
35818	
35819	                        /* Hexadecimal.  %X prints upper case A-F, not %lx. */
35820	                case 'X':
35821	                        x2c= X2C_tab;
35822	                case 'x':
35823	                        base= 0x10;
35824	                        goto getint;
35825	
35826	                        /* Unsigned decimal. */
35827	                case 'U':
35828	                        intsize= LONG;
35829	                case 'u':
35830	                getint:
35831	                        u= intsize == LONG ? va_arg(argp, unsigned long)
35832	                                                : va_arg(argp, unsigned int);
35833	                int2ascii:
35834	                        p= temp + sizeof(temp)-1;
35835	                        *p= 0;
35836	                        do {
35837	                                *--p= x2c[u % base];
35838	                        } while ((u /= base) > 0);
35839	                        goto string_length;
35840	
35841	                        /* A character. */
35842	                case 'c':
35843	                        p= temp;
35844	                        *p= va_arg(argp, int);
35845	                        len= 1;
35846	                        goto string_print;
35847	
35848	                        /* Simply a percent. */
35849	                case '%':
35850	                        p= temp;
35851	                        *p= '%';
35852	                        len= 1;
35853	                        goto string_print;
35854	
35855	                        /* A string.  The other cases will join in here. */
35856	                case 's':
35857	                        p= va_arg(argp, char *);
35858	
35859	                string_length:
35860	                        for (len= 0; p[len] != 0 && len < max; len++) {}
35861	
35862	                string_print:
35863	                        width -= len;
35864	                        if (i < 0) width--;
.Ep 350 src/lib/other/printk.c
35865	                        if (fill == '0' && i < 0) putk('-');
35866	                        if (adjust == RIGHT) {
35867	                                while (width > 0) { putk(fill); width--; }
35868	                        }
35869	                        if (fill == ' ' && i < 0) putk('-');
35870	                        while (len > 0) { putk((unsigned char) *p++); len--; }
35871	                        while (width > 0) { putk(fill); width--; }
35872	                        break;
35873	
35874	                        /* Unrecognized format key, echo it back. */
35875	                default:
35876	                        putk('%');
35877	                        putk(c);
35878	                }
35879	        }
35880	
35881	        /* Mark the end with a null (should be something else, like -1). */
35882	        putk(0);
35883	        va_end(argp);
35884	}
	
35886	/*
35887	 * $PchHeader: /mount/hd2/minix/lib/minix/other/RCS/printk.c,v 1.2 1994/09/07 18:45:05 philip Exp $
35888	 */


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/putenv.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

35900	/*
35901	 * (c) copyright 1989 by the Vrije Universiteit, Amsterdam, The Netherlands.
35902	 * See the copyright notice in the ACK home directory, in the file "Copyright".
35903	 */
35904	/* $Header: putenv.c,v 1.3 90/05/14 12:30:18 ceriel Exp $ */
35905	
35906	#include        <stdlib.h>
35907	#include        <string.h>
35908	
35909	#define ENTRY_INC       10
35910	#define rounded(x)      (((x / ENTRY_INC) + 1) * ENTRY_INC)
35911	
35912	extern _CONST char **environ;
35913	
35914	int
35915	putenv(name)
35916	_CONST char *name;
35917	{
35918	        register _CONST char **v = environ;
35919	        register char *r;
35920	        static int size = 0;
35921	        /* When size != 0, it contains the number of entries in the
35922	         * table (including the final NULL pointer). This means that the
35923	         * last non-null entry  is environ[size - 2].
35924	         */
35925	
35926	        if (!name) return 0;
35927	        if (environ == NULL) return 1;
35928	        if (r = strchr(name, '=')) {
35929	                register _CONST char *p, *q;
.Op 351 src/lib/other/putenv.c
35930	
35931	                *r = '\0';
35932	
35933	                if (v != NULL) {
35934	                        while ((p = *v) != NULL) {
35935	                                q = name;
35936	                                while (*q && (*q++ == *p++))
35937	                                        /* EMPTY */ ;
35938	                                if (*q || (*p != '=')) {
35939	                                        v++;
35940	                                } else {
35941	                                        /* The name was already in the
35942	                                         * environment.
35943	                                         */
35944	                                        *r = '=';
35945	                                        *v = name;
35946	                                        return 0;
35947	                                }
35948	                        }
35949	                }
35950	                *r = '=';
35951	                v = environ;
35952	        }
35953	
35954	        if (!size) {
35955	                register _CONST char **p;
35956	                register int i = 0;
35957	
35958	                if (v)
35959	                        do {
35960	                                i++;
35961	                        } while (*v++);
35962	                if (!(v = malloc(rounded(i) * sizeof(char **))))
35963	                        return 1;
35964	                size = i;
35965	                p = environ;
35966	                environ = v;
35967	                while (*v++ = *p++);            /* copy the environment */
35968	                v = environ;
35969	        } else if (!(size % ENTRY_INC)) {
35970	                if (!(v = realloc(environ, rounded(size) * sizeof(char **))))
35971	                        return 1;
35972	                environ = v;
35973	        }
35974	        v[size - 1] = name;
35975	        v[size] = NULL;
35976	        size++;
35977	        return 0;
35978	}
.Ep 352 src/lib/other/putw.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/putw.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

36000	/* 
36001	 * putw - write an word on a stream
36002	 */
36003	/* $Header: putw.c,v 1.1 89/12/18 14:40:15 eck Exp $ */
36004	
36005	#include        <stdio.h>
36006	
36007	_PROTOTYPE(int putw, (int w, FILE *stream ));
36008	
36009	int
36010	putw(w, stream)
36011	int w;
36012	register FILE *stream;
36013	{
36014	        register int cnt = sizeof(int);
36015	        register char *p = (char *) &w;
36016	
36017	        while (cnt--) {
36018	                putc(*p++, stream);
36019	        }
36020	        if (ferror(stream)) return EOF;
36021	        return w;
36022	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/regexp.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

36100	/* regcomp and regexec -- regsub and regerror are elsewhere
36101	 *
36102	 *      Copyright (c) 1986 by University of Toronto.
36103	 *      Written by Henry Spencer.  Not derived from licensed software.
36104	 *
36105	 *      Permission is granted to anyone to use this software for any
36106	 *      purpose on any computer system, and to redistribute it freely,
36107	 *      subject to the following restrictions:
36108	 *
36109	 *      1. The author is not responsible for the consequences of use of
36110	 *              this software, no matter how awful, even if they arise
36111	 *              from defects in it.
36112	 *
36113	 *      2. The origin of this software must not be misrepresented, either
36114	 *              by explicit claim or by omission.
36115	 *
36116	 *      3. Altered versions must be plainly marked as such, and must not
36117	 *              be misrepresented as being the original software.
36118	 *
36119	 * Beware that some of this code is subtly aware of the way operator
36120	 * precedence is structured in regular expressions.  Serious changes in
36121	 * regular-expression syntax might require a total rethink.
36122	 *
36123	 *      The third parameter to regexec was added by Martin C. Atkins.
36124	 *      Andy Tanenbaum also made some changes.
.Op 353 src/lib/other/regexp.c
36125	 */
36126	
36127	#include <minix/config.h>
36128	#include <minix/const.h>
36129	#include <stdlib.h>
36130	#include <string.h>
36131	#include <regexp.h>
36132	#include <stdio.h>
36133	
36134	/* The first byte of the regexp internal "program" is actually this magic
36135	 * number; the start node begins in the second byte.
36136	 */
36137	#define MAGIC   0234
36138	
36139	/* The "internal use only" fields in regexp.h are present to pass info from
36140	 * compile to execute that permits the execute phase to run lots faster on
36141	 * simple cases.  They are:
36142	 *
36143	 * regstart     char that must begin a match; '\0' if none obvious
36144	 * reganch      is the match anchored (at beginning-of-line only)?
36145	 * regmust      string (pointer into program) that match must include, or NULL
36146	 * regmlen      length of regmust string
36147	 *
36148	 * Regstart and reganch permit very fast decisions on suitable starting points
36149	 * for a match, cutting down the work a lot.  Regmust permits fast rejection
36150	 * of lines that cannot possibly match.  The regmust tests are costly enough
36151	 * that regcomp() supplies a regmust only if the r.e. contains something
36152	 * potentially expensive (at present, the only such thing detected is * or +
36153	 * at the start of the r.e., which can involve a lot of backup).  Regmlen is
36154	 * supplied because the test in regexec() needs it and regcomp() is computing
36155	 * it anyway.
36156	 */
36157	
36158	/* Structure for regexp "program".  This is essentially a linear encoding
36159	 * of a nondeterministic finite-state machine (aka syntax charts or
36160	 * "railroad normal form" in parsing technology).  Each node is an opcode
36161	 * plus a "next" pointer, possibly plus an operand.  "Next" pointers of
36162	 * all nodes except BRANCH implement concatenation; a "next" pointer with
36163	 * a BRANCH on both ends of it is connecting two alternatives.  (Here we
36164	 * have one of the subtle syntax dependencies:  an individual BRANCH (as
36165	 * opposed to a collection of them) is never concatenated with anything
36166	 * because of operator precedence.)  The operand of some types of node is
36167	 * a literal string; for others, it is a node leading into a sub-FSM.  In
36168	 * particular, the operand of a BRANCH node is the first node of the branch.
36169	 * (NB this is *not* a tree structure:  the tail of the branch connects
36170	 * to the thing following the set of BRANCHes.)  The opcodes are:
36171	 */
36172	
36173	/* Definition   number  opnd?   meaning */
36174	#define END     0               /* no   End of program. */
36175	#define BOL     1               /* no   Match "" at beginning of line. */
36176	#define EOL     2               /* no   Match "" at end of line. */
36177	#define ANY     3               /* no   Match any one character. */
36178	#define ANYOF   4               /* str  Match any character in this string. */
36179	#define ANYBUT  5               /* str  Match any character not in this
36180	                         * string. */
36181	#define BRANCH  6               /* node Match this alternative, or the
36182	                         * next... */
36183	#define BACK    7               /* no   Match "", "next" ptr points backward. */
36184	#define EXACTLY 8               /* str  Match this string. */
.Ep 354 src/lib/other/regexp.c
36185	#define NOTHING 9               /* no   Match empty string. */
36186	#define STAR    10              /* node Match this (simple) thing 0 or more
36187	                         * times. */
36188	#define PLUS    11              /* node Match this (simple) thing 1 or more
36189	                         * times. */
36190	#define OPEN    20              /* no   Mark this point in input as start of
36191	                         * #n. */
36192	 /* OPEN+1 is number 1, etc. */
36193	#define CLOSE   30              /* no   Analogous to OPEN. */
36194	
36195	/* Opcode notes:
36196	 *
36197	 * BRANCH       The set of branches constituting a single choice are hooked
36198	 *              together with their "next" pointers, since precedence prevents
36199	 *              anything being concatenated to any individual branch.  The
36200	 *              "next" pointer of the last BRANCH in a choice points to the
36201	 *              thing following the whole choice.  This is also where the
36202	 *              final "next" pointer of each individual branch points; each
36203	 *              branch starts with the operand node of a BRANCH node.
36204	 *
36205	 * BACK         Normal "next" pointers all implicitly point forward; BACK
36206	 *              exists to make loop structures possible.
36207	 *
36208	 * STAR,PLUS    '?', and complex '*' and '+', are implemented as circular
36209	 *              BRANCH structures using BACK.  Simple cases (one character
36210	 *              per match) are implemented with STAR and PLUS for speed
36211	 *              and to minimize recursive plunges.
36212	 *
36213	 * OPEN,CLOSE   ...are numbered at compile time.
36214	 */
36215	
36216	/* A node is one char of opcode followed by two chars of "next" pointer.
36217	 * "Next" pointers are stored as two 8-bit pieces, high order first.  The
36218	 * value is a positive offset from the opcode of the node containing it.
36219	 * An operand, if any, simply follows the node.  (Note that much of the
36220	 * code generation knows about this implicit relationship.)
36221	 *
36222	 * Using two bytes for the "next" pointer is vast overkill for most things,
36223	 * but allows patterns to get big without disasters.
36224	 */
36225	#define OP(p)   (*(p))
36226	#define NEXT(p) (((*((p)+1)&0377)<<8) + (*((p)+2)&0377))
36227	#define OPERAND(p)      ((p) + 3)
36228	
36229	/* Utility definitions.
36230	 */
36231	#ifndef CHARBITS
36232	#define UCHARAT(p)      ((int)*(unsigned char *)(p))
36233	#else
36234	#define UCHARAT(p)      ((int)*(p)&CHARBITS)
36235	#endif
36236	
36237	#define CFAIL(m)        { regerror(m); return((char *)NULL); }
36238	#define RFAIL(m)        { regerror(m); return((regexp *)NULL); }
36239	#define ISMULT(c)       ((c) == '*' || (c) == '+' || (c) == '?')
36240	#define META    "^$.[()|?+*\\"
36241	
36242	/* Flags to be passed up and down.
36243	 */
36244	#define HASWIDTH        01      /* Known never to match null string. */
.Op 355 src/lib/other/regexp.c
36245	#define SIMPLE          02      /* Simple enough to be STAR/PLUS operand. */
36246	#define SPSTART         04      /* Starts with * or +. */
36247	#define WORST           0       /* Worst case. */
36248	
36249	/* Global work variables for regcomp().
36250	 */
36251	PRIVATE char *regparse;         /* Input-scan pointer. */
36252	PRIVATE int regnpar;            /* () count. */
36253	PRIVATE char regdummy;
36254	PRIVATE char *regcode;          /* Code-emit pointer; &regdummy = don't. */
36255	PRIVATE long regsize;           /* Code size. */
36256	
36257	/* Forward declarations for regcomp()'s friends.
36258	 */
36259	#ifndef STATIC
36260	#define STATIC  PRIVATE
36261	#endif
36262	STATIC _PROTOTYPE( char *reg, (int paren, int *flagp)                   );
36263	STATIC _PROTOTYPE( char *regbranch, (int *flagp)                        );
36264	STATIC _PROTOTYPE( char *regpiece, (int *flagp)                         );
36265	STATIC _PROTOTYPE( char *regatom, (int *flagp)                          );
36266	STATIC _PROTOTYPE( char *regnode, (int op)                              );
36267	STATIC _PROTOTYPE( char *regnext, (char *p)                             );
36268	STATIC _PROTOTYPE( void regc, (int b)                                   );
36269	STATIC _PROTOTYPE( void reginsert, (int op, char *opnd)                 );
36270	STATIC _PROTOTYPE( void regtail, (char *p, char *val)                   );
36271	STATIC _PROTOTYPE( void regoptail, (char *p, char *val)                 );
36272	
36273	/*
36274	 - regcomp - compile a regular expression into internal code
36275	 *
36276	 * We can't allocate space until we know how big the compiled form will be,
36277	 * but we can't compile it (and thus know how big it is) until we've got a
36278	 * place to put the code.  So we cheat:  we compile it twice, once with code
36279	 * generation turned off and size counting turned on, and once "for real".
36280	 * This also means that we don't allocate space until we are sure that the
36281	 * thing really will compile successfully, and we never have to move the
36282	 * code and thus invalidate pointers into it.  (Note that it has to be in
36283	 * one piece because free() must be able to free it all.)
36284	 *
36285	 * Beware that the optimization-preparation code in here knows about some
36286	 * of the structure of the compiled regexp.
36287	 */
36288	regexp *regcomp(exp)
36289	char *exp;
36290	{
36291	  register regexp *r;
36292	  register char *scan;
36293	  register char *longest;
36294	  register int len;
36295	  int flags;
36296	
36297	  if (exp == (char *)NULL) RFAIL("NULL argument");
36298	
36299	  /* First pass: determine size, legality. */
36300	  regparse = exp;
36301	  regnpar = 1;
36302	  regsize = 0L;
36303	  regcode = &regdummy;
36304	  regc(MAGIC);
.Ep 356 src/lib/other/regexp.c
36305	  if (reg(0, &flags) == (char *)NULL) return((regexp *)NULL);
36306	
36307	  /* Small enough for pointer-storage convention? */
36308	  if (regsize >= 32767L)        /* Probably could be 65535L. */
36309	        RFAIL("regexp too big");
36310	
36311	  /* Allocate space. */
36312	  r = (regexp *) malloc(sizeof(regexp) + (unsigned) regsize);
36313	  if (r == (regexp *)NULL) RFAIL("out of space");
36314	
36315	  /* Second pass: emit code. */
36316	  regparse = exp;
36317	  regnpar = 1;
36318	  regcode = r->program;
36319	  regc(MAGIC);
36320	  if (reg(0, &flags) == (char *)NULL) return((regexp *)NULL);
36321	
36322	  /* Dig out information for optimizations. */
36323	  r->regstart = '\0';           /* Worst-case defaults. */
36324	  r->reganch = 0;
36325	  r->regmust = (char *)NULL;
36326	  r->regmlen = 0;
36327	  scan = r->program + 1;        /* First BRANCH. */
36328	  if (OP(regnext(scan)) == END) {       /* Only one top-level choice. */
36329	        scan = OPERAND(scan);
36330	
36331	        /* Starting-point info. */
36332	        if (OP(scan) == EXACTLY)
36333	                r->regstart = *OPERAND(scan);
36334	        else if (OP(scan) == BOL)
36335	                r->reganch++;
36336	
36337	        /* If there's something expensive in the r.e., find the
36338	         * longest literal string that must appear and make it the
36339	         * regmust.  Resolve ties in favor of later strings, since
36340	         * the regstart check works with the beginning of the r.e.
36341	         * and avoiding duplication strengthens checking.  Not a
36342	         * strong reason, but sufficient in the absence of others. */
36343	        if (flags & SPSTART) {
36344	                longest = (char *)NULL;
36345	                len = 0;
36346	                for (; scan != (char *)NULL; scan = regnext(scan))
36347	                        if (OP(scan) == EXACTLY && strlen(OPERAND(scan)) >= len) {
36348	                                longest = OPERAND(scan);
36349	                                len = strlen(OPERAND(scan));
36350	                        }
36351	                r->regmust = longest;
36352	                r->regmlen = len;
36353	        }
36354	  }
36355	  return(r);
36356	}
	
36358	/*
36359	 - reg - regular expression, i.e. main body or parenthesized thing
36360	 *
36361	 * Caller must absorb opening parenthesis.
36362	 *
36363	 * Combining parenthesis handling with the base level of regular expression
36364	 * is a trifle forced, but the need to tie the tails of the branches to what
.Op 357 src/lib/other/regexp.c
36365	 * follows makes it hard to avoid.
36366	 */
36367	PRIVATE char *reg(paren, flagp)
36368	int paren;                      /* Parenthesized? */
36369	int *flagp;
36370	{
36371	  register char *ret;
36372	  register char *br;
36373	  register char *ender;
36374	  register int parno;
36375	  int flags;
36376	
36377	  *flagp = HASWIDTH;            /* Tentatively. */
36378	
36379	  /* Make an OPEN node, if parenthesized. */
36380	  if (paren) {
36381	        if (regnpar >= NSUBEXP) CFAIL("too many ()");
36382	        parno = regnpar;
36383	        regnpar++;
36384	        ret = regnode(OPEN + parno);
36385	  } else {
36386	        parno = 0;              /* not actually used, keep compiler quiet */
36387	        ret = (char *)NULL;
36388	  }
36389	
36390	  /* Pick up the branches, linking them together. */
36391	  br = regbranch(&flags);
36392	  if (br == (char *)NULL) return((char *)NULL);
36393	  if (ret != (char *)NULL)
36394	        regtail(ret, br);       /* OPEN -> first. */
36395	  else
36396	        ret = br;
36397	  if (!(flags & HASWIDTH)) *flagp &= ~HASWIDTH;
36398	  *flagp |= flags & SPSTART;
36399	  while (*regparse == '|') {
36400	        regparse++;
36401	        br = regbranch(&flags);
36402	        if (br == (char *)NULL) return((char *)NULL);
36403	        regtail(ret, br);       /* BRANCH -> BRANCH. */
36404	        if (!(flags & HASWIDTH)) *flagp &= ~HASWIDTH;
36405	        *flagp |= flags & SPSTART;
36406	  }
36407	
36408	  /* Make a closing node, and hook it on the end. */
36409	  ender = regnode((paren) ? CLOSE + parno : END);
36410	  regtail(ret, ender);
36411	
36412	  /* Hook the tails of the branches to the closing node. */
36413	  for (br = ret; br != (char *)NULL; br = regnext(br)) regoptail(br, ender);
36414	
36415	  /* Check for proper termination. */
36416	  if (paren && *regparse++ != ')') {
36417	        CFAIL("unmatched ()");
36418	  } else if (!paren && *regparse != '\0') {
36419	        if (*regparse == ')') {
36420	                CFAIL("unmatched ()");
36421	        } else
36422	                CFAIL("junk on end");   /* "Can't happen". */
36423	        /* NOTREACHED */
36424	  }
.Ep 358 src/lib/other/regexp.c
36425	  return(ret);
36426	}
	
36428	/*
36429	 - regbranch - one alternative of an | operator
36430	 *
36431	 * Implements the concatenation operator.
36432	 */
36433	PRIVATE char *regbranch(flagp)
36434	int *flagp;
36435	{
36436	  register char *ret;
36437	  register char *chain;
36438	  register char *latest;
36439	  int flags;
36440	
36441	  *flagp = WORST;               /* Tentatively. */
36442	
36443	  ret = regnode(BRANCH);
36444	  chain = (char *)NULL;
36445	  while (*regparse != '\0' && *regparse != '|' && *regparse != ')') {
36446	        latest = regpiece(&flags);
36447	        if (latest == (char *)NULL) return((char *)NULL);
36448	        *flagp |= flags & HASWIDTH;
36449	        if (chain == (char *)NULL)      /* First piece. */
36450	                *flagp |= flags & SPSTART;
36451	        else
36452	                regtail(chain, latest);
36453	        chain = latest;
36454	  }
36455	  if (chain == (char *)NULL)            /* Loop ran zero times. */
36456	        regnode(NOTHING);
36457	
36458	  return(ret);
36459	}
	
36461	/*
36462	 - regpiece - something followed by possible [*+?]
36463	 *
36464	 * Note that the branching code sequences used for ? and the general cases
36465	 * of * and + are somewhat optimized:  they use the same NOTHING node as
36466	 * both the endmarker for their branch list and the body of the last branch.
36467	 * It might seem that this node could be dispensed with entirely, but the
36468	 * endmarker role is not redundant.
36469	 */
36470	PRIVATE char *regpiece(flagp)
36471	int *flagp;
36472	{
36473	  register char *ret;
36474	  register char op;
36475	  register char *next;
36476	  int flags;
36477	
36478	  ret = regatom(&flags);
36479	  if (ret == (char *)NULL) return((char *)NULL);
36480	
36481	  op = *regparse;
36482	  if (!ISMULT(op)) {
36483	        *flagp = flags;
36484	        return(ret);
.Op 359 src/lib/other/regexp.c
36485	  }
36486	  if (!(flags & HASWIDTH) && op != '?') CFAIL("*+ operand could be empty");
36487	  *flagp = (op != '+') ? (WORST | SPSTART) : (WORST | HASWIDTH);
36488	
36489	  if (op == '*' && (flags & SIMPLE))
36490	        reginsert(STAR, ret);
36491	  else if (op == '*') {
36492	        /* Emit x* as (x&|), where & means "self". */
36493	        reginsert(BRANCH, ret); /* Either x */
36494	        regoptail(ret, regnode(BACK));  /* and loop */
36495	        regoptail(ret, ret);    /* back */
36496	        regtail(ret, regnode(BRANCH));  /* or */
36497	        regtail(ret, regnode(NOTHING)); /* null. */
36498	  } else if (op == '+' && (flags & SIMPLE))
36499	        reginsert(PLUS, ret);
36500	  else if (op == '+') {
36501	        /* Emit x+ as x(&|), where & means "self". */
36502	        next = regnode(BRANCH); /* Either */
36503	        regtail(ret, next);
36504	        regtail(regnode(BACK), ret);    /* loop back */
36505	        regtail(next, regnode(BRANCH)); /* or */
36506	        regtail(ret, regnode(NOTHING)); /* null. */
36507	  } else if (op == '?') {
36508	        /* Emit x? as (x|) */
36509	        reginsert(BRANCH, ret); /* Either x */
36510	        regtail(ret, regnode(BRANCH));  /* or */
36511	        next = regnode(NOTHING);/* null. */
36512	        regtail(ret, next);
36513	        regoptail(ret, next);
36514	  }
36515	  regparse++;
36516	  if (ISMULT(*regparse)) CFAIL("nested *?+");
36517	
36518	  return(ret);
36519	}
	
36521	/*
36522	 - regatom - the lowest level
36523	 *
36524	 * Optimization:  gobbles an entire sequence of ordinary characters so that
36525	 * it can turn them into a single node, which is smaller to store and
36526	 * faster to run.  Backslashed characters are exceptions, each becoming a
36527	 * separate node; the code is simpler that way and it's not worth fixing.
36528	 */
36529	PRIVATE char *regatom(flagp)
36530	int *flagp;
36531	{
36532	  register char *ret;
36533	  int flags;
36534	
36535	  *flagp = WORST;               /* Tentatively. */
36536	
36537	  switch (*regparse++) {
36538	      case '^': ret = regnode(BOL);             break;
36539	      case '$': ret = regnode(EOL);             break;
36540	      case '.':
36541	        ret = regnode(ANY);
36542	        *flagp |= HASWIDTH | SIMPLE;
36543	        break;
36544	      case '[':{
.Ep 360 src/lib/other/regexp.c
36545	                register int class;
36546	                register int classend;
36547	
36548	                if (*regparse == '^') { /* Complement of range. */
36549	                        ret = regnode(ANYBUT);
36550	                        regparse++;
36551	                } else
36552	                        ret = regnode(ANYOF);
36553	                if (*regparse == ']' || *regparse == '-') regc(*regparse++);
36554	                while (*regparse != '\0' && *regparse != ']') {
36555	                        if (*regparse == '-') {
36556	                                regparse++;
36557	                                if (*regparse == ']' || *regparse == '\0')
36558	                                        regc('-');
36559	                                else {
36560	                                        class = UCHARAT(regparse - 2) + 1;
36561	                                        classend = UCHARAT(regparse);
36562	                                        if (class > classend + 1)
36563	                                                CFAIL("invalid [] range");
36564	                                        for (; class <= classend; class++)
36565	                                                regc(class);
36566	                                        regparse++;
36567	                                }
36568	                        } else
36569	                                regc(*regparse++);
36570	                }
36571	                regc('\0');
36572	                if (*regparse != ']') CFAIL("unmatched []");
36573	                regparse++;
36574	                *flagp |= HASWIDTH | SIMPLE;
36575	        }
36576	        break;
36577	      case '(':
36578	        ret = reg(1, &flags);
36579	        if (ret == (char *)NULL) return((char *)NULL);
36580	        *flagp |= flags & (HASWIDTH | SPSTART);
36581	        break;
36582	      case '\0':
36583	      case '|':
36584	      case ')':
36585	        CFAIL("internal urp");  /* Supposed to be caught earlier. */
36586	        break;
36587	      case '?':
36588	      case '+':
36589	      case '*': CFAIL("?+* follows nothing");           break;
36590	      case '\\':
36591	        if (*regparse == '\0') CFAIL("trailing \\");
36592	        ret = regnode(EXACTLY);
36593	        regc(*regparse++);
36594	        regc('\0');
36595	        *flagp |= HASWIDTH | SIMPLE;
36596	        break;
36597	      default:{
36598	                register int len;
36599	                register char ender;
36600	
36601	                regparse--;
36602	                len = strcspn(regparse, META);
36603	                if (len <= 0) CFAIL("internal disaster");
36604	                ender = *(regparse + len);
.Op 361 src/lib/other/regexp.c
36605	                if (len > 1 && ISMULT(ender))
36606	                        len--;  /* Back off clear of ?+* operand. */
36607	                *flagp |= HASWIDTH;
36608	                if (len == 1) *flagp |= SIMPLE;
36609	                ret = regnode(EXACTLY);
36610	                while (len > 0) {
36611	                        regc(*regparse++);
36612	                        len--;
36613	                }
36614	                regc('\0');
36615	        }
36616	        break;
36617	  }
36618	
36619	  return(ret);
36620	}
	
36622	/*
36623	 - regnode - emit a node
36624	 */
36625	PRIVATE char *regnode(op)
36626	char op;
36627	{
36628	  register char *ret;
36629	  register char *ptr;
36630	
36631	  ret = regcode;
36632	  if (ret == &regdummy) {
36633	        regsize += 3;
36634	        return(ret);
36635	  }
36636	  ptr = ret;
36637	  *ptr++ = op;
36638	  *ptr++ = '\0';                /* Null "next" pointer. */
36639	  *ptr++ = '\0';
36640	  regcode = ptr;
36641	
36642	  return(ret);
36643	}
	
36645	/*
36646	 - regc - emit (if appropriate) a byte of code
36647	 */
36648	PRIVATE void regc(b)
36649	char b;
36650	{
36651	  if (regcode != &regdummy)
36652	        *regcode++ = b;
36653	  else
36654	        regsize++;
36655	}
	
36657	/*
36658	 - reginsert - insert an operator in front of already-emitted operand
36659	 *
36660	 * Means relocating the operand.
36661	 */
36662	PRIVATE void reginsert(op, opnd)
36663	char op;
36664	char *opnd;
.Ep 362 src/lib/other/regexp.c
36665	{
36666	  register char *src;
36667	  register char *dst;
36668	  register char *place;
36669	
36670	  if (regcode == &regdummy) {
36671	        regsize += 3;
36672	        return;
36673	  }
36674	  src = regcode;
36675	  regcode += 3;
36676	  dst = regcode;
36677	  while (src > opnd) *--dst = *--src;
36678	
36679	  place = opnd;                 /* Op node, where operand used to be. */
36680	  *place++ = op;
36681	  *place++ = '\0';
36682	  *place++ = '\0';
36683	}
	
36685	/*
36686	 - regtail - set the next-pointer at the end of a node chain
36687	 */
36688	PRIVATE void regtail(p, val)
36689	char *p;
36690	char *val;
36691	{
36692	  register char *scan;
36693	  register char *temp;
36694	  register int offset;
36695	
36696	  if (p == &regdummy) return;
36697	
36698	  /* Find last node. */
36699	  scan = p;
36700	  for (;;) {
36701	        temp = regnext(scan);
36702	        if (temp == (char *)NULL) break;
36703	        scan = temp;
36704	  }
36705	
36706	  if (OP(scan) == BACK)
36707	        offset = scan - val;
36708	  else
36709	        offset = val - scan;
36710	  *(scan + 1) = (offset >> 8) & 0377;
36711	  *(scan + 2) = offset & 0377;
36712	}
	
36714	/*
36715	 - regoptail - regtail on operand of first argument; nop if operandless
36716	 */
36717	PRIVATE void regoptail(p, val)
36718	char *p;
36719	char *val;
36720	{
36721	  /* "Operandless" and "op != BRANCH" are synonymous in practice. */
36722	  if (p == (char *)NULL || p == &regdummy || OP(p) != BRANCH) return;
36723	  regtail(OPERAND(p), val);
36724	}
.Op 363 src/lib/other/regexp.c
	
36726	/* regexec and friends
36727	 */
36728	
36729	/* Global work variables for regexec().
36730	 */
36731	PRIVATE char *reginput;         /* String-input pointer. */
36732	PRIVATE char *regbol;           /* Beginning of input, for ^ check. */
36733	PRIVATE char **regstartp;       /* Pointer to startp array. */
36734	PRIVATE char **regendp;         /* Ditto for endp. */
36735	
36736	/* Forwards.
36737	 */
36738	STATIC _PROTOTYPE( int regtry, (regexp *prog, char *string)             );
36739	STATIC _PROTOTYPE( int regmatch, (char *prog)                           );
36740	STATIC _PROTOTYPE( int regrepeat, (char *p)                             );
36741	
36742	#ifdef DEBUG
36743	int regnarrate = 0;
36744	void regdump();
36745	STATIC _PROTOTYPE( char *regprop, (char *op)                            );
36746	#endif
36747	
36748	/*
36749	 - regexec - match a regexp against a string
36750	 */
36751	int regexec(prog, string, bolflag)
36752	register regexp *prog;
36753	register char *string;
36754	int bolflag;
36755	{
36756	  register char *s;
36757	
36758	  /* Be paranoid... */
36759	  if (prog == (regexp *)NULL || string == (char *)NULL) {
36760	        regerror("NULL parameter");
36761	        return(0);
36762	  }
36763	
36764	  /* Check validity of program. */
36765	  if (UCHARAT(prog->program) != MAGIC) {
36766	        regerror("corrupted program");
36767	        return(0);
36768	  }
36769	
36770	  /* If there is a "must appear" string, look for it. */
36771	  if (prog->regmust != (char *)NULL) {
36772	        s = string;
36773	        while ((s = strchr(s, prog->regmust[0])) != (char *)NULL) {
36774	                if (strncmp(s, prog->regmust, prog->regmlen) == 0)
36775	                        break;  /* Found it. */
36776	                s++;
36777	        }
36778	        if (s == (char *)NULL)          /* Not present. */
36779	                return(0);
36780	  }
36781	
36782	  /* Mark beginning of line for ^ . */
36783	  if (bolflag)
36784	        regbol = string;
.Ep 364 src/lib/other/regexp.c
36785	  else
36786	        regbol = (char *)NULL;
36787	
36788	  /* Simplest case:  anchored match need be tried only once. */
36789	  if (prog->reganch) return(regtry(prog, string));
36790	
36791	  /* Messy cases:  unanchored match. */
36792	  s = string;
36793	  if (prog->regstart != '\0')   /* We know what char it must start with. */
36794	        while ((s = strchr(s, prog->regstart)) != (char *)NULL) {
36795	                if (regtry(prog, s)) return(1);
36796	                s++;
36797	        }
36798	  else
36799	        /* We don't -- general case. */
36800	        do {
36801	                if (regtry(prog, s)) return(1);
36802	        } while (*s++ != '\0');
36803	
36804	  /* Failure. */
36805	  return(0);
36806	}
	
36808	/*
36809	 - regtry - try match at specific point
36810	 */
36811	PRIVATE int regtry(prog, string)   /* 0 failure, 1 success */
36812	regexp *prog;
36813	char *string;
36814	{
36815	  register int i;
36816	  register char **sp;
36817	  register char **ep;
36818	
36819	  reginput = string;
36820	  regstartp = prog->startp;
36821	  regendp = prog->endp;
36822	
36823	  sp = prog->startp;
36824	  ep = prog->endp;
36825	  for (i = NSUBEXP; i > 0; i--) {
36826	        *sp++ = (char *)NULL;
36827	        *ep++ = (char *)NULL;
36828	  }
36829	  if (regmatch(prog->program + 1)) {
36830	        prog->startp[0] = string;
36831	        prog->endp[0] = reginput;
36832	        return(1);
36833	  } else
36834	        return(0);
36835	}
	
36837	/*
36838	 - regmatch - main matching routine
36839	 *
36840	 * Conceptually the strategy is simple:  check to see whether the current
36841	 * node matches, call self recursively to see whether the rest matches,
36842	 * and then act accordingly.  In practice we make some effort to avoid
36843	 * recursion, in particular by going through "ordinary" nodes (that don't
36844	 * need to know whether the rest of the match failed) by a loop instead of
.Op 365 src/lib/other/regexp.c
36845	 * by recursion.
36846	 */
36847	PRIVATE int regmatch(prog)      /* 0 failure, 1 success */ 
36848	char *prog;
36849	{
36850	  register char *scan;          /* Current node. */
36851	  char *next;                   /* Next node. */
36852	
36853	  scan = prog;
36854	#ifdef DEBUG
36855	  if (scan != (char *)NULL && regnarrate) fprintf(stderr, "%s(\n", regprop(scan));
36856	#endif
36857	  while (scan != (char *)NULL) {
36858	#ifdef DEBUG
36859	        if (regnarrate) fprintf(stderr, "%s...\n", regprop(scan));
36860	#endif
36861	        next = regnext(scan);
36862	
36863	        switch (OP(scan)) {
36864	            case BOL:
36865	                if (reginput != regbol) return(0);
36866	                break;
36867	            case EOL:
36868	                if (*reginput != '\0') return(0);
36869	                break;
36870	            case ANY:
36871	                if (*reginput == '\0') return(0);
36872	                reginput++;
36873	                break;
36874	            case EXACTLY:{
36875	                        register int len;
36876	                        register char *opnd;
36877	
36878	                        opnd = OPERAND(scan);
36879	                        /* Inline the first character, for speed. */
36880	                        if (*opnd != *reginput) return(0);
36881	                        len = strlen(opnd);
36882	                        if (len > 1 && strncmp(opnd, reginput, len) != 0)
36883	                                return(0);
36884	                        reginput += len;
36885	                }
36886	                break;
36887	            case ANYOF:
36888	                if (*reginput == '\0' || strchr(OPERAND(scan), *reginput) == (char *)NULL)
36889	                        return(0);
36890	                reginput++;
36891	                break;
36892	            case ANYBUT:
36893	                if (*reginput == '\0' || strchr(OPERAND(scan), *reginput) != (char *)NULL)
36894	                        return(0);
36895	                reginput++;
36896	                break;
36897	            case NOTHING:
36898	                break;
36899	            case BACK:
36900	                break;
36901	            case OPEN + 1:
36902	            case OPEN + 2:
36903	            case OPEN + 3:
36904	            case OPEN + 4:
.Ep 366 src/lib/other/regexp.c
36905	            case OPEN + 5:
36906	            case OPEN + 6:
36907	            case OPEN + 7:
36908	            case OPEN + 8:
36909	            case OPEN + 9:{
36910	                        register int no;
36911	                        register char *save;
36912	
36913	                        no = OP(scan) - OPEN;
36914	                        save = reginput;
36915	
36916	                        if (regmatch(next)) {
36917	                                /* Don't set startp if some later
36918	                                 * invocation of the same parentheses
36919	                                 * already has. */
36920	                                if (regstartp[no] == (char *)NULL)
36921	                                        regstartp[no] = save;
36922	                                return(1);
36923	                        } else
36924	                                return(0);
36925	                }
36926	                break;
36927	            case CLOSE + 1:
36928	            case CLOSE + 2:
36929	            case CLOSE + 3:
36930	            case CLOSE + 4:
36931	            case CLOSE + 5:
36932	            case CLOSE + 6:
36933	            case CLOSE + 7:
36934	            case CLOSE + 8:
36935	            case CLOSE + 9:{
36936	                        register int no;
36937	                        register char *save;
36938	
36939	                        no = OP(scan) - CLOSE;
36940	                        save = reginput;
36941	
36942	                        if (regmatch(next)) {
36943	                                /* Don't set endp if some later
36944	                                 * invocation of the same parentheses
36945	                                 * already has. */
36946	                                if (regendp[no] == (char *)NULL) regendp[no] = save;
36947	                                return(1);
36948	                        } else
36949	                                return(0);
36950	                }
36951	                break;
36952	            case BRANCH:{
36953	                        register char *save;
36954	
36955	                        if (OP(next) != BRANCH) /* No choice. */
36956	                                next = OPERAND(scan);   /* Avoid recursion. */
36957	                        else {
36958	                                do {
36959	                                        save = reginput;
36960	                                        if (regmatch(OPERAND(scan)))
36961	                                                return(1);
36962	                                        reginput = save;
36963	                                        scan = regnext(scan);
36964	                                } while (scan != (char *)NULL && OP(scan) == BRANCH);
.Op 367 src/lib/other/regexp.c
36965	                                return(0);
36966	                                /* NOTREACHED */
36967	                        }
36968	                }
36969	                break;
36970	            case STAR:
36971	            case PLUS:{
36972	                        register char nextch;
36973	                        register int no;
36974	                        register char *save;
36975	                        register int min;
36976	
36977	                        /* Lookahead to avoid useless match attempts
36978	                         * when we know what character comes next. */
36979	                        nextch = '\0';
36980	                        if (OP(next) == EXACTLY) nextch = *OPERAND(next);
36981	                        min = (OP(scan) == STAR) ? 0 : 1;
36982	                        save = reginput;
36983	                        no = regrepeat(OPERAND(scan));
36984	                        while (no >= min) {
36985	                                /* If it could work, try it. */
36986	                                if (nextch == '\0' || *reginput == nextch)
36987	                                        if (regmatch(next)) return(1);
36988	                                /* Couldn't or didn't -- back up. */
36989	                                no--;
36990	                                reginput = save + no;
36991	                        }
36992	                        return(0);
36993	                }
36994	                break;
36995	            case END:
36996	                return(1);      /* Success! */
36997	                break;
36998	            default:
36999	                regerror("memory corruption");
37000	                return(0);
37001	                break;
37002	        }
37003	
37004	        scan = next;
37005	  }
37006	
37007	  /* We get here only if there's trouble -- normally "case END" is the
37008	   * terminating point. */
37009	  regerror("corrupted pointers");
37010	  return(0);
37011	}
	
37013	/*
37014	 - regrepeat - repeatedly match something simple, report how many
37015	 */
37016	PRIVATE int regrepeat(p)
37017	char *p;
37018	{
37019	  register int count = 0;
37020	  register char *scan;
37021	  register char *opnd;
37022	
37023	  scan = reginput;
37024	  opnd = OPERAND(p);
.Ep 368 src/lib/other/regexp.c
37025	  switch (OP(p)) {
37026	      case ANY:
37027	        count = strlen(scan);
37028	        scan += count;
37029	        break;
37030	      case EXACTLY:
37031	        while (*opnd == *scan) {
37032	                count++;
37033	                scan++;
37034	        }
37035	        break;
37036	      case ANYOF:
37037	        while (*scan != '\0' && strchr(opnd, *scan) != (char *)NULL) {
37038	                count++;
37039	                scan++;
37040	        }
37041	        break;
37042	      case ANYBUT:
37043	        while (*scan != '\0' && strchr(opnd, *scan) == (char *)NULL) {
37044	                count++;
37045	                scan++;
37046	        }
37047	        break;
37048	      default:                  /* Oh dear.  Called inappropriately. */
37049	        regerror("internal foulup");
37050	        count = 0;              /* Best compromise. */
37051	        break;
37052	  }
37053	  reginput = scan;
37054	
37055	  return(count);
37056	}
	
37058	/*
37059	 - regnext - dig the "next" pointer out of a node
37060	 */
37061	PRIVATE char *regnext(p)
37062	register char *p;
37063	{
37064	  register int offset;
37065	
37066	  if (p == &regdummy) return((char *)NULL);
37067	
37068	  offset = NEXT(p);
37069	  if (offset == 0) return((char *)NULL);
37070	
37071	  if (OP(p) == BACK)
37072	        return(p - offset);
37073	  else
37074	        return(p + offset);
37075	}
	
37077	#ifdef DEBUG
37078	
37079	STATIC char *regprop();
37080	
37081	/*
37082	 - regdump - dump a regexp onto stdout in vaguely comprehensible form
37083	 */
37084	void regdump(r)
.Op 369 src/lib/other/regexp.c
37085	regexp *r;
37086	{
37087	  register char *s;
37088	  register char op = EXACTLY;   /* Arbitrary non-END op. */
37089	  register char *next;
37090	
37091	  s = r->program + 1;
37092	  while (op != END) {           /* While that wasn't END last time... */
37093	        op = OP(s);
37094	        printf("%2d%s", (int) (s - r->program), regprop(s));    /* Where, what. */
37095	        next = regnext(s);
37096	        if (next == (char *)NULL)       /* Next ptr. */
37097	                printf("(0)");
37098	        else
37099	                printf("(%d)", (int) (s - r->program) + (int) (next - s));
37100	        s += 3;
37101	        if (op == ANYOF || op == ANYBUT || op == EXACTLY) {
37102	                /* Literal string, where present. */
37103	                while (*s != '\0') {
37104	                        putchar(*s);
37105	                        s++;
37106	                }
37107	                s++;
37108	        }
37109	        putchar('\n');
37110	  }
37111	
37112	  /* Header fields of interest. */
37113	  if (r->regstart != '\0') printf("start `%c' ", r->regstart);
37114	  if (r->reganch) printf("anchored ");
37115	  if (r->regmust != (char *)NULL) printf("must have \"%s\"", r->regmust);
37116	  printf("\n");
37117	}
	
37119	/*
37120	 - regprop - printable representation of opcode
37121	 */
37122	PRIVATE char *regprop(op)
37123	char *op;
37124	{
37125	  register char *p;
37126	  PRIVATE char buf[50];
37127	
37128	  (void) strcpy(buf, ":");
37129	
37130	  switch (OP(op)) {
37131	      case BOL: p = "BOL";              break;
37132	      case EOL: p = "EOL";              break;
37133	      case ANY: p = "ANY";              break;
37134	      case ANYOF:       p = "ANYOF";            break;
37135	      case ANYBUT:      p = "ANYBUT";           break;
37136	      case BRANCH:      p = "BRANCH";           break;
37137	      case EXACTLY:     p = "EXACTLY";          break;
37138	      case NOTHING:     p = "NOTHING";          break;
37139	      case BACK:        p = "BACK";             break;
37140	      case END: p = "END";              break;
37141	      case OPEN + 1:
37142	      case OPEN + 2:
37143	      case OPEN + 3:
37144	      case OPEN + 4:
.Ep 370 src/lib/other/regexp.c
37145	      case OPEN + 5:
37146	      case OPEN + 6:
37147	      case OPEN + 7:
37148	      case OPEN + 8:
37149	      case OPEN + 9:
37150	        sprintf(buf + strlen(buf), "OPEN%d", OP(op) - OPEN);
37151	        p = (char *)NULL;
37152	        break;
37153	      case CLOSE + 1:
37154	      case CLOSE + 2:
37155	      case CLOSE + 3:
37156	      case CLOSE + 4:
37157	      case CLOSE + 5:
37158	      case CLOSE + 6:
37159	      case CLOSE + 7:
37160	      case CLOSE + 8:
37161	      case CLOSE + 9:
37162	        sprintf(buf + strlen(buf), "CLOSE%d", OP(op) - CLOSE);
37163	        p = (char *)NULL;
37164	        break;
37165	      case STAR:        p = "STAR";             break;
37166	      case PLUS:        p = "PLUS";             break;
37167	      default:  regerror("corrupted opcode"); p = (char *) NULL; break;
37168	  }
37169	  if (p != (char *)NULL) (void) strcat(buf, p);
37170	  return(buf);
37171	}
	
37173	#endif


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/regsub.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

37200	/* regsub
37201	 *
37202	 *      Copyright (c) 1986 by University of Toronto.
37203	 *      Written by Henry Spencer.  Not derived from licensed software.
37204	 *
37205	 *      Permission is granted to anyone to use this software for any
37206	 *      purpose on any computer system, and to redistribute it freely,
37207	 *      subject to the following restrictions:
37208	 *
37209	 *      1. The author is not responsible for the consequences of use of
37210	 *              this software, no matter how awful, even if they arise
37211	 *              from defects in it.
37212	 *
37213	 *      2. The origin of this software must not be misrepresented, either
37214	 *              by explicit claim or by omission.
37215	 *
37216	 *      3. Altered versions must be plainly marked as such, and must not
37217	 *              be misrepresented as being the original software.
37218	 */
37219	
37220	#include <lib.h>
37221	#include <string.h>
37222	#include <regexp.h>
37223	#include <stdio.h>
37224	
.Op 371 src/lib/other/regsub.c
37225	/* The first byte of the regexp internal "program" is actually this magic
37226	 * number; the start node begins in the second byte.
37227	 */
37228	#define MAGIC   0234
37229	
37230	#define CHARBITS 0377
37231	#ifndef CHARBITS
37232	#define UCHARAT(p)      ((int)*(unsigned char *)(p))
37233	#else
37234	#define UCHARAT(p)      ((int)*(p)&CHARBITS)
37235	#endif
37236	
37237	/*
37238	 - regsub - perform substitutions after a regexp match
37239	 */
37240	void regsub(prog, source, dest)
37241	regexp *prog;
37242	char *source;
37243	char *dest;
37244	{
37245	  register char *src;
37246	  register char *dst;
37247	  register char c;
37248	  register int no;
37249	  register int len;
37250	
37251	  if (prog == (regexp *)NULL || source == (char *)NULL || dest == (char *)NULL) {
37252	        regerror("NULL parm to regsub");
37253	        return;
37254	  }
37255	  if (UCHARAT(prog->program) != MAGIC) {
37256	        regerror("damaged regexp fed to regsub");
37257	        return;
37258	  }
37259	  src = source;
37260	  dst = dest;
37261	  while ((c = *src++) != '\0') {
37262	        if (c == '&')
37263	                no = 0;
37264	        else if (c == '\\' && '0' <= *src && *src <= '9')
37265	                no = *src++ - '0';
37266	        else
37267	                no = -1;
37268	
37269	        if (no < 0) {           /* Ordinary character. */
37270	                if (c == '\\' && (*src == '\\' || *src == '&')) c = *src++;
37271	                *dst++ = c;
37272	        } else
37273	        if (prog->startp[no] != (char *)NULL && prog->endp[no] != (char *)NULL) {
37274	                len = (int) (prog->endp[no] - prog->startp[no]);
37275	                strncpy(dst, prog->startp[no], len);
37276	                dst += len;
37277	                if (len != 0 && *(dst - 1) == '\0') {   /* strncpy hit NUL. */
37278	                        regerror("damaged match string");
37279	                        return;
37280	                }
37281	        }
37282	  }
37283	  *dst++ = '\0';
37284	}
.Ep 372 src/lib/other/rindex.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/rindex.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

37300	#include <lib.h>
37301	/* rindex - find last occurrence of a character in a string  */
37302	
37303	#include <string.h>
37304	
37305	char *rindex(s, charwanted)     /* found char, or NULL if none */
37306	_CONST char *s;
37307	char charwanted;
37308	{
37309	  return(strrchr(s, charwanted));
37310	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/stderr.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

37400	#include <lib.h>
37401	#include <sys/types.h>
37402	#include <unistd.h>
37403	
37404	_PROTOTYPE( void std_err, (char *s));
37405	
37406	void std_err(s)
37407	char *s;
37408	{
37409	  register char *p = s;
37410	
37411	  while (*p != 0) p++;
37412	  write(2, s, (int) (p - s));
37413	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/swab.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

37500	#include <lib.h>
37501	/*  swab(3)
37502	 *
37503	 *  Author: Terrence W. Holm          Sep. 1988
37504	 */
37505	_PROTOTYPE( void swab, (char *from, char *to, int count));
37506	
37507	void swab(from, to, count)
37508	char *from;
37509	char *to;
37510	int count;
37511	{
37512	  register char temp;
37513	
37514	  count >>= 1;
.Op 373 src/lib/other/swab.c
37515	
37516	  while (--count >= 0) {
37517	        temp = *from++;
37518	        *to++ = *from++;
37519	        *to++ = temp;
37520	  }
37521	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/syscall.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

37600	#include <lib.h>
37601	
37602	PUBLIC int _syscall(who, syscallnr, msgptr)
37603	int who;
37604	int syscallnr;
37605	register message *msgptr;
37606	{
37607	  int status;
37608	
37609	  msgptr->m_type = syscallnr;
37610	  status = _sendrec(who, msgptr);
37611	  if (status != 0) {
37612	        /* 'sendrec' itself failed. */
37613	        /* XXX - strerror doesn't know all the codes */
37614	        msgptr->m_type = status;
37615	  }
37616	  if (msgptr->m_type < 0) {
37617	        errno = -msgptr->m_type;
37618	        return(-1);
37619	  }
37620	  return(msgptr->m_type);
37621	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/sysconf.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

37700	/* sysconf.c                                            POSIX 4.8.1
37701	 *      long int sysconf(int name);
37702	 *
37703	 *      POSIX allows some of the values in <limits.h> to be increased at
37704	 *      run time.  The sysconf() function allows such values to be checked
37705	 *      at run time.  MINIX does not use this facility - the run time
37706	 *      limits are those given in <limits.h>.
37707	 */
37708	
37709	#include <lib.h>
37710	#include <unistd.h>
37711	#include <time.h>
37712	
37713	PUBLIC long int sysconf(name)
37714	int name;                       /* property being inspected */
.Ep 374 src/lib/other/sysconf.c
37715	{
37716	  switch(name) {
37717	        case _SC_ARG_MAX:
37718	                return (long) ARG_MAX;
37719	
37720	        case _SC_CHILD_MAX:
37721	                return (long) CHILD_MAX;
37722	
37723	        case _SC_CLK_TCK:
37724	                return (long) CLOCKS_PER_SEC;
37725	
37726	        case _SC_NGROUPS_MAX:
37727	                return (long) NGROUPS_MAX;
37728	
37729	        case _SC_OPEN_MAX:
37730	                return (long) OPEN_MAX;
37731	
37732	        case _SC_JOB_CONTROL:
37733	                return -1L;                     /* no job control */
37734	
37735	        case _SC_SAVED_IDS:
37736	                return -1L;                     /* no saved uid/gid */
37737	
37738	        case _SC_VERSION:
37739	                return (long) _POSIX_VERSION;
37740	
37741	        case _SC_STREAM_MAX:
37742	                return (long) STREAM_MAX;
37743	
37744	        case _SC_TZNAME_MAX:
37745	                return (long) TZNAME_MAX;
37746	
37747	        default:
37748	                errno = EINVAL;
37749	                return -1L;
37750	  }
37751	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/telldir.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

37800	/*      telldir()                                       Author: Kees J. Bot
37801	 *                                                              24 Apr 1989
37802	 */
37803	#define nil 0
37804	#include <lib.h>
37805	#include <sys/types.h>
37806	#include <dirent.h>
37807	#include <errno.h>
37808	
37809	off_t telldir(DIR *dp)
37810	/* Return the current read position in a directory. */
37811	{
37812	        if (dp == nil) { errno= EBADF; return -1; }
37813	
37814	        return dp->_pos;
.Op 375 src/lib/other/telldir.c
37815	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/termcap.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

37900	/*
37901	 *      termcap.c       V1.1    20/7/87         agc     Joypace Ltd
37902	 *
37903	 *      Copyright Joypace Ltd, London, UK, 1987. All rights reserved.
37904	 *      This file may be freely distributed provided that this notice
37905	 *      remains attached.
37906	 *
37907	 *      A public domain implementation of the termcap(3) routines.
37908	 *
37909	 *
37910	 *
37911	 *       Klamer Schutte       V1.2    Nov. 1988
37912	 *
37913	 *   - Can match multiple terminal names                 [tgetent]
37914	 *   - Removal of **area assignments                     [tgetstr]
37915	 *
37916	 *       Terrence W. Holm     V1.3    May, Sep, Oct.  1988
37917	 *
37918	 *   - Correct when TERM != name and TERMCAP is defined  [tgetent]
37919	 *   - Correct the comparison for the terminal name      [tgetent]
37920	 *   - Correct the value of ^x escapes                   [tgetstr]
37921	 *   - Added %r to reverse row/column                    [tgoto]
37922	 *   - Fixed end of definition test                      [tgetnum/flag/str]
37923	 *
37924	 *       Terrence W. Holm     V1.4    Jan. 1989
37925	 *
37926	 *   - Incorporated Klamer's V1.2 fixes into V1.3
37927	 *   - Added %d, (old %d is now %2)                      [tgoto]
37928	 *   - Allow '#' comments in definition file             [tgetent]
37929	 */
37930	
37931	#include <lib.h>
37932	#include <termcap.h>
37933	#include <ctype.h>
37934	#include <stdlib.h>
37935	#include <string.h>
37936	#include <stdio.h>
37937	
37938	char *capab = (char *)NULL;             /* the capability itself */
37939	
37940	#if 0
37941	/*  The following are not yet used.  */
37942	extern short ospeed;            /* output speed */
37943	extern char PC;                 /* padding character */
37944	extern char *BC;                /* back cursor movement */
37945	extern char *UP;                /* up cursor movement */
37946	#endif
37947	
37948	/*
37949	 *      tgetent - get the termcap entry for terminal name, and put it
.Ep 376 src/lib/other/termcap.c
37950	 *      in bp (which must be an array of 1024 chars). Returns 1 if
37951	 *      termcap entry found, 0 if not found, and -1 if file not found.
37952	 */
37953	
37954	int tgetent(bp, name)
37955	char *bp;
37956	char *name;
37957	{
37958	  FILE *fp;
37959	  char *file;
37960	  char *term;
37961	  short len = strlen(name);
37962	
37963	  capab = bp;
37964	
37965	  /* If TERMCAP begins with a '/' then use TERMCAP as the path   */
37966	  /* Name of the termcap definitions file. If TERMCAP is a       */
37967	  /* Definition and TERM equals "name" then use TERMCAP as the   */
37968	  /* Definition. Otherwise use "/etc/termcap" as the path name.  */
37969	
37970	  if ((file = getenv("TERMCAP")) == (char *)NULL)
37971	        file = "/etc/termcap";
37972	  else if (*file != '/')
37973	        if ((term = getenv("TERM")) != (char *)NULL && strcmp(term, name) == 0) {
37974	                *bp = '\0';
37975	                strncat(bp, file, 1023);
37976	                return(1);
37977	        } else
37978	                file = "/etc/termcap";
37979	
37980	  if ((fp = fopen(file, "r")) == (FILE *) NULL) {
37981	        capab = (char *)NULL;           /* no valid termcap  */
37982	        return(-1);
37983	  }
37984	  for (;;) {
37985	        /* Read in each definition */
37986	        int def_len = 0;
37987	        char *cp = bp;
37988	
37989	        do {
37990	                if (fgets(&bp[def_len], (unsigned int)(1024 - def_len), fp) == (char *)NULL) {
37991	                        fclose(fp);
37992	                        capab = (char *)NULL;   /* no valid termcap */
37993	                        return(0);
37994	                }
37995	                def_len = strlen(bp) - 2;
37996	        } while (bp[def_len] == '\\');
37997	
37998	        while (isspace(*cp)) cp++;
37999	
38000	        /* Comment lines start with a '#'  */
38001	        if (*cp == '#') continue;
38002	
38003	        /* See if any of the terminal names in this definition */
38004	        /* Match "name".                                                 */
38005	
38006	        do {
38007	                if (strncmp(name, cp, len) == 0 &&
38008	                    (cp[len] == '|' || cp[len] == ':')) {
38009	                        fclose(fp);
.Op 377 src/lib/other/termcap.c
38010	                        return(1);
38011	                }
38012	                while ((*cp) && (*cp != '|') && (*cp != ':')) cp++;
38013	        } while (*cp++ == '|');
38014	  }
38015	}
	
	
38018	/*
38019	 *      tgetnum - get the numeric terminal capability corresponding
38020	 *      to id. Returns the value, -1 if invalid.
38021	 */
38022	
38023	int tgetnum(id)
38024	char *id;
38025	{
38026	  register char *cp = capab;
38027	
38028	  if (cp == (char *)NULL || id == (char *)NULL) return(-1);
38029	
38030	  for (;;) {
38031	        while (*cp++ != ':')
38032	                if (cp[-1] == '\0') return(-1);
38033	
38034	        while (isspace(*cp)) cp++;
38035	
38036	        if (strncmp(cp, id, 2) == 0 && cp[2] == '#') return(atoi(cp + 3));
38037	  }
38038	}
	
	
38041	/*
38042	 *      tgetflag - get the boolean flag corresponding to id. Returns -1
38043	 *      if invalid, 0 if the flag is not in termcap entry, or 1 if it is
38044	 *      present.
38045	 */
38046	
38047	int tgetflag(id)
38048	char *id;
38049	{
38050	  register char *cp = capab;
38051	
38052	  if (cp == (char *)NULL || id == (char *)NULL) return(-1);
38053	
38054	  for (;;) {
38055	        while (*cp++ != ':')
38056	                if (cp[-1] == '\0') return(0);
38057	
38058	        while (isspace(*cp)) cp++;
38059	
38060	        if (strncmp(cp, id, 2) == 0) return(1);
38061	  }
38062	}
	
	
38065	/*
38066	 *      tgetstr - get the string capability corresponding to id and place
38067	 *      it in area (advancing area at same time). Expand escape sequences
38068	 *      etc. Returns the string, or NULL if it can't do it.
38069	 */
.Ep 378 src/lib/other/termcap.c
38070	
38071	char *tgetstr(id, area)
38072	char *id;
38073	char **area;
38074	{
38075	  register char *cp = capab;
38076	  register char *wsp = *area;   /* workspace pointer  */
38077	
38078	  if (cp == (char *)NULL || id == (char *)NULL) return((char *)NULL);
38079	
38080	  for (;;) {
38081	        while (*cp++ != ':')
38082	                if (cp[-1] == '\0') return((char *)NULL);
38083	
38084	        while (isspace(*cp)) cp++;
38085	
38086	        if (strncmp(cp, id, 2) == 0 && cp[2] == '=') {
38087	                for (cp += 3; *cp && *cp != ':'; wsp++, cp++) switch (*cp) {
38088	                            case '^':
38089	                                *wsp = *++cp - '@';
38090	                                break;
38091	
38092	                            case '\\':
38093	                                switch (*++cp) {
38094	                                    case 'E':
38095	                                        *wsp = '\033';
38096	                                        break;
38097	                                    case 'n':
38098	                                        *wsp = '\n';
38099	                                        break;
38100	                                    case 'r':
38101	                                        *wsp = '\r';
38102	                                        break;
38103	                                    case 't':
38104	                                        *wsp = '\t';
38105	                                        break;
38106	                                    case 'b':
38107	                                        *wsp = '\b';
38108	                                        break;
38109	                                    case 'f':
38110	                                        *wsp = '\f';
38111	                                        break;
38112	                                    case '0':
38113	                                    case '1':
38114	                                    case '2':
38115	                                    case '3':
38116	                                        {
38117	                                                int i;
38118	                                                int t = 0;
38119	                                                for (i = 0; i < 3 &&
38120	                                                     isdigit(*cp); ++i, ++cp)
38121	                                                        t = t * 8 + *cp - '0';
38122	                                                *wsp = t;
38123	                                                cp--;
38124	                                                break;
38125	                                        }
38126	                                    default:
38127	                                        *wsp = *cp;
38128	                                }
38129	                                break;
.Op 379 src/lib/other/termcap.c
38130	
38131	                            default:    *wsp = *cp;
38132	                        }
38133	
38134	                *wsp++ = '\0';
38135	
38136	                {
38137	                        char *ret = *area;
38138	                        *area = wsp;
38139	                        return(ret);
38140	                }
38141	        }
38142	  }                             /* end for(;;) */
38143	}
	
	
	
38147	/*
38148	 *      tgoto - given the cursor motion string cm, make up the string
38149	 *      for the cursor to go to (destcol, destline), and return the string.
38150	 *      Returns "OOPS" if something's gone wrong, or the string otherwise.
38151	 */
38152	
38153	char *tgoto(cm, destcol, destline)
38154	char *cm;
38155	int destcol;
38156	int destline;
38157	{
38158	  PRIVATE char ret[24];
38159	  char *rp = ret;
38160	  int incr = 0;
38161	  int argno = 0;
38162	  int numval;
38163	
38164	  for (; *cm; cm++) {
38165	        if (*cm == '%') {
38166	                switch (*++cm) {
38167	                    case 'i':   incr = 1;                               break;
38168	
38169	                    case 'r':   argno = 1;                              break;
38170	
38171	                    case '+':
38172	                        numval = (argno == 0 ? destline : destcol);
38173	                        *rp++ = numval + incr + *++cm;
38174	                        argno = 1 - argno;
38175	                        break;
38176	
38177	                    case '2':
38178	                        numval = (argno == 0 ? destline : destcol);
38179	                        numval = (numval + incr) % 100;
38180	                        *rp++ = '0' + (numval / 10);
38181	                        *rp++ = '0' + (numval % 10);
38182	                        argno = 1 - argno;
38183	                        break;
38184	
38185	                    case 'd':
38186	                        numval = (argno == 0 ? destline : destcol);
38187	                        numval = (numval + incr) % 1000;
38188	                        if (numval > 99) *rp++ = '0' + (numval / 100);
38189	                        if (numval > 9) *rp++ = '0' + (numval / 10) % 10;
.Ep 380 src/lib/other/termcap.c
38190	                        *rp++ = '0' + (numval % 10);
38191	                        argno = 1 - argno;
38192	                        break;
38193	
38194	                    case '%':   *rp++ = '%';                            break;
38195	
38196	                    default:    return("OOPS");
38197	                }
38198	
38199	        } else
38200	                *rp++ = *cm;
38201	  }
38202	
38203	  *rp = '\0';
38204	  return(ret);
38205	}
	
	
	
38209	/*
38210	 *      tputs - put the string cp out onto the terminal, using the function
38211	 *      outc. This should do padding for the terminal, but I can't find a
38212	 *      terminal that needs padding at the moment...
38213	 */
38214	
38215	int tputs(cp, affcnt, outc)
38216	register char *cp;
38217	int affcnt;
38218	_PROTOTYPE( void (*outc), (int ch));
38219	{
38220	  if (cp == (char *)NULL) return(1);
38221	  /* Do any padding interpretation - left null for MINIX just now */
38222	  while (*cp) (*outc) (*cp++);
38223	  return(1);
38224	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/ttyname.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

38300	/* ttyname.c                                            POSIX 4.7.2
38301	 *      char *ttyname(int fildes);
38302	 *
38303	 *      Determines name of a terminal device.
38304	 */
38305	
38306	#include <lib.h>
38307	#include <sys/stat.h>
38308	#include <dirent.h>
38309	#include <fcntl.h>
38310	#include <stddef.h>
38311	#include <string.h>
38312	#include <unistd.h>
38313	
38314	PRIVATE char base[] = "/dev";
.Op 381 src/lib/other/ttyname.c
38315	PRIVATE char path[sizeof(base) + 1 + NAME_MAX]; /* extra 1 for '/' */
38316	
38317	PUBLIC char *ttyname(fildes)
38318	int fildes;
38319	{
38320	  DIR *devices;
38321	  struct dirent *entry;
38322	  struct stat tty_stat;
38323	  struct stat dev_stat;
38324	
38325	  /* Simple first test: file descriptor must be a character device */
38326	  if (fstat(fildes, &tty_stat) < 0 || !S_ISCHR(tty_stat.st_mode))
38327	        return (char *) NULL;
38328	
38329	  /* Open device directory for reading  */
38330	  if ((devices = opendir(base)) == (DIR *) NULL)
38331	        return (char *) NULL;
38332	
38333	  /* Scan the entries for one that matches perfectly */
38334	  while ((entry = readdir(devices)) != (struct dirent *) NULL) {
38335	        if (tty_stat.st_ino != entry->d_ino)
38336	                continue;
38337	        strcpy(path, base);
38338	        strcat(path, "/");
38339	        strcat(path, entry->d_name);
38340	        if (stat(path, &dev_stat) < 0 || !S_ISCHR(dev_stat.st_mode))
38341	                continue;
38342	        if (tty_stat.st_ino == dev_stat.st_ino &&
38343	            tty_stat.st_dev == dev_stat.st_dev &&
38344	            tty_stat.st_rdev == dev_stat.st_rdev) {
38345	                closedir(devices);
38346	                return path;
38347	        }
38348	  }
38349	
38350	  closedir(devices);
38351	  return (char *) NULL;
38352	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/other/ttyslot.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

38400	/*
38401	ttyslot.c
38402	
38403	Return the index in the utmp file for the current user's terminal. The 
38404	current user's terminal is the first file descriptor in the range 0..2
38405	for which ttyname() returns a name. The index is the line number in the
38406	/etc/ttytab file. 0 will be returned in case of an error.
38407	
38408	Created:        Oct 11, 1992 by Philip Homburg
38409	*/
38410	
38411	#define _MINIX_SOURCE
38412	
38413	#include <sys/types.h>
38414	#include <ttyent.h>
.Ep 382 src/lib/other/ttyslot.c
38415	#include <string.h>
38416	#include <unistd.h>
38417	
38418	int ttyslot()
38419	{
38420	        int slot;
38421	
38422	        slot= fttyslot(0);
38423	        if (slot == 0) slot= fttyslot(1);
38424	        if (slot == 0) slot= fttyslot(2);
38425	        return slot;
38426	}
	
38428	int fttyslot(fd)
38429	int fd;
38430	{
38431	        char *tname;
38432	        int lineno;
38433	        struct ttyent *ttyp;
38434	
38435	        tname= ttyname(fd);
38436	        if (tname == NULL) return 0;
38437	
38438	        /* Assume that tty devices are in /dev */
38439	        if (strncmp(tname, "/dev/", 5) != 0)
38440	                return 0;       /* Malformed tty name. */
38441	        tname += 5;
38442	
38443	        /* Scan /etc/ttytab. */
38444	        lineno= 1;
38445	        while ((ttyp= getttyent()) != NULL)
38446	        {
38447	                if (strcmp(tname, ttyp->ty_name) == 0)
38448	                {
38449	                        endttyent();
38450	                        return lineno;
38451	                }
38452	                lineno++;
38453	        }
38454	        /* No match */
38455	        endttyent();
38456	        return 0;
38457	}
	
38459	/*
38460	 * $PchHeader: /mount/hd2/minix/lib/misc/RCS/ttyslot.c,v 1.3 1994/12/22 13:49:12 philip Exp $
38461	 */
.Op 383 src/lib/posix/__exit.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/__exit.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

38500	#define _exit   __exit
38501	#include <lib.h>
38502	#include <unistd.h>
38503	
38504	PUBLIC void _exit(status)
38505	int status;
38506	{
38507	  message m;
38508	
38509	  m.m1_i1 = status;
38510	  _syscall(MM, EXIT, &m);
38511	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_access.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

38600	#include <lib.h>
38601	#define access  _access
38602	#include <unistd.h>
38603	
38604	PUBLIC int access(name, mode)
38605	_CONST char *name;
38606	int mode;
38607	{
38608	  message m;
38609	
38610	  m.m3_i2 = mode;
38611	  _loadname(name, &m);
38612	  return(_syscall(FS, ACCESS, &m));
38613	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_alarm.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

38700	#include <lib.h>
38701	#define alarm   _alarm
38702	#include <unistd.h>
38703	
38704	PUBLIC unsigned int alarm(sec)
38705	unsigned int sec;
38706	{
38707	  message m;
38708	
38709	  m.m1_i1 = (int) sec;
38710	  return( (unsigned) _syscall(MM, ALARM, &m));
38711	}
.Ep 384 src/lib/posix/_cfgetispeed.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_cfgetispeed.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

38800	/*
38801	posix/_cfgetispeed
38802	
38803	Created:        June 11, 1993 by Philip Homburg
38804	*/
38805	
38806	#include <termios.h>
38807	
38808	speed_t _cfgetispeed(const struct termios *termios_p)
38809	{
38810	  return termios_p->c_ispeed;
38811	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_cfgetospeed.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

38900	/*
38901	posix/_cfgetospeed
38902	
38903	Created:        June 11, 1993 by Philip Homburg
38904	*/
38905	
38906	#include <termios.h>
38907	
38908	speed_t _cfgetospeed(const struct termios *termios_p)
38909	{
38910	  return termios_p->c_ospeed;
38911	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_cfsetispeed.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

39000	/*
39001	posix/_cfsetispeed
39002	
39003	Created:        June 11, 1993 by Philip Homburg
39004	*/
39005	
39006	#include <termios.h>
39007	
39008	int _cfsetispeed(struct termios *termios_p, speed_t speed)
39009	{
39010	  termios_p->c_ispeed= speed;
39011	  return 0;
39012	}
.Op 385 src/lib/posix/_cfsetospeed.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_cfsetospeed.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

39100	/*
39101	posix/_cfsetospeed
39102	
39103	Created:        June 11, 1993 by Philip Homburg
39104	*/
39105	
39106	#include <termios.h>
39107	
39108	int _cfsetospeed(struct termios *termios_p, speed_t speed)
39109	{
39110	  termios_p->c_ospeed= speed;
39111	  return 0;
39112	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_chdir.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

39200	#include <lib.h>
39201	#define chdir   _chdir
39202	#include <unistd.h>
39203	
39204	PUBLIC int chdir(name)
39205	_CONST char *name;
39206	{
39207	  message m;
39208	
39209	  _loadname(name, &m);
39210	  return(_syscall(FS, CHDIR, &m));
39211	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_chmod.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

39300	#include <lib.h>
39301	#define chmod   _chmod
39302	#include <sys/stat.h>
39303	
39304	PUBLIC int chmod(name, mode)
39305	_CONST char *name;
39306	Mode_t mode;
39307	{
39308	  message m;
39309	
39310	  m.m3_i2 = mode;
39311	  _loadname(name, &m);
39312	  return(_syscall(FS, CHMOD, &m));
39313	}
.Ep 386 src/lib/posix/_chown.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_chown.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

39400	#include <lib.h>
39401	#define chown   _chown
39402	#include <string.h>
39403	#include <unistd.h>
39404	
39405	PUBLIC int chown(name, owner, grp)
39406	_CONST char *name;
39407	Uid_t owner;
39408	Gid_t grp;
39409	{
39410	  message m;
39411	
39412	  m.m1_i1 = strlen(name) + 1;
39413	  m.m1_i2 = owner;
39414	  m.m1_i3 = grp;
39415	  m.m1_p1 = (char *) name;
39416	  return(_syscall(FS, CHOWN, &m));
39417	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_chroot.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

39500	#include <lib.h>
39501	#define chroot  _chroot
39502	#include <unistd.h>
39503	
39504	PUBLIC int chroot(name)
39505	_CONST char *name;
39506	{
39507	  message m;
39508	
39509	  _loadname(name, &m);
39510	  return(_syscall(FS, CHROOT, &m));
39511	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_close.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

39600	#include <lib.h>
39601	#define close   _close
39602	#include <unistd.h>
39603	
39604	PUBLIC int close(fd)
39605	int fd;
39606	{
39607	  message m;
39608	
39609	  m.m1_i1 = fd;
.Op 387 src/lib/posix/_close.c
39610	  return(_syscall(FS, CLOSE, &m));
39611	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_closedir.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

39700	/*      closedir()                                      Author: Kees J. Bot
39701	 *                                                              24 Apr 1989
39702	 */
39703	#define nil 0
39704	#include <lib.h>
39705	#define close   _close
39706	#define closedir _closedir
39707	#include <sys/types.h>
39708	#include <dirent.h>
39709	#include <unistd.h>
39710	#include <stdlib.h>
39711	#include <errno.h>
39712	
39713	int closedir(DIR *dp)
39714	/* Finish reading a directory. */
39715	{
39716	        int d;
39717	
39718	        if (dp == nil) { errno= EBADF; return -1; }
39719	
39720	        d= dp->_fd;
39721	        free((void *) dp);
39722	        return close(d);
39723	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_creat.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

39800	#include <lib.h>
39801	#define creat   _creat
39802	#include <fcntl.h>
39803	
39804	PUBLIC int creat(name, mode)
39805	_CONST char *name;
39806	Mode_t mode;
39807	{
39808	  message m;
39809	
39810	  m.m3_i2 = mode;
39811	  _loadname(name, &m);
39812	  return(_syscall(FS, CREAT, &m));
39813	}
.Ep 388 src/lib/posix/_dup.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_dup.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

39900	#include <lib.h>
39901	#define dup     _dup
39902	#define fcntl   _fcntl
39903	#include <fcntl.h>
39904	#include <unistd.h>
39905	
39906	PUBLIC int dup(fd)
39907	int fd;
39908	{
39909	  return(fcntl(fd, F_DUPFD, 0));
39910	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_dup2.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

40000	#include <lib.h>
40001	#define close   _close
40002	#define dup2    _dup2
40003	#define fcntl   _fcntl
40004	#include <fcntl.h>
40005	#include <unistd.h>
40006	
40007	PUBLIC int dup2(fd, fd2)
40008	int fd, fd2;
40009	{
40010	/* The behavior of dup2 is defined by POSIX in 6.2.1.2 as almost, but not
40011	 * quite the same as fcntl.
40012	 */
40013	
40014	  if (fd2 < 0 || fd2 > OPEN_MAX) {
40015	        errno = EBADF;
40016	        return(-1);
40017	  }
40018	
40019	  /* Check to see if fildes is valid. */
40020	  if (fcntl(fd, F_GETFL) < 0) {
40021	        /* 'fd' is not valid. */
40022	        return(-1);
40023	  } else {
40024	        /* 'fd' is valid. */
40025	        if (fd == fd2) return(fd2);
40026	        close(fd2);
40027	        return(fcntl(fd, F_DUPFD, fd2));
40028	  }
40029	}
.Op 389 src/lib/posix/_exec.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_exec.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

40100	#include <lib.h>
40101	#define execl   _execl
40102	#define execle  _execle
40103	#define execv   _execv
40104	#define execve  _execve
40105	#define sbrk    _sbrk
40106	#include <minix/minlib.h>
40107	#include <stdarg.h>
40108	#include <string.h>
40109	#include <unistd.h>
40110	
40111	extern char **environ;
40112	
40113	#define PTRSIZE (sizeof(char *))
40114	
40115	#ifdef _ANSI
40116	PUBLIC int execl(const char *name, const char *arg, ...)
40117	#else
40118	PUBLIC int execl(name)
40119	char *name;
40120	#endif
40121	{
40122	  va_list argp;
40123	  int result;
40124	
40125	  va_start(argp, name);
40126	
40127	  /* The following cast of argp is not portable.  Doing it right by copying
40128	   * the args to a true array will cost as much as ARG_MAX bytes of space.
40129	   */
40130	  result = execve(name, (char **) argp, environ);
40131	  va_end(argp);
40132	  return(result);
40133	}
	
	
40136	#ifdef _ANSI
40137	PUBLIC int execle(const char *name, const char *arg, ...)
40138	#else
40139	PUBLIC int execle(name)
40140	char *name;
40141	#endif
40142	{
40143	  va_list argp;
40144	  char **p;
40145	  int result;
40146	
40147	  va_start(argp, name);
40148	  
40149	  /* The following cast of argp is not portable, as for execl(). */
40150	  p = (char **) argp;
40151	  while (*p++ != NIL_PTR)
40152	        ;                       /* null statement */
40153	  result = execve(name, (char **) argp, (char **) *p);
40154	  va_end(argp);
.Ep 390 src/lib/posix/_exec.c
40155	  return(result);
40156	}
	
	
40159	PUBLIC int execv(name, argv)
40160	_CONST char *name;
40161	char * _CONST argv[];
40162	{
40163	  return(execve(name, argv, environ));
40164	}
	
	
40167	PUBLIC int execve(path, argv, envp)
40168	_CONST char *path;              /* pointer to name of file to be executed */
40169	char * _CONST argv[];           /* pointer to argument array */
40170	char * _CONST envp[];           /* pointer to environment */
40171	{
40172	  int i, j;
40173	
40174	  /* Count the argument pointers and environment pointers. */
40175	  i = 0;
40176	  if (argv != NULL)
40177	  {
40178	        while (argv[i] != NULL) i++;
40179	  }
40180	  j = 0;
40181	  if (envp != NULL)
40182	  {
40183	        while (envp[j] != NULL) j++;
40184	  }
40185	
40186	  return(__execve(path, argv, envp, i, j));
40187	}
	
	
40190	PUBLIC int __execve(path, argv, envp, nargs, nenvps)
40191	_CONST char *path;              /* pointer to name of file to be executed */
40192	char * _CONST argv[];           /* pointer to argument array */
40193	char * _CONST envp[];           /* pointer to environment */
40194	int nargs;                      /* number of args */
40195	int nenvps;                     /* number of environment strings */
40196	{
40197	/* This is split off from execve to be called from execvp, so execvp does not
40198	 * have to allocate up to ARG_MAX bytes just to prepend "sh" to the arg array.
40199	 */
40200	
40201	  char *hp, **ap, *p;
40202	  int i, stackbytes, npointers, overflow, temp;
40203	  char *stack;
40204	  message m;
40205	
40206	  /* Decide how big a stack is needed. Be paranoid about overflow. */
40207	  overflow = FALSE;
40208	  npointers = 1 + nargs + 1 + nenvps + 1;       /* 1's for argc and NULLs */
40209	  stackbytes = nargs + nenvps;          /* 1 byte for each null in strings */
40210	  if (nargs < 0 || nenvps < 0 || stackbytes < nargs || npointers < stackbytes)
40211	        overflow = TRUE;
40212	  for (i = PTRSIZE; i != 0; i--) {
40213	        temp = stackbytes + npointers;
40214	        if (temp < stackbytes) overflow = TRUE;
.Op 391 src/lib/posix/_exec.c
40215	        stackbytes = temp;
40216	  }
40217	  ap = (char **) argv;
40218	  for (i = 0; i < nargs; i++) {
40219	        temp = stackbytes + strlen(*ap++);
40220	        if (temp < stackbytes) overflow = TRUE;
40221	        stackbytes = temp;
40222	  }
40223	  ap = (char **) envp;
40224	  for (i = 0; i < nenvps; i++) {
40225	        temp = stackbytes + strlen(*ap++);
40226	        if (temp < stackbytes) overflow = TRUE;
40227	        stackbytes = temp;
40228	  }
40229	  temp = stackbytes + PTRSIZE - 1;
40230	  if (temp < stackbytes) overflow = TRUE;
40231	  stackbytes = (temp / PTRSIZE) * PTRSIZE;
40232	
40233	  /* Check for overflow before committing sbrk. */
40234	  if (overflow) {
40235	        errno = E2BIG;
40236	        return(-1);
40237	  }
40238	
40239	  /* Allocate the stack. */
40240	  stack = sbrk(stackbytes);
40241	  if (stack == (char *) -1) {
40242	        errno = E2BIG;
40243	        return(-1);
40244	  }
40245	
40246	  /* Prepare the stack vector and argc. */
40247	  ap = (char **) stack;
40248	  hp = &stack[npointers * PTRSIZE];
40249	  *ap++ = (char *) nargs;
40250	
40251	  /* Prepare the argument pointers and strings. */
40252	  for (i = 0; i < nargs; i++) {
40253	        *ap++ = (char *) (hp - stack);
40254	        p = *argv++;
40255	        while ( (*hp++ = *p++) != 0)
40256	                ;
40257	  }
40258	  *ap++ = (char *) NULL;
40259	
40260	  /* Prepare the environment pointers and strings. */
40261	  for (i = 0; i < nenvps; i++) {
40262	        *ap++ = (char *) (hp - stack);
40263	        p = *envp++;
40264	        while ( (*hp++ = *p++) != 0)
40265	                ;
40266	  }
40267	  *ap++ = (char *) NULL;
40268	
40269	  /* Do the real work. */
40270	  m.m1_i1 = strlen(path) + 1;
40271	  m.m1_i2 = stackbytes;
40272	  m.m1_p1 = (char *) path;
40273	  m.m1_p2 = stack;
40274	  (void) _syscall(MM, EXEC, &m);
.Ep 392 src/lib/posix/_exec.c
40275	
40276	  /* The exec failed. */
40277	  sbrk(-stackbytes);
40278	  return(m.m_type);
40279	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_execn.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

40300	#include <lib.h>
40301	#include <string.h>
40302	
40303	#define PTRSIZE sizeof(char *)
40304	_PROTOTYPE( int _execn, (char * name));
40305	
40306	PUBLIC int _execn(name)
40307	char *name;                     /* pointer to file to be exec'd */
40308	{
40309	/* Special version used when there are no args and no environment.  This call
40310	 * is principally used by INIT, to avoid having to allocate ARG_MAX.
40311	 */
40312	
40313	  PRIVATE char stack[3 * PTRSIZE];
40314	  message m;
40315	
40316	  m.m1_i1 = strlen(name) + 1;
40317	  m.m1_i2 = sizeof(stack);
40318	  m.m1_p1 = name;
40319	  m.m1_p2 = stack;
40320	  (void) _syscall(MM, EXEC, &m);
40321	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_fcntl.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

40400	#include <lib.h>
40401	#define fcntl _fcntl
40402	#include <fcntl.h>
40403	#include <stdarg.h>
40404	
40405	#if _ANSI
40406	PUBLIC int fcntl(int fd, int cmd, ...)
40407	#else
40408	PUBLIC int fcntl(fd, cmd)
40409	int fd;
40410	int cmd;
40411	#endif
40412	{
40413	  va_list argp;
40414	  message m;
.Op 393 src/lib/posix/_fcntl.c
40415	
40416	  va_start(argp, cmd);
40417	
40418	  /* Set up for the sensible case where there is no variable parameter.  This
40419	   * covers F_GETFD, F_GETFL and invalid commands.
40420	   */
40421	  m.m1_i3 = 0;
40422	  m.m1_p1 = NIL_PTR;
40423	
40424	  /* Adjust for the stupid cases. */
40425	  switch(cmd) {
40426	     case F_DUPFD:
40427	     case F_SETFD:
40428	     case F_SETFL:
40429	        m.m1_i3 = va_arg(argp, int);
40430	        break;
40431	     case F_GETLK:
40432	     case F_SETLK:
40433	     case F_SETLKW:
40434	        m.m1_p1 = (char *) va_arg(argp, struct flock *);
40435	        break;
40436	  }
40437	
40438	  /* Clean up and make the system call. */
40439	  va_end(argp);
40440	  m.m1_i1 = fd;
40441	  m.m1_i2 = cmd;
40442	  return(_syscall(FS, FCNTL, &m));
40443	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_fork.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

40500	#include <lib.h>
40501	#define fork    _fork
40502	#include <unistd.h>
40503	
40504	PUBLIC pid_t fork()
40505	{
40506	  message m;
40507	
40508	  return(_syscall(MM, FORK, &m));
40509	}
.Ep 394 src/lib/posix/_fpathconf.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_fpathconf.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

40600	/* POSIX fpathconf (Sec. 5.7.1)                 Author: Andy Tanenbaum */
40601	
40602	#include <lib.h>
40603	#define fstat           _fstat
40604	#define fpathconf       _fpathconf
40605	#include <sys/stat.h>
40606	#include <errno.h>
40607	#include <limits.h>
40608	#include <unistd.h>
40609	#include <termios.h>
40610	
40611	PUBLIC long fpathconf(fd, name)
40612	int fd;                         /* file descriptor being interrogated */
40613	int name;                       /* property being inspected */
40614	{
40615	/* POSIX allows some of the values in <limits.h> to be increased at
40616	 * run time.  The pathconf and fpathconf functions allow these values
40617	 * to be checked at run time.  MINIX does not use this facility.
40618	 * The run-time limits are those given in <limits.h>.
40619	 */
40620	
40621	  struct stat stbuf;
40622	
40623	  switch(name) {
40624	        case _PC_LINK_MAX:
40625	                /* Fstat the file.  If that fails, return -1. */
40626	                if (fstat(fd, &stbuf) != 0) return(-1);
40627	                if (S_ISDIR(stbuf.st_mode))
40628	                        return(1L);     /* no links to directories */
40629	                else
40630	                        return( (long) LINK_MAX);
40631	
40632	        case _PC_MAX_CANON:
40633	                return( (long) MAX_CANON);
40634	
40635	        case _PC_MAX_INPUT:
40636	                return( (long) MAX_INPUT);
40637	
40638	        case _PC_NAME_MAX:
40639	                return( (long) NAME_MAX);
40640	
40641	        case _PC_PATH_MAX:
40642	                return( (long) PATH_MAX);
40643	
40644	        case _PC_PIPE_BUF:
40645	                return( (long) PIPE_BUF);
40646	
40647	        case _PC_CHOWN_RESTRICTED:
40648	                return( (long) _POSIX_CHOWN_RESTRICTED);
40649	
40650	        case _PC_NO_TRUNC:
40651	                return( (long) _POSIX_NO_TRUNC);
40652	
40653	        case _PC_VDISABLE:
40654	                return( (long) _POSIX_VDISABLE);
.Op 395 src/lib/posix/_fpathconf.c
40655	
40656	        default:
40657	                errno = EINVAL;
40658	                return(-1);
40659	  }
40660	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_fstat.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

40700	#include <lib.h>
40701	#define fstat   _fstat
40702	#include <sys/stat.h>
40703	
40704	PUBLIC int fstat(fd, buffer)
40705	int fd;
40706	struct stat *buffer;
40707	{
40708	  message m;
40709	
40710	  m.m1_i1 = fd;
40711	  m.m1_p1 = (char *) buffer;
40712	  return(_syscall(FS, FSTAT, &m));
40713	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_getcwd.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

40800	/*      getcwd() - get the name of the current working directory.
40801	 *                                                      Author: Kees J. Bot
40802	 *                                                              30 Apr 1989
40803	 */
40804	#define nil 0
40805	#define chdir _chdir
40806	#define closedir _closedir
40807	#define getcwd _getcwd
40808	#define opendir _opendir
40809	#define readdir _readdir
40810	#define rewinddir _rewinddir
40811	#define stat _stat
40812	#include <sys/types.h>
40813	#include <sys/stat.h>
40814	#include <errno.h>
40815	#include <unistd.h>
40816	#include <dirent.h>
40817	#include <limits.h>
40818	#include <string.h>
40819	
40820	static int addpath(const char *path, char **ap, const char *entry)
40821	/* Add the name of a directory entry at the front of the path being built.
40822	 * Note that the result always starts with a slash.
40823	 */
40824	{
.Ep 396 src/lib/posix/_getcwd.c
40825	        const char *e= entry;
40826	        char *p= *ap;
40827	
40828	        while (*e != 0) e++;
40829	
40830	        while (e > entry && p > path) *--p = *--e;
40831	
40832	        if (p == path) return -1;
40833	        *--p = '/';
40834	        *ap= p;
40835	        return 0;
40836	}
	
40838	static int recover(char *p)
40839	/* Undo all those chdir("..")'s that have been recorded by addpath.  This
40840	 * has to be done entry by entry, because the whole pathname may be too long.
40841	 */
40842	{
40843	        int e= errno, slash;
40844	        char *p0;
40845	
40846	        while (*p != 0) {
40847	                p0= ++p;
40848	
40849	                do p++; while (*p != 0 && *p != '/');
40850	                slash= *p; *p= 0;
40851	
40852	                if (chdir(p0) < 0) return -1;
40853	                *p= slash;
40854	        }
40855	        errno= e;
40856	        return 0;
40857	}
	
40859	char *getcwd(char *path, size_t size)
40860	{
40861	        struct stat above, current, tmp;
40862	        struct dirent *entry;
40863	        DIR *d;
40864	        char *p, *up, *dotdot;
40865	        int cycle;
40866	
40867	        if (path == nil || size <= 2) { errno= EINVAL; return nil; }
40868	
40869	        p= path + size;
40870	        *--p = 0;
40871	
40872	        if (stat(".", &current) < 0) return nil;
40873	
40874	        while (1) {
40875	                dotdot= "..";
40876	                if (stat(dotdot, &above) < 0) { recover(p); return nil; }
40877	
40878	                if (above.st_dev == current.st_dev
40879	                                        && above.st_ino == current.st_ino)
40880	                        break;  /* Root dir found */
40881	
40882	                if ((d= opendir(dotdot)) == nil) { recover(p); return nil; }
40883	
40884	                /* Cycle is 0 for a simple inode nr search, or 1 for a search
.Op 397 src/lib/posix/_getcwd.c
40885	                 * for inode *and* device nr.
40886	                 */
40887	                cycle= above.st_dev == current.st_dev ? 0 : 1;
40888	
40889	                do {
40890	                        char name[3 + NAME_MAX + 1];
40891	
40892	                        tmp.st_ino= 0;
40893	                        if ((entry= readdir(d)) == nil) {
40894	                                switch (++cycle) {
40895	                                case 1:
40896	                                        rewinddir(d);
40897	                                        continue;
40898	                                case 2:
40899	                                        closedir(d);
40900	                                        errno= ENOENT;
40901	                                        recover(p);
40902	                                        return nil;
40903	                                }
40904	                        }
40905	                        if (strcmp(entry->d_name, ".") == 0) continue;
40906	                        if (strcmp(entry->d_name, "..") == 0) continue;
40907	
40908	                        switch (cycle) {
40909	                        case 0:
40910	                                /* Simple test on inode nr. */
40911	                                if (entry->d_ino != current.st_ino) continue;
40912	                                /*FALL THROUGH*/
40913	
40914	                        case 1:
40915	                                /* Current is mounted. */
40916	                                strcpy(name, "../");
40917	                                strcpy(name+3, entry->d_name);
40918	                                if (stat(name, &tmp) < 0) continue;
40919	                                break;
40920	                        }
40921	                } while (tmp.st_ino != current.st_ino
40922	                                        || tmp.st_dev != current.st_dev);
40923	
40924	                up= p;
40925	                if (addpath(path, &up, entry->d_name) < 0) {
40926	                        closedir(d);
40927	                        errno = ERANGE;
40928	                        recover(p);
40929	                        return nil;
40930	                }
40931	                closedir(d);
40932	
40933	                if (chdir(dotdot) < 0) { recover(p); return nil; }
40934	                p= up;
40935	
40936	                current= above;
40937	        }
40938	        if (recover(p) < 0) return nil; /* Undo all those chdir("..")'s. */
40939	        if (*p == 0) *--p = '/';        /* Cwd is "/" if nothing added */
40940	        if (p > path) strcpy(path, p);  /* Move string to start of path. */
40941	        return path;
40942	}
.Ep 398 src/lib/posix/_getegid.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_getegid.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

41000	#include <lib.h>
41001	#define getegid _getegid
41002	#include <unistd.h>
41003	
41004	PUBLIC gid_t getegid()
41005	{
41006	  message m;
41007	
41008	  /* POSIX says that this function is always successful and that no
41009	   * return value is reserved to indicate an error.  Minix syscalls
41010	   * are not always successful and Minix returns the unreserved value
41011	   * (gid_t) -1 when there is an error.
41012	   */
41013	  if (_syscall(MM, GETGID, &m) < 0) return ( (gid_t) -1);
41014	  return( (gid_t) m.m2_i1);
41015	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_geteuid.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

41100	#include <lib.h>
41101	#define geteuid _geteuid
41102	#include <unistd.h>
41103	
41104	PUBLIC uid_t geteuid()
41105	{
41106	  message m;
41107	
41108	  /* POSIX says that this function is always successful and that no
41109	   * return value is reserved to indicate an error.  Minix syscalls
41110	   * are not always successful and Minix returns the unreserved value
41111	   * (uid_t) -1 when there is an error.
41112	   */
41113	  if (_syscall(MM, GETUID, &m) < 0) return ( (uid_t) -1);
41114	  return( (uid_t) m.m2_i1);
41115	}
.Op 399 src/lib/posix/_getgid.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_getgid.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

41200	#include <lib.h>
41201	#define getgid  _getgid
41202	#include <unistd.h>
41203	
41204	PUBLIC gid_t getgid()
41205	{
41206	  message m;
41207	
41208	  return( (gid_t) _syscall(MM, GETGID, &m));
41209	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_getgroups.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

41300	/* getgroups.c                                          POSIX 4.2.3
41301	 *      int getgroups(gidsetsize, grouplist);
41302	 *
41303	 *      This call relates to suplementary group ids, which are not
41304	 *      supported in MINIX.
41305	 */
41306	
41307	#include <lib.h>
41308	#define getgroups _getgroups
41309	#include <unistd.h>
41310	#include <time.h>
41311	
41312	PUBLIC int getgroups(gidsetsize, grouplist)
41313	int gidsetsize;
41314	gid_t grouplist[];
41315	{
41316	  return(0);
41317	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_getpgrp.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

41400	#include <lib.h>
41401	#define getpgrp _getpgrp
41402	#include <unistd.h>
41403	
41404	PUBLIC pid_t getpgrp()
41405	{
41406	  message m;
41407	
41408	  return(_syscall(MM, GETPGRP, &m));
41409	}
.Ep 400 src/lib/posix/_getpid.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_getpid.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

41500	#include <lib.h>
41501	#define getpid  _getpid
41502	#include <unistd.h>
41503	
41504	PUBLIC pid_t getpid()
41505	{
41506	  message m;
41507	
41508	  return(_syscall(MM, GETPID, &m));
41509	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_getppid.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

41600	#include <lib.h>
41601	#define getppid _getppid
41602	#include <unistd.h>
41603	
41604	PUBLIC pid_t getppid()
41605	{
41606	  message m;
41607	
41608	  /* POSIX says that this function is always successful and that no
41609	   * return value is reserved to indicate an error.  Minix syscalls
41610	   * are not always successful and Minix returns the reserved value
41611	   * (pid_t) -1 when there is an error.
41612	   */
41613	  if (_syscall(MM, GETPID, &m) < 0) return ( (pid_t) -1);
41614	  return( (pid_t) m.m2_i1);
41615	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_getuid.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

41700	#include <lib.h>
41701	#define getuid  _getuid
41702	#include <unistd.h>
41703	
41704	PUBLIC uid_t getuid()
41705	{
41706	  message m;
41707	
41708	  return( (uid_t) _syscall(MM, GETUID, &m));
41709	}
.Op 401 src/lib/posix/_ioctl.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_ioctl.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

41800	#include <lib.h>
41801	#define ioctl   _ioctl
41802	#include <minix/com.h>
41803	#include <sys/ioctl.h>
41804	
41805	PUBLIC int ioctl(fd, request, data)
41806	int fd;
41807	int request;
41808	void *data;
41809	{
41810	  message m;
41811	
41812	  m.TTY_LINE = fd;
41813	  m.TTY_REQUEST = request;
41814	  m.ADDRESS = (char *) data;
41815	  return(_syscall(FS, IOCTL, &m));
41816	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_isatty.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

41900	#include <lib.h>
41901	#define isatty _isatty
41902	#define tcgetattr _tcgetattr
41903	#include <termios.h>
41904	#include <unistd.h>
41905	
41906	PUBLIC int isatty(fd)
41907	int fd;
41908	{
41909	  struct termios dummy;
41910	
41911	  return(tcgetattr(fd, &dummy) == 0);
41912	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_kill.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

42000	#include <lib.h>
42001	#define kill    _kill
42002	#include <signal.h>
42003	
42004	PUBLIC int kill(proc, sig)
42005	int proc;                       /* which process is to be sent the signal */
42006	int sig;                        /* signal number */
42007	{
42008	  message m;
42009	
.Ep 402 src/lib/posix/_kill.c
42010	  m.m1_i1 = proc;
42011	  m.m1_i2 = sig;
42012	  return(_syscall(MM, KILL, &m));
42013	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_link.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

42100	#include <lib.h>
42101	#define link    _link
42102	#include <string.h>
42103	#include <unistd.h>
42104	
42105	PUBLIC int link(name, name2)
42106	_CONST char *name, *name2;
42107	{
42108	  message m;
42109	
42110	  m.m1_i1 = strlen(name) + 1;
42111	  m.m1_i2 = strlen(name2) + 1;
42112	  m.m1_p1 = (char *) name;
42113	  m.m1_p2 = (char *) name2;
42114	  return(_syscall(FS, LINK, &m));
42115	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_lseek.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

42200	#include <lib.h>
42201	#define lseek   _lseek
42202	#include <unistd.h>
42203	
42204	PUBLIC off_t lseek(fd, offset, whence)
42205	int fd;
42206	off_t offset;
42207	int whence;
42208	{
42209	  message m;
42210	
42211	  m.m2_i1 = fd;
42212	  m.m2_l1 = offset;
42213	  m.m2_i2 = whence;
42214	  if (_syscall(FS, LSEEK, &m) < 0) return( (off_t) -1);
42215	  return( (off_t) m.m2_l1);
42216	}
.Op 403 src/lib/posix/_mkdir.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_mkdir.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

42300	#include <lib.h>
42301	#define mkdir   _mkdir
42302	#include <sys/stat.h>
42303	#include <string.h>
42304	
42305	PUBLIC int mkdir(name, mode)
42306	_CONST char *name;
42307	Mode_t mode;
42308	{
42309	  message m;
42310	
42311	  m.m1_i1 = strlen(name) + 1;
42312	  m.m1_i2 = mode;
42313	  m.m1_p1 = (char *) name;
42314	  return(_syscall(FS, MKDIR, &m));
42315	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_mkfifo.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

42400	#include <lib.h>
42401	#define mkfifo  _mkfifo
42402	#define mknod   _mknod
42403	#include <sys/stat.h>
42404	#include <unistd.h>
42405	
42406	PUBLIC int mkfifo(name, mode)
42407	_CONST char *name;
42408	Mode_t mode;
42409	{
42410	  return mknod(name, mode | S_IFIFO, (Dev_t) 0);
42411	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_mknod.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

42500	#include <lib.h>
42501	#define mknod   _mknod
42502	#include <string.h>
42503	#include <stdlib.h>
42504	#include <unistd.h>
42505	
42506	PUBLIC int mknod(name, mode, dev)
42507	_CONST char *name;
42508	Mode_t mode;
42509	Dev_t dev;
.Ep 404 src/lib/posix/_mknod.c
42510	{
42511	  message m;
42512	
42513	  m.m1_i1 = strlen(name) + 1;
42514	  m.m1_i2 = mode;
42515	  m.m1_i3 = dev;
42516	  m.m1_p1 = (char *) name;
42517	  m.m1_p2 = (char *) ((int) 0);         /* obsolete size field */
42518	  return(_syscall(FS, MKNOD, &m));
42519	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_mktemp.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

42600	/* mktemp - make a name for a temporary file */
42601	
42602	#include <lib.h>
42603	#define access  _access
42604	#define getpid  _getpid
42605	#define mktemp  _mktemp
42606	#include <unistd.h>
42607	
42608	PUBLIC char *mktemp(template)
42609	char *template;
42610	{
42611	  register int k;
42612	  register char *p;
42613	  register pid_t pid;
42614	
42615	  pid = getpid();               /* get process id as semi-unique number */
42616	  p = template;
42617	  while (*p != 0) p++;          /* find end of string */
42618	
42619	  /* Replace XXXXXX at end of template with a letter, then as many of the
42620	   * trailing digits of the pid as fit.
42621	   */
42622	  while (*--p == 'X') {
42623	        *p = '0' + (pid % 10);
42624	        pid /= 10;
42625	  }
42626	  if (*++p != 0) {
42627	        for (k = 'a'; k <= 'z'; k++) {
42628	                *p = k;
42629	                if (access(template, F_OK) < 0) return(template);
42630	        }
42631	  }
42632	  return("/");
42633	}
.Op 405 src/lib/posix/_mount.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_mount.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

42700	#include <lib.h>
42701	#define mount   _mount
42702	#include <string.h>
42703	#include <unistd.h>
42704	
42705	PUBLIC int mount(special, name, rwflag)
42706	char *name, *special;
42707	int rwflag;
42708	{
42709	  message m;
42710	
42711	  m.m1_i1 = strlen(special) + 1;
42712	  m.m1_i2 = strlen(name) + 1;
42713	  m.m1_i3 = rwflag;
42714	  m.m1_p1 = special;
42715	  m.m1_p2 = name;
42716	  return(_syscall(FS, MOUNT, &m));
42717	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_open.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

42800	#include <lib.h>
42801	#define open    _open
42802	#include <fcntl.h>
42803	#include <stdarg.h>
42804	#include <string.h>
42805	
42806	#if _ANSI
42807	PUBLIC int open(const char *name, int flags, ...)
42808	#else
42809	PUBLIC int open(name, flags)
42810	_CONST char *name;
42811	int flags;
42812	#endif
42813	{
42814	  va_list argp;
42815	  message m;
42816	
42817	  va_start(argp, flags);
42818	  if (flags & O_CREAT) {
42819	        m.m1_i1 = strlen(name) + 1;
42820	        m.m1_i2 = flags;
42821	        m.m1_i3 = va_arg(argp, Mode_t);
42822	        m.m1_p1 = (char *) name;
42823	  } else {
42824	        _loadname(name, &m);
42825	        m.m3_i2 = flags;
42826	  }
42827	  va_end(argp);
42828	  return (_syscall(FS, OPEN, &m));
42829	}
.Ep 406 src/lib/posix/_opendir.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_opendir.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

42900	/*      opendir()                                       Author: Kees J. Bot
42901	 *                                                              24 Apr 1989
42902	 */
42903	#define nil 0
42904	#include <lib.h>
42905	#define close   _close
42906	#define fcntl   _fcntl
42907	#define fstat   _fstat
42908	#define open    _open
42909	#define opendir _opendir
42910	#define stat    _stat
42911	#include <sys/types.h>
42912	#include <sys/stat.h>
42913	#include <dirent.h>
42914	#include <unistd.h>
42915	#include <stdlib.h>
42916	#include <fcntl.h>
42917	#include <errno.h>
42918	
42919	DIR *opendir(const char *name)
42920	/* Open a directory for reading. */
42921	{
42922	        int d, f;
42923	        DIR *dp;
42924	        struct stat st;
42925	
42926	        /* Only read directories. */
42927	        if (stat(name, &st) < 0) return nil;
42928	        if (!S_ISDIR(st.st_mode)) { errno= ENOTDIR; return nil; }
42929	
42930	        if ((d= open(name, O_RDONLY | O_NONBLOCK)) < 0) return nil;
42931	
42932	        /* Check the type again, mark close-on-exec, get a buffer. */
42933	        if (fstat(d, &st) < 0
42934	                || (errno= ENOTDIR, !S_ISDIR(st.st_mode))
42935	                || (f= fcntl(d, F_GETFD)) < 0
42936	                || fcntl(d, F_SETFD, f | FD_CLOEXEC) < 0
42937	                || (dp= (DIR *) malloc(sizeof(*dp))) == nil
42938	        ) {
42939	                int err= errno;
42940	                (void) close(d);
42941	                errno= err;
42942	                return nil;
42943	        }
42944	
42945	        dp->_fd= d;
42946	        dp->_v7= -1;
42947	        dp->_count= 0;
42948	        dp->_pos= 0;
42949	
42950	        return dp;
42951	}
.Op 407 src/lib/posix/_pathconf.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_pathconf.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

43000	/* POSIX pathconf (Sec. 5.7.1)          Author: Andy Tanenbaum */
43001	
43002	#include <lib.h>
43003	#define close           _close
43004	#define open            _open
43005	#define pathconf        _pathconf
43006	#include <fcntl.h>
43007	#include <errno.h>
43008	#include <unistd.h>
43009	
43010	PUBLIC long pathconf(path, name)
43011	_CONST char *path;              /* name of file being interrogated */
43012	int name;                       /* property being inspected */
43013	{
43014	/* POSIX allows some of the values in <limits.h> to be increased at
43015	 * run time.  The pathconf and fpathconf functions allow these values
43016	 * to be checked at run time.  MINIX does not use this facility.
43017	 * The run-time limits are those given in <limits.h>.
43018	 */
43019	
43020	  int fd;
43021	  long val;
43022	
43023	  if ( (fd = open(path, O_RDONLY)) < 0) return(-1L);
43024	  val = fpathconf(fd, name);
43025	  close(fd);
43026	  return(val);
43027	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_pause.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

43100	#include <lib.h>
43101	#define pause   _pause
43102	#include <unistd.h>
43103	
43104	PUBLIC int pause()
43105	{
43106	  message m;
43107	
43108	  return(_syscall(MM, PAUSE, &m));
43109	}
.Ep 408 src/lib/posix/_pipe.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_pipe.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

43200	#include <lib.h>
43201	#define pipe    _pipe
43202	#include <unistd.h>
43203	
43204	PUBLIC int pipe(fild)
43205	int fild[2];
43206	{
43207	  message m;
43208	
43209	  if (_syscall(FS, PIPE, &m) < 0) return(-1);
43210	  fild[0] = m.m1_i1;
43211	  fild[1] = m.m1_i2;
43212	  return(0);
43213	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_ptrace.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

43300	#include <lib.h>
43301	#define ptrace  _ptrace
43302	#include <unistd.h>
43303	
43304	PUBLIC long ptrace(req, pid, addr, data)
43305	int req;
43306	pid_t pid;
43307	long addr;
43308	long data;
43309	{
43310	  message m;
43311	
43312	  m.m2_i1 = pid;
43313	  m.m2_i2 = req;
43314	  m.m2_l1 = addr;
43315	  m.m2_l2 = data;
43316	  if (_syscall(MM, PTRACE, &m) < 0) return(-1);
43317	
43318	  /* There was no error, but -1 is a legal return value.  Clear errno if
43319	   * necessary to distinguish this case.  _syscall has set errno to nonzero
43320	   * for the error case.
43321	   */
43322	  if (m.m2_l2 == -1) errno = 0;
43323	  return(m.m2_l2);
43324	}
.Op 409 src/lib/posix/_read.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_read.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

43400	#include <lib.h>
43401	#define read    _read
43402	#include <unistd.h>
43403	
43404	PUBLIC ssize_t read(fd, buffer, nbytes)
43405	int fd;
43406	void *buffer;
43407	size_t nbytes;
43408	{
43409	  message m;
43410	
43411	  m.m1_i1 = fd;
43412	  m.m1_i2 = nbytes;
43413	  m.m1_p1 = (char *) buffer;
43414	  return(_syscall(FS, READ, &m));
43415	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_readdir.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

43500	/*      readdir()                                       Author: Kees J. Bot
43501	 *                                                              24 Apr 1989
43502	 */
43503	#define nil 0
43504	#include <lib.h>
43505	#define read    _read
43506	#define readdir _readdir
43507	#include <sys/types.h>
43508	#include <sys/stat.h>
43509	#include <dirent.h>
43510	#include <unistd.h>
43511	#include <stdlib.h>
43512	#include <fcntl.h>
43513	#include <limits.h>
43514	#include <errno.h>
43515	#include <string.h>
43516	
43517	#define v7ent(p)        ((struct _v7_direct *) (p))
43518	
43519	struct dirent *readdir(DIR *dp)
43520	/* Return the next entry in a directory.  Handle V7 and FLEX format dirs. */
43521	{
43522	        struct dirent *e;
43523	
43524	        if (dp == nil) { errno= EBADF; return nil; }
43525	
43526	        do {
43527	                if (dp->_count <= 0) {
43528	                        /* Read the next directory block. */
43529	                        dp->_count= read(dp->_fd, dp->_buf, sizeof(dp->_buf));
.Ep 410 src/lib/posix/_readdir.c
43530	                        if (dp->_count <= 0) return nil;
43531	
43532	                        dp->_count/= sizeof(dp->_buf[0]);
43533	                        dp->_ptr= dp->_buf;
43534	
43535	                        /* Extent is zero of the first flex entry. */
43536	                        if (dp->_v7 == (char)-1) dp->_v7= dp->_buf[0].d_extent;
43537	                }
43538	
43539	                if (!dp->_v7) {
43540	                        /* FLEX. */
43541	                        e= (struct dirent *) dp->_ptr;
43542	                } else {
43543	                        /* V7: transform to FLEX. */
43544	                        e= (struct dirent *) dp->_v7f;
43545	                        e->d_ino= v7ent(dp->_ptr)->d_ino;
43546	                        e->d_extent= 1;
43547	                        memcpy(e->d_name, v7ent(dp->_ptr)->d_name, 14);
43548	                        e->d_name[14]= 0;
43549	                }
43550	
43551	                dp->_ptr+= 1 + e->d_extent;
43552	                dp->_count-= 1 + e->d_extent;
43553	                dp->_pos+= (1 + e->d_extent) * sizeof(*dp->_ptr);
43554	
43555	        } while (e->d_ino == 0);
43556	        return e;
43557	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_rename.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

43600	#include <lib.h>
43601	#define rename  _rename
43602	#include <string.h>
43603	#include <stdio.h>
43604	
43605	PUBLIC int rename(name, name2)
43606	_CONST char *name, *name2;
43607	{
43608	  message m;
43609	
43610	  m.m1_i1 = strlen(name) + 1;
43611	  m.m1_i2 = strlen(name2) + 1;
43612	  m.m1_p1 = (char *) name;
43613	  m.m1_p2 = (char *) name2;
43614	  return(_syscall(FS, RENAME, &m));
43615	}
.Op 411 src/lib/posix/_rewinddir.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_rewinddir.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

43700	/*      rewinddir()                                     Author: Kees J. Bot
43701	 *                                                              24 Apr 1989
43702	 */
43703	#define nil 0
43704	#include <lib.h>
43705	#define rewinddir _rewinddir
43706	#define seekdir _seekdir
43707	#include <sys/types.h>
43708	#include <dirent.h>
43709	
43710	void rewinddir(DIR *dp)
43711	{
43712	        (void) seekdir(dp, 0);
43713	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_rmdir.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

43800	#include <lib.h>
43801	#define rmdir   _rmdir
43802	#include <unistd.h>
43803	
43804	PUBLIC int rmdir(name)
43805	_CONST char *name;
43806	{
43807	  message m;
43808	
43809	  _loadname(name, &m);
43810	  return(_syscall(FS, RMDIR, &m));
43811	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_setgid.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

43900	#include <lib.h>
43901	#define setgid  _setgid
43902	#include <unistd.h>
43903	
43904	PUBLIC int setgid(grp)
43905	gid_t grp;
43906	{
43907	  message m;
43908	
43909	  m.m1_i1 = (int) grp;
43910	  return(_syscall(MM, SETGID, &m));
43911	}
.Ep 412 src/lib/posix/_setsid.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_setsid.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

44000	#include <lib.h>
44001	#define setsid  _setsid
44002	#include <unistd.h>
44003	
44004	PUBLIC pid_t setsid()
44005	{
44006	  message m;
44007	
44008	  return(_syscall(MM, SETSID, &m));
44009	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_setuid.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

44100	#include <lib.h>
44101	#define setuid  _setuid
44102	#include <unistd.h>
44103	
44104	PUBLIC int setuid(usr)
44105	Uid_t usr;
44106	{
44107	  message m;
44108	
44109	  m.m1_i1 = usr;
44110	  return(_syscall(MM, SETUID, &m));
44111	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_sigaction.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

44200	#include <lib.h>
44201	#define sigaction _sigaction
44202	#include <sys/sigcontext.h>
44203	#include <signal.h>
44204	
44205	_PROTOTYPE(int __sigreturn, (void));
44206	
44207	PUBLIC int sigaction(sig, act, oact)
44208	int sig;
44209	_CONST struct sigaction *act;
44210	struct sigaction *oact;
44211	{
44212	  message m;
44213	
44214	  m.m1_i2 = sig;
.Op 413 src/lib/posix/_sigaction.c
44215	
44216	  /* XXX - yet more type puns because message struct is short of types. */
44217	  m.m1_p1 = (char *) act;
44218	  m.m1_p2 = (char *) oact;
44219	  m.m1_p3 = (char *) __sigreturn;
44220	
44221	  return(_syscall(MM, SIGACTION, &m));
44222	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_sigpending.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

44300	#include <lib.h>
44301	#define sigpending _sigpending
44302	#include <signal.h>
44303	
44304	PUBLIC int sigpending(set)
44305	sigset_t *set;
44306	{
44307	  message m;
44308	
44309	  if (_syscall(MM, SIGPENDING, &m) < 0) return(-1);
44310	  *set = (sigset_t) m.m2_l1;
44311	  return(m.m_type);
44312	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_sigprocmask.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

44400	#include <lib.h>
44401	#define sigprocmask _sigprocmask
44402	#include <signal.h>
44403	
44404	PUBLIC int sigprocmask(how, set, oset)
44405	int how;
44406	_CONST sigset_t *set;
44407	sigset_t *oset;
44408	{
44409	  message m;
44410	
44411	  if (set == (sigset_t *) NULL) {
44412	        m.m2_i1 = SIG_INQUIRE;
44413	        m.m2_l1 = 0;
44414	  } else {
44415	        m.m2_i1 = how;
44416	        m.m2_l1 = (long) *set;
44417	  }
44418	  if (_syscall(MM, SIGPROCMASK, &m) < 0) return(-1);
44419	  if (oset != (sigset_t *) NULL) *oset = (sigset_t) (m.m2_l1);
44420	  return(m.m_type);
44421	}
.Ep 414 src/lib/posix/_sigreturn.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_sigreturn.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

44500	#include <lib.h>
44501	#define sigfillset      _sigfillset
44502	#define sigjmp          _sigjmp
44503	#define sigprocmask     _sigprocmask
44504	#define sigreturn       _sigreturn
44505	#include <sys/sigcontext.h>
44506	#include <setjmp.h>
44507	#include <signal.h>
44508	
44509	_PROTOTYPE( int sigjmp, (jmp_buf jb, int retval));
44510	
44511	#if (_SETJMP_SAVES_REGS == 0)
44512	/* 'sigreturn' using a short format jmp_buf (no registers saved). */
44513	PUBLIC int sigjmp(jb, retval)
44514	jmp_buf jb;
44515	int retval;
44516	{
44517	  struct sigcontext sc;
44518	
44519	  sc.sc_flags = jb[0].__flags;
44520	  sc.sc_mask = jb[0].__mask;
44521	
44522	#if (CHIP == INTEL)
44523	  sc.sc_pc = (int) jb[0].__pc;
44524	  sc.sc_sp = (int) jb[0].__sp;
44525	  sc.sc_fp = (int) jb[0].__lb;
44526	#endif
44527	
44528	#if (CHIP == M68000)
44529	  sc.sc_pc = (long) jb[0].__pc;
44530	  sc.sc_sp = (long) jb[0].__sp;
44531	  sc.sc_fp = (long) jb[0].__lb;
44532	#endif
44533	
44534	  sc.sc_retreg = retval;
44535	  return sigreturn(&sc);
44536	}
44537	#endif
44538	
44539	PUBLIC int sigreturn(scp)
44540	register struct sigcontext *scp;
44541	{
44542	  sigset_t set;
44543	
44544	  /* The message can't be on the stack, because the stack will vanish out
44545	   * from under us.  The send part of sendrec will succeed, but when
44546	   * a message is sent to restart the current process, who knows what will
44547	   * be in the place formerly occupied by the message?
44548	   */
44549	  static message m;
44550	
44551	  /* Protect against race conditions by blocking all interrupts. */
44552	  sigfillset(&set);             /* splhi */
44553	  sigprocmask(SIG_SETMASK, &set, (sigset_t *) NULL);
44554	
.Op 415 src/lib/posix/_sigreturn.c
44555	  m.m2_l1 = scp->sc_mask;
44556	  m.m2_i2 = scp->sc_flags;
44557	  m.m2_p1 = (char *) scp;
44558	  return(_syscall(MM, SIGRETURN, &m));  /* normally this doesn't return */
44559	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_sigset.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

44600	#include <lib.h>
44601	/* XXX - these have to be hidden because signal() uses them and signal() is
44602	 * ANSI and not POSIX.  It would be surely be better to use macros for the
44603	 * library and system uses, and perhaps macros as well as functions for the
44604	 * POSIX user interface.  The macros would not need underlines.  It may be
44605	 * inconvenient to match the exact semantics of the current functions
44606	 * because the interface is bloated by reporting errors.  For library and
44607	 * system uses, the signal number is mostly already known to be valid
44608	 * before the sigset-changing routines are called.
44609	 */
44610	#define sigaddset       _sigaddset
44611	#define sigdelset       _sigdelset
44612	#define sigemptyset     _sigemptyset
44613	#define sigfillset      _sigfillset
44614	#define sigismember     _sigismember
44615	#include <signal.h>
44616	
44617	/* Low bit of signal masks. */
44618	#define SIGBIT_0        ((sigset_t) 1)
44619	
44620	/* Mask of valid signals (0 - _NSIG).  Assume the shift doesn't overflow. */
44621	#define SIGMASK         ((SIGBIT_0 << (_NSIG + 1)) - 1)
44622	
44623	#define sigisvalid(signo) ((unsigned) (signo) <= _NSIG)
44624	
44625	PUBLIC int sigaddset(set, signo)
44626	sigset_t *set;
44627	int signo;
44628	{
44629	  if (!sigisvalid(signo)) {
44630	        errno = EINVAL;
44631	        return -1;
44632	  }
44633	  *set |= SIGBIT_0 << signo;
44634	  return 0;
44635	}
	
44637	PUBLIC int sigdelset(set, signo)
44638	sigset_t *set;
44639	int signo;
44640	{
44641	  if (!sigisvalid(signo)) {
44642	        errno = EINVAL;
44643	        return -1;
44644	  }
.Ep 416 src/lib/posix/_sigset.c
44645	  *set &= ~(SIGBIT_0 << signo);
44646	  return 0;
44647	}
	
44649	PUBLIC int sigemptyset(set)
44650	sigset_t *set;
44651	{
44652	  *set = 0;
44653	  return 0;
44654	}
	
44656	PUBLIC int sigfillset(set)
44657	sigset_t *set;
44658	{
44659	  *set = SIGMASK;
44660	  return 0;
44661	}
	
44663	PUBLIC int sigismember(set, signo)
44664	sigset_t *set;
44665	int signo;
44666	{
44667	  if (!sigisvalid(signo)) {
44668	        errno = EINVAL;
44669	        return -1;
44670	  }
44671	  if (*set & (SIGBIT_0 << signo))
44672	        return 1;
44673	  return 0;
44674	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_sigsetjmp.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

44700	#include <lib.h>
44701	#include <sys/sigcontext.h>
44702	#include <setjmp.h>
44703	
44704	PUBLIC void siglongjmp(env, val)
44705	sigjmp_buf env;
44706	int val;
44707	{
44708	  if (env[0].__flags & SC_SIGCONTEXT)
44709	        longjmp(env, val);
44710	  else
44711	        _longjmp(env, val);
44712	}
.Op 417 src/lib/posix/_sigsuspend.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_sigsuspend.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

44800	#include <lib.h>
44801	#define sigsuspend _sigsuspend
44802	#include <signal.h>
44803	
44804	PUBLIC int sigsuspend(set)
44805	_CONST sigset_t *set;
44806	{
44807	  message m;
44808	
44809	  m.m2_l1 = (long) *set;
44810	  return(_syscall(MM, SIGSUSPEND, &m));
44811	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_sleep.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

44900	/*  sleep(3)
44901	 *
44902	 *  Sleep(n) pauses for 'n' seconds by scheduling an alarm interrupt.
44903	 *
44904	 *  Changed to conform with POSIX      Terrence W. Holm      Oct. 1988
44905	 */
44906	
44907	#include <lib.h>
44908	#define sleep _sleep
44909	#include <signal.h>
44910	#include <unistd.h>
44911	
44912	FORWARD _PROTOTYPE( void _alfun, (int signo) );
44913	
44914	PRIVATE void _alfun(signo)
44915	int signo;
44916	{
44917	/* Dummy signal handler used with sleep() below. */
44918	}
	
44920	PUBLIC unsigned sleep(secs)
44921	unsigned secs;
44922	{
44923	  unsigned current_secs;
44924	  unsigned remaining_secs;
44925	  struct sigaction act, oact;
44926	  sigset_t ss;
44927	
44928	  if (secs == 0) return(0);
44929	
44930	  current_secs = alarm(0);      /* is there currently an alarm?  */
44931	
44932	  if (current_secs == 0 || current_secs > secs) {
44933	        act.sa_flags = 0;
44934	        act.sa_mask = 0;
.Ep 418 src/lib/posix/_sleep.c
44935	        act.sa_handler = _alfun;
44936	        sigaction(SIGALRM, &act, &oact);
44937	
44938	        alarm(secs);
44939	        sigemptyset(&ss);
44940	        sigsuspend(&ss);
44941	        remaining_secs = alarm(0);
44942	
44943	        sigaction(SIGALRM, &oact, (struct sigaction *) NULL);
44944	
44945	        if (current_secs > secs)
44946	                alarm(current_secs - (secs - remaining_secs));
44947	
44948	        return(remaining_secs);
44949	  }
44950	
44951	  /* Current_secs <= secs,  ie. alarm should occur before secs.  */
44952	
44953	  alarm(current_secs);
44954	  pause();
44955	  remaining_secs = alarm(0);
44956	
44957	  alarm(remaining_secs);
44958	
44959	  return(secs - (current_secs - remaining_secs));
44960	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_stat.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

45000	#include <lib.h>
45001	#define stat    _stat
45002	#include <sys/stat.h>
45003	#include <string.h>
45004	
45005	PUBLIC int stat(name, buffer)
45006	_CONST char *name;
45007	struct stat *buffer;
45008	{
45009	  message m;
45010	
45011	  m.m1_i1 = strlen(name) + 1;
45012	  m.m1_p1 = (char *) name;
45013	  m.m1_p2 = (char *) buffer;
45014	  return(_syscall(FS, STAT, &m));
45015	}
.Op 419 src/lib/posix/_stime.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_stime.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

45100	#include <lib.h>
45101	#define stime   _stime
45102	#include <minix/minlib.h>
45103	#include <time.h>
45104	
45105	PUBLIC int stime(top)
45106	long *top;
45107	{
45108	  message m;
45109	
45110	  m.m2_l1 = *top;
45111	  return(_syscall(FS, STIME, &m));
45112	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_sync.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

45200	#include <lib.h>
45201	#define sync    _sync
45202	#include <unistd.h>
45203	
45204	PUBLIC int sync()
45205	{
45206	  message m;
45207	
45208	  return(_syscall(FS, SYNC, &m));
45209	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_tcdrain.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

45300	/*
45301	posix/_tcdrain.c
45302	
45303	Created:        July 26, 1994 by Philip Homburg
45304	*/
45305	
45306	#define tcdrain _tcdrain
45307	#define ioctl _ioctl
45308	#include <termios.h>
45309	#include <sys/ioctl.h>
45310	
45311	int tcdrain(fd)
45312	int fd;
45313	{
45314	  return(ioctl(fd, TCDRAIN, (void *)0));
.Ep 420 src/lib/posix/_tcdrain.c
45315	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_tcflow.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

45400	/*
45401	posix/_tcflow.c
45402	
45403	Created:        June 8, 1993 by Philip Homburg
45404	*/
45405	
45406	#define tcflow _tcflow
45407	#define ioctl _ioctl
45408	#include <termios.h>
45409	#include <sys/ioctl.h>
45410	
45411	int tcflow(fd, action)
45412	int fd;
45413	int action;
45414	{
45415	  return(ioctl(fd, TCFLOW, &action));
45416	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_tcflush.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

45500	/*      tcflush() - flush buffered characters           Author: Kees J. Bot
45501	 *                                                              13 Jan 1994
45502	 */
45503	#define tcflush _tcflush
45504	#define ioctl _ioctl
45505	#include <termios.h>
45506	#include <sys/ioctl.h>
45507	
45508	int tcflush(int fd, int queue_selector)
45509	{
45510	  return(ioctl(fd, TCFLSH, &queue_selector));
45511	}
.Op 421 src/lib/posix/_tcgetattr.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_tcgetattr.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

45600	#define tcgetattr _tcgetattr
45601	#define ioctl _ioctl
45602	#include <sys/ioctl.h>
45603	#include <errno.h>
45604	#include <termios.h>
45605	
45606	int tcgetattr(fd, termios_p)
45607	int fd;
45608	struct termios *termios_p;
45609	{
45610	  return(ioctl(fd, TCGETS, termios_p));
45611	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_tcsendbreak.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

45700	/*      tcsendbreak() - send a break                    Author: Kees J. Bot
45701	 *                                                              13 Jan 1994
45702	 */
45703	#define tcsendbreak _tcsendbreak
45704	#define ioctl _ioctl
45705	#include <termios.h>
45706	#include <sys/ioctl.h>
45707	
45708	int tcsendbreak(int fd, int duration)
45709	{
45710	  return(ioctl(fd, TCSBRK, &duration));
45711	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_tcsetattr.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

45800	/*
45801	posix/_tcsetattr.c
45802	
45803	Created:        June 11, 1993 by Philip Homburg
45804	*/
45805	
45806	#define tcsetattr _tcsetattr
45807	#define ioctl _ioctl
45808	#include <errno.h>
45809	#include <termios.h>
45810	#include <sys/ioctl.h>
45811	#include <sys/types.h>
45812	
45813	int tcsetattr(fd, opt_actions, termios_p)
45814	int fd;
.Ep 422 src/lib/posix/_tcsetattr.c
45815	int opt_actions;
45816	_CONST struct termios *termios_p;
45817	{
45818	  int request;
45819	
45820	  switch(opt_actions)
45821	  {
45822	    case TCSANOW:       request = TCSETS;       break;
45823	    case TCSADRAIN:     request = TCSETSW;      break;
45824	    case TCSAFLUSH:     request = TCSETSF;      break;
45825	    default:            errno = EINVAL;         return(-1);
45826	  };
45827	  return(ioctl(fd, request, (void *) termios_p));
45828	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_time.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

45900	#include <lib.h>
45901	#define time    _time
45902	#include <time.h>
45903	
45904	PUBLIC time_t time(tp)
45905	time_t *tp;
45906	{
45907	  message m;
45908	
45909	  if (_syscall(FS, TIME, &m) < 0) return( (time_t) -1);
45910	  if (tp != (time_t *) 0) *tp = m.m2_l1;
45911	  return(m.m2_l1);
45912	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_times.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

46000	#include <lib.h>
46001	#define times   _times
46002	#include <sys/times.h>
46003	#include <time.h>
46004	
46005	PUBLIC clock_t times(buf)
46006	struct tms *buf;
46007	{
46008	  message m;
46009	
46010	  m.m4_l5 = 0;                  /* return this if system is pre-1.6 */
46011	  if (_syscall(FS, TIMES, &m) < 0) return( (clock_t) -1);
46012	  buf->tms_utime = m.m4_l1;
46013	  buf->tms_stime = m.m4_l2;
46014	  buf->tms_cutime = m.m4_l3;
46015	  buf->tms_cstime = m.m4_l4;
46016	  return(m.m4_l5);
46017	}
.Op 423 src/lib/posix/_umask.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_umask.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

46100	#include <lib.h>
46101	#define umask   _umask
46102	#include <sys/stat.h>
46103	
46104	PUBLIC mode_t umask(complmode)
46105	Mode_t complmode;
46106	{
46107	  message m;
46108	
46109	  m.m1_i1 = complmode;
46110	  return( (mode_t) _syscall(FS, UMASK, &m));
46111	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_umount.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

46200	#include <lib.h>
46201	#define umount  _umount
46202	#include <unistd.h>
46203	
46204	PUBLIC int umount(name)
46205	_CONST char *name;
46206	{
46207	  message m;
46208	
46209	  _loadname(name, &m);
46210	  return(_syscall(FS, UMOUNT, &m));
46211	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_uname.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

46300	/*      uname() - get system info                       Author: Kees J. Bot
46301	 *                                                              7 Nov 1994
46302	 * Returns information about the Minix system.  Alas most
46303	 * of it is gathered at compile time, so machine is wrong, and
46304	 * release and version become wrong if not recompiled.
46305	 * More chip types and Minix versions need to be added.
46306	 */
46307	#define uname   _uname
46308	#define open    _open
46309	#define read    _read
46310	#define close   _close
46311	#include <sys/types.h>
46312	#include <sys/utsname.h>
46313	#include <unistd.h>
46314	#include <fcntl.h>
.Ep 424 src/lib/posix/_uname.c
46315	#include <string.h>
46316	#include <errno.h>
46317	#include <minix/config.h>
46318	#include <minix/minlib.h>
46319	
46320	int uname(name) struct utsname *name;
46321	{
46322	  int hf, n, err;
46323	  char *nl;
46324	
46325	  /* Read the node name from /etc/hostname.file. */
46326	  if ((hf = open("/etc/hostname.file", O_RDONLY)) < 0) {
46327	        if (errno != ENOENT) return(-1);
46328	        strcpy(name->nodename, "noname");
46329	  } else {
46330	        n = read(hf, name->nodename, sizeof(name->nodename) - 1);
46331	        err = errno;
46332	        close(hf);
46333	        errno = err;
46334	        if (n < 0) return(-1);
46335	        name->nodename[n] = 0;
46336	        if ((nl = strchr(name->nodename, '\n')) != NULL) {
46337	                memset(nl, 0, (name->nodename + sizeof(name->nodename)) - nl);
46338	        }
46339	  }
46340	
46341	  strcpy(name->sysname, "Minix");
46342	  strcpy(name->release, OS_RELEASE);
46343	  strcpy(name->version, OS_VERSION);
46344	#if (CHIP == INTEL)
46345	  name->machine[0] = 'i';
46346	  strcpy(name->machine + 1, itoa(getprocessor()));
46347	#if _WORD_SIZE == 4
46348	  strcpy(name->arch, "i386");
46349	#else
46350	  strcpy(name->arch, "i86");
46351	#endif
46352	#endif
46353	  return(0);
46354	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_unlink.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

46400	#include <lib.h>
46401	#define unlink  _unlink
46402	#include <unistd.h>
46403	
46404	PUBLIC int unlink(name)
46405	_CONST char *name;
46406	{
46407	  message m;
46408	
46409	  _loadname(name, &m);
.Op 425 src/lib/posix/_unlink.c
46410	  return(_syscall(FS, UNLINK, &m));
46411	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_utime.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

46500	/* utime(2) for POSIX           Authors: Terrence W. Holm & Edwin L. Froese */
46501	
46502	#include <lib.h>
46503	#define utime   _utime
46504	#include <string.h>
46505	#include <utime.h>
46506	
46507	PUBLIC int utime(name, timp)
46508	_CONST char *name;
46509	_CONST struct utimbuf *timp;
46510	{
46511	  message m;
46512	
46513	  if (timp == NULL) {
46514	        m.m2_i1 = 0;            /* name size 0 means NULL `timp' */
46515	        m.m2_i2 = strlen(name) + 1;     /* actual size here */
46516	  } else {
46517	        m.m2_l1 = timp->actime;
46518	        m.m2_l2 = timp->modtime;
46519	        m.m2_i1 = strlen(name) + 1;
46520	  }
46521	  m.m2_p1 = (char *) name;
46522	  return(_syscall(FS, UTIME, &m));
46523	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_wait.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

46600	#include <lib.h>
46601	#define wait    _wait
46602	#include <sys/wait.h>
46603	
46604	PUBLIC pid_t wait(status)
46605	int *status;
46606	{
46607	  message m;
46608	
46609	  if (_syscall(MM, WAIT, &m) < 0) return(-1);
46610	  if (status != 0) *status = m.m2_i1;
46611	  return(m.m_type);
46612	}
.Ep 426 src/lib/posix/_waitpid.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_waitpid.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

46700	#include <lib.h>
46701	#define waitpid _waitpid
46702	#include <sys/wait.h>
46703	
46704	PUBLIC pid_t waitpid(pid, status, options)
46705	pid_t pid;
46706	int *status;
46707	int options;
46708	{
46709	  message m;
46710	
46711	  m.m1_i1 = pid;
46712	  m.m1_i2 = options;
46713	  if (_syscall(MM, WAITPID, &m) < 0) return(-1);
46714	  if (status != 0) *status = m.m2_i1;
46715	  return m.m_type;
46716	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/posix/_write.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

46800	#include <lib.h>
46801	#define write   _write
46802	#include <unistd.h>
46803	
46804	PUBLIC ssize_t write(fd, buffer, nbytes)
46805	int fd;
46806	_CONST void *buffer;
46807	size_t nbytes;
46808	{
46809	  message m;
46810	
46811	  m.m1_i1 = fd;
46812	  m.m1_i2 = nbytes;
46813	  m.m1_p1 = (char *) buffer;
46814	  return(_syscall(FS, WRITE, &m));
46815	}
.Op 427 src/lib/stdio/loc_incl.h

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/loc_incl.h	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

46900	/*
46901	 * loc_incl.h - local include file for stdio library
46902	 */
46903	/* $Header: loc_incl.h,v 1.5 91/06/10 17:07:18 ceriel Exp $ */
46904	
46905	#include        <stdio.h>
46906	
46907	#define io_testflag(p,x)        ((p)->_flags & (x))
46908	
46909	#include        <stdarg.h>
46910	
46911	#ifdef _ANSI
46912	int _doprnt(const char *format, va_list ap, FILE *stream);
46913	int _doscan(FILE * stream, const char *format, va_list ap);
46914	char *_i_compute(unsigned long val, int base, char *s, int nrdigits);
46915	char *_f_print(va_list *ap, int flags, char *s, char c, int precision);
46916	void __cleanup(void);
46917	
46918	FILE *popen(const char *command, const char *type);
46919	FILE *fdopen(int fd, const char *mode);
46920	
46921	#ifndef NOFLOAT
46922	char *_ecvt(long double value, int ndigit, int *decpt, int *sign);
46923	char *_fcvt(long double value, int ndigit, int *decpt, int *sign);
46924	#endif  /* NOFLOAT */
46925	#endif
46926	
46927	#define FL_LJUST        0x0001          /* left-justify field */
46928	#define FL_SIGN         0x0002          /* sign in signed conversions */
46929	#define FL_SPACE        0x0004          /* space in signed conversions */
46930	#define FL_ALT          0x0008          /* alternate form */
46931	#define FL_ZEROFILL     0x0010          /* fill with zero's */
46932	#define FL_SHORT        0x0020          /* optional h */
46933	#define FL_LONG         0x0040          /* optional l */
46934	#define FL_LONGDOUBLE   0x0080          /* optional L */
46935	#define FL_WIDTHSPEC    0x0100          /* field width is specified */
46936	#define FL_PRECSPEC     0x0200          /* precision is specified */
46937	#define FL_SIGNEDCONV   0x0400          /* may contain a sign */
46938	#define FL_NOASSIGN     0x0800          /* do not assign (in scanf) */
46939	#define FL_NOMORE       0x1000          /* all flags collected */
.Ep 428 src/lib/stdio/clearerr.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/clearerr.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

47000	/*
47001	 * clearerr.c - clear error and end-of-file indicators of a stream
47002	 */
47003	/* $Header: clearerr.c,v 1.2 91/01/03 14:23:54 ceriel Exp $ */
47004	
47005	#include        <stdio.h>
47006	
47007	void
47008	(clearerr)(FILE *stream)
47009	{
47010	        clearerr(stream);
47011	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/data.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

47100	/*
47101	 * data.c - this is the initialization for the standard streams
47102	 */
47103	/* $Header: data.c,v 1.3 90/04/09 15:35:09 eck Exp $ */
47104	
47105	#include        <stdio.h>
47106	
47107	struct __iobuf __stdin = {
47108	        0, 0, _IOREAD, 0,
47109	        (unsigned char *)NULL, (unsigned char *)NULL, 
47110	};
47111	
47112	struct __iobuf __stdout = {
47113	        0, 1, _IOWRITE, 0,
47114	        (unsigned char *)NULL, (unsigned char *)NULL, 
47115	};
47116	
47117	struct __iobuf __stderr = {
47118	        0, 2, _IOWRITE | _IOLBF, 0,
47119	        (unsigned char *)NULL, (unsigned char *)NULL, 
47120	};
47121	
47122	FILE *__iotab[FOPEN_MAX] = {
47123	        &__stdin,
47124	        &__stdout,
47125	        &__stderr,
47126	        0
47127	};
.Op 429 src/lib/stdio/doprnt.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/doprnt.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

47200	/*
47201	 * doprnt.c - print formatted output
47202	 */
47203	/* $Header: doprnt.c,v 1.8 91/06/10 17:06:53 ceriel Exp $ */
47204	
47205	#include        <ctype.h>
47206	#include        <stdio.h>
47207	#include        <stdarg.h>
47208	#include        <string.h>
47209	#include        "loc_incl.h"
47210	
47211	/* gnum() is used to get the width and precision fields of a format. */
47212	static const char *
47213	gnum(register const char *f, int *ip, va_list *app)
47214	{
47215	        register int    i, c;
47216	
47217	        if (*f == '*') {
47218	                *ip = va_arg((*app), int);
47219	                f++;
47220	        } else {
47221	                i = 0;
47222	                while ((c = *f - '0') >= 0 && c <= 9) {
47223	                        i = i*10 + c;
47224	                        f++;
47225	                }
47226	                *ip = i;
47227	        }
47228	        return f;
47229	}
	
47231	#if     _EM_WSIZE == _EM_PSIZE
47232	#define set_pointer(flags)                              /* nothing */
47233	#elif   _EM_LSIZE == _EM_PSIZE
47234	#define set_pointer(flags)      (flags |= FL_LONG)
47235	#else
47236	#error garbage pointer size
47237	#define set_pointer(flags)              /* compilation might continue */
47238	#endif
47239	
47240	/* print an ordinal number */
47241	static char *
47242	o_print(va_list *ap, int flags, char *s, char c, int precision, int is_signed)
47243	{
47244	        long signed_val;
47245	        unsigned long unsigned_val;
47246	        char *old_s = s;
47247	        int base;
47248	
47249	        switch (flags & (FL_SHORT | FL_LONG)) {
47250	        case FL_SHORT:
47251	                if (is_signed) {
47252	                        signed_val = (short) va_arg(*ap, int);
47253	                } else {
47254	                        unsigned_val = (unsigned short) va_arg(*ap, unsigned);
.Ep 430 src/lib/stdio/doprnt.c
47255	                }
47256	                break;
47257	        case FL_LONG:
47258	                if (is_signed) {
47259	                        signed_val = va_arg(*ap, long);
47260	                } else {
47261	                        unsigned_val = va_arg(*ap, unsigned long);
47262	                }
47263	                break;
47264	        default:
47265	                if (is_signed) {
47266	                        signed_val = va_arg(*ap, int);
47267	                } else {
47268	                        unsigned_val = va_arg(*ap, unsigned int);
47269	                }
47270	                break;
47271	        }
47272	
47273	        if (is_signed) {
47274	                if (signed_val < 0) {
47275	                        *s++ = '-';
47276	                        signed_val = -signed_val;
47277	                } else if (flags & FL_SIGN) *s++ = '+';
47278	                else if (flags & FL_SPACE) *s++ = ' ';
47279	                unsigned_val = signed_val;
47280	        }
47281	        if ((flags & FL_ALT) && (c == 'o')) *s++ = '0';
47282	        if (!unsigned_val) {
47283	                 if (!precision)
47284	                        return s;
47285	        } else if (((flags & FL_ALT) && (c == 'x' || c == 'X'))
47286	                    || c == 'p') {
47287	                *s++ = '0';
47288	                *s++ = (c == 'X' ? 'X' : 'x');
47289	        }
47290	
47291	        switch (c) {
47292	        case 'b':       base = 2;       break;
47293	        case 'o':       base = 8;       break;
47294	        case 'd':
47295	        case 'i':
47296	        case 'u':       base = 10;      break;
47297	        case 'x':
47298	        case 'X':
47299	        case 'p':       base = 16;      break;
47300	        }
47301	
47302	        s = _i_compute(unsigned_val, base, s, precision);
47303	
47304	        if (c == 'X')
47305	                while (old_s != s) {
47306	                        *old_s = toupper(*old_s);
47307	                        old_s++;
47308	                }
47309	
47310	        return s;
47311	}
	
47313	int
47314	_doprnt(register const char *fmt, va_list ap, FILE *stream)
.Op 431 src/lib/stdio/doprnt.c
47315	{
47316	        register char   *s;
47317	        register int    j;
47318	        int             i, c, width, precision, zfill, flags, between_fill;
47319	        int             nrchars=0;
47320	        const char      *oldfmt;
47321	        char            *s1, buf[1025];
47322	
47323	        while (c = *fmt++) {
47324	                if (c != '%') {
47325	#ifdef  CPM
47326	                        if (c == '\n') {
47327	                                if (putc('\r', stream) == EOF)
47328	                                        return nrchars ? -nrchars : -1;
47329	                                nrchars++;
47330	                        }
47331	#endif
47332	                        if (putc(c, stream) == EOF)
47333	                                return nrchars ? -nrchars : -1;
47334	                        nrchars++;
47335	                        continue;
47336	                }
47337	                flags = 0;
47338	                do {
47339	                        switch(*fmt) {
47340	                        case '-':       flags |= FL_LJUST;      break;
47341	                        case '+':       flags |= FL_SIGN;       break;
47342	                        case ' ':       flags |= FL_SPACE;      break;
47343	                        case '#':       flags |= FL_ALT;        break;
47344	                        case '0':       flags |= FL_ZEROFILL;   break;
47345	                        default:        flags |= FL_NOMORE;     continue;
47346	                        }
47347	                        fmt++;
47348	                } while(!(flags & FL_NOMORE));
47349	
47350	                oldfmt = fmt;
47351	                fmt = gnum(fmt, &width, &ap);
47352	                if (fmt != oldfmt) flags |= FL_WIDTHSPEC;
47353	
47354	                if (*fmt == '.') {
47355	                        fmt++; oldfmt = fmt;
47356	                        fmt = gnum(fmt, &precision, &ap);
47357	                        if (precision >= 0) flags |= FL_PRECSPEC;
47358	                }
47359	
47360	                if ((flags & FL_WIDTHSPEC) && width < 0) {
47361	                        width = -width;
47362	                        flags |= FL_LJUST;
47363	                }
47364	                if (!(flags & FL_WIDTHSPEC)) width = 0;
47365	
47366	                if (flags & FL_SIGN) flags &= ~FL_SPACE;
47367	
47368	                if (flags & FL_LJUST) flags &= ~FL_ZEROFILL;
47369	
47370	
47371	                s = s1 = buf;
47372	
47373	                switch (*fmt) {
47374	                case 'h':       flags |= FL_SHORT; fmt++; break;
.Ep 432 src/lib/stdio/doprnt.c
47375	                case 'l':       flags |= FL_LONG; fmt++; break;
47376	                case 'L':       flags |= FL_LONGDOUBLE; fmt++; break;
47377	                }
47378	
47379	                switch (c = *fmt++) {
47380	                default:
47381	#ifdef  CPM
47382	                        if (c == '\n') {
47383	                                if (putc('\r', stream) == EOF)
47384	                                        return nrchars ? -nrchars : -1;
47385	                                nrchars++;
47386	                        }
47387	#endif
47388	                        if (putc(c, stream) == EOF)
47389	                                return nrchars ? -nrchars : -1;
47390	                        nrchars++;
47391	                        continue;
47392	                case 'n':
47393	                        if (flags & FL_SHORT)
47394	                                *va_arg(ap, short *) = (short) nrchars;
47395	                        else if (flags & FL_LONG)
47396	                                *va_arg(ap, long *) = (long) nrchars;
47397	                        else
47398	                                *va_arg(ap, int *) = (int) nrchars;
47399	                        continue;
47400	                case 's':
47401	                        s1 = va_arg(ap, char *);
47402	                        if (s1 == NULL)
47403	                                s1 = "(null)";
47404	                        s = s1;
47405	                        while (precision || !(flags & FL_PRECSPEC)) {
47406	                                if (*s == '\0')
47407	                                        break;
47408	                                s++;
47409	                                precision--;
47410	                        }
47411	                        break;
47412	                case 'p':
47413	                        set_pointer(flags);
47414	                        /* fallthrough */
47415	                case 'b':
47416	                case 'o':
47417	                case 'u':
47418	                case 'x':
47419	                case 'X':
47420	                        if (!(flags & FL_PRECSPEC)) precision = 1;
47421	                        else if (c != 'p') flags &= ~FL_ZEROFILL;
47422	                        s = o_print(&ap, flags, s, c, precision, 0);
47423	                        break;
47424	                case 'd':
47425	                case 'i':
47426	                        flags |= FL_SIGNEDCONV;
47427	                        if (!(flags & FL_PRECSPEC)) precision = 1;
47428	                        else flags &= ~FL_ZEROFILL;
47429	                        s = o_print(&ap, flags, s, c, precision, 1);
47430	                        break;
47431	                case 'c':
47432	                        *s++ = va_arg(ap, int);
47433	                        break;
47434	#ifndef NOFLOAT
.Op 433 src/lib/stdio/doprnt.c
47435	                case 'G':
47436	                case 'g':
47437	                        if ((flags & FL_PRECSPEC) && (precision == 0))
47438	                                precision = 1;
47439	                case 'f':
47440	                case 'E':
47441	                case 'e':
47442	                        if (!(flags & FL_PRECSPEC)) 
47443	                                precision = 6;
47444	
47445	                        if (precision >= sizeof(buf))
47446	                                precision = sizeof(buf) - 1;
47447	
47448	                        flags |= FL_SIGNEDCONV;
47449	                        s = _f_print(&ap, flags, s, c, precision);
47450	                        break;
47451	#endif  /* NOFLOAT */
47452	                case 'r':
47453	                        ap = va_arg(ap, va_list);
47454	                        fmt = va_arg(ap, char *);
47455	                        continue;
47456	                }
47457	                zfill = ' ';
47458	                if (flags & FL_ZEROFILL) zfill = '0';
47459	                j = s - s1;
47460	
47461	                /* between_fill is true under the following conditions:
47462	                 * 1- the fill character is '0'
47463	                 * and
47464	                 * 2a- the number is of the form 0x... or 0X...
47465	                 * or
47466	                 * 2b- the number contains a sign or space
47467	                 */
47468	                between_fill = 0;
47469	                if ((flags & FL_ZEROFILL)
47470	                    && (((c == 'x' || c == 'X') && (flags & FL_ALT))
47471	                        || (c == 'p')
47472	                        || ((flags & FL_SIGNEDCONV)
47473	                            && ( *s1 == '+' || *s1 == '-' || *s1 == ' '))))
47474	                        between_fill++;
47475	
47476	                if ((i = width - j) > 0)
47477	                        if (!(flags & FL_LJUST)) {      /* right justify */
47478	                                nrchars += i;
47479	                                if (between_fill) {
47480	                                    if (flags & FL_SIGNEDCONV) {
47481	                                        j--; nrchars++;
47482	                                        if (putc(*s1++, stream) == EOF)
47483	                                                return nrchars ? -nrchars : -1;
47484	                                    } else {
47485	                                        j -= 2; nrchars += 2;
47486	                                        if ((putc(*s1++, stream) == EOF)
47487	                                            || (putc(*s1++, stream) == EOF))
47488	                                                return nrchars ? -nrchars : -1;
47489	                                    }
47490	                                }
47491	                                do {
47492	                                        if (putc(zfill, stream) == EOF)
47493	                                                return nrchars ? -nrchars : -1;
47494	                                } while (--i);
.Ep 434 src/lib/stdio/doprnt.c
47495	                        }
47496	
47497	                nrchars += j;
47498	                while (--j >= 0) {
47499	                        if (putc(*s1++, stream) == EOF)
47500	                                return nrchars ? -nrchars : -1;
47501	                }
47502	
47503	                if (i > 0) nrchars += i;
47504	                while (--i >= 0)
47505	                        if (putc(zfill, stream) == EOF)
47506	                                return nrchars ? -nrchars : -1;
47507	        }
47508	        return nrchars;
47509	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/doscan.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

47600	/*
47601	 * doscan.c - scan formatted input
47602	 */
47603	/* $Header: doscan.c,v 1.9 91/03/26 18:41:47 ceriel Exp $ */
47604	
47605	#include        <stdio.h>
47606	#include        <stdlib.h>
47607	#include        <ctype.h>
47608	#include        <stdarg.h>
47609	#include        "loc_incl.h"
47610	
47611	#if     _EM_WSIZE == _EM_PSIZE
47612	#define set_pointer(flags)                              /* nothing */
47613	#elif   _EM_LSIZE == _EM_PSIZE
47614	#define set_pointer(flags)      (flags |= FL_LONG)
47615	#else
47616	#error garbage pointer size
47617	#define set_pointer(flags)              /* compilation might continue */
47618	#endif
47619	
47620	#define NUMLEN  512
47621	#define NR_CHARS        256
47622	
47623	static char     Xtable[NR_CHARS];
47624	static char     inp_buf[NUMLEN];
47625	
47626	/* Collect a number of characters which constitite an ordinal number.
47627	 * When the type is 'i', the base can be 8, 10, or 16, depending on the
47628	 * first 1 or 2 characters. This means that the base must be adjusted
47629	 * according to the format of the number. At the end of the function, base
47630	 * is then set to 0, so strtol() will get the right argument.
47631	 */
47632	static char *
47633	o_collect(register int c, register FILE *stream, char type,
47634	                        unsigned int width, int *basep)
.Op 435 src/lib/stdio/doscan.c
47635	{
47636	        register char *bufp = inp_buf;
47637	        register int base;
47638	
47639	        switch (type) {
47640	        case 'i':       /* i means octal, decimal or hexadecimal */
47641	        case 'p':
47642	        case 'x':
47643	        case 'X':       base = 16;      break;
47644	        case 'd':
47645	        case 'u':       base = 10;      break;
47646	        case 'o':       base = 8;       break;
47647	        case 'b':       base = 2;       break;
47648	        }
47649	
47650	        if (c == '-' || c == '+') {
47651	                *bufp++ = c;
47652	                if (--width)
47653	                    c = getc(stream);
47654	        }
47655	
47656	        if (width && c == '0' && base == 16) {
47657	                *bufp++ = c;
47658	                if (--width)
47659	                        c = getc(stream);
47660	                if (c != 'x' && c != 'X') {
47661	                        if (type == 'i') base = 8;
47662	                }
47663	                else if (width) {
47664	                        *bufp++ = c;
47665	                        if (--width)
47666	                                c = getc(stream);
47667	                }
47668	        }
47669	        else if (type == 'i') base = 10;
47670	
47671	        while (width) {
47672	                if (((base == 10) && isdigit(c))
47673	                    || ((base == 16) && isxdigit(c))
47674	                    || ((base == 8) && isdigit(c) && (c < '8'))
47675	                    || ((base == 2) && isdigit(c) && (c < '2'))) {
47676	                        *bufp++ = c;
47677	                        if (--width)
47678	                                c = getc(stream);
47679	                }
47680	                else break;
47681	        }
47682	
47683	        if (width && c != EOF) ungetc(c, stream);
47684	        if (type == 'i') base = 0;
47685	        *basep = base;
47686	        *bufp = '\0';
47687	        return bufp - 1;
47688	}
	
47690	#ifndef NOFLOAT
47691	/* The function f_collect() reads a string that has the format of a
47692	 * floating-point number. The function returns as soon as a format-error
47693	 * is encountered, leaving the offending character in the input. This means
47694	 * that 1.el leaves the 'l' in the input queue. Since all detection of
.Ep 436 src/lib/stdio/doscan.c
47695	 * format errors is done here, _doscan() doesn't call strtod() when it's
47696	 * not necessary, although the use of the width field can cause incomplete
47697	 * numbers to be passed to strtod(). (e.g. 1.3e+)
47698	 */
47699	static char *
47700	f_collect(register int c, register FILE *stream, register unsigned int width)
47701	{
47702	        register char *bufp = inp_buf;
47703	        int digit_seen = 0;
47704	
47705	        if (c == '-' || c == '+') {
47706	                *bufp++ = c;
47707	                if (--width)
47708	                        c = getc(stream);
47709	        }
47710	
47711	        while (width && isdigit(c)) {
47712	                digit_seen++;
47713	                *bufp++ = c;
47714	                if (--width)
47715	                        c = getc(stream);
47716	        }
47717	        if (width && c == '.') {
47718	                *bufp++ = c;
47719	                if(--width)
47720	                        c = getc(stream);
47721	                while (width && isdigit(c)) {
47722	                        digit_seen++;
47723	                        *bufp++ = c;
47724	                        if (--width)
47725	                                c = getc(stream);
47726	                }
47727	        }
47728	
47729	        if (!digit_seen) {
47730	                if (width && c != EOF) ungetc(c, stream);
47731	                return inp_buf - 1;
47732	        }
47733	        else digit_seen = 0;
47734	
47735	        if (width && (c == 'e' || c == 'E')) {
47736	                *bufp++ = c;
47737	                if (--width)
47738	                        c = getc(stream);
47739	                if (width && (c == '+' || c == '-')) {
47740	                        *bufp++ = c;
47741	                        if (--width)
47742	                                c = getc(stream);
47743	                }
47744	                while (width && isdigit(c)) {
47745	                        digit_seen++;
47746	                        *bufp++ = c;
47747	                        if (--width)
47748	                                c = getc(stream);
47749	                }
47750	                if (!digit_seen) {
47751	                        if (width && c != EOF) ungetc(c,stream);
47752	                        return inp_buf - 1;
47753	                }
47754	        }
.Op 437 src/lib/stdio/doscan.c
47755	
47756	        if (width && c != EOF) ungetc(c, stream);
47757	        *bufp = '\0';
47758	        return bufp - 1;
47759	}
47760	#endif  /* NOFLOAT */
47761	
47762	
47763	/*
47764	 * the routine that does the scanning 
47765	 */
47766	
47767	int
47768	_doscan(register FILE *stream, const char *format, va_list ap)
47769	{
47770	        int             done = 0;       /* number of items done */
47771	        int             nrchars = 0;    /* number of characters read */
47772	        int             conv = 0;       /* # of conversions */
47773	        int             base;           /* conversion base */
47774	        unsigned long   val;            /* an integer value */
47775	        register char   *str;           /* temporary pointer */
47776	        char            *tmp_string;    /* ditto */
47777	        unsigned        width = 0;      /* width of field */
47778	        int             flags;          /* some flags */
47779	        int             reverse;        /* reverse the checking in [...] */
47780	        int             kind;
47781	        register int    ic = EOF;       /* the input character */
47782	#ifndef NOFLOAT
47783	        long double     ld_val;
47784	#endif
47785	
47786	        if (!*format) return 0;
47787	
47788	        while (1) {
47789	                if (isspace(*format)) {
47790	                        while (isspace(*format))
47791	                                format++;       /* skip whitespace */
47792	                        ic = getc(stream);
47793	                        nrchars++;
47794	                        while (isspace (ic)) {
47795	                                ic = getc(stream);
47796	                                nrchars++;
47797	                        }
47798	                        if (ic != EOF) ungetc(ic,stream);
47799	                        nrchars--;
47800	                }
47801	                if (!*format) break;    /* end of format */
47802	
47803	                if (*format != '%') {
47804	                        ic = getc(stream);
47805	                        nrchars++;
47806	                        if (ic != *format++) break;     /* error */
47807	                        continue;
47808	                }
47809	                format++;
47810	                if (*format == '%') {
47811	                        ic = getc(stream);
47812	                        nrchars++;
47813	                        if (ic == '%') {
47814	                                format++;
.Ep 438 src/lib/stdio/doscan.c
47815	                                continue;
47816	                        }
47817	                        else break;
47818	                }
47819	                flags = 0;
47820	                if (*format == '*') {
47821	                        format++;
47822	                        flags |= FL_NOASSIGN;
47823	                }
47824	                if (isdigit (*format)) {
47825	                        flags |= FL_WIDTHSPEC;
47826	                        for (width = 0; isdigit (*format);)
47827	                                width = width * 10 + *format++ - '0';
47828	                }
47829	
47830	                switch (*format) {
47831	                case 'h': flags |= FL_SHORT; format++; break;
47832	                case 'l': flags |= FL_LONG; format++; break;
47833	                case 'L': flags |= FL_LONGDOUBLE; format++; break;
47834	                }
47835	                kind = *format;
47836	                if ((kind != 'c') && (kind != '[') && (kind != 'n')) {
47837	                        do {
47838	                                ic = getc(stream);
47839	                                nrchars++;
47840	                        } while (isspace(ic));
47841	                        if (ic == EOF) break;           /* outer while */
47842	                } else if (kind != 'n') {               /* %c or %[ */
47843	                        ic = getc(stream);
47844	                        if (ic == EOF) break;           /* outer while */
47845	                        nrchars++;
47846	                }
47847	                switch (kind) {
47848	                default:
47849	                        /* not recognized, like %q */
47850	                        return conv || (ic != EOF) ? done : EOF;
47851	                        break;
47852	                case 'n':
47853	                        if (!(flags & FL_NOASSIGN)) {   /* silly, though */
47854	                                if (flags & FL_SHORT)
47855	                                        *va_arg(ap, short *) = (short) nrchars;
47856	                                else if (flags & FL_LONG)
47857	                                        *va_arg(ap, long *) = (long) nrchars;
47858	                                else
47859	                                        *va_arg(ap, int *) = (int) nrchars;
47860	                        }
47861	                        break;
47862	                case 'p':               /* pointer */
47863	                        set_pointer(flags);
47864	                        /* fallthrough */
47865	                case 'b':               /* binary */
47866	                case 'd':               /* decimal */
47867	                case 'i':               /* general integer */
47868	                case 'o':               /* octal */
47869	                case 'u':               /* unsigned */
47870	                case 'x':               /* hexadecimal */
47871	                case 'X':               /* ditto */
47872	                        if (!(flags & FL_WIDTHSPEC) || width > NUMLEN)
47873	                                width = NUMLEN;
47874	                        if (!width) return done;
.Op 439 src/lib/stdio/doscan.c
47875	
47876	                        str = o_collect(ic, stream, kind, width, &base);
47877	                        if (str < inp_buf
47878	                            || (str == inp_buf
47879	                                    && (*str == '-'
47880	                                        || *str == '+'))) return done;
47881	
47882	                        /*
47883	                         * Although the length of the number is str-inp_buf+1
47884	                         * we don't add the 1 since we counted it already
47885	                         */
47886	                        nrchars += str - inp_buf;
47887	
47888	                        if (!(flags & FL_NOASSIGN)) {
47889	                                if (kind == 'd' || kind == 'i')
47890	                                    val = strtol(inp_buf, &tmp_string, base);
47891	                                else
47892	                                    val = strtoul(inp_buf, &tmp_string, base);
47893	                                if (flags & FL_LONG)
47894	                                        *va_arg(ap, unsigned long *) = (unsigned long) val;
47895	                                else if (flags & FL_SHORT)
47896	                                        *va_arg(ap, unsigned short *) = (unsigned short) val;
47897	                                else
47898	                                        *va_arg(ap, unsigned *) = (unsigned) val;
47899	                        }
47900	                        break;
47901	                case 'c':
47902	                        if (!(flags & FL_WIDTHSPEC))
47903	                                width = 1;
47904	                        if (!(flags & FL_NOASSIGN))
47905	                                str = va_arg(ap, char *);
47906	                        if (!width) return done;
47907	
47908	                        while (width && ic != EOF) {
47909	                                if (!(flags & FL_NOASSIGN))
47910	                                        *str++ = (char) ic;
47911	                                if (--width) {
47912	                                        ic = getc(stream);
47913	                                        nrchars++;
47914	                                }
47915	                        }
47916	
47917	                        if (width) {
47918	                                if (ic != EOF) ungetc(ic,stream);
47919	                                nrchars--;
47920	                        }
47921	                        break;
47922	                case 's':
47923	                        if (!(flags & FL_WIDTHSPEC))
47924	                                width = 0xffff;
47925	                        if (!(flags & FL_NOASSIGN))
47926	                                str = va_arg(ap, char *);
47927	                        if (!width) return done;
47928	
47929	                        while (width && ic != EOF && !isspace(ic)) {
47930	                                if (!(flags & FL_NOASSIGN))
47931	                                        *str++ = (char) ic;
47932	                                if (--width) {
47933	                                        ic = getc(stream);
47934	                                        nrchars++;
.Ep 440 src/lib/stdio/doscan.c
47935	                                }
47936	                        }
47937	                        /* terminate the string */
47938	                        if (!(flags & FL_NOASSIGN))
47939	                                *str = '\0';    
47940	                        if (width) {
47941	                                if (ic != EOF) ungetc(ic,stream);
47942	                                nrchars--;
47943	                        }
47944	                        break;
47945	                case '[':
47946	                        if (!(flags & FL_WIDTHSPEC))
47947	                                width = 0xffff;
47948	                        if (!width) return done;
47949	
47950	                        if ( *++format == '^' ) {
47951	                                reverse = 1;
47952	                                format++;
47953	                        } else
47954	                                reverse = 0;
47955	
47956	                        for (str = Xtable; str < &Xtable[NR_CHARS]
47957	                                                        ; str++)
47958	                                *str = 0;
47959	
47960	                        if (*format == ']') Xtable[*format++] = 1;
47961	
47962	                        while (*format && *format != ']') {
47963	                                Xtable[*format++] = 1;
47964	                                if (*format == '-') {
47965	                                        format++;
47966	                                        if (*format
47967	                                            && *format != ']'
47968	                                            && *(format) >= *(format -2)) {
47969	                                                int c;
47970	
47971	                                                for( c = *(format -2) + 1
47972	                                                    ; c <= *format ; c++)
47973	                                                        Xtable[c] = 1;
47974	                                                format++;
47975	                                        }
47976	                                        else Xtable['-'] = 1;
47977	                                }
47978	                        }
47979	                        if (!*format) return done;
47980	                        
47981	                        if (!(Xtable[ic] ^ reverse)) return done;
47982	
47983	                        if (!(flags & FL_NOASSIGN))
47984	                                str = va_arg(ap, char *);
47985	
47986	                        do {
47987	                                if (!(flags & FL_NOASSIGN))
47988	                                        *str++ = (char) ic;
47989	                                if (--width) {
47990	                                        ic = getc(stream);
47991	                                        nrchars++;
47992	                                }
47993	                        } while (width && ic != EOF && (Xtable[ic] ^ reverse));
47994	
.Op 441 src/lib/stdio/doscan.c
47995	                        if (width) {
47996	                                if (ic != EOF) ungetc(ic, stream);
47997	                                nrchars--;
47998	                        }
47999	                        if (!(flags & FL_NOASSIGN)) {   /* terminate string */
48000	                                *str = '\0';    
48001	                        }
48002	                        break;
48003	#ifndef NOFLOAT
48004	                case 'e':
48005	                case 'E':
48006	                case 'f':
48007	                case 'g':
48008	                case 'G':
48009	                        if (!(flags & FL_WIDTHSPEC) || width > NUMLEN)
48010	                                width = NUMLEN;
48011	
48012	                        if (!width) return done;
48013	                        str = f_collect(ic, stream, width);
48014	
48015	                        if (str < inp_buf
48016	                            || (str == inp_buf
48017	                                && (*str == '-'
48018	                                    || *str == '+'))) return done;
48019	
48020	                        /*
48021	                         * Although the length of the number is str-inp_buf+1
48022	                         * we don't add the 1 since we counted it already
48023	                         */
48024	                        nrchars += str - inp_buf;
48025	
48026	                        if (!(flags & FL_NOASSIGN)) {
48027	                                ld_val = strtod(inp_buf, &tmp_string);
48028	                                if (flags & FL_LONGDOUBLE)
48029	                                        *va_arg(ap, long double *) = (long double) ld_val;
48030	                                else
48031	                                    if (flags & FL_LONG)
48032	                                        *va_arg(ap, double *) = (double) ld_val;
48033	                                else
48034	                                        *va_arg(ap, float *) = (float) ld_val;
48035	                        }
48036	                        break;
48037	#endif
48038	                }               /* end switch */
48039	                conv++;
48040	                if (!(flags & FL_NOASSIGN) && kind != 'n') done++;
48041	                format++;
48042	        }
48043	        return conv || (ic != EOF) ? done : EOF;
48044	}
.Ep 442 src/lib/stdio/ecvt.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/ecvt.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

48100	/* $Header: ecvt.c,v 1.4 90/02/27 16:47:28 eck Exp $ */
48101	
48102	#ifndef NOFLOAT
48103	
48104	#include        "../ansi/ext_fmt.h"
48105	void _dbl_ext_cvt(double value, struct EXTEND *e);
48106	char *_ext_str_cvt(struct EXTEND *e, int ndigit, int *decpt, int * sign, int ecvtflag);
48107	
48108	static char *
48109	cvt(long double value, int ndigit, int *decpt, int *sign, int ecvtflag)
48110	{
48111	        struct EXTEND e;
48112	
48113	        _dbl_ext_cvt(value, &e);
48114	        return _ext_str_cvt(&e, ndigit, decpt, sign, ecvtflag);
48115	}
	
48117	char *
48118	_ecvt(long double value, int ndigit, int *decpt, int *sign)
48119	{
48120	
48121	        return cvt(value, ndigit, decpt, sign, 1);
48122	}
	
48124	char *
48125	_fcvt(long double value, int ndigit, int *decpt, int *sign)
48126	{
48127	        return cvt(value, ndigit, decpt, sign, 0);
48128	}
	
48130	#endif  /* NOFLOAT */





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/fclose.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

48200	/*
48201	 * fclose.c - flush a stream and close the file
48202	 */
48203	/* $Header: fclose.c,v 1.4 90/01/22 11:10:54 eck Exp $ */
48204	
48205	#include        <stdio.h>
48206	#include        <stdlib.h>
48207	#include        "loc_incl.h"
48208	
48209	int _close(int d);
48210	
48211	int
48212	fclose(FILE *fp)
48213	{
48214	        register int i, retval = 0;
.Op 443 src/lib/stdio/fclose.c
48215	
48216	        for (i=0; i<FOPEN_MAX; i++)
48217	                if (fp == __iotab[i]) {
48218	                        __iotab[i] = 0;
48219	                        break;
48220	                }
48221	        if (i >= FOPEN_MAX)
48222	                return EOF;
48223	        if (fflush(fp)) retval = EOF;
48224	        if (_close(fileno(fp))) retval = EOF;
48225	        if ( io_testflag(fp,_IOMYBUF) && fp->_buf )
48226	                free((void *)fp->_buf);
48227	        if (fp != stdin && fp != stdout && fp != stderr)
48228	                free((void *)fp);
48229	        return retval;
48230	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/feof.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

48300	/*
48301	 * feof.c - test if eof on a stream occurred
48302	 */
48303	/* $Header: feof.c,v 1.2 89/12/18 15:00:39 eck Exp $ */
48304	
48305	#include        <stdio.h>
48306	
48307	int
48308	(feof)(FILE *stream)
48309	{
48310	        return feof(stream);
48311	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/ferror.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

48400	/*
48401	 * ferror .c - test if an error on a stream occurred
48402	 */
48403	/* $Header: ferror.c,v 1.2 89/12/18 15:00:47 eck Exp $ */
48404	
48405	#include        <stdio.h>
48406	
48407	int
48408	(ferror)(FILE *stream)
48409	{
48410	        return ferror(stream);
48411	}
.Ep 444 src/lib/stdio/fflush.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/fflush.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

48500	/*
48501	 * fflush.c - flush stream(s)
48502	 */
48503	/* $Header: fflush.c,v 1.6 90/04/04 15:52:01 eck Exp $ */
48504	
48505	#include        <sys/types.h>
48506	#include        <stdio.h>
48507	#include        "loc_incl.h"
48508	
48509	ssize_t _write(int d, const char *buf, size_t nbytes);
48510	off_t _lseek(int fildes, off_t offset, int whence);
48511	
48512	int
48513	fflush(FILE *stream)
48514	{
48515	        int count, c1, i, retval = 0;
48516	
48517	        if (!stream) {
48518	            for(i= 0; i < FOPEN_MAX; i++)
48519	                if (__iotab[i] && fflush(__iotab[i]))
48520	                        retval = EOF;
48521	            return retval;
48522	        }
48523	
48524	        if (!stream->_buf
48525	            || (!io_testflag(stream, _IOREADING)
48526	                && !io_testflag(stream, _IOWRITING)))
48527	                return 0;
48528	        if (io_testflag(stream, _IOREADING)) {
48529	                /* (void) fseek(stream, 0L, SEEK_CUR); */
48530	                int adjust = 0;
48531	                if (stream->_buf && !io_testflag(stream,_IONBF))
48532	                        adjust = stream->_count;
48533	                stream->_count = 0;
48534	                _lseek(fileno(stream), (off_t) adjust, SEEK_CUR);
48535	                if (io_testflag(stream, _IOWRITE))
48536	                        stream->_flags &= ~(_IOREADING | _IOWRITING);
48537	                stream->_ptr = stream->_buf;
48538	                return 0;
48539	        } else if (io_testflag(stream, _IONBF)) return 0;
48540	
48541	        if (io_testflag(stream, _IOREAD))               /* "a" or "+" mode */
48542	                stream->_flags &= ~_IOWRITING;
48543	
48544	        count = stream->_ptr - stream->_buf;
48545	        stream->_ptr = stream->_buf;
48546	
48547	        if ( count <= 0 )
48548	                return 0;
48549	
48550	        if (io_testflag(stream, _IOAPPEND)) {
48551	                if (_lseek(fileno(stream), 0L, SEEK_END) == -1) {
48552	                        stream->_flags |= _IOERR;
48553	                        return EOF;
48554	                }
.Op 445 src/lib/stdio/fflush.c
48555	        }
48556	        c1 = _write(stream->_fd, (char *)stream->_buf, count);
48557	
48558	        stream->_count = 0;
48559	
48560	        if ( count == c1 )
48561	                return 0;
48562	
48563	        stream->_flags |= _IOERR;
48564	        return EOF; 
48565	}
	
48567	void
48568	__cleanup(void)
48569	{
48570	        register int i;
48571	
48572	        for(i= 0; i < FOPEN_MAX; i++)
48573	                if (__iotab[i] && io_testflag(__iotab[i], _IOWRITING))
48574	                        (void) fflush(__iotab[i]);
48575	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/fgetc.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

48600	/*
48601	 * fgetc - get an unsigned character and return it as an int
48602	 */
48603	/* $Header: fgetc.c,v 1.1 89/05/30 13:27:35 eck Exp $ */
48604	
48605	#include        <stdio.h>
48606	
48607	int
48608	fgetc(FILE *stream)
48609	{
48610	        return getc(stream);
48611	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/fgetpos.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

48700	/*
48701	 * fgetpos.c - get the position in the file
48702	 */
48703	/* $Header: fgetpos.c,v 1.2 89/12/18 15:01:03 eck Exp $ */
48704	
48705	#include        <stdio.h>
48706	
48707	int
48708	fgetpos(FILE *stream, fpos_t *pos)
48709	{
.Ep 446 src/lib/stdio/fgetpos.c
48710	        *pos = ftell(stream);
48711	        if (*pos == -1) return -1;
48712	        return 0;
48713	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/fgets.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

48800	/*
48801	 * fgets.c - get a string from a file
48802	 */
48803	/* $Header: fgets.c,v 1.3 89/12/18 15:01:11 eck Exp $ */
48804	
48805	#include        <stdio.h>
48806	
48807	char *
48808	fgets(char *s, register int n, register FILE *stream)
48809	{
48810	        register int ch;
48811	        register char *ptr;
48812	
48813	        ptr = s;
48814	        while (--n > 0 && (ch = getc(stream)) != EOF) {
48815	                *ptr++ = ch;
48816	                if ( ch == '\n')
48817	                        break;
48818	        }
48819	        if (ch == EOF) {
48820	                if (feof(stream)) {
48821	                        if (ptr == s) return NULL;
48822	                } else return NULL;
48823	        }
48824	        *ptr = '\0';
48825	        return s;
48826	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/fileno.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

48900	/*
48901	 * fileno .c - map a stream to a file descriptor
48902	 */
48903	/* $Header: fileno.c,v 1.1 89/12/18 14:59:31 eck Exp $ */
48904	
48905	#include        <stdio.h>
48906	
48907	int
48908	(fileno)(FILE *stream)
48909	{
48910	        return stream->_fd;
48911	}
.Op 447 src/lib/stdio/fillbuf.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/fillbuf.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

49000	/*
49001	 * fillbuf.c - fill a buffer
49002	 */
49003	/* $Header: fillbuf.c,v 1.5 90/06/21 11:13:23 eck Exp $ */
49004	
49005	#if     defined(_POSIX_SOURCE)
49006	#include        <sys/types.h>
49007	#endif
49008	#include        <stdio.h>
49009	#include        <stdlib.h>
49010	#include        "loc_incl.h"
49011	
49012	ssize_t _read(ssize_t d, char *buf, size_t nbytes);
49013	
49014	int
49015	__fillbuf(register FILE *stream)
49016	{
49017	        static unsigned char ch[FOPEN_MAX];
49018	        register int i;
49019	
49020	        stream->_count = 0;
49021	        if (fileno(stream) < 0) return EOF;
49022	        if (io_testflag(stream, (_IOEOF | _IOERR ))) return EOF; 
49023	        if (!io_testflag(stream, _IOREAD))
49024	             { stream->_flags |= _IOERR; return EOF; }
49025	        if (io_testflag(stream, _IOWRITING))
49026	             { stream->_flags |= _IOERR; return EOF; }
49027	
49028	        if (!io_testflag(stream, _IOREADING))
49029	                stream->_flags |= _IOREADING;
49030	        
49031	        if (!io_testflag(stream, _IONBF) && !stream->_buf) {
49032	                stream->_buf = (unsigned char *) malloc(BUFSIZ);
49033	                if (!stream->_buf) {
49034	                        stream->_flags |= _IONBF;
49035	                }
49036	                else {
49037	                        stream->_flags |= _IOMYBUF;
49038	                        stream->_bufsiz = BUFSIZ;
49039	                }
49040	        }
49041	
49042	        /* flush line-buffered output when filling an input buffer */
49043	        for (i = 0; i < FOPEN_MAX; i++) {
49044	                if (__iotab[i] && io_testflag(__iotab[i], _IOLBF))
49045	                        if (io_testflag(__iotab[i], _IOWRITING))
49046	                                (void) fflush(__iotab[i]);
49047	        }
49048	
49049	        if (!stream->_buf) {
49050	                stream->_buf = &ch[fileno(stream)];
49051	                stream->_bufsiz = 1;
49052	        }
49053	        stream->_ptr = stream->_buf;
49054	        stream->_count = _read(stream->_fd, (char *)stream->_buf, stream->_bufsiz);
.Ep 448 src/lib/stdio/fillbuf.c
49055	
49056	        if (stream->_count <= 0){
49057	                if (stream->_count == 0) {
49058	                        stream->_flags |= _IOEOF;
49059	                }
49060	                else 
49061	                        stream->_flags |= _IOERR;
49062	
49063	                return EOF;
49064	        }
49065	        stream->_count--;
49066	
49067	        return *stream->_ptr++;
49068	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/flushbuf.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

49100	/*
49101	 * flushbuf.c - flush a buffer
49102	 */
49103	/* $Header: flushbuf.c,v 1.6 91/06/10 17:07:10 ceriel Exp $ */
49104	
49105	#include        <stdio.h>
49106	#include        <stdlib.h>
49107	#include        "loc_incl.h"
49108	
49109	#include        <sys/types.h>
49110	
49111	off_t _lseek(int fildes, off_t offset, int whence);
49112	ssize_t _write(int d, const char *buf, size_t nbytes);
49113	int _isatty(int d);
49114	extern void (*_clean)(void);
49115	
49116	static int
49117	do_write(int d, char *buf, int nbytes)
49118	{
49119	        int c;
49120	
49121	        /* POSIX actually allows write() to return a positive value less
49122	           than nbytes, so loop ...
49123	        */
49124	        while ((c = _write(d, buf, nbytes)) > 0 && c < nbytes) {
49125	                nbytes -= c;
49126	                buf += c;
49127	        }
49128	        return c > 0;
49129	}
	
49131	int
49132	__flushbuf(int c, FILE * stream)
49133	{
49134	        _clean = __cleanup;
49135	        if (fileno(stream) < 0) return EOF;
49136	        if (!io_testflag(stream, _IOWRITE)) return EOF;
49137	        if (io_testflag(stream, _IOREADING) && !feof(stream)) return EOF;
49138	
49139	        stream->_flags &= ~_IOREADING;
.Op 449 src/lib/stdio/flushbuf.c
49140	        stream->_flags |= _IOWRITING;
49141	        if (!io_testflag(stream, _IONBF)) {
49142	                if (!stream->_buf) {
49143	                        if (stream == stdout && _isatty(fileno(stdout))) {
49144	                                if (!(stream->_buf =
49145	                                            (unsigned char *) malloc(BUFSIZ))) {
49146	                                        stream->_flags |= _IONBF;
49147	                                } else {
49148	                                        stream->_flags |= _IOLBF|_IOMYBUF;
49149	                                        stream->_bufsiz = BUFSIZ;
49150	                                        stream->_count = -1;
49151	                                }
49152	                        } else {
49153	                                if (!(stream->_buf =
49154	                                            (unsigned char *) malloc(BUFSIZ))) {
49155	                                        stream->_flags |= _IONBF;
49156	                                } else {
49157	                                        stream->_flags |= _IOMYBUF;
49158	                                        stream->_bufsiz = BUFSIZ;
49159	                                        if (!io_testflag(stream, _IOLBF))
49160	                                                stream->_count = BUFSIZ - 1;
49161	                                        else    stream->_count = -1;
49162	                                }
49163	                        }
49164	                        stream->_ptr = stream->_buf;
49165	                }
49166	        }
49167	
49168	        if (io_testflag(stream, _IONBF)) {
49169	                char c1 = c;
49170	
49171	                stream->_count = 0;
49172	                if (io_testflag(stream, _IOAPPEND)) {
49173	                        if (_lseek(fileno(stream), 0L, SEEK_END) == -1) {
49174	                                stream->_flags |= _IOERR;
49175	                                return EOF;
49176	                        }
49177	                }
49178	                if (_write(fileno(stream), &c1, 1) != 1) {
49179	                        stream->_flags |= _IOERR;
49180	                        return EOF;
49181	                }
49182	                return c;
49183	        } else if (io_testflag(stream, _IOLBF)) {
49184	                *stream->_ptr++ = c;
49185	                if (c == '\n' || stream->_count == -stream->_bufsiz) {
49186	                        if (io_testflag(stream, _IOAPPEND)) {
49187	                                if (_lseek(fileno(stream), 0L, SEEK_END) == -1) {
49188	                                        stream->_flags |= _IOERR;
49189	                                        return EOF;
49190	                                }
49191	                        }
49192	                        if (! do_write(fileno(stream), (char *)stream->_buf,
49193	                                        -stream->_count)) {
49194	                                stream->_flags |= _IOERR;
49195	                                return EOF;
49196	                        } else {
49197	                                stream->_ptr  = stream->_buf;
49198	                                stream->_count = 0;
49199	                        }
.Ep 450 src/lib/stdio/flushbuf.c
49200	                }
49201	        } else {
49202	                int count = stream->_ptr - stream->_buf;
49203	
49204	                stream->_count = stream->_bufsiz - 1;
49205	                stream->_ptr = stream->_buf + 1;
49206	
49207	                if (count > 0) {
49208	                        if (io_testflag(stream, _IOAPPEND)) {
49209	                                if (_lseek(fileno(stream), 0L, SEEK_END) == -1) {
49210	                                        stream->_flags |= _IOERR;
49211	                                        return EOF;
49212	                                }
49213	                        }
49214	                        if (! do_write(fileno(stream), (char *)stream->_buf, count)) {
49215	                                *(stream->_buf) = c;
49216	                                stream->_flags |= _IOERR;
49217	                                return EOF;
49218	                        }
49219	                }
49220	                *(stream->_buf) = c;
49221	        }
49222	        return c;
49223	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/fopen.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

49300	/*
49301	 * fopen.c - open a stream
49302	 */
49303	/* $Header: fopen.c,v 1.8 91/02/22 16:29:46 ceriel Exp $ */
49304	
49305	#if     defined(_POSIX_SOURCE)
49306	#include        <sys/types.h>
49307	#endif
49308	#include        <stdio.h>
49309	#include        <stdlib.h>
49310	#include        "loc_incl.h"
49311	
49312	#define PMODE           0666
49313	
49314	/* The next 3 defines are true in all UNIX systems known to me.
49315	 */
49316	#define O_RDONLY        0
49317	#define O_WRONLY        1
49318	#define O_RDWR          2
49319	
49320	/* Since the O_CREAT flag is not available on all systems, we can't get it
49321	 * from the standard library. Furthermore, even if we know that <fcntl.h>
49322	 * contains such a flag, it's not sure whether it can be used, since we
49323	 * might be cross-compiling for another system, which may use an entirely
49324	 * different value for O_CREAT (or not support such a mode). The safest
49325	 * thing is to just use the Version 7 semantics for open, and use creat()
49326	 * whenever necessary.
49327	 *
49328	 * Another problem is O_APPEND, for which the same holds. When "a"
49329	 * open-mode is used, an lseek() to the end is done before every write()
.Op 451 src/lib/stdio/fopen.c
49330	 * system-call.
49331	 *
49332	 * The O_CREAT, O_TRUNC and O_APPEND given here, are only for convenience.
49333	 * They are not passed to open(), so the values don't have to match a value
49334	 * from the real world. It is enough when they are unique.
49335	 */
49336	#define O_CREAT         0x010
49337	#define O_TRUNC         0x020
49338	#define O_APPEND        0x040
49339	
49340	int _open(const char *path, int flags);
49341	int _creat(const char *path, Mode_t mode);
49342	int _close(int d);
49343	
49344	FILE *
49345	fopen(const char *name, const char *mode)
49346	{
49347	        register int i;
49348	        int rwmode = 0, rwflags = 0;
49349	        FILE *stream;
49350	        int fd, flags = 0;
49351	
49352	        for (i = 0; __iotab[i] != 0 ; i++) 
49353	                if ( i >= FOPEN_MAX-1 )
49354	                        return (FILE *)NULL;
49355	
49356	        switch(*mode++) {
49357	        case 'r':
49358	                flags |= _IOREAD | _IOREADING;  
49359	                rwmode = O_RDONLY;
49360	                break;
49361	        case 'w':
49362	                flags |= _IOWRITE | _IOWRITING;
49363	                rwmode = O_WRONLY;
49364	                rwflags = O_CREAT | O_TRUNC;
49365	                break;
49366	        case 'a': 
49367	                flags |= _IOWRITE | _IOWRITING | _IOAPPEND;
49368	                rwmode = O_WRONLY;
49369	                rwflags |= O_APPEND | O_CREAT;
49370	                break;         
49371	        default:
49372	                return (FILE *)NULL;
49373	        }
49374	
49375	        while (*mode) {
49376	                switch(*mode++) {
49377	                case 'b':
49378	                        continue;
49379	                case '+':
49380	                        rwmode = O_RDWR;
49381	                        flags |= _IOREAD | _IOWRITE;
49382	                        continue;
49383	                /* The sequence may be followed by additional characters */
49384	                default:
49385	                        break;
49386	                }
49387	                break;
49388	        }
49389	
.Ep 452 src/lib/stdio/fopen.c
49390	        /* Perform a creat() when the file should be truncated or when
49391	         * the file is opened for writing and the open() failed.
49392	         */
49393	        if ((rwflags & O_TRUNC)
49394	            || (((fd = _open(name, rwmode)) < 0)
49395	                    && (rwflags & O_CREAT))) {
49396	                if (((fd = _creat(name, PMODE)) > 0) && flags  | _IOREAD) {
49397	                        (void) _close(fd);
49398	                        fd = _open(name, rwmode);
49399	                }
49400	                        
49401	        }
49402	
49403	        if (fd < 0) return (FILE *)NULL;
49404	
49405	        if (( stream = (FILE *) malloc(sizeof(FILE))) == NULL ) {
49406	                _close(fd);
49407	                return (FILE *)NULL;
49408	        }
49409	
49410	        if ((flags & (_IOREAD | _IOWRITE))  == (_IOREAD | _IOWRITE))
49411	                flags &= ~(_IOREADING | _IOWRITING);
49412	
49413	        stream->_count = 0;
49414	        stream->_fd = fd;
49415	        stream->_flags = flags;
49416	        stream->_buf = NULL;
49417	        __iotab[i] = stream;
49418	        return stream;
49419	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/fprintf.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

49500	/*
49501	 * fprintf - write output on a stream
49502	 */
49503	/* $Header: fprintf.c,v 1.3 89/12/18 15:01:54 eck Exp $ */
49504	
49505	#include        <stdio.h>
49506	#include        <stdarg.h>
49507	#include        "loc_incl.h"
49508	
49509	int
49510	fprintf(FILE *stream, const char *format, ...)
49511	{
49512	        va_list ap;
49513	        int retval;
49514	        
49515	        va_start(ap, format);
49516	
49517	        retval = _doprnt (format, ap, stream);
49518	
49519	        va_end(ap);
.Op 453 src/lib/stdio/fprintf.c
49520	
49521	        return retval;
49522	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/fputc.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

49600	/*
49601	 * fputc.c - print an unsigned character
49602	 */
49603	/* $Header: fputc.c,v 1.1 89/05/30 13:28:45 eck Exp $ */
49604	
49605	#include        <stdio.h>
49606	
49607	int
49608	fputc(int c, FILE *stream)
49609	{
49610	        return putc(c, stream);
49611	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/fputs.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

49700	/*
49701	 * fputs - print a string
49702	 */
49703	/* $Header: fputs.c,v 1.2 89/12/18 15:02:01 eck Exp $ */
49704	
49705	#include        <stdio.h>
49706	
49707	int
49708	fputs(register const char *s, register FILE *stream)
49709	{
49710	        register int i = 0;
49711	
49712	        while (*s) 
49713	                if (putc(*s++, stream) == EOF) return EOF;
49714	                else i++;
49715	
49716	        return i;
49717	}
.Ep 454 src/lib/stdio/fread.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/fread.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

49800	/*
49801	 * fread.c - read a number of members into an array
49802	 */
49803	/* $Header: fread.c,v 1.2 89/12/18 15:02:09 eck Exp $ */
49804	
49805	#include        <stdio.h>
49806	
49807	size_t
49808	fread(void *ptr, size_t size, size_t nmemb, register FILE *stream)
49809	{
49810	        register char *cp = ptr;
49811	        register int c;
49812	        size_t ndone = 0;
49813	        register size_t s;
49814	
49815	        if (size)
49816	                while ( ndone < nmemb ) {
49817	                        s = size;
49818	                        do {
49819	                                if ((c = getc(stream)) != EOF)
49820	                                        *cp++ = c;
49821	                                else
49822	                                        return ndone;
49823	                        } while (--s);
49824	                        ndone++;
49825	                }
49826	
49827	        return ndone;
49828	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/freopen.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

49900	/*
49901	 * freopen.c - open a file and associate a stream with it
49902	 */
49903	/* $Header: freopen.c,v 1.8 90/08/28 13:44:48 eck Exp $ */
49904	
49905	#if     defined(_POSIX_SOURCE)
49906	#include        <sys/types.h>
49907	#endif
49908	#include        <stdio.h>
49909	#include        <stdlib.h>
49910	#include        "loc_incl.h"
49911	
49912	#define PMODE           0666
49913	
49914	/* Do not "optimize" this file to use the open with O_CREAT if the file
49915	 * does not exist. The reason is given in fopen.c.
49916	 */
49917	#define O_RDONLY        0
49918	#define O_WRONLY        1
49919	#define O_RDWR          2
.Op 455 src/lib/stdio/freopen.c
49920	
49921	#define O_CREAT         0x010
49922	#define O_TRUNC         0x020
49923	#define O_APPEND        0x040
49924	
49925	int _open(const char *path, int flags);
49926	int _creat(const char *path, Mode_t mode);
49927	int _close(int d);
49928	
49929	FILE *
49930	freopen(const char *name, const char *mode, FILE *stream)
49931	{
49932	        register int i;
49933	        int rwmode = 0, rwflags = 0;
49934	        int fd, flags = stream->_flags & (_IONBF | _IOFBF | _IOLBF | _IOMYBUF);
49935	
49936	        (void) fflush(stream);                          /* ignore errors */
49937	        (void) _close(fileno(stream));
49938	
49939	        switch(*mode++) {
49940	        case 'r':
49941	                flags |= _IOREAD;       
49942	                rwmode = O_RDONLY;
49943	                break;
49944	        case 'w':
49945	                flags |= _IOWRITE;
49946	                rwmode = O_WRONLY;
49947	                rwflags = O_CREAT | O_TRUNC;
49948	                break;
49949	        case 'a': 
49950	                flags |= _IOWRITE | _IOAPPEND;
49951	                rwmode = O_WRONLY;
49952	                rwflags |= O_APPEND | O_CREAT;
49953	                break;         
49954	        default:
49955	                return (FILE *)NULL;
49956	        }
49957	
49958	        while (*mode) {
49959	                switch(*mode++) {
49960	                case 'b':
49961	                        continue;
49962	                case '+':
49963	                        rwmode = O_RDWR;
49964	                        flags |= _IOREAD | _IOWRITE;
49965	                        continue;
49966	                /* The sequence may be followed by aditional characters */
49967	                default:
49968	                        break;
49969	                }
49970	                break;
49971	        }
49972	
49973	        if ((rwflags & O_TRUNC)
49974	            || (((fd = _open(name, rwmode)) < 0)
49975	                    && (rwflags & O_CREAT))) {
49976	                if (((fd = _creat(name, PMODE)) < 0) && flags | _IOREAD) {
49977	                        (void) _close(fd);
49978	                        fd = _open(name, rwmode);
49979	                }
.Ep 456 src/lib/stdio/freopen.c
49980	        }
49981	
49982	        if (fd < 0) {
49983	                for( i = 0; i < FOPEN_MAX; i++) {
49984	                        if (stream == __iotab[i]) {
49985	                                __iotab[i] = 0;
49986	                                break;
49987	                        }
49988	                }
49989	                if (stream != stdin && stream != stdout && stream != stderr)
49990	                        free((void *)stream);
49991	                return (FILE *)NULL;
49992	        }
49993	
49994	        stream->_count = 0;
49995	        stream->_fd = fd;
49996	        stream->_flags = flags;
49997	        return stream;
49998	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/fscanf.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

50000	/*
50001	 * fscanf.c - read formatted input from stream
50002	 */
50003	/* $Header: fscanf.c,v 1.1 89/05/30 13:29:17 eck Exp $ */
50004	
50005	#include        <stdio.h>
50006	#include        <stdarg.h>
50007	#include        "loc_incl.h"
50008	
50009	int
50010	fscanf(FILE *stream, const char *format, ...)
50011	{
50012	        va_list ap;
50013	        int retval;
50014	
50015	        va_start(ap, format);
50016	
50017	        retval = _doscan(stream, format, ap);
50018	
50019	        va_end(ap);
50020	
50021	        return retval;
50022	}
.Op 457 src/lib/stdio/fseek.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/fseek.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

50100	/*
50101	 * fseek.c - perform an fseek
50102	 */
50103	/* $Header: fseek.c,v 1.4 90/01/22 11:12:00 eck Exp $ */
50104	
50105	#include        <stdio.h>
50106	
50107	#if     (SEEK_CUR != 1) || (SEEK_END != 2) || (SEEK_SET != 0)
50108	#error SEEK_* values are wrong
50109	#endif
50110	
50111	#include        "loc_incl.h"
50112	
50113	#include        <sys/types.h>
50114	
50115	off_t _lseek(int fildes, off_t offset, int whence);
50116	
50117	int
50118	fseek(FILE *stream, long int offset, int whence)
50119	{
50120	        int adjust = 0;
50121	        long pos;
50122	
50123	        stream->_flags &= ~(_IOEOF | _IOERR);
50124	        /* Clear both the end of file and error flags */
50125	
50126	        if (io_testflag(stream, _IOREADING)) {
50127	                if (whence == SEEK_CUR
50128	                    && stream->_buf
50129	                    && !io_testflag(stream,_IONBF))
50130	                        adjust = stream->_count;
50131	                stream->_count = 0;
50132	        } else if (io_testflag(stream,_IOWRITING)) {
50133	                fflush(stream);
50134	        } else  /* neither reading nor writing. The buffer must be empty */
50135	                /* EMPTY */ ;
50136	
50137	        pos = _lseek(fileno(stream), offset - adjust, whence);
50138	        if (io_testflag(stream, _IOREAD) && io_testflag(stream, _IOWRITE))
50139	                stream->_flags &= ~(_IOREADING | _IOWRITING);
50140	
50141	        stream->_ptr = stream->_buf;
50142	        return ((pos == -1) ? -1 : 0);
50143	}
.Ep 458 src/lib/stdio/fsetpos.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/fsetpos.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

50200	/*
50201	 * fsetpos.c - set the position in the file
50202	 */
50203	/* $Header: fsetpos.c,v 1.1 89/05/30 13:29:34 eck Exp $ */
50204	
50205	#include        <stdio.h>
50206	
50207	int
50208	fsetpos(FILE *stream, fpos_t *pos)
50209	{
50210	        return fseek(stream, *pos, SEEK_SET);
50211	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/ftell.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

50300	/*
50301	 * ftell.c - obtain the value of the file-position indicator of a stream
50302	 */
50303	/* $Header: ftell.c,v 1.4 90/01/22 11:12:12 eck Exp $ */
50304	
50305	#include        <stdio.h>
50306	
50307	#if     (SEEK_CUR != 1) || (SEEK_SET != 0) || (SEEK_END != 2)
50308	#error SEEK_* values are wrong
50309	#endif
50310	
50311	#include        "loc_incl.h"
50312	
50313	#include        <sys/types.h>
50314	
50315	off_t _lseek(int fildes, off_t offset, int whence);
50316	
50317	long ftell(FILE *stream)
50318	{
50319	        long result;
50320	        int adjust = 0;
50321	
50322	        if (io_testflag(stream,_IOREADING))
50323	                adjust = -stream->_count;
50324	        else if (io_testflag(stream,_IOWRITING)
50325	                    && stream->_buf
50326	                    && !io_testflag(stream,_IONBF))
50327	                adjust = stream->_ptr - stream->_buf;
50328	        else adjust = 0;
50329	
50330	        result = _lseek(fileno(stream), (off_t)0, SEEK_CUR);
50331	
50332	        if ( result == -1 )
50333	                return result;
50334	
.Op 459 src/lib/stdio/ftell.c
50335	        result += (long) adjust;
50336	        return result;
50337	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/fwrite.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

50400	/*
50401	 * fwrite.c - write a number of array elements on a file
50402	 */
50403	/* $Header: fwrite.c,v 1.3 89/12/18 15:02:39 eck Exp $ */
50404	
50405	#include        <stdio.h>
50406	
50407	size_t
50408	fwrite(const void *ptr, size_t size, size_t nmemb,
50409	            register FILE *stream)
50410	{
50411	        register const unsigned char *cp = ptr;
50412	        register size_t s;
50413	        size_t ndone = 0;
50414	
50415	        if (size)
50416	                while ( ndone < nmemb ) {
50417	                        s = size;
50418	                        do {
50419	                                if (putc((int)*cp, stream)
50420	                                        == EOF)
50421	                                        return ndone;
50422	                                cp++;
50423	                        } 
50424	                        while (--s);
50425	                        ndone++;
50426	                }
50427	        return ndone;
50428	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/getc.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

50500	/*
50501	 * getc.c - read an unsigned character
50502	 */
50503	/* $Header: getc.c,v 1.2 89/12/18 15:02:45 eck Exp $ */
50504	
50505	#include        <stdio.h>
50506	
50507	int
50508	(getc)(FILE *stream)
50509	{
50510	        return getc(stream);
50511	}
.Ep 460 src/lib/stdio/getchar.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/getchar.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

50600	/*
50601	 * getchar.c - read a character from the standard input stream
50602	 */
50603	/* $Header: getchar.c,v 1.2 89/12/18 15:02:53 eck Exp $ */
50604	
50605	#include        <stdio.h>
50606	
50607	int
50608	(getchar)(void)
50609	{
50610	        return getchar();
50611	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/gets.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

50700	/*
50701	 * gets.c - read a line from a stream
50702	 */
50703	/* $Header: gets.c,v 1.2 89/12/18 15:03:00 eck Exp $ */
50704	
50705	#include        <stdio.h>
50706	
50707	char *
50708	gets(char *s)
50709	{
50710	        register FILE *stream = stdin;
50711	        register int ch;
50712	        register char *ptr;
50713	
50714	        ptr = s;
50715	        while ((ch = getc(stream)) != EOF && ch != '\n')
50716	                *ptr++ = ch;
50717	
50718	        if (ch == EOF) {
50719	                if (feof(stream)) {
50720	                        if (ptr == s) return NULL;
50721	                } else return NULL;
50722	        }
50723	
50724	        *ptr = '\0';
50725	        return s;
50726	}
.Op 461 src/lib/stdio/icompute.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/icompute.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

50800	/*
50801	 * icompute.c - compute an integer
50802	 */
50803	/* $Header: icompute.c,v 1.1 89/12/18 14:59:38 eck Exp $ */
50804	
50805	#include        "loc_incl.h"
50806	
50807	/* This routine is used in doprnt.c as well as in tmpfile.c and tmpnam.c. */
50808	
50809	char *
50810	_i_compute(unsigned long val, int base, char *s, int nrdigits)
50811	{
50812	        int c;
50813	
50814	        c= val % base ;
50815	        val /= base ;
50816	        if (val || nrdigits > 1)
50817	                s = _i_compute(val, base, s, nrdigits - 1);
50818	        *s++ = (c>9 ? c-10+'a' : c+'0');
50819	        return s;
50820	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/perror.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

50900	/*
50901	 * perror.c - print an error message on the standard error output
50902	 */
50903	/* $Header: perror.c,v 1.1 89/05/30 13:31:30 eck Exp $ */
50904	
50905	#if     defined(_POSIX_SOURCE)
50906	#include        <sys/types.h>
50907	#endif
50908	#include        <stdio.h>
50909	#include        <errno.h>
50910	#include        <stdio.h>
50911	#include        <string.h>
50912	#include        "loc_incl.h"
50913	
50914	ssize_t _write(int d, const char *buf, size_t nbytes);
50915	
50916	void
50917	perror(const char *s)
50918	{
50919	        char *p;
50920	        int fd;
50921	
50922	        p = strerror(errno);
50923	        fd = fileno(stderr);
50924	        fflush(stdout);
.Ep 462 src/lib/stdio/perror.c
50925	        fflush(stderr);
50926	        if (s && *s) {
50927	                _write(fd, s, strlen(s));
50928	                _write(fd, ": ", 2);
50929	        }
50930	        _write(fd, p, strlen(p));
50931	        _write(fd, "\n", 1);
50932	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/printf.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

51000	/*
51001	 * printf - write on the standard output stream
51002	 */
51003	/* $Header: printf.c,v 1.3 89/12/18 15:03:08 eck Exp $ */
51004	
51005	#include        <stdio.h>
51006	#include        <stdarg.h>
51007	#include        "loc_incl.h"
51008	
51009	int
51010	printf(const char *format, ...)
51011	{
51012	        va_list ap;
51013	        int retval;
51014	
51015	        va_start(ap, format);
51016	
51017	        retval = _doprnt(format, ap, stdout);
51018	
51019	        va_end(ap);
51020	
51021	        return retval;
51022	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/putc.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

51100	/*
51101	 * putc.c - print (or buffer) one character
51102	 */
51103	/* $Header: putc.c,v 1.2 89/12/18 15:03:15 eck Exp $ */
51104	
51105	#include        <stdio.h>
51106	
51107	int
51108	(putc)(int c, FILE *stream)
51109	{
51110	        return putc(c, stream);
51111	}
.Op 463 src/lib/stdio/putchar.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/putchar.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

51200	/*
51201	 * putchar.c - print (or buffer) a character on the standard output stream
51202	 */
51203	/* $Header: putchar.c,v 1.2 89/12/18 15:03:23 eck Exp $ */
51204	
51205	#include        <stdio.h>
51206	
51207	int
51208	(putchar)(int c)
51209	{
51210	        return putchar(c);
51211	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/puts.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

51300	/*
51301	 * puts.c - print a string onto the standard output stream
51302	 */
51303	/* $Header: puts.c,v 1.2 89/12/18 15:03:30 eck Exp $ */
51304	
51305	#include        <stdio.h>
51306	
51307	int
51308	puts(register const char *s)
51309	{
51310	        register FILE *file = stdout;
51311	        register int i = 0;
51312	
51313	        while (*s) {
51314	                if (putc(*s++, file) == EOF) return EOF;
51315	                else i++;
51316	        }
51317	        if (putc('\n', file) == EOF) return EOF;
51318	        return i + 1;
51319	}
.Ep 464 src/lib/stdio/remove.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/remove.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

51400	/*
51401	 * remove.c - remove a file
51402	 */
51403	/* $Header: remove.c,v 1.2 90/01/22 11:12:44 eck Exp $ */
51404	
51405	#include        <stdio.h>
51406	
51407	int _unlink(const char *path);
51408	
51409	int
51410	remove(const char *filename) {
51411	        return _unlink(filename);
51412	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/rewind.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

51500	/*
51501	 * rewind.c - set the file position indicator of a stream to the start
51502	 */
51503	/* $Header: rewind.c,v 1.1 89/05/30 13:32:52 eck Exp $ */
51504	
51505	#include        <stdio.h>
51506	#include        "loc_incl.h"
51507	
51508	void
51509	rewind(FILE *stream)
51510	{
51511	        (void) fseek(stream, 0L, SEEK_SET);
51512	        clearerr(stream);
51513	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/scanf.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

51600	/*
51601	 * scanf.c - read formatted input from the standard input stream
51602	 */
51603	/* $Header: scanf.c,v 1.1 89/05/30 13:33:03 eck Exp $ */
51604	
51605	#include        <stdio.h>
51606	#include        <stdarg.h>
51607	#include        "loc_incl.h"
51608	
51609	int
51610	scanf(const char *format, ...)
51611	{
51612	        va_list ap;
51613	        int retval;
51614	
.Op 465 src/lib/stdio/scanf.c
51615	        va_start(ap, format);
51616	
51617	        retval = _doscan(stdin, format, ap);
51618	
51619	        va_end(ap);
51620	
51621	        return retval;
51622	}
	
	






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/setbuf.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

51700	/*
51701	 * setbuf.c - control buffering of a stream
51702	 */
51703	/* $Header: setbuf.c,v 1.2 89/06/26 10:36:22 eck Exp $ */
51704	
51705	#include        <stdio.h>
51706	#include        "loc_incl.h"
51707	
51708	void
51709	setbuf(register FILE *stream, char *buf)
51710	{
51711	        (void) setvbuf(stream, buf, (buf ? _IOFBF : _IONBF), (size_t) BUFSIZ);
51712	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/setvbuf.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

51800	/*
51801	 * setbuf.c - control buffering of a stream
51802	 */
51803	/* $Id: setvbuf.c,v 1.8 1995/12/18 11:02:18 ceriel Exp $ */
51804	
51805	#include        <stdio.h>
51806	#include        <stdlib.h>
51807	#include        "loc_incl.h"
51808	
51809	extern void (*_clean)(void);
51810	
51811	int
51812	setvbuf(register FILE *stream, char *buf, int mode, size_t size)
51813	{
51814	        int retval = 0;
51815	
51816	        _clean = __cleanup;
51817	        if (mode != _IOFBF && mode != _IOLBF && mode != _IONBF)
51818	                return EOF;
51819	
.Ep 466 src/lib/stdio/setvbuf.c
51820	        if (stream->_buf && io_testflag(stream,_IOMYBUF) )
51821	                free((void *)stream->_buf);
51822	
51823	        stream->_flags &= ~(_IOMYBUF | _IONBF | _IOLBF);
51824	
51825	        if (buf && size <= 0) retval = EOF;
51826	        if (!buf && (mode != _IONBF)) {
51827	                if (size <= 0 || (buf = (char *) malloc(size)) == NULL) {
51828	                        retval = EOF;
51829	                } else {
51830	                        stream->_flags |= _IOMYBUF;
51831	                }
51832	        }
51833	
51834	        stream->_buf = (unsigned char *) buf;
51835	
51836	        stream->_count = 0;
51837	        stream->_flags |= mode;
51838	        stream->_ptr = stream->_buf;
51839	
51840	        if (!buf) {
51841	                stream->_bufsiz = 1;
51842	        } else {
51843	                stream->_bufsiz = size;
51844	        }
51845	
51846	        return retval;
51847	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/sprintf.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

51900	/*
51901	 * sprintf - print formatted output on an array
51902	 */
51903	/* $Header: sprintf.c,v 1.2 89/12/18 15:03:52 eck Exp $ */
51904	
51905	#include        <stdio.h>
51906	#include        <stdarg.h>
51907	#include        "loc_incl.h"
51908	
51909	int
51910	sprintf(char * s, const char *format, ...)
51911	{
51912	        va_list ap;
51913	        int retval;
51914	        FILE tmp_stream;
51915	
51916	        va_start(ap, format);
51917	
51918	        tmp_stream._fd     = -1;
51919	        tmp_stream._flags  = _IOWRITE + _IONBF + _IOWRITING;
51920	        tmp_stream._buf    = (unsigned char *) s;
51921	        tmp_stream._ptr    = (unsigned char *) s;
51922	        tmp_stream._count  = 32767;
51923	
51924	        retval = _doprnt(format, ap, &tmp_stream);
.Op 467 src/lib/stdio/sprintf.c
51925	        putc('\0',&tmp_stream);
51926	
51927	        va_end(ap);
51928	
51929	        return retval;
51930	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/sscanf.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

52000	/*
52001	 * sscanf - read formatted output from a string
52002	 */
52003	/* $Header: sscanf.c,v 1.3 90/09/26 13:17:39 eck Exp $ */
52004	
52005	#include        <stdio.h>
52006	#include        <stdarg.h>
52007	#include        <string.h>
52008	#include        "loc_incl.h"
52009	
52010	int sscanf(const char *s, const char *format, ...)
52011	{
52012	        va_list ap;
52013	        int retval;
52014	        FILE tmp_stream;
52015	
52016	        va_start(ap, format);
52017	
52018	        tmp_stream._fd     = -1;
52019	        tmp_stream._flags  = _IOREAD + _IONBF + _IOREADING;
52020	        tmp_stream._buf    = (unsigned char *) s;
52021	        tmp_stream._ptr    = (unsigned char *) s;
52022	        tmp_stream._count  = strlen(s);
52023	
52024	        retval = _doscan(&tmp_stream, format, ap);
52025	
52026	        va_end(ap);
52027	
52028	        return retval;
52029	}
.Ep 468 src/lib/stdio/tmpfile.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/tmpfile.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

52100	/*
52101	 * tmpfile.c - create and open a temporary file
52102	 */
52103	/* $Header: tmpfile.c,v 1.3 90/01/22 11:13:15 eck Exp $ */
52104	
52105	#if     defined(_POSIX_SOURCE)
52106	#include        <sys/types.h>
52107	#endif
52108	#include        <stdio.h>
52109	#include        <string.h>
52110	#include        "loc_incl.h"
52111	
52112	pid_t _getpid(void);
52113	
52114	FILE *
52115	tmpfile(void) {
52116	        static char name_buffer[L_tmpnam] = "/tmp/tmp." ;
52117	        static char *name = NULL;
52118	        FILE *file;
52119	
52120	        if (!name) {
52121	                name = name_buffer + strlen(name_buffer);
52122	                name = _i_compute(_getpid(), 10, name, 5);
52123	                *name = '\0';
52124	        }
52125	
52126	        file = fopen(name_buffer,"wb+");
52127	        if (!file) return (FILE *)NULL;
52128	        (void) remove(name_buffer);
52129	        return file;
52130	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/tmpnam.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

52200	/*
52201	 * tmpnam.c - create a unique filename
52202	 */
52203	/* $Header: tmpnam.c,v 1.4 91/02/26 09:28:39 ceriel Exp $ */
52204	
52205	#if     defined(_POSIX_SOURCE)
52206	#include        <sys/types.h>
52207	#endif
52208	#include        <stdio.h>
52209	#include        <string.h>
52210	#include        "loc_incl.h"
52211	
52212	pid_t _getpid(void);
52213	
52214	char *
.Op 469 src/lib/stdio/tmpnam.c
52215	tmpnam(char *s) {
52216	        static char name_buffer[L_tmpnam] = "/tmp/tmp.";
52217	        static unsigned long count = 0;
52218	        static char *name = NULL;
52219	
52220	        if (!name) { 
52221	                name = name_buffer + strlen(name_buffer);
52222	                name = _i_compute((unsigned long)_getpid(), 10, name, 5);
52223	                *name++ = '.';
52224	                *name = '\0';
52225	        }
52226	        if (++count > TMP_MAX) count = 1;       /* wrap-around */
52227	        *_i_compute(count, 10, name, 3) = '\0';
52228	        if (s) return strcpy(s, name_buffer);
52229	        else return name_buffer;
52230	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/ungetc.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

52300	/*
52301	 * ungetc.c - push a character back onto an input stream
52302	 */
52303	/* $Header: ungetc.c,v 1.3 90/03/28 16:33:05 eck Exp $ */
52304	
52305	#include        <stdio.h>
52306	#include        "loc_incl.h"
52307	
52308	int
52309	ungetc(int ch, FILE *stream)
52310	{
52311	        unsigned char *p;
52312	
52313	        if (ch == EOF  || !io_testflag(stream,_IOREADING))
52314	                return EOF;
52315	        if (stream->_ptr == stream->_buf) {
52316	                if (stream->_count != 0) return EOF;
52317	                stream->_ptr++;
52318	        }
52319	        stream->_count++;
52320	        p = --(stream->_ptr);           /* ??? Bloody vax assembler !!! */
52321	        /* ungetc() in sscanf() shouldn't write in rom */
52322	        if (*p != (unsigned char) ch)
52323	                *p = (unsigned char) ch;
52324	        return ch;
52325	}
.Ep 470 src/lib/stdio/vfprintf.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/vfprintf.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

52400	/*
52401	 * vfprintf - formatted output without ellipsis
52402	 */
52403	/* $Header: vfprintf.c,v 1.2 89/12/18 15:04:07 eck Exp $ */
52404	
52405	#include        <stdio.h>
52406	#include        <stdarg.h>
52407	#include        "loc_incl.h"
52408	
52409	int
52410	vfprintf(FILE *stream, const char *format, va_list arg)
52411	{
52412	        return _doprnt (format, arg, stream);
52413	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/vprintf.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

52500	/*
52501	 * vprintf - formatted output without ellipsis to the standard output stream
52502	 */
52503	/* $Header: vprintf.c,v 1.3 89/12/18 15:04:14 eck Exp $ */
52504	
52505	#include        <stdio.h>
52506	#include        <stdarg.h>
52507	#include        "loc_incl.h"
52508	
52509	int
52510	vprintf(const char *format, va_list arg)
52511	{
52512	        return _doprnt(format, arg, stdout);
52513	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/vsprintf.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

52600	/*
52601	 * vsprintf - print formatted output without ellipsis on an array
52602	 */
52603	/* $Header: vsprintf.c,v 1.2 90/11/13 10:56:53 eck Exp $ */
52604	
52605	#include        <stdio.h>
52606	#include        <stdarg.h>
52607	#include        "loc_incl.h"
52608	
52609	int
52610	vsprintf(char *s, const char *format, va_list arg)
52611	{
52612	        int retval;
52613	        FILE tmp_stream;
52614	
.Op 471 src/lib/stdio/vsprintf.c
52615	        tmp_stream._fd     = -1;
52616	        tmp_stream._flags  = _IOWRITE + _IONBF + _IOWRITING;
52617	        tmp_stream._buf    = (unsigned char *) s;
52618	        tmp_stream._ptr    = (unsigned char *) s;
52619	        tmp_stream._count  = 32767;
52620	
52621	        retval = _doprnt(format, arg, &tmp_stream);
52622	        putc('\0',&tmp_stream);
52623	
52624	        return retval;
52625	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/vscanf.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

52700	/*
52701	 * vscanf.c - read formatted input from the standard input stream
52702	 */
52703	
52704	#include        <stdio.h>
52705	#include        <stdarg.h>
52706	#include        "loc_incl.h"
52707	
52708	int
52709	vscanf(const char *format, va_list ap)
52710	{
52711	        return _doscan(stdin, format, ap);
52712	}



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/stdio/vsscanf.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

52800	/*
52801	 * vsscanf - read formatted output from a string
52802	 */
52803	
52804	#include        <stdio.h>
52805	#include        <stdarg.h>
52806	#include        <string.h>
52807	#include        "loc_incl.h"
52808	
52809	int vsscanf(const char *s, const char *format, va_list ap)
52810	{
52811	        FILE tmp_stream;
52812	
52813	        tmp_stream._fd     = -1;
52814	        tmp_stream._flags  = _IOREAD + _IONBF + _IOREADING;
52815	        tmp_stream._buf    = (unsigned char *) s;
52816	        tmp_stream._ptr    = (unsigned char *) s;
52817	        tmp_stream._count  = strlen(s);
52818	
52819	        return _doscan(&tmp_stream, format, ap);
.Ep 472 src/lib/stdio/vsscanf.c
52820	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/_exit.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

52900	.sect .text
52901	.extern ___exit
52902	.define __exit
52903	.align 2
52904	
52905	__exit:
52906	        jmp     ___exit




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/access.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

53000	.sect .text
53001	.extern __access
53002	.define _access
53003	.align 2
53004	
53005	_access:
53006	        jmp     __access




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/alarm.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

53100	.sect .text
53101	.extern __alarm
53102	.define _alarm
53103	.align 2
53104	
53105	_alarm:
53106	        jmp     __alarm
.Op 473 src/lib/syscall/brk.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/brk.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

53200	.sect .text
53201	.extern __brk
53202	.define _brk
53203	.align 2
53204	
53205	_brk:
53206	        jmp     __brk




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/cfgetispeed.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

53300	.sect .text
53301	.extern __cfgetispeed
53302	.define _cfgetispeed
53303	.align 2
53304	
53305	_cfgetispeed:
53306	        jmp     __cfgetispeed




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/cfgetospeed.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

53400	.sect .text
53401	.extern __cfgetospeed
53402	.define _cfgetospeed
53403	.align 2
53404	
53405	_cfgetospeed:
53406	        jmp     __cfgetospeed




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/cfsetispeed.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

53500	.sect .text
53501	.extern __cfsetispeed
53502	.define _cfsetispeed
53503	.align 2
53504	
53505	_cfsetispeed:
53506	        jmp     __cfsetispeed
.Ep 474 src/lib/syscall/cfsetospeed.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/cfsetospeed.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

53600	.sect .text
53601	.extern __cfsetospeed
53602	.define _cfsetospeed
53603	.align 2
53604	
53605	_cfsetospeed:
53606	        jmp     __cfsetospeed




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/chdir.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

53700	.sect .text
53701	.extern __chdir
53702	.define _chdir
53703	.align 2
53704	
53705	_chdir:
53706	        jmp     __chdir




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/chmod.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

53800	.sect .text
53801	.extern __chmod
53802	.define _chmod
53803	.align 2
53804	
53805	_chmod:
53806	        jmp     __chmod




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/chown.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

53900	.sect .text
53901	.extern __chown
53902	.define _chown
53903	.align 2
53904	
53905	_chown:
53906	        jmp     __chown
.Op 475 src/lib/syscall/chroot.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/chroot.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

54000	.sect .text
54001	.extern __chroot
54002	.define _chroot
54003	.align 2
54004	
54005	_chroot:
54006	        jmp     __chroot




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/close.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

54100	.sect .text
54101	.extern __close
54102	.define _close
54103	.align 2
54104	
54105	_close:
54106	        jmp     __close




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/closedir.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

54200	.sect .text
54201	.extern __closedir
54202	.define _closedir
54203	.align 2
54204	
54205	_closedir:
54206	        jmp     __closedir




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/creat.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

54300	.sect .text
54301	.extern __creat
54302	.define _creat
54303	.align 2
54304	
54305	_creat:
54306	        jmp     __creat
.Ep 476 src/lib/syscall/dup.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/dup.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

54400	.sect .text
54401	.extern __dup
54402	.define _dup
54403	.align 2
54404	
54405	_dup:
54406	        jmp     __dup




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/dup2.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

54500	.sect .text
54501	.extern __dup2
54502	.define _dup2
54503	.align 2
54504	
54505	_dup2:
54506	        jmp     __dup2




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/execl.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

54600	.sect .text
54601	.extern __execl
54602	.define _execl
54603	.align 2
54604	
54605	_execl:
54606	        jmp     __execl




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/execle.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

54700	.sect .text
54701	.extern __execle
54702	.define _execle
54703	.align 2
54704	
54705	_execle:
54706	        jmp     __execle
.Op 477 src/lib/syscall/execv.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/execv.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

54800	.sect .text
54801	.extern __execv
54802	.define _execv
54803	.align 2
54804	
54805	_execv:
54806	        jmp     __execv




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/execve.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

54900	.sect .text
54901	.extern __execve
54902	.define _execve
54903	.align 2
54904	
54905	_execve:
54906	        jmp     __execve




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/fcntl.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

55000	.sect .text
55001	.extern __fcntl
55002	.define _fcntl
55003	.align 2
55004	
55005	_fcntl:
55006	        jmp     __fcntl




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/fork.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

55100	.sect .text
55101	.extern __fork
55102	.define _fork
55103	.align 2
55104	
55105	_fork:
55106	        jmp     __fork
.Ep 478 src/lib/syscall/fpathconf.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/fpathconf.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

55200	.sect .text
55201	.extern __fpathconf
55202	.define _fpathconf
55203	.align 2
55204	
55205	_fpathconf:
55206	        jmp     __fpathconf




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/fstat.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

55300	.sect .text
55301	.extern __fstat
55302	.define _fstat
55303	.align 2
55304	
55305	_fstat:
55306	        jmp     __fstat




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/getcwd.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

55400	.sect .text
55401	.extern __getcwd
55402	.define _getcwd
55403	.align 2
55404	
55405	_getcwd:
55406	        jmp     __getcwd




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/getegid.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

55500	.sect .text
55501	.extern __getegid
55502	.define _getegid
55503	.align 2
55504	
55505	_getegid:
55506	        jmp     __getegid
.Op 479 src/lib/syscall/geteuid.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/geteuid.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

55600	.sect .text
55601	.extern __geteuid
55602	.define _geteuid
55603	.align 2
55604	
55605	_geteuid:
55606	        jmp     __geteuid




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/getgid.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

55700	.sect .text
55701	.extern __getgid
55702	.define _getgid
55703	.align 2
55704	
55705	_getgid:
55706	        jmp     __getgid




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/getgroups.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

55800	.sect .text
55801	.extern __getgroups
55802	.define _getgroups
55803	.align 2
55804	
55805	_getgroups:
55806	        jmp     __getgroups




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/getpgrp.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

55900	.sect .text
55901	.extern __getpgrp
55902	.define _getpgrp
55903	.align 2
55904	
55905	_getpgrp:
55906	        jmp     __getpgrp
.Ep 480 src/lib/syscall/getpid.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/getpid.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

56000	.sect .text
56001	.extern __getpid
56002	.define _getpid
56003	.align 2
56004	
56005	_getpid:
56006	        jmp     __getpid




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/getppid.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

56100	.sect .text
56101	.extern __getppid
56102	.define _getppid
56103	.align 2
56104	
56105	_getppid:
56106	        jmp     __getppid




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/getuid.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

56200	.sect .text
56201	.extern __getuid
56202	.define _getuid
56203	.align 2
56204	
56205	_getuid:
56206	        jmp     __getuid




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/ioctl.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

56300	.sect .text
56301	.extern __ioctl
56302	.define _ioctl
56303	.align 2
56304	
56305	_ioctl:
56306	        jmp     __ioctl
.Op 481 src/lib/syscall/isatty.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/isatty.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

56400	.sect .text
56401	.extern __isatty
56402	.define _isatty
56403	.align 2
56404	
56405	_isatty:
56406	        jmp     __isatty




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/kill.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

56500	.sect .text
56501	.extern __kill
56502	.define _kill
56503	.align 2
56504	
56505	_kill:
56506	        jmp     __kill




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/link.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

56600	.sect .text
56601	.extern __link
56602	.define _link
56603	.align 2
56604	
56605	_link:
56606	        jmp     __link




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/lseek.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

56700	.sect .text
56701	.extern __lseek
56702	.define _lseek
56703	.align 2
56704	
56705	_lseek:
56706	        jmp     __lseek
.Ep 482 src/lib/syscall/mkdir.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/mkdir.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

56800	.sect .text
56801	.extern __mkdir
56802	.define _mkdir
56803	.align 2
56804	
56805	_mkdir:
56806	        jmp     __mkdir




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/mkfifo.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

56900	.sect .text
56901	.extern __mkfifo
56902	.define _mkfifo
56903	.align 2
56904	
56905	_mkfifo:
56906	        jmp     __mkfifo




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/mknod.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

57000	.sect .text
57001	.extern __mknod
57002	.define _mknod
57003	.align 2
57004	
57005	_mknod:
57006	        jmp     __mknod




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/mktemp.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

57100	.sect .text
57101	.extern __mktemp
57102	.define _mktemp
57103	.align 2
57104	
57105	_mktemp:
57106	        jmp     __mktemp
.Op 483 src/lib/syscall/mount.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/mount.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

57200	.sect .text
57201	.extern __mount
57202	.define _mount
57203	.align 2
57204	
57205	_mount:
57206	        jmp     __mount




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/open.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

57300	.sect .text
57301	.extern __open
57302	.define _open
57303	.align 2
57304	
57305	_open:
57306	        jmp     __open




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/opendir.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

57400	.sect .text
57401	.extern __opendir
57402	.define _opendir
57403	.align 2
57404	
57405	_opendir:
57406	        jmp     __opendir




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/pathconf.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

57500	.sect .text
57501	.extern __pathconf
57502	.define _pathconf
57503	.align 2
57504	
57505	_pathconf:
57506	        jmp     __pathconf
.Ep 484 src/lib/syscall/pause.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/pause.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

57600	.sect .text
57601	.extern __pause
57602	.define _pause
57603	.align 2
57604	
57605	_pause:
57606	        jmp     __pause




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/pipe.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

57700	.sect .text
57701	.extern __pipe
57702	.define _pipe
57703	.align 2
57704	
57705	_pipe:
57706	        jmp     __pipe




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/ptrace.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

57800	.sect .text
57801	.extern __ptrace
57802	.define _ptrace
57803	.align 2
57804	
57805	_ptrace:
57806	        jmp     __ptrace




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/read.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

57900	.sect .text
57901	.extern __read
57902	.define _read
57903	.align 2
57904	
57905	_read:
57906	        jmp     __read
.Op 485 src/lib/syscall/readdir.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/readdir.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

58000	.sect .text
58001	.extern __readdir
58002	.define _readdir
58003	.align 2
58004	
58005	_readdir:
58006	        jmp     __readdir




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/reboot.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

58100	.sect .text
58101	.extern __reboot
58102	.define _reboot
58103	.align 2
58104	
58105	_reboot:
58106	        jmp     __reboot




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/rename.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

58200	.sect .text
58201	.extern __rename
58202	.define _rename
58203	.align 2
58204	
58205	_rename:
58206	        jmp     __rename




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/rewinddir.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

58300	.sect .text
58301	.extern __rewinddir
58302	.define _rewinddir
58303	.align 2
58304	
58305	_rewinddir:
58306	        jmp     __rewinddir
.Ep 486 src/lib/syscall/rmdir.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/rmdir.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

58400	.sect .text
58401	.extern __rmdir
58402	.define _rmdir
58403	.align 2
58404	
58405	_rmdir:
58406	        jmp     __rmdir




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/sbrk.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

58500	.sect .text
58501	.extern __sbrk
58502	.define _sbrk
58503	.align 2
58504	
58505	_sbrk:
58506	        jmp     __sbrk




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/seekdir.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

58600	.sect .text
58601	.extern __seekdir
58602	.define _seekdir
58603	.align 2
58604	
58605	_seekdir:
58606	        jmp     __seekdir




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/setgid.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

58700	.sect .text
58701	.extern __setgid
58702	.define _setgid
58703	.align 2
58704	
58705	_setgid:
58706	        jmp     __setgid
.Op 487 src/lib/syscall/setsid.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/setsid.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

58800	.sect .text
58801	.extern __setsid
58802	.define _setsid
58803	.align 2
58804	
58805	_setsid:
58806	        jmp     __setsid




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/setuid.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

58900	.sect .text
58901	.extern __setuid
58902	.define _setuid
58903	.align 2
58904	
58905	_setuid:
58906	        jmp     __setuid




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/sigaction.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

59000	.sect .text
59001	.extern __sigaction
59002	.define _sigaction
59003	.align 2
59004	
59005	_sigaction:
59006	        jmp     __sigaction




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/sigaddset.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

59100	.sect .text
59101	.extern __sigaddset
59102	.define _sigaddset
59103	.align 2
59104	
59105	_sigaddset:
59106	        jmp     __sigaddset
.Ep 488 src/lib/syscall/sigdelset.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/sigdelset.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

59200	.sect .text
59201	.extern __sigdelset
59202	.define _sigdelset
59203	.align 2
59204	
59205	_sigdelset:
59206	        jmp     __sigdelset




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/sigemptyset.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

59300	.sect .text
59301	.extern __sigemptyset
59302	.define _sigemptyset
59303	.align 2
59304	
59305	_sigemptyset:
59306	        jmp     __sigemptyset




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/sigfillset.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

59400	.sect .text
59401	.extern __sigfillset
59402	.define _sigfillset
59403	.align 2
59404	
59405	_sigfillset:
59406	        jmp     __sigfillset




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/sigismember.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

59500	.sect .text
59501	.extern __sigismember
59502	.define _sigismember
59503	.align 2
59504	
59505	_sigismember:
59506	        jmp     __sigismember
.Op 489 src/lib/syscall/sigpending.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/sigpending.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

59600	.sect .text
59601	.extern __sigpending
59602	.define _sigpending
59603	.align 2
59604	
59605	_sigpending:
59606	        jmp     __sigpending




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/sigprocmask.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

59700	.sect .text
59701	.extern __sigprocmask
59702	.define _sigprocmask
59703	.align 2
59704	
59705	_sigprocmask:
59706	        jmp     __sigprocmask




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/sigreturn.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

59800	.sect .text
59801	.extern __sigreturn
59802	.define _sigreturn
59803	.align 2
59804	
59805	_sigreturn:
59806	        jmp     __sigreturn




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/sigsuspend.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

59900	.sect .text
59901	.extern __sigsuspend
59902	.define _sigsuspend
59903	.align 2
59904	
59905	_sigsuspend:
59906	        jmp     __sigsuspend
.Ep 490 src/lib/syscall/sleep.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/sleep.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

60000	.sect .text
60001	.extern __sleep
60002	.define _sleep
60003	.align 2
60004	
60005	_sleep:
60006	        jmp     __sleep




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/stat.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

60100	.sect .text
60101	.extern __stat
60102	.define _stat
60103	.align 2
60104	
60105	_stat:
60106	        jmp     __stat




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/stime.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

60200	.sect .text
60201	.extern __stime
60202	.define _stime
60203	.align 2
60204	
60205	_stime:
60206	        jmp     __stime




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/sync.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

60300	.sect .text
60301	.extern __sync
60302	.define _sync
60303	.align 2
60304	
60305	_sync:
60306	        jmp     __sync
.Op 491 src/lib/syscall/tcdrain.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/tcdrain.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

60400	.sect .text
60401	.extern __tcdrain
60402	.define _tcdrain
60403	.align 2
60404	
60405	_tcdrain:
60406	        jmp     __tcdrain




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/tcflow.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

60500	.sect .text
60501	.extern __tcflow
60502	.define _tcflow
60503	.align 2
60504	
60505	_tcflow:
60506	        jmp     __tcflow




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/tcflush.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

60600	.sect .text
60601	.extern __tcflush
60602	.define _tcflush
60603	.align 2
60604	
60605	_tcflush:
60606	        jmp     __tcflush




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/tcgetattr.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

60700	.sect .text
60701	.extern __tcgetattr
60702	.define _tcgetattr
60703	.align 2
60704	
60705	_tcgetattr:
60706	        jmp     __tcgetattr
.Ep 492 src/lib/syscall/tcsendbreak.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/tcsendbreak.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

60800	.sect .text
60801	.extern __tcsendbreak
60802	.define _tcsendbreak
60803	.align 2
60804	
60805	_tcsendbreak:
60806	        jmp     __tcsendbreak




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/tcsetattr.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

60900	.sect .text
60901	.extern __tcsetattr
60902	.define _tcsetattr
60903	.align 2
60904	
60905	_tcsetattr:
60906	        jmp     __tcsetattr




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/time.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

61000	.sect .text
61001	.extern __time
61002	.define _time
61003	.align 2
61004	
61005	_time:
61006	        jmp     __time




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/times.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

61100	.sect .text
61101	.extern __times
61102	.define _times
61103	.align 2
61104	
61105	_times:
61106	        jmp     __times
.Op 493 src/lib/syscall/umask.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/umask.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

61200	.sect .text
61201	.extern __umask
61202	.define _umask
61203	.align 2
61204	
61205	_umask:
61206	        jmp     __umask




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/umount.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

61300	.sect .text
61301	.extern __umount
61302	.define _umount
61303	.align 2
61304	
61305	_umount:
61306	        jmp     __umount




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/uname.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

61400	.sect .text
61401	.extern __uname
61402	.define _uname
61403	.align 2
61404	
61405	_uname:
61406	        jmp     __uname




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/unlink.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

61500	.sect .text
61501	.extern __unlink
61502	.define _unlink
61503	.align 2
61504	
61505	_unlink:
61506	        jmp     __unlink
.Ep 494 src/lib/syscall/utime.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/utime.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

61600	.sect .text
61601	.extern __utime
61602	.define _utime
61603	.align 2
61604	
61605	_utime:
61606	        jmp     __utime




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/wait.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

61700	.sect .text
61701	.extern __wait
61702	.define _wait
61703	.align 2
61704	
61705	_wait:
61706	        jmp     __wait




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/waitpid.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

61800	.sect .text
61801	.extern __waitpid
61802	.define _waitpid
61803	.align 2
61804	
61805	_waitpid:
61806	        jmp     __waitpid




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syscall/write.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

61900	.sect .text
61901	.extern __write
61902	.define _write
61903	.align 2
61904	
61905	_write:
61906	        jmp     __write
.Op 495 src/lib/syslib/syslib.h

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syslib/syslib.h	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

62000	/*      syslib.h - System library common definitions.   */
62001	
62002	#define _SYSTEM
62003	
62004	#include <lib.h>
62005	#include <minix/com.h>
62006	#include <minix/syslib.h>




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syslib/sys_abort.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

62100	#include "syslib.h"
62101	#include <stdarg.h>
62102	#include <unistd.h>
62103	
62104	PUBLIC int sys_abort(int how, ...)
62105	{
62106	/* Something awful has happened.  Abandon ship. */
62107	
62108	  message m;
62109	  va_list ap;
62110	
62111	  va_start(ap, how);
62112	  if ((m.m1_i1 = how) == RBT_MONITOR) m.m1_p1 = va_arg(ap, char *);
62113	  va_end(ap);
62114	
62115	  return(_taskcall(SYSTASK, SYS_ABORT, &m));
62116	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syslib/sys_copy.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

62200	#include "syslib.h"
62201	
62202	PUBLIC int sys_copy(src_proc, src_seg, src_vir,
62203	                                        dst_proc, dst_seg, dst_vir, bytes)
62204	int src_proc;                   /* source process */
62205	int src_seg;                    /* source segment: T, D, or S */
62206	phys_bytes src_vir;             /* source virtual address (phys addr for ABS)*/
62207	int dst_proc;                   /* dest process */
62208	int dst_seg;                    /* dest segment: T, D, or S */
62209	phys_bytes dst_vir;             /* dest virtual address (phys addr for ABS) */
62210	phys_bytes bytes;               /* how many bytes */
62211	{
62212	/* Transfer a block of data.  The source and destination can each either be a
62213	 * process (including MM) or absolute memory, indicate by setting 'src_proc'
62214	 * or 'dst_proc' to ABS.
.Ep 496 src/lib/syslib/sys_copy.c
62215	 */
62216	
62217	  message copy_mess;
62218	
62219	  if (bytes == 0L) return(OK);
62220	  copy_mess.SRC_SPACE = src_seg;
62221	  copy_mess.SRC_PROC_NR = src_proc;
62222	  copy_mess.SRC_BUFFER = (long) src_vir;
62223	
62224	  copy_mess.DST_SPACE = dst_seg;
62225	  copy_mess.DST_PROC_NR = dst_proc;
62226	  copy_mess.DST_BUFFER = (long) dst_vir;
62227	
62228	  copy_mess.COPY_BYTES = (long) bytes;
62229	  return(_taskcall(SYSTASK, SYS_COPY, &copy_mess));
62230	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syslib/sys_endsig.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

62300	#include "syslib.h"
62301	
62302	PUBLIC int sys_endsig(proc)
62303	int proc;
62304	{
62305	  message m;
62306	
62307	  m.m1_i1 = proc;
62308	  return(_taskcall(SYSTASK, SYS_ENDSIG, &m));
62309	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syslib/sys_exec.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

62400	#include "syslib.h"
62401	
62402	PUBLIC int sys_exec(proc, ptr, traced, prog_name, initpc)
62403	int proc;                       /* process that did exec */
62404	char *ptr;                      /* new stack pointer */
62405	int traced;                     /* is tracing enabled? */
62406	char *prog_name;                /* name of the new program */
62407	vir_bytes initpc;
62408	{
62409	/* A process has exec'd.  Tell the kernel. */
62410	
62411	  message m;
62412	
62413	  m.m1_i1 = proc;
62414	  m.m1_i2 = traced;
.Op 497 src/lib/syslib/sys_exec.c
62415	  m.m1_p1 = ptr;
62416	  m.m1_p2 = prog_name;
62417	  m.m1_p3 = (char *)initpc;
62418	  return(_taskcall(SYSTASK, SYS_EXEC, &m));
62419	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syslib/sys_fork.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

62500	#include "syslib.h"
62501	
62502	PUBLIC int sys_fork(parent, child, pid, child_base_or_shadow)
62503	int parent;                     /* process doing the fork */
62504	int child;                      /* which proc has been created by the fork */
62505	int pid;                        /* process id assigned by MM */
62506	phys_clicks child_base_or_shadow;       /* position for child [VM386];
62507	                                 * memory allocated for shadow [68000] */
62508	{
62509	/* A process has forked.  Tell the kernel. */
62510	
62511	  message m;
62512	
62513	  m.m1_i1 = parent;
62514	  m.m1_i2 = child;
62515	  m.m1_i3 = pid;
62516	  m.m1_p1 = (char *) child_base_or_shadow;
62517	  return(_taskcall(SYSTASK, SYS_FORK, &m));
62518	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syslib/sys_fresh.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

62600	#include "syslib.h"
62601	
62602	PUBLIC int sys_fresh(proc, ptr, dc, basep, sizep)
62603	int proc;                       /* process whose map is to be changed */
62604	struct mem_map *ptr;            /* pointer to new map */
62605	phys_clicks dc;                 /* size of initialized data */
62606	phys_clicks *basep, *sizep;     /* base and size for free_mem() */
62607	{
62608	/* Create a fresh process image for exec().  Tell the kernel. */
62609	
62610	  message m;
62611	  int r;
62612	
62613	  m.m1_i1 = proc;
62614	  m.m1_i2 = (int) dc;
62615	  m.m1_p1 = (char *) ptr;
62616	  r = _taskcall(SYSTASK, SYS_FRESH, &m);
62617	  *basep = (phys_clicks) m.m1_i1;
62618	  *sizep = (phys_clicks) m.m1_i2;
62619	  return(r);
.Ep 498 src/lib/syslib/sys_fresh.c
62620	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syslib/sys_getmap.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

62700	#include "syslib.h"
62701	
62702	PUBLIC int sys_getmap(proc, ptr)
62703	int proc;                       /* process whose map is to be fetched */
62704	struct mem_map *ptr;            /* pointer to new map */
62705	{
62706	/* Want to know map of a process, ask the kernel. */
62707	
62708	  message m;
62709	
62710	  m.m1_i1 = proc;
62711	  m.m1_p1 = (char *) ptr;
62712	  return(_taskcall(SYSTASK, SYS_GETMAP, &m));
62713	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syslib/sys_getsp.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

62800	#include "syslib.h"
62801	
62802	PUBLIC int sys_getsp(proc, newsp)
62803	int proc;                       /* process whose sp is wanted */
62804	vir_bytes *newsp;               /* place to put sp read from kernel */
62805	{
62806	/* Ask the kernel what the sp is. */
62807	
62808	  message m;
62809	  int r;
62810	
62811	  m.m1_i1 = proc;
62812	  r = _taskcall(SYSTASK, SYS_GETSP, &m);
62813	  *newsp = (vir_bytes) m.STACK_PTR;
62814	  return(r);
62815	}
.Op 499 src/lib/syslib/sys_kill.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syslib/sys_kill.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

62900	#include "syslib.h"
62901	
62902	PUBLIC int sys_kill(proc, signr)
62903	int proc;                       /* which proc has exited */
62904	int signr;                      /* signal number: 1 - 16 */
62905	{
62906	/* A proc has to be signaled via MM.  Tell the kernel. */
62907	  message m;
62908	
62909	  m.m6_i1 = proc;
62910	  m.m6_i2 = signr;
62911	  return(_taskcall(SYSTASK, SYS_KILL, &m));
62912	}
	


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syslib/sys_newmap.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

63000	#include "syslib.h"
63001	
63002	PUBLIC int sys_newmap(proc, ptr)
63003	int proc;                       /* process whose map is to be changed */
63004	struct mem_map *ptr;            /* pointer to new map */
63005	{
63006	/* A process has been assigned a new memory map.  Tell the kernel. */
63007	
63008	  message m;
63009	
63010	  m.m1_i1 = proc;
63011	  m.m1_p1 = (char *) ptr;
63012	  return(_taskcall(SYSTASK, SYS_NEWMAP, &m));
63013	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syslib/sys_oldsig.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

63100	#include "syslib.h"
63101	
63102	PUBLIC int sys_oldsig(proc, sig, sighandler)
63103	int proc;                       /* process to be signaled  */
63104	int sig;                        /* signal number: 1 to _NSIG */
63105	sighandler_t sighandler;        /* pointer to signal handler in user space */
63106	{
63107	/* A proc has to be signaled.  Tell the kernel. This function is obsolete. */
63108	
63109	  message m;
63110	
63111	  m.m6_i1 = proc;
63112	  m.m6_i2 = sig;
63113	  m.m6_f1 = sighandler;
63114	  return(_taskcall(SYSTASK, SYS_OLDSIG, &m));
.Ep 500 src/lib/syslib/sys_oldsig.c
63115	}





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syslib/sys_sendsig.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

63200	#include "syslib.h"
63201	
63202	PUBLIC int sys_sendsig(proc, smp)
63203	int proc;
63204	struct sigmsg *smp;
63205	{
63206	  message m;
63207	
63208	  m.m1_i1 = proc;
63209	  m.m1_p1 = (char *) smp;
63210	  return(_taskcall(SYSTASK, SYS_SENDSIG, &m));
63211	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syslib/sys_sigret.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

63300	#include "syslib.h"
63301	
63302	PUBLIC int sys_sigreturn(proc, scp, flags)
63303	int proc;
63304	vir_bytes scp;
63305	int flags;
63306	{
63307	  message m;
63308	
63309	  m.m1_i1 = proc;
63310	  m.m1_i2 = flags;
63311	  m.m1_p1 = (char *) scp;
63312	  return(_taskcall(SYSTASK, SYS_SIGRETURN, &m));
63313	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syslib/sys_times.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

63400	#include "syslib.h"
63401	
63402	PUBLIC int sys_times(proc, ptr)
63403	int proc;                       /* proc whose times are needed */
63404	clock_t ptr[5];                 /* pointer to time buffer */
63405	{
63406	/* Fetch the accounting info for a proc. */
63407	  message m;
63408	  int r;
63409	
.Op 501 src/lib/syslib/sys_times.c
63410	  m.m1_i1 = proc;
63411	  m.m1_p1 = (char *)ptr;
63412	  r = _taskcall(SYSTASK, SYS_TIMES, &m);
63413	  ptr[0] = m.USER_TIME;
63414	  ptr[1] = m.SYSTEM_TIME;
63415	  ptr[2] = m.CHILD_UTIME;
63416	  ptr[3] = m.CHILD_STIME;
63417	  ptr[4] = m.BOOT_TICKS;
63418	  return(r);
63419	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syslib/sys_trace.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

63500	#include "syslib.h"
63501	
63502	PUBLIC int sys_trace(req, procnr, addr, data_p)
63503	int req, procnr;
63504	long addr, *data_p;
63505	{
63506	  message m;
63507	  int r;
63508	
63509	  m.m2_i1 = procnr;
63510	  m.m2_i2 = req;
63511	  m.m2_l1 = addr;
63512	  if (data_p) m.m2_l2 = *data_p;
63513	  r = _taskcall(SYSTASK, SYS_TRACE, &m);
63514	  if (data_p) *data_p = m.m2_l2;
63515	  return(r);
63516	}




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syslib/sys_xit.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

63600	#include "syslib.h"
63601	
63602	PUBLIC int sys_xit(parent, proc, basep, sizep)
63603	int parent;                     /* parent of exiting process */
63604	int proc;                       /* which process has exited */
63605	phys_clicks *basep;             /* where to return base of shadow [68000] */
63606	phys_clicks *sizep;             /* where to return size of shadow [68000] */
63607	{
63608	/* A process has exited.  Tell the kernel. */
63609	
63610	  message m;
63611	  int r;
63612	
63613	  m.m1_i1 = parent;
63614	  m.m1_i2 = proc;
.Ep 502 src/lib/syslib/sys_xit.c
63615	  r = _taskcall(SYSTASK, SYS_XIT, &m);
63616	  *basep = (phys_clicks) m.m1_i1;
63617	  *sizep = (phys_clicks) m.m1_i2;
63618	  return(r);
63619	}






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/syslib/taskcall.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

63700	/* _taskcall() is the same as _syscall() except it returns negative error
63701	 * codes directly and not in errno.  This is a better interface for MM and
63702	 * FS.
63703	 */
63704	
63705	#include <lib.h>
63706	#include <minix/syslib.h>
63707	
63708	PUBLIC int _taskcall(who, syscallnr, msgptr)
63709	int who;
63710	int syscallnr;
63711	register message *msgptr;
63712	{
63713	  int status;
63714	
63715	  msgptr->m_type = syscallnr;
63716	  status = _sendrec(who, msgptr);
63717	  if (status != 0) return(status);
63718	  return(msgptr->m_type);
63719	}





