/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2011
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
 *   @n This is an example to benchmark the EDMA3 performance
 */
/* =============================================================================
 *  Revision History
 *  ===============
 *  Nov 5, 2011 Brighton Feng   File Created
 * ============================================================================
 */

#include <stdio.h>
#include <csl_cacheAux.h>
#include <csl_edma3.h>
#include <cslr_device.h>
#include "KeyStone_common.h"
#include "common_test.h"

#define 	A_COUNT 	(16*1024)
#define 	B_COUNT 	(4)

#define 	Local_LL2_TEST_SRC (0x11820000)
#define 	Local_LL2_TEST_DST (0x11840000)
#define 	HyperLink_LL2_TEST_SRC (0x42820000)
#define 	HyperLink_LL2_TEST_DST (0x42840000)

#define 	Local_SL2_TEST_SRC 	(0xC080000)
#define 	Local_SL2_TEST_DST 	(0xC0C0000)
#define 	HyperLink_SL2_TEST_SRC 	(0x4C100000)
#define 	HyperLink_SL2_TEST_DST 	(0x4C140000)

#define 	Local_DDR_TEST_SRC 	(0x88000000)
#define 	Local_DDR_TEST_DST 	(0x88100000)
#define 	HyperLink_DDR_TEST_SRC 	(0x48200000)
#define 	HyperLink_DDR_TEST_DST 	(0x48300000)


void edma_performance_test(Uint32 uiEDMA, Uint32 uiTC)
{   
	CSL_TpccRegs*  EDMACCRegs= gpEDMA_CC_regs[uiEDMA];

	printf("Overhead test with EDMA%d TC%d\n", uiEDMA, uiTC);

	edma_Throughput_Test(Local_LL2_TEST_SRC, HyperLink_LL2_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_LL2_TEST_SRC, HyperLink_SL2_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_LL2_TEST_SRC, HyperLink_DDR_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);

	edma_Throughput_Test(HyperLink_LL2_TEST_SRC, Local_LL2_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(HyperLink_SL2_TEST_SRC, Local_LL2_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(HyperLink_DDR_TEST_SRC, Local_LL2_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);

	edma_Throughput_Test(Local_SL2_TEST_SRC, HyperLink_LL2_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_SL2_TEST_SRC, HyperLink_SL2_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_SL2_TEST_SRC, HyperLink_DDR_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);

	edma_Throughput_Test(HyperLink_LL2_TEST_SRC, Local_SL2_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(HyperLink_SL2_TEST_SRC, Local_SL2_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(HyperLink_DDR_TEST_SRC, Local_SL2_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);

	edma_Throughput_Test(Local_DDR_TEST_SRC, HyperLink_LL2_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_DDR_TEST_SRC, HyperLink_SL2_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_DDR_TEST_SRC, HyperLink_DDR_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);

	edma_Throughput_Test(HyperLink_LL2_TEST_SRC, Local_DDR_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(HyperLink_SL2_TEST_SRC, Local_DDR_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(HyperLink_DDR_TEST_SRC, Local_DDR_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);

	edma_Throughput_Test(HyperLink_LL2_TEST_SRC, HyperLink_LL2_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(HyperLink_SL2_TEST_SRC, HyperLink_SL2_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(HyperLink_DDR_TEST_SRC, HyperLink_DDR_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);
   
	printf("Throughput test with EDMA%d TC%d\n", uiEDMA, uiTC);

	edma_Throughput_Test(Local_LL2_TEST_SRC, HyperLink_LL2_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_LL2_TEST_SRC, HyperLink_SL2_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_LL2_TEST_SRC, HyperLink_DDR_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);

	edma_Throughput_Test(HyperLink_LL2_TEST_SRC, Local_LL2_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(HyperLink_SL2_TEST_SRC, Local_LL2_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(HyperLink_DDR_TEST_SRC, Local_LL2_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);

	edma_Throughput_Test(Local_SL2_TEST_SRC, HyperLink_LL2_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_SL2_TEST_SRC, HyperLink_SL2_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_SL2_TEST_SRC, HyperLink_DDR_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);

	edma_Throughput_Test(HyperLink_LL2_TEST_SRC, Local_SL2_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(HyperLink_SL2_TEST_SRC, Local_SL2_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(HyperLink_DDR_TEST_SRC, Local_SL2_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);

	edma_Throughput_Test(Local_DDR_TEST_SRC, HyperLink_LL2_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_DDR_TEST_SRC, HyperLink_SL2_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_DDR_TEST_SRC, HyperLink_DDR_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);

	edma_Throughput_Test(HyperLink_LL2_TEST_SRC, Local_DDR_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(HyperLink_SL2_TEST_SRC, Local_DDR_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(HyperLink_DDR_TEST_SRC, Local_DDR_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);

	edma_Throughput_Test(HyperLink_LL2_TEST_SRC, HyperLink_LL2_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(HyperLink_SL2_TEST_SRC, HyperLink_SL2_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(HyperLink_DDR_TEST_SRC, HyperLink_DDR_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
   
    return;
}

void HyperLink_edma_test(void)
{  
	int i;

	//make HyperLink space non-cacheable
	for(i=64; i<80; i++)
		gpCGEM_regs->MAR[i]=0;	
	
	//Configure L1D as 16KB cache, 16K RAM for test
	CACHE_setL1PSize(CACHE_L1_16KCACHE);
	CACHE_setL1DSize(CACHE_L1_16KCACHE);
	CACHE_setL2Size(CACHE_0KCACHE);

	EDMA_init();

	for(i=0; i<CSL_EDMA3_TPCC0_NUM_TC; i++)
		edma_performance_test(0, i);
	for(i=0; i<CSL_EDMA3_TPCC1_NUM_TC; i++)
		edma_performance_test(1, i);
	for(i=0; i<CSL_EDMA3_TPCC2_NUM_TC; i++)
		edma_performance_test(2, i);
	
	puts("EDMA test complete");
    return;
}

