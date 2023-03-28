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

#include <stdio.h>
#include "KeyStone_EMIF16_Init.h"
#include <cslr_device.h>

#if defined(CSL_EMIF16_CFG_REGS)
CSL_Emif16Regs * EMIF16_Regs= (CSL_Emif16Regs *)CSL_EMIF16_CFG_REGS;
#endif

#if defined(CSL_EMIF16_REGS)
CSL_Emif16Regs * EMIF16_Regs= (CSL_Emif16Regs *)CSL_EMIF16_REGS;
#endif

void KeyStone_EMIF16_init(EMIF16_Config *pEmif16Cfg)
{
	int i;
	Uint32 regVal;
	volatile Uint32 *ACR= &EMIF16_Regs->A0CR;
	volatile Uint32 PMCR= 0;
	volatile Uint32 AWCCR= 0;
	volatile Uint32 NANDFCR= 0;
	EMIF16_CE_Config * ceCfg;

	for(i=0; i<4; i++) 	/*4 CEs*/
	{
		if(NULL == pEmif16Cfg->ceCfg[i])
			continue;

		ceCfg= pEmif16Cfg->ceCfg[i];
		
		/*timing parameter check*/
		if((ceCfg->wrSetup<<CSL_EMIF16_A0CR_WSETUP_SHIFT) & (~CSL_EMIF16_A0CR_WSETUP_MASK))
		{
			puts("write setup timing value is too large");
			continue;
		}
		if((ceCfg->wrStrobe<<CSL_EMIF16_A0CR_WSTROBE_SHIFT) & (~CSL_EMIF16_A0CR_WSTROBE_MASK))
		{
			puts("write strobe timing value is too large");
			continue;
		}
		if((ceCfg->wrHold<<CSL_EMIF16_A0CR_WHOLD_SHIFT) & (~CSL_EMIF16_A0CR_WHOLD_MASK))
		{
			puts("write hold timing value is too large");
			continue;
		}
		if((ceCfg->rdSetup<<CSL_EMIF16_A0CR_RSETUP_SHIFT) & (~CSL_EMIF16_A0CR_RSETUP_MASK))
		{
			puts("read setup timing value is too large");
			continue;
		}
		if((ceCfg->rdStrobe<<CSL_EMIF16_A0CR_RSTROBE_SHIFT) & (~CSL_EMIF16_A0CR_RSTROBE_MASK))
		{
			puts("read strobe timing value is too large");
			continue;
		}
		if((ceCfg->rdHold<<CSL_EMIF16_A0CR_RHOLD_SHIFT) & (~CSL_EMIF16_A0CR_RHOLD_MASK))
		{
			puts("read hold timing value is too large");
			continue;
		}
		if((ceCfg->turnAroundCycles<<CSL_EMIF16_A0CR_TA_SHIFT) & (~CSL_EMIF16_A0CR_TA_MASK))
		{
			puts("turn around timing value is too large");
			continue;
		}

		/*Async Config Register*/
		regVal= ((ceCfg->strobeMode<<CSL_EMIF16_A0CR_SS_SHIFT) & CSL_EMIF16_A0CR_SS_MASK)
				|((ceCfg->wrSetup<<CSL_EMIF16_A0CR_WSETUP_SHIFT) & CSL_EMIF16_A0CR_WSETUP_MASK)
				|((ceCfg->wrStrobe<<CSL_EMIF16_A0CR_WSTROBE_SHIFT) & CSL_EMIF16_A0CR_WSTROBE_MASK)
				|((ceCfg->wrHold<<CSL_EMIF16_A0CR_WHOLD_SHIFT) & CSL_EMIF16_A0CR_WHOLD_MASK)
				|((ceCfg->rdSetup<<CSL_EMIF16_A0CR_RSETUP_SHIFT) & CSL_EMIF16_A0CR_RSETUP_MASK)
				|((ceCfg->rdStrobe<<CSL_EMIF16_A0CR_RSTROBE_SHIFT) & CSL_EMIF16_A0CR_RSTROBE_MASK)
				|((ceCfg->rdHold<<CSL_EMIF16_A0CR_RHOLD_SHIFT) & CSL_EMIF16_A0CR_RHOLD_MASK)
				|((ceCfg->turnAroundCycles<<CSL_EMIF16_A0CR_TA_SHIFT) & CSL_EMIF16_A0CR_TA_MASK)
				|((ceCfg->busWidth<<CSL_EMIF16_A0CR_ASIZE_SHIFT)&CSL_EMIF16_A0CR_ASIZE_MASK);
		if(EMIF_WAIT_NONE!=ceCfg->waitMode)
		{
			regVal |= ((1<<CSL_EMIF16_A0CR_EW_SHIFT) & CSL_EMIF16_A0CR_EW_MASK);
			AWCCR |= (((ceCfg->waitMode<<CSL_EMIF16_AWCCR_CE0WAIT_SHIFT)&CSL_EMIF16_AWCCR_CE0WAIT_MASK)<<(i*2));
		}
		ACR[i]= regVal;

		/*Page Mode Control Register*/
		if(ceCfg->nor_pg_Cfg)
		{
			regVal = ((ceCfg->nor_pg_Cfg->pageDelay<<CSL_EMIF16_PMCR_CE0PGDEL_SHIFT)&CSL_EMIF16_PMCR_CE0PGDEL_MASK)
				|((ceCfg->nor_pg_Cfg->pageSize<<CSL_EMIF16_PMCR_CE0PGSIZE_SHIFT)&CSL_EMIF16_PMCR_CE0PGSIZE_MASK)
				|((1<<CSL_EMIF16_PMCR_CE0PGMDEN_SHIFT)&CSL_EMIF16_PMCR_CE0PGMDEN_MASK);
			PMCR |= (regVal<<(i*8));
		}
		
		/*NAND Flash Control Register*/
		if(NAND_MODE==ceCfg->opMode)
			NANDFCR |= (((1<<CSL_EMIF16_NANDFCTL_CE0NAND_SHIFT)&CSL_EMIF16_NANDFCTL_CE0NAND_MASK)<<i);
	}

	/*Async Wait Cycle Config Register*/
	AWCCR |= (((pEmif16Cfg->maxWait<<CSL_EMIF16_AWCCR_MAXEXTWAIT_SHIFT)&CSL_EMIF16_AWCCR_MAXEXTWAIT_MASK)
		|((pEmif16Cfg->wait0Polarity<<CSL_EMIF16_AWCCR_WP0_SHIFT)&CSL_EMIF16_AWCCR_WP0_MASK)
		|((pEmif16Cfg->wait1Polarity<<CSL_EMIF16_AWCCR_WP1_SHIFT)&CSL_EMIF16_AWCCR_WP1_MASK));
	EMIF16_Regs->AWCCR= AWCCR;

	EMIF16_Regs->PMCR = PMCR;
	EMIF16_Regs->NANDFCTL= NANDFCR;

	/*Although it supports only asynchronous mode operation on all KeyStone©\I 
	devices, the EMIF16 module has a legacy ¡®synchronous mode¡¯ feature that 
	is enabled by default.*/
	*(Uint32*) 0x20C00008 |= 0x80000000; //Disable synchronous mode feature

}

