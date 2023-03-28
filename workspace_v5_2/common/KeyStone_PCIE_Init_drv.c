/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2013
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
* ============================================================================
 Example to show the configuration of PCIE on KeyStone DSP
* =============================================================================
 *  Revision History
 *  ===============
 *  June 13, 2013 Thomas Yang   File Created
 *  June 22, 2013 Brighton Feng   Update APIs
 * ============================================================================
 */
#include <stdio.h>
#include <csl_pscAux.h>
#include <csl_bootcfgAux.h>
#include "KeyStone_PCIE_init_drv.h"

/************************Global Variable define******************************/
CSL_Pciess_appRegs   				*gpPCIE_app_regs = (CSL_Pciess_appRegs*)CSL_PCIE_CONFIG_REGS;
CSL_Pcie_cfg_space_rootcomplexRegs 	*gpPCIE_RC_regs  = (CSL_Pcie_cfg_space_rootcomplexRegs*)(CSL_PCIE_CONFIG_REGS + 0x1000);
CSL_Pcie_cfg_space_endpointRegs		*gpPCIE_EP_regs  = (CSL_Pcie_cfg_space_endpointRegs*)(CSL_PCIE_CONFIG_REGS + 0x1000);

//PCIE capability and implementation specific registers
PCIE_CAP_Implement_Regs 			*gpPCIE_CAP_implement_regs 	= (PCIE_CAP_Implement_Regs *)(CSL_PCIE_CONFIG_REGS + 0x1040);

//remote PCIE EP registers  
CSL_Pcie_cfg_space_endpointRegs		*gpPCIE_remote_EP_regs = (CSL_Pcie_cfg_space_endpointRegs*)(CSL_PCIE_CONFIG_REGS+0x3000);

/*********************Variables for PCIE address allocation*******************/
unsigned long long gullFreePrefetchPcieAddress;
unsigned long long gullFreeNonfetchPcieAddress;
Uint32 uiOutboundRegionIndex;

/*PCIE soft reset*/
void KeyStone_PCIE_soft_reset()
{
	/*Disable LTSSM*/
	gpPCIE_app_regs->CMD_STATUS &= ~CSL_PCIESS_APP_CMD_STATUS_LTSSM_EN_MASK;

	/*Wait to finish any current DMA transfer.*/
	TSC_delay_ms(50);

	//disable Serdes
	CSL_BootCfgUnlockKicker();
	gpBootCfgRegs->PCIE_CFGPLL= 0;

	//disable PCIE through PSC
	KeyStone_disable_PSC_module(CSL_PSC_PD_PCIEX, CSL_PSC_LPSC_PCIEX);
	KeyStone_disable_PSC_Power_Domain(CSL_PSC_PD_PCIEX);
}

/*round up to power of 2*/
static inline unsigned long long round_up_power_2(unsigned long long ullNumber)
{
	if(0x80000000<ullNumber&&ullNumber<= 0xFFFFFFFF)
		ullNumber= 0x100000000;
	else if(_hill(ullNumber))
		ullNumber= _itoll(1<<(31-_lmbd(1, _hill(ullNumber))+1), 0);
	else
		ullNumber= _itoll(0, 1<<(31-_lmbd(1, _loll(ullNumber))+1));
	return ullNumber;
}

/*****************************************************************************
 Prototype    : KeyStone_PCIE_Serdes_init
 Description  : PCIE SERDES configuration
 Input        : KeyStone_PCIE_SerdesConfig * pcie_serdes_cfg  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2013/8/19
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_PCIE_Serdes_init(
	KeyStone_PCIE_SerdesConfig * pcie_serdes_cfg)
{
	Uint32 SERDES_CFG;
	float pllMpy;

	if(NULL== pcie_serdes_cfg)
		return;
		
	CSL_BootCfgUnlockKicker();

	SERDES_CFG= 
		(pcie_serdes_cfg->loopBack<<PCIE_SERDES_CFG_TX_LOOPBACK_SHIFT)| 
		(pcie_serdes_cfg->loopBack<<PCIE_SERDES_CFG_RX_LOOPBACK_SHIFT)| 
		(1 << PCIE_SERDES_CFG_TX_CM_SHIFT)| 	/*Common mode adjustment*/
		(pcie_serdes_cfg->txInvertPolarity<<PCIE_SERDES_CFG_TX_INVPAIR_SHIFT)|
		(pcie_serdes_cfg->rxInvertPolarity<<PCIE_SERDES_CFG_RX_INVPAIR_SHIFT)|
		(1<<PCIE_SERDES_CFG_RX_ENOC_SHIFT)| 	/*Enable offset compensation*/
		(pcie_serdes_cfg->rxEqualizerConfig<<PCIE_SERDES_CFG_RX_EQ_SHIFT)|
		(pcie_serdes_cfg->rxCDR<<PCIE_SERDES_CFG_RX_CDR_SHIFT)|
		(pcie_serdes_cfg->rxLos<<PCIE_SERDES_CFG_RX_LOS_SHIFT)|
		(pcie_serdes_cfg->rxAlign<<PCIE_SERDES_CFG_RX_ALIGN_SHIFT);

	gpPCIE_app_regs->SERDES_CFG0= SERDES_CFG|
		(1 << PCIE_SERDES_CFG_TX_MSYNC_SHIFT); 	/*Synchronization Master.*/

	gpPCIE_app_regs->SERDES_CFG1= SERDES_CFG|
		(0 << PCIE_SERDES_CFG_TX_MSYNC_SHIFT); 	

	/*calculate PLL MPY factor according to input reference clock speed*/
	pllMpy= 2500.f/pcie_serdes_cfg->inputRefClock_MHz;

	/*PLL multiply factors between 4 and 60*/
	if(pllMpy<4 || pllMpy>60)
		pllMpy= 4;

	gpBootCfgRegs->PCIE_CFGPLL = (1<<SERDES_PLL_CFG_ENPLL_SHIFT)|
		(pcie_serdes_cfg->loopBandwidth<<SERDES_PLL_CFG_LOOPBANDWIDTH_SHIFT)|
		(SERDES_PLL_VCO_RANGE_LOW<<SERDES_PLL_CFG_VRANGE_SHIFT)|
		((Uint32)(pllMpy*4)<<SERDES_PLL_CFG_MPY_SHIFT);

}

