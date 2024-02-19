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
.Op 1 src/lib/i86/em/adi.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/adi.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

00000	.define .adi
00001	
00002	        .text
00003	.adi:
00004	        pop     bx 
00005	        cmp     cx,#2
00006	        jne     1f
00007	        pop     cx
00008	        add     ax,cx
00009	        jmp     (bx)
00010	1:
00011	        cmp     cx,#4
00012	        jne     9f
00013	        pop     dx
00014	        pop     cx
00015	        add     ax,cx
00016	        pop     cx
00017	        adc     dx,cx
00018	        push    dx
00019	        jmp     (bx)
00020	9:
00021	.extern .trpilin
00022	        push    bx
00023	        jmp     .trpilin


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/and.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

00100	.define .and
00101	
00102	        ! #bytes in cx
00103	        ! save di; it might be a register variable
00104	
00105	        .text
00106	.and:
00107	        pop     bx              ! return address
00108	        mov     dx,di
00109	        mov     di,sp
00110	        add     di,cx
00111	        sar     cx,#1
00112	1:
00113	        pop     ax
00114	        and     ax,(di)
00115	        stos
00116	        loop    1b
00117	        mov     di,dx
00118	        jmp     (bx)
.Ep 2 src/lib/i86/em/blm.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/blm.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

00200	.define .blm
00201	.text
00202	
00203	        ! cx: count in words
00204	.blm:
00205	        mov     bx,sp
00206	        mov     ax,si
00207	        mov     dx,di
00208	        mov     di,2(bx)
00209	        mov     si,4(bx)
00210	        rep
00211	        mov
00212	        mov     si,ax
00213	        mov     di,dx
00214	        ret     4






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/cii.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

00300	.define .cii
00301	
00302	.text
00303	.cii:
00304	        pop     bx              ! return address
00305	                                ! pop     cx, dest. size
00306	                                ! pop     dx, src. size
00307	                                ! ax is first word of source
00308	        cmp     dx,#1
00309	        jne     2f
00310	        cbw
00311	        mov     dx,#2
00312	2:
00313	        cmp     dx,cx
00314	        je      8f
00315	        cmp     dx,#2
00316	        je      1f
00317	        cmp     dx,#4
00318	        jne     9f
00319	        cmp     cx,#2
00320	        jne     9f
00321	        pop     dx
00322	8:
00323	        jmp     (bx)
00324	1:
00325	        cmp     cx,#4
00326	        jne     9f
00327	        cwd
00328	        push    dx
00329	        jmp     (bx)
.Op 3 src/lib/i86/em/cii.s
00330	9:
00331	        push    ax              ! push low source
00332	EILLINS = 18
00333	.extern .fat
00334	        mov     ax,#EILLINS
00335	        push    ax
00336	        jmp     .fat




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/cmi4.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

00400	.define .cmi4
00401	
00402	.text
00403	.cmi4:
00404	        pop     bx              ! return address
00405	        pop     cx
00406	        pop     dx
00407	        pop     ax
00408	        push    si
00409	        mov     si,sp
00410	        xchg    bx,2(si)
00411	        pop     si
00412	        cmp     bx,dx
00413	        jg      1f
00414	        jl      2f
00415	        cmp     ax,cx
00416	        ja      1f
00417	        je      3f
00418	2:
00419	        mov     ax,#-1
00420	        ret
00421	3:
00422	        xor     ax,ax
00423	        ret
00424	1:
00425	        mov     ax,#1
00426	        ret




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/cms.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

00500	.define .cms
00501	
00502	        ! #bytes in cx
00503	        .text
00504	.cms:
00505	        pop     bx              ! return address
00506	        mov     dx,sp
00507	        push    si
00508	        push    di
00509	        mov     si,dx
.Ep 4 src/lib/i86/em/cms.s
00510	        add     dx,cx
00511	        mov     di,dx
00512	        add     dx,cx
00513	        sar     cx,#1
00514	        repe
00515	        cmp
00516	        je      1f
00517	        inc     cx
00518	1:
00519	        pop     di
00520	        pop     si
00521	        mov     sp,dx
00522	        jmp     (bx)



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/cmu4.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

00600	.define .cmu4
00601	
00602	.text
00603	.cmu4:
00604	        pop     bx              ! return address
00605	        pop     cx
00606	        pop     dx
00607	        pop     ax
00608	        push    si
00609	        mov     si,sp
00610	        xchg    bx,2(si)
00611	        pop     si
00612	        cmp     bx,dx
00613	        ja      1f
00614	        jb      2f
00615	        cmp     ax,cx
00616	        ja      1f
00617	        je      3f
00618	2:
00619	        mov     ax,#-1
00620	        ret
00621	3:
00622	        xor     ax,ax
00623	        ret
00624	1:
00625	        mov     ax,#1
00626	        ret
.Op 5 src/lib/i86/em/com.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/com.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

00700	.define .com
00701	
00702	        ! #bytes in cx
00703	        .text
00704	.com:
00705	        mov     bx,sp
00706	        inc     bx
00707	        inc     bx
00708	        sar     cx,#1
00709	1:
00710	        not     (bx)
00711	        inc     bx
00712	        inc     bx
00713	        loop    1b
00714	        ret






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/csa2.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

00800	.define .csa2
00801	
00802	.text
00803	.csa2:
00804	                                ! bx, descriptor address
00805	                                ! ax, index
00806	        mov     dx,(bx)         ! default
00807	        sub     ax,2(bx)
00808	        cmp     ax,4(bx)
00809	        ja      1f
00810	        sal     ax,#1
00811	        add     bx,ax
00812	        mov     bx,6(bx)
00813	        test    bx,bx
00814	        jnz     2f
00815	1:
00816	        mov     bx,dx
00817	        test    bx,bx
00818	        jnz     2f
00819	ECASE = 20
00820	.extern .fat
00821	        mov     ax,#ECASE
00822	        push    ax
00823	        jmp     .fat
00824	2:
00825	        jmp     (bx)
.Ep 6 src/lib/i86/em/csa4.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/csa4.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

00900	.sect .text; .sect .rom; .sect .data; .sect .bss
00901	.define .csa4
00902	
00903	.sect .text
00904	.csa4:
00905	                                ! bx, descriptor address
00906	                                ! ax, dx: index
00907	        mov     cx,(bx)         ! default
00908	        sub     ax,2(bx)
00909	                                ! ignore high order word; if non-zero, the
00910	                                ! case descriptor would not fit anyway
00911	        cmp     ax,6(bx)
00912	        ja      1f
00913	2:
00914	        sal     ax,#1
00915	        add     bx,ax
00916	        mov     bx,10(bx)
00917	        test    bx,bx
00918	        jnz     2f
00919	1:
00920	        mov     bx,cx
00921	        test    bx,bx
00922	        jnz     2f
00923	ECASE = 20
00924	.extern .fat
00925	        mov     ax,#ECASE
00926	        push    ax
00927	        jmp     .fat
00928	2:
00929	        jmp     (bx)






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/csb2.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

01000	.define .csb2
01001	
01002	.text
01003	.csb2:
01004	                                !bx, descriptor address
01005	                                !ax,  index
01006	        mov     dx,(bx)
01007	        mov     cx,2(bx)
01008	1:
01009	        add     bx,#4
01010	        dec     cx
01011	        jl      4f
01012	        cmp     ax,(bx)
01013	        jne     1b
01014	        mov     bx,2(bx)
.Op 7 src/lib/i86/em/csb2.s
01015	2:
01016	        test    bx,bx
01017	        jnz     3f
01018	ECASE = 20
01019	.extern .fat
01020	        mov     ax,#ECASE
01021	        push    ax
01022	        jmp     .fat
01023	3:
01024	        jmp     (bx)
01025	4:
01026	        mov     bx,dx
01027	        jmp     2b



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/csb4.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

01100	.sect .text; .sect .rom; .sect .data; .sect .bss
01101	.define .csb4
01102	
01103	.sect .text
01104	.csb4:
01105	                                !bx: descriptor address
01106	                                !ax, dx:  index
01107	        push    (bx)            ! default
01108	        mov     cx,2(bx)        ! count (ignore high order word, the descriptor
01109	                                ! would not fit anyway)
01110	1:
01111	        add     bx,#6
01112	        dec     cx
01113	        jl      4f
01114	        cmp     ax,(bx)
01115	        jne     1b
01116	        cmp     dx,2(bx)
01117	        jne     1b
01118	        pop     bx
01119	        mov     bx,4(bx)
01120	2:
01121	        test    bx,bx
01122	        jnz     3f
01123	ECASE = 20
01124	.extern .fat
01125	        mov     ax,#ECASE
01126	        push    ax
01127	        jmp     .fat
01128	3:
01129	        jmp     (bx)
01130	4:
01131	        pop     bx
01132	        jmp     2b
.Ep 8 src/lib/i86/em/cuu.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/cuu.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

01200	.define .ciu
01201	.define .cui
01202	.define .cuu
01203	
01204	.text
01205	.ciu:
01206	.cui:
01207	.cuu:
01208	        pop     bx              ! return address
01209	                                ! pop     cx, dest. size
01210	                                ! pop     dx, source size
01211	                                ! ax is low word of source
01212	        cmp     dx,cx
01213	        je      8f
01214	        cmp     dx,#2
01215	        je      1f
01216	        cmp     dx,#4
01217	        jne     9f
01218	        cmp     cx,#2
01219	        jne     9f
01220	        pop     dx
01221	8:
01222	        jmp     (bx)
01223	1:
01224	        cmp     cx,#4
01225	        jne     9f
01226	        xor     dx,dx
01227	        push    dx
01228	        jmp     (bx)
01229	9:
01230	        push    ax              ! to help debugging ?
01231	EILLINS = 18
01232	.extern .fat
01233	        mov     ax,#EILLINS
01234	        push    ax
01235	        jmp     .fat





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/dup.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

01300	.define .dup
01301	
01302	        ! #bytes in cx
01303	        .text
01304	.dup:
01305	        pop     bx              ! return address
01306	        mov     ax,si
01307	        mov     dx,di
01308	        mov     si,sp
01309	        sub     sp,cx
.Op 9 src/lib/i86/em/dup.s
01310	        mov     di,sp
01311	        sar     cx,#1
01312	        rep
01313	        mov
01314	        mov     si,ax
01315	        mov     di,dx
01316	        jmp     (bx)




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/dvi4.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

01400	.define .dvi4
01401	
01402	yl=6
01403	yh=8
01404	xl=10
01405	xh=12
01406	
01407	.text
01408	.dvi4:
01409	        push    si
01410	        push    di
01411	        mov     si,sp           ! copy of sp
01412	        mov     bx,yl(si)
01413	        mov     ax,yh(si)
01414	        cwd
01415	        mov     di,dx
01416	        cmp     dx,ax
01417	        jne     7f
01418	        and     dx,dx
01419	        jge     1f
01420	        neg     bx
01421	        je      7f
01422	1:
01423	        xor     dx,dx
01424	        mov     cx,xl(si)
01425	        mov     ax,xh(si)
01426	        and     ax,ax
01427	        jge     2f
01428	        neg     ax
01429	        neg     cx
01430	        sbb     ax,dx
01431	        not     di
01432	2:
01433	        div     bx
01434	        xchg    ax,cx
01435	        div     bx              ! cx = high abs(result), ax=low abs(result)
01436	9:
01437	        and     di,di
01438	        jge     1f
01439	        neg     cx
01440	        neg     ax
01441	        sbb     cx,#0
01442	1:
01443	                        ! cx is high order result
01444	                        ! ax is low order result
.Ep 10 src/lib/i86/em/dvi4.s
01445	        mov     dx,cx
01446	        pop     di
01447	        pop     si
01448	        ret     8       ! result in ax/dx
01449	
01450	7:
01451	        push    dx              ! sign of y
01452	        mov     di,ax
01453	        xor     bx,bx
01454	        and     di,di
01455	        jge     1f
01456	        neg     di
01457	        neg     yl(si)
01458	        sbb     di,bx
01459	1:
01460	        mov     ax,xl(si)
01461	        mov     dx,xh(si)
01462	        and     dx,dx
01463	        jge     1f
01464	        neg     dx
01465	        neg     ax
01466	        sbb     dx,bx
01467	        not     -2(si)
01468	1:
01469	        mov     cx,#16
01470	1:
01471	        shl     ax,#1
01472	        rcl     dx,#1
01473	        rcl     bx,#1
01474	        cmp     di,bx
01475	        ja      3f
01476	        jb      2f
01477	        cmp     yl(si),dx
01478	        jbe     2f
01479	3:
01480	        loop    1b
01481	        jmp     1f
01482	2:
01483	        sub     dx,yl(si)
01484	        sbb     bx,di
01485	        inc     ax
01486	        loop    1b
01487	1:
01488	        pop     di              ! di=sign of result,ax= result
01489	        jmp     9b
.Op 11 src/lib/i86/em/dvu4.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/dvu4.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

