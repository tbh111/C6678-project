/******************************************************************************

  Copyright (C), 2013, Texas Instrument.

 ******************************************************************************
  File Name     : KeyStone_GE_Init_drv.c
  Version       : Initial Draft
  Author        : Kevin Cai
  Created       : July 3, 2013
  Last Modified :
  Description   : example for Gigbit ethernet configuration and 
                  transfer driver on KeyStone DSP

  History       :
  1.Date        : July 3, 2013
    Author      : Kevin Cai
    Modification: Created file
  2.Date        : August 25, 2013
    Author      : Brighton Feng
    Modification: Update SGMII Serdes configuration
  3.Date        : Dec 8, 2014
    Author      : Brighton Feng
    Modification: fixed a SGMII_STATUS check bug, and a MDIO status check bug
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <csl_cpsw_3gfAux.h>
#include <csl_cpgmac_slAux.h>
#include <csl_pscAux.h>
#include "KeyStone_common.h"
#include "KeyStone_Navigator_init_drv.h"
#include "KeyStone_GE_Init_drv.h"

CSL_CpsgmiiRegs *  gpSGMII_regs[2] = 
{
	(CSL_CpsgmiiRegs *)(CSL_PA_SS_CFG_REGS + 0x00090100),
	(CSL_CpsgmiiRegs *)(CSL_PA_SS_CFG_REGS + 0x00090200)
};

CSL_Mdio_Regs *  gpMDIO_regs = ((CSL_Mdio_Regs *) (CSL_PA_SS_CFG_REGS + 0x00090300));
CSL_Cpsw_3gfRegs *  gpCPSW_regs = ((CSL_Cpsw_3gfRegs *) (CSL_PA_SS_CFG_REGS + 0x00090800));
CSL_Cpsw_3gfPort_info_groupRegs *  gpCPSW_port_regs[2] = 
{
	(CSL_Cpsw_3gfPort_info_groupRegs *)(CSL_PA_SS_CFG_REGS + 0x00090860),
	(CSL_Cpsw_3gfPort_info_groupRegs *)(CSL_PA_SS_CFG_REGS + 0x00090890)
};
CSL_Cpgmac_slRegs *  gpMAC_regs[2] = 
{
	(CSL_Cpgmac_slRegs *)(CSL_PA_SS_CFG_REGS + 0x00090900),
	(CSL_Cpgmac_slRegs *)(CSL_PA_SS_CFG_REGS + 0x00090940)
};

/*STATA and STATB */
CSL_Cpsw_3gfPort_stats_groupRegs * gpStats_regs[2]=
{
	(CSL_Cpsw_3gfPort_stats_groupRegs *)(CSL_PA_SS_CFG_REGS + 0x00090B00),
	(CSL_Cpsw_3gfPort_stats_groupRegs *)(CSL_PA_SS_CFG_REGS + 0x00090C00)
};

CSL_Pa_ssRegs * gpNetCP_regs = (CSL_Pa_ssRegs *)CSL_PA_SS_CFG_REGS;

//accumulation of the statistics values
Ethernet_Statistics statistics[GE_NUM_ETHERNET_PORT];

//enable stauts of statistics modules
Bool STATS_EN[2]= {FALSE, FALSE};

SerdesRegs * sgmiiSerdesRegs;

/*Giga bit ethernet switch subsystem reset*/
void KeyStone_GE_soft_reset()
{
	int i;
	
	if ((CSL_PSC_getModuleState (CSL_PSC_LPSC_PKTPROC) == PSC_MODSTATE_ENABLE))
	{
		/*shut down packet DMA transaction*/
		for(i=0; i<GE_NUM_ETHERNET_PORT; i++)
			KeyStone_pktDma_RxCh_teardown(gpNetCP_DMA_RxChCfgRegs,
				GE_DIRECT_RX_PORT1_CHANNEL+i, PKTDMA_WAIT_FOREVER);
		KeyStone_pktDma_TxCh_teardown(gpNetCP_DMA_TxChCfgRegs, GE_DIRECT_TX_CHANNEL, PKTDMA_WAIT_FOREVER);
	
		for(i=0; i<GE_NUM_ETHERNET_PORT; i++)
		{
		    /* Reset the SGMII port*/
			gpSGMII_regs[i]->SOFT_RESET_REG = CSL_CPSGMII_SOFT_RESET_REG_SOFT_RESET_MASK;

		    /* Reset MAC port*/            
		    CSL_CPGMAC_SL_resetMac (i);
			//wati for EMAC reset complete
		    while (CSL_CPGMAC_SL_isMACResetDone (i) != TRUE);
		}
	}
		
	/*Wait to finish any current DMA transfer.*/
	TSC_delay_ms(50);

	//disable Serdes
	KeyStone_Serdes_disable(sgmiiSerdesRegs, GE_NUM_ETHERNET_PORT);
	
	//disable CPSW and Packet DMA (in PA) through PSC
	//KeyStone_disable_PSC_module(CSL_PSC_PD_PASS, CSL_PSC_LPSC_CPGMAC);
	//KeyStone_disable_PSC_module(CSL_PSC_PD_PASS, CSL_PSC_LPSC_PKTPROC);
	KeyStone_disable_PSC_Power_Domain(CSL_PSC_PD_PASS);
}