/*initialize BAR for prefetchable or non-prefetchable memory regions.
Please note, multiple regions with same properties may map to same BAR,
BAR mask are set according to the total size of the regions.
This step doesn't config BAR base address, which should be set later
*/
void KeyStone_PCIE_BAR_Init(Uint32 uiBAR_num, 
	PCIE_Memory_Regions * regions, 
	PCIE_Address_Width address_width)
{
	int i;
	volatile Uint32 uiDummy;
	Uint32 uiAddressType= 0;
	Uint32 uiNumRegions;
	unsigned long long ullMask;
	unsigned long long ullTotalMemorySize;
	PCIE_Memory_Region * memory_regions;
	
	if(0!=uiBAR_num)
	{
		uiNumRegions= regions->uiNumRegions;
		memory_regions= regions->memory_regions;

		/*calculate total memory size of the regions*/
		ullTotalMemorySize= 0;
		for(i=0; i< uiNumRegions; i++)
		{
			ullTotalMemorySize+= memory_regions[i].uiNumBytes;
		}
		regions->ullTotalSize= ullTotalMemorySize;

		if(0==ullTotalMemorySize)
			return;

		if((ullTotalMemorySize>= 0xFFFFFFFF) && 
			(PCIE_ADDRESS_32_BITS==address_width))
		{
			puts("PCIE memory regions size larger than 4GB, but the address is 32 bits!");
			return;
		}
		
		/*round the memory size up to power of 2*/
		ullTotalMemorySize= round_up_power_2(ullTotalMemorySize);

		ullMask= ullTotalMemorySize-1;
		regions->ullBAR_Mask= ullMask;
	}
	else/*BAR_Num = 0 for application region*/
	{
		ullMask = 0x00000000000FFFFF;/*Fixed value for Keystone applicaton region mask*/
	}
	/*BAR Mask registers are accessible, for configuration purposes of the
	BARs, only when DBI_CS2 (bit 5 in Command Status Register (CMD_STATUS)) 
	is enabled.*/
	gpPCIE_app_regs->CMD_STATUS |= CSL_PCIESS_APP_CMD_STATUS_DBI_CS2_MASK;

	/*software needs to always read back the written value to DBI_CS2 after 
	modification to ensure that the write has completed since the read will 
	not happen until the write completion.*/
	uiDummy= gpPCIE_app_regs->CMD_STATUS;
	
	//setup the low 32-bit of the MASK and enable the BAR
	gpPCIE_EP_regs->BAR[uiBAR_num]= _loll(ullMask)|1;

	//setup the high 32-bit of the MASK and enable the BAR
	if(PCIE_ADDRESS_64_BITS==address_width) //64-bit address
	{
		gpPCIE_EP_regs->BAR[uiBAR_num+1]= _hill(ullMask);
		uiAddressType= 2; /*2 means 64-bit type*/
	}

	/*The software needs to clear CMD_STATUS [DBI_CS2] 
	after initial configuration prior to RC starts enumeration.*/
	gpPCIE_app_regs->CMD_STATUS &= ~CSL_PCIESS_APP_CMD_STATUS_DBI_CS2_MASK;

	/*software needs to always read back the written value to DBI_CS2 after 
	modification to ensure that the write has completed since the read will 
	not happen until the write completion.*/
	uiDummy= gpPCIE_app_regs->CMD_STATUS;

	//setup BAR properties
	if(0!=uiBAR_num)
	{
		gpPCIE_EP_regs->BAR[uiBAR_num]= 
			(uiAddressType<<CSL_PCIE_CFG_SPACE_ENDPOINT_BAR_TYPE_SHIFT)
			|(regions->bPrefetchable<<CSL_PCIE_CFG_SPACE_ENDPOINT_BAR_PREFETCHABLE_SHIFT);
	}
	else
	{
		gpPCIE_EP_regs->BAR[uiBAR_num]=
			(uiAddressType<<CSL_PCIE_CFG_SPACE_ENDPOINT_BAR_TYPE_SHIFT)
			|(0<<CSL_PCIE_CFG_SPACE_ENDPOINT_BAR_PREFETCHABLE_SHIFT);/*Non prefetch for application region*/
	}
}

/*PCIE inbound memory regions initialization.
All non-prefetchalbe memory regions are mapped to one BAR.
All prefetchalbe memory regions are mapped to another BAR.
the BAR PCIE address should be allocated and set by RC during enumeration*/
void KeyStone_PCIE_Inbound_Memory_Region_Init(KeyStone_PCIE_Config * pcie_cfg)
{
	Uint32 uiNumRegions=0, uiNumPrefetchRegions=0, uiBAR_index;
	PCIE_Inbound_Memory_Regions * inbound_memory_regions;
	
	/*BAR0 (and BAR1 for 64-bit mode) is reserved for accessing PCIE registers*/
	KeyStone_PCIE_BAR_Init(0, 0, pcie_cfg->address_width);

	/*NULL pointer menas inbound access is not used*/
	if(NULL==pcie_cfg->inbound_memory_regions)
		return;

	//enabled to be accessed as memory
	gpPCIE_EP_regs->STATUS_COMMAND |= 
		CSL_PCIE_CFG_SPACE_ENDPOINT_STATUS_COMMAND_MEMORY_SPACE_MASK;

	inbound_memory_regions= pcie_cfg->inbound_memory_regions;

	if(inbound_memory_regions->nonfetch_regions)
		uiNumRegions= inbound_memory_regions->nonfetch_regions->uiNumRegions;
	if(inbound_memory_regions->prefetch_regions)
		uiNumPrefetchRegions= inbound_memory_regions->prefetch_regions->uiNumRegions;

	/*RC mode only has two BARs, one configuration BAR (fixed), one memory BAR.*/
	if(PCIE_RC_MODE==pcie_cfg->PcieMode)
	{
		if(PCIE_ADDRESS_64_BITS==pcie_cfg->address_width)
		{
			puts("RC in 64-bit address mode does not have BAR for memory space!");
			if(uiNumRegions)
				inbound_memory_regions->nonfetch_regions->uiNumRegions= 0;
			if(uiNumPrefetchRegions)
				inbound_memory_regions->prefetch_regions->uiNumRegions= 0;
			return;
		}

		/*both prefetch regions and non-prefetch regions exist,
		but RC in 32-bit mode only has one BAR for memory spce*/
		if((uiNumRegions>0) && (uiNumPrefetchRegions>0))
		{
			uiNumRegions= 0;
			inbound_memory_regions->nonfetch_regions->uiNumRegions= 0;
			
			puts("RC mode does not have more than two BARs! The non-prefetch regions are ignored");	
		}
	}

	if((uiNumRegions+uiNumPrefetchRegions)>4)
	{
		puts("Maximum number of inbound regions is 4, addtional regions are ingnored!");

		/*remove the addtional regions*/
		if(uiNumPrefetchRegions>4)
		{
			uiNumPrefetchRegions= 4;
			inbound_memory_regions->prefetch_regions->uiNumRegions= 4;
			if(uiNumRegions>0)
			{
				uiNumRegions= 0;
				inbound_memory_regions->nonfetch_regions->uiNumRegions= 0;
			}
		}
		else
		{
			uiNumRegions= 4-uiNumPrefetchRegions;
			inbound_memory_regions->nonfetch_regions->uiNumRegions= 
				4-uiNumPrefetchRegions;
		}
	}

	/*BAR0 (and BAR1 for 64-bit mode) is reserved for accessing PCIE registers*/
	if(PCIE_ADDRESS_32_BITS==pcie_cfg->address_width)
		uiBAR_index = 1; //first memory BAR is BAR1
	else
		uiBAR_index = 2; //first memory BAR is BAR2

	//initialize BAR for prefetchable regions
	if(uiNumPrefetchRegions)
	{
		inbound_memory_regions->prefetch_regions->bPrefetchable= TRUE;
		KeyStone_PCIE_BAR_Init(uiBAR_index, 
			inbound_memory_regions->prefetch_regions,
			pcie_cfg->address_width);

		//increment BAR index for non-prefetch regions
		if(PCIE_ADDRESS_32_BITS==pcie_cfg->address_width)
			uiBAR_index = 2;
		else
			uiBAR_index = 4;
	}

	//initialize BAR for non-prefetch regions
	if(uiNumRegions)
	{
		inbound_memory_regions->nonfetch_regions->bPrefetchable= FALSE;
		KeyStone_PCIE_BAR_Init(uiBAR_index, 
			inbound_memory_regions->nonfetch_regions,
			pcie_cfg->address_width);
	}

	//enable inbound address translation
	if(uiNumPrefetchRegions||uiNumRegions)
		gpPCIE_app_regs->CMD_STATUS |= CSL_PCIESS_APP_CMD_STATUS_IB_XLT_EN_MASK;
	
}

