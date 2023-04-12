/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2013
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
* ============================================================================
 Example to show the initialization of Multicore Navigator for GE test 
 on KeyStone DSP
* =============================================================================
 *  Revision History
 *  ===============
 *  July 5, 2013 Kevin Cai   File Created
 * ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include "Keystone_common.h"
#include "GE_PktDMA_init.h"
#include "KeyStone_GE_init_drv.h"
#include <csl_qm_queue.h>

#ifdef _BIG_ENDIAN
#ifdef ACC_48_CHANNEL
#include <ti/drv/qmss/firmware/acc48_be_bin.h>
#else
#include <ti/drv/qmss/firmware/acc32_be_bin.h>
#include <ti/drv/qmss/firmware/acc16_be_bin.h>
#endif
#else
#ifdef ACC_48_CHANNEL
#include <ti/drv/qmss/firmware/acc48_le_bin.h>
#else
#include <ti/drv/qmss/firmware/acc32_le_bin.h>
#include <ti/drv/qmss/firmware/acc16_le_bin.h>
#endif
#endif

#pragma DATA_SECTION(hostDescriptor_SL2,"QMSS_Data:Descriptor_SL2")
#pragma DATA_ALIGN (hostDescriptor_SL2, 128)
Uint8 hostDescriptor_SL2[SL2_HOST_DESCRIPTOR_NUM][GE_DESCRIPTOR_SIZE];

#pragma DATA_SECTION(packetBuffer_SL2_Size0,"PacketData:buffer_SL2")
#pragma DATA_SECTION(packetBuffer_SL2_Size1,"PacketData:buffer_SL2")
Uint8 packetBuffer_SL2_Size0[SL2_SIZE0_PKT_BUF_NUM][SL2_PACKET_BUFFER_SIZE0];
Uint8 packetBuffer_SL2_Size1[SL2_SIZE1_PKT_BUF_NUM][SL2_PACKET_BUFFER_SIZE1];

#pragma DATA_SECTION(hostDescriptor_LL2,"QMSS_Data:Descriptor_LL2")
#pragma DATA_ALIGN (hostDescriptor_LL2, 64)
Uint8 hostDescriptor_LL2[LL2_HOST_DESCRIPTOR_NUM][GE_DESCRIPTOR_SIZE];

#pragma DATA_SECTION(packetBuffer_LL2_Size0,"PacketData:buffer_LL2")
#pragma DATA_SECTION(packetBuffer_LL2_Size1,"PacketData:buffer_LL2")
Uint8 packetBuffer_LL2_Size0[LL2_SIZE0_PKT_BUF_NUM][LL2_PACKET_BUFFER_SIZE0];
Uint8 packetBuffer_LL2_Size1[LL2_SIZE1_PKT_BUF_NUM][LL2_PACKET_BUFFER_SIZE1];

#pragma DATA_SECTION(hostDescriptor_DDR,"QMSS_Data:Descriptor_DDR")
#pragma DATA_ALIGN (hostDescriptor_DDR, 128)
Uint8 hostDescriptor_DDR[DDR_HOST_DESCRIPTOR_NUM][GE_DESCRIPTOR_SIZE];

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
    {(Uint32)hostDescriptor_SL2,                GE_DESCRIPTOR_SIZE,       SL2_HOST_DESC_LINK_ENTRY_NUM},/*host descriptor in SL2*/
    {((Uint32)hostDescriptor_LL2)+0x10000000,   GE_DESCRIPTOR_SIZE,       LL2_HOST_DESC_LINK_ENTRY_NUM},/*host descriptor in Core0 LL2*/
    {((Uint32)hostDescriptor_LL2)+0x11000000,   GE_DESCRIPTOR_SIZE,       LL2_HOST_DESC_LINK_ENTRY_NUM},/*host descriptor in Core1 LL2*/
    {((Uint32)hostDescriptor_LL2)+0x12000000,   GE_DESCRIPTOR_SIZE,       LL2_HOST_DESC_LINK_ENTRY_NUM},/*host descriptor in Core2 LL2*/
    {((Uint32)hostDescriptor_LL2)+0x13000000,   GE_DESCRIPTOR_SIZE,       LL2_HOST_DESC_LINK_ENTRY_NUM},/*host descriptor in Core3 LL2*/
    {(Uint32)hostDescriptor_DDR,                GE_DESCRIPTOR_SIZE,       DDR_HOST_DESC_LINK_ENTRY_NUM}/*host descriptor in DDR*/
};

