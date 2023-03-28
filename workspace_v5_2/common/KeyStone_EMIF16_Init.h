/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2012
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
EMIF16 configuration
 * =============================================================================
 *  Revision History
 *  ===============
 *  2011.12 Zhan Xiang  file created
 *  Jan 14, 2012 Brighton Feng Updated
 * =============================================================================
 */
#ifndef _KEYSTONE_EMIF16_INIT_H_
#define _KEYSTONE_EMIF16_INIT_H_

#include <ti\csl\tistdtypes.h>
#include <cslr_emif16.h>

typedef enum {
	NOR_ASRAM_MODE = 0,
	NAND_MODE = 1
}EMIF16_OP_MODE;

typedef enum {
	WE_STROBE = 0,
	SS_STROBE = 1
}EMIF16_STROBE_MODE;

typedef enum {
	EMIF_WAIT_NONE = -1,
	EMIF_WAIT0 = 0,
	EMIF_WAIT1 = 1
}EMIF16_WAIT_MODE;

typedef enum {
	EMIF_LOW_WAIT = 0,
	EMIF_HIGH_WAIT = 1
}EMIF16_WAIT_POLARITY;

typedef enum {
	EMIF_BUS_8BIT = 0,
	EMIF_BUS_16BIT = 1
}EMIF16_BUS_WIDTH;

typedef enum {
	EMIF16_4_WORDS_NOR_PAGE = 0,
	EMIF16_8_WORDS_NOR_PAGE = 1
}EMIF16_NOR_PAGE_MODE_SIZE;

typedef struct{
	EMIF16_NOR_PAGE_MODE_SIZE 		pageSize;
	Uint8 		pageDelay;

}EMIF16_NOR_PG_CFG;

typedef struct  {

	/* Asynchronous Memory Bus Width */
	EMIF16_BUS_WIDTH    busWidth;
	/* Nor/ASRAM or Nand */
	EMIF16_OP_MODE 		opMode;
	/* WE strobe or SS */
	EMIF16_STROBE_MODE 	strobeMode;
	/* extend wait mode */
	EMIF16_WAIT_MODE 	waitMode;
	/* write timing parameter */
	Uint8 wrSetup; 	/*unsigned 4 bit, expected value -1*/
	Uint8 wrStrobe; /*unsigned 6 bit, expected value -1*/
	Uint8 wrHold;   /*unsigned 3 bit, expected value -1*/
	/* Read timing parameter */
	Uint8 rdSetup;  /*unsigned 4 bit, expected value -1*/
	Uint8 rdStrobe; /*unsigned 6 bit, expected value -1*/
	Uint8 rdHold;   /*unsigned 3 bit, expected value -1*/
	/*Turn Around cycles*/
	Uint8 turnAroundCycles; /*unsigned 2 bit, expected value -1*/
	/* NOR flash page mode configuration */
	EMIF16_NOR_PG_CFG * nor_pg_Cfg;

} EMIF16_CE_Config;

typedef struct  {

	/*polarity of wait pins*/	
	EMIF16_WAIT_POLARITY wait0Polarity;
	EMIF16_WAIT_POLARITY wait1Polarity;
	/*Maximum extended wait cycles. 
	EMIF16 will actually wait for (MAX_EXT_WAIT + 1) x 16 cycles*/
	Uint8  				maxWait;
	EMIF16_CE_Config * ceCfg[4];

} EMIF16_Config;

extern CSL_Emif16Regs * EMIF16_Regs;

extern void KeyStone_EMIF16_init(EMIF16_Config *pEmif16Cfg);


#endif