/*PCIE outbound memory regions initialization*/
void KeyStone_PCIE_Outbound_Memory_Region_Init(
	PCIE_Outbound_Memory_Regions * outbound_memory_regions)
{
	int i;
	Uint32 uiNumRegions= outbound_memory_regions->uiNumRegions; 
	unsigned long long * address_offset=
		outbound_memory_regions->address_offset;

	if(NULL==outbound_memory_regions)
		return;
		
	//enabled to be PCIE master
	gpPCIE_EP_regs->STATUS_COMMAND |= 
		CSL_PCIE_CFG_SPACE_ENDPOINT_STATUS_COMMAND_BUS_MASTER_MASK;

	gpPCIE_app_regs->OB_SIZE= outbound_memory_regions->OB_size;

	//enable outbound address translation
	gpPCIE_app_regs->CMD_STATUS |= CSL_PCIESS_APP_CMD_STATUS_OB_XLT_EN_MASK;

	/*setup outbound memory regions with maunally input parameters,
	typcially used to setup outbound regions in EP mode*/
	uiNumRegions= outbound_memory_regions->uiNumRegions; 
	address_offset= outbound_memory_regions->address_offset;

	if(uiNumRegions>32)
	{
		puts("outbound only has 32 regions, more regions are ignored!");
		return;
	}

	//enable and setup the outbound regions
	for(i= 0; i<uiNumRegions; i++)
	{
		gpPCIE_app_regs->OUTBOUND_TRANSLATION[i].OB_OFFSET_INDEX= 
			CSL_PCIESS_APP_OB_OFFSET_INDEX_OB_ENABLE_MASK
			|((_loll(address_offset[i])>>20)
			  <<CSL_PCIESS_APP_OB_OFFSET_INDEX_OB_OFFSET_LO_SHIFT);
		gpPCIE_app_regs->OUTBOUND_TRANSLATION[i].OB_OFFSET_HI= 
			_hill(address_offset[i]);
	}	
}

//PCIE RC specific initialization
void KeyStone_PCIE_RC_Init(PCIE_RC_Config * rc_cfg, 
	PCIE_Address_Width address_width)
{
	if(NULL==rc_cfg)
		return;

	gpPCIE_RC_regs->MEMSPACE= 
		((rc_cfg->memory_base>>20)
		  <<CSL_PCIE_CFG_SPACE_ROOTCOMPLEX_MEMSPACE_MEMORY_BASE_SHIFT)
		|((rc_cfg->memory_limit>>20)
		  <<CSL_PCIE_CFG_SPACE_ROOTCOMPLEX_MEMSPACE_MEMORY_LIMIT_SHIFT);

	gpPCIE_RC_regs->PREFETCH_MEM= 
		(address_width<<0) 		/*base address width*/
		|(address_width<<16) 	/*limit address width*/
		|((_loll(rc_cfg->prefetch_memory_base)>>20)
		  <<CSL_PCIE_CFG_SPACE_ROOTCOMPLEX_PREFETCH_MEM_STARTADDRESS_SHIFT)
		|((_loll(rc_cfg->prefetch_memory_limit)>>20)
		  <<CSL_PCIE_CFG_SPACE_ROOTCOMPLEX_PREFETCH_MEM_END_ADDRESS_SHIFT);

	gpPCIE_RC_regs->PREFETCH_BASE= _hill(rc_cfg->prefetch_memory_base);
	gpPCIE_RC_regs->PREFETCH_LIMIT= _hill(rc_cfg->prefetch_memory_limit);
	
	//initialize the free pointer for address allocation to memory base
	gullFreeNonfetchPcieAddress= rc_cfg->memory_base;
	gullFreePrefetchPcieAddress= rc_cfg->prefetch_memory_base;
}

/*initialize internal bus between PCIE and memory subsystem*/
void KeyStone_PCIE_Internal_Bus_Init(PCIE_Internal_Bus_Config * bus_cfg)
{
	/*set privilege ID of PCIE to 11, this is the default value*/
	gpPCIE_app_regs->PRIORITY |= 
		(11<<CSL_PCIESS_APP_PRIORITY_MST_PRIVID_SHIFT);

	if(NULL==bus_cfg)
		return;

	gpPCIE_app_regs->PRIORITY |= 
		(bus_cfg->bSupervisor<<CSL_PCIESS_APP_PRIORITY_MST_PRIV_SHIFT)
		|(bus_cfg->priority<<CSL_PCIESS_APP_PRIORITY_MST_PRIORITY_SHIFT);

	gpPCIE_app_regs->ENDIAN |= 
		(bus_cfg->endian_swap<<CSL_PCIESS_APP_ENDIAN_ENDIAN_MODE_SHIFT);
		
}