FreeHostQueueCfg freeHostQueueCfgTable[]=
{
    /*free queue number*/       /*address of first descriptor*/                                         /*size of descriptor*/  /*number of descriptors*/   /*address of first buffer*/                     /*size of each buffer*/ 
    {SL2_HOST_SIZE0_FDQ,        (Uint32)&hostDescriptor_SL2[0][0],                                      GE_DESCRIPTOR_SIZE,   SL2_SIZE0_PKT_BUF_NUM,      (Uint32)packetBuffer_SL2_Size0,                 SL2_PACKET_BUFFER_SIZE0},/*SL2 Size0 host Free Descriptor Queue*/
    {SL2_HOST_SIZE1_FDQ,        (Uint32)&hostDescriptor_SL2[SL2_SIZE1_DESC_START_IDX][0],               GE_DESCRIPTOR_SIZE,   SL2_SIZE1_PKT_BUF_NUM,      (Uint32)packetBuffer_SL2_Size1,                 SL2_PACKET_BUFFER_SIZE1},/*SL2 Size1 host Free Descriptor Queue*/

    {CORE0_LL2_HOST_SIZE0_FDQ,  ((Uint32)&hostDescriptor_LL2[0][0])+0x10000000,                         GE_DESCRIPTOR_SIZE,   LL2_SIZE0_PKT_BUF_NUM,      ((Uint32)packetBuffer_LL2_Size0)+0x10000000,    LL2_PACKET_BUFFER_SIZE0},/*CORE0 LL2 Size0 host Free Descriptor Queue*/
    {CORE0_LL2_HOST_SIZE1_FDQ,  ((Uint32)&hostDescriptor_LL2[LL2_SIZE1_DESC_START_IDX][0])+0x10000000,  GE_DESCRIPTOR_SIZE,   LL2_SIZE1_PKT_BUF_NUM,      ((Uint32)packetBuffer_LL2_Size1)+0x10000000,    LL2_PACKET_BUFFER_SIZE1},/*CORE0 LL2 Size1 host Free Descriptor Queue*/

    {CORE1_LL2_HOST_SIZE0_FDQ,  ((Uint32)&hostDescriptor_LL2[0][0])+0x11000000,                         GE_DESCRIPTOR_SIZE,   LL2_SIZE0_PKT_BUF_NUM,      ((Uint32)packetBuffer_LL2_Size0)+0x11000000,    LL2_PACKET_BUFFER_SIZE0},/*CORE1 LL2 Size0 host Free Descriptor Queue*/
    {CORE1_LL2_HOST_SIZE1_FDQ,  ((Uint32)&hostDescriptor_LL2[LL2_SIZE1_DESC_START_IDX][0])+0x11000000,  GE_DESCRIPTOR_SIZE,   LL2_SIZE1_PKT_BUF_NUM,      ((Uint32)packetBuffer_LL2_Size1)+0x11000000,    LL2_PACKET_BUFFER_SIZE1},/*CORE1 LL2 Size1 host Free Descriptor Queue*/

    {CORE2_LL2_HOST_SIZE0_FDQ,  ((Uint32)&hostDescriptor_LL2[0][0])+0x12000000,                         GE_DESCRIPTOR_SIZE,   LL2_SIZE0_PKT_BUF_NUM,      ((Uint32)packetBuffer_LL2_Size0)+0x12000000,    LL2_PACKET_BUFFER_SIZE0},/*CORE2 LL2 Size0 host Free Descriptor Queue*/
    {CORE2_LL2_HOST_SIZE1_FDQ,  ((Uint32)&hostDescriptor_LL2[LL2_SIZE1_DESC_START_IDX][0])+0x12000000,  GE_DESCRIPTOR_SIZE,   LL2_SIZE1_PKT_BUF_NUM,      ((Uint32)packetBuffer_LL2_Size1)+0x12000000,    LL2_PACKET_BUFFER_SIZE1},/*CORE2 LL2 Size1 host Free Descriptor Queue*/

    {CORE3_LL2_HOST_SIZE0_FDQ,  ((Uint32)&hostDescriptor_LL2[0][0])+0x13000000,                         GE_DESCRIPTOR_SIZE,   LL2_SIZE0_PKT_BUF_NUM,      ((Uint32)packetBuffer_LL2_Size0)+0x13000000,    LL2_PACKET_BUFFER_SIZE0},/*CORE3 LL2 Size0 host Free Descriptor Queue*/
    {CORE3_LL2_HOST_SIZE1_FDQ,  ((Uint32)&hostDescriptor_LL2[LL2_SIZE1_DESC_START_IDX][0])+0x13000000,  GE_DESCRIPTOR_SIZE,   LL2_SIZE1_PKT_BUF_NUM,      ((Uint32)packetBuffer_LL2_Size1)+0x13000000,    LL2_PACKET_BUFFER_SIZE1},/*CORE3 LL2 Size1 host Free Descriptor Queue*/

    {DDR_HOST_SIZE0_FDQ,        (Uint32)&hostDescriptor_DDR[0][0],                                      GE_DESCRIPTOR_SIZE,   DDR_SIZE0_PKT_BUF_NUM,      (Uint32)packetBuffer_DDR_Size0,                 DDR_PACKET_BUFFER_SIZE0},/*DDR Size0 host Free Descriptor Queue*/
    {DDR_HOST_SIZE1_FDQ,        (Uint32)&hostDescriptor_DDR[DDR_SIZE1_DESC_START_IDX][0],               GE_DESCRIPTOR_SIZE,   DDR_SIZE1_PKT_BUF_NUM,      (Uint32)packetBuffer_DDR_Size1,                 DDR_PACKET_BUFFER_SIZE1}/*DDR Size1 host Free Descriptor Queue*/
};

