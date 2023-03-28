/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2011
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
* ============================================================================
Memory Test Example on KeyStone DSP
* =============================================================================
 *  Revision History
 *  ===============
 *  11-Jan-2011 Brighton Feng   File Created
 * ============================================================================
 */
#include <stdio.h>
#include <string.h>
#include <csl_cacheAux.h>
#include <c6x.h>
#include "mem_test_get_unused_L2_address.h"
#include "mem_test_DMA.h"
#include "mem_test_DSP_core.h"
#include "KeyStone_DDR_Init.h"
#include "KeyStone_common.h"
#include "common_test.h"

#define LL1_MEM_TEST 		1
#define OTHER_L1_TEST 		1
#define LL2_MEM_TEST 		1
#define OTHER_L2_TEST 		1
#define SL2_MEM_TEST 		1
#define EXTERNAL_MEM_TEST 	1
#define TEST_BY_DSP_CORE 	1
#define TEST_BY_DMA			1

/*test infinitely. For burning test*/
#define TEST_INFINITELY 	0

/*Enable ECC for DDR test*/
#define DDR_ECC_ENABLE 		0

#define LL2_TEST_START_ARRD 	(0x840000)
#define SL2_TEST_START_ARRD 	(0x0C000000)
#define REMAPPED_SL2_TEST_START_ARRD 	(0x18000000)
#define DDR_TEST_START_ARRD 	(0x80000000)

/*memory protection and address extension configuration table 
Please note, this configuration table can not be put in the memory segment 
that will be modified based on this table. Normally, to put this table 
in LL2 is a good idea.
*/
#pragma DATA_SECTION(XMC_MPAX_cfg_table,".far:Core_MPAX")
MPAX_Config XMC_MPAX_cfg_table[]=
{
    /*BADDR         RADDR           SegementSize      AccessPermisionMask
    32-bit virtual  36-bit physical in byte, must     Access types allowed
    base address    address right   be power of 2     in this address range
                    shift by 4                                            */
    {0x0C000000,    0x00C000000>>4,  4*1024*1024,    MP_SR|MP_SW|MP_SX|MP_UR|MP_UW|MP_UX},/*SL2, RWX*/
    {0x10000000,    0x010000000>>4,  0x08000000,     MP_SR|MP_SW|MP_UR|MP_UW},/*L1, LL2 global address space, RW*/
    {0x18000000,    0x00C000000>>4,  4*1024*1024,    MP_SR|MP_SW|MP_UR|MP_UW},/*remap Shared L2 to 0x18000000 for test*/
    {0x21000000,    0x100000000>>4,  4*1024,         MP_SR|MP_SW|MP_UR|MP_UW},/*map DDR2 configuration registers at physical address 0x1:00000000 to logical address 0x21000000*/
    {0x80000000,    0x800000000>>4,  0x80000000,     MP_SR|MP_SW|MP_SX|MP_UR|MP_UW|MP_UX}/*DDR, RWX*/
};

unsigned int tscl, tsch;
int iFailedTimes=0, iPassedTimes=0;
unsigned int uiLL2StartAddress, uiLL2EndAddress; 
unsigned int uiSL2EndAddress, uiRemappedSL2EndAddress, uiDDREndAddress;
int number_of_cores;

#define DMA_BUF_SIZE 	(32*1024)
unsigned char dma_buf[DMA_BUF_SIZE];

void pass_fail_count(int iResult) 
{
	if(iResult)
		iFailedTimes++;
	else
		iPassedTimes++;
	printf("Passed Times: %8d \t\tFailed Times: %8d \n", iPassedTimes, iFailedTimes);
}

void LL1_memory_IDMA_test()
{
	tscl= TSCL;
	tsch= TSCH;
	printf("\nlocal L1 memory test with IDMA at %lld cycle\n", _itoll(tsch, tscl));

	//enable all access to L1
	L1P_memory_protection_cfg(0xFFFF);
	L1D_memory_protection_cfg(0xFFFF);

	/*Test L1D with EDMA*/
	pass_fail_count(IDMA_MEM_Test(0xf00000, 0xf08000,
			(unsigned int)dma_buf, DMA_BUF_SIZE));
	/*Test L1P with EDMA*/
	pass_fail_count(IDMA_MEM_Test(0xe00000, 0xe08000,
			(unsigned int)dma_buf, DMA_BUF_SIZE));

	//protect L1 from any access except for cache
	L1_cache_protection();

}

