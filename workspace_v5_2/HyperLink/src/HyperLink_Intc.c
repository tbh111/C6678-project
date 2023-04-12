/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2012
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
Interrupts configuration for HyperLink
 * =============================================================================
 *  Revision History
 *  ===============
 *  17-March-2012 Brighton Feng  file created
 * =============================================================================
 */

#include <stdio.h>
#include <c6x.h> 
#include <csl_cpintc.h>
#include "KeyStone_common.h"
#include "KeyStone_HyperLink_init.h"
#include "HyperLink_debug.h"

Uint32 HyperLinkIntTSCL;
int uiHyLink_pend_host_event_num;
void HyperLink_Interrupts_Init(void)
{
	/* Disable Global host interrupts. */
	gpCIC0_regs->GLOBAL_ENABLE_HINT_REG= 0;

	if(C6670_EVM==gDSP_board_type
		||DUAL_NYQUIST_EVM==gDSP_board_type
		||TCI6614_EVM==gDSP_board_type)
	{
		uiHyLink_pend_host_event_num= 64;

		/*map HyperLink Interrupt events (111) to CIC0 out64*/
		KeyStone_CIC_event_map(gpCIC0_regs, CSL_INTC0_VUSR_INT_O, 64);
	}
	else if(C6678_EVM==gDSP_board_type)
	{	
		uiHyLink_pend_host_event_num= 33;

		/*map HyperLink Interrupt events (111) to CIC0 out33*/
		KeyStone_CIC_event_map(gpCIC0_regs, CSL_INTC0_VUSR_INT_O, 33);
	}
	else
	{
		puts("Unknown DSP board type!");
		return;
	}	
	
	/* Enable Global host interrupts. */
	gpCIC0_regs->GLOBAL_ENABLE_HINT_REG= 1;

	/*on Nyquist, CIC0 out64 event number are 22 on core 0		
	on Shannon, CIC0 out33 event number are 22 on core0		
	map this event 22 to INT6 */
	gpCGEM_regs->INTMUX1 = (22<<CSL_CGEM_INTMUX1_INTSEL6_SHIFT);

	//enable INT6
	CPU_interrupt_enable(1<<6);

}

void HyperLink_Int_Handle()
{
	Uint32 intVector;
	
	/*read the HyperLink interrupt vector*/
	intVector= gpHyperLinkRegs->INT_PRI_VEC;

	while(0==(intVector&CSL_VUSR_INT_PRI_VEC_NOINTPEND_MASK))
	{
		if(0==intVector)//HyperLink error is routed to vector 0 for this test.
		{
			//print status
			print_HyperLink_status();

			/*disable all portal or remote register operation
			This bit should be set before iloop or reset bits are changed.*/
			gpHyperLinkRegs->CTL |= CSL_VUSR_CTL_SERIAL_STOP_MASK;

			/*Wait until no Remote Pending Request*/
			while(gpHyperLinkRegs->STS&CSL_VUSR_STS_RPEND_MASK);

			/*Reset*/
			//gpHyperLinkRegs->CTL |= CSL_VUSR_CTL_RESET_MASK;

			//clear error status
			gpHyperLinkRegs->STS |= CSL_VUSR_STS_LERROR_MASK
				|CSL_VUSR_STS_RERROR_MASK;
			
			/*release from Reset*/
			//gpHyperLinkRegs->CTL &= ~(CSL_VUSR_CTL_RESET_MASK);

			/*enable operation*/
			gpHyperLinkRegs->CTL &= ~(CSL_VUSR_CTL_SERIAL_STOP_MASK);

		}

		printf("HyperLink interrupt %d happens at TSC= %u\n",
			intVector, HyperLinkIntTSCL);

		/*write back to clear that interrupt*/
		gpHyperLinkRegs->INT_PRI_VEC= intVector;
		gpHyperLinkRegs->INT_CLR= (1<<intVector);

		/*read the HyperLink interrupt vector*/
		intVector= gpHyperLinkRegs->INT_PRI_VEC;
	}
}

interrupt void HyperLinkISR(void)
{
	HyperLinkIntTSCL= TSCL; 	//save the TSC when the interrupt happens

	/* Disable the host interrupt */
	gpCIC0_regs->HINT_ENABLE_CLR_INDEX_REG = uiHyLink_pend_host_event_num;

	if(gpCIC0_regs->ENA_STATUS_REG[CSL_INTC0_VUSR_INT_O/32]&(1<<(CSL_INTC0_VUSR_INT_O%32)))
	{
		/* Clear HyperLink Interrupt events (111) in INTC*/
		KeyStone_CIC_clear_system_event(gpCIC0_regs, CSL_INTC0_VUSR_INT_O);

		HyperLink_Int_Handle();
	}
	/* Enable the host interrupt */
	gpCIC0_regs->HINT_ENABLE_SET_INDEX_REG = uiHyLink_pend_host_event_num;
}


