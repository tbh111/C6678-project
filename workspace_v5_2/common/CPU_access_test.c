/******************************************************************************

  Copyright (C), 2014, Texas Instrument.

 ******************************************************************************
  File Name     : CPU_access_test.c
  Version       : Initial Draft
  Author        : Brighton Feng
  Created       : June 5, 2014
  Last Modified : 
  Description   : CPU read/write test functions and definitions
  History       :
  1.Date        : June 5, 2014
    Author      : Brighton Feng
    Modification: Created file

******************************************************************************/

#include <stdio.h>
#include <csl_edma3.h>
#include "KeyStone_common.h"
#include "CPU_access_test.h"

/*execute LDDW test in "buff" of "uiByteCnt" with different index*/
void LDDWTest(Uint32 buff_addr, Uint32 uiByteCnt)
{
	unsigned int cycles;
	Uint32 		uiIndex;

	printf("Memory access performance test at 0x%x\n", buff_addr);
	uiIndex=0;
	while(uiIndex*8*LOAD_STORE_TIMES<uiByteCnt)
	{
		CACHE_wbInvAllL1d(CACHE_WAIT);
		CACHE_wbInvAllL2(CACHE_WAIT);
		_mfence();
		_mfence();

		cycles= TSCL;
		Asm_LDDW_Test(buff_addr, uiIndex, LOAD_STORE_TIMES);
		cycles= TSC_count_cycle_from(cycles)-LD_ST_TEST_OVERHEAD;
		printf("Index=%4d  Cycles/LDDW= %.2f\n", uiIndex, (float)cycles/(LOAD_STORE_TIMES));
		if(uiIndex<8)
			uiIndex++;
		else if(uiIndex<32)
			uiIndex+=2;
		else if(uiIndex<64)
			uiIndex+=8;
		else if(uiIndex<128)
			uiIndex+=32;
		else if(uiIndex<256)
			uiIndex+=64;
		else if(uiIndex<512)
			uiIndex+=128;
		else if(uiIndex<1024)
			uiIndex+=256;
		else if(uiIndex<2048)
			uiIndex+=512;
		else if(uiIndex<4096)
			uiIndex+=1024;
		else if(uiIndex<8192)
			uiIndex+=2048;
		else 
			uiIndex+=4096;
	}
}

/*execute STDW test in "buff" of "uiByteCnt" with different index*/
void STDWTest(Uint32 buff_addr, Uint32 uiByteCnt)
{
	unsigned int cycles;
	Uint32 		uiIndex;

	printf("Memory access performance test at 0x%x\n", buff_addr);
	uiIndex=0;
	while(uiIndex*8*LOAD_STORE_TIMES<uiByteCnt)
	{
		CACHE_wbInvAllL1d(CACHE_WAIT);
		CACHE_wbInvAllL2(CACHE_WAIT);
		_mfence();
		_mfence();

		cycles= TSCL;
		Asm_STDW_Test(buff_addr, uiIndex, LOAD_STORE_TIMES);
		cycles= TSC_count_cycle_from(cycles)-LD_ST_TEST_OVERHEAD;
		printf("Index=%4d  Cycles/STDW= %.2f\n", uiIndex, (float)cycles/(LOAD_STORE_TIMES));
		if(uiIndex<8)
			uiIndex++;
		else if(uiIndex<32)
			uiIndex+=2;
		else if(uiIndex<64)
			uiIndex+=8;
		else if(uiIndex<128)
			uiIndex+=32;
		else if(uiIndex<256)
			uiIndex+=64;
		else if(uiIndex<512)
			uiIndex+=128;
		else if(uiIndex<1024)
			uiIndex+=256;
		else if(uiIndex<2048)
			uiIndex+=512;
		else if(uiIndex<4096)
			uiIndex+=1024;
		else if(uiIndex<8192)
			uiIndex+=2048;
		else 
			uiIndex+=4096;
	}
}


