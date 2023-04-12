/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2012
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
 example for I2C temperature sensor driver on KeyStone DSP. 
 Tested on TMP100, TMP101, ADT75.
 * =============================================================================
 *  Revision History
 *  ===============
 *  Jan 18, 2012 Brighton Feng  file created
 * =============================================================================*/

#include <stdio.h>
#include "KeyStone_common.h"
#include "KeyStone_I2C_init_drv.h"


/*I2C temperature sensor initialization.
set low temperatue thereshold to "iTLow"(¡æ), 
set high temperatue threshold to "iTHigh"(¡æ)*/
void TMP_Sensor_init(Uint32 I2C_address, Int16 iTLow, Int16 iTHigh)
{
	Uint8 ucBuffer[4];

	/*if the TMP100 and TMP101 are powered down by removing supply voltage from 
	the device, but the supply voltage is not assured to be less than 0.3V, 
	it is recommended to issue a General Call reset command (write 00000110 
	to I2C address 0) on the I2C interface 	bus to ensure that the TMP100 and 
	TMP101 are completely reset.*/
	ucBuffer[0]= 6;
	ucBuffer[1]= 0;
	ucBuffer[2]= 0;
	I2C_write(0, 1, ucBuffer, I2C_WAIT);
	TSC_delay_ms(1);
	
	/*first byte = 1, select the configuration register*/
	ucBuffer[0]= 1;
	/*second byte is configuration register value: interrupt mode, high pulse*/
	ucBuffer[1]= 0x6;
	I2C_write(I2C_address, 2, ucBuffer, I2C_WAIT);
	
	/*select the low threshold register*/
	ucBuffer[0]= 2;
	ucBuffer[1]= iTLow;
	I2C_write(I2C_address, 3, ucBuffer, I2C_WAIT);
	
	/*select the high threshold register*/
	ucBuffer[0]= 3;
	ucBuffer[1]= iTHigh;
	I2C_write(I2C_address, 3, ucBuffer, I2C_WAIT);

	/*before read temperature register,
	execute a dummy write to set the pointer to 0 (temperature register)*/
	ucBuffer[0]= 0;
	I2C_write(I2C_address, 1, ucBuffer, I2C_WAIT);

	
}


/*Read temperature from the I2C temperatue sensor*/
Int16 GetTemperature(Uint32 I2C_address)
{
	Uint8 ucBuffer[4];

	/*wait for about 100ms for temperture conversion complete*/
	TSC_delay_ms(200);

	if(2!=I2C_read(I2C_address, 2, ucBuffer, I2C_WAIT))
		return -9999;

	return ((Int16)ucBuffer[0]);
}

