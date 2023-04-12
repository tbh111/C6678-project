/*  ============================================================================
 *     Copyright (C) 2013 Texas Instruments Incorporated.       *
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
Interrupts configuration for SPI
 * =============================================================================
 *  Revision History
 *  ===============
 *  23-July-2013 cheng  file created
 *  19-October-2013 Brighton  update to support multiple ports
 * =============================================================================
 */

#include "SPI_Intc.h"

int uiSPI_event_num = 0;

void SPI_Interrupts_Init(void)
{
	if(C6670_EVM==gDSP_board_type
		||DUAL_NYQUIST_EVM==gDSP_board_type
		||TCI6614_EVM==gDSP_board_type)
	{
		uiSPI_event_num= 64;  	
	}
	else if(C6678_EVM==gDSP_board_type)
	{	
	    //PC: can also map event 34 to CorePac Primary interrupts(event 23 :CIC0_OUT(34 +11*n))
	    //for CorePac 0-3: CIC0; For CorPac4-7 is CIC1
		uiSPI_event_num= 33;
	}
	else
	{
		puts("Unknown DSP board type!");
		return;
	}	
    
	/* Disable Global host interrupts. */
	gpCIC0_regs->GLOBAL_ENABLE_HINT_REG= 0;   

	/*map SPI Interrupt events (54 for SPIINT0) to CIC0 INTC0 out*/
	KeyStone_CIC_event_map(gpCIC0_regs, CSL_INTC0_SPIINT0, uiSPI_event_num);

   	
	/* Enable Global host interrupts. */
	gpCIC0_regs->GLOBAL_ENABLE_HINT_REG= 1;


    //PC TEST: if change CorePac primary event number to 23, uiSPI_event_num should be 65 for 6614; 34 for 6608. 
#ifdef CSL_GEM_CIC0_OUT_64_PLUS_10_MUL_N 	//for C6670, TCI6614
	gpCGEM_regs->INTMUX1 = 
		(CSL_GEM_CIC0_OUT_64_PLUS_10_MUL_N<<CSL_CGEM_INTMUX1_INTSEL6_SHIFT);
#else  //for C6678
	gpCGEM_regs->INTMUX1 = 
		(CSL_GEM_INTC0_OUT_33_PLUS_11_MUL_N_OR_INTC0_OUT_33_PLUS_11_MUL_N_MINUS_4<<CSL_CGEM_INTMUX1_INTSEL6_SHIFT);
#endif

	//enable INT6
	CPU_interrupt_enable(1<<6);
}

char * spi_int_str[]=
{
	"Error",
	"receive buffer full",
	"receive buffer overrun",
	"transmit buffer empty"
};

void SPI_Interrupt_Handler()
{
	Uint32 intVector;
	

	intVector=(gpSPI_regs->INTVEC[0]&CSL_SPI_INTVEC_INTVECT_MASK)>>
		CSL_SPI_INTVEC_INTVECT_SHIFT;
	while((0x11<=intVector)&&(intVector<=0x14))
	{
	    printf("SPI %s interrupt happened at %u\n",
	    	spi_int_str[intVector-0x11], TSCL);

		intVector=(gpSPI_regs->INTVEC[0]&CSL_SPI_INTVEC_INTVECT_MASK)>>
			CSL_SPI_INTVEC_INTVECT_SHIFT;

		if(gpSPI_regs->SPIFLG&CSL_SPI_SPIFLG_BITERRFLG_MASK)
		{
			puts("  TX bit error!");
			gpSPI_regs->SPIFLG |= CSL_SPI_SPIFLG_BITERRFLG_MASK;
		}
	}
}

interrupt void SPI_ISR()
{
	/* Disable the host interrupt */
	gpCIC0_regs->HINT_ENABLE_CLR_INDEX_REG = uiSPI_event_num;

	if(gpCIC0_regs->ENA_STATUS_REG[CSL_INTC0_SPIINT0/32]&(1<<(CSL_INTC0_SPIINT0%32)))
	{
		/* Clear SPI Interrupt events in CIC*/
		KeyStone_CIC_clear_system_event(gpCIC0_regs, CSL_INTC0_SPIINT0);		

		SPI_Interrupt_Handler();		
	}


	/* Enable the host interrupt */
	gpCIC0_regs->HINT_ENABLE_SET_INDEX_REG = uiSPI_event_num;
}

