/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2012
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
 Example of SPI configuration and read/write driver
 * =============================================================================
 *  Revision History
 *  ===============
 *  Feb 19, 2012 Brighton Feng created
Dec 8, 2014, Brighton Feng fixed CS selection and polarity bug.
 * =============================================================================
 */

#include <stdio.h>
#include "KeyStone_common.h"
#include "KeyStone_SPI_Init_drv.h"
#include <cslr_device.h>
#include <csl_edma3.h>

#define SPI_TIMEOUT_CYCLES 	((1000)*8*100)

CSL_SpiRegs * gpSPI_regs= (CSL_SpiRegs *)CSL_SPI_REGS;

void KeyStone_SPI_init(SPI_Config *spiCfg)
{
	int i;
	Uint8 clockPreScale;
	Uint8 delayBetweenTrans_clocks;
	Uint16 C2T_delay_clocks;
	Uint16 T2C_delay_clocks;
	SPI_Data_Format * datFmt;
	SPI_Interrupt_Config * intCfg;

	/*1. Reset the SPI by clearing the RESET bit in the SPI global control register 0
	(SPIGCR0) to 0.*/
	gpSPI_regs->SPIGCR0 = 0;

	/*2. Take the SPI out of reset by setting SPIGCR0.RESET to 1.*/
	gpSPI_regs->SPIGCR0 = 1;

	/*3. Configure the SPI for master mode by configuring the CLKMOD and MASTER
	bits in the SPI global control register 1 (SPIGCR1).*/
	gpSPI_regs->SPIGCR1 = 
		((spiCfg->loopBack<<CSL_SPI_SPIGCR1_LOOPBACK_SHIFT)&CSL_SPI_SPIGCR1_LOOPBACK_MASK)
		|((1<<CSL_SPI_SPIGCR1_CLKMOD_SHIFT)&CSL_SPI_SPIGCR1_CLKMOD_MASK)
		|((1<<CSL_SPI_SPIGCR1_MASTER_SHIFT)&CSL_SPI_SPIGCR1_MASTER_MASK);

	/*4. Configure the SPI for 3-pin or 4-pin with chip select mode by configuring the SPI
	pin control register 0 (SPIPC0).*/
	gpSPI_regs->SPIPC0 = CSL_SPI_SPIPC0_SOMIFUN_MASK
		|CSL_SPI_SPIPC0_SIMOFUN_MASK|CSL_SPI_SPIPC0_CLKFUN_MASK;
	if(spiCfg->number_SPI_pins>3)
		gpSPI_regs->SPIPC0 |= CSL_SPI_SPIPC0_SCS0FUN0_MASK;
	if(spiCfg->number_SPI_pins>4)
		gpSPI_regs->SPIPC0 |= CSL_SPI_SPIPC0_SCS0FUN1_MASK;
	if(spiCfg->number_SPI_pins>5)
		gpSPI_regs->SPIPC0 |= CSL_SPI_SPIPC0_SCS0FUN2_MASK;
	if(spiCfg->number_SPI_pins>6)
		gpSPI_regs->SPIPC0 |= CSL_SPI_SPIPC0_SCS0FUN3_MASK;

	/*6. Configure the SPI data rate, character length, shift direction, phase, polarity and
	other format options using SPIFMTn selected in step 5.*/	
	for(i= 0; i<4; i++)	/*4 possible formats*/
	{
		if(spiCfg->dataFormat[i])
		{
			datFmt= spiCfg->dataFormat[i];
			if(datFmt->clockSpeedKHz > 66000)
			{
				printf("ERROR: SPI format %d speed higher than 66 MHz!\n", i);
				continue;
			}

			/*SPI internal input clock is (DSP core clock)/6,
			it should be Prescale to expected output clock speed*/
			clockPreScale= gDSP_Core_Speed_Hz/6/(1000*datFmt->clockSpeedKHz);

			/*round up*/
			if(clockPreScale*datFmt->clockSpeedKHz<gDSP_Core_Speed_Hz/1000/6)
				clockPreScale++;

			delayBetweenTrans_clocks = datFmt->delayBetweenTrans_ns/
				(1000000000/(gDSP_Core_Speed_Hz/6));
			if(delayBetweenTrans_clocks<2)
				delayBetweenTrans_clocks=2;
			if(delayBetweenTrans_clocks > 65)
			{
				puts("ERROR: delay between transmissions > 65*6 DSP core clocks!");
				continue;
			}
			gpSPI_regs->SPIFMT[i]= 
				(((delayBetweenTrans_clocks-2)<<CSL_SPI_SPIFMT_WDELAY_SHIFT)&CSL_SPI_SPIFMT_WDELAY_MASK)
				|((datFmt->ShifDirection<<CSL_SPI_SPIFMT_SHIFTDIR_SHIFT)&CSL_SPI_SPIFMT_SHIFTDIR_MASK)
				|((datFmt->disable_CS_timing<<CSL_SPI_SPIFMT_DISCSTIMERS_SHIFT)&CSL_SPI_SPIFMT_DISCSTIMERS_MASK)
				|((datFmt->clockPolarity<<CSL_SPI_SPIFMT_POLARITY_SHIFT)&CSL_SPI_SPIFMT_POLARITY_MASK)
				|((datFmt->clockPhase<<CSL_SPI_SPIFMT_PHASE_SHIFT)&CSL_SPI_SPIFMT_PHASE_MASK)
				|(((clockPreScale-1)<<CSL_SPI_SPIFMT_PRESCALE_SHIFT)&CSL_SPI_SPIFMT_PRESCALE_MASK)
				|((datFmt->wordLength<<CSL_SPI_SPIFMT_CHARLEN_SHIFT)&CSL_SPI_SPIFMT_CHARLEN_MASK);
		}
	}

	/*The timeing value is calculated as follows:
	tC2TDELAY = (C2TDELAY + 2) ¡Á SPI module clock period
	Note: If C2TDELAY = 0, then tC2TDELAY = 0.*/
	C2T_delay_clocks = spiCfg->C2T_delay_ns/(1000000000/(gDSP_Core_Speed_Hz/6));
	if(2==C2T_delay_clocks||3==C2T_delay_clocks)
		C2T_delay_clocks= 1;
	if(C2T_delay_clocks>4)
		C2T_delay_clocks -= 2;
	T2C_delay_clocks = spiCfg->T2C_delay_ns/(1000000000/(gDSP_Core_Speed_Hz/6));
	if(2==T2C_delay_clocks||3==T2C_delay_clocks)
		T2C_delay_clocks= 1;
	if(T2C_delay_clocks>4)
		T2C_delay_clocks -= 2;

	if(C2T_delay_clocks > 255)
	{
		C2T_delay_clocks = 255;
		puts("ERROR: Chip-select-active-to-transmit-start-delay > 257*6 DSP core clocks");
	}
	if(T2C_delay_clocks > 255)
	{
		T2C_delay_clocks = 255;
		puts("ERROR: Transmit-end-to-chip-select-inactive-delay > 257*6 DSP core clocks");
	}

	/*7. In master mode, configure the master delay options using the SPI delay register
	(SPIDELAY).*/
	gpSPI_regs->SPIDELAY= (C2T_delay_clocks<<CSL_SPI_SPIDELAY_C2TDELAY_SHIFT)
		|(T2C_delay_clocks<<CSL_SPI_SPIDELAY_T2CDELAY_SHIFT);

	/*the CS_polarity is defined as invert of the register field*/
	//gpSPI_regs->SPIDEF = !spiCfg->CS_polarity;
	gpSPI_regs->SPIDEF = 0xFF;

	/*8. Select the error interrupt notifications by configuring the SPI interrupt register
	(SPIINT0) and the SPI interrupt level register (SPILVL).*/
	if(spiCfg->interruptCfg)
	{
		intCfg= spiCfg->interruptCfg;
		gpSPI_regs->SPILVL = 
			((intCfg->TX_INT_map<<CSL_SPI_SPILVL_TXINTLVL_SHIFT)&CSL_SPI_SPILVL_TXINTLVL_MASK)
			|((intCfg->RX_INT_map<<CSL_SPI_SPILVL_RXINTLVL_SHIFT)&CSL_SPI_SPILVL_RXINTLVL_MASK)
			|((intCfg->overrun_INT_map<<CSL_SPI_SPILVL_OVRNINTLVL_SHIFT)&CSL_SPI_SPILVL_OVRNINTLVL_MASK)
			|((intCfg->bitError_INT_map<<CSL_SPI_SPILVL_BITERRLVL_SHIFT)&CSL_SPI_SPILVL_BITERRLVL_MASK);
		
		gpSPI_regs->SPIINT0 =
			((intCfg->TX_interruptEnable<<CSL_SPI_SPIINT0_TXINTENA_SHIFT)&CSL_SPI_SPIINT0_TXINTENA_MASK)
			|((intCfg->RX_interruptEnable<<CSL_SPI_SPIINT0_RXINTENA_SHIFT)&CSL_SPI_SPIINT0_RXINTENA_MASK)
			|((intCfg->overrunInterruptEnable<<CSL_SPI_SPIINT0_OVRNINTENA_SHIFT)&CSL_SPI_SPIINT0_OVRNINTENA_MASK)
			|((intCfg->bitErrorInterruptEnable<<CSL_SPI_SPIINT0_BITERRENA_SHIFT)&CSL_SPI_SPIINT0_BITERRENA_MASK);
	}

    if(spiCfg->DMA_requestEnable)
    {//when EDMA is enabled, CPU should not handle TX/RX interrupt
    	gpSPI_regs->SPIINT0 &= ~(CSL_SPI_SPIINT0_TXINTENA_MASK|
    	    CSL_SPI_SPIINT0_RXINTENA_MASK);
    }
    
	/*9. Enable the SPI communication by setting the SPIGCR1.ENABLE to 1.*/
	gpSPI_regs->SPIGCR1 |= 
		((1<<CSL_SPI_SPIGCR1_ENABLE_SHIFT)&CSL_SPI_SPIGCR1_ENABLE_MASK);

	/*10. Setup and enable the DMA for SPI data handling and then enable the DMA
	servicing for the SPI data requests by setting the SPIINT0.DMAREQEN to 1.*/
    gpSPI_regs->SPIINT0 |= (spiCfg->DMA_requestEnable<<CSL_SPI_SPIINT0_DMAREQEN_SHIFT);

}