01500	.define .dvu4
01501	
01502	yl=6
01503	yh=8
01504	xl=10
01505	xh=12
01506	
01507	.text
01508	.dvu4:
01509	        push    si
01510	        push    di
01511	        mov     si,sp           ! copy of sp
01512	        mov     bx,yl(si)
01513	        mov     ax,yh(si)
01514	        or      ax,ax
01515	        jne     7f
01516	        xor     dx,dx
01517	        mov     cx,xl(si)
01518	        mov     ax,xh(si)
01519	        div     bx
01520	        xchg    ax,cx
01521	        div     bx
01522	9:
01523	                        ! cx is high order result
01524	                        ! ax is low order result
01525	        mov     dx,cx
01526	        pop     di
01527	        pop     si
01528	        ret     8       ! result in ax/dx
01529	
01530	7:
01531	        mov     di,ax
01532	        xor     bx,bx
01533	        mov     ax,xl(si)
01534	        mov     dx,xh(si)
01535	        mov     cx,#16
01536	1:
01537	        shl     ax,#1
01538	        rcl     dx,#1
01539	        rcl     bx,#1
01540	        cmp     di,bx
01541	        ja      3f
01542	        jb      2f
01543	        cmp     yl(si),dx
01544	        jbe     2f
01545	3:
01546	        loop    1b
01547	        jmp     9b
01548	2:
01549	        sub     dx,yl(si)
01550	        sbb     bx,di
01551	        inc     ax
01552	        loop    1b
01553	        jmp     9b
.Ep 12 src/lib/i86/em/error.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/error.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

01600	.define .error
01601	.define .Xtrp
01602	
01603	        ! ax is trap number
01604	        ! all registers must be saved
01605	        ! because return is possible
01606	        ! May only be called with error no's <16
01607	.text
01608	.error:
01609	        push bp
01610	        push si
01611	        push di
01612	        push dx
01613	        push cx
01614	        push bx
01615	        push ax
01616	        mov  cx,ax
01617	        mov  bx,#1
01618	        sal  bx,cl
01619	.extern .ignmask
01620	.extern .trp
01621	        test bx,.ignmask
01622	        jne  2f
01623	        call    .trp
01624	2:
01625	        pop  ax
01626	        pop  bx
01627	        pop  cx
01628	        pop  dx
01629	        pop  di
01630	        pop  si
01631	        pop  bp
01632	        ret
01633	
01634	.Xtrp:
01635	        cmp     ax,#16
01636	        jge     1f
01637	        call    .error
01638	        ret
01639	1:
01640	        jmp     .trp
.Op 13 src/lib/i86/em/exg.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/exg.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

01700	.define .exg
01701	
01702	        ! #bytes in cx
01703	.text
01704	.exg:
01705	        push    di
01706	        mov     sp,di
01707	        add     di,#4
01708	        mov     bx,di
01709	        add     bx,cx
01710	        sar     cx,#1
01711	1:
01712	        mov     ax,(bx)
01713	        xchg    ax,(di)
01714	        mov     (bx),ax
01715	        add     di,#2
01716	        add     bx,#2
01717	        loop    1b
01718	2:
01719	        pop     di
01720	        ret





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/fat.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

01800	.define .fat
01801	.text
01802	
01803	.fat:
01804	.extern .trp
01805	.extern .stop
01806	        call    .trp
01807	        call    .stop
01808	        ! no return


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/fp8087.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

