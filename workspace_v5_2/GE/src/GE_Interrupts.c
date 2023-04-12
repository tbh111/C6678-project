/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2013
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
 Interrupts configuration and handling for GE
 * =============================================================================
 *  Revision History
 *  ===============
 *  July 4, 2013 Kevin Cai  file created
 *  August 25, 2013 Brighton Feng, add interrupt handling for MDIO and MISC
 * =============================================================================
 */

#include <stdio.h>
#include <c6x.h> 
#include <csl_cpintc.h>
#include <cslr_pa_ss.h>
#include "Keystone_common.h"
#include "KeyStone_GE_Init_drv.h"
#include "GE_test.h"
#include "GE_debug.h"
#include "GE_PktDMA_Init.h"
#include "GE_Interrupts.h"

Uint32 uiGE_MISC_MDIO_INT_num= 0;
CSL_CPINTCRegs * CIC_Regs;
void GE_Interrupts_Init(void)
{
	if(DNUM<4) /*core 0~3 use CIC0*/
		CIC_Regs= gpCIC0_regs;
	else
		CIC_Regs= gpCIC1_regs;

	/* Disable Global host interrupts. */
	gpCIC0_regs->GLOBAL_ENABLE_HINT_REG= 0;

	/* Clear Interrupt events in MDIO*/
	gpMDIO_regs->LINK_INT_RAW_REG= 0xFFFFFFFF;

	if(C6670_EVM==gDSP_board_type
		||DUAL_NYQUIST_EVM==gDSP_board_type
		||TCI6614_EVM==gDSP_board_type)
	{
		uiGE_MISC_MDIO_INT_num= 64;

		/*map GE MISC Interrupt and MDIO events to CIC0 out*/
		KeyStone_CIC_event_map(gpCIC0_regs, CSL_INTC0_MDIO_LINK_INTR0, 64);
		KeyStone_CIC_event_map(gpCIC0_regs, CSL_INTC0_MDIO_LINK_INTR1, 64);
		KeyStone_CIC_event_map(gpCIC0_regs, CSL_INTC0_MISC_INTR, 64);
	}
	else if(C6678_EVM==gDSP_board_type)
	{	
		uiGE_MISC_MDIO_INT_num= 33;

		/*map GE MISC Interrupt and MDIO events to CIC0 out*/
		KeyStone_CIC_event_map(gpCIC0_regs, CSL_INTC0_MDIO_LINK_INTR0, 33);
		KeyStone_CIC_event_map(gpCIC0_regs, CSL_INTC0_MDIO_LINK_INTR1, 33);
		KeyStone_CIC_event_map(gpCIC0_regs, CSL_INTC0_MISC_INTR, 33);
	}
	else
	{
		puts("Unknown DSP board type!");
		return;
	}	
	
	/* Enable Global host interrupts. */
	gpCIC0_regs->GLOBAL_ENABLE_HINT_REG= 1;

	/*map message descriptor accumulation high priority channel	interrupt to INT4
	map MISC/MDIO interrupt to INT5*/
	gpCGEM_regs->INTMUX1 =
		(CSL_GEM_QM_INT_HIGH_N<<CSL_CGEM_INTMUX1_INTSEL4_SHIFT)
		|(22<<CSL_CGEM_INTMUX1_INTSEL5_SHIFT);

	//enable INT4 and INT5
	CPU_interrupt_enable((1<<4)|(1<<5));
}

//parser the received packet
void parserRxPacket(HostPacketDescriptor * hostDescriptor)
{
	Uint32 length;
	Ethernet_Packet_Type type;
	unsigned long long sourceMAC, destMAC;
	Uint8 lastData;
	
	/*invalid cache before read descriptor RAM*/
	InvalidCache((void *)hostDescriptor, 64);

	length= hostDescriptor->packet_length- EMAC_HEADER_LEN- EMAC_CRC_LEN;

	Get_EMAC_header((Uint8 *)hostDescriptor->buffer_ptr,
		&type, &sourceMAC, &destMAC);

	//check the last lastData
	lastData= *(Uint8 *)(hostDescriptor->buffer_ptr+length+EMAC_HEADER_LEN-1);

	printf("received %d bytes (last byte=0x%x), packet type = 0x%04x, from srouce address 0x%012llx to destination address 0x%012llx\n",
		length, lastData, type, sourceMAC, destMAC);
}

