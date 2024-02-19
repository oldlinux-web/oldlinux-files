/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	mach_msg_server.c,v $
 * Revision 2.4  91/05/14  17:53:22  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/14  14:17:47  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  12:44:20  mrt]
 * 
 * Revision 2.2  90/08/06  17:23:58  rpd
 * 	Created.
 * 
 */

#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/port.h>
#include <mach/message.h>
#include <mach/mig_errors.h>

/*
 *	Routine:	mach_msg_server
 *	Purpose:
 *		A simple generic server function.
 */

mach_msg_return_t
__mach_msg_server_timeout(demux, max_size, rcv_name, option, timeout)
     boolean_t (*demux)();
     mach_msg_size_t max_size;
     mach_port_t rcv_name;
     mach_msg_option_t option;
     mach_msg_timeout_t timeout;
{
    register mig_reply_header_t *bufRequest, *bufReply, *bufTemp;
    register mach_msg_return_t mr;

    bufRequest = (mig_reply_header_t *) malloc(max_size);
    if (bufRequest == 0)
	return KERN_RESOURCE_SHORTAGE;
    bufReply = (mig_reply_header_t *) malloc(max_size);
    if (bufReply == 0)
	return KERN_RESOURCE_SHORTAGE;

    for (;;) {
      get_request:
	mr = __mach_msg(&bufRequest->Head, MACH_RCV_MSG|option,
			0, max_size, rcv_name,
			timeout, MACH_PORT_NULL);
	while (mr == MACH_MSG_SUCCESS) {
	    /* we have a request message */

	    (void) (*demux)(&bufRequest->Head, &bufReply->Head);

	    if (bufReply->RetCode != KERN_SUCCESS) {
		if (bufReply->RetCode == MIG_NO_REPLY)
		    goto get_request;

		/* don't destroy the reply port right,
		   so we can send an error message */
		bufRequest->Head.msgh_remote_port = MACH_PORT_NULL;
		__mach_msg_destroy(&bufRequest->Head);
	    }

	    if (bufReply->Head.msgh_remote_port == MACH_PORT_NULL) {
		/* no reply port, so destroy the reply */
		if (bufReply->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX)
		    __mach_msg_destroy(&bufReply->Head);

		goto get_request;
	    }

	    /* send reply and get next request */

	    bufTemp = bufRequest;
	    bufRequest = bufReply;
	    bufReply = bufTemp;

	    mr = __mach_msg(&bufRequest->Head,
			    MACH_SEND_MSG|MACH_RCV_MSG|option,
			    bufRequest->Head.msgh_size, max_size, rcv_name,
			    timeout, MACH_PORT_NULL);
	}

	/* a message error occurred */

	if (mr != MACH_SEND_INVALID_DEST)
	    break;

	/* the reply can't be delivered, so destroy it */
	__mach_msg_destroy(&bufRequest->Head);
    }

    free((char *) bufRequest);
    free((char *) bufReply);
    return mr;
}


mach_msg_return_t
__mach_msg_server (demux, max_size, rcv_name)
     boolean_t (*demux) ();
     mach_msg_size_t max_size;
     mach_port_t rcv_name;
{
  return __mach_msg_server_timeout (demux, max_size, rcv_name,
				    option, MACH_MSG_TIMEOUT_NONE);
}
