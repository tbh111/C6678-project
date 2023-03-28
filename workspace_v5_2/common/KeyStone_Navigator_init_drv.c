/******************************************************************************

  Copyright (C), 2001-2012, Texas Instrument.

 ******************************************************************************
  File Name     : KeyStone_Navigator_init_drv.c
  Version       : Initial Draft
  Author        : Zhan
  Created       : 2012/10/30
  Last Modified :
  Description   : Multicore Navigator initialization and driver

  Function List :
              KeyStone_Copy_FDQ
              KeyStone_Host_Descriptor_Queues_init
              KeyStone_Mono_Descriptor_Queues_init
              KeyStone_pktDma_configureRxFlow
              KeyStone_pktDma_Global_Control
              KeyStone_pktDma_TxCh_config
              KeyStone_Qmss_Config_Acc_Channel
              KeyStone_Qmss_config_Acc_Timer
              KeyStone_Qmss_Config_Reclaim_Queue
              KeyStone_QMSS_Descriptor_Regions_init
              KeyStone_Qmss_disable_Acc_Channel
              KeyStone_Qmss_Download_Firmware
              KeyStone_QMSS_Linking_RAM_init
  History       :
  1.Date        : 2011/4/30
    Author      : Brighton Feng
    Modification: Created file
******************************************************************************/
#include <c6x.h>
#include <stdio.h>
#include <string.h>
#include <cslr_device.h>
#include <csl_cacheAux.h>
#include "KeyStone_common.h"
#include "KeyStone_Navigator_init_drv.h"
/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/
/*Queue Manager definition*/
CSL_Qm_configRegs * gpQM_configRegs= (CSL_Qm_configRegs *)CSL_QM_SS_CFG_CONFIG_STARVATION_COUNTER_REGS;
QMSS_DescriptorMemoryRegionRegs * gpQM_descriptorRegions= 
	(QMSS_DescriptorMemoryRegionRegs *)CSL_QM_SS_CFG_DESCRIPTION_REGS;

/*queue access registers through VBUSP configration bus*/
QueueManageRegs * gpQueueManageRegs= 
	(QueueManageRegs *)CSL_QM_SS_CFG_QM_QUEUE_DEQUEUE_REGS;
QueueManageRegs * gpQueueManageProxyRegs= 
	(QueueManageRegs *)CSL_QM_SS_CFG_PROXY_QUEUE_DEQUEUE_REGS;

/*queue access data space through VBUSM data bus*/
QueueManageRegs * gpQueueManageVBUSM= 
	(QueueManageRegs *)CSL_QM_SS_DATA_QM_QUEUE_DEQUEUE_REGS;
QueueManageRegs * gpQueueManageProxyVBUSM= 
	(QueueManageRegs *)CSL_QM_SS_DATA_PROXY_QUEUE_DEQUEUE_REGS;

QueueStatusConfigRegs * gpQueueStatusConfigRegs= 
	(QueueStatusConfigRegs *)CSL_QM_SS_CFG_QUE_PEEK_REGS;
volatile Uint32 * gpQueueThresholdStatus= (volatile Uint32 *)CSL_QM_SS_CFG_QM_STATUS_RAM_REGS;
CSL_Qm_intdRegs * gpQM_INTD_regs= (CSL_Qm_intdRegs *)CSL_QM_SS_CFG_INTD_REGS;


/*PDSP definition*/
volatile Uint32 * gpQM_PDSP_IRAM[2] = 
{
	(volatile Uint32 * )CSL_QM_SS_CFG_APDSP1_RAM_REGS,
	(volatile Uint32 * )CSL_QM_SS_CFG_APDSP2_RAM_REGS
};
CSL_PdspRegs * gpQM_PDSP_CtrlRegs[2]= 
{
(CSL_PdspRegs * )CSL_QM_SS_CFG_ADSP1_REGS,
(CSL_PdspRegs * )CSL_QM_SS_CFG_ADSP2_REGS
};
volatile Uint32 * gpQM_PDSP_Cmd[2]= 
{
	(volatile Uint32 * )CSL_QM_SS_CFG_SCRACH_RAM1_REGS,
	(volatile Uint32 * )CSL_QM_SS_CFG_SCRACH_RAM2_REGS
};

/*PacketDMA definition*/
CSL_Cppidma_global_configRegs * gpQM_DMA_CfgRegs= 
	(CSL_Cppidma_global_configRegs * )CSL_QM_SS_CFG_CPPI_DMA_GLOBAL_CFG_REGS;
CSL_Cppidma_tx_channel_configRegs * gpQM_DMA_TxChCfgRegs= 
	(CSL_Cppidma_tx_channel_configRegs * )CSL_QM_SS_CFG_CPPI_DMA_TX_CFG_REGS;
CSL_Cppidma_rx_channel_configRegs * gpQM_DMA_RxChCfgRegs= 
	(CSL_Cppidma_rx_channel_configRegs * )CSL_QM_SS_CFG_CPPI_DMA_RX_CFG_REGS;
