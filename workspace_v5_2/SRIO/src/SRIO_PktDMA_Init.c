
/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2011
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
* ============================================================================
 Example to show the initialization of Multicore Navigator for SRIO test 
 on KeyStone DSP
* =============================================================================
 *  Revision History
 *  ===============
 *  June 14, 2011 Brighton Feng   File Created
 * ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include "SRIO_PktDMA_init.h"
#include "KeyStone_common.h"
#include <csl_qm_queue.h>

#ifdef _BIG_ENDIAN
#include <ti/drv/qmss/firmware/acc48_be_bin.h>
#else
#include <ti/drv/qmss/firmware/acc48_le_bin.h>
#endif

#pragma DATA_SECTION(hostDescriptor_SL2,"QMSS_Data:Descriptor_SL2")
#pragma DATA_ALIGN (hostDescriptor_SL2, 128)
Uint8 hostDescriptor_SL2[SL2_HOST_DESCRIPTOR_NUM][SRIO_DESCRIPTOR_SIZE];

#pragma DATA_SECTION(packetBuffer_SL2_Size0,"PacketData:buffer_SL2")
#pragma DATA_SECTION(packetBuffer_SL2_Size1,"PacketData:buffer_SL2")
Uint8 packetBuffer_SL2_Size0[SL2_SIZE0_PKT_BUF_NUM][SL2_PACKET_BUFFER_SIZE0];
Uint8 packetBuffer_SL2_Size1[SL2_SIZE1_PKT_BUF_NUM][SL2_PACKET_BUFFER_SIZE1];

#pragma DATA_SECTION(hostDescriptor_LL2,"QMSS_Data:Descriptor_LL2")
#pragma DATA_ALIGN (hostDescriptor_LL2, 64)
Uint8 hostDescriptor_LL2[LL2_HOST_DESCRIPTOR_NUM][SRIO_DESCRIPTOR_SIZE];

#pragma DATA_SECTION(packetBuffer_LL2_Size0,"PacketData:buffer_LL2")
#pragma DATA_SECTION(packetBuffer_LL2_Size1,"PacketData:buffer_LL2")
Uint8 packetBuffer_LL2_Size0[LL2_SIZE0_PKT_BUF_NUM][LL2_PACKET_BUFFER_SIZE0];
Uint8 packetBuffer_LL2_Size1[LL2_SIZE1_PKT_BUF_NUM][LL2_PACKET_BUFFER_SIZE1];

#pragma DATA_SECTION(hostDescriptor_DDR,"QMSS_Data:Descriptor_DDR")
#pragma DATA_ALIGN (hostDescriptor_DDR, 128)
Uint8 hostDescriptor_DDR[DDR_HOST_DESCRIPTOR_NUM][SRIO_DESCRIPTOR_SIZE];

#pragma DATA_SECTION(packetBuffer_DDR_Size0,"PacketData:buffer_DDR")
#pragma DATA_SECTION(packetBuffer_DDR_Size1,"PacketData:buffer_DDR")
Uint8 packetBuffer_DDR_Size0[DDR_SIZE0_PKT_BUF_NUM][DDR_PACKET_BUFFER_SIZE0];
Uint8 packetBuffer_DDR_Size1[DDR_SIZE1_PKT_BUF_NUM][DDR_PACKET_BUFFER_SIZE1];

/* descriptor memory region configuration.
descriptor Base address must be specified in ascending order,
no overlap is allowed between regions*/
Qmss_DescMemRegionCfg descMemRegionsCfg[]=
{   /*The base address of descriptor region     Size of each descriptor     Number of descriptors 
                                                16*n,1<=n<=8192             2^(5+n), 0<=n<=15*/
    {(Uint32)hostDescriptor_SL2,                SRIO_DESCRIPTOR_SIZE,       SL2_HOST_DESC_LINK_ENTRY_NUM},/*host descriptor in SL2*/
    {((Uint32)hostDescriptor_LL2)+0x10000000,   SRIO_DESCRIPTOR_SIZE,       LL2_HOST_DESC_LINK_ENTRY_NUM},/*host descriptor in Core0 LL2*/
    {((Uint32)hostDescriptor_LL2)+0x11000000,   SRIO_DESCRIPTOR_SIZE,       LL2_HOST_DESC_LINK_ENTRY_NUM},/*host descriptor in Core1 LL2*/
    {((Uint32)hostDescriptor_LL2)+0x12000000,   SRIO_DESCRIPTOR_SIZE,       LL2_HOST_DESC_LINK_ENTRY_NUM},/*host descriptor in Core2 LL2*/
    {((Uint32)hostDescriptor_LL2)+0x13000000,   SRIO_DESCRIPTOR_SIZE,       LL2_HOST_DESC_LINK_ENTRY_NUM},/*host descriptor in Core3 LL2*/
    {(Uint32)hostDescriptor_DDR,                SRIO_DESCRIPTOR_SIZE,       DDR_HOST_DESC_LINK_ENTRY_NUM}/*host descriptor in DDR*/
};

