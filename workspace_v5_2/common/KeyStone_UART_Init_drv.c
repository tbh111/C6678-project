/******************************************************************************

  Copyright (C), 2013, Texas Instrument.

 ******************************************************************************
  File Name     : KeyStone_UART_Init_drv.c
  Version       : Initial Draft
  Author        : Vincent Han
  Created       : 2013/5/17
  Last Modified :
  Description   :  UART configuration and driver on KeyStone 

  History       :
  1.Date        : May 17, 2013
    Author      : Vincent Han
    Modification: Created file for KeyStone
  2.Date        : July 27, 2013
    Author      : Brighton Feng
    Modification: update APIs

******************************************************************************/

#include <stdio.h>
#include <csl_edma3.h>
#include "KeyStone_UART_Init_drv.h"
#include "KeyStone_common.h"
#include "C6x.h"

CSL_UartRegs *gpUartRegs[CSL_UART_PER_CNT]=
{
#ifdef CSL_UART_A_REGS
	(CSL_UartRegs *)CSL_UART_A_REGS
#else
	(CSL_UartRegs *)CSL_UART_REGS
#endif
#ifdef CSL_UART_B_REGS
    ,
	(CSL_UartRegs *)CSL_UART_B_REGS
#endif
};

/*talbe to indicate EDMA channels for each UART*/
EDMA_CC_Chanel_TC UART_EDMA_TX_channels[CSL_UART_PER_CNT]=
{
	/*CC,       Channel,           TC*/
#ifdef CSL_TPCC2_UTXEVT_A //for TCI6614
	{2, CSL_TPCC2_UTXEVT_A, 0} /*for UART 0*/
#else //for C6670 and C6678
	{2, CSL_TPCC2_UTXEVT, 0} /*for UART 0*/
#endif
#ifdef CSL_TPCC2_UTXEVT_B //for TCI6614
	,{2, CSL_TPCC2_UTXEVT_B, 0}  /*for UART 1*/
#endif
};

UART_Tx_Master txMaster[CSL_UART_PER_CNT];