volatile Uint32 * gpQM_DMA_TxChPriority = 
	(volatile Uint32 *)CSL_QM_SS_CFG_CPPI_DMA_TX_SCHEDULER_CFG_REGS;
CSL_Cppidma_rx_flow_configRegs * gpQM_DMA_RxFlowCfgRegs= 
	(CSL_Cppidma_rx_flow_configRegs * )CSL_QM_SS_CFG_CPPI_DMA_RX_FLOW_CFG_REGS;
PKT_DMA_Regs gpQM_PKTDMA_regs=
{
	(CSL_Cppidma_global_configRegs * )CSL_QM_SS_CFG_CPPI_DMA_GLOBAL_CFG_REGS,
	(CSL_Cppidma_tx_channel_configRegs * )CSL_QM_SS_CFG_CPPI_DMA_TX_CFG_REGS,
	(CSL_Cppidma_rx_channel_configRegs * )CSL_QM_SS_CFG_CPPI_DMA_RX_CFG_REGS,
	(volatile Uint32 *)CSL_QM_SS_CFG_CPPI_DMA_TX_SCHEDULER_CFG_REGS,
	(CSL_Cppidma_rx_flow_configRegs * )CSL_QM_SS_CFG_CPPI_DMA_RX_FLOW_CFG_REGS
};

CSL_Cppidma_global_configRegs * gpSRIO_DMA_CfgRegs= 
	(CSL_Cppidma_global_configRegs * )CSL_SRIO_CONFIG_CPPI_DMA_GLOBAL_CFG_REGS;
CSL_Cppidma_tx_channel_configRegs * gpSRIO_DMA_TxChCfgRegs= 
	(CSL_Cppidma_tx_channel_configRegs * )CSL_SRIO_CONFIG_CPPI_DMA_TX_CFG_REGS;
CSL_Cppidma_rx_channel_configRegs * gpSRIO_DMA_RxChCfgRegs= 
	(CSL_Cppidma_rx_channel_configRegs * )CSL_SRIO_CONFIG_CPPI_DMA_RX_CFG_REGS;
volatile Uint32 * gpSRIO_DMA_TxChPriority = 
	(volatile Uint32 *)CSL_SRIO_CONFIG_CPPI_DMA_TX_SCHEDULER_CFG_REGS;
CSL_Cppidma_rx_flow_configRegs * gpSRIO_DMA_RxFlowCfgRegs= 
	(CSL_Cppidma_rx_flow_configRegs * )CSL_SRIO_CONFIG_CPPI_DMA_RX_FLOW_CFG_REGS;
PKT_DMA_Regs gpSRIO_PKTDMA_regs=
{
	(CSL_Cppidma_global_configRegs * )CSL_SRIO_CONFIG_CPPI_DMA_GLOBAL_CFG_REGS,
	(CSL_Cppidma_tx_channel_configRegs * )CSL_SRIO_CONFIG_CPPI_DMA_TX_CFG_REGS,
	(CSL_Cppidma_rx_channel_configRegs * )CSL_SRIO_CONFIG_CPPI_DMA_RX_CFG_REGS,
	(volatile Uint32 *)CSL_SRIO_CONFIG_CPPI_DMA_TX_SCHEDULER_CFG_REGS,
	(CSL_Cppidma_rx_flow_configRegs * )CSL_SRIO_CONFIG_CPPI_DMA_RX_FLOW_CFG_REGS
};

CSL_Cppidma_global_configRegs * gpNetCP_DMA_CfgRegs= 
	(CSL_Cppidma_global_configRegs * )CSL_PA_SS_CFG_CPPI_DMA_GLOBAL_CFG_REGS;
CSL_Cppidma_tx_channel_configRegs * gpNetCP_DMA_TxChCfgRegs= 
	(CSL_Cppidma_tx_channel_configRegs * )CSL_PA_SS_CFG_CPPI_DMA_TX_CFG_REGS;
CSL_Cppidma_rx_channel_configRegs * gpNetCP_DMA_RxChCfgRegs= 
	(CSL_Cppidma_rx_channel_configRegs * )CSL_PA_SS_CFG_CPPI_DMA_RX_CFG_REGS;
volatile Uint32 * gpNetCP_DMA_TxChPriority = 
	(volatile Uint32 *)CSL_PA_SS_CFG_CPPI_DMA_TX_SCHEDULER_CFG_REGS;
CSL_Cppidma_rx_flow_configRegs * gpNetCP_DMA_RxFlowCfgRegs= 
	(CSL_Cppidma_rx_flow_configRegs * )CSL_PA_SS_CFG_CPPI_DMA_RX_FLOW_CFG_REGS;
PKT_DMA_Regs gpNetCP_PKTDMA_regs=
{
	(CSL_Cppidma_global_configRegs * )CSL_PA_SS_CFG_CPPI_DMA_GLOBAL_CFG_REGS,
	(CSL_Cppidma_tx_channel_configRegs * )CSL_PA_SS_CFG_CPPI_DMA_TX_CFG_REGS,
	(CSL_Cppidma_rx_channel_configRegs * )CSL_PA_SS_CFG_CPPI_DMA_RX_CFG_REGS,
	(volatile Uint32 *)CSL_PA_SS_CFG_CPPI_DMA_TX_SCHEDULER_CFG_REGS,
	(CSL_Cppidma_rx_flow_configRegs * )CSL_PA_SS_CFG_CPPI_DMA_RX_FLOW_CFG_REGS
};

