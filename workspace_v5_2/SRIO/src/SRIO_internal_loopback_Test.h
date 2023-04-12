/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2011
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
* ============================================================================
 Examples to show the usage and performance of SRIO on KeyStone DSP
 these tests are loopback
* =============================================================================
 *  Revision History
 *  ===============
 *  June 13, 2011 Brighton Feng   File Created
 * ============================================================================
 */
#ifndef _SRIO_INTERNAL_LOOPBACK_TEST_H_
#define _SRIO_INTERNAL_LOOPBACK_TEST_H_

/*test SRIO transfer through single port*/
extern void srio_signle_port_internal_loopback_test(SRIO_Loopback_Mode loopback_mode);

/*test SRIO transfer through multiple ports in parallel*/
extern void srio_multiple_ports_internal_loopback_test();

#endif

