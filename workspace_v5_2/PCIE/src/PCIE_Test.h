/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2013
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
Test configuration for PCIE
 * =============================================================================
 *  Revision History
 *  ===============
 *  June 23, 2013 Brighton Feng  file created
 * =============================================================================
 */
#ifndef _PCIE_TEST_H_
#define _PCIE_TEST_H_

typedef struct  {
	Uint32 DDR_SRC_ADDR;
	Uint32 DDR_DST_ADDR;
	Uint32 SL2_SRC_ADDR;
	Uint32 SL2_DST_ADDR;
	Uint32 LL2_SRC_ADDR;
	Uint32 LL2_DST_ADDR;
} PCIERemoteTestAddress;

/*first outbound region is for configuration for this test,
data window start from the second outbound region*/
#define PCIE_OUTBOUND_DATA_WINDOW 	(CSL_PCIE_REGS+0x800000)

#define PCIE_EXAMPLE_BUF_EMPTY 0
#define PCIE_EXAMPLE_BUF_FULL  0xABCD

/* Size of application buffers */
#define PCIE_BUFSIZE_APP 40
#define PCIE_EXAMPLE_UINT32_SIZE           4 /* preprocessor #if requires a real constant, not a sizeof() */
#define PCIE_EXAMPLE_DSTBUF_BYTES ((PCIE_BUFSIZE_APP + 1) * PCIE_EXAMPLE_UINT32_SIZE)

extern void PCIE_Test();
#endif