PktDma_RxFlowCfg flowCfgTable[]=
{
    /*dest_qnum*/ /*sop_offset*/ /*ps_loc*/  /*desc_type*/   /*error retry*/ /*psinfo*/  /*einfo*/  /*size_thresh0(1,2)_en*/ /*size_thresh0,1,2*/ /*fdq0_sz0_qnum*/         /*fdq0_sz1_qnum, sz2_qnum, sz3_qnum*/ /*fdq1_qnum*/           /*fdq2_qnum*/           /*fdq3_qnum*/  /*dest_tag_lo*/ /*dest_tag_hi*/ /*src_tag_lo*/  /*src_tag_hi*/  /*dest_tag_lo_sel*/ /*dest_tag_hi_sel*/ /*src_tag_lo_sel*/  /*src_tag_hi_sel*/
    {GE_RX_DST_Q    ,    0,      0,      Cppi_DescType_HOST,     0,          1,          0,          1, 0, 0,    DDR_PACKET_BUFFER_SIZE0, 0, 0, DDR_HOST_SIZE0_FDQ,       DDR_HOST_SIZE1_FDQ,       0, 0,   DDR_HOST_SIZE1_FDQ,       DDR_HOST_SIZE1_FDQ,       DDR_HOST_SIZE1_FDQ,         0,      0,              0,              0,              0,                  0,                  0,                  0},
    {GE_RX_DST_Q    ,    0,      0,      Cppi_DescType_HOST,     0,          1,          0,          1, 0, 0,    SL2_PACKET_BUFFER_SIZE0, 0, 0, SL2_HOST_SIZE0_FDQ,       SL2_HOST_SIZE1_FDQ,       0, 0,   SL2_HOST_SIZE1_FDQ,       SL2_HOST_SIZE1_FDQ,       SL2_HOST_SIZE1_FDQ,         0,      0,              0,              0,              0,                  0,                  0,                  0}
};

