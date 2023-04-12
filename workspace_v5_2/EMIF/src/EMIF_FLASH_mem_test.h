/*
FLASH memory test
Author: Brighton Feng
Created on 2012-1-17
*/

#ifndef _FLASH_MEM_TEST_H
#define _FLASH_MEM_TEST_H
 
//call back function for FLASH erase
extern int (* FLASH_erase_blocks)(unsigned int flashAddress, 
	unsigned int byteNumber);

//call back function for FLASH write
extern int (* FLASH_write_block)(unsigned int srcDatAddress, 
	unsigned int dstFlashAddress, unsigned int byteNumber);

//call back function for FLASH read
extern int (* FLASH_read_block)(unsigned int dstDatAddress, 
	unsigned int srcFlashAddress, unsigned int byteNumber);

int FLASH_MEM_Test(unsigned int uiStartAddress, 
	unsigned int uiStopAddress,
	unsigned int uiBufAddress,
	unsigned int uiBufByteSize);

 #endif

