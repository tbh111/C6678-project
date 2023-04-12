/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2012
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
 example for I2C EEPROM driver on KeyStone DSP. 
 Tested on AT24C512B, AT24C1024B.
 * =============================================================================
 *  Revision History
 *  ===============
 *  Jan 9, 2012 Brighton Feng  file created
 * =============================================================================*/
#ifndef _I2C_EEPROM_DRV_H
#define _I2C_EEPROM_DRV_H

#include <tistdtypes.h>

#define I2C_EEPROM_BLOCK_SIZE 	128


/*read "uiByteCount" data to "ucBuffer" from "data_address" of 
I2C EEPROM with address "I2C_address".
return number of bytes transfered*/
extern Uint32 I2C_EEPROM_read(Uint32 I2C_address, Uint32 uiAddress, 
	Uint32 uiByteCount, Uint8 * ucBuffer);

/*write "uiByteCount" data in "ucBuffer" to address "data_address" 
in the I2C EEPROM with "I2C_address".*/
extern Uint32 I2C_EEPROM_write(Uint32 I2C_address, Uint32 uiAddress, 
	Uint32 uiByteCount, Uint8 * ucBuffer);

#endif