FreeHostQueueCfg freeHostQueueCfgTable[]=
{
    /*free queue number*/       /*address of first descriptor*/                                         /*size of descriptor*/  /*number of descriptors*/   /*address of first buffer*/                     /*size of each buffer*/ 
    {SL2_HOST_SIZE0_FDQ,        (Uint32)&hostDescriptor_SL2[0][0],                                      SRIO_DESCRIPTOR_SIZE,   SL2_SIZE0_PKT_BUF_NUM,      (Uint32)packetBuffer_SL2_Size0,                 SL2_PACKET_BUFFER_SIZE0},/*SL2 Size0 host Free Descriptor Queue*/
    {SL2_HOST_SIZE1_FDQ,        (Uint32)&hostDescriptor_SL2[SL2_SIZE1_DESC_START_IDX][0],               SRIO_DESCRIPTOR_SIZE,   SL2_SIZE1_PKT_BUF_NUM,      (Uint32)packetBuffer_SL2_Size1,                 SL2_PACKET_BUFFER_SIZE1},/*SL2 Size1 host Free Descriptor Queue*/

    {CORE0_LL2_HOST_SIZE0_FDQ,  ((Uint32)&hostDescriptor_LL2[0][0])+0x10000000,                         SRIO_DESCRIPTOR_SIZE,   LL2_SIZE0_PKT_BUF_NUM,      ((Uint32)packetBuffer_LL2_Size0)+0x10000000,    LL2_PACKET_BUFFER_SIZE0},/*CORE0 LL2 Size0 host Free Descriptor Queue*/
    {CORE0_LL2_HOST_SIZE1_FDQ,  ((Uint32)&hostDescriptor_LL2[LL2_SIZE1_DESC_START_IDX][0])+0x10000000,  SRIO_DESCRIPTOR_SIZE,   LL2_SIZE1_PKT_BUF_NUM,      ((Uint32)packetBuffer_LL2_Size1)+0x10000000,    LL2_PACKET_BUFFER_SIZE1},/*CORE0 LL2 Size1 host Free Descriptor Queue*/

    {CORE1_LL2_HOST_SIZE0_FDQ,  ((Uint32)&hostDescriptor_LL2[0][0])+0x11000000,                         SRIO_DESCRIPTOR_SIZE,   LL2_SIZE0_PKT_BUF_NUM,      ((Uint32)packetBuffer_LL2_Size0)+0x11000000,    LL2_PACKET_BUFFER_SIZE0},/*CORE1 LL2 Size0 host Free Descriptor Queue*/
    {CORE1_LL2_HOST_SIZE1_FDQ,  ((Uint32)&hostDescriptor_LL2[LL2_SIZE1_DESC_START_IDX][0])+0x11000000,  SRIO_DESCRIPTOR_SIZE,   LL2_SIZE1_PKT_BUF_NUM,      ((Uint32)packetBuffer_LL2_Size1)+0x11000000,    LL2_PACKET_BUFFER_SIZE1},/*CORE1 LL2 Size1 host Free Descriptor Queue*/

    {CORE2_LL2_HOST_SIZE0_FDQ,  ((Uint32)&hostDescriptor_LL2[0][0])+0x12000000,                         SRIO_DESCRIPTOR_SIZE,   LL2_SIZE0_PKT_BUF_NUM,      ((Uint32)packetBuffer_LL2_Size0)+0x12000000,    LL2_PACKET_BUFFER_SIZE0},/*CORE2 LL2 Size0 host Free Descriptor Queue*/
    {CORE2_LL2_HOST_SIZE1_FDQ,  ((Uint32)&hostDescriptor_LL2[LL2_SIZE1_DESC_START_IDX][0])+0x12000000,  SRIO_DESCRIPTOR_SIZE,   LL2_SIZE1_PKT_BUF_NUM,      ((Uint32)packetBuffer_LL2_Size1)+0x12000000,    LL2_PACKET_BUFFER_SIZE1},/*CORE2 LL2 Size1 host Free Descriptor Queue*/

    {CORE3_LL2_HOST_SIZE0_FDQ,  ((Uint32)&hostDescriptor_LL2[0][0])+0x13000000,                         SRIO_DESCRIPTOR_SIZE,   LL2_SIZE0_PKT_BUF_NUM,      ((Uint32)packetBuffer_LL2_Size0)+0x13000000,    LL2_PACKET_BUFFER_SIZE0},/*CORE3 LL2 Size0 host Free Descriptor Queue*/
    {CORE3_LL2_HOST_SIZE1_FDQ,  ((Uint32)&hostDescriptor_LL2[LL2_SIZE1_DESC_START_IDX][0])+0x13000000,  SRIO_DESCRIPTOR_SIZE,   LL2_SIZE1_PKT_BUF_NUM,      ((Uint32)packetBuffer_LL2_Size1)+0x13000000,    LL2_PACKET_BUFFER_SIZE1},/*CORE3 LL2 Size1 host Free Descriptor Queue*/

    {DDR_HOST_SIZE0_FDQ,        (Uint32)&hostDescriptor_DDR[0][0],                                      SRIO_DESCRIPTOR_SIZE,   DDR_SIZE0_PKT_BUF_NUM,      (Uint32)packetBuffer_DDR_Size0,                 DDR_PACKET_BUFFER_SIZE0},/*DDR Size0 host Free Descriptor Queue*/
    {DDR_HOST_SIZE1_FDQ,        (Uint32)&hostDescriptor_DDR[DDR_SIZE1_DESC_START_IDX][0],               SRIO_DESCRIPTOR_SIZE,   DDR_SIZE1_PKT_BUF_NUM,      (Uint32)packetBuffer_DDR_Size1,                 DDR_PACKET_BUFFER_SIZE1}/*DDR Size1 host Free Descriptor Queue*/
};