/*wait a flag in SPIFLG, 
retun 1 when the SPIFLG&flag_mask=expect, return 0 when timeout*/
Int32 KeyStone_SPI_wait_flag(Uint32 flag_mask, Uint32 expect)
{
	Uint32 startTSC;
	Uint32 delay;
	volatile Uint32 flag;

	startTSC= TSCL;

	flag= gpSPI_regs->SPIFLG&flag_mask;

	/*Wait until SPI flag= expect value*/
	while(flag!= expect)
	{
		/*if wait time is much larger than theoretical transfer time of
		a byte, then it is looked as timeout.*/		
		delay= TSC_getDelay(startTSC);
		if(delay> SPI_TIMEOUT_CYCLES)
		{
			printf("KeyStone_SPI_wait_flag 0x%x timeout, SPIFLG=0x%x\n", 
				flag_mask, gpSPI_regs->SPIFLG);

			return 0;
		}
		flag= gpSPI_regs->SPIFLG&flag_mask;

	};

	return 1;
}

/*construct data format: higher 16 bit of SPIDAT1.
"dataFormat" is for data except the last one, 
"lastDataFormat" is for the last word.
input: transfer parameters structure pointer*/
void KeyStone_SPIDAT1_format(SPI_Transfer_Param * transfer, 
	Uint32 *dataFormat, Uint32 *lastDataFormat)
{
	if(SPI_CS_NO_HOLD== transfer->CS_hold)
		*dataFormat= SPI_TRANSMIT_FORMAT_MAKE(0, transfer->delayEnable, 
			transfer->formatSelect, transfer->CS_select);
	else
		*dataFormat= SPI_TRANSMIT_FORMAT_MAKE(1, transfer->delayEnable, 
			transfer->formatSelect, transfer->CS_select);

	/*treat last word specially for CS hold*/
	if(SPI_CS_ALWAYS_HOLD== transfer->CS_hold)
		*lastDataFormat= SPI_TRANSMIT_FORMAT_MAKE(1, transfer->delayEnable, 
			transfer->formatSelect, transfer->CS_select);
	else
		*lastDataFormat= SPI_TRANSMIT_FORMAT_MAKE(0, transfer->delayEnable, 
			transfer->formatSelect, transfer->CS_select);
}

