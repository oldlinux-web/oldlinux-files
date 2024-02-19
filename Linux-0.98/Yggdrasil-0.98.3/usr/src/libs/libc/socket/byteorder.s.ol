/*
 * Modified by H.J. Lu for Linux 1992
 */
/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Roell makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Thomas Roell, roell@informatik.tu-muenchen.de
 *
 * $Header: /proj/X11/mit/server/ddx/at386/bsdemul/RCS/byteorder.s,v 1.2 91/02/10 16:23:22 root Exp $
 */

        .text

	.globl _htons
	.globl _ntohs
	.align  2
_ntohs:
_htons:
	movl	4(%esp),%eax	/* param = %eax */
	xchgb	%al,%ah		/* swap */
	ret

	.globl _htonl
	.globl _ntohl
	.align  2
_ntohl:
_htonl:
 	movl	4(%esp),%eax	/* param = %eax */
	xchgb	%al,%ah		/* swap lower bytes */
	rorl	$16,%eax	/* swap words */
	xchgb	%al,%ah		/* swap higher bytes */
 	ret