/*address look up table initialize*/
void KeyStone_GE_ALE_Init(Ethernet_ALE_Config *   ale_cfg)
{
	int i;
	Uint32 uiALE_entry_index;

	/*Note: Even you do not want to use the features of ALE, ALE should always 
	be enabled and port should be enabled to forward packet. Otherwise, all
	packets to ALE ports will be dropped.
	If you do not need the features of ALE, you should enable ALE and ports and
	enable the BYPASS mode*/

	/*set port state as forward*/
	for(i=0; i< GE_NUM_ETHERNET_PORT+1; i++)
		gpCPSW_regs->ALE_PORT_CONTROL_REG[i]= 
			ALE_PORTSTATE_FORWARD<<CSL_CPSW_3GF_ALE_PORT_CONTROL_REG_PORT_STATE_SHIFT;

	if(NULL==ale_cfg)
	{
		//no ALE entries, set ALE in bypass mode
		gpCPSW_regs->ALE_CONTROL_REG = CSL_CPSW_3GF_ALE_CONTROL_REG_ENABLE_ALE_MASK
			|CSL_CPSW_3GF_ALE_CONTROL_REG_ALE_BYPASS_MASK;
		return;
	}

	/*The input clock is divided by this value for use in the 
	multicast/broadcast rate limiters. The minimum operating value 
	is 10h. The prescaler is off when the value is zero.*/
	gpCPSW_regs->ALE_PRESCALE_REG= 0x1e848;

	/*Clear ALE address table. Setting this bit causes the ALE hardware to write 
	all table bit values to zero. Software must perform a clear table operation 
	as part of the ALE setup/configuration process. Setting this bit causes all 
	ALE accesses to be held up for 64 clocks while the clear is performed. 
	Access to all ALE registers will be blocked (wait states) until the 64 clocks 
	have completed. This bit cannot be read as one because the read is blocked 
	until the clear table is completed at which time this bit is cleared to zero.*/
	gpCPSW_regs->ALE_CONTROL_REG = CSL_CPSW_3GF_ALE_CONTROL_REG_ENABLE_ALE_MASK
		|CSL_CPSW_3GF_ALE_CONTROL_REG_CLEAR_TABLE_MASK;

	uiALE_entry_index= 0;

	for(i=0; i< ale_cfg->num_multicastEntries; i++)
	{
		CSL_CPSW_3GF_setAleMcastAddrEntry(uiALE_entry_index,
			&ale_cfg->multicastEntries[i]);
		uiALE_entry_index++;
	}
	
	for(i=0; i< ale_cfg->num_OUI_entries; i++)
	{
		CSL_CPSW_3GF_setAleOUIAddrEntry(uiALE_entry_index,
			&ale_cfg->OUI_entries[i]);
		uiALE_entry_index++;
	}
	
	for(i=0; i< ale_cfg->num_unicastEntries; i++)
	{
		CSL_CPSW_3GF_setAleUnicastAddrEntry(uiALE_entry_index,
			&ale_cfg->unicastEntries[i]);
		uiALE_entry_index++;
	}
	
	for(i=0; i< ale_cfg->num_VLAN_entires; i++)
	{
		CSL_CPSW_3GF_setAleVlanEntry(uiALE_entry_index,
			&ale_cfg->VLAN_entires[i]);
		uiALE_entry_index++;
	}
	
	for(i=0; i< ale_cfg->num_VLAN_multicastEntries; i++)
	{
		CSL_CPSW_3GF_setAleVlanMcastAddrEntry(uiALE_entry_index,
			&ale_cfg->VLAN_multicastEntries[i]);
		uiALE_entry_index++;
	}
	
	for(i=0; i< ale_cfg->num_VLAN_UnicastEntries; i++)
	{
		CSL_CPSW_3GF_setAleVlanUnicastAddrEntry(uiALE_entry_index,
			&ale_cfg->VLAN_UnicastEntries[i]);
		uiALE_entry_index++;
	}

}