/*initialize interrupts*/
void KeyStone_PCIE_Interrupt_Init(PCIE_Interrupt_Config * interrupt_cfg, 
	PCIE_Address_Width address_width)
{
	int i;
	Uint32 mask;

	if(NULL==interrupt_cfg)
		return;

	if(interrupt_cfg->MSI_rx_enable_mask)
	{
		mask= interrupt_cfg->MSI_rx_enable_mask;
		for(i=0; i<8; i++)
			gpPCIE_app_regs->MSIX_IRQ[i].MSI_IRQ_ENABLE_SET=
				((mask>>(i))&1)|((mask>>(7+i))&2)|
				((mask>>(14+i))&4)|((mask>>(21+i))&8);
	}

	if(interrupt_cfg->Err_rx_enable)//enable all error interrupt
		gpPCIE_app_regs->ERR_IRQ_ENABLE_SET= 0xFFFFFFFF;

	if(interrupt_cfg->PMRST_rx_enable)//enable all PM and reset interrupt
		gpPCIE_app_regs->PMRST_ENABLE_SET= 0xFFFFFFFF;

	//setup MSI capability register (for EP)
	if(interrupt_cfg->number_tx_MSI!= PCIE_NO_TX_MSI)
	{
		gpPCIE_EP_regs->MSI_CAP = (gpPCIE_EP_regs->MSI_CAP&
			(~(CSL_PCIE_CFG_SPACE_ENDPOINT_MSI_CAP_64BIT_EN_MASK
				|CSL_PCIE_CFG_SPACE_ENDPOINT_MSI_CAP_MULT_MSG_CAP_MASK)))
			|(1<<CSL_PCIE_CFG_SPACE_ENDPOINT_MSI_CAP_MSI_EN_SHIFT)
			|(address_width<<CSL_PCIE_CFG_SPACE_ENDPOINT_MSI_CAP_64BIT_EN_SHIFT)
			|(interrupt_cfg->number_tx_MSI<<CSL_PCIE_CFG_SPACE_ENDPOINT_MSI_CAP_MULT_MSG_CAP_SHIFT);
	}
			
}

/*PCIE initialization*/
void KeyStone_PCIE_Init(KeyStone_PCIE_Config * pcie_cfg)
{
	//soft reset PCIE if it is already in use
	if ((CSL_PSC_getPowerDomainState(CSL_PSC_PD_PCIEX) == PSC_PDSTATE_ON) &&
	   		(CSL_PSC_getModuleState (CSL_PSC_LPSC_PCIEX) == PSC_MODSTATE_ENABLE))
	{
		// Disable PCIE before reconfiguring
		KeyStone_PCIE_soft_reset();	//soft reset PCIE if it is already enabled
	}

	//enable PCIE power and clock domain
	KeyStone_enable_PSC_module(CSL_PSC_PD_PCIEX, CSL_PSC_LPSC_PCIEX);

	/*The Bootcfg module contains a kicker mechanism to prevent spurious writes 
	from changing any of the Bootcfg MMR (memory mapped registers) values. 
	When the kicker is locked (which it is initially after power on reset), 
	none of the Bootcfg MMRs are writable (they are only readable). 
	This mechanism requires an MMR write to each of the KICK0 and KICK1 registers 
	with exact data values before the kicker lock mechanism is unlocked.*/
	CSL_BootCfgUnlockKicker();

	/*Set PCIESSMODE[1:0] in the device level register BOOT_REG0/DEVSTAT in KeyStone 1,
	or DEVCFG in KeyStone 2*/
	gpBootCfgRegs->BOOT_REG0= (gpBootCfgRegs->BOOT_REG0
 		&(~CSL_BOOTCFG_BOOT_REG0_PCIESS_MODE_MASK))
 		|(pcie_cfg->PcieMode<<CSL_BOOTCFG_BOOT_REG0_PCIESS_MODE_SHIFT);

	/*Set the DIR_SPD bit to 1 in the PL_GEN2 register during the 
	initialization can switch the PCIe link speed
	mode from Gen1 (2.5Gbps) to Gen2 (5.0Gbps).*/
 	if(5.f==pcie_cfg->serdes_cfg.linkSpeed_GHz)
 		gpPCIE_CAP_implement_regs->PL_GEN2 |= CSL_PCIE_CFG_SPACE_ENDPOINT_PL_GEN2_DIR_SPD_MASK;
 	else
 		gpPCIE_CAP_implement_regs->PL_GEN2 &= ~CSL_PCIE_CFG_SPACE_ENDPOINT_PL_GEN2_DIR_SPD_MASK;

	/*Set the number of lanes be used*/
 	gpPCIE_CAP_implement_regs->PL_GEN2 = (gpPCIE_CAP_implement_regs->PL_GEN2&(~(CSL_PCIE_CFG_SPACE_ENDPOINT_PL_GEN2_LN_EN_MASK)))
		|(pcie_cfg->serdes_cfg.numLanes<<CSL_PCIE_CFG_SPACE_ENDPOINT_PL_GEN2_LN_EN_SHIFT);

 	if(pcie_cfg->serdes_cfg.tx_cfg)
	{
	 	gpPCIE_CAP_implement_regs->PL_GEN2 = (gpPCIE_CAP_implement_regs->PL_GEN2&(~(CSL_PCIE_CFG_SPACE_ENDPOINT_PL_GEN2_CFG_TX_SWING_MASK
	 		|CSL_PCIE_CFG_SPACE_ENDPOINT_PL_GEN2_DEEMPH_MASK)))
			|(pcie_cfg->serdes_cfg.tx_cfg->swing<<CSL_PCIE_CFG_SPACE_ENDPOINT_PL_GEN2_CFG_TX_SWING_SHIFT)
			|(pcie_cfg->serdes_cfg.tx_cfg->EP_de_emphasis<<CSL_PCIE_CFG_SPACE_ENDPOINT_PL_GEN2_DEEMPH_SHIFT);

		gpPCIE_CAP_implement_regs->LINK_CTRL2= (gpPCIE_CAP_implement_regs->LINK_CTRL2&(~(CSL_PCIE_CFG_SPACE_ENDPOINT_LINK_CTRL2_DE_EMPH_MASK
	 		|CSL_PCIE_CFG_SPACE_ENDPOINT_LINK_CTRL2_TX_MARGIN_MASK|CSL_PCIE_CFG_SPACE_ENDPOINT_LINK_CTRL2_SEL_DEEMPH_MASK)))
			|(pcie_cfg->serdes_cfg.tx_cfg->de_emphasis<<CSL_PCIE_CFG_SPACE_ENDPOINT_LINK_CTRL2_DE_EMPH_SHIFT)
			|(pcie_cfg->serdes_cfg.tx_cfg->tx_margin<<CSL_PCIE_CFG_SPACE_ENDPOINT_LINK_CTRL2_TX_MARGIN_SHIFT)
			|(pcie_cfg->serdes_cfg.tx_cfg->EP_5G_de_emphasis<<CSL_PCIE_CFG_SPACE_ENDPOINT_LINK_CTRL2_SEL_DEEMPH_SHIFT);
	}
	
	/*configure Serdes loopback mode according to PCIE loopback mode*/
	if((PCIE_PHY_LOOPBACK==pcie_cfg->loop_mode))
	{
		pcie_cfg->serdes_cfg.loopBack = SERDES_LOOPBACK_ENABLE;
	}
	else
	{
		pcie_cfg->serdes_cfg.loopBack = SERDES_LOOPBACK_DISABLE;
	}

	KeyStone_PCIE_Serdes_init(&pcie_cfg->serdes_cfg);
	Wait_PCIE_PLL_Lock();

	/*Disable link training by de-asserting the LTSSM_EN bit in the PCIESS
	Command Status Register (CMD_STATUS[LTSSM_EN]=0). Upon reset, the
	LTSSM_EN is de-asserted automatically by hardware.*/
	gpPCIE_app_regs->CMD_STATUS &= ~CSL_PCIESS_APP_CMD_STATUS_LTSSM_EN_MASK;

	if(pcie_cfg->bCommon_clock)
		gpPCIE_CAP_implement_regs->LINK_STAT_CTRL |= 
			CSL_PCIE_CFG_SPACE_ENDPOINT_LINK_STAT_CTRL_COMMON_CLK_CFG_MASK;

	KeyStone_PCIE_Inbound_Memory_Region_Init(pcie_cfg);

	KeyStone_PCIE_Outbound_Memory_Region_Init(pcie_cfg->outbound_memory_regions);

	KeyStone_PCIE_Internal_Bus_Init(pcie_cfg->bus_cfg);

	KeyStone_PCIE_Interrupt_Init(pcie_cfg->interrupt_cfg, pcie_cfg->address_width);

	//RC specific configuration
	if(pcie_cfg->rc_cfg)//only valid for RC mode
		KeyStone_PCIE_RC_Init(pcie_cfg->rc_cfg, pcie_cfg->address_width);

	/*Initiate link training can be initiated by asserting LTSSM_EN bit in the
	CMD_STATUS register (CMD_STATUS[LTSSM_EN]=1).*/
	gpPCIE_app_regs->CMD_STATUS |= CSL_PCIESS_APP_CMD_STATUS_LTSSM_EN_MASK;

	puts("PCIE start link training...");

	if((PCIE_PHY_LOOPBACK == pcie_cfg->loop_mode)&&
	(LTSSM_STAT_L0 != ((gpPCIE_CAP_implement_regs->DEBUG0
		&CSL_PCIE_CFG_SPACE_ENDPOINT_DEBUG0_LTSSM_STATE_MASK)
		>>CSL_PCIE_CFG_SPACE_ENDPOINT_DEBUG0_LTSSM_STATE_SHIFT)))
	{
		/*DETECT state (0 and 1) should be skipped in loopback mode 
		by forcing link state to start with POLL_ACTIVE state (2) */
		gpPCIE_CAP_implement_regs->PL_FORCE_LINK |= 
			CSL_PCIE_CFG_SPACE_ENDPOINT_PL_FORCE_LINK_FORCE_LINK_MASK
			|(LTSSM_STAT_POLL_ACTIVE
			 <<CSL_PCIE_CFG_SPACE_ENDPOINT_PL_FORCE_LINK_LNK_STATE_SHIFT);
	}

	/*Insure link training completion and success by observing LTSSM_STATE field in
	DEBUG0 register change to 0x11.*/
	while(LTSSM_STAT_L0 != ((gpPCIE_CAP_implement_regs->DEBUG0
		&CSL_PCIE_CFG_SPACE_ENDPOINT_DEBUG0_LTSSM_STATE_MASK)
		>>CSL_PCIE_CFG_SPACE_ENDPOINT_DEBUG0_LTSSM_STATE_SHIFT));

	puts("PCIE link training is finished.");
}

