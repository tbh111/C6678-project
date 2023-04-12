/******************************************************************************

  Copyright (C), 2013, Texas Instrument.

 ******************************************************************************
  File Name     : UART_Interrupt.c
  Version       : Initial Draft
  Author        : Vincent Han
  Created       : 2013/5/17
  Last Modified :
  Description   :  UART interrupt setup and handling 

  History       :
  1.Date        : May 17, 2013
    Author      : Vincent Han
    Modification: Created file for KeyStone
  2.Date        : July 27, 2013
    Author      : Brighton Feng
    Modification: to support selective interrupt enable for different case

******************************************************************************/

#include <stdio.h>
#include <csl_bootcfgAux.h>
#include <csl_cpintc.h>
#include "KeyStone_common.h"
#include "KeyStone_UART_Init_drv.h"
#include "UART_interrupt.h"

CSL_CPINTCRegs* CIC_Regs;

//RX buffer used by RX interrupt
#pragma DATA_ALIGN(UART_Rx_Buf,16);
/* Need to be the same with Rx FIFO trigger level size */
unsigned char UART_Rx_Buf[RX_BUF_BYTE_SIZE]; 

volatile Bool bUartEchoBack; //Echo the received data back

#pragma DATA_ALIGN(UART_Echo_Buf,16);
unsigned char UART_Echo_Buf[2*RX_BUF_BYTE_SIZE]; 

Uint32 uiCIC_out_num; //first CIC output for UART

//echo back the received characters
void UART_Echo_back(Uint32 rx_cnt, Uint32 uartNum)
{
	Uint32 echo_cnt=0, i;
	
    for (i = 0; i < rx_cnt; i++)
    {
        printf("%c", UART_Rx_Buf[i]);
    }
    fflush(stdout);

	//copy the data from RX buffer to TX buffer for echo back
	for(i = 0; i < rx_cnt; i++)
	{
		UART_Echo_Buf[echo_cnt++]= UART_Rx_Buf[i];
        if((UART_Rx_Buf[i] == 13)&& (UART_Rx_Buf[i+1] != 13)) 	//return without new line
        {
            UART_Echo_Buf[echo_cnt++] = 10; 	//add new line after return
        }
	}

	//write back the characters
    KeyStone_UART_write(UART_Echo_Buf, echo_cnt, uartNum);
}

interrupt void KeyStone_UART_Rx_ISR()
{
	Uint32 rx_cnt = 0;

    /* Disable the CIC out for interrupt processing */
    CIC_Regs->HINT_ENABLE_CLR_INDEX_REG = uiCIC_out_num+1;

    // Interrupt from UART0
    if(CIC_Regs->RAW_STATUS_REG[CSL_INTC0_URXEVT>>5] & (1<<(CSL_INTC0_URXEVT%32)))
    {
		rx_cnt= KeyStone_UART_read(UART_Rx_Buf, RX_BUF_BYTE_SIZE, 0);

		CIC_Regs->STATUS_CLR_INDEX_REG = CSL_INTC0_URXEVT;

		//echo back the received characters
		if(bUartEchoBack)
			UART_Echo_back(rx_cnt, 0);
    }
#ifdef CSL_UART_B
	// Interrupt from UART1
    if(CIC_Regs->RAW_STATUS_REG[CSL_INTC0_URXEVT1>>5] & (1<<(CSL_INTC0_URXEVT1%32)))
    {
		rx_cnt= KeyStone_UART_read(UART_Rx_Buf, RX_BUF_BYTE_SIZE, CSL_UART_B);

		CIC_Regs->STATUS_CLR_INDEX_REG = CSL_INTC0_URXEVT1;

		//echo back the received characters
		if(bUartEchoBack)
			UART_Echo_back(rx_cnt, CSL_UART_B);
    }
#endif

    /* Enable the CIC out when finish interrupt processing */
    CIC_Regs->HINT_ENABLE_SET_INDEX_REG = uiCIC_out_num+1;
    
}

interrupt void KeyStone_UART_Error_ISR()
{
    /* Disable the CIC out for interrupt processing */
    CIC_Regs->HINT_ENABLE_CLR_INDEX_REG = uiCIC_out_num;

	// Interrupt from UART0
    if(CIC_Regs->RAW_STATUS_REG[CSL_INTC0_UARTINT>>5] & (1<<(CSL_INTC0_UARTINT%32))) 
    {
    	KeyStone_UART_Error_Handler(0);
		CIC_Regs->STATUS_CLR_INDEX_REG = CSL_INTC0_UARTINT;
    }
#ifdef CSL_UART_B
	// Interrupt from UART1
    if(CIC_Regs->RAW_STATUS_REG[CSL_INTC0_UARTINT1>>5] & (1<<(CSL_INTC0_UARTINT1%32))) 
    {
    	KeyStone_UART_Error_Handler(CSL_UART_B);
		CIC_Regs->STATUS_CLR_INDEX_REG = CSL_INTC0_UARTINT1;
    }
#endif

    /* Enable the CIC out when finish interrupt processing */
    CIC_Regs->HINT_ENABLE_SET_INDEX_REG = uiCIC_out_num;
    
}

