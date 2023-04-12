/*
SPI NOR FLASH test with DSP core
Author: Brighton Feng
Created on 2012-2-20
*/

#include <stdio.h>
#include <string.h>
#include "KeyStone_SPI_init_drv.h"
#include "KeyStone_common.h"
#include "SPI_NOR_FLASH_drv.h"

#define MAX_FILL_FAIL_COUNT 		(10)
#define MAX_ADDRESS_FAIL_COUNT 		(10)

#define BIT_PATTERN_FILLING_TEST 	1
#define ADDRESS_TEST 				1

#define SAVE_FLASH_CONTENT 			1
#define SAVE_BUFFER_ADDRESS 		0x80000000
#define SAVE_BUFFER_BYTE_COUNT 		0x20000000

#define PRINT_DETAILS 			1
#if PRINT_DETAILS
#define 	PRINT	 		printf
#else
#define 	PRINT			// 		
#endif

#define 	SPI_TEST_BUF_SIZE 	256

unsigned int uiDataPatternTable[] = { 0x00000000, /*0xffffffff,*/ 0xaaaaaaaa/*, 0x12345678*/ };

/*
This function fills the patterns into SPI_NOR_FLASH and reads back and verification
*/
unsigned int SPI_NOR_FLASH_FillTest( 
	unsigned int uiStartAddress,
	unsigned int uiStopAddress,
	unsigned int uiAddressWidth, 
	unsigned int uiBitPattern)
{
    unsigned int uiLoopCount, i, j, uiFailCount=0;
    volatile unsigned int uipAddressPointer;
    volatile unsigned int uiReadBack;
	unsigned int spiBuffer[SPI_TEST_BUF_SIZE/4+2], byteSuccess=0;

    uiLoopCount = ((uiStopAddress - uiStartAddress)/SPI_TEST_BUF_SIZE);

	/*fill the temporary buffer*/
	for(i=0; i<SPI_TEST_BUF_SIZE/4; i++)
	{
		spiBuffer[i] = uiBitPattern;
	}

    /* Write the pattern */
	uipAddressPointer = uiStartAddress;
	for(i=0; i<uiLoopCount; i++)
	{
		/* fill with pattern */
		byteSuccess= SPI_NOR_FLASH_write(uipAddressPointer, uiAddressWidth,
			SPI_TEST_BUF_SIZE, (Uint8 *)spiBuffer);
		if(SPI_TEST_BUF_SIZE!=byteSuccess)
		{
			PRINT("SPI NOR FLASH Fill Test fails at 0x%5x, SPI writes %d of %d bytes\n", 
				uipAddressPointer, byteSuccess, SPI_TEST_BUF_SIZE);
			return (SPI_TEST_BUF_SIZE-byteSuccess);
		}
        uipAddressPointer += SPI_TEST_BUF_SIZE;
    }

	for(j=0; j< SPI_TEST_BUF_SIZE/4; j++)
	{
		spiBuffer[j]= 0x5a5a5a5a;
	}
	uipAddressPointer = uiStartAddress;
	for(i=0; i<uiLoopCount; i++)
	{
		/*read back and verify */
		byteSuccess= SPI_NOR_FLASH_read(uipAddressPointer, uiAddressWidth,
			SPI_TEST_BUF_SIZE, (Uint8 *)spiBuffer);
		if(SPI_TEST_BUF_SIZE!=byteSuccess)
		{
			PRINT("SPI NOR FLASH Fill Test fails at 0x%5x, SPI reads %d of %d bytes\n", 
				uipAddressPointer, byteSuccess, SPI_TEST_BUF_SIZE);
			return (SPI_TEST_BUF_SIZE-byteSuccess);
		}

		for(j=0; j< SPI_TEST_BUF_SIZE/4; j++)
		{
	        uiReadBack = spiBuffer[j];
	        if ( uiReadBack!= uiBitPattern)      /* verify pattern */
	        {
				PRINT("Fill Test fails at 0x%5x, Write 0x%8x, Readback 0x%8x\n", uipAddressPointer, uiBitPattern, uiReadBack);
	            uiFailCount++;
				if(uiFailCount>=MAX_FILL_FAIL_COUNT)
					return uiFailCount;
	        }
	        uipAddressPointer += 4;
		}
    }
	return uiFailCount;
}

