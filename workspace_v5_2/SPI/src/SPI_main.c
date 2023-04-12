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
This example shows SPI master mode operations:
1. SPI loopback
2. read/write SPI NOR FLASH
 ****************************************************************************
 * Created by :                                                             *
 *            Brighton Feng                                                 *
 *            Texas Instruments                                             * 
 *            Feb 25, 2012                                                  *
 *                                                                          *
 *            Dec 3,  2013 Cheng Peng Updated                               *
***************************************************************************/
#include <c6x.h>
#include <stdio.h>
#include "KeyStone_common.h"
#include "Keystone_DDR_Init.h"
#include "KeyStone_SPI_init_drv.h"
#include "SPI_loopback_TEST.h"
#include "SPI_NOR_FLASH_test.h"
#include "SPI_Intc.h"
#include "SPI_EDMA_Test.h"


#define SPI_LOOPBACK_TEST 		1
#define SPI_NOR_FLASH_TEST 		1
#define SPI_EDMA_TEST           1

SPI_Config spiCfg;
SPI_Interrupt_Config spiIntCfg;


void SPI_Init()
{
	/*data format for NOR FLASH test*/
	spiCfg.dataFormat[0]= &FlashDataFormat;

	/*data format for loopback test*/
	spiCfg.dataFormat[1]= &loopbackDataFormat;

	/*data format for EDMA loopback test*/	
	spiCfg.dataFormat[2]= &EdmaDataFormat;

	spiIntCfg.overrunInterruptEnable= TRUE;
	spiIntCfg.bitErrorInterruptEnable= TRUE;
	spiCfg.interruptCfg     = &spiIntCfg;	  
	
	
	spiCfg.number_SPI_pins  = 4;

	KeyStone_SPI_init(&spiCfg);
}


void main()
{
	Uint32 tscl, tsch;
	int iFLASH_size_KB;
	int iFLASH_addres_width;	

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

	/*clear configuration structure to default values*/
	memset(&spiCfg, 0, sizeof(spiCfg));
	memset(&spiIntCfg, 0, sizeof(spiIntCfg));
	
	
	if(C6678_EVM==gDSP_board_type)
	{
		//DSP core speed: 100*10/1=1000MHz
		KeyStone_main_PLL_init(100, 10, 1); 

		//On Shannon, SPI belong to clock domain 3 which need be enabled
		KeyStone_enable_PSC_module(CSL_PSC_PD_ALWAYSON, 3);
#if SPI_NOR_FLASH_TEST
		/*N25Q128 NOR FLASH test on Shannon EVM*/
		FlashDataFormat.clockSpeedKHz =54000;
		iFLASH_size_KB= 16*1024;
		uiSPI_NOR_FLASH_page_size= 256;
		needEraseBeforeWrite= TRUE;
#endif
	}
	else if(C6670_EVM==gDSP_board_type)
	{
		//DSP core speed: 122.88*236/29= 999.9889655MHz
		KeyStone_main_PLL_init(122.88, 236, 29);

#if SPI_NOR_FLASH_TEST
		/*N25Q128 NOR FLASH test on Nyquist EVM*/
		FlashDataFormat.clockSpeedKHz =54000;
		iFLASH_size_KB= 16*1024;
		uiSPI_NOR_FLASH_page_size= 256;
		needEraseBeforeWrite= TRUE;
#endif
	}
	else if(TCI6614_EVM==gDSP_board_type)
	{
		//DSP core speed: 122.88*236/29= 999.9889655MHz
		KeyStone_main_PLL_init(122.88, 236, 29);

#if SPI_NOR_FLASH_TEST
		/*N25Q032 NOR FLASH test on Appleton EVM*/
		FlashDataFormat.clockSpeedKHz =54000;
		iFLASH_size_KB= 4*1024;
		uiSPI_NOR_FLASH_page_size= 256;
		needEraseBeforeWrite= TRUE;
#endif
	}
	else if(DUAL_NYQUIST_EVM==gDSP_board_type)
	{
		//DSP core speed: 122.88*236/29= 999.9889655MHz
		KeyStone_main_PLL_init(122.88, 236, 29);

#if SPI_NOR_FLASH_TEST
		/*AT25640B NOR FLASH test on dual Nyquist EVM*/
		FlashDataFormat.clockSpeedKHz =5000;
		iFLASH_size_KB= 8;
		uiSPI_NOR_FLASH_page_size= 32;
		needEraseBeforeWrite= FALSE;
#endif
	}
	else
	{
		puts("Unknown DSP board type!");
		return;
	}

	//DDR init 66.66667*20/1= 1333
	KeyStone_DDR_init (66.66667, 20, 1, NULL);

    SPI_Interrupts_Init();

    
#if SPI_LOOPBACK_TEST	
	spiCfg.loopBack = TRUE;	
    spiCfg.DMA_requestEnable= FALSE;
    
	SPI_Init();	
	printf("\nSPI internal loopback test at %dMHz...\n",
		loopbackDataFormat.clockSpeedKHz/1000);
	SPI_loopback_test();
#endif
	
#if SPI_EDMA_TEST     
    SPI_EDMA_Init(); 
    
    spiCfg.loopBack = TRUE;
    spiCfg.DMA_requestEnable= TRUE;
	SPI_Init();
	printf("\nSPI EDMA test at %dMHz...\n",EdmaDataFormat.clockSpeedKHz/1000);
	    
	SPI_EDMA_test();	
#endif


#if SPI_NOR_FLASH_TEST
	spiCfg.loopBack = FALSE;	
    spiCfg.DMA_requestEnable= FALSE;
	SPI_Init();

	if(iFLASH_size_KB>(16*1024))
		iFLASH_addres_width = 32;
	else if(iFLASH_size_KB>(64))
		iFLASH_addres_width = 24;
	else
		iFLASH_addres_width = 16;

	printf("\nSPI NOR FLASH test at %dMHz...\n",
		FlashDataFormat.clockSpeedKHz/1000);

	if(iFLASH_addres_width>24) 	//enabele 4 bytes address
		SPI_NOR_FLASH_command(SPI_NOR_FLASH_EN4B);

	Uint8 ID[4];
	SPI_NOR_FLASH_ID_read(ID);
	printf("Read SPI NOR FLASH ID = 0x%x 0x%x 0x%x\n", ID[0], ID[1], ID[2]);
	
	if(iFLASH_size_KB<=256)
		SPI_NOR_FLASH_Test(0, iFLASH_size_KB*1024, iFLASH_addres_width);
	else
	{
		int i;
		/*spilt large space into small sections to show the progress,
		reserved the first half, only test the second half to save time*/
		//for(i=iFLASH_size_KB/2; i<iFLASH_size_KB; i+= 256)
		for(i=iFLASH_size_KB*15/16; i<iFLASH_size_KB; i+= 256)
			SPI_NOR_FLASH_Test(i*1024, (i+256)*1024, iFLASH_addres_width);
	}
#endif
	
	tscl= TSCL;
	tsch= TSCH;
	printf("SPI test complete at %lld cycle\n", _itoll(tsch, tscl));
	CACHE_invAllL1p(CACHE_WAIT);
	CACHE_wbInvAllL1d(CACHE_WAIT);
	CSL_XMC_invalidatePrefetchBuffer();
	_mfence();
	_mfence();
}