/*statistics enable*/
void KeyStone_GE_Statistics_Init(Ethernet_Port_Config * ethernet_port_cfg[])
{
	int i;
	Uint32 uiReg_value= 0;

	memset(statistics, 0, sizeof(statistics));
	STATS_EN[0]= FALSE;
	STATS_EN[1]= FALSE;

	gpCPSW_regs->STAT_PORT_EN_REG= 0; 	//disable statistics before configuration

	if(ethernet_port_cfg[0])
	{
		if(ethernet_port_cfg[0]->ethenet_port_statistics_enable)
		{
			uiReg_value |= CSL_CPSW_3GF_STAT_PORT_EN_REG_P1_STAT_EN_MASK;
			STATS_EN[1]= TRUE;
		}
		if(ethernet_port_cfg[0]->host_port_statistics_enable)
		{
			uiReg_value |= CSL_CPSW_3GF_STAT_PORT_EN_REG_P0A_STAT_EN_MASK;
			STATS_EN[0]= TRUE;
		}
	}

	if(ethernet_port_cfg[1])
	{
		if(ethernet_port_cfg[1]->ethenet_port_statistics_enable)
		{
			uiReg_value |= CSL_CPSW_3GF_STAT_PORT_EN_REG_P2_STAT_EN_MASK;
			STATS_EN[1]= TRUE;
		}
		if(ethernet_port_cfg[1]->host_port_statistics_enable)
		{
			uiReg_value |= CSL_CPSW_3GF_STAT_PORT_EN_REG_P0B_STAT_EN_MASK;
			STATS_EN[0]= TRUE;
		}
	}

	//clear statistics values
	for(i=0; i<GE_NUM_ETHERNET_PORT; i++)
	{
		/*While disabled, all statistics registers can be read and written 
		normally, so writing to 0x00000000 clears a statistics register.*/
		memset(gpStats_regs[i], 0, sizeof(CSL_CPSW_3GF_STATS));
	}

	gpCPSW_regs->STAT_PORT_EN_REG= uiReg_value;
}

/*MAC address initialization for flow control*/
void KeyStone_GE_Flow_Control_MAC_Address_Init(
	Ethernet_Port_Config * ethernet_port_cfg[])
{
	int i;
	Uint32 uiReg_value= 0;

	for(i=0; i< GE_NUM_ETHERNET_PORT; i++)
	{
		if(ethernet_port_cfg[i])	
		{
			if(ethernet_port_cfg[i]->RX_flow_control_enable||
				ethernet_port_cfg[i]->RX_flow_control_enable)
			{
				gpCPSW_port_regs[i]->SL_SA_LO_REG= 
					_hill(ethernet_port_cfg[i]->flow_control_MAC_Address);
				gpCPSW_port_regs[i]->SL_SA_HI_REG= 
					_loll(ethernet_port_cfg[i]->flow_control_MAC_Address);
			}
			
			/*setup value for gpCPSW_regs->FLOW_CONTROL_REG*/
			if(ethernet_port_cfg[i]->RX_flow_control_enable)
				uiReg_value |= (1<<(i+1));
		}
	}
	/*enable port 0 and ethernet port flow control.
	port 0 flow control should always be enabled, it will push back to 
	packet DMA to avoid it transmit too many packet to overflow the FIFO in CPSW*/
	gpCPSW_regs->FLOW_CONTROL_REG= uiReg_value
		|CSL_CPSW_3GF_FLOW_CONTROL_REG_P0_FLOW_EN_MASK;
}

/*initialize the CPPI Info Word 0 SRC_ID field for received packets*/
void KeyStone_GE_CPPI_Src_ID_Init(
	Ethernet_Port_Config * ethernet_port_cfg[])
{
	Uint32 uiReg_value= 0;

	if(ethernet_port_cfg[0])	
		uiReg_value |= (ethernet_port_cfg[0]->CPPI_Src_ID
				<<CSL_CPSW_3GF_P0_CPPI_SRC_ID_REG_TXA_SRC_ID_SHIFT);
	
	if(ethernet_port_cfg[1])	
		uiReg_value |= (ethernet_port_cfg[1]->CPPI_Src_ID
				<<CSL_CPSW_3GF_P0_CPPI_SRC_ID_REG_TXB_SRC_ID_SHIFT);

	gpCPSW_regs->P0_CPPI_SRC_ID_REG= uiReg_value;
}