Uint32 numLinkingEntry= 0; 	/*total number of linking entries*/
/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/*****************************************************************************
 Prototype    : KeyStone_QMSS_Linking_RAM_init
 Description  : use QMSS internal linking RAM for region 0
                if linkingRAM1!=NULL, use linkingRAM1 for linking RAM region
                1
 Input        : unsigned long long * linkingRAM1  
                Uint32 linkingRMA1NumEntry        
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/4/30
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_QMSS_Linking_RAM_init(unsigned long long * linkingRAM1,
	Uint32 linkingRMA1NumEntry)
{
	int i;
	
	/*use QMSS internal linking RAM for region 0*/
	gpQM_configRegs->LINKING_RAM_REGION_0_BASE_ADDRESS_REG= QMSS_LINKING_RAM_OFFSET;
	gpQM_configRegs->LINKING_RAM_REGION_0_SIZE_REG= QMSS_LINKING_RAM_REGION_0_DEFAULT_SIZE;
	numLinkingEntry= QMSS_LINKING_RAM_REGION_0_DEFAULT_SIZE+1;

	/*configure linking RAM region 1*/
	if(NULL!=linkingRAM1&&linkingRMA1NumEntry)
	{
		gpQM_configRegs->LINKING_RAM_REGION_1_BASE_ADDRESS_REG= GLOBAL_ADDR(linkingRAM1);
		numLinkingEntry+= linkingRMA1NumEntry;
	}
	
	//empty all queues
	for(i=0; i<8192 ; i++)
		gpQueueManageRegs[i].REG_D_Descriptor= 0;
	
}