void KeyStone_memory_bus_test(unsigned int uiBaseAddress,
	unsigned int uiDataBusWidth, unsigned int uiNumBytes, char * mem_name)
{
	Uint32 uiFailCount;
	
	tscl= TSCL;
	tsch= TSCH;
	printf("\n%s bus test at %lld cycle\n", mem_name, _itoll(tsch, tscl));

	//Test memory buses		
	uiFailCount  = Memory_Data_Bus_Test(uiBaseAddress, uiDataBusWidth);
#if (DDR_ECC_ENABLE==0)
	uiFailCount += Memory_Address_Bus_Test(uiBaseAddress, uiNumBytes, 1);
#endif
	pass_fail_count(uiFailCount);

}
void KeyStone_memory_test(unsigned int uiStartAddress, 
	unsigned int uiStopAddress, unsigned int uiStep, char * mem_name)
{
	tscl= TSCL;
	tsch= TSCH;
	printf("\n%s memory test at %lld cycle\n", mem_name, _itoll(tsch, tscl));
	pass_fail_count(DSP_core_MEM_Test(uiStartAddress, uiStopAddress, uiStep)); 
}

void KeyStone_memory_EDMA_test(unsigned int uiStartAddress, 
	unsigned int uiStopAddress, unsigned int uiStep, char * mem_name)
{
	tscl= TSCL;
	tsch= TSCH;
	printf("\n%s memory test with EDMA at %lld cycle\n", mem_name, _itoll(tsch, tscl));
	pass_fail_count(EDMA_MEM_Test(GLOBAL_ADDR(uiStartAddress), GLOBAL_ADDR(uiStopAddress),
			GLOBAL_ADDR(dma_buf), DMA_BUF_SIZE)); 
}

void KeyStone_other_L2_memory_test()
{
	int i;
	
	tscl= TSCL;
	tsch= TSCH;
	printf("\nother L2 memory test at %lld cycle\n", _itoll(tsch, tscl));

	//Test other cores L2 memory		
	for(i=0; i< number_of_cores; i++)
	{
		if(i!= DNUM)
		{
			pass_fail_count(DSP_core_MEM_Test(0x10800000+i*0x1000000, 
				0x10000000+i*0x1000000+uiLL2EndAddress, 1));
		}
	}
}

void KeyStone_other_L2_memory_EDMA_test()
{
	int i;
	
	tscl= TSCL;
	tsch= TSCH;
	printf("\nother L2 memory test with EDMA at %lld cycle\n", _itoll(tsch, tscl));

	//Test other cores L2 memory		
	for(i=0; i< number_of_cores; i++)
	{
		if(i!= DNUM)
		{
			pass_fail_count(EDMA_MEM_Test(0x10800000+i*0x1000000, 
				0x10000000+i*0x1000000+uiLL2EndAddress, 
				GLOBAL_ADDR(dma_buf), DMA_BUF_SIZE));
		}
	}
}

void KeyStone_other_L1_memory_EDMA_test()
{
	int i;
	
	tscl= TSCL;
	tsch= TSCH;
	printf("\nother L1 memory test with EDMA at %lld cycle\n", _itoll(tsch, tscl));

	for(i=0; i< number_of_cores; i++)
	{
		if(i!= DNUM)
		{
			pass_fail_count(EDMA_MEM_Test(0x10f00000+i*0x1000000, 0x10f08000+i*0x1000000,
				GLOBAL_ADDR(dma_buf), DMA_BUF_SIZE));
			pass_fail_count(EDMA_MEM_Test(0x10e00000+i*0x1000000, 0x10e08000+i*0x1000000,
				GLOBAL_ADDR(dma_buf), DMA_BUF_SIZE));
		}
	}
}