PktDma_RxFlowCfg flowCfgTable[]=
{
    /*dest_qnum*/                        /*sop_offset*/  /*ps_loc*/  /*desc_type*/   /*error retry*/ /*psinfo*/  /*einfo*/   /*size_thresh0(1,2)_en*/ /*size_thresh0,1,2*/ /*fdq0_sz0_qnum*/         /*fdq0_sz1_qnum,   sz2_qnum, sz3_qnum*/ /*fdq1_qnum*/           /*fdq2_qnum*/           /*fdq3_qnum*/   /*dest_tag_lo*/ /*dest_tag_hi*/ /*src_tag_lo*/  /*src_tag_hi*/  /*dest_tag_lo_sel*/ /*dest_tag_hi_sel*/ /*src_tag_lo_sel*/  /*src_tag_hi_sel*/
    {SRIO_RX_FLOW0_DST_Q+SRIO_RX_FLOW_DDR      ,    0,      0,      Cppi_DescType_HOST,     0,          1,          0,          1, 0, 0,    DDR_PACKET_BUFFER_SIZE0, 0, 0, DDR_HOST_SIZE0_FDQ,          DDR_HOST_SIZE1_FDQ,    0, 0,      DDR_HOST_SIZE1_FDQ,     DDR_HOST_SIZE1_FDQ,     DDR_HOST_SIZE1_FDQ,         0,      0,              0,              0,              0,                  0,                  0,                  0},
    {SRIO_RX_FLOW0_DST_Q+SRIO_RX_FLOW_SL2      ,    0,      0,      Cppi_DescType_HOST,     0,          1,          0,          1, 0, 0,    SL2_PACKET_BUFFER_SIZE0, 0, 0, SL2_HOST_SIZE0_FDQ,          SL2_HOST_SIZE1_FDQ,    0, 0,      SL2_HOST_SIZE1_FDQ,     SL2_HOST_SIZE1_FDQ,     SL2_HOST_SIZE1_FDQ,         0,      0,              0,              0,              0,                  0,                  0,                  0},
    {SRIO_RX_FLOW0_DST_Q+SRIO_RX_FLOW_CORE0_LL2,    0,      0,      Cppi_DescType_HOST,     0,          1,          0,          1, 0, 0,    LL2_PACKET_BUFFER_SIZE0, 0, 0, CORE0_LL2_HOST_SIZE0_FDQ, CORE0_LL2_HOST_SIZE1_FDQ, 0, 0, CORE0_LL2_HOST_SIZE1_FDQ, CORE0_LL2_HOST_SIZE1_FDQ, CORE0_LL2_HOST_SIZE1_FDQ,    0,      0,              0,              0,              0,                  0,                  0,                  0},
    {SRIO_RX_FLOW0_DST_Q+SRIO_RX_FLOW_CORE1_LL2,    0,      0,      Cppi_DescType_HOST,     0,          1,          0,          1, 0, 0,    LL2_PACKET_BUFFER_SIZE0, 0, 0, CORE1_LL2_HOST_SIZE0_FDQ, CORE1_LL2_HOST_SIZE1_FDQ, 0, 0, CORE1_LL2_HOST_SIZE1_FDQ, CORE1_LL2_HOST_SIZE1_FDQ, CORE1_LL2_HOST_SIZE1_FDQ,    0,      0,              0,              0,              0,                  0,                  0,                  0},
    {SRIO_RX_FLOW0_DST_Q+SRIO_RX_FLOW_CORE2_LL2,    0,      0,      Cppi_DescType_HOST,     0,          1,          0,          1, 0, 0,    LL2_PACKET_BUFFER_SIZE0, 0, 0, CORE2_LL2_HOST_SIZE0_FDQ, CORE2_LL2_HOST_SIZE1_FDQ, 0, 0, CORE2_LL2_HOST_SIZE1_FDQ, CORE2_LL2_HOST_SIZE1_FDQ, CORE2_LL2_HOST_SIZE1_FDQ,    0,      0,              0,              0,              0,                  0,                  0,                  0},
    {SRIO_RX_FLOW0_DST_Q+SRIO_RX_FLOW_CORE3_LL2,    0,      0,      Cppi_DescType_HOST,     0,          1,          0,          1, 0, 0,    LL2_PACKET_BUFFER_SIZE0, 0, 0, CORE3_LL2_HOST_SIZE0_FDQ, CORE3_LL2_HOST_SIZE1_FDQ, 0, 0, CORE3_LL2_HOST_SIZE1_FDQ, CORE3_LL2_HOST_SIZE1_FDQ, CORE3_LL2_HOST_SIZE1_FDQ,    0,      0,              0,              0,              0,                  0,                  0,                  0}
};

