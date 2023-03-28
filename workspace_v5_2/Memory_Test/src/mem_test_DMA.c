/*
memory test with DMA
Author: Brighton Feng
Created on 2010-10-27
last modified on 2010-11-5
*/
#include <stdio.h>
#include <soc.h>
#include "KeyStone_common.h"

#define MAX_FILL_FAIL_COUNT 		(10)
#define MAX_ADDRESS_FAIL_COUNT 		(10)
#define MAX_BITWALKING_FAIL_COUNT 	(2)
#define MAX_BITWALKING_RANGE 		(1024*1024*1024) 	/*test smaller range to save time*/

#define BIT_PATTERN_FILLING_TEST 	1
#define ADDRESS_TEST 				1
#define BIT_WALKING_TEST 			1

#define PRINT_DETAILS 			1
#if PRINT_DETAILS
#define 	PRINT	 		printf
#else
#define 	PRINT			// 		
#endif

extern unsigned long long ulDataPatternTable[];

EDMA_CC_Channel_Num TC_channel_Table[NUM_EDMA_TC]=
{
	EDMA_CC0_CH0,
	EDMA_CC0_CH1,
	EDMA_CC1_CH0,
	EDMA_CC1_CH1,
	EDMA_CC1_CH2,
	EDMA_CC1_CH3,
	EDMA_CC2_CH0,
	EDMA_CC2_CH1,
	EDMA_CC2_CH2,
	EDMA_CC2_CH3
#if (NUM_EDMA_TC>10)
	,EDMA_CC3_CH0,
	EDMA_CC3_CH1,
	EDMA_CC4_CH0,
	EDMA_CC4_CH1
#endif
};

EDMA_CC_Channel_Num available_TC_channel_Table[NUM_EDMA_TC];
unsigned int num_available_TC=0;

/*allocate EDMA TCs between cores sequentially
this function must be called before EDMA test*/
void allocate_EDMA_TC(unsigned int core_num, unsigned int number_of_cores)
{
	int i;
	for(i=0; i<NUM_EDMA_TC; i++)
		if(core_num==i%number_of_cores)
			available_TC_channel_Table[num_available_TC++]= TC_channel_Table[i];
}

/*get an availible TC sequentially*/
unsigned int uiTC_channel= 0;
EDMA_CC_Channel_Num get_EDMA_TC()
{
	return available_TC_channel_Table[(uiTC_channel++)%num_available_TC];
}

/*
This function fills the patterns into memory and reads back and verification
*/
unsigned int DMA_MEM_FillTest(unsigned int uiStartAddress,
                        unsigned int uiByteCount,
                        unsigned long long * dma_buf,
                        unsigned int uiDmaBufByteSize,
                        unsigned long long ulBitPattern,
                        EDMA_CC_Channel_Num TC_channel,
                        unsigned int use_idma)
{
    unsigned int i, uiFailCount=0;
    unsigned int uiAddress, uiEndAddress, uiDMA_count;
    volatile unsigned long long ulReadBack;

	uiEndAddress= uiStartAddress + uiByteCount;
    /* Write the pattern to DMA buffer*/
	for(i= 0; i< uiDmaBufByteSize/8; i++)
		dma_buf[i]= ulBitPattern;

	/*DMA the pattern to memory under test*/
	uiAddress = uiStartAddress;
	while(uiAddress < uiEndAddress)
	{
		uiDMA_count = uiEndAddress - uiAddress;
		if(uiDMA_count > uiDmaBufByteSize)
			uiDMA_count = uiDmaBufByteSize;
		if(use_idma)
			IDMA_copy((unsigned int)dma_buf, uiAddress, uiDMA_count, DMA_WAIT);
		else
			EDMA_copy((unsigned int)dma_buf, uiAddress, uiDMA_count, TC_channel, DMA_WAIT);
        uiAddress += uiDMA_count;
    }

	uiAddress = uiStartAddress;
	while(uiAddress < uiEndAddress)
	{
		/*read back the data with DMA*/
		uiDMA_count = uiEndAddress - uiAddress;
		if(uiDMA_count > uiDmaBufByteSize)
			uiDMA_count = uiDmaBufByteSize;
		if(use_idma)
			IDMA_copy(uiAddress, (unsigned int)dma_buf, uiDMA_count, DMA_WAIT);
		else
			EDMA_copy(uiAddress, (unsigned int)dma_buf, uiDMA_count, TC_channel, DMA_WAIT);

		/*compare the data*/
		for(i= 0; i< uiDMA_count/8; i++)
		{
			ulReadBack = dma_buf[i];
	        if (ulReadBack != ulBitPattern)      /* verify pattern */
	        {
				PRINT("  Memory Test with DMA fails at 0x%8x, Write 0x%016llx, Readback 0x%016llx\n", uiAddress + i*8, ulBitPattern, ulReadBack);
	            uiFailCount++;
				if(uiFailCount>=MAX_FILL_FAIL_COUNT)
					return uiFailCount;
	        }
		}
        uiAddress += uiDMA_count;
    }
	return uiFailCount;
}

