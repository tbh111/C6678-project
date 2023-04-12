/*
SPI NOR FLASH test with DSP core
Author: Brighton Feng
Created on 2012-2-20
*/

#ifndef _SPI_NOR_FLASH_TEST_H
#define _SPI_NOR_FLASH_TEST_H

/*test SPI NOR FLASH, return error number*/
int SPI_NOR_FLASH_Test(
	Uint32 start_address, 
	Uint32 stop_address,
	unsigned int uiAddressWidth);

#endif