01900	.define .adf4, .adf8, .sbf4, .sbf8, .mlf4, .mlf8, .dvf4, .dvf8
01901	.define .ngf4, .ngf8, .fif4, .fif8, .fef4, .fef8
01902	.define .cif4, .cif8, .cuf4, .cuf8, .cfi, .cfu, .cff4, .cff8
01903	.define .cmf4, .cmf8
01904	.sect .text; .sect .rom; .sect .data; .sect .bss
01905	
01906	!       $Header: fp8087.s,v 1.2 89/11/28 12:51:35 ceriel Exp $
01907	
01908	!       Implement interface to floating point package for Intel 8087
01909	
.Ep 14 src/lib/i86/em/fp8087.s
01910	        .sect .rom
01911	one:
01912	        .data2  1
01913	two:
01914	        .data2  2
01915	bigmin:
01916	        .data2  0, -32768
01917	
01918	        .sect .text
01919	.adf4:
01920	        mov     bx,sp
01921	        wait
01922	        flds    2(bx)
01923	        wait
01924	        fadds   6(bx)
01925	        wait
01926	        fstps   6(bx)
01927	        wait
01928	        ret
01929	.adf8:
01930	        mov     bx,sp
01931	        wait
01932	        fldd    2(bx)
01933	        wait
01934	        faddd   10(bx)
01935	        wait
01936	        fstpd   10(bx)
01937	        wait
01938	        ret
01939	
01940	.sbf4:
01941	        mov     bx,sp
01942	        wait
01943	        flds    6(bx)
01944	        wait
01945	        fsubs   2(bx)
01946	        wait
01947	        fstps   6(bx)
01948	        wait
01949	        ret
01950	
01951	.sbf8:
01952	        mov     bx,sp
01953	        wait
01954	        fldd    10(bx)
01955	        wait
01956	        fsubd   2(bx)
01957	        wait
01958	        fstpd   10(bx)
01959	        wait
01960	        ret
01961	
01962	.mlf4:
01963	        mov     bx,sp
01964	        wait
01965	        flds    2(bx)
01966	        wait
01967	        fmuls   6(bx)
01968	        wait
01969	        fstps   6(bx)
.Op 15 src/lib/i86/em/fp8087.s
01970	        wait
01971	        ret
01972	.mlf8:
01973	        mov     bx,sp
01974	        wait
01975	        fldd    2(bx)
01976	        wait
01977	        fmuld   10(bx)
01978	        wait
01979	        fstpd   10(bx)
01980	        wait
01981	        ret
01982	
01983	.dvf4:
01984	        mov     bx,sp
01985	        wait
01986	        flds    6(bx)
01987	        wait
01988	        fdivs   2(bx)
01989	        wait
01990	        fstps   6(bx)
01991	        wait
01992	        ret
01993	
01994	.dvf8:
01995	        mov     bx,sp
01996	        wait
01997	        fldd    10(bx)
01998	        wait
01999	        fdivd   2(bx)
02000	        wait
02001	        fstpd   10(bx)
02002	        wait
02003	        ret
02004	
02005	.ngf4:
02006	        mov     bx,sp
02007	        wait
02008	        flds    2(bx)
02009	        wait
02010	        fchs
02011	        wait
02012	        fstps   2(bx)
02013	        wait
02014	        ret
02015	
02016	.ngf8:
02017	        mov     bx,sp
02018	        wait
02019	        fldd    2(bx)
02020	        wait
02021	        fchs
02022	        wait
02023	        fstpd   2(bx)
02024	        wait
02025	        ret
02026	
02027	.fif4:
02028	        mov     bx,sp
02029	        push    bx              ! make room for FP status word
.Ep 16 src/lib/i86/em/fp8087.s
02030	        wait
02031	        flds    4(bx)
02032	        wait
02033	        fmuls   8(bx)           ! multiply
02034	        wait
02035	        fld     st              ! copy result
02036	        wait
02037	        ftst                    ! test sign; handle negative separately
02038	        wait
02039	        fstsw   -2(bx)
02040	        wait
02041	        mov     ax,-2(bx)
02042	        sahf                    ! result of test in condition codes
02043	        jb      1f
02044	        frndint                 ! this one rounds (?)
02045	        wait
02046	        fcom    st(1)           ! compare with original; if <=, then OK
02047	        wait
02048	        fstsw   -2(bx)
02049	        wait
02050	        mov     ax,-2(bx)
02051	        sahf
02052	        jbe     2f
02053	        fisubs  one             ! else subtract 1
02054	        wait
02055	        jmp     2f
02056	1:                              ! here, negative case
02057	        frndint                 ! this one rounds (?)
02058	        wait
02059	        fcom    st(1)           ! compare with original; if >=, then OK
02060	        wait
02061	        fstsw   -2(bx)
02062	        wait
02063	        mov     ax,-2(bx)
02064	        sahf
02065	        jae     2f
02066	        fiadds  one             ! else add 1
02067	        wait
02068	2:
02069	        fsub    st(1),st        ! subtract integer part
02070	        wait
02071	        mov     bx,2(bx)
02072	        fstps   (bx)
02073	        wait
02074	        fstps   4(bx)
02075	        wait
02076	        pop     bx
02077	        ret
02078	
02079	.fif8:
02080	        mov     bx,sp
02081	        push    bx              ! make room for FP status word
02082	        wait
02083	        fldd    4(bx)
02084	        wait
02085	        fmuld   12(bx)          ! multiply
02086	        wait
02087	        fld     st              ! and copy result
02088	        wait
02089	        ftst                    ! test sign; handle negative separately
.Op 17 src/lib/i86/em/fp8087.s
02090	        wait
02091	        fstsw   -2(bx)
02092	        wait
02093	        mov     ax,-2(bx)
02094	        sahf                    ! result of test in condition codes
02095	        jb      1f
02096	        frndint                 ! this one rounds (?)
02097	        wait
02098	        fcom    st(1)           ! compare with original; if <=, then OK
02099	        wait
02100	        fstsw   -2(bx)
02101	        wait
02102	        mov     ax,-2(bx)
02103	        sahf
02104	        jbe     2f
02105	        fisubs  one             ! else subtract 1
02106	        wait
02107	        jmp     2f
02108	1:                              ! here, negative case
02109	        frndint                 ! this one rounds (?)
02110	        wait
02111	        fcom    st(1)           ! compare with original; if >=, then OK
02112	        wait
02113	        fstsw   -2(bx)
02114	        wait
02115	        mov     ax,-2(bx)
02116	        sahf
02117	        jae     2f
02118	        fiadds  one             ! else add 1
02119	        wait
02120	2:
02121	        fsub    st(1),st        ! subtract integer part
02122	        mov     bx,2(bx)
02123	        fstpd   (bx)
02124	        wait
02125	        fstpd   8(bx)
02126	        wait
02127	        pop     bx
02128	        ret
02129	
02130	.fef4:
02131	                                ! this could be simpler, if only the
02132	                                ! fxtract instruction was emulated properly
02133	        mov     bx,sp
02134	        mov     ax,6(bx)
02135	        and     ax,#077600
02136	        je      1f              ! zero exponent
02137	        mov     cx,#7
02138	        shr     ax,cl
02139	        sub     ax,#126
02140	        mov     cx,ax           ! exponent in cx
02141	        mov     ax,6(bx)
02142	        and     ax,#0100177
02143	        or      ax,#0037400     ! load -1 exponent
02144	        mov     dx,4(bx)
02145	        mov     bx,2(bx)
02146	        mov     4(bx),ax
02147	        mov     2(bx),dx
02148	        mov     (bx),cx
02149	        ret
.Ep 18 src/lib/i86/em/fp8087.s
02150	1:                              ! we get here on zero exp
02151	        mov     ax,6(bx)
02152	        and     ax,#0177
02153	        or      ax,4(bx)
02154	        jne     1f              ! zero result
02155	        xor     ax,ax
02156	        mov     bx,2(bx)
02157	        mov     (bx),ax
02158	        mov     2(bx),ax
02159	        mov     4(bx),ax
02160	        ret
02161	1:                              ! otherwise unnormalized number
02162	        mov     cx,6(bx)
02163	        and     cx,#0100177
02164	        mov     dx,cx
02165	        and     cx,#0x8000
02166	        mov     ax,#-125
02167	2:
02168	        test    dx,#0x80
02169	        jne     1f
02170	        dec     ax
02171	        shl     4(bx),#1
02172	        rcl     dx,#1
02173	        or      dx,cx
02174	        jmp     2b
02175	1:
02176	        mov     cx,4(bx)
02177	        mov     bx,2(bx)
02178	        mov     (bx),ax
02179	        mov     2(bx),cx
02180	        and     dx,#0100177
02181	        or      dx,#0037400     ! load -1 exponent
02182	        mov     4(bx),dx
02183	        ret
02184	
02185	.fef8:
02186	                                ! this could be simpler, if only the
02187	                                ! fxtract instruction was emulated properly
02188	        mov     bx,sp
02189	        mov     ax,10(bx)
02190	        and     ax,#077760
02191	        je      1f              ! zero exponent
02192	        mov     cx,#4
02193	        shr     ax,cl
02194	        sub     ax,#1022
02195	        mov     cx,ax           ! exponent in cx
02196	        mov     ax,10(bx)
02197	        and     ax,#0100017
02198	        or      ax,#0037740     ! load -1 exponent
02199	        push    8(bx)
02200	        push    6(bx)
02201	        push    4(bx)
02202	        mov     bx,2(bx)
02203	        pop     2(bx)
02204	        pop     4(bx)
02205	        pop     6(bx)
02206	        mov     8(bx),ax
02207	        mov     (bx),cx
02208	        ret
02209	1:                              ! we get here on zero exp
.Op 19 src/lib/i86/em/fp8087.s
02210	        mov     ax,10(bx)
02211	        and     ax,#017
02212	        or      ax,8(bx)
02213	        or      ax,6(bx)
02214	        or      ax,4(bx)
02215	        jne     1f              ! zero result
02216	        xor     ax,ax
02217	        mov     bx,2(bx)
02218	        mov     (bx),ax
02219	        mov     2(bx),ax
02220	        mov     4(bx),ax
02221	        mov     6(bx),ax
02222	        mov     8(bx),ax
02223	        ret
02224	1:                              ! otherwise unnormalized number
02225	        mov     cx,10(bx)
02226	        and     cx,#0100017
02227	        mov     dx,cx
02228	        and     cx,#0x8000
02229	        mov     ax,#-1021
02230	2:
02231	        test    dx,#0x10
02232	        jne     1f
02233	        dec     ax
02234	        shl     4(bx),#1
02235	        rcl     6(bx),#1
02236	        rcl     8(bx),#1
02237	        rcl     dx,#1
02238	        or      dx,cx
02239	        jmp     2b
02240	1:
02241	        and     dx,#0100017
02242	        or      dx,#0037740     ! load -1 exponent
02243	        mov     cx,8(bx)
02244	        push    6(bx)
02245	        push    4(bx)
02246	        mov     bx,2(bx)
02247	        mov     (bx),ax
02248	        mov     8(bx),dx
02249	        mov     6(bx),cx
02250	        pop     2(bx)
02251	        pop     4(bx)
02252	        ret
02253	
02254	.cif4:
02255	        mov     bx,sp
02256	        cmp     2(bx),#2
02257	        jne     1f
02258	        wait
02259	        filds   4(bx)
02260	        wait
02261	        fstps   2(bx)
02262	        wait
02263	        ret
02264	1:
02265	        wait
02266	        fildl   4(bx)
02267	        wait
02268	        fstps   4(bx)
02269	        wait
.Ep 20 src/lib/i86/em/fp8087.s
02270	        ret
02271	
02272	.cif8:
02273	        mov     bx,sp
02274	        cmp     2(bx),#2
02275	        jne     1f
02276	        wait
02277	        filds   4(bx)
02278	        wait
02279	        fstpd   2(bx)
02280	        wait
02281	        ret
02282	1:
02283	        wait
02284	        fildl   4(bx)
02285	        wait
02286	        fstpd   2(bx)
02287	        wait
02288	        ret
02289	
02290	.cuf4:
02291	        mov     bx,sp
02292	        cmp     2(bx),#2
02293	        jne     1f
02294	        mov     ax,4(bx)
02295	        mov     2(bx),ax
02296	        mov     4(bx),#0
02297	        wait
02298	        fildl   2(bx)
02299	        wait
02300	        fstps   2(bx)
02301	        wait
02302	        ret
02303	1:
02304	        wait
02305	        fildl   4(bx)
02306	        wait
02307	        cmp     6(bx),#0
02308	        jge     1f
02309	2:
02310	        wait
02311	        fisubl  bigmin
02312	        wait
02313	        fisubl  bigmin
02314	1:
02315	        wait
02316	        fstps   4(bx)
02317	        wait
02318	        ret
02319	
02320	.cuf8:
02321	        mov     bx,sp
02322	        cmp     2(bx),#2
02323	        jne     1f
02324	        mov     6(bx),#0
02325	1:
02326	        wait
02327	        fildl   4(bx)
02328	        wait
02329	        cmp     6(bx),#0
.Op 21 src/lib/i86/em/fp8087.s
02330	        jge     1f
02331	2:
02332	        wait
02333	        fisubl  bigmin
02334	        wait
02335	        fisubl  bigmin
02336	1:
02337	        wait
02338	        fstpd   2(bx)
02339	        wait
02340	        ret
02341	
02342	.cfi:
02343	        mov     bx,sp
02344	        push    bx
02345	        wait
02346	        fstcw   -2(bx)
02347	        wait
02348	        mov     dx,-2(bx)
02349	        or      -2(bx),#0xc00   ! truncating mode
02350	        wait
02351	        fldcw   -2(bx)
02352	        pop     ax
02353	        cmp     4(bx),#4
02354	        jne     2f
02355	                                ! loc 4 loc ? cfi
02356	        wait
02357	        flds    6(bx)
02358	        wait
02359	        fistpl  6(bx)
02360	        wait
02361	        cmp     2(bx),#2
02362	        jne     1f
02363	        mov     ax,6(bx)
02364	1:
02365	        mov     4(bx),dx
02366	        wait
02367	        fldcw   4(bx)
02368	        wait
02369	        ret
02370	2:
02371	                                ! loc 8 loc ? cfi
02372	        wait
02373	        fldd    6(bx)
02374	        wait
02375	        fistpl  10(bx)
02376	        wait
02377	        cmp     2(bx),#2
02378	        jne     1b
02379	        mov     ax,10(bx)
02380	        jmp     1b
02381	
02382	.cfu:
02383	        mov     bx,sp
02384	        push    bx
02385	        wait
02386	        fstcw   -2(bx)
02387	        wait
02388	        mov     dx,-2(bx)
02389	        and     -2(bx),#0xf3ff
.Ep 22 src/lib/i86/em/fp8087.s
02390	        or      -2(bx),#0x400   ! to -infinity
02391	        wait
02392	        fldcw   -2(bx)
02393	        wait
02394	        pop     ax
02395	        cmp     4(bx),#4
02396	        jne     2f
02397	                                ! loc 4 loc ? cfu
02398	        flds    6(bx)
02399	        wait
02400	        fabs                    ! ???
02401	        wait
02402	        fiaddl  bigmin
02403	        fistpl  6(bx)
02404	        wait
02405	        mov     ax,8(bx)
02406	        sub     ax,bigmin+2
02407	        mov     8(bx),ax
02408	        cmp     2(bx),#2
02409	        jne     1f
02410	        mov     ax,6(bx)
02411	1:
02412	        mov     4(bx),dx
02413	        wait
02414	        fldcw   4(bx)
02415	        wait
02416	        ret
02417	2:
02418	        wait
02419	                                ! loc 8 loc ? cfu
02420	        fldd    6(bx)
02421	        wait
02422	        fabs                    ! ???
02423	        wait
02424	        fiaddl  bigmin
02425	        fistpl  10(bx)
02426	        wait
02427	        mov     ax,12(bx)
02428	        sub     ax,bigmin+2
02429	        mov     12(bx),ax
02430	        cmp     2(bx),#2
02431	        jne     1b
02432	        mov     ax,10(bx)
02433	        jmp     1b
02434	
02435	.cff4:
02436	        mov     bx,sp
02437	        wait
02438	        fldd    2(bx)
02439	        wait
02440	        fstcw   2(bx)
02441	        wait
02442	        mov     dx,2(bx)
02443	        and     2(bx),#0xf3ff   ! set to rounding mode
02444	        wait
02445	        fldcw   2(bx)
02446	        wait
02447	        fstps   6(bx)
02448	        mov     2(bx),dx
02449	        wait
.Op 23 src/lib/i86/em/fp8087.s
02450	        fldcw   2(bx)
02451	        wait
02452	        ret
02453	
02454	.cff8:
02455	        mov     bx,sp
02456	        wait
02457	        flds    2(bx)
02458	        wait
02459	        fstpd   2(bx)
02460	        wait
02461	        ret
02462	
02463	.cmf4:
02464	        mov     bx,sp
02465	        push    bx              ! room for 8087 status word
02466	        xor     cx,cx
02467	        wait
02468	        flds    6(bx)
02469	        wait
02470	        flds    2(bx)
02471	        wait
02472	        fcompp                  ! compare and pop operands
02473	        wait
02474	        fstsw   -2(bx)
02475	        wait
02476	        mov     ax,-2(bx)
02477	        sahf
02478	        je      1f
02479	        jb      2f
02480	        dec     cx
02481	        jmp     1f
02482	2:
02483	        inc     cx
02484	1:
02485	        mov     ax,cx
02486	        pop     bx
02487	        ret
02488	
02489	
02490	.cmf8:
02491	        mov     bx,sp
02492	        push    bx              ! room for 8087 status word
02493	        xor     cx,cx
02494	        wait
02495	        fldd    10(bx)
02496	        wait
02497	        fldd    2(bx)
02498	        wait
02499	        fcompp                  ! compare and pop operands
02500	        wait
02501	        fstsw   -2(bx)
02502	        wait
02503	        mov     ax,-2(bx)
02504	        sahf
02505	        je      1f
02506	        jb      2f
02507	        dec     cx
02508	        jmp     1f
02509	2:
.Ep 24 src/lib/i86/em/fp8087.s
02510	        inc     cx
02511	1:
02512	        mov     ax,cx
02513	        pop     bx
02514	        ret






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/gto.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

02600	.define .gto
02601	.text
02602	
02603	.gto:
02604	        mov     bp,4(bx)
02605	        mov     sp,2(bx)
02606	        jmp     @(bx)




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/iaar.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

02700	.define .iaar
02701	.text
02702	
02703	.iaar:
02704	        pop     cx
02705	        pop     dx
02706	        cmp     dx,#2
02707	.extern .unknown
02708	        jne     .unknown
02709	        pop     bx      ! descriptor address
02710	        pop     ax      ! index
02711	        sub     ax,(bx)
02712	        mul     4(bx)
02713	        pop     bx      ! array base
02714	        add     bx,ax
02715	        push    cx
02716	        ret
.Op 25 src/lib/i86/em/ilar.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/ilar.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

02800	.define .ilar
02801	.text
02802	
02803	.ilar:
02804	        pop     cx
02805	        pop     dx
02806	.extern .unknown
02807	        cmp     dx,#2
02808	        jne     .unknown
02809	        pop     bx      ! descriptor address
02810	        pop     ax      ! index
02811	        push    cx
02812	.extern .lar2
02813	        jmp    .lar2


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/inn.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

02900	.define .inn
02901	.text
02902	
02903	        ! #bytes in cx
02904	        ! bit # in ax
02905	.inn:
02906	        xor     dx,dx
02907	        mov     bx,#8
02908	        div     bx
02909	        mov     bx,sp
02910	        add     bx,#2
02911	        add     bx,ax
02912	        cmp     ax,cx
02913	        jae     1f
02914	        movb    al,(bx)
02915	        mov     bx,dx
02916	        testb   al,bits(bx)
02917	        jz      1f
02918	        mov     ax,#1
02919	        jmp     2f
02920	1:
02921	        xor     ax,ax
02922	2:
02923	        pop     bx
02924	        add     sp,cx
02925	        ! ax is result
02926	        jmp     (bx)
02927	
02928	        .data
02929	bits:
02930	        .data1 1,2,4,8,16,32,64,128
.Ep 26 src/lib/i86/em/ior.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/ior.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