/*initialize ethernet ports*/
void KeyStone_Ethernet_Ports_Init(KeyStone_GE_Config * ge_cfg)
{
	int i;
	Bool bMAC_loopback=FALSE, bFullDuplex=TRUE;
	Uint32 speed_mode= 2; //1000M by default
	
	for(i=0; i<GE_NUM_ETHERNET_PORT; i++)
	{
		if(ge_cfg->ethernet_port_cfg[i])
		{
		    /* Reset the port before configuring it */
			gpSGMII_regs[i]->SOFT_RESET_REG = CSL_CPSGMII_SOFT_RESET_REG_SOFT_RESET_MASK;

		    /* Reset MAC port*/            
		    CSL_CPGMAC_SL_resetMac (i);
			//wati for EMAC reset complete
		    while (CSL_CPGMAC_SL_isMACResetDone (i) != TRUE);

		    /*Receive FIFO Maximum Blocks: 3~15.
		    TX_MAX_BLKS= 20 - RX_MAX_BLKS*/
			gpCPSW_port_regs[i]->P_MAX_BLKS_REG=
				(ge_cfg->ethernet_port_cfg[i]->RX_FIFO_Max_blocks
				 <<CSL_CPSW_3GF_P_MAX_BLKS_REG_RX_MAX_BLKS_SHIFT)
				|((20-ge_cfg->ethernet_port_cfg[i]->RX_FIFO_Max_blocks)
				 <<CSL_CPSW_3GF_P_MAX_BLKS_REG_TX_MAX_BLKS_SHIFT);

			gpMAC_regs[i]->RX_MAXLEN_REG= ge_cfg->RX_MAX_length;
		
			if((ETHERNET_AUTO_NEGOTIAT_SLAVE==ge_cfg->ethernet_port_cfg[i]->mode)
				||(ETHERNET_AUTO_NEGOTIAT_MASTER==ge_cfg->ethernet_port_cfg[i]->mode))
			{
				/*1 Setup the SGMII as slave and enable autonegotiation:                         */
				/*1a Set bit 0 of the MR_ADV_ABILITY register                           */
				gpSGMII_regs[i]->MR_ADV_ABILITY_REG &= 0xFFFF0000; /* Clear the register contents */

				if(ETHERNET_AUTO_NEGOTIAT_SLAVE==ge_cfg->ethernet_port_cfg[i]->mode)
				{
					gpSGMII_regs[i]->MR_ADV_ABILITY_REG |= 0x00000001;

					/*1b Enable autonegotiation by setting the MR_AN_ENABLE bit in the*/
					gpSGMII_regs[i]->CONTROL_REG |= CSL_CPSGMII_CONTROL_REG_MR_AN_ENABLE_MASK;
				}
				else
				{
					gpSGMII_regs[i]->MR_ADV_ABILITY_REG |= 0x00009801;

					/*1b Enable autonegotiation by setting the MR_AN_ENABLE bit in the*/
					gpSGMII_regs[i]->CONTROL_REG |= CSL_CPSGMII_CONTROL_REG_MR_AN_ENABLE_MASK
						|CSL_CPSGMII_CONTROL_REG_MASTER_MASK;
				}
				/*2 Poll the SGMII_STATUS register to determine when autonegotiation is
				complete without error. The AN_ERROR bit in the SGMII_STATUS register
				will be set if the mode was commanded to be half-duplex gigabit.*/
				while(0==(gpSGMII_regs[i]->STATUS_REG&CSL_CPSGMII_STATUS_REG_LOCK_MASK));
				while(0==(gpSGMII_regs[i]->STATUS_REG&CSL_CPSGMII_STATUS_REG_LINK_MASK));
				while(0==(gpSGMII_regs[i]->STATUS_REG&CSL_CPSGMII_STATUS_REG_MR_AN_COMPLETE_MASK));

				if(gpSGMII_regs[i]->STATUS_REG&CSL_CPSGMII_STATUS_REG_AN_ERROR_MASK)
					puts("Half dulpex mode was negotiated!");

				/*3 In the MAC module, set the EXT_EN bit in the MAC_CONTROL register to 
				allow the speed and duplex mode to be set by the signals from the SGMII.*/
			}
			else 	//force mode
			{
#if 0
				if((ETHERNET_10M_HALFDUPLEX==ge_cfg->ethernet_port_cfg[i]->mode)
					||(ETHERNET_100M_HALFDUPLEX==ge_cfg->ethernet_port_cfg[i]->mode))
					bFullDuplex=FALSE;
#endif		 	
				if((ETHERNET_10M_FULLDUPLEX==ge_cfg->ethernet_port_cfg[i]->mode)
					/*||(ETHERNET_10M_HALFDUPLEX==ge_cfg->ethernet_port_cfg[i]->mode)*/)
				{
					speed_mode= 0; //10Mbps
				}
				else if((ETHERNET_100M_FULLDUPLEX==ge_cfg->ethernet_port_cfg[i]->mode)
					/*||(ETHERNET_100M_HALFDUPLEX==ge_cfg->ethernet_port_cfg[i]->mode)*/)
				{
					speed_mode= 1; //100Mbps
				}

				/*1 Setup the SGMII in force mode*/
				/*1a Set the MR_ADV_ABILITY register*/
				gpSGMII_regs[i]->MR_ADV_ABILITY_REG &= 0xFFFF0000; /* Clear the register contents */
				gpSGMII_regs[i]->MR_ADV_ABILITY_REG |= 0x8001
					|(speed_mode<<10)|(bFullDuplex<<12);

				/*1b Set the device in master mode without autonegotiation*/
				gpSGMII_regs[i]->CONTROL_REG |= CSL_CPSGMII_CONTROL_REG_MASTER_MASK;

				/*2 Poll the LINK bit in the SGMII_STATUS register to determine when the link is up.*/
				while(0==(gpSGMII_regs[i]->STATUS_REG&CSL_CPSGMII_STATUS_REG_LOCK_MASK));
				if(ETHERNET_MAC_LOOPBACK != ge_cfg->loopback_mode)
					while(0==(gpSGMII_regs[i]->STATUS_REG&CSL_CPSGMII_STATUS_REG_LINK_MASK));

				/*3 In the MAC module, the user must set the EXT_EN bit in the 
				MAC_CONTROL register to allow the speed and duplex mode to be 
				set by the signals from the SGMII.*/

			}
			
			if(ETHERNET_MAC_LOOPBACK==ge_cfg->loopback_mode)
				bMAC_loopback= TRUE;

			gpMAC_regs[i]->MACCONTROL_REG= ge_cfg->ethernet_port_cfg[i]->prmiscuous_mode
				|((speed_mode>>1)<<CSL_CPGMAC_SL_MACCONTROL_REG_GIG_SHIFT)
				|(ge_cfg->ethernet_port_cfg[i]->TX_flow_control_enable
				  <<CSL_CPGMAC_SL_MACCONTROL_REG_TX_FLOW_EN_SHIFT)
				|(ge_cfg->ethernet_port_cfg[i]->RX_flow_control_enable
				  <<CSL_CPGMAC_SL_MACCONTROL_REG_RX_FLOW_EN_SHIFT)
				|(bMAC_loopback<<CSL_CPGMAC_SL_MACCONTROL_REG_LOOPBACK_SHIFT)
				|(bFullDuplex<<CSL_CPGMAC_SL_MACCONTROL_REG_FULLDUPLEX_SHIFT)
				|(1<<CSL_CPGMAC_SL_MACCONTROL_REG_CTL_EN_SHIFT)
				|(1<<CSL_CPGMAC_SL_MACCONTROL_REG_GMII_EN_SHIFT);

			if(ETHERNET_SGMII_LOOPBACK==ge_cfg->loopback_mode)
			{
				/*1 Clear to zero the MR_AN_ENABLE bit in the SGMII_CONTROL*/
				gpSGMII_regs[i]->CONTROL_REG &= ~CSL_CPSGMII_CONTROL_REG_MR_AN_ENABLE_MASK;
				
				/*2 Write to one the RT_SOFT_RESET bit in the SOFT_RESET   */
				gpSGMII_regs[i]->SOFT_RESET_REG |= CSL_CPSGMII_SOFT_RESET_REG_RT_SOFT_RESET_MASK;

				/*3 Write to one the LOOPBACK bit in the SGMII_CONTROL     */
				gpSGMII_regs[i]->CONTROL_REG |= CSL_CPSGMII_CONTROL_REG_LOOPBACK_MASK;
				
				/*4 Write to zero the RT_SOFT_RESET bit in the SOFT_RESET  */
				gpSGMII_regs[i]->SOFT_RESET_REG &= ~CSL_CPSGMII_SOFT_RESET_REG_RT_SOFT_RESET_MASK;
			}		 	

		}
	}
}