/*Get one BAR mask and properties.
Please note that the BAR Mask registers are writable but not readable. 
The read back value will be the BAR registers values instead of BAR Mask 
registers. In order to get the BAR Mask registers. we can write test
pattern into BAR registers, for example, if BAR is 32-bit BAR, Mask 
register = 0x00003FFF, write parttern 0xFFFFFFF0 to the BAR, then BAR 
should be read back as read as 0xFFFFC000, as the lower bits of BAR 
have been masked.
return 1 if valid BAR is read, otherwise, return 0*/
int KeyStone_PCIE_Get_one_BAR_Mask(volatile Uint32 * BAR_address, 
	PCIE_BAR_Config * bar_cfg)
{
	Uint32 uiBAR_value, uiBAR_type;

	uiBAR_value= *BAR_address;

	bar_cfg->bPrefetchable= (uiBAR_value
		&CSL_PCIE_CFG_SPACE_ROOTCOMPLEX_BAR_PREFETCHABLE_MASK)
		>>CSL_PCIE_CFG_SPACE_ROOTCOMPLEX_BAR_PREFETCHABLE_SHIFT;

	bar_cfg->bIO= (uiBAR_value
		&CSL_PCIE_CFG_SPACE_ROOTCOMPLEX_BAR_MEMORY_SPACE_MASK)
		>>CSL_PCIE_CFG_SPACE_ROOTCOMPLEX_BAR_MEMORY_SPACE_SHIFT;

	uiBAR_type= (uiBAR_value
		&CSL_PCIE_CFG_SPACE_ROOTCOMPLEX_BAR_TYPE_MASK)
		>>CSL_PCIE_CFG_SPACE_ROOTCOMPLEX_BAR_TYPE_SHIFT;
	if(0==uiBAR_type)
	{
		bar_cfg->address_width= PCIE_ADDRESS_32_BITS;

		*BAR_address = uiBAR_value|0xFFFFFFF0;

		bar_cfg->ullMask= ~(*BAR_address&0xFFFFFFF0);

	}
	else if(2==uiBAR_type)
	{
		bar_cfg->address_width= PCIE_ADDRESS_64_BITS;

		*BAR_address = uiBAR_value|0xFFFFFFF0;
		*(BAR_address+1) = 0xFFFFFFFF;

		bar_cfg->ullMask= ~(_itoll(*(BAR_address+1), *BAR_address&0xFFFFFFF0));
	}
	else 	//invalid
		return 0; 
	
	if(bar_cfg->ullMask<0x10) //invalid BAR
		return 0;

	return 1;
}