/*Descriptor accumulation buffer*/
Uint32 uiaDescriptorAccumulationList[16*2];

Uint32 uiUsedDescRegionNum;
Uint32 uiInitialHostFdqNum;
Uint32 uiUsedRxFlowNum;

/*must be called after SRIO PSC enabled*/
void SRIO_PktDM_init()
{
	int i;
	
	uiUsedDescRegionNum= sizeof(descMemRegionsCfg)/sizeof(Qmss_DescMemRegionCfg);
	uiInitialHostFdqNum= sizeof(freeHostQueueCfgTable)/sizeof(FreeHostQueueCfg);
	uiUsedRxFlowNum= sizeof(flowCfgTable)/sizeof(PktDma_RxFlowCfg);

	/*-----------------QMSS queue configuration------------------*/
	KeyStone_QMSS_Linking_RAM_init(NULL, 0);

	KeyStone_QMSS_Descriptor_Regions_init(descMemRegionsCfg, uiUsedDescRegionNum);

	KeyStone_Host_Descriptor_Queues_init(freeHostQueueCfgTable, uiInitialHostFdqNum);

	/*-----------------QMSS PDSP fireware configuration------------------*/
#ifdef _BIG_ENDIAN
	KeyStone_Qmss_Download_Firmware(Qmss_PdspId_PDSP1, &acc48_be, sizeof(acc48_be));
#else
	KeyStone_Qmss_Download_Firmware(Qmss_PdspId_PDSP1, &acc48_le, sizeof(acc48_le));
#endif
	gpQM_INTD_regs->STATUS_CLR_REG0= 0xFFFFFFFF;
	gpQM_INTD_regs->STATUS_CLR_REG1= 0xFFFF;

	KeyStone_Qmss_Config_Reclaim_Queue (Qmss_PdspId_PDSP1, RECLAMATION_QUEUE);
	
	/*-----------------SRIO PktDMA configuration------------------*/
	KeyStone_pktDma_Global_Control(gpSRIO_DMA_CfgRegs, 256, 5, 5);

	gpSRIO_DMA_CfgRegs->EMULATION_CONTROL_REG= 
		(0<<CSL_CPPIDMA_GLOBAL_CONFIG_EMULATION_CONTROL_REG_LOOPBACK_EN_SHIFT)
		|(1<<CSL_CPPIDMA_GLOBAL_CONFIG_EMULATION_CONTROL_REG_SOFT_SHIFT)
		|(0<<CSL_CPPIDMA_GLOBAL_CONFIG_EMULATION_CONTROL_REG_FREE_SHIFT);

	/*All 16 Rx channels must always be enabled, because the SRIO picks a
	channel in a round-robin fashion.*/
	for(i= 0; i<SRIO_PKTDMA_MAX_CH_NUM; i++)
	{
		KeyStone_pktDma_RxCh_enable(gpSRIO_DMA_RxChCfgRegs, i);
		KeyStone_pktDma_TxCh_enable(gpSRIO_DMA_TxChCfgRegs, i);
	}
	
	KeyStone_pktDma_configureRxFlow(
		gpSRIO_DMA_RxFlowCfgRegs->RX_FLOW_CONFIG, flowCfgTable, uiUsedRxFlowNum);

}