03000	.define .ior
03001	.text
03002	
03003	        ! #bytes in cx
03004	.ior:
03005	        pop     bx              ! return address
03006	        mov     dx,di
03007	        mov     di,sp
03008	        add     di,cx
03009	        sar     cx,#1
03010	1:
03011	        pop     ax
03012	        or      ax,(di)
03013	        stos
03014	        loop    1b
03015	        mov     di,dx
03016	        jmp     (bx)




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/isar.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

03100	.define .isar
03101	.text
03102	
03103	.isar:
03104	        pop     cx
03105	        pop     ax
03106	        cmp     ax,#2
03107	.extern .unknown
03108	        jne     .unknown
03109	        pop     bx      ! descriptor address
03110	        pop     ax      ! index
03111	        push    cx
03112	.extern .sar2
03113	        jmp    .sar2


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/lar2.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

03200	.define .lar2
03201	.text
03202	
03203	.lar2:
03204	                                ! bx, descriptor address
03205	                                ! ax, index
03206	        pop     cx
03207	        pop     dx              ! base address
03208	        push    cx
03209	        push    si
.Op 27 src/lib/i86/em/lar2.s
03210	        mov     si,dx
03211	        sub     ax,(bx)
03212	        mov     cx,4(bx)
03213	        imul    cx
03214	        add     si,ax
03215	        sar     cx,#1
03216	        jnb     1f
03217	        xorb    ah,ah
03218	        lodsb
03219	        pop     si
03220	        pop     bx
03221	        push    ax
03222	        jmp     (bx)
03223	1:
03224	        pop     dx              ! saved si
03225	        mov     ax,4(bx)
03226	        pop     bx              ! return address
03227	        sub     sp,ax
03228	        mov     ax,di           ! save di
03229	        mov     di,sp
03230	        rep
03231	        mov
03232	        mov     di,ax
03233	        mov     si,dx
03234	        jmp     (bx)






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/lfr6.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

03300	.define .lfr6
03301	.text
03302	.extern .retarea
03303	
03304	.lfr6:
03305	        pop     bx
03306	        push    .retarea+4
03307	        push    .retarea+2
03308	        push    .retarea
03309	        jmp     (bx)
.Ep 28 src/lib/i86/em/lfr8.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/lfr8.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

03400	.define .lfr8
03401	.text
03402	.extern .retarea
03403	
03404	.lfr8:
03405	        pop     bx
03406	        push    .retarea+6
03407	        push    .retarea+4
03408	        push    .retarea+2
03409	        push    .retarea
03410	        jmp     (bx)





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/loi.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

03500	.define .loi
03501	.define .los
03502	.text
03503	
03504	        ! #bytes in cx
03505	        ! address in bx
03506	        ! save si/di. they might be register variables
03507	.los:
03508	        mov     dx,si
03509	        mov     si,bx
03510	        pop     bx
03511	        mov     ax,cx
03512	        sar     cx,#1
03513	        jnb     1f
03514	        xorb    ah,ah
03515	        lodsb
03516	        mov     si,dx
03517	        push    ax
03518	        jmp     (bx)
03519	1:
03520	        sub     sp,ax
03521	        jmp     1f
03522	
03523	.loi:
03524	        ! only called with size > 4
03525	        mov     dx,si
03526	        mov     si,bx
03527	        pop     bx
03528	        sub     sp,cx
03529	        sar     cx,#1
03530	1:
03531	        mov     ax,di
03532	        mov     di,sp
03533	        rep
03534	        mov
.Op 29 src/lib/i86/em/loi.s
03535	        mov     si,dx
03536	        mov     di,ax
03537	        jmp     (bx)



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/mli4.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

03600	.define .mli4
03601	.text
03602	
03603	yl=2
03604	yh=4
03605	        ! x * y
03606	        ! xl in ax
03607	        ! xh in dx
03608	
03609	.mli4:
03610	        mov     bx,sp
03611	        push    dx
03612	        mov     cx,ax
03613	        mul     yh(bx)           ! xl*yh
03614	        pop     dx
03615	        push    ax
03616	        mov     ax,dx
03617	        mul     yl(bx)          ! xh * yl
03618	        pop     dx
03619	        add     dx,ax           ! xh*yl+xl*yh
03620	        mov     ax,cx
03621	        mov     cx,dx
03622	        mul     yl(bx)           ! xl*yl
03623	        add     dx,cx
03624	        ret     4






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/mon.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

03700	.define .mon
03701	.text
03702	
03703	.mon:
03704	.extern .stop
03705	        call    .stop
.Ep 30 src/lib/i86/em/nop.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/nop.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

03800	.define .nop
03801	.text
03802	.nop:
03803	        ret


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/rck.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

03900	.define .rck
03901	.text
03902	
03903	        ! descriptor address in bx
03904	        ! value in ax, must be left there
03905	.rck:
03906	        cmp     ax,(bx)
03907	        jl      2f
03908	        cmp     ax,2(bx)
03909	        jg      2f
03910	        ret
03911	2:
03912	        push    ax
03913	ERANGE = 1
03914	.extern .error
03915	        mov     ax,#ERANGE
03916	        call    .error
03917	        pop     ax
03918	        ret


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/ret6.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

04000	.define .ret6
04001	.text
04002	.extern .retarea
04003	
04004	.ret6:
04005	        pop     bx
04006	        pop     .retarea
04007	        pop     .retarea+2
04008	        pop     .retarea+4
04009	        jmp     (bx)
.Op 31 src/lib/i86/em/ret8.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/ret8.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

04100	.define .ret8
04101	.text
04102	.extern .retarea
04103	
04104	.ret8:
04105	        pop     bx
04106	        pop     .retarea
04107	        pop     .retarea+2
04108	        pop     .retarea+4
04109	        pop     .retarea+6
04110	        jmp     (bx)





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/retarea.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

04200	.define .retarea
04201	
04202	.bss
04203	.retarea:
04204	        .zerow 8/2






++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/return.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

04300	.define .sdret, .dsret, .sret, .dret, .cret 
04301	.text
04302	
04303	.dsret:
04304	        pop     di
04305	.sret:
04306	        pop     si
04307	.cret:
04308	        mov     sp,bp
04309	        pop     bp
04310	        ret
04311	
04312	.sdret:
04313	        pop     si
04314	.dret:
04315	        pop     di
04316	        jmp     .cret
.Ep 32 src/lib/i86/em/rmi4.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/rmi4.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

04400	.define .rmi4
04401	.text
04402	
04403	yl=6
04404	yh=8
04405	xl=10
04406	xh=12
04407	
04408	.rmi4:
04409	        push    si
04410	        push    di
04411	        mov     si,sp           ! copy of sp
04412	        mov     bx,yl(si)
04413	        mov     ax,yh(si)
04414	        cwd
04415	        cmp     dx,ax
04416	        jne     7f
04417	        and     dx,dx
04418	        jge     1f
04419	        neg     bx
04420	        je      7f
04421	1:
04422	        xor     dx,dx
04423	        mov     cx,xl(si)
04424	        mov     ax,xh(si)
04425	        and     ax,ax
04426	        jge     2f
04427	        neg     ax
04428	        neg     cx
04429	        sbb     ax,dx
04430	2:
04431	        div     bx
04432	        xchg    ax,cx
04433	        div     bx              ! dx= result(low), 0=result(high)
04434	        xor     bx,bx
04435	9:
04436	        cmp     xh(si),#0
04437	        jge     1f
04438	        neg     bx
04439	        neg     dx
04440	        sbb     bx,#0
04441	1:
04442	                        ! bx is high order result
04443	                        ! dx is low order result
04444	        mov     ax,dx
04445	        mov     dx,bx   ! result in ax/dx
04446	        pop     di
04447	        pop     si
04448	        ret     8
04449	
04450	7:
04451	        mov     di,ax
04452	        xor     bx,bx
04453	        and     di,di
04454	        jge     1f
.Op 33 src/lib/i86/em/rmi4.s
04455	        neg     di
04456	        neg     yl(si)
04457	        sbb     di,bx
04458	1:
04459	        mov     ax,xl(si)
04460	        mov     dx,xh(si)
04461	        and     dx,dx
04462	        jge     1f
04463	        neg     dx
04464	        neg     ax
04465	        sbb     dx,bx
04466	1:
04467	        mov     cx,#16
04468	1:
04469	        shl     ax,#1
04470	        rcl     dx,#1
04471	        rcl     bx,#1
04472	        cmp     di,bx
04473	        ja      3f
04474	        jb      2f
04475	        cmp     yl(si),dx
04476	        jbe     2f
04477	3:
04478	        loop    1b
04479	                ! dx=result(low), bx=result(high)
04480	        jmp     9b
04481	2:
04482	        sub     dx,yl(si)
04483	        sbb     bx,di
04484	        inc     ax
04485	        loop    1b
04486	1:
04487	                ! dx=result(low), bx=result(high)
04488	        jmp     9b


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/rmu4.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

04500	.define .rmu4
04501	.text
04502	
04503	yl=6
04504	yh=8
04505	xl=10
04506	xh=12
04507	
04508	.rmu4:
04509	        push    si
04510	        push    di
04511	        mov     si,sp           ! copy of sp
04512	        mov     bx,yl(si)
04513	        mov     ax,yh(si)
04514	        or      ax,ax
04515	        jne     7f
04516	1:
04517	        xor     dx,dx
04518	        mov     cx,xl(si)
04519	        mov     ax,xh(si)
.Ep 34 src/lib/i86/em/rmu4.s
04520	2:
04521	        div     bx
04522	        xchg    ax,cx
04523	        div     bx
04524	        xor     bx,bx
04525	9:
04526	                        ! bx is high order result
04527	                        ! dx is low order result
04528	        mov     ax,dx
04529	        mov     dx,bx
04530	        pop     di
04531	        pop     si
04532	        ret     8       ! result in ax/dx
04533	
04534	7:
04535	        mov     di,ax
04536	        xor     bx,bx
04537	        mov     ax,xl(si)
04538	        mov     dx,xh(si)
04539	        mov     cx,#16
04540	1:
04541	        shl     ax,#1
04542	        rcl     dx,#1
04543	        rcl     bx,#1
04544	        cmp     di,bx
04545	        ja      3f
04546	        jb      2f
04547	        cmp     yl(si),dx
04548	        jbe     2f
04549	3:
04550	        loop    1b
04551	                ! dx=result(low), bx=result(high)
04552	        jmp     9b
04553	2:
04554	        sub     dx,yl(si)
04555	        sbb     bx,di
04556	        inc     ax
04557	        loop    1b
04558	1:
04559	                ! dx=result(low), bx=result(high)
04560	        jmp     9b





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/sar2.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

04600	.define .sar2
04601	.text
04602	
04603	.sar2:
04604	                                ! bx, descriptor address
04605	                                ! ax, index
04606	        pop     cx
04607	        pop     dx              ! base address
04608	        push    cx
04609	        xchg    di,dx           ! di = base address, dx is saved di
.Op 35 src/lib/i86/em/sar2.s
04610	        sub     ax,(bx)
04611	        mov     cx,4(bx)
04612	        push    dx
04613	        imul    cx
04614	        pop     dx
04615	        add     di,ax
04616	        sar     cx,#1
04617	        jnb     1f
04618	        pop     bx
04619	        pop     ax
04620	        stosb
04621	        mov     di,dx
04622	        jmp     (bx)
04623	1:
04624	        pop     bx
04625	        mov     ax,si
04626	        mov     si,sp
04627	        rep
04628	        mov
04629	        mov     sp,si
04630	        mov     si,ax
04631	        mov     di,dx
04632	        jmp     (bx)



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/sbi.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

04700	.define .sbi
04701	.text
04702	
04703	        ! #bytes in cx , top of stack in ax
04704	.sbi:
04705	        pop     bx              ! return subress
04706	        cmp     cx,#2
04707	        jne     1f
04708	        pop     cx
04709	        sub     ax,cx
04710	        neg     ax
04711	        jmp     (bx)
04712	1:
04713	        cmp     cx,#4
04714	        jne     9f
04715	        pop     dx
04716	        pop     cx
04717	        sub     cx,ax
04718	        mov     ax,cx
04719	        pop     cx
04720	        sbb     cx,dx
04721	        push    cx
04722	        jmp     (bx)
04723	9:
04724	.extern .trpilin
04725	        push    bx
04726	        jmp     .trpilin
.Ep 36 src/lib/i86/em/set.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/set.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

