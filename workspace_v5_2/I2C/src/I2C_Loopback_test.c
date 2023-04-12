/******************************************************************************
 
   Copyright (C), 2013, Texas Instrument.
 
  ******************************************************************************
   File Name	 : I2C_loopback_test.c
   Version		 : Initial Draft
   Author		 : Brighton Feng
   Created		 : August 14, 2013
   Last Modified : 
   Description	 : I2C loopback test on KeyStone
   History		 :
   1.Date		 : August 14, 2013
   Author 	 	 : Brighton Feng
   Modification: Created file
 
 ******************************************************************************/
#include <c6x.h>
#include "KeyStone_common.h"
#include "KeyStone_I2C_init_drv.h"
#include "I2C_loopback_test.h"

Uint8 i2cRxBuf[I2C_LOOP_TEST_BUF_SIZE];
Uint8 i2cTxBuf[I2C_LOOP_TEST_BUF_SIZE];

int I2C_loopback_test_one_time(Uint8 dataPattern)
{
	int i, iByteSuccess;
	Uint32 startTSC, cycles, throughput;
	
	for(i=0; i<I2C_LOOP_TEST_BUF_SIZE; i++)
	{
		i2cTxBuf[i]= dataPattern;
		i2cRxBuf[i]= ~dataPattern;
	}

	startTSC= TSCL;
	iByteSuccess= I2C_loopback(0x55, I2C_LOOP_TEST_BUF_SIZE, 
		i2cTxBuf, i2cRxBuf);
	cycles= TSC_count_cycle_from(startTSC);

	if(iByteSuccess!=I2C_LOOP_TEST_BUF_SIZE)
	{
		printf("I2C loopback test failed. TX %d bytes, RX %d bytes!\n",
			I2C_LOOP_TEST_BUF_SIZE, iByteSuccess);
		return iByteSuccess;
	}
		
	for(i=0; i<I2C_LOOP_TEST_BUF_SIZE; i++)
	{
		if(i2cTxBuf[i]!=i2cRxBuf[i])
		{
			printf("I2C loopback test failed at word %d: TX 0x%x, RX 0x%x\n",
				i, i2cTxBuf[i], i2cRxBuf[i]);
			return i;
		}
	}

    throughput = (unsigned long long)I2C_LOOP_TEST_BUF_SIZE*8*gDSP_Core_Speed_Hz/
    	((unsigned long long)cycles*1000);

	printf("I2C loopback test passed with data pattern 0x%x. Throughput= %dKbps\n",
		dataPattern, throughput);
		
	return I2C_LOOP_TEST_BUF_SIZE;
}


void I2C_loopback_test()
{
	I2C_Master_Init(400);

	I2C_loopback_test_one_time(0);
	I2C_loopback_test_one_time(0xFF);
	I2C_loopback_test_one_time(0x55);
}