/*
this function write the address to corresponding SPI_NOR_FLASH unit and readback for verification
*/
unsigned int SPI_NOR_FLASH_AddrTest( 
	unsigned int uiStartAddress, 
	unsigned int uiStopAddress,
	unsigned int uiAddressWidth) 
{
    unsigned int uiLoopCount,i, j, uiFailCount=0;
    volatile unsigned int uipAddressPointer;
    volatile unsigned int uiReadBack;
	unsigned int spiBuffer[SPI_TEST_BUF_SIZE/4], byteSuccess=0;

    uiLoopCount = ((uiStopAddress - uiStartAddress)/SPI_TEST_BUF_SIZE);

	uipAddressPointer = uiStartAddress;
	for(i=0; i<uiLoopCount; i++)
	{
		/* fill with address value*/
		for(j=0; j< SPI_TEST_BUF_SIZE/4; j++)
		{
			spiBuffer[j]= uipAddressPointer;
			uipAddressPointer+=4;
		}
		byteSuccess= SPI_NOR_FLASH_write(uipAddressPointer-SPI_TEST_BUF_SIZE, 
			uiAddressWidth, SPI_TEST_BUF_SIZE, (Uint8 *)spiBuffer);
		if(SPI_TEST_BUF_SIZE!=byteSuccess)
		{
			PRINT("SPI NOR FLASH Address Test fails at 0x%5x, SPI writes %d of %d bytes\n", 
				uipAddressPointer-SPI_TEST_BUF_SIZE, byteSuccess, SPI_TEST_BUF_SIZE);
			return (SPI_TEST_BUF_SIZE-byteSuccess);
		}
    }

	for(j=0; j< SPI_TEST_BUF_SIZE/4; j++)
	{
		spiBuffer[j]= 0x5a5a5a5a;
	}
	uipAddressPointer = uiStartAddress;
	for(i=0; i<uiLoopCount; i++)
	{
		/*read back and verify */
		byteSuccess= SPI_NOR_FLASH_read(uipAddressPointer, uiAddressWidth, 
			SPI_TEST_BUF_SIZE, (Uint8 *)spiBuffer);
		if(SPI_TEST_BUF_SIZE!=byteSuccess)
		{
			PRINT("SPI NOR FLASH Address Test fails at 0x%5x, SPI reads %d of %d bytes\n", 
				uipAddressPointer, byteSuccess, SPI_TEST_BUF_SIZE);
			return (SPI_TEST_BUF_SIZE-byteSuccess);
		}

		for(j=0; j< SPI_TEST_BUF_SIZE/4; j++)
		{
	        uiReadBack = spiBuffer[j];
	        if ( uiReadBack != uipAddressPointer ) /* verify data */
	        {
				PRINT("Address Test fails at 0x%5x, Write 0x%8x, Readback 0x%8x\n", uipAddressPointer, uipAddressPointer, uiReadBack);
	            uiFailCount++;
				if(uiFailCount>=MAX_ADDRESS_FAIL_COUNT)
					return uiFailCount;
	        }
			uipAddressPointer+=4;
		}
    }
    return uiFailCount;              /* show no error */
}

//save flash content before test
int SPI_FLASH_save(unsigned int uiStartAddress, 
	unsigned int uiStopAddress,
	unsigned int uiAddressWidth)
{
	unsigned int uiByteCount, byteSuccess=0;

    uiByteCount = (uiStopAddress - uiStartAddress);
    if(SAVE_BUFFER_BYTE_COUNT<uiByteCount)
    {
    	printf("Save buffer size %d < tested flash size %d!\n", 
    		SAVE_BUFFER_BYTE_COUNT, uiByteCount);
    	return 1;
    }

	byteSuccess= SPI_NOR_FLASH_read(uiStartAddress, 
		uiAddressWidth, uiByteCount, (Uint8 *)SAVE_BUFFER_ADDRESS);
	if(uiByteCount!=byteSuccess)
	{
		PRINT("SPI NOR FLASH save fails at 0x%5x, saved %d of %d bytes\n", uiStartAddress, byteSuccess, uiByteCount);
		return 1;
	}
	PRINT("Save data from 0x%6x to 0x%6x.\n", uiStartAddress, uiStopAddress);
	
	return 0;
}