/*****************************************************************************
 Prototype    : KeyStone_QMSS_Descriptor_Regions_init
 Description  : initialize descriptor regions according to "descMemRegionCfg",
                "uiNumDescRegion" is the number of regions to be initialized
                
 Input        : Qmss_DescMemRegionCfg * descMemRegionCfg  
                Uint32 uiNumDescRegion                    
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/4/30
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_QMSS_Descriptor_Regions_init(
	Qmss_DescMemRegionCfg * descMemRegionCfg, Uint32 uiNumDescRegion)
{
	int i;
	Uint32 uiLinkEntryStartIndex= 0;
	Uint32 uiNumLinkEntry; //number of linking entries used by a region

	/*check the region configuration*/
	for(i=0; i< uiNumDescRegion-1; i++)
	{
		if((descMemRegionCfg[i].descBase + 
			descMemRegionCfg[i].descNum*descMemRegionCfg[i].descSize) >
			descMemRegionCfg[i+1].descBase)
		{
			printf("Error: descriptor region %d RAM overlap with descriptor region %d RAM\n", 
				i, i+1);
			return;
		}
	}

	//set registers with configuration values
	for(i=0; i< uiNumDescRegion; i++)
	{
		if(descMemRegionCfg[i].descSize&0xf)
		{
			printf("Error: size of descriptor region %d is %d, not multiple 16 bytes\n",
				i, descMemRegionCfg[i].descSize);
			return;
		}
		if(descMemRegionCfg[i].descSize>(128*1024))
		{
			printf("Error: size of descriptor in region %d is %d > 128K\n",
				i, descMemRegionCfg[i].descSize);
			return;
		}

		if(uiLinkEntryStartIndex+descMemRegionCfg[i].descNum>numLinkingEntry)
		{
			printf("Error: descriptor region %d last index %d exceeds linking RAM entry size %d\n",
				i, uiLinkEntryStartIndex+descMemRegionCfg[i].descNum-1, numLinkingEntry);
			return;
		}

		/*make sure the linking entries used by a region is legal*/
		if(descMemRegionCfg[i].descNum>(1024*1024))
		{
			printf("Error: number of descriptors in region %d is %d > 1M\n",
				i, descMemRegionCfg[i].descNum);
			return;
		}

		if(descMemRegionCfg[i].descNum<32)
		{
			//printf("Waring: number of descriptors in region %d is %d < 32, round the linking entry number up to 32\n",
				//i, descMemRegionCfg[i].descNum);
			uiNumLinkEntry = 32;
		}
		else if(1==(_dotpu4(_bitc4(descMemRegionCfg[i].descNum), 0x01010101)))
		{
			//number of descriptors in region is power of 2
			uiNumLinkEntry =  descMemRegionCfg[i].descNum;
		}
		else
		{
			uiNumLinkEntry =  1<<(31-_lmbd(1, descMemRegionCfg[i].descNum)+1);
			//printf("Waring: number of descriptors in region %d is %d, not power of 2, round the linking entry number up to %d\n",
				//i, descMemRegionCfg[i].descNum, uiNumLinkEntry);
		}


		gpQM_descriptorRegions[i].BASE_ADDRESS_REG=
				GLOBAL_ADDR(descMemRegionCfg[i].descBase);
		gpQM_descriptorRegions[i].START_INDEX_REG= uiLinkEntryStartIndex;
		gpQM_descriptorRegions[i].DESCRIPTOR_SETUP_REG=
			(((descMemRegionCfg[i].descSize>>4)-1)<<16)|
			((31-_lmbd(1, uiNumLinkEntry)-5));

		uiLinkEntryStartIndex+= uiNumLinkEntry;
	}	

}
/*****************************************************************************
 Prototype    : KeyStone_Host_Descriptor_Queues_init
 Description  : initialize host descriptors and queues according to "hostQu-
                Cfg",
                "uiQuCfgNumber" is number of Free Descriptor queues to be
                initalized
 Input        : FreeHostQueueCfg *hostQuCfg  
                Uint32 uiQuCfgNumber         
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/4/30
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_Host_Descriptor_Queues_init(
	FreeHostQueueCfg *hostQuCfg, Uint32 uiQuCfgNumber)
{
	int i, j;
	Uint32 uiTempletSize, uiBufferAddress, uiDescriptorAddress;
	HostPacketDescriptor descriptorTemplet, * descriptor;

	uiTempletSize= sizeof(descriptorTemplet);
	memset(&descriptorTemplet, 0, uiTempletSize); 

	descriptorTemplet.type_id= Cppi_DescType_HOST;
	descriptorTemplet.ret_push_policy= 0 ; //return to queue tail
	descriptorTemplet.return_policy= 1 ; //each descriptor separately goes to pkt_return_qnum

	for(j=0; j<uiQuCfgNumber; j++)
	{
		/*initialze buffer size*/
		//descriptorTemplet.packet_length= hostQuCfg[j].uiBufferSize;
		descriptorTemplet.buffer_len= hostQuCfg[j].uiBufferSize;
		descriptorTemplet.orig_buff0_len= hostQuCfg[j].uiBufferSize;

		/*initialize this free queue as return queue number*/
		descriptorTemplet.pkt_return_qmgr= hostQuCfg[j].uiFreeQuNum>>12;
		descriptorTemplet.pkt_return_qnum= hostQuCfg[j].uiFreeQuNum&0xFFF;

		uiBufferAddress= hostQuCfg[j].uiBufferAddress;
		uiDescriptorAddress= hostQuCfg[j].uiDescriptorAddress;
		for(i=0; i<hostQuCfg[j].uiDescriptorNumber; i++)
		{
			/*copy contents in templet*/
			memcpy((void *)uiDescriptorAddress, 
				(void *)&descriptorTemplet, uiTempletSize);

			/*initialize buffer address*/
			descriptor = (HostPacketDescriptor *)uiDescriptorAddress;
			descriptor->buffer_ptr= uiBufferAddress;
			descriptor->orig_buff0_ptr= uiBufferAddress;
			WritebackCache((void *)uiDescriptorAddress, uiTempletSize);

			/*push this descriptor to free queue*/
			KeyStone_queuePush(hostQuCfg[j].uiFreeQuNum, 
				uiDescriptorAddress|FETCH_SIZE_32);
			
			/*point to next descriptor and buffer*/
			uiDescriptorAddress+= hostQuCfg[j].uiDescriptorSize;
			uiBufferAddress+= hostQuCfg[j].uiBufferSize;
		
		}
	}
}

