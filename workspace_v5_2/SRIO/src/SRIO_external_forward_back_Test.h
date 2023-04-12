/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2011
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
* ============================================================================
 Examples to show the usage and performance of SRIO on KeyStone DSP
 these tests are external loopback test on dual Nyquist EVM
* =============================================================================
 *  Revision History
 *  ===============
 *  July 12, 2011 Brighton Feng   File Created
 * ============================================================================
 */
#ifndef _SRIO_EXTERNAL_FORWARD_BACK_TEST_H_
#define _SRIO_EXTERNAL_FORWARD_BACK_TEST_H_

/*test SRIO transfer through single port*/
extern void srio_signle_port_external_forward_back_test(SRIO_Loopback_Mode loopback_mode);

/*test SRIO transfer through multiple ports in parallel*/
extern void srio_multiple_ports_external_forward_back_test();

#endif

