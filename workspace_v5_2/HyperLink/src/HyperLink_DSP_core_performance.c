/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2010
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
* ============================================================================
Example to show the performance of DSP core access memory
* =============================================================================
 *  Revision History
 *  ===============
 *  Nov 5, 2011 Brighton Feng   File Created
 * ============================================================================
 */

#include <csl_cacheAux.h>
#include <stdio.h>
#include <c6x.h>
#include "KeyStone_common.h"
#include "common_test.h"
#include "CPU_access_test.h"

#define 	COPY_TEST_SIZE 			(64*1024)
#define 	LL2_TEST_BASE_ADDR 		(0x820000)
#define 	HyperLink_LL2_TEST_BASE_ADDR	(0x41800000)
#define 	HyperLink_SL2_TEST_BASE_ADDR	(0x4C100000)
#define 	HyperLink_DDR_TEST_BASE_ADDR	(0x48000000)

#define 	LOAD_STORE_TIMES 		(512)
#define 	LD_ST_TEST_OVERHEAD 	40
#define 	LL2_LOAD_STORE_SIZE 	(256*1024)
#define 	SL2_LOAD_STORE_SIZE 	(1024*1024)
#define 	DDR_LOAD_STORE_SIZE 	(16*1024*1024)

void MemCopyTest(Uint32 uiAddress, Uint32 uiByteCnt)
{
	//make external memory noncacheable, nonprefetchable
	CACHE_wbInvAllL2(CACHE_WAIT);
	gpCGEM_regs->MAR[(uiAddress/16/1024/1024)]=0;	

	puts("     noncacheable, nonprefetchable memory copy");
	MemCopy8Test((unsigned long long *)LL2_TEST_BASE_ADDR,(unsigned long long *)uiAddress, uiByteCnt);
	MemCopy8Test((unsigned long long *)uiAddress,(unsigned long long *)LL2_TEST_BASE_ADDR, uiByteCnt);
	MemCopy8Test((unsigned long long *)uiAddress,(unsigned long long *)(uiAddress+uiByteCnt), uiByteCnt);

	//make external memory prefetchable cacheable, use L1D cache only
	gpCGEM_regs->MAR[(uiAddress/16/1024/1024)]=1|
		(1<<CSL_CGEM_MAR0_PFX_SHIFT);	
	CACHE_setL2Size(CACHE_0KCACHE);

	puts("     32KB L1D cache, prefetchable memory copy");
	MemCopy8Test((unsigned long long *)LL2_TEST_BASE_ADDR,(unsigned long long *)uiAddress, uiByteCnt);
	MemCopy8Test((unsigned long long *)uiAddress,(unsigned long long *)LL2_TEST_BASE_ADDR, uiByteCnt);
	MemCopy8Test((unsigned long long *)uiAddress,(unsigned long long *)(uiAddress+uiByteCnt), uiByteCnt);

	//use both L1D and L2 cache
	CACHE_setL2Size(CACHE_256KCACHE);

	puts("     32KB L1D cache, 256KB L2 cache, prefetchable memory copy");
	CACHE_wbInvAllL2(CACHE_WAIT);
	MemCopy8Test((unsigned long long *)LL2_TEST_BASE_ADDR,(unsigned long long *)uiAddress, uiByteCnt);
	CACHE_wbInvAllL2(CACHE_WAIT);
	MemCopy8Test((unsigned long long *)uiAddress,(unsigned long long *)LL2_TEST_BASE_ADDR, uiByteCnt);
	CACHE_wbInvAllL2(CACHE_WAIT);
	MemCopy8Test((unsigned long long *)uiAddress,(unsigned long long *)(uiAddress+uiByteCnt), uiByteCnt);

}

void LoadStoreCycleTest(Uint32 uiAddress, Uint32 uiByteCnt)
{
	//set 0KB L2 cache
	CACHE_setL2Size(CACHE_0KCACHE);

	//make external memory nonprefetchable, noncacheable
	CACHE_wbInvAllL2(CACHE_WAIT);
	gpCGEM_regs->MAR[uiAddress/16/1024/1024]=0;	

	puts("\nLDDW test: nonprefetchable, noncacheable");
	LDDWTest(uiAddress, uiByteCnt);
	puts("\nSTDW test: nonprefetchable, noncacheable");
	STDWTest(uiAddress, uiByteCnt);
	
	//make external memory prefetchable, cacheable
	gpCGEM_regs->MAR[uiAddress/16/1024/1024]= 1|
		(1<<CSL_CGEM_MAR0_PFX_SHIFT);	 	
	CACHE_setL2Size(CACHE_0KCACHE);

	puts("\nLDDW test: prefetchable, 32KB L1D cahce");
	LDDWTest(uiAddress, uiByteCnt);
	puts("\nSTDW test: prefetchable, 32KB L1D cahce");
	STDWTest(uiAddress, uiByteCnt);
	
	//set 256KB L2 cache
	CACHE_setL2Size(CACHE_256KCACHE);

	puts("\nLDDW test: prefetchable, 32KB L1D, 256KB L2 cahce");
	LDDWTest(uiAddress, uiByteCnt);
	puts("\nSTDW test: prefetchable, 32KB L1D, 256KB L2 cahce");
	STDWTest(uiAddress, uiByteCnt);
	
}

void HyperLink_DSP_core_test()
{
	//Configure L1 as 32KB cache
	CACHE_setL1PSize(CACHE_L1_32KCACHE);
	CACHE_setL1DSize(CACHE_L1_32KCACHE);

	MemCopyTest(HyperLink_LL2_TEST_BASE_ADDR, COPY_TEST_SIZE);
	MemCopyTest(HyperLink_SL2_TEST_BASE_ADDR, COPY_TEST_SIZE);
	MemCopyTest(HyperLink_DDR_TEST_BASE_ADDR, COPY_TEST_SIZE);
#if 1
	LoadStoreCycleTest(HyperLink_LL2_TEST_BASE_ADDR, LL2_LOAD_STORE_SIZE);
	LoadStoreCycleTest(HyperLink_SL2_TEST_BASE_ADDR, SL2_LOAD_STORE_SIZE);
	LoadStoreCycleTest(HyperLink_DDR_TEST_BASE_ADDR, DDR_LOAD_STORE_SIZE);
#endif
}