volatile Uint32 GE_INT_TSCL=0;
volatile Uint32 GE_RX_packet_cnt=0;
interrupt void GE_Message_ISR()
{
	int i;
	Uint32 qmIntStatus0;
	Uint32 uiMask;
	Uint32 * uipAccList;
	HostPacketDescriptor * hostDescriptor;

	GE_INT_TSCL= TSCL; 	//save the TSC when the interrupt happens

	/*read interrupt status*/
	qmIntStatus0= gpQM_INTD_regs->STATUS_REG0;

	/*clear interrupt status*/
	gpQM_INTD_regs->STATUS_CLR_REG0= qmIntStatus0;

	/*high priority interrupt*/
	uiMask=1;
	for(i=0; i<32; i++)
	{
		if(uiMask&qmIntStatus0)
		{
			
			gpQM_INTD_regs->INTCNT_REG[i]= 1 ;
			gpQM_INTD_regs->EOI_REG= i+2;
			//printf("high priority queue channel %d interrupt happens at %u\n", GE_INT_TSCL);
		} 
		uiMask<<=1;
	}

	/*read the acuumulation list.*/
	uipAccList= &uiaDescriptorAccumulationList[(uiAccPingPong&1)*(uiAccPageSize+1)];
	uiAccPingPong++;

	/*The first list entry is used to store the total list entry count*/
	for(i=0; i<uipAccList[0]; i++)
	{
		//read descriptor pointer
		hostDescriptor= (HostPacketDescriptor *)(uipAccList[1+i]&0xFFFFFFF0);
		if(NULL==hostDescriptor)
			continue;

		//for 2 DSP test, parser the packet recieved
		if(GE_TEST_DSP0_TO_DSP1==test_data_path)
		{
			parserRxPacket(hostDescriptor);
		}
		
		/*descriptor Reclamation*/
		KeyStone_queuePush(RECLAMATION_QUEUE, (Uint32)hostDescriptor|FETCH_SIZE_32);
	}

	GE_RX_packet_cnt += uipAccList[0];
}

void PHY_Link_State_Update(Uint32 link_num)
{
	Uint32 uiPhy_NUM;

	uiPhy_NUM= gpMDIO_regs->USER_GROUP[link_num].USER_PHY_SEL_REG&CSL_MDIO_USER_PHY_SEL_REG_LINKSEL_SHIFT;
	if(gpMDIO_regs->LINK_REG&(1<<uiPhy_NUM))
	{
		printf("Ethernet PHY%d link up. TSCL= 0x%x\n", uiPhy_NUM, TSCL);
		print_GE_link_ability(link_num);
	}
	else
		printf("Ethernet PHY%d link down. TSCL= 0x%x\n", uiPhy_NUM, TSCL);

	/* Clear Interrupt events in MDIO*/
	gpMDIO_regs->LINK_INT_RAW_REG= 1<<uiPhy_NUM;
}

interrupt void GE_MISC_MDIO_ISR(void)
{
	/* Disable the host interrupt */
	CIC_Regs->HINT_ENABLE_CLR_INDEX_REG = uiGE_MISC_MDIO_INT_num;

	if(CIC_Regs->ENA_STATUS_REG[CSL_INTC0_MDIO_LINK_INTR0/32]&(1<<(CSL_INTC0_MDIO_LINK_INTR0%32)))
	{
		/* Clear Interrupt events in CIC*/
		KeyStone_CIC_clear_system_event(CIC_Regs, CSL_INTC0_MDIO_LINK_INTR0);

		PHY_Link_State_Update(0);

		/* Clear Interrupt events in CIC*/
		KeyStone_CIC_clear_system_event(CIC_Regs, CSL_INTC0_MDIO_LINK_INTR0);
	}

	if(CIC_Regs->ENA_STATUS_REG[CSL_INTC0_MDIO_LINK_INTR1/32]&(1<<(CSL_INTC0_MDIO_LINK_INTR1%32)))
	{
		/* Clear Interrupt events in CIC*/
		KeyStone_CIC_clear_system_event(CIC_Regs, CSL_INTC0_MDIO_LINK_INTR1);

		PHY_Link_State_Update(1);

		/* Clear Interrupt events in CIC*/
		KeyStone_CIC_clear_system_event(CIC_Regs, CSL_INTC0_MDIO_LINK_INTR1);
	}

	if(CIC_Regs->ENA_STATUS_REG[CSL_INTC0_MISC_INTR/32]&(1<<(CSL_INTC0_MISC_INTR%32)))
	{
		/* Clear Interrupt events in CIC*/
		KeyStone_CIC_clear_system_event(CIC_Regs, CSL_INTC0_MISC_INTR);

		if(gpNetCP_regs->INTD.INTD_STATUS2&CSL_PA_SS_INTD_STATUS2_STATUS_HOST_CPSW_STAT0_SYNCD_MASK)
		{
			puts("GE STATA wraps around");
		}

		if(gpNetCP_regs->INTD.INTD_STATUS2&CSL_PA_SS_INTD_STATUS2_STATUS_HOST_CPSW_STAT1_SYNCD_MASK)
		{
			puts("GE STATB wraps around");
		}

		/*accumulate the statistics values in the registers to the software data structure.*/
		KeyStone_GE_Accumulate_Statistics();

		gpNetCP_regs->INTD.INTD_STATUS_CLR2= gpNetCP_regs->INTD.INTD_STATUS2;

		//EOI
		gpNetCP_regs->INTD.INTD_EOI= gpNetCP_regs->INTD.INTD_INTR_VECTOR;
	}

	/* Enable the host interrupt */
	CIC_Regs->HINT_ENABLE_SET_INDEX_REG = uiGE_MISC_MDIO_INT_num;
}