/*Since SPI TX and RX shares same clock and control signals, SPI TX and RX 
actually happens at the same time even you only want to TX or RX. But, 
application may only care for TX or RX, or part of the TX and RX words. 
This function only TX "numTxByte" valid data from the "firstTxByte", 
the valid data are from the "txBuf"; 
only RX "numRxByte" valid data from the "firstRxByte", 
the valid data are stored to the "rxBuf".
Return number of successful words*/
Uint32 KeyStone_SPI_TxRx(Uint8 * txBuf, Uint32 firstTxByte, 
	Uint32 numTxByte, Uint8 * rxBuf, Uint32 firstRxByte, Uint32 numRxByte, 
	SPI_Transfer_Param * transfer)
{
	Int32 i;
	Int32 length;
	Uint32 txData, rxData;
	Uint32 byteOfWord, dataFormat, lastDataFormat;


	//gpSPI_regs

	/*total transfer word length is the max of the lastRxWord and lastTxWord*/
	if((firstTxByte+numTxByte)>(firstRxByte+numRxByte))
		length= (firstTxByte+numTxByte);
	else
		length= (firstRxByte+numRxByte);

	/*------data format: higher 16 bit of SPIDAT1------*/
	KeyStone_SPIDAT1_format(transfer, &dataFormat, &lastDataFormat);

	byteOfWord= transfer->byteOfWord;
	if(length==byteOfWord)
		dataFormat= lastDataFormat;

	/*--------write the first word--------*/
	if((firstTxByte==0)&&(0<firstTxByte+numTxByte))
	{/*only TX valid data from the "firstTxByte"*/
		if(2==byteOfWord)
		{/*two bytes*/
			txData= *(Uint16 *)txBuf;
			txBuf+=2;
		}
		else
		{/*one byte*/
			txData= *txBuf++;
		}
	}
	else
		txData= 0;

	txData |= dataFormat;

	/* Wait for room in TX buffer */
	if(0==KeyStone_SPI_wait_flag(CSL_SPI_SPIFLG_TXINTFLG_MASK, CSL_SPI_SPIFLG_TXINTFLG_MASK))
		return 0;

	/* Send the word */
	gpSPI_regs->SPIDAT1 = txData;

	/*--------write the word n+1 while read word n--------*/
	for( i=0; i<length-byteOfWord; i+= byteOfWord )
	{
		if((firstTxByte<=i+byteOfWord)&&(i+byteOfWord<firstTxByte+numTxByte))
		{/*only TX valid data from the "firstTxByte"*/
			if(2==byteOfWord)
			{/*two bytes*/
				txData= *(Uint16 *)txBuf;
				txBuf+=2;
			}
			else
			{/*one byte*/
				txData= *txBuf++;
			}
		}
		else
			txData= 0;

		if(i+byteOfWord==length-byteOfWord)	/*the last word*/
			txData |= lastDataFormat;
		else
			txData |= dataFormat;

		/* Wait for room in TX buffer */
		if(0==KeyStone_SPI_wait_flag(CSL_SPI_SPIFLG_TXINTFLG_MASK, CSL_SPI_SPIFLG_TXINTFLG_MASK))
			return i+byteOfWord;

		/* Send the word */
		gpSPI_regs->SPIDAT1 = txData;

		/* Wait for data in RX buffer */
		if(0==KeyStone_SPI_wait_flag(CSL_SPI_SPIFLG_RXINTFLG_MASK, CSL_SPI_SPIFLG_RXINTFLG_MASK))
			return i;

		/* Read the next word */
		rxData = gpSPI_regs->SPIBUF&0xFFFF;
		
		if((firstRxByte<=i)&&(i<firstRxByte+numRxByte))
		{/*only RX valid data from the "firstRxByte"*/
			if(2==byteOfWord)
			{/*two bytes*/
				*(Uint16 *)rxBuf= rxData;
				rxBuf+=2;
			}
			else
			{/*one byte*/
				*rxBuf++= rxData;
			}
		}
	}

	/*--------read the last word--------*/
	/* Wait for data in RX buffer */
	if(0==KeyStone_SPI_wait_flag(CSL_SPI_SPIFLG_RXINTFLG_MASK, CSL_SPI_SPIFLG_RXINTFLG_MASK))
		return length-byteOfWord;

	/* Read the next word */
	rxData = gpSPI_regs->SPIBUF&0xFFFF;
	
	if((firstRxByte<=length-byteOfWord)&&(length-byteOfWord<firstRxByte+numRxByte))
	{/*only RX valid data from the "firstRxByte"*/
		if(2==byteOfWord)
		{/*two bytes*/
			*(Uint16 *)rxBuf= rxData;
			rxBuf+=2;
		}
		else
		{/*one byte*/
			*rxBuf++= rxData;
		}
	}
	
	return length;
}