char * FDQ_str[]=
{
    "SL2_HOST_SIZE0_FDQ",      
    "SL2_HOST_SIZE1_FDQ",      
    "CORE0_LL2_HOST_SIZE0_FDQ",
    "CORE0_LL2_HOST_SIZE1_FDQ",
    "CORE1_LL2_HOST_SIZE0_FDQ",
    "CORE1_LL2_HOST_SIZE1_FDQ",
    "CORE2_LL2_HOST_SIZE0_FDQ",
    "CORE2_LL2_HOST_SIZE1_FDQ",
    "CORE3_LL2_HOST_SIZE0_FDQ",
    "CORE3_LL2_HOST_SIZE1_FDQ",
    "DDR_HOST_SIZE0_FDQ",      
    "DDR_HOST_SIZE1_FDQ"      
};

char * flow_str[]=
{
	"FLOW_TO_DDR",
	"FLOW_TO_SL2",
	"FLOW_TO_CORE0_LL2",
	"FLOW_TO_CORE1_LL2",
	"FLOW_TO_CORE2_LL2",
	"FLOW_TO_CORE3_LL2"
};

/*Descriptor accumulation buffer*/
Uint32 uiaDescriptorAccumulationList[(MAX_ACCU_PAGE_SIZE+1)*2];

Uint32 uiUsedDescRegionNum;
Uint32 uiInitialHostFdqNum;
Uint32 uiUsedRxFlowNum;

/*must be called after GE PSC enabled*/
void GE_PktDMA_init()
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
#ifdef ACC_48_CHANNEL
	KeyStone_Qmss_Download_Firmware(Qmss_PdspId_PDSP1, &acc48_be, sizeof(acc48_be));
#else
	KeyStone_Qmss_Download_Firmware(Qmss_PdspId_PDSP1, &acc32_be, sizeof(acc32_be));
	KeyStone_Qmss_Download_Firmware(Qmss_PdspId_PDSP2, &acc16_be, sizeof(acc16_be));
#endif
#else
#ifdef ACC_48_CHANNEL
	KeyStone_Qmss_Download_Firmware(Qmss_PdspId_PDSP1, &acc48_le, sizeof(acc48_le));
#else
	KeyStone_Qmss_Download_Firmware(Qmss_PdspId_PDSP1, &acc32_le, sizeof(acc32_le));
	KeyStone_Qmss_Download_Firmware(Qmss_PdspId_PDSP2, &acc16_le, sizeof(acc16_le));
#endif
#endif
	gpQM_INTD_regs->STATUS_CLR_REG0= 0xFFFFFFFF;
	gpQM_INTD_regs->STATUS_CLR_REG1= 0xFFFF;

	KeyStone_Qmss_Config_Reclaim_Queue (Qmss_PdspId_PDSP1, RECLAMATION_QUEUE);
	
	/*-----------------GE PktDMA configuration------------------*/
	KeyStone_pktDma_Global_Control(gpNetCP_DMA_CfgRegs, 256, 5, 5);

	gpNetCP_DMA_CfgRegs->EMULATION_CONTROL_REG=
		(0<<CSL_CPPIDMA_GLOBAL_CONFIG_EMULATION_CONTROL_REG_LOOPBACK_EN_SHIFT) /*1->For Packet DMA loopback test only*/
		|(1<<CSL_CPPIDMA_GLOBAL_CONFIG_EMULATION_CONTROL_REG_SOFT_SHIFT)
		|(0<<CSL_CPPIDMA_GLOBAL_CONFIG_EMULATION_CONTROL_REG_FREE_SHIFT);

	for(i=0; i< GE_NUM_ETHERNET_PORT; i++)
		KeyStone_pktDma_RxCh_enable(gpNetCP_DMA_RxChCfgRegs, GE_DIRECT_RX_PORT1_CHANNEL+i);
	KeyStone_pktDma_RxCh_enable(gpNetCP_DMA_RxChCfgRegs , GE_DIRECT_TX_CHANNEL);//For Packet DMA loopback test only
	KeyStone_pktDma_TxCh_enable(gpNetCP_DMA_TxChCfgRegs , GE_DIRECT_TX_CHANNEL);
	
	KeyStone_pktDma_configureRxFlow(
			&gpNetCP_DMA_RxFlowCfgRegs->RX_FLOW_CONFIG[GE_DIRECT_RX_PORT1_CHANNEL], flowCfgTable, uiUsedRxFlowNum);

}

