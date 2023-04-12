/******************************************************************************
 
   Copyright (C), 2013, Texas Instrument.
 
  ******************************************************************************
   File Name	 : I2C_loopback_test.h
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


#ifndef _I2C_LOOPBACK_TEST_H_
#define _I2C_LOOPBACK_TEST_H_

#define I2C_LOOP_TEST_BUF_SIZE 	(4*1024)

extern Uint8 i2cRxBuf[];
extern Uint8 i2cTxBuf[];

extern void I2C_loopback_test();


#endif