/*
this function write the address to corresponding memory unit and readback for verification
*/
unsigned int DMA_MEM_AddrTest(unsigned int uiStartAddress, 
						unsigned int uiByteCount,
                        unsigned long long * dma_buf,
                        unsigned int uiDmaBufByteSize,
                        EDMA_CC_Channel_Num TC_channel,
                        unsigned int use_idma)
{
    unsigned int i, uiFailCount=0;
    unsigned int uiAddress, uiEndAddress, uiDMA_count;
    volatile unsigned long long ulReadBack;

	uiEndAddress= uiStartAddress + uiByteCount;
	uiAddress = uiStartAddress;
	while(uiAddress < uiEndAddress)
	{
	    /* Write the address of memory under test to DMA buffer*/
		for(i= 0; i< uiDmaBufByteSize/8; i++)
			dma_buf[i]= _itoll(uiAddress + i*8 + 4, uiAddress + i*8);

		/*DMA the data to memory under test*/
		uiDMA_count = uiEndAddress - uiAddress;
		if(uiDMA_count > uiDmaBufByteSize)
			uiDMA_count = uiDmaBufByteSize;
		if(use_idma)
			IDMA_copy((unsigned int)dma_buf, uiAddress, uiDMA_count, DMA_WAIT);
		else
			EDMA_copy((unsigned int)dma_buf, uiAddress, uiDMA_count, TC_channel, DMA_WAIT);
        uiAddress += uiDMA_count;
    }

	uiAddress = uiStartAddress;
	while(uiAddress < uiEndAddress)
	{
		/*read back the data with DMA*/
		uiDMA_count = uiEndAddress - uiAddress;
		if(uiDMA_count > uiDmaBufByteSize)
			uiDMA_count = uiDmaBufByteSize;
		if(use_idma)
			IDMA_copy(uiAddress, (unsigned int)dma_buf, uiDMA_count, DMA_WAIT);
		else
			EDMA_copy(uiAddress, (unsigned int)dma_buf, uiDMA_count, TC_channel, DMA_WAIT);

		/*compare the data*/
		for(i= 0; i< uiDMA_count/8; i++)
		{
			ulReadBack = dma_buf[i];
	        if (ulReadBack != _itoll(uiAddress + i*8 + 4, uiAddress + i*8))      
	        {
				PRINT("  Memory Test with DMA fails at 0x%8x, Write 0x%016llx, Readback 0x%016llx\n", uiAddress + i*8, _itoll(uiAddress + i*8 + 4, uiAddress + i*8), ulReadBack);
	            uiFailCount++;
				if(uiFailCount>=MAX_ADDRESS_FAIL_COUNT)
					return uiFailCount;
	        }
		}
        uiAddress += uiDMA_count;
    }
	return uiFailCount;
}

