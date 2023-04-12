/*
FLASH memory test
Author: Brighton Feng
Created on 2012-1-17
Updated on 2014-12-8, save/restore flash content during test
*/
#include <stdio.h>
#include "KeyStone_common.h"

#define MAX_FILL_FAIL_COUNT 		(10)
#define MAX_ADDRESS_FAIL_COUNT 		(10)
#define MAX_BITWALKING_FAIL_COUNT 	(2)

#define BIT_PATTERN_FILLING_TEST 	1
#define ADDRESS_TEST 				1
#define BIT_WALKING_TEST 			0

#define SAVE_FLASH_CONTENT 			1
#define SAVE_BUFFER_ADDRESS 		0x80000000
#define SAVE_BUFFER_BYTE_COUNT 		0x20000000

#define PRINT_DETAILS 			1
#if PRINT_DETAILS
#define 	PRINT	 		printf
#else
#define 	PRINT			// 		
#endif

unsigned int uiDataPatternTable[] = { 0x00000000, /*0xffffffff,*/ 0xaaaaaaaa, 0x55555555 };

//call back function for FLASH erase
int (* FLASH_erase_blocks)(unsigned int flashAddress, 
	unsigned int byteNumber);

//call back function for FLASH write
int (* FLASH_write_block)(unsigned int srcDatAddress, 
	unsigned int dstFlashAddress, unsigned int byteNumber);

//call back function for FLASH read
int (* FLASH_read_block)(unsigned int dstDatAddress, 
	unsigned int srcFlashAddress, unsigned int byteNumber);

/*
This function fills the patterns into memory and reads back and verification
*/
unsigned int FLASH_MEM_FillTest(unsigned int uiStartAddress,
	unsigned int uiByteCount,
	unsigned int * temp_buf,
	unsigned int uiBufByteSize,
	unsigned int uiBitPattern)
{
    unsigned int i, uiFailCount=0;
    unsigned int uiAddress, uiEndAddress, uiCount;
    volatile unsigned int *uipAddressPointer;
    volatile unsigned int uiReadBack;

	uiEndAddress= uiStartAddress + uiByteCount;
    /* Write the pattern to DMA buffer*/
	for(i= 0; i< uiBufByteSize/4; i++)
		temp_buf[i]= uiBitPattern;

	/*copy the pattern to memory under test*/
	uiAddress = uiStartAddress;
	while(uiAddress < uiEndAddress)
	{
		uiCount = uiEndAddress - uiAddress;
		if(uiCount > uiBufByteSize)
			uiCount = uiBufByteSize;
		(*FLASH_write_block)((unsigned int)temp_buf, uiAddress, uiCount);
        uiAddress += uiBufByteSize;
    }

	if(NULL!=FLASH_read_block)
	{/*use call back function for the FLASH can not be directly read*/
		/*read back from memory under test*/
		uiAddress = uiStartAddress;
		while(uiAddress < uiEndAddress)
		{
			uiCount = uiEndAddress - uiAddress;
			if(uiCount > uiBufByteSize)
				uiCount = uiBufByteSize;
			(*FLASH_read_block)((unsigned int)temp_buf, uiAddress, uiCount);
			for(i=0; i<uiCount/4; i++)
			{
		        uiReadBack = temp_buf[i];
		        if ( uiReadBack!= uiBitPattern)      /* verify pattern */
		        {
					PRINT("Memory Test fails at 0x%8x, Write 0x%8x, Readback 0x%8x\n", uiAddress+i*4, uiBitPattern, uiReadBack);
		            uiFailCount++;
					if(uiFailCount>=MAX_FILL_FAIL_COUNT)
						return uiFailCount;
		        }
		    }
	        uiAddress += uiBufByteSize;
	    }
	}
	else 	/*for the FLASH can be directly read*/
	{
		uipAddressPointer = (unsigned int *)uiStartAddress;
		for(i=0; i<uiByteCount/4; i++)
		{
	        uiReadBack = *uipAddressPointer;
	        if ( uiReadBack!= uiBitPattern)      /* verify pattern */
	        {
				PRINT("Memory Test fails at 0x%8x, Write 0x%8x, Readback 0x%8x\n", (unsigned int)uipAddressPointer, uiBitPattern, uiReadBack);
	            uiFailCount++;
				if(uiFailCount>=MAX_FILL_FAIL_COUNT)
					return uiFailCount;
	        }
	        uipAddressPointer++;
	    }
	}
	return uiFailCount;
}

