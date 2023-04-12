/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2013
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
* ============================================================================
 Examples to show the usage and performance of GE on KeyStone DSP
 these tests are loopback test
* =============================================================================
 *  Revision History
 *  ===============
 *  June 11, 2013 Brighton Feng   File Created
 * ============================================================================
 */
#ifndef _GE_LOOPBACK_TEST_H_
#define _GE_LOOPBACK_TEST_H_

#include <tistdtypes.h>

typedef struct 
{
	/*number of packets to be sent. Because of limited descriptors for this test,
	this number should not greater than 16 for signle port test.
	For multiport test, total packet number should not greater than 16*/
	Uint8 numPackets;

	/*this is source queue number*/
	Uint32 sourceQueue; 	

	/*different flow to different memory in this test,
	destFlow also determines port number, and destination MAC address*/
	Uint32 destFlow;

	Uint32 payloadNumBytes;

}GE_Transfer_Param;

extern Uint32 message_src_dest[][2];

/*test GE transfer through single port*/
extern void GE_signle_port_loopback_test();

/*transfer through multiple GE ports in parallel*/
extern void GE_multiple_port_transfer();

#endif

