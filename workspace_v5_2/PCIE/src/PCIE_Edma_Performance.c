/*  ============================================================================
 *     Copyright (C) 2011, 2012, 2013 Texas Instruments Incorporated.       *
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
 *  June 23, 2013 Brighton Feng   File Created
 * ============================================================================
 */

#include <stdio.h>
#include <csl_cacheAux.h>
#include <csl_edma3.h>
#include <cslr_device.h>
#include "KeyStone_common.h"
#include "common_test.h"
#include "PCIE_test.h"

#define 	A_COUNT 	(16*1024)
#define 	B_COUNT 	(4)

#define 	Local_LL2_TEST_SRC (0x11810000)
#define 	Local_LL2_TEST_DST (0x11830000)

#define 	Local_SL2_TEST_SRC 	(0xC080000)
#define 	Local_SL2_TEST_DST 	(0xC0C0000)

#define 	Local_DDR_TEST_SRC 	(0x88000000)
#define 	Local_DDR_TEST_DST 	(0x88100000)

void edma_performance_test(Uint32 uiEDMA, Uint32 uiTC, PCIERemoteTestAddress* remoteAddr)
{   
	CSL_TpccRegs*  EDMACCRegs= gpEDMA_CC_regs[uiEDMA];
#if 0
	printf("Overhead test with EDMA%d TC%d\n", uiEDMA, uiTC);

	edma_Throughput_Test(Local_LL2_TEST_SRC, remoteAddr->LL2_DST_ADDR, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_LL2_TEST_SRC, remoteAddr->SL2_DST_ADDR, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_LL2_TEST_SRC, remoteAddr->DDR_DST_ADDR, 8, 1, 8, EDMACCRegs, uiTC);

	edma_Throughput_Test(remoteAddr->LL2_SRC_ADDR, Local_LL2_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(remoteAddr->SL2_SRC_ADDR, Local_LL2_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(remoteAddr->DDR_SRC_ADDR, Local_LL2_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);

	edma_Throughput_Test(Local_SL2_TEST_SRC, remoteAddr->LL2_DST_ADDR, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_SL2_TEST_SRC, remoteAddr->SL2_DST_ADDR, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_SL2_TEST_SRC, remoteAddr->DDR_DST_ADDR, 8, 1, 8, EDMACCRegs, uiTC);

	edma_Throughput_Test(remoteAddr->LL2_SRC_ADDR, Local_SL2_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(remoteAddr->SL2_SRC_ADDR, Local_SL2_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(remoteAddr->DDR_SRC_ADDR, Local_SL2_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);

	edma_Throughput_Test(Local_DDR_TEST_SRC, remoteAddr->LL2_DST_ADDR, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_DDR_TEST_SRC, remoteAddr->SL2_DST_ADDR, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_DDR_TEST_SRC, remoteAddr->DDR_DST_ADDR, 8, 1, 8, EDMACCRegs, uiTC);

	edma_Throughput_Test(remoteAddr->LL2_SRC_ADDR, Local_DDR_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(remoteAddr->SL2_SRC_ADDR, Local_DDR_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(remoteAddr->DDR_SRC_ADDR, Local_DDR_TEST_DST, 8, 1, 8, EDMACCRegs, uiTC);

	edma_Throughput_Test(remoteAddr->LL2_SRC_ADDR, remoteAddr->LL2_DST_ADDR, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(remoteAddr->SL2_SRC_ADDR, remoteAddr->SL2_DST_ADDR, 8, 1, 8, EDMACCRegs, uiTC);
	edma_Throughput_Test(remoteAddr->DDR_SRC_ADDR, remoteAddr->DDR_DST_ADDR, 8, 1, 8, EDMACCRegs, uiTC);
   
#endif	
	printf("Throughput test with EDMA%d TC%d\n", uiEDMA, uiTC);

	edma_Throughput_Test(Local_LL2_TEST_SRC, remoteAddr->LL2_DST_ADDR, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_LL2_TEST_SRC, remoteAddr->SL2_DST_ADDR, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_LL2_TEST_SRC, remoteAddr->DDR_DST_ADDR, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);

	edma_Throughput_Test(remoteAddr->LL2_SRC_ADDR, Local_LL2_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(remoteAddr->SL2_SRC_ADDR, Local_LL2_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(remoteAddr->DDR_SRC_ADDR, Local_LL2_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);

	edma_Throughput_Test(Local_SL2_TEST_SRC, remoteAddr->LL2_DST_ADDR, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_SL2_TEST_SRC, remoteAddr->SL2_DST_ADDR, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_SL2_TEST_SRC, remoteAddr->DDR_DST_ADDR, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);

	edma_Throughput_Test(remoteAddr->LL2_SRC_ADDR, Local_SL2_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(remoteAddr->SL2_SRC_ADDR, Local_SL2_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(remoteAddr->DDR_SRC_ADDR, Local_SL2_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);

	edma_Throughput_Test(Local_DDR_TEST_SRC, remoteAddr->LL2_DST_ADDR, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_DDR_TEST_SRC, remoteAddr->SL2_DST_ADDR, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(Local_DDR_TEST_SRC, remoteAddr->DDR_DST_ADDR, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);

	edma_Throughput_Test(remoteAddr->LL2_SRC_ADDR, Local_DDR_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(remoteAddr->SL2_SRC_ADDR, Local_DDR_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(remoteAddr->DDR_SRC_ADDR, Local_DDR_TEST_DST, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);

	edma_Throughput_Test(remoteAddr->LL2_SRC_ADDR, remoteAddr->LL2_DST_ADDR, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(remoteAddr->SL2_SRC_ADDR, remoteAddr->SL2_DST_ADDR, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);
	edma_Throughput_Test(remoteAddr->DDR_SRC_ADDR, remoteAddr->DDR_DST_ADDR, A_COUNT, B_COUNT, A_COUNT, EDMACCRegs, uiTC);

    return;
}

void PCIE_edma_test(PCIERemoteTestAddress* remoteAddr)
{  
	int i;

	//make PCIE space non-cacheable
	CACHE_wbInvAllL2(CACHE_WAIT);
	for(i=64; i<80; i++)
		gpCGEM_regs->MAR[i]=0;
	
	//Configure L1D as 16KB cache, 16K RAM for test
	CACHE_setL1PSize(CACHE_L1_16KCACHE);
	CACHE_setL1DSize(CACHE_L1_16KCACHE);
	CACHE_setL2Size(CACHE_0KCACHE);

	EDMA_init();

	for(i=0; i<CSL_EDMA3_TPCC0_NUM_TC; i++)
		edma_performance_test(0, i, remoteAddr);
	for(i=0; i<CSL_EDMA3_TPCC1_NUM_TC; i++)
		edma_performance_test(1, i, remoteAddr);
	for(i=0; i<CSL_EDMA3_TPCC2_NUM_TC; i++)
		edma_performance_test(2, i, remoteAddr);
	
	puts("EDMA test complete");
    return;
}