/*****************************************************************************
 Prototype    : KeyStone_Mono_Descriptor_Queues_init
 Description  : initialize monolithic descriptors and queues according to
                "monoQuCfg",
                "uiQuCfgNumber" is number of Free Descriptor queues to be
                initalized
 Input        : FreeMonoQueueCfg *monoQuCfg  
                Uint32 uiQuCfgNumber         
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/4/30
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_Mono_Descriptor_Queues_init(
	FreeMonoQueueCfg *monoQuCfg, Uint32 uiQuCfgNumber)
{
	int i, j;
	Uint32 uiTempletSize, uiDescriptorAddress;
	MonolithicPacketDescriptor descriptorTemplet;

	uiTempletSize= sizeof(descriptorTemplet);
	memset(&descriptorTemplet, 0, uiTempletSize); 

	descriptorTemplet.type_id= Cppi_DescType_MONOLITHIC;
	descriptorTemplet.ret_push_policy= 0; //return to queue tail
	descriptorTemplet.data_offset= 16;

	for(j=0; j<uiQuCfgNumber; j++)
	{
		/*
		descriptorTemplet.packet_length= 
			monoQuCfg[j].uiDescriptorSize- uiTempletSize;
		*/
		/*initialize this free queue as return queue number*/
		descriptorTemplet.pkt_return_qmgr= monoQuCfg[j].uiFreeQuNum>>12;
		descriptorTemplet.pkt_return_qnum= monoQuCfg[j].uiFreeQuNum&0xFFF;

		uiDescriptorAddress= monoQuCfg[j].uiDescriptorAddress;
		for(i=0; i<monoQuCfg[j].uiDescriptorNumber; i++)
		{
			/*copy contents in templet*/
			memcpy((void *)uiDescriptorAddress, 
				(void *)&descriptorTemplet, uiTempletSize);
			WritebackCache((void *)uiDescriptorAddress, uiTempletSize);

			/*push this descriptor to free queue*/
			KeyStone_queuePush(monoQuCfg[j].uiFreeQuNum, 
				uiDescriptorAddress|FETCH_SIZE_16);
			
			/*point to next descriptor and buffers*/
			uiDescriptorAddress+= monoQuCfg[j].uiDescriptorSize;

		}
	}
}
/*****************************************************************************
 Prototype    : KeyStone_Copy_FDQ
 Description  : Copy descriptors from one FDQ to a new FDQ
                normally used for create a small FDQ from a large FDQ pool
 Input        : Uint32 dstFDQ   
                Uint32 srcFDQ   
                Uint32 numDesc  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/4/30
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_Copy_FDQ(Uint32 dstFDQ, Uint32 srcFDQ, Uint32 numDesc)
{
	int i;
	Uint32 uiDescriptor;

	/*define this pointer for modifing return queue, this field is same
	host and monolithic descriptor, here use monolithic descriptor pointer,
	but it also works for host descriptor*/
	MonolithicPacketDescriptor * descriptor;

	for(i=0; i<numDesc; i++)
	{
		uiDescriptor = KeyStone_queuePop(srcFDQ);
		if(NULL==uiDescriptor)
		{
			printf("Queue %d empty!\n", srcFDQ);
			return;
		}

		descriptor= (MonolithicPacketDescriptor * )uiDescriptor;

		/*modify the return queue number to the new FDQ*/
		descriptor->pkt_return_qnum = dstFDQ;
		descriptor->pkt_return_qmgr = dstFDQ>>12;
		
		/*write back data from cache to descriptor RAM*/
		WritebackCache((void *)descriptor, 64);

		KeyStone_queuePush(dstFDQ, uiDescriptor);
	}
}

/*****************************************************************************
 Prototype    : KeyStone_Qmss_Download_Firmware
 Description  : Downloads the PDSP firmware to PDSP.
 Input        : Qmss_PdspId pdspId  
                void *image         
                Uint32 size         
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/4/30
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_Qmss_Download_Firmware (
	Qmss_PdspId pdspId, void *image, Uint32 size)
{
    Uint32            i, count;
    volatile Uint32   enable;
    Uint32            *data = (Uint32 *) image;

    /* Reset the PDSP */
    gpQM_PDSP_CtrlRegs[pdspId]->PDSP_CONTROL_REG &= 
    	~CSL_PDSP_PDSP_CONTROL_REG_PDSP_ENABLE_MASK;

    /* Confirm PDSP has halted */
    do
    { 
        enable = gpQM_PDSP_CtrlRegs[pdspId]->PDSP_CONTROL_REG & 
        	CSL_PDSP_PDSP_CONTROL_REG_PDSP_STATE_MASK;
    }while (enable);

    count = size / 4;

    /* Download the firmware */
    for(i = 0; i < count; i++)
        gpQM_PDSP_IRAM[pdspId][i] = data[i];

    /* Use the command register to sync the PDSP */
    *gpQM_PDSP_Cmd[pdspId] = 0xFFFFFFFF;

    /* Wait to the memory write to land */
    for(i = 0; i < 20000 && *gpQM_PDSP_Cmd[pdspId] != 0xFFFFFFFF; i++);
   
    /* Reset program counter to zero */
	gpQM_PDSP_CtrlRegs[pdspId]->PDSP_CONTROL_REG &= 
		~CSL_PDSP_PDSP_CONTROL_REG_PCOUNTER_RST_VAL_MASK;
    
	/* Set soft reset to zero to load the program counter */
	gpQM_PDSP_CtrlRegs[pdspId]->PDSP_CONTROL_REG &=
		~CSL_PDSP_PDSP_CONTROL_REG_SOFT_RST_N_MASK;
    
    /* Enable the PDSP */
    gpQM_PDSP_CtrlRegs[pdspId]->PDSP_CONTROL_REG |=
    	CSL_PDSP_PDSP_CONTROL_REG_PDSP_ENABLE_MASK;

    /* Wait for the command register to clear */
    while (*gpQM_PDSP_Cmd[pdspId]);

    return;
}

