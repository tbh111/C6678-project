/*
I2C EEPROM test with DSP core
Author: Brighton Feng
Created on 2012-1-11
*/

#ifndef _I2C_EEPROM_TEST_H
#define _I2C_EEPROM_TEST_H

/*test I2C EEPROM, return error number*/
int I2C_EEPROM_Test(Uint32 I2C_address, Uint32 start_address, 
	Uint32 stop_address);

#endif