/*setup MDIO for PHY controlling*/
void KeyStone_MDIO_Init(Ethernet_MDIO_Config *  mdio_cfg)
{
	if(NULL == mdio_cfg)
		return;

	gpMDIO_regs->CONTROL_REG= CSL_MDIO_CONTROL_REG_ENABLE_MASK
		|CSL_MDIO_CONTROL_REG_FAULT_MASK 	/*write 1 to clear this bit*/
		|CSL_MDIO_CONTROL_REG_FAULT_DETECT_ENABLE_MASK
		|(mdio_cfg->clock_div<<CSL_MDIO_CONTROL_REG_CLKDIV_SHIFT);

	//link INT0 setup
	if(mdio_cfg->link_INT0_PHY_select<=MDIO_INT_SELECT_PHY_31)
		gpMDIO_regs->USER_GROUP[0].USER_PHY_SEL_REG= 
			CSL_MDIO_USER_PHY_SEL_REG_LINKINT_ENABLE_MASK
			|(mdio_cfg->link_INT0_PHY_select<<CSL_MDIO_USER_PHY_SEL_REG_PHYADR_MON_SHIFT);

	//link INT1 setup
	if(mdio_cfg->link_INT1_PHY_select<=MDIO_INT_SELECT_PHY_31)
		gpMDIO_regs->USER_GROUP[1].USER_PHY_SEL_REG= 
			CSL_MDIO_USER_PHY_SEL_REG_LINKINT_ENABLE_MASK
			|(mdio_cfg->link_INT1_PHY_select<<CSL_MDIO_USER_PHY_SEL_REG_PHYADR_MON_SHIFT);

	/*The MDIO module powers up in an idle state before it is enabled.
	wait for it is not idle (really enabled)*/
	while(gpMDIO_regs->CONTROL_REG&CSL_MDIO_CONTROL_REG_IDLE_MASK);
	
}

