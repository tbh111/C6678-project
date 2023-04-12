/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2013
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
 Interrupts configuration and handling for GE
 * =============================================================================
 *  Revision History
 *  ===============
 *  June 11, 2013 Brighton  file created
 * =============================================================================
 */
#ifndef _GE_INT_H_
#define _GE_INT_H_
#include <tistdtypes.h>

extern volatile Uint32 GE_INT_TSCL;
extern volatile Uint32 GE_RX_packet_cnt;
extern void GE_Interrupts_Init(void);

#endif