/*set higher 16 bit of SPIDAT1.
input: transfer parameters structure pointer*/
void KeyStone_set_SPIDAT1_format(SPI_Transfer_Param * transfer)
{
	
	Uint16 * SPIDAT1_h16; //high 16 bits of SPIDAT1 register

#ifdef _BIG_ENDIAN
	SPIDAT1_h16= (Uint16 *)((Uint32)&gpSPI_regs->SPIDAT1);
#else
	SPIDAT1_h16= (Uint16 *)((Uint32)&gpSPI_regs->SPIDAT1+2);
#endif

	if(SPI_CS_NO_HOLD== transfer->CS_hold)
		*SPIDAT1_h16= SPI_TRANSMIT_FORMAT_MAKE(0, transfer->delayEnable, 
			transfer->formatSelect, transfer->CS_select)>>16;
	else
		*SPIDAT1_h16= SPI_TRANSMIT_FORMAT_MAKE(1, transfer->delayEnable, 
			transfer->formatSelect, transfer->CS_select)>>16;
}

/*attach higher 16-bit to orignal data in "txBuf" to constuct data according to 
SPIDAT1 register format, the output is saved in "SPIDAT1_buf".
Normally, this is used to prepare data for EDMA transfer to SPIDAT1*/
void KeyStone_SPIDAT1_buffer_construct(Uint8 * txBuf, Uint32 * SPIDAT1_buf, 
	Uint32 numTxWord, SPI_Transfer_Param * transfer)
{
	Int32 i;
	Uint32 txData;
	Uint32 byteOfWord, dataFormat, lastDataFormat;

	/*------data format: higher 16 bit of SPIDAT1------*/
	KeyStone_SPIDAT1_format(transfer, &dataFormat, &lastDataFormat);

	if(1==numTxWord)
		dataFormat= lastDataFormat;

	byteOfWord= transfer->byteOfWord;
	for( i=0; i<numTxWord-1; i++)
	{
		if(2==byteOfWord)
		{/*two bytes*/
			txData= *(Uint16 *)txBuf;
			txBuf+=2;
		}
		else
		{/*one byte*/
			txData= *txBuf++;
		}

		SPIDAT1_buf[i]= txData|dataFormat;
	}
	
	//handle the last word specially
	if(2==byteOfWord)
	{/*two bytes*/
		txData= *(Uint16 *)txBuf;
		txBuf+=2;
	}
	else
	{/*one byte*/
		txData= *txBuf++;
	}

	SPIDAT1_buf[i]= txData|lastDataFormat;

    WritebackInvalidCache(SPIDAT1_buf, numTxWord*byteOfWord);
}

