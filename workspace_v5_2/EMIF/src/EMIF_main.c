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
This example shows EMIF operations:
1. NAND FLASH test, Verified on NAND512R3A2DZA6E on Shannon EVM,
                               MT29F1G08ABBDAHC on Appleton EVM.
2. NOR FLASH test, Verified on JS28F256M29EW on a customer's board.
 ****************************************************************************
 * Created by :                                                             *
 *            Brighton Feng                                                 *
 *            Texas Instruments                                             * 
 *            Jan 5, 2012                                                 *
***************************************************************************/
#include <stdio.h>
#include <string.h>
#include "KeyStone_common.h"
#include "Keystone_DDR_Init.h"
#include "KeyStone_EMIF16_init.h"
#include "EMIF_NOR_FLASH_test.h"
#include "EMIF_NAND_FLASH_test.h"

/*select between NAND FLASH or NOR FLASH test*/
#define NAND_FLASH_TEST 	1

#define FLASH_TEST_BUFFER_SIZE 	(1*1024*1024)
#pragma DATA_SECTION(testBuf,".far:testBuf")
Uint8	testBuf[FLASH_TEST_BUFFER_SIZE];

EMIF16_CE_Config gNorCeCfg;	/*the configuration for the CE of NOR FLASH*/
EMIF16_CE_Config gNandCeCfg;/*the configuration for the CE of NAND FLASH*/
EMIF16_Config gEmif16Cfg;
void EMIF_init()
{
	memset(&gNorCeCfg, 0, sizeof(gNorCeCfg));
	memset(&gNandCeCfg, 0, sizeof(gNandCeCfg));
	memset(&gEmif16Cfg, 0, sizeof(gEmif16Cfg));
	
	/*configuraiton for the CE of NOR FLASH*/
	gNorCeCfg.busWidth= EMIF_BUS_16BIT;
	gNorCeCfg.opMode  = NOR_ASRAM_MODE;
	gNorCeCfg.strobeMode = SS_STROBE;  
	gNorCeCfg.waitMode = EMIF_WAIT_NONE;
	gNorCeCfg.wrSetup = CSL_EMIF16_A0CR_WSETUP_RESETVAL;          
	gNorCeCfg.wrStrobe = CSL_EMIF16_A0CR_WSTROBE_RESETVAL;         
	gNorCeCfg.wrHold= CSL_EMIF16_A0CR_WHOLD_RESETVAL;            
	gNorCeCfg.rdSetup= CSL_EMIF16_A0CR_RSETUP_RESETVAL;           
	gNorCeCfg.rdStrobe= CSL_EMIF16_A0CR_RSTROBE_RESETVAL;          
	gNorCeCfg.rdHold= CSL_EMIF16_A0CR_RHOLD_RESETVAL;            
	gNorCeCfg.turnAroundCycles= CSL_EMIF16_A0CR_TA_RESETVAL;  
	gNorCeCfg.nor_pg_Cfg= NULL;        

	/*configuraiton for the CE of NAND FLASH*/
	gNandCeCfg.busWidth= EMIF_BUS_8BIT;
	gNandCeCfg.opMode  = NAND_MODE;
	gNandCeCfg.strobeMode = SS_STROBE;  
	gNandCeCfg.waitMode = EMIF_WAIT_NONE;
	/*timing configuration for NAND512R3A2DZA6E*/
#if 1
	gNandCeCfg.wrSetup = 1;//CSL_EMIF16_A0CR_WSETUP_RESETVAL,          
	gNandCeCfg.wrStrobe = 4;//CSL_EMIF16_A0CR_WSTROBE_RESETVAL,         
	gNandCeCfg.wrHold= 5;//CSL_EMIF16_A0CR_WHOLD_RESETVAL,            
	gNandCeCfg.rdSetup= 1;//CSL_EMIF16_A0CR_RSETUP_RESETVAL           
	gNandCeCfg.rdStrobe= 7;//CSL_EMIF16_A0CR_RSTROBE_RESETVAL          
	gNandCeCfg.rdHold= 2;//CSL_EMIF16_A0CR_RHOLD_RESETVAL            
#else
	gNandCeCfg.wrSetup = CSL_EMIF16_A0CR_WSETUP_RESETVAL;          
	gNandCeCfg.wrStrobe = CSL_EMIF16_A0CR_WSTROBE_RESETVAL;         
	gNandCeCfg.wrHold= CSL_EMIF16_A0CR_WHOLD_RESETVAL;            
	gNandCeCfg.rdSetup= CSL_EMIF16_A0CR_RSETUP_RESETVAL;           
	gNandCeCfg.rdStrobe= CSL_EMIF16_A0CR_RSTROBE_RESETVAL;         
	gNandCeCfg.rdHold= CSL_EMIF16_A0CR_RHOLD_RESETVAL;            
#endif
	gNandCeCfg.turnAroundCycles= CSL_EMIF16_A0CR_TA_RESETVAL;  
	gNandCeCfg.nor_pg_Cfg= NULL;        

#if NAND_FLASH_TEST
	gEmif16Cfg.ceCfg[0] = &gNandCeCfg;
#else
	gEmif16Cfg.ceCfg[0] = &gNorCeCfg;
#endif

	gEmif16Cfg.wait0Polarity= EMIF_LOW_WAIT;
	gEmif16Cfg.wait1Polarity= EMIF_LOW_WAIT;
	gEmif16Cfg.maxWait = CSL_EMIF16_AWCCR_MAXEXTWAIT_RESETVAL;
	
	KeyStone_EMIF16_init(&gEmif16Cfg);

}

void main()
{
	/*enable TSC, memory protection interrupts, EDC for internal RAM;
    clear cache; protect L1 as cache*/
	KeyStone_common_CPU_init();
	/*print device information. 
	Enable memory protection interrupts, EDC for MSMC RAM*/
	KeyStone_common_device_init();

	//enable exception handling
	KeyStone_Exception_cfg(TRUE);

    // System init
    CACHE_setL1PSize(CACHE_L1_32KCACHE);
    CACHE_setL1DSize(CACHE_L1_32KCACHE);
    CACHE_setL2Size(CACHE_128KCACHE);

	if(C6678_EVM==gDSP_board_type)
	{
		//DSP core speed: 100*10/1=1000MHz
		KeyStone_main_PLL_init(100, 10, 1); 

		//On Shannon, EMIF belong to clock domain 3 which need be enabled
		KeyStone_enable_PSC_module(CSL_PSC_PD_ALWAYSON, 3);
	}
	else if(TCI6614_EVM==gDSP_board_type)
	{
		//DSP core speed: 122.88*236/29= 999.9889655MHz
		KeyStone_main_PLL_init(122.88, 236, 29);
	}
	else
	{
		puts("Unknown DSP board type!");
		return;
	}

	//DDR init 66.66667*20/1= 1333
	KeyStone_DDR_init (66.66667, 20, 1, NULL);

	EMIF_init();

#if NAND_FLASH_TEST
	NAND_FLASH_test((Uint32)testBuf, FLASH_TEST_BUFFER_SIZE);
#else
	NOR_FLASH_test((Uint32)testBuf, FLASH_TEST_BUFFER_SIZE);
#endif

	puts("FLASH test complete.");
}