/*****************************************************************************
 Prototype    : KeyStone_Qmss_Config_Acc_Channel
 Description  : This function programs the accumulator channel with values
                passed in the cfg structure
 Input        : Qmss_PdspId pdspId   
                Qmss_AccCmdCfg *cfg  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/4/30
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
Uint32 KeyStone_Qmss_Config_Acc_Channel (
	Qmss_PdspId pdspId, Qmss_AccCmdCfg *cfg)
{
    Uint32         cmd[5];
    volatile Uint32   *cmdPtr, *reg;
    Uint32            index;
    Uint8             result;

    memset ((void *) &cmd, 0, 20);
    CSL_FINSR (cmd[0], 7, 0, cfg->channel);
    CSL_FINSR (cmd[0], 15, 8, cfg->command);
    cmd[1] = cfg->queueEnMask;
    cmd[2] = cfg->listAddress;
    CSL_FINSR (cmd[3], 15, 0, cfg->queMgrIndex);
    CSL_FINSR (cmd[3], 31, 16, cfg->maxPageEntries);
    CSL_FINSR (cmd[4], 15, 0, cfg->timerLoadCount);
    CSL_FINSR (cmd[4], 17, 16, cfg->interruptPacingMode);
    CSL_FINSR (cmd[4], 19, 18, cfg->listEntrySize);
    CSL_FINSR (cmd[4], 20, 20, cfg->listCountMode);
    CSL_FINSR (cmd[4], 21, 21, cfg->multiQueueMode);
    
    /* Point to the accumulator command register's last word */
    reg = (Uint32 *) ((Uint8 *) gpQM_PDSP_Cmd[pdspId] + 4 * 4);

    /* Write command word last */
    cmdPtr = ((Uint32 *) &cmd) + 4;

    for (index = 0; index < 5; index++)
        *reg-- = *cmdPtr--;

    /* wait for the command to clear */
    reg++;
    do
    {
        result = CSL_FEXTR (*reg, 15, 8);  
    } while (result != 0);

	/*clear interrupts*/
	gpQM_INTD_regs->INTCNT_REG[cfg->channel]= 0 ;
	gpQM_INTD_regs->EOI_REG= cfg->channel+2;

    return (Uint32) (CSL_FEXTR (*reg, 31, 24));
}

/*****************************************************************************
 Prototype    : KeyStone_Qmss_disable_Acc_Channel
 Description  : This function disables the accumulator functionality for
                the specified channel
 Input        : Qmss_PdspId pdspId  
                Uint8 channel       
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/4/30
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
Uint32 KeyStone_Qmss_disable_Acc_Channel (
	Qmss_PdspId pdspId, Uint8 channel)
{
    Uint32         cmd[5];
    volatile Uint32   *cmdPtr, *reg;
    Uint32            index;
    Uint8             result;

    memset ((void *) &cmd, 0, 20);
    CSL_FINSR (cmd[0], 7, 0, channel);
    CSL_FINSR (cmd[0], 15, 8, Qmss_AccCmd_DISABLE_CHANNEL);

    /* Point to the accumulator command register's last word */
    reg = (Uint32 *) ((Uint8 *) gpQM_PDSP_Cmd[pdspId] + 4 * 4);

    /* Write command word last */
    cmdPtr = ((Uint32 *) &cmd) + 4;

    for (index = 0; index < 5; index++)
        *reg-- = *cmdPtr--;

    /* Wait for the command to clear */
    reg++;
    do
    {
        result = CSL_FEXTR (*reg, 15, 8);  
    } while (result != 0);

    return (Uint32) (CSL_FEXTR (*reg, 31, 24));
}

/**
 *      This function programs the timer constant used by the firmware to generate the timer tick.
 *
 *      The Accumulator time "tick" is controlled by a local timer connected to the PDSP core. 
 *      This timer has a programmable count based on the sub-system clock. When this count expires, 
 *      a local "tick" is registered in the firmware. The tick is used when timing channel interrupts 
 *      based on the "Timer Load Count" value supplied in the timer configuration.
 *
 *      The value of "Timer Constant" is the number of QM sub-system clocks divided by 2 that 
 *      comprise a single "tick" in the accumulator firmware.
 *
 *      For example, if the QM sub-system clock is 350MHz, and the desired firmware "tick" is 20us, 
 *      the proper Timer Constant for this command is computed as follows:
 *      
 *      Timer Constant = (350,000,000 cycles/sec) * (0.000020 sec) / (2 cycles)
 *      Timer Constant = 3,500
 *      
 *      The firmware initializes with a default constant value of 4375. This corresponds to firmware tick of 25us.
 */

/*****************************************************************************
 Prototype    : KeyStone_Qmss_config_Acc_Timer
 Description  : Configure the ACC timer
 Input        : Qmss_PdspId pdspId    
                Uint16 timerConstant  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/4/30
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
Uint32 KeyStone_Qmss_config_Acc_Timer (
	Qmss_PdspId pdspId, Uint16 timerConstant)
{

    volatile Uint32   *reg;
    Uint8             result;

    /* Point to the accumulator command register's last word */
    reg = (Uint32 *) ((Uint8 *) gpQM_PDSP_Cmd[pdspId] + 4);
    *reg-- = timerConstant;
   
    *reg = Qmss_AccCmd_CONFIG_TIMER_CONSTANT << 8; 

    /* wait for the command to clear */
    do
    {
        result = CSL_FEXTR (*reg, 15, 8);  
    } while (result != 0);

    return (Uint32) (CSL_FEXTR (*reg, 31, 24));
}


/**
 *      The Accumulator firmware also includes an optional reclamation queue which can be used for 
 *      packet discards. Any descriptor placed on the reclamation queue will be recycled in the 
 *      same manner as if it had been submitted to CDMA. 
 *
 *      The descriptor packet information field is used to determine the return queue and the 
 *      return handling, including options to unlink host descriptors and push to either the 
 *      front or the back of the return queue.
 *      Setting queue to zero disables the reclamation feature
 */
