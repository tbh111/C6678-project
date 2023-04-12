/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2013
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
* ============================================================================
 Examples to show the usage of GE on KeyStone DSP
 these tests showe GE transfer between 2 DSPs or between DSP and PC
* =============================================================================
 *  Revision History
 *  ===============
 *  June 11, 2013 Brighton Feng   File Created
 * ============================================================================
 */
#ifndef _GE_2DSP_TEST_H_
#define _GE_2DSP_TEST_H_

#include <tistdtypes.h>

typedef struct 
{
	Uint32 payloadNumBytes;

	Uint8 dataPattern; //data pattern filled into payload

	/*number of packets to be sent, it will be limited by descriptors 
	availible for this test*/
	Uint32 numPackets;
}GE_2DSP_Transfer_Param;

extern void GE_2DSP_Test();

#endif