void KeyStone_UART_init(UART_Config *pUARTCfg, Uint32 uartNum)
{
	Uint32 divider;
	Uint32 uartDll, uartDlh;
	Uint32 osmFactor;
	Uint32 effectRate;

	CSL_UartRegs *localUartRegs = gpUartRegs[uartNum];
    
    if(uartNum >= CSL_UART_PER_CNT)
    {
        printf("Invalid UART number!\n");
        return;
    }

	//make sure TX and RX is reset in case it was enabled from last run.
    CSL_FINS(localUartRegs->PWREMU_MGMT, UART_PWREMU_MGMT_UTRST, CSL_UART_PWREMU_MGMT_UTRST_RESET);
    CSL_FINS(localUartRegs->PWREMU_MGMT, UART_PWREMU_MGMT_URRST, CSL_UART_PWREMU_MGMT_URRST_RESET);

	if(pUARTCfg->osmSel == OVER_SAMPLING_13X)
	{
		osmFactor = 13;
	}
	else 
	{
		osmFactor = 16;
	}
    
    CSL_FINS(localUartRegs->MDR, UART_MDR_OSM_SEL, pUARTCfg->osmSel);
	
	divider = (Uint32)((float)(pUARTCfg->DSP_Core_Speed_Hz)/
		(float)(pUARTCfg->baudRate * 6 * osmFactor)+0.5f);

	effectRate= pUARTCfg->DSP_Core_Speed_Hz/(6*divider*osmFactor);
	if(pUARTCfg->baudRate!= effectRate)
		printf("Required baud rate %d, effective baud rate %d!\n", pUARTCfg->baudRate, effectRate);

	uartDll = divider & 0xFF;
	uartDlh = (divider & 0xFF00)>>8;

    CSL_FINS(localUartRegs->DLH, UART_DLL_DLL, uartDlh);
    CSL_FINS(localUartRegs->DLL, UART_DLH_DLH, uartDll);

	//FIFO mode is used
    CSL_FINS(localUartRegs->FCR, UART_FCR_FIFOEN, CSL_UART_FCR_FIFOEN_ENABLE);
    CSL_FINS(localUartRegs->FCR, UART_FCR_RXFIFTL, pUARTCfg->fifoRxTriBytes);
    CSL_FINS(localUartRegs->FCR, UART_FCR_DMAMODE1, CSL_UART_FCR_DMAMODE1_ENABLE);

    if (pUARTCfg->parityMode == PARITY_DISABLE)
	{
        CSL_FINS(localUartRegs->LCR, UART_LCR_PEN, CSL_UART_LCR_PEN_DISABLE);
        CSL_FINS(localUartRegs->LCR, UART_LCR_EPS, CSL_UART_LCR_EPS_ODD);
        CSL_FINS(localUartRegs->LCR, UART_LCR_SP,  CSL_UART_LCR_SP_DISABLE);
	}
	else if (pUARTCfg->parityMode == ODD_PARITY_ENABLE_SET1)
	{
        CSL_FINS(localUartRegs->LCR, UART_LCR_PEN, CSL_UART_LCR_PEN_ENABLE);
        CSL_FINS(localUartRegs->LCR, UART_LCR_EPS, CSL_UART_LCR_EPS_ODD);
        CSL_FINS(localUartRegs->LCR, UART_LCR_SP,  CSL_UART_LCR_SP_DISABLE);
	}
    else if (pUARTCfg->parityMode == EVEN_PARITY_ENABLE_SET1)
	{
        CSL_FINS(localUartRegs->LCR, UART_LCR_PEN, CSL_UART_LCR_PEN_ENABLE);
        CSL_FINS(localUartRegs->LCR, UART_LCR_EPS, CSL_UART_LCR_EPS_EVEN);
        CSL_FINS(localUartRegs->LCR, UART_LCR_SP,  CSL_UART_LCR_SP_DISABLE);
	}
    else if (pUARTCfg->parityMode == STICK_PARITY_ENABLE_SET)
	{
        CSL_FINS(localUartRegs->LCR, UART_LCR_PEN, CSL_UART_LCR_PEN_ENABLE);
        CSL_FINS(localUartRegs->LCR, UART_LCR_EPS, CSL_UART_LCR_EPS_ODD);
        CSL_FINS(localUartRegs->LCR, UART_LCR_SP,  CSL_UART_LCR_SP_ENABLE);
	}
    else if (pUARTCfg->parityMode == STICK_PARITY_ENABLE_CLR)
	{
        CSL_FINS(localUartRegs->LCR, UART_LCR_PEN, CSL_UART_LCR_PEN_ENABLE);
        CSL_FINS(localUartRegs->LCR, UART_LCR_EPS, CSL_UART_LCR_EPS_EVEN);
        CSL_FINS(localUartRegs->LCR, UART_LCR_SP,  CSL_UART_LCR_SP_ENABLE);
	}
    else
    {
        printf("Invalid UART Parity configuration!\n");
        return;
    }

    CSL_FINS(localUartRegs->LCR, UART_LCR_STB, pUARTCfg->stopMode);
    CSL_FINS(localUartRegs->LCR, UART_LCR_WLS, pUARTCfg->dataLen);

    if (pUARTCfg->autoFlow== AUTO_FLOW_DIS)
    {
        CSL_FINS(localUartRegs->MCR, UART_MCR_AFE, CSL_UART_MCR_AFE_DISABLE);
        CSL_FINS(localUartRegs->MCR, UART_MCR_RTS, CSL_UART_MCR_RTS_DISABLE);
    }
    else if (pUARTCfg->autoFlow == AUTO_FLOW_CTS_EN)
    {
        CSL_FINS(localUartRegs->MCR, UART_MCR_AFE, CSL_UART_MCR_AFE_ENABLE);
        CSL_FINS(localUartRegs->MCR, UART_MCR_RTS, CSL_UART_MCR_RTS_DISABLE);
    }
    else if (pUARTCfg->autoFlow == AUTO_FLOW_RTS_CTS_EN)
    {
        CSL_FINS(localUartRegs->MCR, UART_MCR_AFE, CSL_UART_MCR_AFE_ENABLE);
        CSL_FINS(localUartRegs->MCR, UART_MCR_RTS, CSL_UART_MCR_RTS_ENABLE);
    }
    else
    {
        printf("Invalid UART auto flow control configuration!\n");
        return;
    }

    /* Setup the UART to loopback mode */ 
    CSL_FINS(localUartRegs->MCR, UART_MCR_LOOP, pUARTCfg->bLoopBackEnable);

	//enable UART
    CSL_FINS(localUartRegs->PWREMU_MGMT, UART_PWREMU_MGMT_UTRST, CSL_UART_PWREMU_MGMT_UTRST_ENABLE);
    CSL_FINS(localUartRegs->PWREMU_MGMT, UART_PWREMU_MGMT_URRST, CSL_UART_PWREMU_MGMT_URRST_ENABLE);
    CSL_FINS(localUartRegs->PWREMU_MGMT, UART_PWREMU_MGMT_FREE,  CSL_UART_PWREMU_MGMT_FREE_STOP);
	
	//enable receive interrupts
    CSL_FINS(localUartRegs->IER, UART_IER_ELSI, CSL_UART_IER_ELSI_ENABLE);
    CSL_FINS(localUartRegs->IER, UART_IER_ERBI, CSL_UART_IER_ERBI_ENABLE);

    //localUartRegs->IER = 0xFFFFFFFF;

	//record the master used for transmit for this UART
    txMaster[uartNum]= pUARTCfg->txMaster; 

	return;
}

