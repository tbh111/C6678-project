/*  ============================================================================
 *     Copyright (C) 2013 Texas Instruments Incorporated.       *
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
EDMA test for SPI
 * =============================================================================
 *  Revision History
 *  ===============
 *  23-July-2013 cheng  file created
 *  19-October-2013 Brighton  add more test cases
 * =============================================================================
 */
#include "SPI_EDMA_Test.h"



#pragma DATA_ALIGN(spiEdmaRxBuf, 128) 	/*align to L2 cache line*/
Uint16 spiEdmaRxBuf[SPI_EDMA_TEST_BUF_SIZE/2];
#pragma DATA_ALIGN(spiEdmaTxBuf, 128) 	/*align to L2 cache line*/
Uint16 spiEdmaTxBuf[SPI_EDMA_TEST_BUF_SIZE/2];
Uint32 SPIDAT1_EdmaBuf[SPI_EDMA_TEST_BUF_SIZE/2];

SPI_Data_Format EdmaDataFormat =
{
	/*.delayBetweenTrans_ns = */0,
	/*.ShifDirection        = */SPI_MSB_SHIFT_FIRST,
	/*.disable_CS_timing    = */1,
	/*.clockPolarity        = */SPI_CLOCK_LOW_INACTIVE,
	/*.clockPhase           = */0,
	/*.clockSpeedKHz        = */66000,
	/*.wordLength           = */16
};

SPI_Transfer_Param EdmaTransferParam =
{
	0,     /*Chip select number*/
	2,  /*select one of the 4 SPI formats*/
	SPI_CS_NO_LAST_HOLD, /*hold CS between multiple words*/
	FALSE,  /*Enable the delay counter at the end of the current transaction*/
	2   /*number of bytes per SPI word*/
};

Uint32 EDMA_start_TSC;

//EDMA initialization for SPI transfer
void SPI_EDMA_Init()
{
    EDMA_init(); 
    
    //SPI0 use EDMA CC1 
	EDMA_channel_TC_cfg(CSL_EDMA3CC_1, CSL_TPCC1_SPIXEVT, 0);
	EDMA_channel_TC_cfg(CSL_EDMA3CC_1, CSL_TPCC1_SPIREVT, 0);

	//disable EDMA CC1 error (to ignore event miss exception)
	gpCIC0_regs->ENABLE_CLR_INDEX_REG= 0;
}

//initialize source and destination buffers
void SPI_EDMA_BuffInit(Uint8* pucSrcBuf, Uint8* pucDstBuf,Uint32 uiByteLength)
{
    Uint32 uiI;

    for(uiI = 0; uiI < uiByteLength; uiI++)
    {
        *pucSrcBuf++ = (uiI&0xFF);
        *pucDstBuf++ = 0;
    }
    WritebackInvalidCache(pucSrcBuf, uiByteLength);
    WritebackInvalidCache(pucDstBuf, uiByteLength);
}

//check the result of EDMA
void SPI_EDMA_Check(Uint8* pucSrcBuf, Uint8* pucDstBuf, 
	Uint32 uiByteLength)
{   
    Uint32  uiTpccNum;
    Uint32  uiSpiEdmaTxCh;
    Uint32  uiSpiEdmaRxCh;
    Uint32  uiI, uiCycles, uiThroughput;
    Uint8   srcData, dstData;


	uiTpccNum = CSL_EDMA3CC_1;
	uiSpiEdmaTxCh = CSL_TPCC1_SPIXEVT;
	uiSpiEdmaRxCh = CSL_TPCC1_SPIREVT;


    /* Wait for completion */
    if(uiSpiEdmaTxCh<32)
	    while ((gpEDMA_CC_regs[uiTpccNum]->TPCC_IPR&(1<<(uiSpiEdmaTxCh))) ==0);
	else
	    while ((gpEDMA_CC_regs[uiTpccNum]->TPCC_IPRH&(1<<(uiSpiEdmaTxCh-32))) ==0);
	//disable event to avoid EDMA event miss error
	EDMA_event_disable(uiTpccNum, uiSpiEdmaTxCh);

    if(uiSpiEdmaRxCh<32)
	    while ((gpEDMA_CC_regs[uiTpccNum]->TPCC_IPR&(1<<(uiSpiEdmaRxCh))) ==0);
	else
	    while ((gpEDMA_CC_regs[uiTpccNum]->TPCC_IPRH&(1<<(uiSpiEdmaRxCh-32))) ==0);

	//disable event to avoid EDMA event miss error
	EDMA_event_disable(uiTpccNum, uiSpiEdmaRxCh);
	uiCycles= TSC_count_cycle_from(EDMA_start_TSC);

    for(uiI = 0; uiI < uiByteLength; uiI++)
    {
    	srcData= *pucSrcBuf++;
    	dstData= *pucDstBuf++;
        if(srcData!= dstData)
        {
        	printf("data error at byte %d, source data = 0x%x, destination data = 0x%x\n",
        		uiI, srcData, dstData);
            return;
        }
    }

    uiThroughput = (unsigned long long)uiByteLength*8*gDSP_Core_Speed_Hz/
    	((unsigned long long)uiCycles*1000000);

	printf("SPI EDMA loopback test passed. Throughput= %dMbps\n",uiThroughput);
}

void SPI_EDMA_test()
{
	Uint32 numDataWord= SPI_EDMA_TEST_BUF_SIZE/EdmaTransferParam.byteOfWord;

    SPI_EDMA_BuffInit((Uint8*)spiEdmaTxBuf,(Uint8*)spiEdmaRxBuf,
    	sizeof(spiEdmaRxBuf));   

    /*EDMA write to SPIDAT0 */
#ifdef SPIDAT0_EDMA_TEST    
    KeyStone_set_SPIDAT1_format(&EdmaTransferParam);
    EDMA_start_TSC = TSCL;
    KeyStone_SPI_EDMA_TxRx((Uint8*)spiEdmaTxBuf, (Uint8*)spiEdmaRxBuf, 
    numDataWord, EdmaTransferParam.byteOfWord, EDMA_TO_SPIDAT0_2_bytes);
#else
    /*EDMA write to SPIDAT1  */	
	KeyStone_SPIDAT1_buffer_construct((Uint8*)spiEdmaTxBuf, 
		SPIDAT1_EdmaBuf, numDataWord, &EdmaTransferParam);

	EDMA_start_TSC = TSCL;
	KeyStone_SPI_EDMA_TxRx((Uint8*)SPIDAT1_EdmaBuf, (Uint8*)spiEdmaRxBuf, 
		numDataWord, EdmaTransferParam.byteOfWord, EDMA_TO_SPIDAT1_4_bytes);
#endif

    SPI_EDMA_Check((Uint8*)spiEdmaTxBuf,(Uint8*)spiEdmaRxBuf,
    	sizeof(spiEdmaRxBuf));

}

