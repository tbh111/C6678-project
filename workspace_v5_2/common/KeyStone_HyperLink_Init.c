/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2011
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
 example for HyperLink configuration on KeyStone DSP
 * =============================================================================
 *  Revision History
 *  ===============
 *  Nov 4, 2011 Brighton Feng  file created
 *  Dec 8, 2014 Brighton Feng  add 5ms delay after enabling the power domain
 * =============================================================================*/
#include <c6x.h>
#include "KeyStone_HyperLink_init.h"

CSL_VusrRegs * gpHyperLinkRegs = (CSL_VusrRegs *)CSL_MCM_CONFIG_REGS;
SerdesRegs * hyperLinkSerdesRegs;

/*soft shutdown and reset HyperLink*/
void KeyStone_HyperLink_soft_reset()
{
	int i;
	TDSP_Board_Type DSP_Board_Type;
	
	/*disable all portal or remote register operation
	This bit should be set before iloop or reset bits are changed.*/
	gpHyperLinkRegs->CTL |= CSL_VUSR_CTL_SERIAL_STOP_MASK;

	/*Wait until no Remote Pending Request*/
	while(gpHyperLinkRegs->STS&CSL_VUSR_STS_RPEND_MASK);

	/*Reset*/
	gpHyperLinkRegs->CTL |= CSL_VUSR_CTL_RESET_MASK;

	/*wait for a while*/
	for(i=0;i<100;i++)asm(" NOP 5 ");

	DSP_Board_Type= KeyStone_Get_dsp_board_type();
	if(TCI6614_EVM==DSP_Board_Type)
	{
		//disable HyperLink through PSC
		KeyStone_disable_PSC_module(CSL_PSC_PD_ALWAYSON, 3);
	}
	else
	{
		//disable HyperLink through PSC
		KeyStone_disable_PSC_module(5, 12);
		KeyStone_disable_PSC_Power_Domain(5);
	}
}

void KeyStone_HyperLink_Addr_Map(
	HyperLink_Address_Map * addr_map)
{
	int i;
	
	gpHyperLinkRegs->TX_SEL_CTL = 
		(addr_map->tx_addr_mask<<CSL_VUSR_TX_SEL_CTL_TXIGNMSK_SHIFT)
		|(addr_map->tx_priv_id_ovl<<CSL_VUSR_TX_SEL_CTL_TXPRIVIDOVL_SHIFT);
		
	gpHyperLinkRegs->RX_SEL_CTL = 
		(addr_map->rx_seg_sel<<CSL_VUSR_RX_SEL_CTL_RXSEGSEL_SHIFT)
		|(addr_map->rx_priv_id_sel<<CSL_VUSR_RX_SEL_CTL_RXPRIVIDSEL_SHIFT);
		
	for(i= 0; i< 16; i++)
	{
		gpHyperLinkRegs->RX_PRIV_IDX= i;
		gpHyperLinkRegs->RX_PRIV_VAL= addr_map->rx_priv_id_map[i];
	}

	for(i= 0; i< 64; i++)
	{
		gpHyperLinkRegs->RX_SEG_IDX= i;
		gpHyperLinkRegs->RX_SEG_VAL= 
			addr_map->rx_addr_segs[i].Seg_Base_Addr
			|addr_map->rx_addr_segs[i].Seg_Length;
	}

}

void KeyStone_HyperLink_Interrupt_init(
	HyperLink_Interrupt_Cfg * int_cfg)
{
	int i;
	
	gpHyperLinkRegs->CTL = gpHyperLinkRegs->CTL
		|(int_cfg->int_local<<CSL_VUSR_CTL_INTLOCAL_SHIFT)
		|(int_cfg->sts_int_enable<<CSL_VUSR_CTL_INTENABLE_SHIFT)
		|(int_cfg->sts_int_vec<<CSL_VUSR_CTL_INTVEC_SHIFT)
		|(int_cfg->int2cfg<<CSL_VUSR_CTL_INT2CFG_SHIFT);

	for(i=0; i<64; i++)
	{
		gpHyperLinkRegs->INT_CTL_IDX = i;
		gpHyperLinkRegs->INT_CTL_VAL= 
			(int_cfg->int_event_cntl[i].Int_en<<CSL_VUSR_INT_CTL_VAL_INTEN_SHIFT)
			|(int_cfg->int_event_cntl[i].Int_type<<CSL_VUSR_INT_CTL_VAL_INTTYPE_SHIFT)
			|(int_cfg->int_event_cntl[i].Int_pol<<CSL_VUSR_INT_CTL_VAL_INTPOL_SHIFT)
			|(int_cfg->int_event_cntl[i].si_en<<CSL_VUSR_INT_CTL_VAL_SIEN_SHIFT)
			|(int_cfg->int_event_cntl[i].mps<<CSL_VUSR_INT_CTL_VAL_MPS_SHIFT)
			|(int_cfg->int_event_cntl[i].vector<<CSL_VUSR_INT_CTL_VAL_VECTOR_SHIFT);
	}
		
	for(i=0; i<NUM_MPS; i++)
	{
		gpHyperLinkRegs->INT_PTR_IDX = i;
		gpHyperLinkRegs->INT_PTR_VAL= int_cfg->int_set_register_pointer[i];
	}

	//clear any pending interrupt
	gpHyperLinkRegs->INT_CLR= 0xFFFFFFFF;
}