/*write number of "byteLen" data in the "buffer" through UART of "uartNum".
EDMA or DSP core is used depends on the "txMaster" configuration*/
void KeyStone_UART_write(unsigned char *buffer, 
	unsigned int byteLen, Uint32 uartNum)
{
    int i = 0;
    Uint32 tpccNum, uartEdmaTxCh;

	CSL_UartRegs *localUartRegs = gpUartRegs[uartNum];
    
    if(uartNum >= CSL_UART_PER_CNT)
    {
        printf("Invalid UART number!\n");
        return;
    }

    if(txMaster[uartNum] == UART_USE_EDMA_TO_TX) //transmit with EDMA
    {
    	tpccNum= UART_EDMA_TX_channels[uartNum].CC_num;
    	uartEdmaTxCh= UART_EDMA_TX_channels[uartNum].channel_num;
		
    	//wait for previous EDMA transfer complete 
        while(gpEDMA_CC_regs[tpccNum]->PARAMSET[uartEdmaTxCh].A_B_CNT & 0xFFFF0000);

		//wait for UART FIFO empty
		while(!((localUartRegs->LSR) & CSL_UART_LSR_THRE_MASK));

		//clear potential event or error
		if(uartEdmaTxCh<32)
		{
	        gpEDMA_CC_regs[tpccNum]->TPCC_ECR= (1<<uartEdmaTxCh);
	        gpEDMA_CC_regs[tpccNum]->TPCC_SECR= (1<<uartEdmaTxCh);
			gpEDMA_CC_regs[tpccNum]->TPCC_EMCR= (1<<uartEdmaTxCh);
		}
		else
		{
	        gpEDMA_CC_regs[tpccNum]->TPCC_ECRH= (1<<(uartEdmaTxCh-32));
	        gpEDMA_CC_regs[tpccNum]->TPCC_SECRH= (1<<(uartEdmaTxCh-32));
			gpEDMA_CC_regs[tpccNum]->TPCC_EMCRH= (1<<(uartEdmaTxCh-32));
		}

		gpEDMA_CC_regs[tpccNum]->PARAMSET[uartEdmaTxCh].OPT=
				CSL_EDMA3_OPT_MAKE(CSL_EDMA3_ITCCH_DIS,
						            CSL_EDMA3_TCCH_DIS,
						            CSL_EDMA3_ITCINT_DIS,
						            CSL_EDMA3_TCINT_EN,
						            uartEdmaTxCh,
						            CSL_EDMA3_TCC_NORMAL,
						            CSL_EDMA3_FIFOWIDTH_NONE,
						            CSL_EDMA3_STATIC_DIS,
						            CSL_EDMA3_SYNC_A,
						            CSL_EDMA3_ADDRMODE_INCR,
						            CSL_EDMA3_ADDRMODE_INCR);
#ifdef _BIG_ENDIAN
		/*in big endian mode, the expected data byte is at (address+3)*/
		gpEDMA_CC_regs[tpccNum]->PARAMSET[uartEdmaTxCh].SRC= (Uint32)GLOBAL_ADDR(buffer-3);
#else
		gpEDMA_CC_regs[tpccNum]->PARAMSET[uartEdmaTxCh].SRC= (Uint32)GLOBAL_ADDR(buffer);
#endif
        /*Write 4 Bytes as ACNT and set SRCBIDX as 1Byte due to 
        the THR need be written in word in big endian mode*/
        gpEDMA_CC_regs[tpccNum]->PARAMSET[uartEdmaTxCh].A_B_CNT = CSL_EDMA3_CNT_MAKE(4, byteLen); 
		gpEDMA_CC_regs[tpccNum]->PARAMSET[uartEdmaTxCh].DST= (Uint32)(&localUartRegs->THR);
		gpEDMA_CC_regs[tpccNum]->PARAMSET[uartEdmaTxCh].SRC_DST_BIDX= CSL_EDMA3_BIDX_MAKE(1, 0);
		gpEDMA_CC_regs[tpccNum]->PARAMSET[uartEdmaTxCh].LINK_BCNTRLD= CSL_EDMA3_LINKBCNTRLD_MAKE(0xFFFF, byteLen);
		gpEDMA_CC_regs[tpccNum]->PARAMSET[uartEdmaTxCh].SRC_DST_CIDX= 0;
		gpEDMA_CC_regs[tpccNum]->PARAMSET[uartEdmaTxCh].CCNT= 1;

        /*Enable channel. Trigger the first transfer manually.
        Following UART event can trigger more data transfer*/
        if(uartEdmaTxCh >= 32)
        {
            gpEDMA_CC_regs[tpccNum]->TPCC_EESRH = 1<<(uartEdmaTxCh-32);
            gpEDMA_CC_regs[tpccNum]->TPCC_ESRH = 1<<(uartEdmaTxCh-32);
        }
        else
        {
            gpEDMA_CC_regs[tpccNum]->TPCC_EESR = 1<<uartEdmaTxCh;
            gpEDMA_CC_regs[tpccNum]->TPCC_ESR = 1<<uartEdmaTxCh;
        }
        
    }
    else 	//transmit with CPU
    {   
        for (i = 0; i < byteLen; i++)
        {        
			//wait for UART FIFO empty
			while(!((localUartRegs->LSR) & CSL_UART_LSR_THRE_MASK));

            localUartRegs->THR = buffer[i];
        }
    }
    
	return;
}