/*allocate PCIE address for one BAR
return 1 for success, return 0 for fail*/
int KeyStone_PCIE_RC_Allocate_one_BAR_Address(PCIE_BAR_Config * bar_cfg, 
	PCIE_RC_Config * rc_cfg)
{
	unsigned long long ullSize= bar_cfg->ullMask+1;

	if(ullSize<16)
	{
		puts("BAR size smaller than 16 is invalide!");
		return 0;
	}

	/*simply sequenctially allocate free address to BARs*/
	if(bar_cfg->bPrefetchable)
	{
		if(gullFreePrefetchPcieAddress >= rc_cfg->prefetch_memory_limit)
		{
			puts("no free PCIE address sapce for allocation!");
			return 0;
		}

		//align the stat address to BAR size boundary
		gullFreePrefetchPcieAddress &= ~bar_cfg->ullMask;
		bar_cfg->ullStart_address= gullFreePrefetchPcieAddress;

		//increment free address for next allocation
		gullFreePrefetchPcieAddress+= ullSize;
	}
	else
	{
		if(gullFreeNonfetchPcieAddress >= rc_cfg->memory_limit)
		{
			puts("no free PCIE address sapce for allocation!");
			return 0;
		}

		//align the stat address to BAR size boundary
		gullFreeNonfetchPcieAddress &= ~bar_cfg->ullMask;
		bar_cfg->ullStart_address= gullFreeNonfetchPcieAddress;

		//increment free address for next allocation
		gullFreeNonfetchPcieAddress+= ullSize;

	}

	return 1;
}

/*write the PCIE start address to the BAR register*/
void KeyStone_PCIE_Set_one_BAR_Address(volatile Uint32 * BAR_address, 
	PCIE_BAR_Config * bar_cfg)
{
	*BAR_address = _loll(bar_cfg->ullStart_address)
		|(bar_cfg->bPrefetchable<<CSL_PCIE_CFG_SPACE_ROOTCOMPLEX_BAR_PREFETCHABLE_SHIFT)
		|(bar_cfg->address_width<<(1+CSL_PCIE_CFG_SPACE_ROOTCOMPLEX_BAR_TYPE_SHIFT))
		|(bar_cfg->bIO<<CSL_PCIE_CFG_SPACE_ROOTCOMPLEX_BAR_MEMORY_SPACE_SHIFT);

	if(PCIE_ADDRESS_64_BITS == bar_cfg->address_width)
	{
		*(BAR_address+1) = _hill(bar_cfg->ullStart_address);
	}
}

/*allocate and setup outbound regions for one BAR*/
void KeyStone_PCIE_RC_Setup_OB_for_one_BAR(PCIE_BAR_Config * bar_cfg, 
	PCIE_Outbound_Region_Size OB_size)
{
	int i;
	Uint32 uiNumRegions;
	Uint32 uiRegionSize; //size of one region in byte
	unsigned long long ullBarSize= bar_cfg->ullMask+1; //BAR size
	unsigned long long ull_PCIE_address= bar_cfg->ullStart_address;//ull_PCIE_address is EP's bar address

	//convert region size enum type to size in bytes
	uiRegionSize= 1<<(OB_size+20);

	/*calculate number of outbound regions need for this BAR.
	Since the BAR size may be larger than one outbound regions size, so,
	multiple outbound regions may be allocated for one BAR*/
	if(ullBarSize<uiRegionSize)
		uiNumRegions= 1;
	else
		uiNumRegions= (ullBarSize>>(OB_size+20));

	for(i=0; i<uiNumRegions; i++)
	{
		if(uiOutboundRegionIndex>=32)
		{
			puts("outbound only has 32 regions, more region request is ignored!");
			return;
		}

		gpPCIE_app_regs->OUTBOUND_TRANSLATION[uiOutboundRegionIndex].OB_OFFSET_INDEX=
			CSL_PCIESS_APP_OB_OFFSET_INDEX_OB_ENABLE_MASK
			|_loll(ull_PCIE_address);
		gpPCIE_app_regs->OUTBOUND_TRANSLATION[uiOutboundRegionIndex].OB_OFFSET_HI=
			_hill(ull_PCIE_address);

		//increment PCIE address for next region
		ull_PCIE_address+= uiRegionSize;
		
		uiOutboundRegionIndex++;
	}
	
}


/*Get BARs mask and properties, allocate address, and setup the BAR PCIE address
return number of valid BAR read*/
int KeyStone_PCIE_RC_Allocate_one_device_Address(KeyStone_PCIE_Config * pcie_cfg,
	volatile Uint32 * BAR_regs, Uint32 num_BAR)
{
	PCIE_BAR_Config bar_cfg[6];
	Uint32 uiBAR_reg_index;
	Uint32 uiValid_BAR_cnt= 0;
	
	for(uiBAR_reg_index=0;uiBAR_reg_index<num_BAR;uiBAR_reg_index++)
	{
		if(KeyStone_PCIE_Get_one_BAR_Mask(
			&BAR_regs[uiBAR_reg_index], &bar_cfg[uiValid_BAR_cnt]))
		{
			/*allocate PCIE address for one BAR*/
			if(KeyStone_PCIE_RC_Allocate_one_BAR_Address(&bar_cfg[uiValid_BAR_cnt], 
				pcie_cfg->rc_cfg))
			{
				/*write the PCIE start address to the BAR register*/
				KeyStone_PCIE_Set_one_BAR_Address(
					&BAR_regs[uiBAR_reg_index],&bar_cfg[uiValid_BAR_cnt]);
				
				/*allocate and setup outbound regions for one remote BAR*/
				KeyStone_PCIE_RC_Setup_OB_for_one_BAR(&bar_cfg[uiValid_BAR_cnt],
					pcie_cfg->outbound_memory_regions->OB_size);
			}

			if(bar_cfg[uiValid_BAR_cnt].address_width==PCIE_ADDRESS_64_BITS)
				uiBAR_reg_index++;

			uiValid_BAR_cnt++;
		}
	}

	return uiValid_BAR_cnt;
}

//allocate PCIE address for devices
void KeyStone_PCIE_RC_Allocate_devices_Address(KeyStone_PCIE_Config * pcie_cfg)
{
	//enumeration all devices in the sytem
	//to do

	//initialize the free outbound region index
	uiOutboundRegionIndex= 0;

	//for(all devices in the system)
	{
		if(PCIE_PHY_LOOPBACK==pcie_cfg->loop_mode)
		{
			/*Loopback only support by RC mode. There is only 2 BARs in RC mode.
			BAR[0] is fixed for application register mapping, so only BAR[1] is configurable.*/
			gpPCIE_app_regs->OUTBOUND_TRANSLATION[uiOutboundRegionIndex].OB_OFFSET_INDEX=
				CSL_PCIESS_APP_OB_OFFSET_INDEX_OB_ENABLE_MASK
				|pcie_cfg->rc_cfg->BAR0_address;
			gpPCIE_app_regs->OUTBOUND_TRANSLATION[uiOutboundRegionIndex].OB_OFFSET_HI= 0;

			//increment outbound region index for next region
			uiOutboundRegionIndex++;

			/*It is not possible to use configuration type transactions in loopback 
			mode as RC cannot be a target of configuration transactions. Such 
			transactions will be invalid and loopback will not work.
			So, in loopback mode, RC directly read the BAR register through local BUS*/
			KeyStone_PCIE_RC_Allocate_one_device_Address(pcie_cfg, &gpPCIE_RC_regs->BAR[1], 1);
		}
		else
		{
			/*allocate PCIE device address (via the PCI Express initialization procedure).
			The 6 BAR registers of EP are accessed through configuration space*/
			KeyStone_PCIE_RC_Allocate_one_device_Address(pcie_cfg, gpPCIE_remote_EP_regs->BAR, 6);
		}
	}

}