//restore flash content after test
int SPI_FLASH_restore(unsigned int uiStartAddress, 
	unsigned int uiStopAddress,
	unsigned int uiAddressWidth)
{
	unsigned int uiByteCount, byteSuccess=0;

    uiByteCount = (uiStopAddress - uiStartAddress);
    if(SAVE_BUFFER_BYTE_COUNT<uiByteCount)
    {
    	printf("Save buffer size %d < tested flash size %d!\n", 
    		SAVE_BUFFER_BYTE_COUNT, uiByteCount);
    	return 1;
    }

	byteSuccess= SPI_NOR_FLASH_write(uiStartAddress, 
		uiAddressWidth, uiByteCount, (Uint8 *)SAVE_BUFFER_ADDRESS);
	if(uiByteCount!=byteSuccess)
	{
		PRINT("SPI NOR FLASH restore fails at 0x%5x, restored %d of %d bytes\n", uiStartAddress, byteSuccess, uiByteCount);
		return 1;
	}

	PRINT("Restore data from 0x%6x to 0x%6x.\n", uiStartAddress, uiStopAddress);

	return 0;
}

/*test SPI NOR FLASH, return error number*/
int SPI_NOR_FLASH_Test(
	unsigned int start_address, 
	unsigned int stop_address,
	unsigned int uiAddressWidth)
{
    unsigned int uiFailCount=0, uiTotalFailCount=0;
    int j;

#if SAVE_FLASH_CONTENT
	//make save buffer cacheable
	for(j=0; j< SAVE_BUFFER_BYTE_COUNT/16/1024/1024; j++)
		gpCGEM_regs->MAR[(SAVE_BUFFER_ADDRESS/16/1024/1024)+j]=1|
			(1<<CSL_CGEM_MAR0_PFX_SHIFT);	
	
	if(SPI_FLASH_save(start_address, stop_address, uiAddressWidth))
		return 1;
#endif

#if BIT_PATTERN_FILLING_TEST
    // Fill with values from pattern table
    for (j = 0; j < sizeof(uiDataPatternTable)/4; j++)
    {
    	SPI_NOR_FLASH_erase(start_address, 
    		stop_address- start_address, uiAddressWidth);
        uiFailCount = SPI_NOR_FLASH_FillTest(start_address, stop_address, 
        	uiAddressWidth, uiDataPatternTable[j]);
	    if (uiFailCount)
	    {
	    	PRINT("!!!Failed SPI_NOR_FLASH Fill Test at %d Units with pattern 0x%8x\n",uiFailCount, uiDataPatternTable[j]);
	        uiTotalFailCount+=uiFailCount;
	    }
		else
		   	PRINT("Passed SPI_NOR_FLASH Fill Test from 0x%5x to 0x%5x with pattern 0x%8x\n",
		   			start_address, stop_address, uiDataPatternTable[j]);
    }
    
#endif
#if ADDRESS_TEST    
    // Test the address range
	SPI_NOR_FLASH_erase(start_address, 
		stop_address- start_address, uiAddressWidth);
    uiFailCount = SPI_NOR_FLASH_AddrTest(start_address, 
    	stop_address, uiAddressWidth);
    if (uiFailCount)
    {
    	PRINT("!!!Failed SPI_NOR_FLASH Address Test at %d units\n",uiFailCount);
        uiTotalFailCount+=uiFailCount;
    }
	else
	   	PRINT("Passed SPI_NOR_FLASH Address Test from 0x%5x to 0x%5x\n",
	   			start_address, stop_address);
#endif

#if SAVE_FLASH_CONTENT
	SPI_NOR_FLASH_erase(start_address,
		stop_address- start_address, uiAddressWidth);

	SPI_FLASH_restore(start_address, stop_address, uiAddressWidth);
#endif

  	return uiTotalFailCount;
}


