/****************************************************************************\
 *           Copyright (C) 2012 Texas Instruments Incorporated.             *
 *                           All Rights Reserved                            *
 *                                                                          *
 * GENERAL DISCLAIMER                                                       *
 * ------------------                                                       *
 * All software and related documentation is provided "AS IS" and without   *
 * warranty or support of any kind and Texas Instruments expressly disclaims*
 * all other warranties, express or implied, including, but not limited to, *
 * the implied warranties of merchantability and fitness for a particular   *
 * purpose.  Under no circumstances shall Texas Instruments be liable for   *
 * any incidental, special or consequential damages that result from the    *
 * use or inability to use the software or related documentation, even if   *
 * Texas Instruments has been advised of the liability.                     *
 ****************************************************************************
This example shows I2C master mode operations:
1. read/write I2C EEPROM
2. read I2C temperature sensor
 ****************************************************************************
 * Created by :                                                             *
 *            Brighton Feng                                                 *
 *            Texas Instruments                                             * 
 *            Jan 5, 2012                                                 *
***************************************************************************/
#include <c6x.h>
#include <stdio.h>
#include <csl_bootcfgAux.h>
#include "KeyStone_common.h"
#include "Keystone_DDR_Init.h"
#include "KeyStone_I2C_init_drv.h"
#include "I2C_EEPROM_Test.h"
#include "I2C_loopback_test.h"

#define I2C_LOOPBACK_TEST 		1
#define I2C_EEPROM_TEST 		1
#define I2C_TEMP_SENSOR_TEST 	0

#if I2C_TEMP_SENSOR_TEST
#define I2C_TEMP_SENSOR_ADDRESS 0x48
#endif

#if I2C_EEPROM_TEST
#define I2C_EEPROM_SIZE_KB 	64
Uint32 I2C_EEPROM_address= 0x50;
#endif


void main()
{
	int i;
	
	/*enable TSC, memory protection interrupts, EDC for internal RAM;
    clear cache; protect L1 as cache*/
	KeyStone_common_CPU_init();
	/*print device information. 
	Enable memory protection interrupts, EDC for MSMC RAM*/
	KeyStone_common_device_init();

	//enable exception handling
	KeyStone_Exception_cfg(TRUE);

    CACHE_setL1PSize(CACHE_L1_32KCACHE);
    CACHE_setL1DSize(CACHE_L1_32KCACHE);
    CACHE_setL2Size(CACHE_128KCACHE);

	if(C6678_EVM==gDSP_board_type)
	{
		//DSP core speed: 100*10/1=1000MHz
		KeyStone_main_PLL_init(100, 10, 1); 
#if I2C_EEPROM_TEST
		I2C_EEPROM_address= 0x51;	//second half of EEPROM on Nyquist/Appleton EVM
#endif
	}
	else if((C6670_EVM==gDSP_board_type)
		||(TCI6614_EVM==gDSP_board_type))
	{
		//DSP core speed: 122.88*236/29= 999.9889655MHz
		KeyStone_main_PLL_init(122.88, 236, 29);
#if I2C_EEPROM_TEST
		I2C_EEPROM_address= 0x51;	//second half of EEPROM on Shannon EVM
#endif
	}
	else if(DUAL_NYQUIST_EVM==gDSP_board_type)
	{
		//DSP core speed: 122.88*236/29= 999.9889655MHz
		KeyStone_main_PLL_init(122.88, 236, 29);
#if I2C_EEPROM_TEST
		I2C_EEPROM_address= 0x57;	//user EEPROM on Nyquist EVM
#endif
	}
	else
	{
		puts("Unknown DSP board type!");
		return;
	}

	//DDR init 66.66667*20/1= 1333
	KeyStone_DDR_init (66.66667, 20, 1, NULL);

#if I2C_LOOPBACK_TEST
	I2C_loopback_test();
#endif

	I2C_Master_Init(400);

#if I2C_EEPROM_TEST
	puts("I2C EEPROM test start...");
	for(i=I2C_EEPROM_SIZE_KB/2; i<I2C_EEPROM_SIZE_KB; i+= 2)
		I2C_EEPROM_Test(I2C_EEPROM_address, i*1024, (i+2)*1024);
#endif
	
#if I2C_TEMP_SENSOR_TEST
	TMP_Sensor_init(I2C_TEMP_SENSOR_ADDRESS, 75, 80);
	printf("temperature is %d¡æ",GetTemperature(I2C_TEMP_SENSOR_ADDRESS));
#endif
	
	puts("I2C test complete.");
}