/*Giga bit ethernet initialization*/
void KeyStone_GE_Init(KeyStone_GE_Config * ge_cfg)
{
	sgmiiSerdesRegs = (SerdesRegs *)&gpBootCfgRegs->SGMII_CFGPLL;

	if ((CSL_PSC_getPowerDomainState(CSL_PSC_PD_PASS) == PSC_PDSTATE_ON) &&
	   		(CSL_PSC_getModuleState (CSL_PSC_LPSC_CPGMAC) == PSC_MODSTATE_ENABLE))
	{
		// Disable pll before reconfiguring
		KeyStone_GE_soft_reset();	//soft reset GE if it is already enabled
	}

	//enable GE switch subsystem power and clock domain
	KeyStone_enable_PSC_module(CSL_PSC_PD_PASS, CSL_PSC_LPSC_PKTPROC);
	KeyStone_enable_PSC_module (CSL_PSC_PD_PASS,  CSL_PSC_LPSC_CPGMAC);

	/*configure Serdes                                                          */
	if((ETHERNET_MAC_LOOPBACK==ge_cfg->loopback_mode)
		||(ETHERNET_SGMII_LOOPBACK==ge_cfg->loopback_mode)
		||(ETHERNET_SERDES_LOOPBACK==ge_cfg->loopback_mode))
	{
		if(ge_cfg->serdes_cfg.linkSetup[0])
			ge_cfg->serdes_cfg.linkSetup[0]->loopBack = SERDES_LOOPBACK_ENABLE;
		if(ge_cfg->serdes_cfg.linkSetup[1])
			ge_cfg->serdes_cfg.linkSetup[1]->loopBack = SERDES_LOOPBACK_ENABLE;
	}
	else
	{
		if(ge_cfg->serdes_cfg.linkSetup[0])
			ge_cfg->serdes_cfg.linkSetup[0]->loopBack = SERDES_LOOPBACK_DISABLE;
		if(ge_cfg->serdes_cfg.linkSetup[1])
			ge_cfg->serdes_cfg.linkSetup[1]->loopBack = SERDES_LOOPBACK_DISABLE;
	}

	//Serdes should be setup before all other initialization
 	Keystone_SGMII_Serdes_init(&ge_cfg->serdes_cfg, sgmiiSerdesRegs);
	Wait_SGMII_PLL_Lock();

	/* Configure the MDIO and external PHY (if used)                           */
	KeyStone_MDIO_Init(ge_cfg->mdio_cfg);

	/* Configure the MAC and SGMII modules                                   */
	KeyStone_Ethernet_Ports_Init(ge_cfg);

	/* Configure the CPSW_CONTROL register                                     */
	if(ETHERNET_PORT_FIFO_LOOPBACK==ge_cfg->loopback_mode)
		gpCPSW_regs->CPSW_CONTROL_REG= CSL_CPSW_3GF_CPSW_CONTROL_REG_P0_ENABLE_MASK
			|(1<<CSL_CPSW_3GF_CPSW_CONTROL_REG_FIFO_LOOPBACK_SHIFT);
	else
		gpCPSW_regs->CPSW_CONTROL_REG= CSL_CPSW_3GF_CPSW_CONTROL_REG_P0_ENABLE_MASK;

	gpCPSW_regs->RX_MAXLEN_REG= ge_cfg->RX_MAX_length;

	/*initialize the CPPI Info Word 0 SRC_ID field for received packets*/
	KeyStone_GE_CPPI_Src_ID_Init(ge_cfg->ethernet_port_cfg);

	/*2 Configure the MAC1_SA and MAC2_SA  */
	/*  source address hi and lo registers                                      */
	KeyStone_GE_Flow_Control_MAC_Address_Init(ge_cfg->ethernet_port_cfg);

	/*3 Enable the desired statistics ports by programming the CPSW_STAT_PORT_EN*/
	KeyStone_GE_Statistics_Init(ge_cfg->ethernet_port_cfg);

	/*4 Configure the ALE                                                       */
	KeyStone_GE_ALE_Init(ge_cfg->ale_cfg);
	
}

