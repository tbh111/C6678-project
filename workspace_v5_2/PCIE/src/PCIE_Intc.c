/*  ============================================================================
 *     Copyright (C) 2013 Texas Instruments Incorporated.       *
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
Interrupts configuration and ISR for PCIE
 * =============================================================================
 *  Revision History
 *  ===============
 *  August 22, 2013 Brighton Feng file created
 * =============================================================================
 */

#include <stdio.h>
#include <csl_pscAux.h>
#include <c6x.h>
#include "KeyStone_common.h"
#include "KeyStone_PCIE_init_drv.h"

int uiPCIE_pend_host_event_num;

Uint32 PCIE_IntTSCL; //TSCL value when PCIE MSI interrupt happens
Uint32 gudInterruptFlag = 0;
interrupt void PCIE_MSI_ISR(void)
{
	int i;
	
	Uint32 MSI_IRQ_STATUS, MSI_number;
	
	PCIE_IntTSCL= TSCL;
	
	MSI_IRQ_STATUS= gpPCIE_app_regs->MSIX_IRQ[DNUM].MSI_IRQ_STATUS;

	//clear the interrupt
	gpPCIE_app_regs->MSIX_IRQ[DNUM].MSI_IRQ_STATUS= MSI_IRQ_STATUS;

	for(i=0; i<4; i++)
	{
		if((MSI_IRQ_STATUS>>i)&1)
		{
			MSI_number= DNUM+i*8;

			printf("PCIE MSI%d interrupt generated!\n", MSI_number);
		}
	}

	//indicate end-of-interrupt in IRQ_EOI register
	gpPCIE_app_regs->IRQ_EOI = DNUM+4;
	gudInterruptFlag = 1;
}

char * PCIE_ERR_Str[]=
{
	"PCIE System error (OR of fatal, nonfatal, correctable errors) (RC mode only)",
	"PCIe fatal error (RC mode only)",
	"PCIe non-fatal error (RC mode only)",
	"PCIe correctable error (RC mode only)",
	"PCIE AXI Error due to fatal condition in AXI bridge (EP/RC modes)           ",
	"PCIe advanced error (RC mode only)"
};

char * PCIE_PMRST_Str[]=
{
	"PCIE Power management turn-off message interrupt (EP mode only)",
	"PCIE Power management ack message interrupt (RC mode only)",
	"PCIE Power management event interrupt (RC mode only)",
	"PCIE Link request reset interrupt (hot reset or link down) (RC mode only)"
};
/*ISR for PCIE error, power management, reset*/
interrupt void PCIE_ERR_PMRST_ISR(void)
{
	int i;
	Uint32 IRQ_STATUS;
	
	/* Disable the host interrupt */
	gpCIC0_regs->HINT_ENABLE_CLR_INDEX_REG = uiPCIE_pend_host_event_num;

	if(gpCIC0_regs->ENA_STATUS_REG[CSL_INTC0_PCIEXPRESS_ERR_INT/32]&(1<<(CSL_INTC0_PCIEXPRESS_ERR_INT%32)))
	{
		/* Clear PCIE Interrupt events in INTC*/
		KeyStone_CIC_clear_system_event(gpCIC0_regs, CSL_INTC0_PCIEXPRESS_ERR_INT);

		IRQ_STATUS= gpPCIE_app_regs->ERR_IRQ_STATUS;

		//clear the interrupt
		gpPCIE_app_regs->ERR_IRQ_STATUS= IRQ_STATUS;

		for(i=0;i<6;i++)
		{
			if((IRQ_STATUS>>i)&1)
				printf("%s\n", PCIE_ERR_Str[i]);
		}
		
		//indicate end-of-interrupt in IRQ_EOI register
		gpPCIE_app_regs->IRQ_EOI = 12;
	}

	if(gpCIC0_regs->ENA_STATUS_REG[CSL_INTC0_PCIEXPRESS_PM_INT/32]&(1<<(CSL_INTC0_PCIEXPRESS_PM_INT%32)))
	{
		/* Clear PCIE Interrupt events in INTC*/
		KeyStone_CIC_clear_system_event(gpCIC0_regs, CSL_INTC0_PCIEXPRESS_PM_INT);

		IRQ_STATUS= gpPCIE_app_regs->PMRST_IRQ_STATUS;

		//clear the interrupt
		gpPCIE_app_regs->PMRST_IRQ_STATUS= IRQ_STATUS;

		for(i=0;i<4;i++)
		{
			if((IRQ_STATUS>>i)&1)
				printf("%s\n", PCIE_PMRST_Str[i]);
		}
		
		//indicate end-of-interrupt in IRQ_EOI register
		gpPCIE_app_regs->IRQ_EOI = 13;
	}

	/* Enable the host interrupt */
	gpCIC0_regs->HINT_ENABLE_SET_INDEX_REG = uiPCIE_pend_host_event_num;
}

void PCIE_Interrupts_Init(void)
{
	/* Disable Global host interrupts. */
	gpCIC0_regs->GLOBAL_ENABLE_HINT_REG= 0;

	if(C6670_EVM==gDSP_board_type
		||DUAL_NYQUIST_EVM==gDSP_board_type
		||TCI6614_EVM==gDSP_board_type)
	{
		uiPCIE_pend_host_event_num= 64;

		/*map PCIE Interrupt events to CIC0 out64*/
		KeyStone_CIC_event_map(gpCIC0_regs, CSL_INTC0_PCIEXPRESS_ERR_INT, 64);
		KeyStone_CIC_event_map(gpCIC0_regs, CSL_INTC0_PCIEXPRESS_PM_INT, 64);
	}
	else if(C6678_EVM==gDSP_board_type)
	{	
		uiPCIE_pend_host_event_num= 33;

		/*map PCIE Interrupt events to CIC0 out33*/
		KeyStone_CIC_event_map(gpCIC0_regs, CSL_INTC0_PCIEXPRESS_ERR_INT, 33);
		KeyStone_CIC_event_map(gpCIC0_regs, CSL_INTC0_PCIEXPRESS_PM_INT, 33);
	}
	else
	{
		puts("Unknown DSP board type!");
		return;
	}	
	
	/* Enable Global host interrupts. */
	gpCIC0_regs->GLOBAL_ENABLE_HINT_REG= 1;

	//clear all interrupt status registers of PCIE
	KeyStone_PCIE_clear_interrupts();

	/*on Nyquist, CIC0 out64 event number are 22 on core 0; on Shannon, 
	CIC0 out33 event number are 22 on core0. Map this event 22 to INT5.
	Map PCIE MSI event to INT4*/
	gpCGEM_regs->INTMUX1 = (22<<CSL_CGEM_INTMUX1_INTSEL5_SHIFT)
		|(CSL_GEM_PCIEXPRESS_MSI_INTN<<CSL_CGEM_INTMUX1_INTSEL4_SHIFT);

	//enable INT4 and INT5
	CPU_interrupt_enable((1<<4)|(1<<5));
}