unsigned int EDMA_MEM_Bit_Walking(unsigned int uiStartAddress,
						unsigned int uiByteCount,
                        unsigned long long * dma_buf,
                        unsigned int uiDmaBufByteSize)
{
    unsigned int uiFailCount=0;
    unsigned int j;
	EDMA_CC_Channel_Num	TC_channel;
    unsigned int uiBitMask=1;
    for (j = 0; j < 32; j++)
    {
    	TC_channel= get_EDMA_TC();
    	if(uiByteCount>1024*1024)
	  		printf("Memory Bit Walking with EDMA CC%d TC%d at 0x%8x with pattern 0x%16llx\n", ((Uint32)TC_channel)>>16, ((Uint32)TC_channel)&0xFFFF, uiStartAddress, _itoll(uiBitMask, uiBitMask));
        if(DMA_MEM_FillTest(uiStartAddress, uiByteCount,
        	dma_buf, uiDmaBufByteSize, _itoll(uiBitMask, uiBitMask), TC_channel, 0))
        	uiFailCount++;

    	TC_channel= get_EDMA_TC();
    	if(uiByteCount>1024*1024)
	  		printf("Memory Bit Walking with EDMA CC%d TC%d at 0x%8x with pattern 0x%16llx\n", ((Uint32)TC_channel)>>16, ((Uint32)TC_channel)&0xFFFF, uiStartAddress, _itoll(~uiBitMask, ~uiBitMask));
        if(DMA_MEM_FillTest(uiStartAddress, uiByteCount,
        	dma_buf, uiDmaBufByteSize, _itoll(~uiBitMask, ~uiBitMask), TC_channel, 0))
        	uiFailCount++;

		if(uiFailCount>= MAX_BITWALKING_FAIL_COUNT)
			return uiFailCount;

        uiBitMask <<= 1;
    }
	return uiFailCount;
}


unsigned int IDMA_MEM_Bit_Walking(unsigned int uiStartAddress,
						unsigned int uiByteCount,
                        unsigned long long * dma_buf,
                        unsigned int uiDmaBufByteSize)
{
    unsigned int uiFailCount=0;
    unsigned int j;
    unsigned int uiBitMask=1;
    for (j = 0; j < 32; j++)
    {
    	if(uiByteCount>1024*1024)
		  	printf("Memory Bit Walking with IDMA at 0x%8x with pattern 0x%16llx\n", uiStartAddress, _itoll(uiBitMask, uiBitMask));
        if(DMA_MEM_FillTest(uiStartAddress, uiByteCount,
        	dma_buf, uiDmaBufByteSize, _itoll(uiBitMask, uiBitMask), 0, 1))
        	uiFailCount++;
    	if(uiByteCount>1024*1024)
		  	printf("Memory Bit Walking with IDMA at 0x%8x with pattern 0x%16llx\n", uiStartAddress, _itoll(~uiBitMask, ~uiBitMask));
        if(DMA_MEM_FillTest(uiStartAddress, uiByteCount,
        	dma_buf, uiDmaBufByteSize, _itoll(~uiBitMask, ~uiBitMask), 0, 1))
        	uiFailCount++;

		if(uiFailCount>= MAX_BITWALKING_FAIL_COUNT)
			return uiFailCount;

        uiBitMask <<= 1;
    }
	return uiFailCount;
}