/*accumulate the statistics values in the registers to the software data structure.
application should call this function before the 32-bit register counters overflow*/
void KeyStone_GE_Accumulate_Statistics()
{
	int i;
	CSL_CPSW_3GF_STATS stats_regs_copy;
	
	for(i=0; i< GE_NUM_ETHERNET_PORT; i++)
	{
		if(STATS_EN[i])
		{
			//copy the value to local data structure
			memcpy((void *)&stats_regs_copy, (void *)gpStats_regs[i], sizeof(CSL_CPSW_3GF_STATS));

			//decrease the values in the registers
			memcpy((void *)gpStats_regs[i], (void *)&stats_regs_copy, sizeof(CSL_CPSW_3GF_STATS));

			//accumulate the values in register to 64-bit data structure
		    statistics[i].RxGoodFrames      += stats_regs_copy.RxGoodFrames     ;
		    statistics[i].RxBCastFrames     += stats_regs_copy.RxBCastFrames    ;
		    statistics[i].RxMCastFrames     += stats_regs_copy.RxMCastFrames    ;
		    statistics[i].RxPauseFrames     += stats_regs_copy.RxPauseFrames    ;
		    statistics[i].RxCRCErrors       += stats_regs_copy.RxCRCErrors      ;
		    statistics[i].RxAlignCodeErrors += stats_regs_copy.RxAlignCodeErrors;
		    statistics[i].RxOversized       += stats_regs_copy.RxOversized      ;
		    statistics[i].RxJabber          += stats_regs_copy.RxJabber         ;
		    statistics[i].RxUndersized      += stats_regs_copy.RxUndersized     ;
		    statistics[i].RxFragments       += stats_regs_copy.RxFragments      ;
		    statistics[i].reserved          += stats_regs_copy.reserved         ;
		    statistics[i].reserved2         += stats_regs_copy.reserved2        ;
		    statistics[i].RxOctets          += stats_regs_copy.RxOctets         ;
		    statistics[i].TxGoodFrames      += stats_regs_copy.TxGoodFrames     ;
		    statistics[i].TxBCastFrames     += stats_regs_copy.TxBCastFrames    ;
		    statistics[i].TxMCastFrames     += stats_regs_copy.TxMCastFrames    ;
		    statistics[i].TxPauseFrames     += stats_regs_copy.TxPauseFrames    ;
		    statistics[i].TxDeferred        += stats_regs_copy.TxDeferred       ;
		    statistics[i].TxCollision       += stats_regs_copy.TxCollision      ;
		    statistics[i].TxSingleColl      += stats_regs_copy.TxSingleColl     ;
		    statistics[i].TxMultiColl       += stats_regs_copy.TxMultiColl      ;
		    statistics[i].TxExcessiveColl   += stats_regs_copy.TxExcessiveColl  ;
		    statistics[i].TxLateColl        += stats_regs_copy.TxLateColl       ;
		    statistics[i].TxUnderrun        += stats_regs_copy.TxUnderrun       ;
		    statistics[i].TxCarrierSLoss    += stats_regs_copy.TxCarrierSLoss   ;
		    statistics[i].TxOctets          += stats_regs_copy.TxOctets         ;
		    statistics[i].Frame64           += stats_regs_copy.Frame64          ;
		    statistics[i].Frame65t127       += stats_regs_copy.Frame65t127      ;
		    statistics[i].Frame128t255      += stats_regs_copy.Frame128t255     ;
		    statistics[i].Frame256t511      += stats_regs_copy.Frame256t511     ;
		    statistics[i].Frame512t1023     += stats_regs_copy.Frame512t1023    ;
		    statistics[i].Frame1024tUp      += stats_regs_copy.Frame1024tUp     ;
		    statistics[i].NetOctets         += stats_regs_copy.NetOctets        ;
		    statistics[i].RxSOFOverruns     += stats_regs_copy.RxSOFOverruns     ;
		    statistics[i].RxMOFOverruns     += stats_regs_copy.RxMOFOverruns     ;
		    statistics[i].RxDMAOverruns     += stats_regs_copy.RxDMAOverruns     ;
		}
	}	
}

