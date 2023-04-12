/*
I2C EEPROM test with DSP core
Author: Brighton Feng
Created on 2012-1-11
*/

#include <stdio.h>
#include <string.h>
#include "KeyStone_I2C_init_drv.h"
#include "I2C_EEPROM_drv.h"
#include "KeyStone_common.h"

#define MAX_FILL_FAIL_COUNT 		(10)
#define MAX_ADDRESS_FAIL_COUNT 		(10)

#define BIT_PATTERN_FILLING_TEST 	1
#define ADDRESS_TEST 				1

#define SAVE_EEPROM_CONTENT 			1
#define SAVE_BUFFER_ADDRESS 		0x80000000
#define SAVE_BUFFER_BYTE_COUNT 		0x01000000

#define PRINT_DETAILS 			1
#if PRINT_DETAILS
#define 	PRINT	 		printf
#else
#define 	PRINT			// 		
#endif

#define 	I2C_TEST_BUF_SIZE 	256

unsigned int uiDataPatternTable[] = { 0x00000000, 0xffffffff, 0xaaaaaaaa, 0x55555555 };

/*
This function fills the patterns into I2C_EEPROM and reads back and verification
*/
unsigned int I2C_EEPROM_FillTest(unsigned int I2C_address, 
	unsigned int uiStartAddress,
	unsigned int uiStopAddress,
	unsigned int uiBitPattern)
{
    unsigned int uiLoopCount, i, j, uiFailCount=0;
    volatile unsigned int uipAddressPointer;
    volatile unsigned int uiReadBack;
	unsigned int i2cBuffer[I2C_TEST_BUF_SIZE/4+2], byteSuccess=0;

    uiLoopCount = ((uiStopAddress - uiStartAddress)/I2C_TEST_BUF_SIZE);

	/*fill the temporary buffer*/
	for(i=0; i<I2C_TEST_BUF_SIZE/4; i++)
	{
		i2cBuffer[i] = uiBitPattern;
	}

    /* Write the pattern */
	uipAddressPointer = uiStartAddress;
	for(i=0; i<uiLoopCount; i++)
	{
		/* fill with pattern */
		byteSuccess= I2C_EEPROM_write(I2C_address, uipAddressPointer, I2C_TEST_BUF_SIZE, (Uint8 *)i2cBuffer);
		if(I2C_TEST_BUF_SIZE!=byteSuccess)
		{
			PRINT("I2C EEPROM Fill Test fails at 0x%5x, I2C writes %d of %d bytes\n", 
				uipAddressPointer, byteSuccess, I2C_TEST_BUF_SIZE);
			return (I2C_TEST_BUF_SIZE-byteSuccess);
		}
        uipAddressPointer += I2C_TEST_BUF_SIZE;
    }

	for(j=0; j< I2C_TEST_BUF_SIZE/4; j++)
	{
		i2cBuffer[j]= 0x5a5a5a5a;
	}
	uipAddressPointer = uiStartAddress;
	for(i=0; i<uiLoopCount; i++)
	{
		/*read back and verify */
		byteSuccess= I2C_EEPROM_read(I2C_address, uipAddressPointer, I2C_TEST_BUF_SIZE, (Uint8 *)i2cBuffer);
		if(I2C_TEST_BUF_SIZE!=byteSuccess)
		{
			PRINT("I2C EEPROM Fill Test fails at 0x%5x, I2C reads %d of %d bytes\n", 
				uipAddressPointer, byteSuccess, I2C_TEST_BUF_SIZE);
			return (I2C_TEST_BUF_SIZE-byteSuccess);
		}

		for(j=0; j< I2C_TEST_BUF_SIZE/4; j++)
		{
	        uiReadBack = i2cBuffer[j];
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
this function write the address to corresponding I2C_EEPROM unit and readback for verification
*/
unsigned int I2C_EEPROM_AddrTest(unsigned int I2C_address, 
	unsigned int uiStartAddress, 
	unsigned int uiStopAddress)
{
    unsigned int uiLoopCount,i, j, uiFailCount=0;
    volatile unsigned int uipAddressPointer;
    volatile unsigned int uiReadBack;
	unsigned int i2cBuffer[I2C_TEST_BUF_SIZE/4], byteSuccess=0;

    uiLoopCount = ((uiStopAddress - uiStartAddress)/I2C_TEST_BUF_SIZE);

	uipAddressPointer = uiStartAddress;
	for(i=0; i<uiLoopCount; i++)
	{
		/* fill with address value*/
		for(j=0; j< I2C_TEST_BUF_SIZE/4; j++)
		{
			i2cBuffer[j]= uipAddressPointer;
			uipAddressPointer+=4;
		}
		byteSuccess= I2C_EEPROM_write(I2C_address, uipAddressPointer-I2C_TEST_BUF_SIZE, I2C_TEST_BUF_SIZE, (Uint8 *)i2cBuffer);
		if(I2C_TEST_BUF_SIZE!=byteSuccess)
		{
			PRINT("I2C EEPROM Address Test fails at 0x%5x, I2C writes %d of %d bytes\n", 
				uipAddressPointer-I2C_TEST_BUF_SIZE, byteSuccess, I2C_TEST_BUF_SIZE);
			return (I2C_TEST_BUF_SIZE-byteSuccess);
		}
    }

	for(j=0; j< I2C_TEST_BUF_SIZE/4; j++)
	{
		i2cBuffer[j]= 0x5a5a5a5a;
	}
	uipAddressPointer = uiStartAddress;
	for(i=0; i<uiLoopCount; i++)
	{
		/*read back and verify */
		byteSuccess= I2C_EEPROM_read(I2C_address, uipAddressPointer, I2C_TEST_BUF_SIZE, (Uint8 *)i2cBuffer);
		if(I2C_TEST_BUF_SIZE!=byteSuccess)
		{
			PRINT("I2C EEPROM Address Test fails at 0x%5x, I2C reads %d of %d bytes\n", 
				uipAddressPointer, byteSuccess, I2C_TEST_BUF_SIZE);
			return (I2C_TEST_BUF_SIZE-byteSuccess);
		}

		for(j=0; j< I2C_TEST_BUF_SIZE/4; j++)
		{
	        uiReadBack = i2cBuffer[j];
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
int SPI_EEPROM_save(unsigned int I2C_address, unsigned int uiStartAddress, 
	unsigned int uiStopAddress)
{
	unsigned int uiByteCount, byteSuccess=0;

    uiByteCount = (uiStopAddress - uiStartAddress);
    if(SAVE_BUFFER_BYTE_COUNT<uiByteCount)
    {
    	printf("Save buffer size %d < tested flash size %d!\n", 
    		SAVE_BUFFER_BYTE_COUNT, uiByteCount);
    	return 1;
    }

	byteSuccess= I2C_EEPROM_read(I2C_address, uiStartAddress, 
		uiByteCount, (Uint8 *)SAVE_BUFFER_ADDRESS);
	if(uiByteCount!=byteSuccess)
	{
		PRINT("I2C EEPROM save fails at 0x%5x, saved %d of %d bytes\n", uiStartAddress, byteSuccess, uiByteCount);
		return 1;
	}
	PRINT("Save data from 0x%05x to 0x%05x.\n", uiStartAddress, uiStopAddress);
	
	return 0;
}

//restore flash content after test
int SPI_EEPROM_restore(unsigned int I2C_address, unsigned int uiStartAddress, 
	unsigned int uiStopAddress)
{
	unsigned int uiByteCount, byteSuccess=0;

    uiByteCount = (uiStopAddress - uiStartAddress);
    if(SAVE_BUFFER_BYTE_COUNT<uiByteCount)
    {
    	printf("Save buffer size %d < tested flash size %d!\n", 
    		SAVE_BUFFER_BYTE_COUNT, uiByteCount);
    	return 1;
    }

	byteSuccess= I2C_EEPROM_write(I2C_address, uiStartAddress, 
		uiByteCount, (Uint8 *)SAVE_BUFFER_ADDRESS);
	if(uiByteCount!=byteSuccess)
	{
		PRINT("I2C EEPROM restore fails at 0x%5x, restored %d of %d bytes\n", uiStartAddress, byteSuccess, uiByteCount);
		return 1;
	}

	PRINT("Restore data from 0x%05x to 0x%05x.\n", uiStartAddress, uiStopAddress);

	return 0;
}

/*test I2C EEPROM, return error number*/
int I2C_EEPROM_Test(unsigned int I2C_address, 
	unsigned int start_address, 
	unsigned int stop_address)
{
    unsigned int uiFailCount=0, uiTotalFailCount=0;

#if SAVE_EEPROM_CONTENT
	//make save buffer cacheable
	gpCGEM_regs->MAR[(SAVE_BUFFER_ADDRESS/16/1024/1024)]=1|
			(1<<CSL_CGEM_MAR0_PFX_SHIFT);	
	
	if(SPI_EEPROM_save(I2C_address, start_address, stop_address))
		return 1;
#endif

#if BIT_PATTERN_FILLING_TEST
    int j;
    // Fill with values from pattern table
    for (j = 0; j < sizeof(uiDataPatternTable)/4; j++)
    {
        uiFailCount = I2C_EEPROM_FillTest(I2C_address, start_address, stop_address, uiDataPatternTable[j]);
	    if (uiFailCount)
	    {
	    	PRINT("!!!Failed I2C_EEPROM Fill Test at %d Units with pattern 0x%8x\n",uiFailCount, uiDataPatternTable[j]);
	        uiTotalFailCount+=uiFailCount;
	    }
		else
		   	PRINT("Passed I2C_EEPROM Fill Test from 0x%5x to 0x%5x with pattern 0x%8x\n",
		   			start_address, stop_address, uiDataPatternTable[j]);
    }
    
#endif
#if ADDRESS_TEST    
    // Test the address range
    uiFailCount = I2C_EEPROM_AddrTest(I2C_address, start_address, stop_address);
    if (uiFailCount)
    {
    	PRINT("!!!Failed I2C_EEPROM Address Test at %d units\n",uiFailCount);
        uiTotalFailCount+=uiFailCount;
    }
	else
	   	PRINT("Passed I2C_EEPROM Address Test from 0x%5x to 0x%5x\n",
	   			start_address, stop_address);
#endif

#if SAVE_EEPROM_CONTENT
	SPI_EEPROM_restore(I2C_address, start_address, stop_address);
#endif

  	return uiTotalFailCount;
}