void QMSS_Accumulation_config()
{
	Qmss_AccCmdCfg loQuAccCfg;

	loQuAccCfg.channel = 32;
	loQuAccCfg.command = Qmss_AccCmd_ENABLE_CHANNEL;
	loQuAccCfg.queueEnMask = 0xFFFF;
	loQuAccCfg.listAddress = GLOBAL_ADDR(uiaDescriptorAccumulationList);
	loQuAccCfg.queMgrIndex = SRIO_RX_FLOW0_DST_Q;
	loQuAccCfg.maxPageEntries = 1+1;
	loQuAccCfg.timerLoadCount = 1;
	loQuAccCfg.interruptPacingMode = Qmss_AccPacingMode_NONE;
	loQuAccCfg.listEntrySize = Qmss_AccEntrySize_REG_D;
	loQuAccCfg.listCountMode = Qmss_AccCountMode_NULL_TERMINATE;
	loQuAccCfg.multiQueueMode = Qmss_AccQueueMode_MULTI_QUEUE;
	KeyStone_Qmss_Config_Acc_Channel(Qmss_PdspId_PDSP1, &loQuAccCfg);

}

/*check the descriptors in the queue, for debug purpose*/
void SRIO_Check_Free_Queues()
{
	int i, j;
	Uint32 uiQuNum, uiEntryCount, uiDescriptorNumber, uiDescriptor;
	
	_mfence(); 	//force all memory operation complete
	_mfence(); 	//force all memory operation complete
	for(i=0; i<uiInitialHostFdqNum; i++)
	{
		uiQuNum= freeHostQueueCfgTable[i].uiFreeQuNum;
		uiEntryCount= gpQueueStatusConfigRegs[uiQuNum].REG_A_EntryCount;
		uiDescriptorNumber= freeHostQueueCfgTable[i].uiDescriptorNumber;
		if(uiEntryCount!= uiDescriptorNumber)
		{
			printf("number of descriptors in host queue %d changed from %d to %d\n", 
				uiQuNum, uiDescriptorNumber, uiEntryCount);
			//asm( " SWBP 0" ); 	//software breakpoint

			//printf some descriptors in the queue
			for(j= 0; j<uiEntryCount&&j<10; j++)
			{	
				uiDescriptor= gpQueueManageRegs[uiQuNum].REG_D_Descriptor;
				printf("0x%x->", uiDescriptor);
				gpQueueManageVBUSM[uiQuNum].REG_D_Descriptor= uiDescriptor;
			}
			if(j<10)
				puts("NULL");
			else
				puts("...");
		}
	}

}

/*prinf content in error queue and recycle it*/
void SRIO_Handle_Error_Queues(Uint32 uiQuNum)
{
	Uint32 uiReturnQueueNum;
	HostPacketDescriptor * hostDescriptor;

	/*pop descriptor from queue*/
	hostDescriptor= (HostPacketDescriptor *)
		KeyStone_queuePop(uiQuNum);
	while(hostDescriptor)
	{
		/*invalid cache before read descriptor RAM*/
		InvalidCache((void *)hostDescriptor, 64);

		printf("descriptor at 0x%x, buffer address 0x%x, packet type %d, error flag 0x%x\n",
			(Uint32)hostDescriptor, hostDescriptor->buffer_ptr,
			hostDescriptor->packet_type, hostDescriptor->err_flags);
	
		/*descriptor Reclamation*/
		uiReturnQueueNum= (hostDescriptor->pkt_return_qmgr<<12)|
			hostDescriptor->pkt_return_qnum;
		KeyStone_queuePush(uiReturnQueueNum, (Uint32)hostDescriptor|FETCH_SIZE_64);
		
		/*pop the next descriptor from queue*/
		hostDescriptor= (HostPacketDescriptor *)
			KeyStone_queuePop(uiQuNum);

	}
}