/*fill EMAC header to a packet buffer*/
void Fill_EMAC_header(Uint8 *buffer, Ethernet_Packet_Type type,
	unsigned long long sourceMAC, unsigned long long destMAC)
{
#if 1
	/*destination address*/
	buffer[0]= (_hill(destMAC)>>8)&0xFF;
	buffer[1]= (_hill(destMAC)>>0)&0xFF;
	buffer[2]= (_loll(destMAC)>>24)&0xFF;
	buffer[3]= (_loll(destMAC)>>16)&0xFF;
	buffer[4]= (_loll(destMAC)>>8)&0xFF;
	buffer[5]= (_loll(destMAC)>>0)&0xFF;

	/*source address*/
	buffer[6]= (_hill(sourceMAC)>>8)&0xFF;
	buffer[7]= (_hill(sourceMAC)>>0)&0xFF;
	buffer[8]= (_loll(sourceMAC)>>24)&0xFF;
	buffer[9]= (_loll(sourceMAC)>>16)&0xFF;
	buffer[10]= (_loll(sourceMAC)>>8)&0xFF;
	buffer[11]= (_loll(sourceMAC)>>0)&0xFF;
#else
	/*destination address*/
	buffer[5]= (_hill(destMAC)>>8)&0xFF;
	buffer[4]= (_hill(destMAC)>>0)&0xFF;
	buffer[3]= (_loll(destMAC)>>24)&0xFF;
	buffer[2]= (_loll(destMAC)>>16)&0xFF;
	buffer[1]= (_loll(destMAC)>>8)&0xFF;
	buffer[0]= (_loll(destMAC)>>0)&0xFF;

	/*source address*/
	buffer[11]= (_hill(sourceMAC)>>8)&0xFF;
	buffer[10]= (_hill(sourceMAC)>>0)&0xFF;
	buffer[9]= (_loll(sourceMAC)>>24)&0xFF;
	buffer[8]= (_loll(sourceMAC)>>16)&0xFF;
	buffer[7]= (_loll(sourceMAC)>>8)&0xFF;
	buffer[6]= (_loll(sourceMAC)>>0)&0xFF;

#endif

#if 1
	/*packet type*/
	buffer[12]= (type>>8)&0xFF;
	buffer[13]= type&0xFF;
#else
	/*packet type*/
	buffer[12]= type&0xFF;
	buffer[13]= (type>>8)&0xFF;
#endif
}

/*get EMAC header information in a packet buffer*/
void Get_EMAC_header(Uint8 *buffer, Ethernet_Packet_Type * type,
	unsigned long long * sourceMAC, unsigned long long * destMAC)
{
 	*destMAC= _itoll((buffer[0]<<8)|buffer[1], 
 		(buffer[2]<<24)|(buffer[3]<<16)|(buffer[4]<<8)|buffer[5]);

 	*sourceMAC= _itoll((buffer[6]<<8)|buffer[7], 
 		(buffer[8]<<24)|(buffer[9]<<16)|(buffer[10]<<8)|buffer[11]);

 	*type= (Ethernet_Packet_Type)((buffer[12]<<8)|buffer[13]);
}

/*set "data" in to a register of a PHY*/
void KeyStone_MDIO_PHY_Set_Reg(Uint32 phyNum, Uint32 regNum, Uint16 data)
{
	/*wait for idle (GO=0)*/
	while(gpMDIO_regs->USER_GROUP[0].USER_ACCESS_REG&CSL_MDIO_USER_ACCESS_REG_GO_MASK);
	
	gpMDIO_regs->USER_GROUP[0].USER_ACCESS_REG= CSL_MDIO_USER_ACCESS_REG_GO_MASK
		|CSL_MDIO_USER_ACCESS_REG_WRITE_MASK
		|(phyNum<<CSL_MDIO_USER_ACCESS_REG_PHYADR_SHIFT)
		|(regNum<<CSL_MDIO_USER_ACCESS_REG_REGADR_SHIFT)
		|(data<<CSL_MDIO_USER_ACCESS_REG_DATA_SHIFT);

	/*wait for write complete (GO=0)*/
	while(gpMDIO_regs->USER_GROUP[0].USER_ACCESS_REG&CSL_MDIO_USER_ACCESS_REG_GO_MASK);
}

/*read data from a register of a PHY*/
Uint16 KeyStone_MDIO_PHY_Get_Reg(Uint32 phyNum, Uint32 regNum)
{
	/*wait for idle (GO=0)*/
	while(gpMDIO_regs->USER_GROUP[0].USER_ACCESS_REG&CSL_MDIO_USER_ACCESS_REG_GO_MASK);
	
	gpMDIO_regs->USER_GROUP[0].USER_ACCESS_REG= CSL_MDIO_USER_ACCESS_REG_GO_MASK
		|(phyNum<<CSL_MDIO_USER_ACCESS_REG_PHYADR_SHIFT)
		|(regNum<<CSL_MDIO_USER_ACCESS_REG_REGADR_SHIFT);

	/*wait for read complete (GO=0)*/
	while(gpMDIO_regs->USER_GROUP[0].USER_ACCESS_REG&CSL_MDIO_USER_ACCESS_REG_GO_MASK);

	/*wait for read acknowledge (ACK=1)*/
	while(0==(gpMDIO_regs->USER_GROUP[0].USER_ACCESS_REG&CSL_MDIO_USER_ACCESS_REG_ACK_MASK));
	
	return (gpMDIO_regs->USER_GROUP[0].USER_ACCESS_REG&CSL_MDIO_USER_ACCESS_REG_DATA_MASK);
}


