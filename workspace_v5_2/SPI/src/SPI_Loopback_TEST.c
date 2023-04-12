/******************************************************************************
 
   Copyright (C), 2013, Texas Instrument.
 
  ******************************************************************************
   File Name	 : SPI_Loopback_TEST.c
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
#include <c6x.h>
#include "SPI_loopback_TEST.h"

Uint16 spiRxBuf[SPI_LOOP_TEST_BUF_SIZE/2];
Uint16 spiTxBuf[SPI_LOOP_TEST_BUF_SIZE/2];

/*data format for loopback test*/
SPI_Data_Format loopbackDataFormat =
{
	/*.delayBetweenTrans_ns = */0,
	/*.ShifDirection        = */SPI_MSB_SHIFT_FIRST,
	/*.disable_CS_timing    = */1,
	/*.clockPolarity        = */SPI_CLOCK_LOW_INACTIVE,
	/*.clockPhase           = */0,
	/*.clockSpeedKHz        = */66000,
	/*.wordLength           = */16
};

SPI_Transfer_Param loopbackTransferParam =
{
	0,     /*Chip select number*/
	1,  /*select one of the 4 SPI formats*/
	SPI_CS_NO_LAST_HOLD, /*hold CS between multiple words*/
	FALSE,  /*Enable the delay counter at the end of the current transaction*/
	2   /*number of bytes per SPI word*/
};
int SPI_loopback_test_one_time(Uint16 dataPattern)
{
	int i, iByteSuccess;
	Uint32 startTSC, cycles, throughput;
	
	for(i=0; i<SPI_LOOP_TEST_BUF_SIZE/2; i++)
	{
		spiTxBuf[i]= dataPattern;
		spiRxBuf[i]= ~dataPattern;
	}

	startTSC= TSCL;
	iByteSuccess= KeyStone_SPI_TxRx((Uint8 *) spiTxBuf, 0, SPI_LOOP_TEST_BUF_SIZE, 
		(Uint8 *) spiRxBuf, 0, SPI_LOOP_TEST_BUF_SIZE, &loopbackTransferParam);
	cycles= TSC_count_cycle_from(startTSC);

	if(iByteSuccess!=SPI_LOOP_TEST_BUF_SIZE)
	{
		printf("SPI loopback test failed. TX %d bytes, RX %d bytes!\n",
			SPI_LOOP_TEST_BUF_SIZE, iByteSuccess);
		return iByteSuccess;
	}
		
	for(i=0; i<SPI_LOOP_TEST_BUF_SIZE/2; i++)
	{
		if(spiTxBuf[i]!=spiRxBuf[i])
		{
			printf("SPI loopback test failed at word %d: TX 0x%x, RX 0x%x\n",
				i, spiTxBuf[i], spiRxBuf[i]);
			return i;
		}
	}

    throughput = (unsigned long long)SPI_LOOP_TEST_BUF_SIZE*8*gDSP_Core_Speed_Hz/
    	((unsigned long long)cycles*1000000);

	printf("SPI loopback test passed with data pattern 0x%x. Throughput= %dMbps\n",
		dataPattern, throughput);
		
	return SPI_LOOP_TEST_BUF_SIZE;
}

void SPI_interrupt_test()
{	
	printf("SPI interrupt test: manually generate RX overrun error...\n");
	
	//write three data at TX side without read at RX side to generate RX overrun error
	gpSPI_regs->SPIDAT0= 0x5a5a;
	TSC_delay_ms(1);
	gpSPI_regs->SPIDAT0= 0xa5a5;
	TSC_delay_ms(1);
	gpSPI_regs->SPIDAT0= 0xffff;
	TSC_delay_ms(100);
	//asm(" IDLE"); //wait for interrupt
}


void SPI_loopback_test()
{
	SPI_loopback_test_one_time(0);
	SPI_loopback_test_one_time(0xFFFF);
	SPI_loopback_test_one_time(0x5555);
	
	SPI_interrupt_test();
}