/*****************************************************************************
 Prototype    : KeyStone_Qmss_Config_Reclaim_Queue
 Description  : Configure teh reclaim queue
 Input        : Qmss_PdspId pdspId    
                Uint32 uiQueueNumber  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/4/30
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
Uint32 KeyStone_Qmss_Config_Reclaim_Queue (
	Qmss_PdspId pdspId, Uint32 uiQueueNumber)
{
    volatile Uint32   *reg;
    Uint8             result;

    /* Point to the accumulator command register's last word */
    reg = (Uint32 *) ((Uint8 *) gpQM_PDSP_Cmd[pdspId] + 4);
    *reg-- = uiQueueNumber;
    *reg = Qmss_AccCmd_CONFIG_RECLAIM_QUEUE << 8;

    /* wait for the command to clear */
    do
    {
        result = CSL_FEXTR (*reg, 15, 8);  
    } while (result != 0);

    return (Uint32) (CSL_FEXTR (*reg, 31, 24));
}

/*****************************************************************************
 Prototype    : KeyStone_pktDma_Global_Control
 Description  : configure the global control registers of a packet DMA
 Input        : CSL_Cppidma_global_configRegs * pktDmaCfgRegs  
                Uint32 uiStarvationWaitCycles                  
                Uint32 rxPriority                              
                Uint32 txPriority                              
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/4/30
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_pktDma_Global_Control (
	CSL_Cppidma_global_configRegs * pktDmaCfgRegs,
	Uint32 uiStarvationWaitCycles, Uint32 rxPriority, Uint32 txPriority)
{
	pktDmaCfgRegs->PERF_CONTROL_REG = (pktDmaCfgRegs->PERF_CONTROL_REG&
		(~CSL_CPPIDMA_GLOBAL_CONFIG_PERF_CONTROL_REG_TIMEOUT_CNT_MASK))|
		(uiStarvationWaitCycles<<
		CSL_CPPIDMA_GLOBAL_CONFIG_PERF_CONTROL_REG_TIMEOUT_CNT_SHIFT);

	pktDmaCfgRegs->PRIORITY_CONTROL_REG = 
		(rxPriority<<
		CSL_CPPIDMA_GLOBAL_CONFIG_PRIORITY_CONTROL_REG_RX_PRIORITY_SHIFT)|
		(txPriority<<
		CSL_CPPIDMA_GLOBAL_CONFIG_PRIORITY_CONTROL_REG_TX_PRIORITY_SHIFT);

	pktDmaCfgRegs->QM_BASE_ADDRESS_REG[0]= 0x34020000; 	/*queue 0~4095*/
	pktDmaCfgRegs->QM_BASE_ADDRESS_REG[1]= 0x34030000; 	/*queue 4096~8191*/
}
/*****************************************************************************
 Prototype    : KeyStone_pktDma_TxCh_config
 Description  : Configure the TX channel
 Input        : PKT_DMA_Regs * pktDmaRegs       
                Uint32 uiChNum                  
                PktDma_TxChCfg * pktDmaTxChCfg  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/4/30
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_pktDma_TxCh_config(PKT_DMA_Regs * pktDmaRegs, Uint32 uiChNum,
	PktDma_TxChCfg * pktDmaTxChCfg)
{
	pktDmaRegs->txChPriority[uiChNum]= pktDmaTxChCfg->priority;

	pktDmaRegs->txChCfgRegs->TX_CHANNEL_GLOBAL_CONFIG[uiChNum].TX_CHANNEL_GLOBAL_CONFIG_REG_B= 
		(pktDmaTxChCfg->filterEINFO<<
		CSL_CPPIDMA_TX_CHANNEL_CONFIG_TX_CHANNEL_GLOBAL_CONFIG_REG_B_TX_FILT_EINFO_SHIFT)|
		(pktDmaTxChCfg->filterPS<<
		CSL_CPPIDMA_TX_CHANNEL_CONFIG_TX_CHANNEL_GLOBAL_CONFIG_REG_B_TX_FILT_PSWORDS_SHIFT)|
		(pktDmaTxChCfg->aifMonoMode<<
		CSL_CPPIDMA_TX_CHANNEL_CONFIG_TX_CHANNEL_GLOBAL_CONFIG_REG_B_TX_AIF_MONO_MODE_SHIFT);
}

/*****************************************************************************
 Prototype    : KeyStone_pktDma_configureRxFlow
 Description  : Configure the Rx flow
 Input        : CSL_Cppidma_rx_flow_configRx_flow_configRegs * rxFlowCfgRegs  
                PktDma_RxFlowCfg * cfg                                        
                Uint32 uiFlowNum                                              
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/4/30
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_pktDma_configureRxFlow (
	CSL_Cppidma_rx_flow_configRx_flow_configRegs * rxFlowCfgRegs, 
	PktDma_RxFlowCfg * cfg, Uint32 uiFlowNum)
{
	int i;
    Uint32      reg, temp;

	for(i=0; i<uiFlowNum; i++)
	{
	    reg = 0;
		/* Rx flow configuration register A */
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_A_RX_DEST_QNUM, cfg[i].rx_dest_qnum);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_A_RX_DEST_QMGR, cfg[i].rx_dest_qnum>>12);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_A_RX_SOP_OFFSET, cfg[i].rx_sop_offset);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_A_RX_PS_LOCATION, cfg[i].rx_ps_location);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_A_RX_DESC_TYPE, cfg[i].rx_desc_type);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_A_RX_ERROR_HANDLING, cfg[i].rx_error_handling);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_A_RX_PSINFO_PRESENT, cfg[i].rx_psinfo_present);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_A_RX_EINFO_PRESENT, cfg[i].rx_einfo_present);
	    rxFlowCfgRegs[i].RX_FLOW_CONFIG_REG_A = reg;

	    reg = 0;
	    /* Rx flow configuration register B */
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_B_RX_DEST_TAG_LO, cfg[i].rx_dest_tag_lo);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_B_RX_DEST_TAG_HI, cfg[i].rx_dest_tag_hi);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_B_RX_SRC_TAG_LO, cfg[i].rx_src_tag_lo);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_B_RX_SRC_TAG_HI, cfg[i].rx_src_tag_hi);
	    rxFlowCfgRegs[i].RX_FLOW_CONFIG_REG_B = reg;

	    reg = 0;
	    /* Rx flow configuration register C */		
	    temp = ((cfg[i].rx_size_thresh0_en) | (cfg[i].rx_size_thresh1_en << 1) | (cfg[i].rx_size_thresh2_en << 2));

	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_C_RX_SIZE_THRESH_EN, temp);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_C_RX_DEST_TAG_LO_SEL, cfg[i].rx_dest_tag_lo_sel);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_C_RX_DEST_TAG_HI_SEL, cfg[i].rx_dest_tag_hi_sel);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_C_RX_SRC_TAG_LO_SEL, cfg[i].rx_src_tag_lo_sel);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_C_RX_SRC_TAG_HI_SEL, cfg[i].rx_src_tag_hi_sel);
	    rxFlowCfgRegs[i].RX_FLOW_CONFIG_REG_C = reg;

	    reg = 0;
		/* Rx flow configuration register D */
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_D_RX_FDQ1_QNUM, cfg[i].rx_fdq1_qnum);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_D_RX_FDQ1_QMGR, cfg[i].rx_fdq1_qnum>>12);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_D_RX_FDQ0_SZ0_QNUM, cfg[i].rx_fdq0_sz0_qnum);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_D_RX_FDQ0_SZ0_QMGR, cfg[i].rx_fdq0_sz0_qnum>>12);
	    rxFlowCfgRegs[i].RX_FLOW_CONFIG_REG_D = reg;

	    reg = 0;
		/* Rx flow configuration register E */
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_E_RX_FDQ3_QNUM, cfg[i].rx_fdq3_qnum);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_E_RX_FDQ3_QMGR, cfg[i].rx_fdq3_qnum>>12);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_E_RX_FDQ2_QNUM, cfg[i].rx_fdq2_qnum);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_E_RX_FDQ2_QMGR, cfg[i].rx_fdq2_qnum>>12);
	    rxFlowCfgRegs[i].RX_FLOW_CONFIG_REG_E = reg;

	    reg = 0;
		/* Rx flow configuration register F */
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_F_RX_SIZE_THRESH1, (cfg[i].rx_size_thresh1 >> 5));
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_F_RX_SIZE_THRESH0, (cfg[i].rx_size_thresh0  >> 5));
	    rxFlowCfgRegs[i].RX_FLOW_CONFIG_REG_F = reg;
	    
	    reg = 0;
	    /* Rx flow configuration register G */
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_G_RX_FDQ0_SZ1_QNUM, cfg[i].rx_fdq0_sz1_qnum);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_G_RX_FDQ0_SZ1_QMGR, cfg[i].rx_fdq0_sz1_qnum>>12);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_G_RX_SIZE_THRESH2, (cfg[i].rx_size_thresh2) >> 5);
	    rxFlowCfgRegs[i].RX_FLOW_CONFIG_REG_G = reg;

	    reg = 0;
	    /* Rx flow configuration register H */
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_H_RX_FDQ0_SZ3_QNUM, cfg[i].rx_fdq0_sz3_qnum);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_H_RX_FDQ0_SZ3_QMGR, cfg[i].rx_fdq0_sz3_qnum>>12);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_H_RX_FDQ0_SZ2_QNUM, cfg[i].rx_fdq0_sz2_qnum);
	    CSL_FINS (reg, CPPIDMA_RX_FLOW_CONFIG_RX_FLOW_CONFIG_REG_H_RX_FDQ0_SZ2_QMGR, cfg[i].rx_fdq0_sz2_qnum>>12);
	    rxFlowCfgRegs[i].RX_FLOW_CONFIG_REG_H = reg;
	}
}