/*check the descriptors in the queue, for debug purpose*/
void SRIO_Check_TxRx_Queues()
{
	int i;
	Uint32 uiQuNum, uiEntryCount;
	
	_mfence(); 	//force all memory operation complete
	_mfence(); 	//force all memory operation complete
	for(i= 0; i< SRIO_PKTDMA_MAX_CH_NUM; i++)
	{
		uiEntryCount= 
			gpQueueStatusConfigRegs[QMSS_SRIO_QUEUE_BASE+i].REG_A_EntryCount;
		if(uiEntryCount)
			printf("TX queue %d entry count = %d\n", i, uiEntryCount);
	}

 	uiEntryCount= 
		gpQueueStatusConfigRegs[SRIO_RX_SIZE_ERROR_GARBAGE_Q].REG_A_EntryCount;
	if(uiEntryCount)
	{
		printf("SRIO_RX_SIZE_ERROR_GARBAGE_Q entry count = %d\n", uiEntryCount);
		SRIO_Handle_Error_Queues(SRIO_RX_SIZE_ERROR_GARBAGE_Q);
	}

	uiEntryCount= 
		gpQueueStatusConfigRegs[SRIO_RX_TIMEOUT_GARBAGE_Q].REG_A_EntryCount;
	if(uiEntryCount)
	{
		printf("SRIO_RX_TIMEOUT_GARBAGE_Q entry count = %d\n", uiEntryCount);
		SRIO_Handle_Error_Queues(SRIO_RX_TIMEOUT_GARBAGE_Q);
	}

	uiEntryCount= 
		gpQueueStatusConfigRegs[SRIO_TX_EXCESSIVE_RETRIES_GARBAGE_Q].REG_A_EntryCount;
	if(uiEntryCount)
	{
		printf("SRIO_TX_EXCESSIVE_RETRIES_GARBAGE_Q entry count = %d\n", uiEntryCount);
		SRIO_Handle_Error_Queues(SRIO_TX_EXCESSIVE_RETRIES_GARBAGE_Q);
	}

	uiEntryCount= 
		gpQueueStatusConfigRegs[SRIO_TX_ERROR_GARBAGE_Q].REG_A_EntryCount;
	if(uiEntryCount)
	{
		printf("SRIO_TX_ERROR_GARBAGE_Q entry count = %d\n", uiEntryCount);
		SRIO_Handle_Error_Queues(SRIO_TX_ERROR_GARBAGE_Q);
	}

	uiEntryCount= 
		gpQueueStatusConfigRegs[SRIO_TX_SIZE_ERROR_GARBAGE_Q].REG_A_EntryCount;
	if(uiEntryCount)
	{
		printf("SRIO_TX_SIZE_ERROR_GARBAGE_Q entry count = %d\n", uiEntryCount);
		SRIO_Handle_Error_Queues(SRIO_TX_SIZE_ERROR_GARBAGE_Q);
	}

	for(i=0; i<uiUsedRxFlowNum; i++)
	{
		uiQuNum= flowCfgTable[i].rx_dest_qnum;
		uiEntryCount= gpQueueStatusConfigRegs[uiQuNum].REG_A_EntryCount;
		if(uiEntryCount)
			printf("number of descriptors in RX queue %d = %d\n", 
				uiQuNum, uiEntryCount);
	}

	uiEntryCount= 
		gpQueueStatusConfigRegs[0].REG_A_EntryCount;
	if(uiEntryCount)
		printf("Queue 0 entry count = %d\n", uiEntryCount);

}

void SRIO_Check_Queue(Uint32 uiQIdx, Uint32 uiExpectEntryCount)
{
	Uint32 uiEntryCount;
	
	_mfence(); 	//force all memory operation complete
	_mfence(); 	//force all memory operation complete
	uiEntryCount= 
		gpQueueStatusConfigRegs[uiQIdx].REG_A_EntryCount;
	if(uiEntryCount!=uiExpectEntryCount)
		printf("queue %d entry count = %d\n", uiQIdx, uiEntryCount);
}