//wait for TX complete
void KeyStone_UART_TX_wait(Uint32 uartNum)
{
    Uint32 tpccNum, uartEdmaTxCh;

	CSL_UartRegs *localUartRegs = gpUartRegs[uartNum];
    
    if(uartNum >= CSL_UART_PER_CNT)
    {
        printf("Invalid UART number!\n");
        return;
    }

    if(txMaster[uartNum] == UART_USE_EDMA_TO_TX) //transmit with EDMA
    {
    	tpccNum= UART_EDMA_TX_channels[uartNum].CC_num;
    	uartEdmaTxCh= UART_EDMA_TX_channels[uartNum].channel_num;
		
    	//wait for previous EDMA transfer complete 
        while(gpEDMA_CC_regs[tpccNum]->PARAMSET[uartEdmaTxCh].A_B_CNT & 0xFFFF0000);

	}

	//wait for UART FIFO empty
	while(!((localUartRegs->LSR) & CSL_UART_LSR_THRE_MASK));

}

/*read data from the UART RX FIFO to the receive "buffer".
Return when FIFO is empty or the receive "buffer" is full.
Number of byte read is returned*/
unsigned int KeyStone_UART_read(unsigned char *buffer, 
	unsigned int buffByteLen, Uint32 uartNum)
{
    Uint32 byteCnt;
    CSL_UartRegs *localUART_Regs;

	localUART_Regs = gpUartRegs[uartNum];
    
	for(byteCnt=0; byteCnt<buffByteLen; byteCnt++)
	{
		if(localUART_Regs->LSR & CSL_UART_LSR_DR_MASK)
		{
			buffer[byteCnt] = localUART_Regs->RBR;
		}
		else 	//FIFO empty
			break;
	}
	return byteCnt;
}

char * uart_err_str[]=
{
	"UART_Receive_FIFO_Error",
	"UART_Break_Error       ",
	"UART_Frame_Error       ",
	"UART_Parity_Error      ",
	"UART_Overrun_Error     ",
};

//check and print UART error status
void KeyStone_UART_Error_Handler(Uint32 uartNum)
{
	int i;
	Uint32 uLSR;
	Uint32 uartErrShift[5] = {CSL_UART_LSR_RXFIFOE_SHIFT,
							  CSL_UART_LSR_BI_SHIFT,
							  CSL_UART_LSR_FE_SHIFT,
							  CSL_UART_LSR_PE_SHIFT,
							  CSL_UART_LSR_OE_SHIFT};
    CSL_UartRegs *localUART_Regs;

    localUART_Regs = gpUartRegs[uartNum];

	uLSR = localUART_Regs->LSR;
    
	for(i=0; i<5; i++)
	{
		if((uLSR >> uartErrShift[i])& 1)
		{
			printf("UART%d error is %s\n",uartNum, uart_err_str[i]);
		}

	}

}


