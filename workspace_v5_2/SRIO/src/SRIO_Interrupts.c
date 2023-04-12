/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2011
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
 Interrupts configuration and handling for SRIO
 * =============================================================================
 *  Revision History
 *  ===============
 *  June 20, 2011 Brighton Feng  file created
 * =============================================================================
 */

#include <stdio.h>
#include <c6x.h> 
#include <csl_cpintc.h>
#include "KeyStone_common.h"
#include "KeyStone_SRIO_Init_drv.h"
#include "SRIO_test.h"
#include "SRIO_PktDMA_Init.h"

SRIO_Interrupt_Map interrupt_map[]=
{
    /*interrupt_event*/ /*INTDST_number*/
	{DOORBELL0_0_INT, 	INTDST_16}, 	/*route to core 0*/
	{DOORBELL0_1_INT, 	INTDST_16}, 	/*route to core 0*/
	{DOORBELL0_2_INT, 	INTDST_16}, 	/*route to core 0*/
	{DOORBELL0_3_INT, 	INTDST_16}, 	/*route to core 0*/
	{DOORBELL0_4_INT, 	INTDST_16} 		/*route to core 0*/
};
SRIO_Interrupt_Cfg interrupt_cfg;

void SRIO_Interrupts_Init(void)
{
	/*map SRIO doorbell interrupts to INT4.
	map message descriptor accumulation low priority channel 0 interrupt 
	to INT5*/
	gpCGEM_regs->INTMUX1 = 
		(CSL_GEM_INTDST_N_PLUS_16<<CSL_CGEM_INTMUX1_INTSEL4_SHIFT)|
		(CSL_GEM_QM_INT_LOW_0<<CSL_CGEM_INTMUX1_INTSEL5_SHIFT);

	//enable INT4, 5
	CPU_interrupt_enable((1<<4)|(1<<5));

	interrupt_cfg.interrupt_map = interrupt_map;
	interrupt_cfg.uiNumInterruptMap = 
		sizeof(interrupt_map)/sizeof(SRIO_Interrupt_Map);

	/*interrupt rate control is not used in this test*/
	interrupt_cfg.interrupt_rate= NULL;
    interrupt_cfg.uiNumInterruptRateCfg= 0;

	interrupt_cfg.doorbell_route_ctl= SRIO_DOORBELL_ROUTE_TO_DEDICATE_INT;

	srio_cfg.interrupt_cfg = &interrupt_cfg;

}


/*define the message convey by doorbell for this test*/
char * doorbell_msg_str[]=
{
	"doorbell for latency test.", 
	"doorbell which indicates NREAD operation done.",
	"doorbell which indicates NWRITE operation done.",
	"doorbell which indicates NWRITE_R operation done.",  
	"doorbell which indicates SWRITE operation done."
};
interrupt void SRIO_Doorbell_ISR()
{
	Uint32 doorbell;
	uiDoorbell_TSC= TSCL;

	//read doorbell. this test only use doorbell reg 0
	doorbell= gpSRIO_regs->DOORBELL_ICSR_ICCR[0].RIO_DOORBELL_ICSR;

	//clear doorbell interrupt
	gpSRIO_regs->DOORBELL_ICSR_ICCR[0].RIO_DOORBELL_ICCR= doorbell; 

	//printf doorbell message
	if(doorbell&1)
		printf("received %s\n", doorbell_msg_str[0]);
	if(doorbell&2)
		printf("received %s\n", doorbell_msg_str[1]);
	if(doorbell&4)
		printf("received %s\n", doorbell_msg_str[2]);
	if(doorbell&8)
		printf("received %s\n", doorbell_msg_str[3]);
	if(doorbell&16)
		printf("received %s\n", doorbell_msg_str[4]);
		
}

interrupt void SRIO_Message_ISR()
{
	Uint32 qmIntStatus1;
	Uint32 uiQueueNum;
	HostPacketDescriptor * hostDescriptor;
	SRIO_Type9_Message_RX_Descriptor * type9MsgRxDesc;

	/*read interrupt status*/
	qmIntStatus1= gpQM_INTD_regs->STATUS_REG1;

	/*clear interrupt status*/
	gpQM_INTD_regs->STATUS_CLR_REG1= qmIntStatus1;

	/*low priority channel 0 interrupt*/
	if(1&qmIntStatus1)
	{
		//clear interrupt		
		gpQM_INTD_regs->INTCNT_REG[32]= 1 ;
		gpQM_INTD_regs->EOI_REG= 34;

		//read descriptor pointer
		hostDescriptor= (HostPacketDescriptor *)(uiaDescriptorAccumulationList[0]&0xFFFFFFF0);

		/*invalid cache before read descriptor RAM*/
		InvalidCache((void *)hostDescriptor, 64);

		type9MsgRxDesc= (SRIO_Type9_Message_RX_Descriptor *)
			(((Uint32)hostDescriptor)+32);

		printf("received %d bytes message at 0x%x, from srouce ID 0x%x to destination ID 0x%x\n",
			hostDescriptor->packet_length, hostDescriptor->buffer_ptr,
			type9MsgRxDesc->SRC_ID, type9MsgRxDesc->Dest_ID);

		/*descriptor Reclamation*/
		uiQueueNum= (hostDescriptor->pkt_return_qmgr<<12)|
			hostDescriptor->pkt_return_qnum;
		KeyStone_queuePush(uiQueueNum, (Uint32)hostDescriptor|FETCH_SIZE_64);

	} 
	
}