/*setup inbound address translation regions for one BAR*/
void KeyStone_PCIE_Inbound_Address_translation(
PCIE_Memory_Regions * regions, Uint32 uiBAR_index, 
CSL_Pciess_appInbound_translationRegs * inboundTranslationRegs, 
PCIE_Address_Width address_width, KeyStone_PCIE_Config * pcie_cfg)
{
	int i;
	unsigned long long ullPCIE_address;

	if(PCIE_EP_MODE==pcie_cfg->PcieMode)
	{
		/*read the PCIE address in BAR register for the regions, 
		which should have been setup by RC during enumeration*/
		do{
			if(PCIE_ADDRESS_32_BITS==address_width)
			{
				ullPCIE_address = gpPCIE_EP_regs->BAR[uiBAR_index]
					&CSL_PCIE_CFG_SPACE_ENDPOINT_BAR_BASE_ADDRESS_MASK;
			}
			else
			{
				ullPCIE_address = _itoll(gpPCIE_EP_regs->BAR[uiBAR_index+1],
					gpPCIE_EP_regs->BAR[uiBAR_index]
					&CSL_PCIE_CFG_SPACE_ENDPOINT_BAR_BASE_ADDRESS_MASK);
			}
		}while(0==ullPCIE_address);
	}
	else
	{	/*read the PCIE address in BAR register for the regions, 
		which should have been setup by RC during enumeration*/
		do{
			if(PCIE_ADDRESS_32_BITS==address_width)
			{
				ullPCIE_address = gpPCIE_RC_regs->BAR[uiBAR_index]
					&CSL_PCIE_CFG_SPACE_ROOTCOMPLEX_BAR_BASE_ADDRESS_MASK;
			}
			else
			{
				puts("RC inbound BAR only can support 32bit width!");
			}
	      }while(0==ullPCIE_address);
	}

	/*The PCIE address in BAR is 0 at initialization,
	the RC may write 0xFFFFFFFF into the BAR to test the size of the BAR,
	and then, RC should setup allocated address in the BAR register*/
	if((~regions->ullBAR_Mask)==ullPCIE_address)
	{//the BAR is beining testing and configuring by the RC
		TSC_delay_us(1000);	//wait for RC configuration complete
		if(PCIE_EP_MODE==pcie_cfg->PcieMode)
		{
			//read the PCIE address of the BAR
			if(PCIE_ADDRESS_32_BITS==address_width)
			{
				ullPCIE_address = gpPCIE_EP_regs->BAR[uiBAR_index]
					&CSL_PCIE_CFG_SPACE_ENDPOINT_BAR_BASE_ADDRESS_MASK;
			}
			else
			{
				ullPCIE_address = _itoll(gpPCIE_EP_regs->BAR[uiBAR_index+1],
					gpPCIE_EP_regs->BAR[uiBAR_index]
					&CSL_PCIE_CFG_SPACE_ENDPOINT_BAR_BASE_ADDRESS_MASK);
			}
		}
		else
		{
			//read the PCIE address of the BAR
			if(PCIE_ADDRESS_32_BITS==address_width)
			{
				ullPCIE_address = gpPCIE_RC_regs->BAR[uiBAR_index]
					&CSL_PCIE_CFG_SPACE_ROOTCOMPLEX_BAR_BASE_ADDRESS_MASK;
			}
			else
			{
					puts("RC inbound BAR only can support 32bit width!");
			}
		}
	}

	for(i=0; i< regions->uiNumRegions; i++)
	{
		inboundTranslationRegs[i].IB_BAR= uiBAR_index;

		inboundTranslationRegs[i].IB_START_LO= _loll(ullPCIE_address);
		inboundTranslationRegs[i].IB_START_HI= _hill(ullPCIE_address);

		inboundTranslationRegs[i].IB_OFFSET= 
			regions->memory_regions[i].uiTargetAddress;

		ullPCIE_address+= regions->memory_regions[i].uiNumBytes;
	}
			
}

/*setup inbound address translation regions*/
void KeyStone_PCIE_Inbound_Address_Setup(
	KeyStone_PCIE_Config * pcie_cfg)
{
	Uint32 uiBAR_index;
	Uint32 uiNum_prefetch_regions;
	Uint32 uiNum_nonfetch_regions;
	PCIE_Memory_Regions * prefetch_regions; 
	PCIE_Memory_Regions * nonfetch_regions;
	PCIE_Address_Width address_width= pcie_cfg->address_width;

	if(NULL==pcie_cfg->inbound_memory_regions)
		return;

	prefetch_regions= pcie_cfg->inbound_memory_regions->prefetch_regions;
	nonfetch_regions= pcie_cfg->inbound_memory_regions->nonfetch_regions;
	if(prefetch_regions)
		uiNum_prefetch_regions= prefetch_regions->uiNumRegions;
	else
		uiNum_prefetch_regions= 0;
	if(nonfetch_regions)
		uiNum_nonfetch_regions= nonfetch_regions->uiNumRegions;
	else
		uiNum_nonfetch_regions= 0;

	/*BAR0 (and BAR1 for 64-bit mode) is reserved for accessing PCIE registers*/
	if(PCIE_ADDRESS_32_BITS==address_width)
		uiBAR_index = 1; //first memory BAR is BAR1
	else
	{
		if(PCIE_RC_MODE==pcie_cfg->PcieMode)
			return; 	//RC 64-bit mode does not have data BAR
			
		uiBAR_index = 2; //first memory BAR is BAR2
	}

	/*setup prefetchable regions*/
	if(uiNum_prefetch_regions)
	{
		KeyStone_PCIE_Inbound_Address_translation(prefetch_regions,	uiBAR_index, 
			&gpPCIE_app_regs->INBOUND_TRANSLATION[0], address_width, pcie_cfg);

		if(PCIE_RC_MODE==pcie_cfg->PcieMode)
			return; 	//RC mode only has one data BAR
			
		//increment BAR index for non-prefetch regions
		if(PCIE_ADDRESS_32_BITS==address_width)
			uiBAR_index = 2;
		else
			uiBAR_index = 4;
	}

	/*setup non-prefetchable regions*/
	if(uiNum_nonfetch_regions)
	{
		KeyStone_PCIE_Inbound_Address_translation(prefetch_regions,	uiBAR_index, 
			&gpPCIE_app_regs->INBOUND_TRANSLATION[uiNum_prefetch_regions],
			address_width, pcie_cfg);
	}
}