04800	.define .set
04801	.text
04802	
04803	        ! #bytes in cx
04804	        ! bit # in ax
04805	.set:
04806	        pop     bx              ! return address
04807	        xor     dx,dx
04808	!ifdef create set
04809	        sub     sp,cx
04810	        push    bx
04811	        push    di
04812	        mov     bx,sp
04813	        xor     di,di
04814	        sar     cx,#1
04815	1:
04816	        mov     4(bx)(di),dx
04817	        inc     di
04818	        inc     di
04819	        loop    1b
04820	!endif
04821	        mov     bx,#8
04822	        div     bx
04823	        cmp     ax,di
04824	        jae     2f
04825	        mov     di,dx
04826	        movb    dl,bits(di)
04827	        mov     di,sp
04828	        add     di,ax
04829	        orb     4(di),dl
04830	        pop     di
04831	        ret
04832	2:
04833	ESET = 2
04834	.extern .error
04835	        pop     di
04836	        mov     ax,#ESET
04837	        call    .error
04838	        ret
04839	
04840	        .data
04841	bits:
04842	        .data1   1,2,4,8,16,32,64,128
.Op 37 src/lib/i86/em/stb.c

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/stb.c	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

04900	/*
04901	 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
04902	 * See the copyright notice in the ACK home directory, in the file "Copyright".
04903	 */
04904	/* $Header: stb.c,v 1.1 89/02/07 11:01:46 ceriel Exp $ */
04905	/* library routine for copying structs */
04906	
04907	#include <ansi.h>
04908	
04909	_PROTOTYPE(int __stb, (int n, char *f, char *t ));
04910	__stb(n, f, t)
04911	register int n;
04912	register char *f, *t;
04913	{
04914	        if (n > 0)
04915	                do
04916	                        *t++ = *f++;
04917	                while (--n);
04918	}


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/sti.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

05000	.define .sti
05001	.define .sts
05002	.text
05003	
05004	        ! #bytes in cx
05005	        ! address in bx
05006	        ! save di/si. they might be register variables
05007	.sts:
05008	        mov     dx,di           ! save di
05009	        mov     di,bx
05010	        pop     bx              ! return address
05011	        sar     cx,#1
05012	        jnb     1f
05013	        pop     ax
05014	        stosb
05015	        mov     di,dx
05016	        jmp     (bx)
05017	.sti:
05018	        ! only called with count > 4
05019	        mov     dx,di
05020	        mov     di,bx
05021	        pop     bx
05022	        sar     cx,#1
05023	1:
05024	        mov     ax,si
05025	        mov     si,sp
05026	        rep
05027	        mov
05028	        mov     sp,si
05029	        mov     di,dx
.Ep 38 src/lib/i86/em/sti.s
05030	        mov     si,ax
05031	        jmp     (bx)




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/stop.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

05100	.define .stop
05101	.text
05102	.stop:
05103	        call    __exit


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/strhp.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

05200	.define .strhp
05201	.text
05202	.extern .reghp, .limhp, .fat
05203	EHEAP = 17
05204	
05205	.strhp:
05206	        pop     bx
05207	        pop     ax
05208	        mov     .reghp,ax
05209	        cmp     ax,.limhp
05210	        jb      1f
05211	        add     ax,#01000
05212	        and     ax,#0177000
05213	        mov     .limhp,ax
05214	        cmp     ax,sp
05215	        jae     2f
05216	1:
05217	        jmp     (bx)
05218	2:
05219	        mov     ax,#EHEAP
05220	        jmp     .fat





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/trp.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

05300	.define .trpdivz
05301	.define .trpilin
05302	.define .trpcase
05303	.define .trprang
05304	.define .trpset
05305	.define .trpnofp
05306	.define .trpheap
05307	.define .trp
05308	
05309	.bss
.Op 39 src/lib/i86/em/trp.s
05310	.M: .zerow 24/2
05311	
05312	.text
05313	.extern .trpdivz
05314	.extern .trpilin
05315	.extern .trpcase
05316	.extern .trprang
05317	.extern .trpset
05318	.extern .trpnofp
05319	.extern .trpheap
05320	.extern .trp
05321	
05322	.trpdivz:
05323	mov ax,#6
05324	mov dx,#.Mdivz
05325	jmp .Trp
05326	.trpilin:
05327	mov ax,#18
05328	mov dx,#.Milin
05329	jmp .Trp
05330	.trpcase:
05331	mov ax,#20
05332	mov dx,#.Mcase
05333	jmp .Trp
05334	.trprang:
05335	mov ax,#1
05336	mov dx,#.Mrang
05337	jmp .Trp
05338	.trpset:
05339	mov ax,#2
05340	mov dx,#.Mset
05341	jmp .Trp
05342	.trpnofp:
05343	mov ax,#18
05344	mov dx,#.Mnofp
05345	jmp .Trp
05346	.trpheap:
05347	mov ax,#17
05348	mov dx,#.Mheap
05349	jmp .Trp
05350	
05351	.Trp:
05352	xor     bx,bx
05353	.extern .trppc
05354	xchg    bx,.trppc
05355	test    bx,bx
05356	jz      2f
05357	push    ax
05358	call    (bx)
05359	pop     ax
05360	ret
05361	2:
05362	mov bx,#22
05363	push bx
05364	push dx
05365	mov ax,#2
05366	push ax
05367	call .Write
05368	call __exit
05369	
.Ep 40 src/lib/i86/em/trp.s
05370	.trp:
05371	mov dx,ax
05372	cmp dx,#21
05373	jae 1f
05374	sal dx,#1
05375	mov bx,#.Mtable
05376	add bx,dx
05377	mov bx,(bx)
05378	test bx,bx
05379	jz 1f
05380	mov dx,bx
05381	jmp 2f
05382	1:
05383	mov bx,#.Mtrp+14
05384	mov cx,#6
05385	mov dx,ax
05386	1:
05387	and dx,#7
05388	add dx,'0'
05389	movb (bx),dl
05390	dec bx
05391	sar dx,#1
05392	sar dx,#1
05393	sar dx,#1
05394	loop 1b
05395	mov dx,#.Mtrp
05396	2:
05397	jmp .Trp
05398	
05399	.Write:
05400	push bp
05401	mov bp,sp
05402	mov .M+2,#4
05403	mov bx,4(bp)
05404	mov .M+4,bx
05405	mov bx,8(bp)
05406	mov .M+6,bx
05407	mov bx,6(bp)
05408	mov .M+10,bx
05409	mov ax,#.M
05410	push ax
05411	mov ax,#1
05412	push ax
05413	
05414	mov ax,#1
05415	mov bx,#.M
05416	mov cx,#3
05417	int 32
05418	mov sp,bp
05419	pop bp
05420	ret
05421	
05422	
05423	.data
05424	.Mtable:
05425	        .data2 0,       .Mrang, .Mset,  0,      0,      0,      .Mdivz, 0
05426	        .data2 0,       0,      0,      0,      0,      0,      0,      0
05427	        .data2 0,       .Mheap, .Milin, .Milin, .Mcase
05428	
05429	.Mdivz: .asciz "Error: Division by 0 \n"
.Op 41 src/lib/i86/em/trp.s
05430	.Milin: .asciz "Illegal EM instruct'n\n"
05431	.Mcase: .asciz "Err in EM case instr \n"
05432	.Mrang: .asciz "Variable out of range\n"
05433	.Mset:  .asciz "Err in EM set instr  \n"
05434	.Mnofp: .asciz "Floating pt not impl.\n"
05435	.Mheap: .asciz "Heap overflow        \n"
05436	
05437	.Mtrp:  .asciz "EM trap 0000000 octal\n"



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/unknown.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

05500	.define .unknown
05501	.text
05502	.extern .fat
05503	EILLINS = 18
05504	
05505	.unknown:
05506	        mov  ax,#EILLINS
05507	        push ax
05508	        jmp  .fat


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/vars.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

05600	.define .reghp, .limhp, .ignmask, .trppc
05601	.extern .reghp, .limhp, .ignmask, .trppc
05602	
05603	.data
05604	.reghp:
05605	        .data2   endbss
05606	.limhp:
05607	        .data2   endbss
05608	.ignmask:
05609	        .data2   1336
05610	.trppc:
05611	        .data2   0




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/em/xor.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

05700	.define .xor
05701	.text
05702	
05703	        ! #bytes in cx
05704	.xor:
05705	        pop     bx              ! return address
05706	        mov     dx,di
05707	        mov     di,sp
05708	        add     di,cx
05709	        sar     cx,#1
.Ep 42 src/lib/i86/em/xor.s
05710	1:
05711	        pop     ax
05712	        xor     ax,(di)
05713	        stos
05714	        loop    1b
05715	        mov     di,dx
05716	        jmp     (bx)




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/misc/alloca.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

05800	#
05801	!       alloca() - allocate space on the stack          Author: Kees J. Bot
05802	!                                                               26 Jan 1994
05803	
05804	#if __ACK__     /* BCC can't do alloca(), register saving is wrong. */
05805	
05806	.text
05807	.define _alloca
05808	_alloca:
05809	        pop     cx              ! Return address
05810	        pop     ax              ! Bytes to allocate
05811	        add     ax, #2*2+1      ! Add space for two saved register variables
05812	        andb    al, #0xFE       ! Align
05813	        mov     bx, sp          ! Keep current sp
05814	        sub     sp, ax          ! Lower stack
05815	        mov     ax, sp          ! Return value
05816	        push    2(bx)           ! Push what is probably the saved si
05817	        push    (bx)            ! Saved di
05818	                                ! Now ACK can still do:
05819	                                !       pop di; pop si; mov sp, bp; pop bp; ret
05820	        push    ax              ! Dummy argument
05821	        jmp     (cx)
05822	#endif



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/misc/getprocessor.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

05900	!       getprocessor() - determine processor type       Author: Kees J. Bot
05901	!                                                               26 Jan 1994
05902	
05903	.text
05904	
05905	        o32 = 0x66              ! 32 bit operand size prefix
05906	
05907	! int getprocessor(void);
05908	!       Return 86, 186, 286, 386, 486, 586, ...
05909	
05910	.define _getprocessor
05911	
05912	_getprocessor:
05913	        push    bp
05914	        mov     bp, sp
.Op 43 src/lib/i86/misc/getprocessor.s
05915	        push    sp              ! see if pushed sp == sp
05916	        pop     ax
05917	        cmp     ax, sp
05918	        jz      new_processor
05919	        mov     cx, #0x0120     ! see if shifts are mod 32
05920	        shlb    ch, cl          ! zero tells if 86
05921	        mov     ax, #86
05922	        jz      got_processor
05923	        mov     ax, #186
05924	        jmp     got_processor
05925	
05926	new_processor:                  ! see if high bits are set in saved GDT
05927	        sub     sp, #6          ! space for GDT ptr
05928	        .data1  0x0F            ! Prefix for 286 instruction: sgdt -6(bp)
05929	        add     -6(bp), ax      ! save 3 word GDT ptr (This is NOT an add!)
05930	        cmpb    -1(bp), #0      ! top byte of GDT ptr is zero only for 386
05931	        mov     ax, #286
05932	        jnz     got_processor
05933	
05934	! 386, 486, 586
05935	        and     sp, #0xFFFC     ! Align stack to avoid AC fault (needed?)
05936	        mov     cx, #0x0004     ! Try to flip the AC bit introduced on the 486
05937	        call    flip
05938	        mov     ax, #386        ! 386 if it didn't react to "flipping"
05939	        jz      got_processor
05940	        mov     cx, #0x0020     ! Try to flip the ID bit introduced on the 586
05941	        call    flip
05942	        mov     ax, #486        ! 486 if it didn't react
05943	        jz      got_processor
05944	        .data1  o32
05945	        pushf
05946	        .data1  o32
05947	        pusha                   ! Save the world
05948	        .data1  o32
05949	        xor     ax, ax
05950	        inc     ax              ! eax = 1
05951	        .data1  0x0F, 0xA2      ! CPUID instruction tells the processor type
05952	        andb    ah, #0x0F       ! Extract the family (5, 6, ...)
05953	        movb    al, ah
05954	        movb    ah, #100
05955	        mulb    ah              ! 500, 600, ...
05956	        add     ax, #86         ! 586, 686, ...
05957	        mov     bx, sp
05958	        mov     7*4(bx), ax     ! Pass ax through
05959	        .data1  o32
05960	        popa
05961	        .data1  o32
05962	        popf
05963	
05964	got_processor:
05965	        mov     sp, bp
05966	        pop     bp
05967	        ret
05968	
05969	flip:
05970	        push    bx              ! Save bx and realign stack to multiple of 4
05971	        .data1  o32             ! About to operate on a 32 bit object
05972	        pushf                   ! Push eflags
05973	        pop     ax
05974	        pop     dx              ! dx:ax = eflags
.Ep 44 src/lib/i86/misc/getprocessor.s
05975	        mov     bx, dx          ! Save original eflags (high word only)
05976	        xor     dx, cx          ! Flip the bit to test
05977	        push    dx
05978	        push    ax              ! Push modified eflags value
05979	        .data1  o32
05980	        popf                    ! Load modified eflags register
05981	        .data1  o32
05982	        pushf
05983	        pop     ax
05984	        pop     dx              ! Get it again
05985	        push    bx
05986	        push    ax
05987	        .data1  o32
05988	        popf                    ! Restore original eflags register
05989	        xor     dx, bx          ! See if the bit changed
05990	        test    dx, cx
05991	        pop     bx              ! Restore bx
05992	        ret



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/misc/iolib.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