int EDMA_MEM_Test(unsigned int uiStartAddress, unsigned int uiStopAddress,
                        unsigned int uiDmaBufAddress,
                        unsigned int uiDmaBufByteSize)
{
    int j;
    unsigned int uiByteCount, uiFailCount=0, uiTotalFailCount=0;
    unsigned int TC_channel;

    uiByteCount = (uiStopAddress - uiStartAddress);

#if BIT_PATTERN_FILLING_TEST
    // Fill with values from pattern table
    for (j = 0; j < 4; j++)
    {
    	TC_channel= get_EDMA_TC();
        uiFailCount = DMA_MEM_FillTest(uiStartAddress, uiByteCount, 
        	(unsigned long long *)uiDmaBufAddress, uiDmaBufByteSize, ulDataPatternTable[j], TC_channel, 0);
	    if (uiFailCount)
	    {
	    	PRINT(" !!!Failed Memory Fill Test at %d Units with pattern 0x%016llx with EDMA CC%d TC%d\n",uiFailCount, ulDataPatternTable[j], ((Uint32)TC_channel)>>16, ((Uint32)TC_channel)&0xFFFF);
	        uiTotalFailCount+=uiFailCount;
	    }
		else
		   	PRINT("Passed Memory Fill Test from 0x%8x to 0x%8x with pattern 0x%16llx with EDMA CC%d TC%d\n",uiStartAddress,uiStopAddress, ulDataPatternTable[j], ((Uint32)TC_channel)>>16, ((Uint32)TC_channel)&0xFFFF);
    }
    
#endif
#if ADDRESS_TEST    
   	TC_channel= get_EDMA_TC();
    // Test the address range from low to high
    uiFailCount = DMA_MEM_AddrTest(uiStartAddress, uiByteCount, 
    	(unsigned long long *)uiDmaBufAddress, uiDmaBufByteSize, TC_channel, 0); 
    if (uiFailCount)
    {
    	PRINT("!!!Failed Memory Address Test at %d units with EDMA CC%d TC%d\n",uiFailCount, ((Uint32)TC_channel)>>16, ((Uint32)TC_channel)&0xFFFF);
        uiTotalFailCount+=uiFailCount;
    }
	else
	   	PRINT("Passed Memory Address Test from 0x%8x to 0x%8x with EDMA CC%d TC%d\n",uiStartAddress,uiStopAddress, ((Uint32)TC_channel)>>16, ((Uint32)TC_channel)&0xFFFF);

#endif
#if BIT_WALKING_TEST
    // Do walking ones and zeroes test
    if(uiByteCount>MAX_BITWALKING_RANGE)
			uiByteCount= MAX_BITWALKING_RANGE; //test smaller range to save time
    uiFailCount = EDMA_MEM_Bit_Walking(uiStartAddress,uiByteCount,
    	(unsigned long long *)uiDmaBufAddress, uiDmaBufByteSize); 
    if (uiFailCount)
    {
	   	PRINT(" !!!Failed Memory Bit Walking at %d units with DMA\n",uiFailCount);
        uiTotalFailCount+=uiFailCount;
    }    
	else
	  	PRINT("Passed Memory Bit Walking from 0x%8x to 0x%8x with DMA\n",uiStartAddress,uiStopAddress);
#endif
  	return uiTotalFailCount;

}
int IDMA_MEM_Test(unsigned int uiStartAddress, unsigned int uiStopAddress,
                        unsigned int  uiDmaBufAddress,
                        unsigned int uiDmaBufByteSize)
{
    int j;
    unsigned int uiByteCount, uiFailCount=0, uiTotalFailCount=0;

    uiByteCount = (uiStopAddress - uiStartAddress);

#if BIT_PATTERN_FILLING_TEST
    // Fill with values from pattern table
    for (j = 0; j < 4; j++)
    {
        uiFailCount = DMA_MEM_FillTest(uiStartAddress, uiByteCount,
        	(unsigned long long *)uiDmaBufAddress, uiDmaBufByteSize, ulDataPatternTable[j], 0, 1);
	    if (uiFailCount)
	    {
	    	PRINT(" !!!Failed Memory Fill Test at %d Units with pattern 0x%016llx with IDMA\n",uiFailCount, ulDataPatternTable[j]);
	        uiTotalFailCount+=uiFailCount;
	    }
		else
		   	PRINT("Passed Memory Fill Test from 0x%8x to 0x%8x with pattern 0x%16llx with IDMA\n",uiStartAddress,uiStopAddress, ulDataPatternTable[j]);
    }
    
#endif
#if ADDRESS_TEST    
    // Test the address range from low to high
    uiFailCount = DMA_MEM_AddrTest(uiStartAddress, uiByteCount,
    	(unsigned long long *)uiDmaBufAddress, uiDmaBufByteSize, 0, 1); 
    if (uiFailCount)
    {
    	PRINT("!!!Failed Memory Address Test at %d units with IDMA\n",uiFailCount);
        uiTotalFailCount+=uiFailCount;
    }
	else
	   	PRINT("Passed Memory Address Test from 0x%8x to 0x%8x with IDMA\n",uiStartAddress,uiStopAddress);

#endif
#if BIT_WALKING_TEST
    // Do walking ones and zeroes test
    uiFailCount = IDMA_MEM_Bit_Walking(uiStartAddress,uiByteCount,
    	(unsigned long long *)uiDmaBufAddress, uiDmaBufByteSize); 
    if (uiFailCount)
    {
	   	PRINT("!!!Failed Memory Bit Walking at %d units with IDMA\n",uiFailCount);
        uiTotalFailCount+=uiFailCount;
    }    
	else
	  	PRINT("Passed Memory Bit Walking from 0x%8x to 0x%8x with IDMA\n",uiStartAddress,uiStopAddress);
#endif
  	return uiTotalFailCount;

}