/*
this function write the address to corresponding memory unit and readback for verification
*/
unsigned int FLASH_MEM_AddrTest(unsigned int uiStartAddress, 
	unsigned int uiByteCount,
    unsigned int * temp_buf,
    unsigned int uiBufByteSize)
{
    unsigned int i, uiFailCount=0;
    unsigned int uiAddress, uiEndAddress, uiCount;
    volatile unsigned int *uipAddressPointer;
    volatile unsigned int uiReadBack;

	uiEndAddress= uiStartAddress + uiByteCount;
	uiAddress = uiStartAddress;
	while(uiAddress < uiEndAddress)
	{
	    /* Write the address of memory under test to DMA buffer*/
		for(i= 0; i< uiBufByteSize/4; i++)
			temp_buf[i]= uiAddress + i*4;

		/*DMA the data to memory under test*/
		uiCount = uiEndAddress - uiAddress;
		if(uiCount > uiBufByteSize)
			uiCount = uiBufByteSize;
		(*FLASH_write_block)((unsigned int)temp_buf, uiAddress, uiCount);
        uiAddress += uiBufByteSize;
    }

	if(NULL!=FLASH_read_block)
	{/*use call back function for the FLASH can not be directly read*/
		/*read back from memory under test*/
		uiAddress = uiStartAddress;
		while(uiAddress < uiEndAddress)
		{
			uiCount = uiEndAddress - uiAddress;
			if(uiCount > uiBufByteSize)
				uiCount = uiBufByteSize;
			(*FLASH_read_block)((unsigned int)temp_buf, uiAddress, uiCount);
			for(i=0; i<uiCount/4; i++)
			{
		        uiReadBack = temp_buf[i];
		        if ( uiReadBack != (uiAddress+i*4) ) /* verify data */
		        {
					PRINT("Memory Test fails at 0x%8x, Write 0x%8x, Readback 0x%8x\n", (uiAddress+i*4), (uiAddress+i*4), uiReadBack);
		            uiFailCount++;
					if(uiFailCount>=MAX_ADDRESS_FAIL_COUNT)
						return uiFailCount;
		        }
		    }
	        uiAddress += uiBufByteSize;
	    }
	}
	else 	/*for the FLASH can be directly read*/
	{
		uipAddressPointer = (unsigned int *)uiStartAddress;
		for(i=0; i<uiByteCount/4; i++)
		{
	        uiReadBack = *uipAddressPointer;
	        if ( uiReadBack != ( (unsigned int )(uipAddressPointer)) ) /* verify data */
	        {
				PRINT("Memory Test fails at 0x%8x, Write 0x%8x, Readback 0x%8x\n", (unsigned int)uipAddressPointer, (unsigned int)uipAddressPointer, uiReadBack);
	            uiFailCount++;
				if(uiFailCount>=MAX_ADDRESS_FAIL_COUNT)
					return uiFailCount;
	        }
	        uipAddressPointer++;
	    }
	}
	return uiFailCount;
}

unsigned int FLASH_MEM_Bit_Walking(unsigned int uiStartAddress,
	unsigned int uiByteCount,
	unsigned int * temp_buf,
	unsigned int uiBufByteSize)
{
    unsigned int uiFailCount=0;
    unsigned int j;
    unsigned int uiBitMask=0x00010001;
    for (j = 0; j < 16; j++) 	/*16-bit bus*/
    {
	  	PRINT("Memory Bit Walking at 0x%8x with pattern 0x%8x\n", uiStartAddress, uiBitMask);
		if((*FLASH_erase_blocks)(uiStartAddress, uiByteCount))
	    {
	    	PRINT("!!!Failed erase blocks at 0x%x for Memory bit walking Test\n",uiStartAddress);
	        return 1;
	    }
        if(FLASH_MEM_FillTest(uiStartAddress, uiByteCount,
        	temp_buf, uiBufByteSize, uiBitMask))
        	uiFailCount++;
	  	PRINT("Memory Bit Walking at 0x%8x with pattern 0x%8x\n", uiStartAddress, ~uiBitMask);
		if((*FLASH_erase_blocks)(uiStartAddress, uiByteCount))
	    {
	    	PRINT("!!!Failed erase blocks at 0x%x for Memory bit walking Test\n",uiStartAddress);
	        return 1;
	    }
        if(FLASH_MEM_FillTest(uiStartAddress, uiByteCount,
        	temp_buf, uiBufByteSize, ~uiBitMask))
        	uiFailCount++;

		if(uiFailCount>= MAX_BITWALKING_FAIL_COUNT)
			return uiFailCount;

        uiBitMask <<= 1;
    }
	return uiFailCount;
}

//save flash content before test
int FLASH_MEM_save(unsigned int uiStartAddress, 
	unsigned int uiStopAddress)
{
	int i;
	unsigned long long *ullpFLashAddress, *ullpBufferAddress;
	unsigned int uiByteCount;

    uiByteCount = (uiStopAddress - uiStartAddress);
    if(SAVE_BUFFER_BYTE_COUNT<uiByteCount)
    {
    	printf("Save buffer size %d < tested flash size %d!\n", 
    		SAVE_BUFFER_BYTE_COUNT, uiByteCount);
    	return 1;
    }
	

	if(NULL!=FLASH_read_block)
	{/*use call back function for the FLASH can not be directly read*/
		(*FLASH_read_block)(SAVE_BUFFER_ADDRESS, uiStartAddress, uiByteCount);
	}
	else 	/*for the FLASH can be directly read*/
	{
		ullpFLashAddress = (unsigned long long *)uiStartAddress;
		ullpBufferAddress = (unsigned long long *)SAVE_BUFFER_ADDRESS;
		for(i=0; i<uiByteCount/8; i++)
		{
	        *ullpBufferAddress = *ullpFLashAddress;
	    }
	}

	PRINT("Save data from 0x%08x to 0x%08x.\n", uiStartAddress, uiStopAddress);

	return 0;
}

