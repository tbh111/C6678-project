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
#ifndef _I2C_TEMP_SENSOR_H
#define _I2C_TEMP_SENSOR_H

#include <tistdtypes.h>

/*I2C temperature sensor initialization.
set low temperatue thereshold to "iTLow"(¡æ), 
set high temperatue threshold to "iTHigh"(¡æ)*/
void TMP_Sensor_init(Uint32 I2C_address, Int16 iTLow, Int16 iTHigh);

/*Read temperature from the I2C temperatue sensor*/
extern Int16 GetTemperature(Uint32 I2C_address);

#endif