void UART_EDMA_complete_handler(Uint32 tpccNum)
{
	Uint32 IPR, IPRH;
	
	IPRH = gpEDMA_CC_regs[tpccNum]->TPCC_IPRH;
    IPR  = gpEDMA_CC_regs[tpccNum]->TPCC_IPR;

	//disable event to avoid EDMA event miss error
	gpEDMA_CC_regs[tpccNum]->TPCC_EECRH= IPRH;
    gpEDMA_CC_regs[tpccNum]->TPCC_EECR = IPR;

	gpEDMA_CC_regs[tpccNum]->TPCC_ICRH= IPRH;
    gpEDMA_CC_regs[tpccNum]->TPCC_ICR = IPR;

	printf("EDMA CC%d for UART transfer complete.\n", tpccNum);
            
    gpEDMA_CC_regs[tpccNum]->TPCC_IEVAL = 1;

}

/****************** Process for EDMA Tx Interrupt *******************/
interrupt void KeyStone_UART_EDMA_ISR()
{
    /* Disable the CIC out for interrupt processing */
    CIC_Regs->HINT_ENABLE_CLR_INDEX_REG = uiCIC_out_num+2;

     // Interrupt from UART0
    if(CIC_Regs->RAW_STATUS_REG[CSL_INTC0_CPU_3_2_EDMACC_GINT>>5] & (0x00000001<<(CSL_INTC0_CPU_3_2_EDMACC_GINT%32)))
    {
    	UART_EDMA_complete_handler(2);
	    CIC_Regs->STATUS_CLR_INDEX_REG = CSL_INTC0_CPU_3_2_EDMACC_GINT;
    }
    
    /* Enable the CIC out when finish interrupt processing */
    CIC_Regs->HINT_ENABLE_SET_INDEX_REG = uiCIC_out_num+2;
    
}

void KeyStone_UART_Interrupts_Init(Bool bRxIntEnable, Bool bEDMAInterruptEnable)
{
	if(C6670_EVM==gDSP_board_type
		||DUAL_NYQUIST_EVM==gDSP_board_type
		||TCI6614_EVM==gDSP_board_type)
	{	
		uiCIC_out_num= 64;//CIC out 64 is INTC input 22
	}
	else if(C6678_EVM==gDSP_board_type)
	{	
		uiCIC_out_num= 33;//in C6678, CIC out 33 is also INTC input 22
	}
	else
	{
		puts("Unknown DSP board type!");
		return;
	}	

	if(DNUM<4) //for core 0, 1, 2, 3
		CIC_Regs= gpCIC0_regs;
	else
		CIC_Regs= gpCIC1_regs;
    
	/* Disable Global host interrupts. */
	CIC_Regs->GLOBAL_ENABLE_HINT_REG= 0;

	/* Configure no nesting support in the CPINTC Module. */
	CIC_Regs->CONTROL_REG= ((CIC_Regs->CONTROL_REG&
		~CSL_CPINTC_CONTROL_REG_NEST_MODE_MASK)|
		(CPINTC_NO_NESTING<<CSL_CPINTC_CONTROL_REG_NEST_MODE_SHIFT));

	/*map UART ERR INT, RX EVT, and EDMA CC2 GINT to CIC out*/
	KeyStone_CIC_event_map(CIC_Regs, CSL_INTC0_UARTINT, uiCIC_out_num);
	KeyStone_CIC_event_map(CIC_Regs, CSL_INTC0_URXEVT, uiCIC_out_num+1);
#ifdef CSL_INTC0_UARTINT1
	KeyStone_CIC_event_map(CIC_Regs, CSL_INTC0_UARTINT1, uiCIC_out_num);
	KeyStone_CIC_event_map(CIC_Regs, CSL_INTC0_URXEVT1, uiCIC_out_num+1);
#endif
	KeyStone_CIC_event_map(CIC_Regs, CSL_INTC0_CPU_3_2_EDMACC_GINT, uiCIC_out_num+2);

	/* Enable Global host interrupts. */
	CIC_Regs->GLOBAL_ENABLE_HINT_REG= 1;

	/* map UART ERR interrupt to INT4 */
	/* map UART RX interrupt to INT5 */
	/* map EDMA GINT to INT6 */
#ifdef CSL_GEM_CIC0_OUT_64_PLUS_10_MUL_N 	//for C6670, TCI6614
	gpCGEM_regs->INTMUX1 = 
		(CSL_GEM_CIC0_OUT_64_PLUS_10_MUL_N<<CSL_CGEM_INTMUX1_INTSEL4_SHIFT)|\
		(CSL_GEM_CIC0_OUT_65_PLUS_10_MUL_N<<CSL_CGEM_INTMUX1_INTSEL5_SHIFT)|\
		(CSL_GEM_CIC0_OUT_66_PLUS_10_MUL_N<<CSL_CGEM_INTMUX1_INTSEL6_SHIFT);
#else  //for C6678
	gpCGEM_regs->INTMUX1 = 
		(CSL_GEM_INTC0_OUT_33_PLUS_11_MUL_N_OR_INTC0_OUT_33_PLUS_11_MUL_N_MINUS_4<<CSL_CGEM_INTMUX1_INTSEL4_SHIFT)|\
		(CSL_GEM_INTC0_OUT_34_PLUS_11_MUL_N_OR_INTC0_OUT_34_PLUS_11_MUL_N_MINUS_4<<CSL_CGEM_INTMUX1_INTSEL5_SHIFT)|\
		(CSL_GEM_INTC0_OUT_35_PLUS_11_MUL_N_OR_INTC0_OUT_35_PLUS_11_MUL_N_MINUS_4<<CSL_CGEM_INTMUX1_INTSEL6_SHIFT);
#endif
	/* enable INT4, 5, 6 */
	CPU_interrupt_enable((1<<4)|(bRxIntEnable<<5)|(bEDMAInterruptEnable<<6));
}