/*ping/pong switch of descriptor accumulation buffer*/
Uint32 uiAccPingPong; 	
Uint32 uiAccPageSize;
void GE_QMSS_Accumulation_config(Uint8 channel, Uint16 pageSize,
	Qmss_AccPacingMode  interruptPacingMode)
{
	Qmss_AccCmdCfg hiQuAccCfg;

	uiAccPingPong=0;
	uiAccPageSize= pageSize;

	if(MAX_ACCU_PAGE_SIZE<pageSize)
		pageSize= MAX_ACCU_PAGE_SIZE;

	hiQuAccCfg.channel = channel;
	hiQuAccCfg.command = Qmss_AccCmd_ENABLE_CHANNEL;
	hiQuAccCfg.queueEnMask = 1;
	hiQuAccCfg.listAddress = GLOBAL_ADDR(uiaDescriptorAccumulationList);
	hiQuAccCfg.queMgrIndex = GE_RX_DST_Q;
	hiQuAccCfg.maxPageEntries = pageSize+1;
	hiQuAccCfg.timerLoadCount = 5;
	hiQuAccCfg.interruptPacingMode = interruptPacingMode;
	hiQuAccCfg.listEntrySize = Qmss_AccEntrySize_REG_D;
	hiQuAccCfg.listCountMode = Qmss_AccCountMode_ENTRY_COUNT;
	hiQuAccCfg.multiQueueMode = Qmss_AccQueueMode_SINGLE_QUEUE;
	KeyStone_Qmss_Config_Acc_Channel(Qmss_PdspId_PDSP1, &hiQuAccCfg);

}

/*check the descriptors in the queue, for debug purpose*/
void GE_Check_Free_Queues()
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
			printf("number of descriptors in host queue %s (%d) changed from %d to %d\n",
				FDQ_str[i], uiQuNum, uiDescriptorNumber, uiEntryCount);

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

/*check the descriptors in the queue, for debug purpose*/
void GE_Check_TxRx_Queues()
{
	int i;
	Uint32 uiQuNum, uiEntryCount;
	
	_mfence(); 	//force all memory operation complete
	_mfence(); 	//force all memory operation complete

	uiEntryCount= 
		gpQueueStatusConfigRegs[GE_DIRECT_TX_QUEUE].REG_A_EntryCount;
	if(uiEntryCount)
		printf("TX queue %d entry count = %d\n", GE_DIRECT_TX_QUEUE, uiEntryCount);

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

void GE_Check_Queue(Uint32 uiQIdx, Uint32 uiExpectEntryCount)
{
	Uint32 uiEntryCount;
	
	_mfence(); 	//force all memory operation complete
	_mfence(); 	//force all memory operation complete
	uiEntryCount= 
		gpQueueStatusConfigRegs[uiQIdx].REG_A_EntryCount;
	if(uiEntryCount!=uiExpectEntryCount)
		printf("queue %d entry count = %d\n", uiQIdx, uiEntryCount);
}