void main()
{
	int i;
	DDR_ECC_Config DDR_ECC_cfg;

	/*enable TSC, memory protection interrupts, EDC for internal RAM;
    clear cache; protect L1 as cache*/
	KeyStone_common_CPU_init();
	/*print device information. 
	Enable memory protection interrupts, EDC for MSMC RAM*/
	KeyStone_common_device_init();

	//enable exception handling
	KeyStone_Exception_cfg(TRUE);

	//reset cache from previous run. Not required from cold reset or power up.
	CACHE_setL1PSize(CACHE_L1_0KCACHE);
	CACHE_setL1DSize(CACHE_L1_0KCACHE);
	CACHE_setL2Size(CACHE_0KCACHE);
	//disable cache and prefetch
	for(i= 16; i< 256; i++)
		gpCGEM_regs->MAR[i]=0;		

	/*XMC memory address extension/mapping and memory protection.
	*remap Shared L2 to 0x18000000 for noncacheable and nonprefetchable access*/
	KeyStone_XMC_MPAX_setup(XMC_MPAX_cfg_table, 0,
		sizeof(XMC_MPAX_cfg_table)/sizeof(MPAX_Config));

	EDMA_init();

	/*allocate EDMA TCs between cores sequentially*/
	allocate_EDMA_TC(DNUM, 1);

	memset((void *)&DDR_ECC_cfg, 0, sizeof(DDR_ECC_cfg));
#if DDR_ECC_ENABLE
	DDR_ECC_cfg.addressRange[0].startAddr= 0x00000000;
	DDR_ECC_cfg.addressRange[0].byteCnt=   0x08000000;
	DDR_ECC_cfg.addressRange[1].startAddr= 0x10000000;
	DDR_ECC_cfg.addressRange[1].byteCnt=   0x08000000;
	DDR_ECC_cfg.rangeMode= EN_ECC_WITHIN_DEFINED_RANGES;
#endif

	if(C6678_EVM==gDSP_board_type)
	{
		//DSP core speed: 100*10/1=1000MHz
		KeyStone_main_PLL_init(100, 10, 1); 
	}
	else if(TCI6614_EVM==gDSP_board_type
		||DUAL_NYQUIST_EVM==gDSP_board_type
		||C6670_EVM==gDSP_board_type)
	{
		//DSP core speed: 122.88*236/29= 999.9889655MHz
		KeyStone_main_PLL_init(122.88, 236, 29);
	}
	else
	{
		puts("Unknown DSP board type!");
		return;
	}

	//DDR init 66.66667*20/1= 1333
	KeyStone_DDR_init (66.66667, 20, 1, &DDR_ECC_cfg);

	//make remapped SL2 noncacheable, nonprefetchable
	gpCGEM_regs->MAR[(REMAPPED_SL2_TEST_START_ARRD/16/1024/1024)]=0;	

	/*make DDR cacheable and prefetchable*/
	for(i= 0; i< (uiDDREndAddress-DDR_TEST_START_ARRD)/16/1024/1024; i++)
		gpCGEM_regs->MAR[(DDR_TEST_START_ARRD/16/1024/1024)+i]=1|
			(1<<CSL_CGEM_MAR0_PFX_SHIFT);		

	/*the value of NUM_MM field, 0 means 1, 1 means 2...*/
	number_of_cores= ((gpCGEM_regs->L2CFG&CSL_CGEM_L2CFG_NUM_MM_MASK)>>
		CSL_CGEM_L2CFG_NUM_MM_SHIFT)+1;
		
	//make other cores L2 cacheable and prefetchable
	for(i=0; i< number_of_cores; i++)
	{
		gpCGEM_regs->MAR[0x10800000/16/1024/1024+i]=1|
			(1<<CSL_CGEM_MAR0_PFX_SHIFT);	
	}

	if(DUAL_NYQUIST_EVM==gDSP_board_type||TCI6614_EVM==gDSP_board_type)
	{
		/*Nyquist/Appleton has 1MB LL2/core, 2MB SL2*/
		uiLL2EndAddress= 0x00900000;
		uiSL2EndAddress= 0x0C200000;
		uiRemappedSL2EndAddress= REMAPPED_SL2_TEST_START_ARRD+0x200000;
		uiDDREndAddress= 0xC0000000; /*1GB on Dual Nyquist/Appleton EVM*/
	}
	else if(C6670_EVM==gDSP_board_type)
	{
		/*Nyquist has 1MB LL2/core, 2MB SL2*/
		uiLL2EndAddress= 0x00900000;
		uiSL2EndAddress= 0x0C200000;
		uiRemappedSL2EndAddress= REMAPPED_SL2_TEST_START_ARRD+0x200000;
		uiDDREndAddress= 0xA0000000; /*512MB on Single Nyquist EVM*/
	}
	else if(C6678_EVM==gDSP_board_type)
	{
		/*Shannon has 0.5MB LL2/core, 4MB SL2*/
		uiLL2EndAddress= 0x00880000;
		uiSL2EndAddress= 0x0C400000;
		uiRemappedSL2EndAddress= REMAPPED_SL2_TEST_START_ARRD+0x400000;
		uiDDREndAddress= 0xA0000000; /*512MB on C6678_EVM EVM*/
	}
	else
	{
		puts("Unknown DSP board type!");
		uiLL2EndAddress= 0x00840000;
		uiSL2EndAddress= 0x0C040000;
		uiRemappedSL2EndAddress= REMAPPED_SL2_TEST_START_ARRD+0x40000;
		uiDDREndAddress= 0x81000000;
	}

	uiLL2StartAddress= get_unused_L2_address();

#if TEST_INFINITELY
	while(1) 	/*test infinitely*/
#endif
	{
		tscl= TSCL;
		tsch= TSCH;
		printf("Memory Test Start at %lld cycle\n", _itoll(tsch, tscl));

		CACHE_setL1PSize(CACHE_L1_0KCACHE);
		CACHE_setL1DSize(CACHE_L1_0KCACHE);
		CACHE_setL2Size(CACHE_0KCACHE);
		puts("\nSet 0KB L1P, 0KB L1D, 0KB L2");

#if (LL2_MEM_TEST&TEST_BY_DSP_CORE)
		KeyStone_memory_bus_test(0x800000+(uiLL2EndAddress-0x800000)/2, 256*4, 
			(uiLL2EndAddress-0x800000)/2, "LL2");
#endif

#if (SL2_MEM_TEST&TEST_BY_DSP_CORE)
		KeyStone_memory_bus_test(SL2_TEST_START_ARRD, 256*4, 
			uiSL2EndAddress-SL2_TEST_START_ARRD, "SL2");
#endif

#if (EXTERNAL_MEM_TEST&TEST_BY_DSP_CORE)
		KeyStone_memory_bus_test(DDR_TEST_START_ARRD, 64, 
			uiDDREndAddress-DDR_TEST_START_ARRD, "DDR");
#endif

#if (LL1_MEM_TEST&TEST_BY_DMA)
		LL1_memory_IDMA_test();
#endif
#if (OTHER_L1_TEST&TEST_BY_DMA)
		KeyStone_other_L1_memory_EDMA_test();
#endif

		CACHE_setL1PSize(CACHE_L1_32KCACHE);
		CACHE_setL1DSize(CACHE_L1_0KCACHE);
		CACHE_setL2Size(CACHE_0KCACHE);
		puts("\nSet 32KB L1P, 0KB L1D, 0KB L2");
#if (LL2_MEM_TEST&TEST_BY_DSP_CORE)
		KeyStone_memory_test(uiLL2StartAddress, uiLL2EndAddress, 1, "Local L2");
#endif
#if (OTHER_L2_TEST&TEST_BY_DSP_CORE)
		KeyStone_other_L2_memory_test();
#endif
#if (SL2_MEM_TEST&TEST_BY_DSP_CORE)
		KeyStone_memory_test(0x0C000000, uiSL2EndAddress, 1, "Shared L2");
#endif
#if (EXTERNAL_MEM_TEST&TEST_BY_DSP_CORE)
		KeyStone_memory_test(0x80000000, 0x80000000+1024, 1, "DDR3");
#endif

		CACHE_setL1PSize(CACHE_L1_32KCACHE);
		CACHE_setL1DSize(CACHE_L1_32KCACHE);
		CACHE_setL2Size(CACHE_0KCACHE);
		puts("\nSet 32KB L1P, 32KB L1D, 0KB L2");
#if (LL2_MEM_TEST&TEST_BY_DMA)
		KeyStone_memory_EDMA_test(uiLL2StartAddress, uiLL2EndAddress, 1, "Local L2");
#endif
#if (LL2_MEM_TEST&TEST_BY_DSP_CORE)
		KeyStone_memory_test(uiLL2StartAddress, uiLL2EndAddress, 1, "Local L2");
#endif
#if (OTHER_L2_TEST&TEST_BY_DMA)
		KeyStone_other_L2_memory_EDMA_test();
#endif
#if (OTHER_L2_TEST&TEST_BY_DSP_CORE)
		KeyStone_other_L2_memory_test();
#endif
#if (SL2_MEM_TEST&TEST_BY_DMA)
		KeyStone_memory_EDMA_test(0x0C000000, uiSL2EndAddress, 1, "Shared L2");
#endif
#if (SL2_MEM_TEST&TEST_BY_DSP_CORE)
		KeyStone_memory_test(0x0C000000, uiSL2EndAddress, 1, "Shared L2");
#endif
#if (EXTERNAL_MEM_TEST&TEST_BY_DMA)
		KeyStone_memory_EDMA_test(0x80000000, uiDDREndAddress, 1, "DDR3");
#endif
#if (EXTERNAL_MEM_TEST&TEST_BY_DSP_CORE)
		KeyStone_memory_test(0x80000000, 0x80000000+64*1024, 1, "DDR3");
#endif

		CACHE_setL1PSize(CACHE_L1_32KCACHE);
		CACHE_setL1DSize(CACHE_L1_32KCACHE);
		CACHE_setL2Size(CACHE_256KCACHE);
		puts("\nSet 32KB L1P, 32KB L1D, 256KB L2");
#if (OTHER_L2_TEST&TEST_BY_DSP_CORE)
		KeyStone_other_L2_memory_test();
#endif
#if (EXTERNAL_MEM_TEST&TEST_BY_DSP_CORE)
		KeyStone_memory_test(0x80000000, 0x80000000+512*1024, 1, "DDR3");
#endif

#if (SL2_MEM_TEST&TEST_BY_DSP_CORE)
		KeyStone_memory_test(REMAPPED_SL2_TEST_START_ARRD, uiRemappedSL2EndAddress, 1, "Remapped Shared L2");
#endif

		tscl= TSCL;
		tsch= TSCH;
		printf("Memory test complete at %lld cycle\n", _itoll(tsch, tscl));
	}
}