06000	! Some I/O related routines like:
06001	!       unsigned char inb(int port);
06002	!       unsigned short inw(int port);
06003	!       void outb(int port, int value);
06004	!       void outw(int port, int value);
06005	!       void rep_inb(int port, unsigned char *buf, size_t count);
06006	!       void rep_inw(int port, unsigned short *buf, size_t count);
06007	!       void rep_outb(int port, unsigned char *buf, size_t count);
06008	!       void rep_outw(int port, unsigned short *buf, size_t count);
06009	!       void intr_enable(void);
06010	!       void intr_disable(void);
06011	
06012	.text
06013	.define _inb
06014	_inb:
06015	        push    bp
06016	        mov     bp, sp
06017	        mov     dx, 4(bp)               ! port
06018	        inb     dx                      ! read 1 byte
06019	        xorb    ah, ah
06020	        pop     bp
06021	        ret
06022	
06023	.define _inw
06024	_inw:
06025	        push    bp
06026	        mov     bp, sp
06027	        mov     dx, 4(bp)               ! port
06028	        in      dx                      ! read 1 word
06029	        pop     bp
06030	        ret
06031	
06032	.define _outb
06033	_outb:
06034	        push    bp
.Op 45 src/lib/i86/misc/iolib.s
06035	        mov     bp, sp
06036	        mov     dx, 4(bp)               ! port
06037	        mov     ax, 4+2(bp)             ! value
06038	        outb    dx                      ! output 1 byte
06039	        pop     bp
06040	        ret
06041	
06042	.define _outw
06043	_outw:
06044	        push    bp
06045	        mov     bp, sp
06046	        mov     dx, 4(bp)               ! port
06047	        mov     ax, 4+2(bp)             ! value
06048	        out     dx                      ! output 1 word
06049	        pop     bp
06050	        ret
06051	
06052	.define _rep_inb
06053	_rep_inb:
06054	        push    bp
06055	        mov     bp, sp
06056	        push    di
06057	        mov     dx, 4(bp)               ! port
06058	        mov     di, 6(bp)               ! buf
06059	        mov     cx, 8(bp)               ! byte count
06060	    rep inb     dx                      ! input many bytes
06061	        pop     di
06062	        pop     bp
06063	        ret
06064	
06065	.define _rep_inw
06066	_rep_inw:
06067	        push    bp
06068	        mov     bp, sp
06069	        push    di
06070	        mov     dx, 4(bp)               ! port
06071	        mov     di, 6(bp)               ! buf
06072	        mov     cx, 8(bp)               ! byte count
06073	        shr     cx, #1                  ! word count
06074	    rep in      dx                      ! input many words
06075	        pop     di
06076	        pop     bp
06077	        ret
06078	
06079	.define _rep_outb
06080	_rep_outb:
06081	        push    bp
06082	        mov     bp, sp
06083	        push    si
06084	        mov     dx, 4(bp)               ! port
06085	        mov     si, 6(bp)               ! buf
06086	        mov     cx, 8(bp)               ! byte count
06087	    rep outb    dx                      ! output many bytes
06088	        pop     si
06089	        pop     bp
06090	        ret
06091	
06092	.define _rep_outw
06093	_rep_outw:
06094	        push    bp
.Ep 46 src/lib/i86/misc/iolib.s
06095	        mov     bp, sp
06096	        push    si
06097	        mov     dx, 4(bp)               ! port
06098	        mov     si, 6(bp)               ! buf
06099	        mov     cx, 8(bp)               ! byte count
06100	        shr     cx, #1                  ! word count
06101	    rep out     dx                      ! output many words
06102	        pop     si
06103	        pop     bp
06104	        ret
06105	
06106	.define _intr_disable
06107	_intr_disable:
06108	        push    bp
06109	        mov     bp, sp
06110	        cli
06111	        pop     bp
06112	        ret
06113	
06114	.define _intr_enable
06115	_intr_enable:
06116	        push    bp
06117	        mov     bp, sp
06118	        sti
06119	        pop     bp
06120	        ret





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/rts/__sigreturn.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

06200	! This routine is the low-level code for returning from signals.  
06201	! It calls __sigreturn, which is the normal "system call" routine.
06202	! Both ___sigreturn and __sigreturn are needed.
06203	.sect .text; .sect .rom; .sect .data; .sect .bss
06204	.sect .text
06205	.define ___sigreturn
06206	.extern __sigreturn
06207	___sigreturn:
06208	        add sp, #8
06209	        jmp __sigreturn
.Op 47 src/lib/i86/rts/_sendrec.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/rts/_sendrec.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

06300	.define __send, __receive, __sendrec
06301	
06302	! See ../h/com.h for C definitions
06303	SEND = 1
06304	RECEIVE = 2
06305	BOTH = 3
06306	SYSVEC = 32
06307	
06308	!*========================================================================*
06309	!                           _send and _receive                            *
06310	!*========================================================================*
06311	! _send(), _receive(), _sendrec() all save bp, but destroy ax, bx, and cx.
06312	.extern __send, __receive, __sendrec
06313	__send: mov cx,*SEND            ! _send(dest, ptr)
06314	        jmp L0
06315	
06316	__receive:
06317	        mov cx,*RECEIVE         ! _receive(src, ptr)
06318	        jmp L0
06319	
06320	__sendrec:
06321	        mov cx,*BOTH            ! _sendrec(srcdest, ptr)
06322	        jmp L0
06323	
06324	  L0:   push bp                 ! save bp
06325	        mov bp,sp               ! can't index off sp
06326	        mov ax,4(bp)            ! ax = dest-src
06327	        mov bx,6(bp)            ! bx = message pointer
06328	        int SYSVEC              ! trap to the kernel
06329	        pop bp                  ! restore bp
06330	        ret                     ! return
06331	




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/rts/brksize.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

06400	.define __brksize
06401	.data
06402	.extern endbss, __brksize
06403	__brksize: .data2 endbss
.Ep 48 src/lib/i86/rts/ncrtso.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/rts/ncrtso.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

06500	! This is the C run-time start-off routine.  It's job is to take the
06501	! arguments as put on the stack by EXEC, and to parse them and set them up the
06502	! way _main expects them.
06503	! It also initializes _environ when this variable isn't defined by the
06504	! programmer.  The detection of whether _environ belong to us is rather
06505	! simplistic.  We simply check for some magic value, but there is no other
06506	! way.
06507	
06508	.extern _main, _exit, crtso, __penvp
06509	.extern begtext, begdata, begbss, endtext, enddata, endbss
06510	.text
06511	begtext:
06512	crtso:          mov     bx,sp
06513	                mov     cx,(bx)
06514	                add     bx,*2
06515	                mov     ax,cx
06516	                inc     ax
06517	                shl     ax,#1
06518	                add     ax,bx
06519	                mov     __penvp,ax      ! save envp in __envp
06520	
06521	                ! Test whether address of environ < address of end.
06522	                ! This is done for separate I&D systems.
06523	                mov     dx,#_environ
06524	                cmp     dx,#__end
06525	                jae     1f
06526	                cmp     _environ,#21331         ! is it our _environ?
06527	                jne     1f
06528	                mov     _environ,ax
06529	1:
06530	                push    ax      ! push environ
06531	                push    bx      ! push argv
06532	                push    cx      ! push argc
06533	                xor     bp,bp   ! clear bp for traceback of core files
06534	                call    _main
06535	                add     sp,*6
06536	                push    ax      ! push exit status
06537	                call    _exit
06538	
06539	.data
06540	begdata:
06541	__penvp:        .data2 0
06542	.bss
06543	begbss:
.Op 49 src/lib/i86/rts/setjmp.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/rts/setjmp.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

06600	.sect .text; .sect .data; .sect .bss
06601	.sect .bss
06602	.comm _gtobuf, 12
06603	.extern ___setjmp
06604	.extern _sigprocmask
06605	.extern __longjerr
06606	.extern __sigjmp
06607	.extern _printf, _exit
06608	.extern .blm, .sti, .cret
06609	.sect .text
06610	!
06611	! Warning:  this code depends on the C language declaration of
06612	! jmp_buf in <setjmp.h>, and on the definitions of the flags
06613	! SC_ONSTACK, SC_SIGCONTEXT, and SC_NOREGLOCALS in <sys/sigcontext.h>.
06614	SC_SIGCONTEXT=2
06615	SC_NOREGLOCALS=4
06616	!
06617	! ___setjmp is called with two arguments.  The third argument is pushed
06618	! by compiler generated code transparently to the caller.
06619	!
06620	! ___setjmp(jmp_buf, savemask, retaddr)
06621	!
06622	___setjmp:
06623	        push    bp
06624	        mov     bp,sp
06625	
06626	        push    0(bp)           ! frame pointer
06627	        lea     ax, 4(bp)
06628	        push    ax              ! stack pointer
06629	        push    8(bp)           ! program counter
06630	        
06631	        xor     ax, ax
06632	        push    ax              ! signal mask high
06633	        push    ax              ! signal mask low
06634	        mov     ax,#SC_NOREGLOCALS      ! flags (4 is SC_NOREGLOCALS)
06635	        push    ax
06636	
06637	        mov     ax, 6(bp)       ! get the savemask arg
06638	        cmp     ax, #0
06639	        je      nosigstate      ! don't save signal state
06640	
06641	        or      -12(bp), #SC_SIGCONTEXT         ! flags |= SC_SIGCONTEXT
06642	
06643	        lea     ax, -10(bp)
06644	        push    ax
06645	        push    #0
06646	        push    #0
06647	        call    _sigprocmask    ! fill in signal mask
06648	        add     sp, #6
06649	
06650	nosigstate:
06651	        mov     bx, 4(bp)       ! jmp_buf
06652	        mov     cx, #12         ! sizeof(jmp_buf)
06653	        call    .sti            ! sp = src addr, bx = dest addr, cx = count
06654	        xor     ax, ax
.Ep 50 src/lib/i86/rts/setjmp.s
06655	        jmp     .cret
06656	
06657	.extern __longjmp
06658	__longjmp:
06659	        push    bp
06660	        mov     bp, sp
06661	
06662	! Copy the jmp_buf to the gtobuf.
06663	        push    4(bp)                   ! src address
06664	        mov     ax, #_gtobuf
06665	        push    ax                      ! dst address
06666	        mov     cx, #6                  ! word count
06667	        call    .blm                    ! block move                    
06668	
06669	! Check that this jmp_buf has no saved registers.
06670	        mov     ax, _gtobuf             ! get flags
06671	        test    ax, #4
06672	        je      __longjerr
06673	
06674	! Set up the value to return in ax.
06675	        mov     ax, 6(bp)               ! value to return
06676	        or      ax, ax
06677	        jne     nonzero
06678	        mov     ax, #1
06679	nonzero:
06680	        mov     bx, #_gtobuf
06681	        mov     bp, 10(bx)
06682	        mov     sp, 8(bx)
06683	        jmp     @6(bx)
06684	
06685	.extern _longjmp
06686	_longjmp:
06687	        push    bp
06688	        mov     bp, sp
06689	
06690	! Check that this is a jmp_buf with no saved regs and with signal context info.
06691	        mov     bx, 4(bp)               ! pointer to jmp_buf
06692	        mov     ax, 0(bx)               ! get the flags
06693	        test    ax, #4                  ! check for no saved registers
06694	        je      __longjerr
06695	        test    ax, #2                  ! check for signal context
06696	        je      __longjerr
06697	        
06698	! Compute the value to return
06699	        mov     ax, 6(bp)               ! proposed value to return
06700	        or      ax, ax
06701	        jne     nonzero1
06702	        mov     ax, #1
06703	nonzero1:
06704	
06705	! Call _sigjmp to restore the old signal context.
06706	        push    ax
06707	        push    4(bp)
06708	        call    __sigjmp
06709	        add     sp, #4
.Op 51 src/lib/i86/string/_memmove.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/_memmove.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

