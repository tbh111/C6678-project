/******************************************************************************
 
   Copyright (C), 2013, Texas Instrument.
 
  ******************************************************************************
   File Name	 : SPI_Loopback_TEST.h
   Version		 : Initial Draft
   Author		 : Cheng Peng
   Created		 : 2013-3-5
   Last Modified : 
   Description	 : KeyStone SPI demo code
   History		 :
   1.Date		 : 2013-3-5
   Author 	 	 : Cheng Peng
   Modification: Created file
 
 ******************************************************************************/


#ifndef _KEYSTONEII_SPI_TEST_H_
#define _KEYSTONEII_SPI_TEST_H_

#include <c6x.h>
#include <stdio.h>
#include <string.h>
#include <csl_bootcfgAux.h>
#include "KeyStone_common.h"
#include "KeyStone_SPI_init_drv.h"
#include "SPI_NOR_FLASH_Test.h"
#include "SPI_NOR_FLASH_drv.h"


#define SPI_LOOP_TEST_BUF_SIZE 	(32*1024)


extern Uint16 spiRxBuf[SPI_LOOP_TEST_BUF_SIZE/2];
extern Uint16 spiTxBuf[SPI_LOOP_TEST_BUF_SIZE/2];
extern SPI_Data_Format loopbackDataFormat;
extern SPI_Transfer_Param loopbackTransferParam;

extern int SPI_loopback_test_one_time(Uint16 dataPattern);
extern void SPI_loopback_test();


#endif