//restore flash content after test
int FLASH_MEM_restore(unsigned int uiStartAddress, 
	unsigned int uiStopAddress)
{
	unsigned int uiByteCount;

    uiByteCount = (uiStopAddress - uiStartAddress);
    if(SAVE_BUFFER_BYTE_COUNT<uiByteCount)
    {
    	printf("Save buffer size %d < tested flash size %d!\n", 
    		SAVE_BUFFER_BYTE_COUNT, uiByteCount);
    	return 1;
    }

	/*use call back function for the FLASH can not be directly write*/
	(*FLASH_write_block)(SAVE_BUFFER_ADDRESS, uiStartAddress, uiByteCount);

	PRINT("Restore data from 0x%08x to 0x%08x.\n", uiStartAddress, uiStopAddress);

	return 0;
}

int FLASH_MEM_Test(unsigned int uiStartAddress, 
	unsigned int uiStopAddress,
	unsigned int uiBufAddress,
	unsigned int uiBufByteSize)
{
    int j;
    unsigned int uiByteCount, uiFailCount=0, uiTotalFailCount=0;

#if SAVE_FLASH_CONTENT
	//make save buffer cacheable
	for(j=0; j< SAVE_BUFFER_BYTE_COUNT/16/1024/1024; j++)
		gpCGEM_regs->MAR[(SAVE_BUFFER_ADDRESS/16/1024/1024)+j]=1|
			(1<<CSL_CGEM_MAR0_PFX_SHIFT);	
	
	if(FLASH_MEM_save(uiStartAddress, uiStopAddress))
		return 1;
#endif

    uiByteCount = (uiStopAddress - uiStartAddress);

#if BIT_PATTERN_FILLING_TEST
    // Fill with values from pattern table
    for (j = 0; j < sizeof(uiDataPatternTable)/4; j++)
    {
		if((*FLASH_erase_blocks)(uiStartAddress, uiByteCount))
	    {
	    	PRINT("!!!Failed erase blocks at 0x%x for Memory Fill Test\n",uiStartAddress);
	        return 1;
	    }
		
        uiFailCount = FLASH_MEM_FillTest(uiStartAddress, uiByteCount, 
        	(unsigned int *)uiBufAddress, uiBufByteSize, uiDataPatternTable[j]);
	    if (uiFailCount)
	    {
	    	PRINT("!!!Failed Memory Fill Test at %d Units with pattern 0x%8x\n",uiFailCount, uiDataPatternTable[j]);
	        uiTotalFailCount+=uiFailCount;
	    }
		else
		   	PRINT("Passed Memory Fill Test from 0x%8x to 0x%8x with pattern 0x%8x\n",uiStartAddress,uiStopAddress, uiDataPatternTable[j]);
    }
    
#endif
#if ADDRESS_TEST    
    // Test the address range from low to high
	if((*FLASH_erase_blocks)(uiStartAddress, uiByteCount))
    {
    	PRINT("!!!Failed erase blocks at 0x%x for Memory address Test\n",uiStartAddress);
        return 1;
    }
    uiFailCount = FLASH_MEM_AddrTest(uiStartAddress, uiByteCount, 
    	(unsigned int *)uiBufAddress, uiBufByteSize); 
    if (uiFailCount)
    {
    	PRINT("!!!Failed Memory Address Test at %d units\n",uiFailCount);
        uiTotalFailCount+=uiFailCount;
    }
	else
	   	PRINT("Passed Memory Address Test from 0x%8x to 0x%8x\n",uiStartAddress,uiStopAddress);

#endif
#if BIT_WALKING_TEST
    // Do walking ones and zeroes test
    uiFailCount = FLASH_MEM_Bit_Walking(uiStartAddress,uiBufByteSize,
    	(unsigned int *)uiBufAddress, uiBufByteSize); 
    if (uiFailCount)
    {
	   	PRINT("!!!Failed Memory Bit Walking at %d units\n",uiFailCount);
        uiTotalFailCount+=uiFailCount;
    }    
	else
	  	PRINT("Passed Memory Bit Walking from 0x%8x to 0x%8x\n",uiStartAddress,uiStopAddress);
#endif

#if SAVE_FLASH_CONTENT
	if((*FLASH_erase_blocks)(uiStartAddress, uiByteCount))
    {
    	PRINT("!!!Failed erase blocks at 0x%x for FLASH content restore\n",uiStartAddress);
        return 1;
    }
	FLASH_MEM_restore(uiStartAddress, uiStopAddress);
#endif

  	return uiTotalFailCount;
}

