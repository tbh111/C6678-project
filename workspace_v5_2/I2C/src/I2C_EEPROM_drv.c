/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2012
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
 example for I2C EEPROM driver on KeyStone DSP. 
 Tested on AT24C512B, AT24C1024B and M24M01.
 * =============================================================================
 *  Revision History
 *  ===============
 *  Jan 9, 2012 Brighton Feng  file created
 * =============================================================================*/

#include <stdio.h>
#include "KeyStone_common.h"
#include "KeyStone_I2C_init_drv.h"
#include "I2C_EEPROM_drv.h"

/*read "uiByteCount" data to "ucBuffer" from "data_address" of 
I2C EEPROM with address "I2C_address".
return number of bytes transfered*/
Uint32 I2C_EEPROM_read(Uint32 I2C_address, Uint32 data_address, 
	Uint32 uiByteCount, Uint8 * ucBuffer)
{
	Uint8 ucAddrBuffer[4];
	Uint32 byteSuccess=0;
	
 	/*put data address into first two bytes of ucBuffer,
 	and write it to I2C EEPROM*/
	ucAddrBuffer[0]= data_address>>8;
	ucAddrBuffer[1]= data_address;

	/*Dummy write, setup read address*/
	byteSuccess= I2C_write(I2C_address, 2, ucAddrBuffer, I2C_WAIT);
	if(2!=byteSuccess)
		return 0;

	//wait about 5ms for I2C EEPROM internal write complete
	TSC_delay_ms(6);

	/*read data*/
	byteSuccess= I2C_read(I2C_address, uiByteCount, ucBuffer, I2C_WAIT);

	return byteSuccess;
}

/*write "uiByteCount" data in "ucBuffer" to "data_address" of I2C EEPROM
with address "I2C_address".
Please note, the data must be in I2C_EEPROM_BLOCK_SIZE, that is,
((data_address&0xff)+uiByteCount)<=I2C_EEPROM_BLOCK_SIZE
return number of bytes transfered.*/
Uint32 I2C_EEPROM_block_write(Uint32 I2C_address, Uint32 data_address, 
	Uint32 uiByteCount, Uint8 * ucBuffer)
{
	Uint8 i2cBuffer[I2C_EEPROM_BLOCK_SIZE+4];

 	/*put data address into first two bytes of i2cBuffer,
 	and write it to I2C EEPROM*/
	i2cBuffer[0]= data_address>>8;
	i2cBuffer[1]= data_address;
	memcpy(i2cBuffer+2, ucBuffer, uiByteCount);

	uiByteCount= I2C_write(I2C_address, uiByteCount+2, i2cBuffer, I2C_WAIT);
	if(uiByteCount>2)
	{
		uiByteCount-= 2;
		//wait about 5ms for I2C EEPROM internal write complete
		TSC_delay_ms(6);
	}
	else
		uiByteCount=0;

	return uiByteCount;
}

/*write "uiByteCount" data in "ucBuffer" to address "data_address" 
in the I2C EEPROM with "I2C_address".*/
Uint32 I2C_EEPROM_write(Uint32 I2C_address, Uint32 data_address, 
	Uint32 uiByteCount, Uint8 * ucBuffer)
{
	Uint32 blockByteCount, blockAddrOffset;
	Uint32 byteSuccess, tempCount= uiByteCount;

	/*Deassert the I2C_EEPROM write protection through GPIO pin*/

	/*I2C EEPROM write must be in I2C_EEPROM_BLOCK_SIZE, write more than 
	I2C_EEPROM_BLOCK_SIZE a time will wrap around and overwrite privious data*/

	/*checke the address offset in a I2C_EEPROM_BLOCK_SIZE*/
	blockAddrOffset= data_address&(I2C_EEPROM_BLOCK_SIZE-1);

	/*handle non-aligned data firstly*/
	if(blockAddrOffset)
	{
		blockByteCount= I2C_EEPROM_BLOCK_SIZE- blockAddrOffset;
		if(blockByteCount>uiByteCount)
			blockByteCount= uiByteCount;

		byteSuccess= I2C_EEPROM_block_write(I2C_address, data_address, 
			blockByteCount, ucBuffer);
		if(byteSuccess!= blockByteCount)
			return byteSuccess;

		data_address+= blockByteCount;
		ucBuffer+= blockByteCount;
		tempCount-= blockByteCount;
	}

	/*write the remaining data block by block*/
	while(tempCount)
	{
		if(tempCount>=I2C_EEPROM_BLOCK_SIZE)
			blockByteCount= I2C_EEPROM_BLOCK_SIZE;
		else
			blockByteCount= tempCount;

		byteSuccess= I2C_EEPROM_block_write(I2C_address, data_address, 
			blockByteCount, ucBuffer);
		if(byteSuccess!= blockByteCount)
			return byteSuccess;

		data_address+= blockByteCount;
		ucBuffer+= blockByteCount;
		tempCount-= blockByteCount;
	}

	/*Assert the I2C_EEPROM write protection through GPIO pin*/

	return uiByteCount;
}