void KeyStone_HyperLink_Init(HyperLink_Config * hyperLink_cfg)
{
	TDSP_Board_Type DSP_Board_Type;

	DSP_Board_Type= KeyStone_Get_dsp_board_type();
	if(TCI6614_EVM==DSP_Board_Type)
	{
		//enable HyperLink power and clock domain
		KeyStone_enable_PSC_module(CSL_PSC_PD_ALWAYSON, 3);
	}
	else
	{
		KeyStone_enable_PSC_module(5, 12);
	}

	TSC_delay_ms(5); //wait for power module up complete.

	if(HyperLink_LOOPBACK==hyperLink_cfg->loopback_mode)
	{
		/*disable all portal or remote register operation
		This bit should be set before iloop or reset bits are changed.*/
		gpHyperLinkRegs->CTL |= CSL_VUSR_CTL_SERIAL_STOP_MASK;

		/*Wait until no Remote Pending Request*/
		while(gpHyperLinkRegs->STS&CSL_VUSR_STS_RPEND_MASK);

		gpHyperLinkRegs->CTL |= CSL_VUSR_CTL_LOOPBACK_MASK;
	}
#if 1
	//force 4 lanes always
	gpHyperLinkRegs->PWR = 
		(7<<CSL_VUSR_PWR_H2L_SHIFT)
		|(7<<CSL_VUSR_PWR_L2H_SHIFT)
		|(1<<CSL_VUSR_PWR_PWC_SHIFT)
		|(1<<CSL_VUSR_PWR_QUADLANE_SHIFT)
		|(0<<CSL_VUSR_PWR_ZEROLANE_SHIFT)
		|(0<<CSL_VUSR_PWR_SINGLELANE_SHIFT);
#endif
	/*enable operation*/
	gpHyperLinkRegs->CTL &= ~(CSL_VUSR_CTL_SERIAL_STOP_MASK);

    KeyStone_HyperLink_Addr_Map(&hyperLink_cfg->address_map);

    KeyStone_HyperLink_Interrupt_init(&hyperLink_cfg->interrupt_cfg);

	/*tell all receivers to ignore close to the first 3uS of data at beginning of training sequence*/
	gpHyperLinkRegs->SERDES_CTL_STS1= 0xFFFF0000;

    //PC TEST
	//gpHyperLinkRegs->SERDES_CTL_STS4= 0x000F0000;//No SERDES sleep for four lane mode operation

	hyperLinkSerdesRegs = (SerdesRegs *)&gpBootCfgRegs->VUSR_CFGPLL;
	KeyStone_HyperLink_Serdes_init(&hyperLink_cfg->serdes_cfg, hyperLinkSerdesRegs);
	Wait_Hyperlink_PLL_Lock();

	/*---------wait for link status OK-------------*/
	while(gpHyperLinkRegs->STS&CSL_VUSR_STS_SERIAL_HALT_MASK);
	while(gpHyperLinkRegs->STS&CSL_VUSR_STS_PLL_UNLOCK_MASK);
	while(0==(gpHyperLinkRegs->STS&CSL_VUSR_STS_LINK_MASK));
	while(0==(gpHyperLinkRegs->LINK_STS&CSL_VUSR_LINK_STS_RX_ONE_ID_MASK));

	/*after initialization, change the delay to default value to improve performance*/
	//gpHyperLinkRegs->SERDES_CTL_STS1= 0x092E0000;

}