06800	!       _memmove()                                      Author: Kees J. Bot
06801	!                                                               27 Jan 1994
06802	.sect .text; .sect .rom; .sect .data; .sect .bss
06803	
06804	! void *_memmove(void *s1, const void *s2, size_t n)
06805	!       Copy a chunk of memory.  Handle overlap.
06806	!
06807	.sect .text
06808	.define __memmove, __memcpy
06809	__memmove:
06810	        push    bp
06811	        mov     bp, sp
06812	        push    si
06813	        push    di
06814	        mov     di, 4(bp)       ! String s1
06815	        mov     si, 6(bp)       ! String s2
06816	        mov     cx, 8(bp)       ! Length
06817	        mov     ax, di
06818	        sub     ax, si
06819	        cmp     ax, cx
06820	        jb      downwards       ! if (s2 - s1) < n then copy downwards
06821	__memcpy:
06822	        cld                     ! Clear direction bit: upwards
06823	        cmp     cx, #16
06824	        jb      upbyte          ! Don't bother being smart with short arrays
06825	        mov     ax, si
06826	        or      ax, di
06827	        testb   al, #1
06828	        jnz     upbyte          ! Bit 0 set, use byte copy
06829	upword: shr     cx, #1
06830	    rep movs                    ! Copy words
06831	        adc     cx, cx          ! One more byte?
06832	upbyte:
06833	    rep movsb                   ! Copy bytes
06834	done:   mov     ax, 4(bp)       ! Absolutely noone cares about this value
06835	        pop     di
06836	        pop     si
06837	        pop     bp
06838	        ret
06839	
06840	! Handle bad overlap by copying downwards, don't bother to do word copies.
06841	downwards:
06842	        std                     ! Set direction bit: downwards
06843	        add     si, cx
06844	        dec     si
06845	        add     di, cx
06846	        dec     di
06847	    rep movsb                   ! Copy bytes
06848	        cld
06849	        jmp     done
.Ep 52 src/lib/i86/string/_strncat.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/_strncat.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

06900	!       _strncat()                                      Author: Kees J. Bot
06901	!                                                               27 Jan 1994
06902	.sect .text; .sect .rom; .sect .data; .sect .bss
06903	
06904	! char *_strncat(char *s1, const char *s2, size_t dx)
06905	!       Append string s2 to s1.
06906	!
06907	.sect .text
06908	.define __strncat
06909	__strncat:
06910	        push    bp
06911	        mov     bp, sp
06912	        push    si
06913	        push    di
06914	        mov     di, 4(bp)       ! String s1
06915	        mov     cx, #-1
06916	        xorb    al, al          ! Null byte
06917	        cld
06918	  repne scasb                   ! Look for the zero byte in s1
06919	        dec     di              ! Back one up (and clear 'Z' flag)
06920	        push    di              ! Save end of s1
06921	        mov     di, 6(bp)       ! di = string s2
06922	        mov     cx, dx          ! Maximum count
06923	  repne scasb                   ! Look for the end of s2
06924	        jne     no0
06925	        inc     cx              ! Exclude null byte
06926	no0:    sub     dx, cx          ! Number of bytes in s2
06927	        mov     cx, dx
06928	        mov     si, 6(bp)       ! si = string s2
06929	        pop     di              ! di = end of string s1
06930	    rep movsb                   ! Copy bytes
06931	        stosb                   ! Add a terminating null
06932	        mov     ax, 4(bp)       ! Return s1
06933	        pop     di
06934	        pop     si
06935	        pop     bp
06936	        ret




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/_strncmp.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

07000	!       strncmp()                                       Author: Kees J. Bot
07001	!                                                               27 Jan 1994
07002	.sect .text; .sect .rom; .sect .data; .sect .bss
07003	
07004	! int strncmp(const char *s1, const char *s2, size_t cx)
07005	!       Compare two strings.
07006	!
07007	.sect .text
07008	.define __strncmp
07009	__strncmp:
.Op 53 src/lib/i86/string/_strncmp.s
07010	        push    bp
07011	        mov     bp, sp
07012	        push    si
07013	        push    di
07014	        xor     ax, ax          ! Prepare return value
07015	        test    cx, cx          ! Max length is zero?
07016	        je      equal
07017	        mov     si, 4(bp)       ! si = string s1
07018	        mov     di, 6(bp)       ! di = string s2
07019	        cld
07020	compare:
07021	        cmpsb                   ! Compare two bytes
07022	        jne     unequal
07023	        cmpb    -1(si), #0      ! End of string?
07024	        je      equal
07025	        dec     cx              ! Length limit reached?
07026	        jne     compare
07027	        jmp     equal
07028	unequal:
07029	        ja      after
07030	        sub     ax, #2          ! if (s1 < s2) ax -= 2;
07031	after:  inc     ax              ! ax++, now it's -1 or 1
07032	equal:  pop     di
07033	        pop     si
07034	        pop     bp
07035	        ret





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/_strncpy.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

07100	!       _strncpy()                                      Author: Kees J. Bot
07101	!                                                               27 Jan 1994
07102	.sect .text; .sect .rom; .sect .data; .sect .bss
07103	
07104	! char *_strncpy(char *s1, const char *s2, size_t cx)
07105	!       Copy string s2 to s1.
07106	!
07107	.sect .text
07108	.define __strncpy
07109	__strncpy:
07110	        mov     di, 6(bp)       ! di = string s2
07111	        xorb    al, al          ! Look for a zero byte
07112	        mov     dx, cx          ! Save maximum count
07113	        cld
07114	  repne scasb                   ! Look for end of s2
07115	        sub     dx, cx          ! Number of bytes in s2 including null
07116	        xchg    cx, dx
07117	        mov     si, 6(bp)       ! si = string s2
07118	        mov     di, 4(bp)       ! di = string s1
07119	    rep movsb                   ! Copy bytes
07120	        ret
.Ep 54 src/lib/i86/string/_strnlen.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/_strnlen.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

07200	!       _strnlen()                                      Author: Kees J. Bot
07201	!                                                               27 Jan 1994
07202	.sect .text; .sect .rom; .sect .data; .sect .bss
07203	
07204	! size_t _strnlen(const char *s, size_t cx)
07205	!       Return the length of a string.
07206	!
07207	.sect .text
07208	.define __strnlen
07209	__strnlen:
07210	        push    bp
07211	        mov     bp, sp
07212	        push    di
07213	        mov     di, 4(bp)       ! di = string
07214	        xorb    al, al          ! Look for a zero byte
07215	        mov     dx, cx          ! Save maximum count
07216	        cmpb    cl, #1          ! 'Z' bit must be clear if cx = 0
07217	        cld
07218	  repne scasb                   ! Look for zero
07219	        jne     no0
07220	        inc     cx              ! Don't count zero byte
07221	no0:    mov     ax, dx
07222	        sub     ax, cx          ! Compute bytes scanned
07223	        pop     di
07224	        pop     bp
07225	        ret





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/bcmp.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