/*PCIE address allocation (via the PCI Express enumeration procedure).
setup outbound and inbound address mapping*/
void KeyStone_PCIE_Address_setup(KeyStone_PCIE_Config * pcie_cfg)
{
	PCIE_Outbound_Memory_Regions * outbound_memory_regions;

	outbound_memory_regions= pcie_cfg->outbound_memory_regions;

	/*if outbound_memory_regions is used, and it is not setup manually, 
	and PCIE is in RC mode, the outbound memory regions should be setup via the 
	PCI Express enumeration procedure*/
	if(PCIE_RC_MODE==pcie_cfg->PcieMode)
	{
		if(outbound_memory_regions)
		{
			/*there is NO input parameters to maually setup outbound regions*/
			if(0==outbound_memory_regions->uiNumRegions)
			{
				KeyStone_PCIE_RC_Allocate_devices_Address(pcie_cfg);
			}
		}

		/*BAR0 is mapped to RC application registers */
		gpPCIE_RC_regs->BAR[0]= pcie_cfg->rc_cfg->BAR0_address;
		
		if(PCIE_PHY_LOOPBACK == pcie_cfg->loop_mode)
		{
			//setup inbound address regions after the BAR is setup by RC
			KeyStone_PCIE_Inbound_Address_Setup(pcie_cfg);
		}
	}
	else if(PCIE_EP_MODE==pcie_cfg->PcieMode)
	{
		//setup inbound address regions after the BAR is setup by RC
		KeyStone_PCIE_Inbound_Address_Setup(pcie_cfg);
	}
}

/*Remote Configuration Transaction Setup,
select the bus, device and function number of the target*/
void KeyStone_PCIE_remote_CFG_setup(PCIE_Remote_CFG_SETUP * cfg_setup)
{
	gpPCIE_app_regs->CFG_SETUP= 
		(cfg_setup->config_type<<CSL_PCIESS_APP_CFG_SETUP_CFG_TYPE_SHIFT)
		|(cfg_setup->config_bus<<CSL_PCIESS_APP_CFG_SETUP_CFG_BUS_SHIFT)
		|(cfg_setup->config_device<<CSL_PCIESS_APP_CFG_SETUP_CFG_DEVICE_SHIFT)
		|(cfg_setup->config_function<<CSL_PCIESS_APP_CFG_SETUP_CFG_FUNC_SHIFT);
}

Uint32 uiMSI_index= 0; //index of the next free MSI vector
/*PCIE MSI allocation for one device.
msi_regs: pointer to remote configuration space. For loopback test, pointer to local bus space
MSI_address: PCIE address for EP to write to generate MSI*/
void KeyStone_PCIE_RC_MSI_allocate(PCIE_MSI_Regs * msi_regs,
	unsigned long long PCIE_MSI_address)
{
	Uint32 MSI_CAP, uiNum_MSI_requested, uiNum_MSI_allocated;
	Uint32 MULT_MSG_EN;

	MSI_CAP= msi_regs->MSI_CAP;
	if(0==(MSI_CAP&CSL_PCIE_CFG_SPACE_ENDPOINT_MSI_CAP_MSI_EN_MASK))
		return; //MSI is not enabled in this device

	if(uiMSI_index>=32)
	{
		puts("all 32 MSI vector are used, no free MSI be allocated for this device!");
		msi_regs->MSI_CAP= MSI_CAP&(~CSL_PCIE_CFG_SPACE_ENDPOINT_MSI_CAP_MSI_EN_MASK);
		return;
	}

	//read number of the MSI vectors requested by EP
	uiNum_MSI_requested= 1<<((msi_regs->MSI_CAP
		&CSL_PCIE_CFG_SPACE_ENDPOINT_MSI_CAP_MULT_MSG_CAP_MASK)
		>>CSL_PCIE_CFG_SPACE_ENDPOINT_MSI_CAP_MULT_MSG_CAP_SHIFT);

	if(uiMSI_index+uiNum_MSI_requested>=32)
		uiNum_MSI_allocated= 32- uiMSI_index;
	else
		uiNum_MSI_allocated= uiNum_MSI_requested;

	//align to power of 2
	MULT_MSG_EN= 31 - _lmbd(1, uiNum_MSI_allocated);

	//write the number of the allocated vectors for the EP
	msi_regs->MSI_CAP= (MSI_CAP&(~CSL_PCIE_CFG_SPACE_ENDPOINT_MSI_CAP_MULT_MSG_EN_MASK))
		|(MULT_MSG_EN<<CSL_PCIE_CFG_SPACE_ENDPOINT_MSI_CAP_MULT_MSG_EN_SHIFT);

	//write the PCIE address for EP to write to generate MSI
	msi_regs->MSI_LOW32= _loll(PCIE_MSI_address);
	msi_regs->MSI_UP32 = _hill(PCIE_MSI_address);	

	uiNum_MSI_allocated= 1<<MULT_MSG_EN;

	//align MSI index to the power of 2 boundary
	if(uiMSI_index&(uiNum_MSI_allocated-1))
	{
		uiMSI_index= (uiMSI_index+uiNum_MSI_allocated)&(~(uiNum_MSI_allocated-1));
	}

	//write the index of the first MSI allocation for the EP
	msi_regs->MSI_DATA= uiMSI_index;	
}

/*generate one MSI interrupt through PCIE.
MSI_number: 0~31
MSI_address: the remapped address in PCIE window that should be written to generate the MSI*/
void KeyStone_PCIE_generate_MSI(Uint32 MSI_number, Uint32 * MSI_address)
{
	Uint32 MSI_data;
	Uint32 max_MSI;

	max_MSI= 1<<((gpPCIE_EP_regs->MSI_CAP
		&CSL_PCIE_CFG_SPACE_ENDPOINT_MSI_CAP_MULT_MSG_EN_MASK)
		>>CSL_PCIE_CFG_SPACE_ENDPOINT_MSI_CAP_MULT_MSG_EN_SHIFT);
	if(MSI_number>=max_MSI)
	{
		printf("Error: MSI number %d >= allocated MAX number %d\n",
			MSI_number, max_MSI);
		return;
	}

	MSI_data= gpPCIE_EP_regs->MSI_DATA&(~(max_MSI-1))|MSI_number;

	*MSI_address= MSI_data;
}

//clear all interrupt status registers of PCIE
void KeyStone_PCIE_clear_interrupts()
{
	int i;

	for(i=0;i<8;i++)
		gpPCIE_app_regs->MSIX_IRQ[i].MSI_IRQ_STATUS= 0xF;

	gpPCIE_app_regs->ERR_IRQ_STATUS= 0x2F;

	gpPCIE_app_regs->PMRST_IRQ_STATUS= 0xF;
}