/*Use EDMA to transmit and receive SPI data.
Since SPI TX and RX shares same clock and control signals, SPI TX and RX 
actually happens at the same time even you only want to TX or RX, or part of them. 
Since EDMA can not skip data, there may be unused data received at the beginning
of "rxDataBuf", it is the caller's responsibility to skip those unused data.
EDMA RX interrupt is enabled, which may be used by caller to determine the completion*/
void KeyStone_SPI_EDMA_TxRx(Uint8 * txDataBuf,
	Uint8 * rxDataBuf, Uint32 numDataWord, Uint32 byteOfWord, 
	SPI_EDMA_Dest_Width EDMA_dest)
{
	Uint32 EDMA_SRC_A_CNT, EDMA_DST;
    Uint32  uiTpccNum;
    Uint32  uiSpiEdmaTxCh;
    Uint32  uiSpiEdmaRxCh;
    
    uiTpccNum = CSL_EDMA3CC_1;
	uiSpiEdmaTxCh = CSL_TPCC1_SPIXEVT;
	uiSpiEdmaRxCh = CSL_TPCC1_SPIREVT;

	if(EDMA_TO_SPIDAT1_4_bytes==EDMA_dest)
	{
		EDMA_SRC_A_CNT= 4;
		EDMA_DST= (Uint32)&gpSPI_regs->SPIDAT1;
	}
	else if(EDMA_TO_SPIDAT0_2_bytes==EDMA_dest)
	{
		EDMA_SRC_A_CNT= 2;
#ifdef _BIG_ENDIAN 
        EDMA_DST= (Uint32)&gpSPI_regs->SPIDAT0 + 4 - EDMA_SRC_A_CNT;
#else
        EDMA_DST= (Uint32)&gpSPI_regs->SPIDAT0;
#endif		
	}
	else
	{
		EDMA_SRC_A_CNT= 1;
#ifdef _BIG_ENDIAN 
        EDMA_DST= (Uint32)&gpSPI_regs->SPIDAT0 + 4 - EDMA_SRC_A_CNT;
#else
        EDMA_DST= (Uint32)&gpSPI_regs->SPIDAT0;
#endif			
	}

    //wait for previous EDMA transfer complete 
    while(gpEDMA_CC_regs[uiTpccNum]->PARAMSET[uiSpiEdmaTxCh].A_B_CNT & 0xFFFF0000);
    while(gpEDMA_CC_regs[uiTpccNum]->PARAMSET[uiSpiEdmaRxCh].A_B_CNT & 0xFFFF0000);


    if(0==KeyStone_SPI_wait_flag(CSL_SPI_SPIFLG_TXINTFLG_MASK, CSL_SPI_SPIFLG_TXINTFLG_MASK))
        return;


    //clear potential event, status or error
    if(uiSpiEdmaTxCh<32)
    {
        gpEDMA_CC_regs[uiTpccNum]->TPCC_ECR= (1<<uiSpiEdmaTxCh);
        gpEDMA_CC_regs[uiTpccNum]->TPCC_ICR= (1<<uiSpiEdmaTxCh);
        gpEDMA_CC_regs[uiTpccNum]->TPCC_SECR= (1<<uiSpiEdmaTxCh);
        gpEDMA_CC_regs[uiTpccNum]->TPCC_EMCR= (1<<uiSpiEdmaTxCh);       
    }
    else
    {
        gpEDMA_CC_regs[uiTpccNum]->TPCC_ECRH= (1<<(uiSpiEdmaTxCh-32));
        gpEDMA_CC_regs[uiTpccNum]->TPCC_ICRH= (1<<(uiSpiEdmaTxCh-32));
        gpEDMA_CC_regs[uiTpccNum]->TPCC_SECRH= (1<<(uiSpiEdmaTxCh-32));
        gpEDMA_CC_regs[uiTpccNum]->TPCC_EMCRH= (1<<(uiSpiEdmaTxCh-32));
    }

    if(uiSpiEdmaRxCh<32)
    {    
        gpEDMA_CC_regs[uiTpccNum]->TPCC_ECR= (1<<uiSpiEdmaRxCh);
        gpEDMA_CC_regs[uiTpccNum]->TPCC_ICR= (1<<uiSpiEdmaRxCh);
        gpEDMA_CC_regs[uiTpccNum]->TPCC_SECR= (1<<uiSpiEdmaRxCh);
        gpEDMA_CC_regs[uiTpccNum]->TPCC_EMCR= (1<<uiSpiEdmaRxCh);
    }
    else
    {       
        gpEDMA_CC_regs[uiTpccNum]->TPCC_ECRH= (1<<(uiSpiEdmaRxCh-32));
        gpEDMA_CC_regs[uiTpccNum]->TPCC_ICRH= (1<<(uiSpiEdmaRxCh-32));
        gpEDMA_CC_regs[uiTpccNum]->TPCC_SECRH= (1<<(uiSpiEdmaRxCh-32));
        gpEDMA_CC_regs[uiTpccNum]->TPCC_EMCRH= (1<<(uiSpiEdmaRxCh-32));
    }

    gpEDMA_CC_regs[uiTpccNum]->PARAMSET[uiSpiEdmaTxCh].OPT=
            CSL_EDMA3_OPT_MAKE( CSL_EDMA3_ITCCH_DIS,
                                CSL_EDMA3_TCCH_DIS,
                                CSL_EDMA3_ITCINT_DIS,
                                CSL_EDMA3_TCINT_EN,
                                uiSpiEdmaTxCh,
                                CSL_EDMA3_TCC_NORMAL,
                                CSL_EDMA3_FIFOWIDTH_NONE,
                                CSL_EDMA3_STATIC_DIS,
                                CSL_EDMA3_SYNC_A,
                                CSL_EDMA3_ADDRMODE_INCR,
                                CSL_EDMA3_ADDRMODE_INCR);

    gpEDMA_CC_regs[uiTpccNum]->PARAMSET[uiSpiEdmaTxCh].SRC= GLOBAL_ADDR((Uint32)txDataBuf);    
    gpEDMA_CC_regs[uiTpccNum]->PARAMSET[uiSpiEdmaTxCh].A_B_CNT = CSL_EDMA3_CNT_MAKE(EDMA_SRC_A_CNT, numDataWord);
    gpEDMA_CC_regs[uiTpccNum]->PARAMSET[uiSpiEdmaTxCh].DST = EDMA_DST;
    gpEDMA_CC_regs[uiTpccNum]->PARAMSET[uiSpiEdmaTxCh].SRC_DST_BIDX= CSL_EDMA3_BIDX_MAKE(EDMA_SRC_A_CNT,0 );
    gpEDMA_CC_regs[uiTpccNum]->PARAMSET[uiSpiEdmaTxCh].LINK_BCNTRLD= CSL_EDMA3_LINKBCNTRLD_MAKE(0xFFFF, numDataWord);
    gpEDMA_CC_regs[uiTpccNum]->PARAMSET[uiSpiEdmaTxCh].SRC_DST_CIDX= 0;
    gpEDMA_CC_regs[uiTpccNum]->PARAMSET[uiSpiEdmaTxCh].CCNT= 1;

     gpEDMA_CC_regs[uiTpccNum]->PARAMSET[uiSpiEdmaRxCh].OPT=
            CSL_EDMA3_OPT_MAKE( CSL_EDMA3_ITCCH_DIS,
                                CSL_EDMA3_TCCH_DIS,
                                CSL_EDMA3_ITCINT_DIS,
                                CSL_EDMA3_TCINT_EN,
                                uiSpiEdmaRxCh,
                                CSL_EDMA3_TCC_NORMAL,
                                CSL_EDMA3_FIFOWIDTH_NONE,
                                CSL_EDMA3_STATIC_DIS,
                                CSL_EDMA3_SYNC_A,
                                CSL_EDMA3_ADDRMODE_INCR,
                                CSL_EDMA3_ADDRMODE_INCR);

#ifdef _BIG_ENDIAN                                
    gpEDMA_CC_regs[uiTpccNum]->PARAMSET[uiSpiEdmaRxCh].SRC= (Uint32)(&gpSPI_regs->SPIBUF) + 4 - byteOfWord;
#else
    gpEDMA_CC_regs[uiTpccNum]->PARAMSET[uiSpiEdmaRxCh].SRC= (Uint32)(&gpSPI_regs->SPIBUF);
#endif
    gpEDMA_CC_regs[uiTpccNum]->PARAMSET[uiSpiEdmaRxCh].A_B_CNT = CSL_EDMA3_CNT_MAKE(byteOfWord, numDataWord);
    gpEDMA_CC_regs[uiTpccNum]->PARAMSET[uiSpiEdmaRxCh].DST = (Uint32)GLOBAL_ADDR((Uint32)rxDataBuf);
    gpEDMA_CC_regs[uiTpccNum]->PARAMSET[uiSpiEdmaRxCh].SRC_DST_BIDX= CSL_EDMA3_BIDX_MAKE(0, byteOfWord);
    gpEDMA_CC_regs[uiTpccNum]->PARAMSET[uiSpiEdmaRxCh].LINK_BCNTRLD= CSL_EDMA3_LINKBCNTRLD_MAKE(0xFFFF, numDataWord);
    gpEDMA_CC_regs[uiTpccNum]->PARAMSET[uiSpiEdmaRxCh].SRC_DST_CIDX= 0;
    gpEDMA_CC_regs[uiTpccNum]->PARAMSET[uiSpiEdmaRxCh].CCNT= 1;
	
    /* Trigger the first transfer manually.
    Following SPI event can trigger more data transfer*/
    if(uiSpiEdmaTxCh >= 32)
    {
        gpEDMA_CC_regs[uiTpccNum]->TPCC_EESRH = 1<<(uiSpiEdmaRxCh-32);
        gpEDMA_CC_regs[uiTpccNum]->TPCC_EESRH = 1<<(uiSpiEdmaTxCh-32);
        gpEDMA_CC_regs[uiTpccNum]->TPCC_ESRH = 1<<(uiSpiEdmaTxCh-32);
    }
    else
    {
        gpEDMA_CC_regs[uiTpccNum]->TPCC_EESR = 1<<uiSpiEdmaRxCh;
        gpEDMA_CC_regs[uiTpccNum]->TPCC_EESR = 1<<uiSpiEdmaTxCh;
        gpEDMA_CC_regs[uiTpccNum]->TPCC_ESR = 1<<uiSpiEdmaTxCh;
    }
}