07300	!       bcmp()                                          Author: Kees J. Bot
07301	!                                                               27 Jan 1994
07302	.sect .text; .sect .rom; .sect .data; .sect .bss
07303	
07304	! int bcmp(const void *s1, const void *s2, size_t n)
07305	!       Compare two chunks of memory.
07306	!       This is a BSD routine that escaped from the kernel.  Don't use.
07307	!       (Alas it is not without some use, it reports the number of bytes
07308	!       after the bytes that are equal.  So it can't be simply replaced.)
07309	!
07310	.sect .text
07311	.define _bcmp
07312	_bcmp:
07313	        push    bp
07314	        mov     bp, sp
07315	        push    8(bp)
07316	        push    6(bp)
07317	        push    4(bp)
07318	        call    _memcmp         ! Let memcmp do the work
07319	        mov     sp, bp
.Op 55 src/lib/i86/string/bcmp.s
07320	        test    ax, ax
07321	        jz      equal
07322	        sub     dx, 4(bp)       ! Memcmp was nice enough to leave "si" in dx
07323	        dec     dx              ! Number of bytes that are equal
07324	        mov     ax, 8(bp)
07325	        sub     ax, dx          ! Number of bytes that are unequal
07326	equal:  pop     bp
07327	        ret



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/bcopy.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

07400	!       bcopy()                                         Author: Kees J. Bot
07401	!                                                               27 Jan 1994
07402	.sect .text; .sect .rom; .sect .data; .sect .bss
07403	
07404	! void bcopy(const void *s1, void *s2, size_t n)
07405	!       Copy a chunk of memory.  Handle overlap.
07406	!       This is a BSD routine that escaped from the kernel.  Don't use.
07407	!
07408	.sect .text
07409	.define _bcopy
07410	.extern __memmove
07411	_bcopy:
07412	        pop     cx
07413	        pop     ax
07414	        pop     dx              ! Pop return address and arguments
07415	        push    ax
07416	        push    dx              ! Arguments reversed
07417	        push    cx
07418	        jmp     __memmove       ! Call the proper routine


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/bzero.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

07500	!       bzero()                                         Author: Kees J. Bot
07501	!                                                               27 Jan 1994
07502	.sect .text; .sect .rom; .sect .data; .sect .bss
07503	
07504	! void bzero(void *s, size_t n)
07505	!       Set a chunk of memory to zero.
07506	!       This is a BSD routine that escaped from the kernel.  Don't use.
07507	!
07508	.sect .text
07509	.define _bzero
07510	_bzero:
07511	        push    bp
07512	        mov     bp, sp
07513	        push    6(bp)           ! Size
07514	        xor     ax, ax
07515	        push    ax              ! Zero
07516	        push    4(bp)           ! String
07517	        call    _memset         ! Call the proper routine
07518	        mov     sp, bp
07519	        pop     bp
.Ep 56 src/lib/i86/string/bzero.s
07520	        ret





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/index.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

07600	!       index()                                         Author: Kees J. Bot
07601	!                                                               27 Jan 1994
07602	.sect .text; .sect .rom; .sect .data; .sect .bss
07603	
07604	! char *index(const char *s, int c)
07605	!       Look for a character in a string.  Has suffered from a hostile
07606	!       takeover by strchr().
07607	!
07608	.sect .text
07609	.define _index
07610	_index:
07611	        jmp     _strchr




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/memchr.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

07700	!       memchr()                                        Author: Kees J. Bot
07701	!                                                               27 Jan 1994
07702	.sect .text; .sect .rom; .sect .data; .sect .bss
07703	
07704	! void *memchr(const void *s, int c, size_t n)
07705	!       Look for a character in a chunk of memory.
07706	!
07707	.sect .text
07708	.define _memchr
07709	_memchr:
07710	        push    bp
07711	        mov     bp, sp
07712	        push    di
07713	        mov     di, 4(bp)       ! di = string
07714	        movb    al, 6(bp)       ! The character to look for
07715	        mov     cx, 8(bp)       ! Length
07716	        cmpb    cl, #1          ! 'Z' bit must be clear if cx = 0
07717	        cld
07718	  repne scasb
07719	        jne     failure
07720	        lea     ax, -1(di)      ! Found
07721	        pop     di
07722	        pop     bp
07723	        ret
07724	failure:xor     ax, ax
07725	        pop     di
07726	        pop     bp
07727	        ret
.Op 57 src/lib/i86/string/memcmp.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/memcmp.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

07800	!       memcmp()                                        Author: Kees J. Bot
07801	!                                                               27 Jan 1994
07802	.sect .text; .sect .rom; .sect .data; .sect .bss
07803	
07804	! int memcmp(const void *s1, const void *s2, size_t n)
07805	!       Compare two chunks of memory.
07806	!
07807	.sect .text
07808	.define _memcmp
07809	_memcmp:
07810	        cld
07811	        push    bp
07812	        mov     bp, sp
07813	        push    si
07814	        push    di
07815	        xor     ax, ax          ! Prepare return value
07816	        mov     si, 4(bp)       ! String s1
07817	        mov     di, 6(bp)       ! String s2
07818	        mov     cx, 8(bp)       ! Length
07819	        cmp     cx, #16
07820	        jb      cbyte           ! Don't bother being smart with short arrays
07821	        mov     dx, si
07822	        or      dx, di
07823	        andb    dl, #1
07824	        jnz     cbyte           ! Bit 0 set, use byte compare
07825	cword:  sar     cx, #1
07826	        adcb    dl, dl          ! Save carry
07827	   repe cmps                    ! Compare words
07828	        mov     cx, #2          ! Recompare the last word
07829	        sub     si, cx
07830	        sub     di, cx
07831	        addb    cl, dl          ! One more byte?
07832	cbyte:  test    cx, cx          ! Set 'Z' flag if cx = 0
07833	last:
07834	   repe cmpsb                   ! Look for the first differing byte
07835	        je      equal
07836	        ja      after
07837	        sub     ax, #2          ! if (s1 < s2) ax -= 2;
07838	after:  inc     ax              ! ax++, now it's -1 or 1
07839	equal:  mov     dx, si          ! For bcmp() to play with
07840	        pop     di
07841	        pop     si
07842	        pop     bp
07843	        ret
.Ep 58 src/lib/i86/string/memcpy.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/memcpy.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

07900	!       memcpy()                                        Author: Kees J. Bot
07901	!                                                               27 Jan 1994
07902	.sect .text; .sect .rom; .sect .data; .sect .bss
07903	
07904	! void *memcpy(void *s1, const void *s2, size_t n)
07905	!       Copy a chunk of memory.
07906	!       This routine need not handle overlap, so it does not handle overlap.
07907	!       One could simply call __memmove, the cost of the overlap check is
07908	!       negligible, but you are dealing with a programmer who believes that
07909	!       if anything can go wrong, it should go wrong.
07910	!
07911	.sect .text
07912	.define _memcpy
07913	_memcpy:
07914	        push    bp
07915	        mov     bp, sp
07916	        push    si
07917	        push    di
07918	        mov     di, 4(bp)       ! String s1
07919	        mov     si, 6(bp)       ! String s2
07920	        mov     cx, 8(bp)       ! Length
07921	        ! No overlap check here
07922	        jmp     __memcpy        ! Call the part of __memmove that copies up



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/memmove.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

08000	!       memmove()                                       Author: Kees J. Bot
08001	!                                                               27 Jan 1994
08002	.sect .text; .sect .rom; .sect .data; .sect .bss
08003	
08004	! void *memmove(void *s1, const void *s2, size_t n)
08005	!       Copy a chunk of memory.  Handle overlap.
08006	!
08007	.sect .text
08008	.define _memmove
08009	_memmove:
08010	        jmp     __memmove       ! Call common code
.Op 59 src/lib/i86/string/memset.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/memset.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

08100	!       memset()                                        Author: Kees J. Bot
08101	!                                                               27 Jan 1994
08102	.sect .text; .sect .rom; .sect .data; .sect .bss
08103	
08104	! void *memset(void *s, int c, size_t n)
08105	!       Set a chunk of memory to the same byte value.
08106	!
08107	.sect .text
08108	.define _memset
08109	_memset:
08110	        push    bp
08111	        mov     bp, sp
08112	        push    di
08113	        mov     di, 4(bp)       ! The string
08114	        movb    al, 6(bp)       ! The fill byte
08115	        mov     cx, 8(bp)       ! Length
08116	        cld
08117	        cmp     cx, #16
08118	        jb      sbyte           ! Don't bother being smart with short arrays
08119	        test    di, #1
08120	        jnz     sbyte           ! Bit 0 set, use byte store
08121	sword:  movb    ah, al          ! One byte to two bytes
08122	        sar     cx, #1
08123	    rep stos                    ! Store words
08124	        adc     cx, cx          ! One more byte?
08125	sbyte:
08126	    rep stosb                   ! Store bytes
08127	done:   mov     ax, 4(bp)       ! Return some value you have no need for
08128	        pop     di
08129	        pop     bp
08130	        ret





++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/rindex.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

08200	!       rindex()                                        Author: Kees J. Bot
08201	!                                                               27 Jan 1994
08202	.sect .text; .sect .rom; .sect .data; .sect .bss
08203	
08204	! char *rindex(const char *s, int c)
08205	!       Look for the last occurrence a character in a string.  Has suffered
08206	!       from a hostile takeover by strrchr().
08207	!
08208	.sect .text
08209	.define _rindex
08210	_rindex:
08211	        jmp     _strrchr
.Ep 60 src/lib/i86/string/strcat.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/strcat.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

08300	!       strcat()                                        Author: Kees J. Bot
08301	!                                                               27 Jan 1994
08302	.sect .text; .sect .rom; .sect .data; .sect .bss
08303	
08304	! char *strcat(char *s1, const char *s2)
08305	!       Append string s2 to s1.
08306	!
08307	.sect .text
08308	.define _strcat
08309	_strcat:
08310	        mov     dx, #-1         ! Unlimited length
08311	        jmp     __strncat       ! Common code




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/strchr.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

08400	!       strchr()                                        Author: Kees J. Bot
08401	!                                                               27 Jan 1994
08402	.sect .text; .sect .rom; .sect .data; .sect .bss
08403	
08404	! char *strchr(const char *s, int c)
08405	!       Look for a character in a string.
08406	!
08407	.sect .text
08408	.define _strchr
08409	_strchr:
08410	        push    bp
08411	        mov     bp, sp
08412	        push    di
08413	        cld
08414	        mov     di, 4(bp)       ! di = string
08415	        mov     dx, #16         ! Look at small chunks of the string
08416	next:   shl     dx, #1          ! Chunks become bigger each time
08417	        mov     cx, dx
08418	        xorb    al, al          ! Look for the zero at the end
08419	  repne scasb
08420	        pushf                   ! Remember the flags
08421	        sub     cx, dx
08422	        neg     cx              ! Some or all of the chunk
08423	        sub     di, cx          ! Step back
08424	        movb    al, 6(bp)       ! The character to look for
08425	  repne scasb
08426	        je      found
08427	        popf                    ! Did we find the end of string earlier?
08428	        jne     next            ! No, try again
08429	        xor     ax, ax          ! Return NULL
08430	        pop     di
08431	        pop     bp
08432	        ret
08433	found:  pop     ax              ! Get rid of those flags
08434	        lea     ax, -1(di)      ! Address of byte found
.Op 61 src/lib/i86/string/strchr.s
08435	        pop     di
08436	        pop     bp
08437	        ret



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/strcmp.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

08500	!       strcmp()                                        Author: Kees J. Bot
08501	!                                                               27 Jan 1994
08502	.sect .text; .sect .rom; .sect .data; .sect .bss
08503	
08504	! int strcmp(const char *s1, const char *s2)
08505	!       Compare two strings.
08506	!
08507	.sect .text
08508	.define _strcmp
08509	_strcmp:
08510	        mov     cx, #-1         ! Unlimited length
08511	        jmp     __strncmp       ! Common code




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/strcpy.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

08600	!       strcpy()                                        Author: Kees J. Bot
08601	!                                                               27 Jan 1994
08602	.sect .text; .sect .rom; .sect .data; .sect .bss
08603	
08604	! char *strcpy(char *s1, const char *s2)
08605	!       Copy string s2 to s1.
08606	!
08607	.sect .text
08608	.define _strcpy
08609	_strcpy:
08610	        push    bp
08611	        mov     bp, sp
08612	        push    si
08613	        push    di
08614	        mov     cx, #-1         ! Unlimited length
08615	        call    __strncpy       ! Common code
08616	        mov     ax, 4(bp)       ! Return s1
08617	        pop     di
08618	        pop     si
08619	        pop     bp
08620	        ret
.Ep 62 src/lib/i86/string/strlen.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/strlen.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

08700	!       strlen()                                        Author: Kees J. Bot
08701	!                                                               27 Jan 1994
08702	.sect .text; .sect .rom; .sect .data; .sect .bss
08703	
08704	! size_t strlen(const char *s)
08705	!       Return the length of a string.
08706	!
08707	.sect .text
08708	.define _strlen
08709	_strlen:
08710	        mov     cx, #-1         ! Unlimited length
08711	        jmp     __strnlen       ! Common code




++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/strncat.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

08800	!       strncat()                                       Author: Kees J. Bot
08801	!                                                               27 Jan 1994
08802	.sect .text; .sect .rom; .sect .data; .sect .bss
08803	
08804	! size_t strncat(char *s1, const char *s2, size_t n)
08805	!       Append string s2 to s1.
08806	!
08807	.sect .text
08808	.define _strncat
08809	_strncat:
08810	        mov     bx, sp
08811	        mov     dx, 6(bx)       ! Maximum length
08812	        jmp     __strncat       ! Common code



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/strncmp.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

08900	!       strncmp()                                       Author: Kees J. Bot
08901	!                                                               27 Jan 1994
08902	.sect .text; .sect .rom; .sect .data; .sect .bss
08903	
08904	! int strncmp(const char *s1, const char *s2, size_t n)
08905	!       Compare two strings.
08906	!
08907	.sect .text
08908	.define _strncmp
08909	_strncmp:
08910	        mov     bx, sp
08911	        mov     cx, 6(bx)       ! Maximum length
08912	        jmp     __strncmp       ! Common code
.Op 63 src/lib/i86/string/strncpy.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/strncpy.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

09000	!       strncpy()                                       Author: Kees J. Bot
09001	!                                                               27 Jan 1994
09002	.sect .text; .sect .rom; .sect .data; .sect .bss
09003	
09004	! char *strncpy(char *s1, const char *s2, size_t n)
09005	!       Copy string s2 to s1.
09006	!
09007	.sect .text
09008	.define _strncpy
09009	_strncpy:
09010	        push    bp
09011	        mov     bp, sp
09012	        push    si
09013	        push    di
09014	        mov     cx, 8(bp)       ! Maximum length
09015	        call    __strncpy       ! Common code
09016	        mov     cx, dx          ! Number of bytes not copied
09017	    rep stosb                   ! strncpy always copies n bytes by null padding
09018	        mov     ax, 4(bp)       ! Return s1
09019	        pop     di
09020	        pop     si
09021	        pop     bp
09022	        ret



++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/strnlen.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

09100	!       strnlen()                                       Author: Kees J. Bot
09101	!                                                               27 Jan 1994
09102	.sect .text; .sect .rom; .sect .data; .sect .bss
09103	
09104	! size_t strnlen(const char *s, size_t n)
09105	!       Return the length of a string.
09106	!
09107	.sect .text
09108	.define _strnlen
09109	_strnlen:
09110	        mov     bx, sp
09111	        mov     cx, 4(bx)       ! Maximum length
09112	        jmp     __strnlen       ! Common code
.Ep 64 src/lib/i86/string/strrchr.s

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			src/lib/i86/string/strrchr.s	 	 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

09200	!       strrchr()                                       Author: Kees J. Bot
09201	!                                                               27 Jan 1994
09202	.sect .text; .sect .rom; .sect .data; .sect .bss
09203	
09204	! char *strrchr(const char *s, int c)
09205	!       Look for the last occurrence a character in a string.
09206	!
09207	.sect .text
09208	.define _strrchr
09209	_strrchr:
09210	        push    bp
09211	        mov     bp, sp
09212	        push    di
09213	        mov     di, 4(bp)       ! di = string
09214	        mov     cx, #-1
09215	        xorb    al, al
09216	        cld
09217	  repne scasb                   ! Look for the end of the string
09218	        not     cx              ! -1 - cx = Length of the string + null
09219	        dec     di              ! Put di back on the zero byte
09220	        movb    al, 6(bp)       ! The character to look for
09221	        std                     ! Downwards search
09222	  repne scasb
09223	        cld                     ! Direction bit back to default
09224	        jne     failure
09225	        lea     ax, 1(di)       ! Found it
09226	        pop     di
09227	        pop     bp
09228	        ret
09229	failure:xor     ax, ax          ! Not there
09230	        pop     di
09231	        pop     bp
09232	        ret


