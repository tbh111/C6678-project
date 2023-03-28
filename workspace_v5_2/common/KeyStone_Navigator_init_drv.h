/******************************************************************************

  Copyright (C), 2001-2012, Texas Instrument.

 ******************************************************************************
  File Name     : KeyStone_Navigator_init_drv.h
  Version       : Initial Draft
  Author        : Brighton Feng
  Created       : 2011/4/30
  Last Modified :
  Description   : KeyStone_Navigator_init_drv.c header file Multicore Navigator initialization and driver

  Function List :
  History       :
  1.Date        : 2011/4/30
    Author      : Brighton Feng
    Modification: Created file

******************************************************************************/

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

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/
#ifndef _KEYSTONE_MULTICORE_NAVIGATOR_H_
#define _KEYSTONE_MULTICORE_NAVIGATOR_H_

#include <ti\csl\tistdtypes.h>
#include <ti\csl\cslr_qm_config.h>
#include <ti\csl\cslr_qm_descriptor_region_config.h>
#include <ti\csl\cslr_qm_queue_management.h>
#include <ti\csl\cslr_qm_queue_status_config.h>
#include <ti\csl\cslr_qm_intd.h>
#include <ti\csl\cslr_pdsp.h>
#include <ti\csl\cslr_cppidma_global_config.h>
#include <ti\csl\cslr_cppidma_rx_channel_config.h>
#include <ti\csl\cslr_cppidma_rx_flow_config.h>
#include <ti\csl\cslr_cppidma_tx_channel_config.h>
#include "KeyStone_Packet_Descriptor.h"
#include "KeyStone_common.h"

#define QMSS_PKTDMA_MAX_CH_NUM       32
#define QMSS_PKTDMA_MAX_RX_FLOW_NUM  64
#define SRIO_PKTDMA_MAX_CH_NUM       16
#define SRIO_PKTDMA_MAX_RX_FLOW_NUM  20
#define PA_PKTDMA_MAX_RX_CH_NUM      24
#define PA_PKTDMA_MAX_TX_CH_NUM      9 
#define PA_PKTDMA_MAX_RX_FLOW_NUM    32
#define AIF_PKTDMA_MAX_CH_NUM        129
#define AIF_PKTDMA_MAX_RX_FLOW_NUM   129
#define BCP_PKTDMA_MAX_CH_NUM        8 
#define BCP_PKTDMA_MAX_RX_FLOW_NUM   64
#define FFTC_PKTDMA_MAX_CH_NUM       4
#define FFTC_PKTDMA_MAX_RX_FLOW_NUM  8

typedef enum
{
	PKTDMA_NO_WAIT = 0, 						
	PKTDMA_WAIT_FOREVER= -1
}PacketDMA_Wait;


/** Internal Linking RAM offset */
#define QMSS_LINKING_RAM_OFFSET                     0x80000
/** Internal Linking RAM default size */
#define QMSS_LINKING_RAM_REGION_0_DEFAULT_SIZE      0x3FFF

typedef struct  { 
    volatile Uint32 BASE_ADDRESS_REG;
    volatile Uint32 START_INDEX_REG;
    volatile Uint32 DESCRIPTOR_SETUP_REG;
    volatile Uint8 RSVD0[4];
} QMSS_DescriptorMemoryRegionRegs;

typedef struct 
{
    /** The base address of descriptor region. Note the 
     * descriptor Base address must be specified in ascending memory order
     * */
    Uint32          descBase;
    /** Size of each descriptor in the memory region. Must be a multiple of 16 */
    Uint32          descSize;
    /** Number of descriptors in the memory region. 
     * Must be a minimum of 32. 
     * Must be 2^(5 or greater) 
     * Maximum supported value 2^20
     * */
    Uint32          descNum;
} Qmss_DescMemRegionCfg;

typedef struct  {
    volatile Uint32 REG_A_EntryCount;
    volatile Uint32 REG_B_ByteCount;
    volatile Uint32 REG_C_HeadTail_PacketSize;
    volatile Uint32 REG_D_Descriptor;
} QueueManageRegs;

typedef struct  {
    volatile Uint32 REG_A_EntryCount;
    volatile Uint32 REG_B_ByteCount;
    volatile Uint32 REG_C_PacketSize;
    volatile Uint32 REG_D_Threshold;
} QueueStatusConfigRegs;

typedef enum
{
	FETCH_SIZE_16 = 0, 						
	FETCH_SIZE_32 , 						
	FETCH_SIZE_48 , 						
	FETCH_SIZE_64 , 						
	FETCH_SIZE_80 , 						
	FETCH_SIZE_96 ,						
	FETCH_SIZE_112,						
	FETCH_SIZE_128,						
	FETCH_SIZE_144,						
	FETCH_SIZE_160,						
	FETCH_SIZE_176,						
	FETCH_SIZE_192,						
	FETCH_SIZE_208,						
	FETCH_SIZE_224,						
	FETCH_SIZE_240,						
	FETCH_SIZE_256						
}Initial_Descriptor_Fetch_Size;

/** 
 * @brief location where the packet is queued
 */
typedef enum
{
    /** Queue packet to the tail of the queue. Default behavior. */
    Qmss_Location_TAIL = 0,
    /** Queue packet to the head of the queue. */
    Qmss_Location_HEAD 
}Qmss_Location;

typedef struct 
{
	Uint32 uiFreeQuNum; 		/*free queue number*/
	Uint32 uiDescriptorAddress; /*address of first descriptor*/
	Uint32 uiDescriptorSize; 	/*size of each descriptor*/
	Uint32 uiDescriptorNumber; 	/*number of descriptors*/
	Uint32 uiBufferAddress; 	/*address of first buffer*/
	Uint32 uiBufferSize; 		/*size of each buffer*/
}FreeHostQueueCfg;

typedef struct 
{
	Uint32 uiFreeQuNum; 		/*free queue number*/
	Uint32 uiDescriptorAddress; /*address of first descriptor*/
	Uint32 uiDescriptorSize; 	/*size of each descriptor*/
	Uint32 uiDescriptorNumber; 	/*number of descriptors*/
}FreeMonoQueueCfg;

typedef enum
{
    /** PDSP 1 */
    Qmss_PdspId_PDSP1 = 0,
    /** PDSP 2 */
    Qmss_PdspId_PDSP2
}Qmss_PdspId;

typedef enum
{
    /** Accumulator command to disable channel */
    Qmss_AccCmd_DISABLE_CHANNEL = 0x80,
    /** Accumulator command to enable channel */
    Qmss_AccCmd_ENABLE_CHANNEL = 0x81,
    /** Accumulator command to configure timer constant */
    Qmss_AccCmd_CONFIG_TIMER_CONSTANT = 0x82,
    /** Accumulator command to configure reclamation queue */
    Qmss_AccCmd_CONFIG_RECLAIM_QUEUE = 0x83,
    /** Accumulator command to configure diversion queue */
    Qmss_AccCmd_CONFIG_DIVERSION_QUEUE = 0x84
}Qmss_AccCmdType;

typedef enum
{
    /** Interrupt on entry threshold count only */
    Qmss_AccPacingMode_NONE = 0,
    /** Time delay since last interrupt */
    Qmss_AccPacingMode_LAST_INTERRUPT,
    /** Time delay since first new packet */
    Qmss_AccPacingMode_FIRST_NEW_PACKET,
    /** Time delay since last new packet */
    Qmss_AccPacingMode_LAST_NEW_PACKET
}Qmss_AccPacingMode;

typedef enum
{
    /** 'D' register only (4 byte entries)
     * Word 0 : Packet Descriptor Pointer 
     */
    Qmss_AccEntrySize_REG_D = 0,
    /** 'C,D' registers (8 byte entries) 
     * Word 0 : Packet Length (as reported by queue manager)
     * Word 1 : Packet Descriptor Pointer
     */
    Qmss_AccEntrySize_REG_CD,
    /** 'A,B,C,D' registers (16 byte entries) 
     * Word 0 : Packet Count on Queue (when read)
     * Word 1 : Byte Count on Queue (when read)
     * Word 2 : Packet Length (as reported by queue manager)
     * Word 3 : Packet Descriptor Pointer
     */
    Qmss_AccEntrySize_REG_ABCD
}Qmss_AccEntrySize;

typedef enum
{
    /** NULL Terminate Mode - The last list entry is used to store a NULL pointer 
     * record (NULL terminator) to mark the end of list. In either case there is room for one less 
     * list entry in a page than is actually specified by the host.
     */
    Qmss_AccCountMode_NULL_TERMINATE = 0,
    /** Entry Count Mode - The first list entry is used to store the total list entry 
     * count (not including the length entry). 
     */
    Qmss_AccCountMode_ENTRY_COUNT
}Qmss_AccCountMode;

typedef enum
{
    /** Single Queue Mode - The channel monitors a single queue. */
    Qmss_AccQueueMode_SINGLE_QUEUE = 0,
    /** Multi-Queue Mode - The channel monitors up to 32 queues starting at the supplied base queue index. */
    Qmss_AccQueueMode_MULTI_QUEUE
}Qmss_AccQueueMode;

typedef struct
{
    /** Accumulator channel affected (0-47) */
    Uint8               channel;
    /** Accumulator channel command - Qmss_AccCmd_ENABLE_CHANNEL : Enable channel 
     * Qmss_AccCmd_DISABLE_CHANNEL : Disable channel */
    Qmss_AccCmdType     command;
    /** This field specifies which queues are to be included in the queue group. 
     * Bit 0 corresponds to the base queue index, and bit 31 corresponds to the base 
     * queue index plus 31. For any bit set in this mask, the corresponding queue index 
     * is included in the monitoring function.
     *
     * This field is ignored in single-queue mode.*/
    Uint32              queueEnMask;
    /** Physical pointer to list ping/pong buffer. NULL when channel disabled */
    Uint32              listAddress;
    /** Queue Manager and Queue Number index to monitor. This serves as a base queue index when the 
     * channel in multi-queue mode, and must be a multiple of 32 when multi-queue mode is enabled. */
    Uint16              queMgrIndex;
    /** Max entries per list buffer page */
    Uint16              maxPageEntries;
    /** Number of timer ticks to delay interrupt */
    Uint16              timerLoadCount;
    /** Interrupt pacing mode. Specifies when the interrupt should be trigerred */
    Qmss_AccPacingMode  interruptPacingMode;
    /** List entry size. Specifies the size of each data entry */
    Qmss_AccEntrySize   listEntrySize;
    /** List count Mode. The number of entries in the list */
    Qmss_AccCountMode   listCountMode; 
    /** Queue mode. Moitor single or multiple queues */
    Qmss_AccQueueMode   multiQueueMode;
} Qmss_AccCmdCfg;

typedef struct
{
	CSL_Cppidma_global_configRegs * globalCfgRegs;
	CSL_Cppidma_tx_channel_configRegs * txChCfgRegs;
	CSL_Cppidma_rx_channel_configRegs * rxChCfgRegs;
	volatile Uint32 * txChPriority;
	CSL_Cppidma_rx_flow_configRegs * rxFlowCfgRegs;
}PKT_DMA_Regs;

typedef struct
{
    /** Tx scheduling priority for channelNum */
    Uint8           priority;
    /** Tx Filter Software Info.  This field controls whether or not the DMA controller will pass the 
     * extended packet information fields (if present) from the descriptor to the back end application.
     * 0 - DMA controller will pass extended packet info fields if they are present in the descriptor
     * 1 - DMA controller will filter extended packet info fields
     */
    Uint16             filterEINFO;
    /** Filter Protocol Specific Words. This field controls whether or not the DMA controller will 
     * pass the protocol specific words (if present) from the descriptor to the back end application.
     * 0 - DMA controller will pass PS words if present in descriptor
     * 1 - DMA controller will filter PS words
     */
    Uint16             filterPS;
    /**
     * AIF Specific Monolithic Packet Mode. This field when set indicates that all monolithic packets 
     * which will be transferred on this channel will be formatted in an optimal configuration as needed 
     * by the Antenna Interface Peripheral.  The AIF configuration uses a fixed descriptor format which 
     * includes the 3 mandatory descriptor info words, a single Protocol Specific Word and data 
     * immediately following (data offset = 16).
     */
    Uint16             aifMonoMode;
}PktDma_TxChCfg;

typedef struct 
{
    /** This field indicates the default receive queue that this channel should use */
    Uint16          rx_dest_qnum;
    /** This field specifies the number of bytes that are to be skipped in the SOP buffer before beginning 
     * to write the payload or protocol specific bytes(if they are in the sop buffer).  This value must
     * be less than the minimum size of a buffer in the system */
    Uint16          rx_sop_offset;
    /** This field controls where the Protocol Specific words will be placed in the Host Mode CPPI data structure 
     * 0 - protocol specific information is located in descriptor 
     * 1 - protocol specific information is located in SOP buffer */
    Uint16             rx_ps_location;
    /** This field indicates the descriptor type to use 1 = Host, 2 = Monolithic */
    Uint8           rx_desc_type;
    /** This field controls the error handling mode for the flow and is only used when channel errors occurs 
     * 0 = Starvation errors result in dropping packet and reclaiming any used descriptor or buffer resources 
     * back to the original queues/pools they were allocated to
     * 1 = Starvation errors result in subsequent re-try of the descriptor allocation operation.  
     */
    Uint16             rx_error_handling;
    /** This field controls whether or not the Protocol Specific words will be present in the Rx Packet Descriptor 
     * 0 - The port DMA will set the PS word count to 0 in the PD and will drop any PS words that are presented 
     * from the back end application.
     * 1 - The port DMA will set the PS word count to the value given by the back end application and will copy 
     * the PS words from the back end application to the location 
     */
    Uint16             rx_psinfo_present;
    /** This field controls whether or not the Extended Packet Info Block will be present in the Rx Packet Descriptor.  
     * 0 - The port DMA will clear the Extended Packet Info Block Present bit in the PD and will drop any extended 
     * packet info words that are presented from the back end application. 
     * 1 - The port DMA will set the Extended Packet Info Block Present bit in the PD and will copy any extended packet
     * info words that are presented across the Rx streaming interface into the extended packet info words in the descriptor.
     * If no extended packet info words are presented from the back end application, the port DMA will overwrite the fields with zeroes.
     */
    Uint16             rx_einfo_present;

    /** This bits control whether or not the flow will compare the packet size received from the back end application 
     * against the rx_size_thresh0 fields to determine which FDQ to allocate the SOP buffer from.  
     * The bits in this field is encoded as follows:
     * 0 = Do not use the threshold.
     * 1 = Use the thresholds to select SOP FDQ rx_fdq0_sz0_qnum.
     */

    Uint8             rx_size_thresh0_en;
    /** This bits control whether or not the flow will compare the packet size received from the back end application 
     * against the rx_size_thresh1 fields to determine which FDQ to allocate the SOP buffer from.  
     * The bits in this field is encoded as follows:
     * 0 = Do not use the threshold.
     * 1 = Use the thresholds to select SOP FDQ rx_fdq0_sz1_qnum.
     */
    Uint8             rx_size_thresh1_en;
        /** This bits control whether or not the flow will compare the packet size received from the back end application 
     * against the rx_size_thresh2 fields to determine which FDQ to allocate the SOP buffer from.  
     * The bits in this field is encoded as follows:
     * 0 = Do not use the threshold.
     * 1 = Use the thresholds to select SOP FDQ rx_fdq0_sz2_qnum.
     */
    Uint8             rx_size_thresh2_en;

    /** This value is left shifted by 5 bits and compared against the packet size to determine which free descriptor 
     * queue should be used for the SOP buffer in the packet.  If the packet size is less than or equal to the value 
     * given in this threshold, the DMA controller in the port will allocate the SOP buffer from the queue given by 
     * the rx_fdq0_sz0_qnum fields. This field is optional.
     */
    Uint32          rx_size_thresh0;
    /** This value is left shifted by 5 bits and compared against the packet size to determine which free descriptor 
     * queue should be used for the SOP buffer in the packet.  If the  packet size is greater than the rx_size_thresh0 
     * but is less than or equal to the value given in this threshold, the DMA controller in the port will allocate the 
     * SOP buffer from the queue given by the rx_fdq0_sz1_qnum fields. 
     * If enabled, this value must be greater than the value given in the rx_size_thresh0 field. This field is optional.
     */
    Uint32          rx_size_thresh1;
    /** This value is left shifted by 5 bits and compared against the packet size to determine which free descriptor 
     * queue should be used for the SOP buffer in the packet.  If the  packet size is less than or equal to the value
     * given in this threshold, the DMA controller in the port will allocate the SOP buffer from the queue given by the 
     * rx_fdq0_sz2_qnum fields.
     * If enabled, this value must be greater than the value given in the rx_size_thresh1 field. This field is optional.
     */
    Uint32  		rx_size_thresh2;

    /** This field specifies which Free Descriptor Queue should be used for the 1st Rx buffer in a packet whose 
     * size is less than or equal to the rx_size0 value */
    Uint16          rx_fdq0_sz0_qnum;
    /** This field specifies which Queue should be used for the 1st Rx buffer in a packet whose size is 
     * less than or equal to the rx_size1 value */
    Uint16          rx_fdq0_sz1_qnum;
    /** This field specifies which Free Descriptor Queue should be used for the 1st Rx buffer in a packet 
     * whose size is less than or equal to the rx_size2 value */
    Uint16          rx_fdq0_sz2_qnum;
    /** This field specifies which Free Descriptor Queue should be used for the 1st Rx buffer in a
     * packet whose size is less than or equal to the rx_size3 value */
    Uint16          rx_fdq0_sz3_qnum;

    /** This field specifies which Free Descriptor Queue should be used for the 2nd Rx buffer in a host type packet */
    Uint16          rx_fdq1_qnum;
    /** This field specifies which Free Descriptor Queue should be used for the 3rd Rx buffer in a host type packet */
    Uint16          rx_fdq2_qnum;
    /** This field specifies which Free Descriptor Queue should be used for the 4th or later Rx
     *  buffers in a host type packet */
    Uint16          rx_fdq3_qnum;

    /** This is the value to insert into bits 7:0 of the destination tag if the rx_dest_tag_lo_sel is set to 1 */
    Uint8           rx_dest_tag_lo;
    /** This is the value to insert into bits 15:8 of the destination tag if the rx_dest_tag_hi_sel is set to 1 */
    Uint8           rx_dest_tag_hi;
    /** This is the value to insert into bits 7:0 of the source tag if the rx_src_tag_lo_sel is set to 1 */
    Uint8           rx_src_tag_lo;
    /** This is the value to insert into bits 15:8 of the source tag if the rx_src_tag_hi_sel is set to 1 */
    Uint8           rx_src_tag_hi;    

    /** This field specifies the source for bits 7:0 of the source tag field in word 1 of the output PD.
     * This field is encoded as follows:
     * 0 = do not overwrite
     * 1 = overwrite with value given in rx_dest_tag_lo
     * 2 = overwrite with flow_id[7:0] from back end application
     * 3 = RESERVED
     * 4 = overwrite with dest_tag[7:0] from back end application
     * 5 = overwrite with dest_tag[15:8] from back end application
     * 6-7 = RESERVED
     */    
    Uint8           rx_dest_tag_lo_sel;
    /** This field specifies the source for bits 15:8 of the source tag field in the word 1 of the output PD.
     * This field is encoded as follows:
     * 0 = do not overwrite
     * 1 = overwrite with value given in rx_dest_tag_hi
     * 2 = overwrite with flow_id[7:0] from back end application
     * 3 = RESERVED
     * 4 = overwrite with dest_tag[7:0] from back end application
     * 5 = overwrite with dest_tag[15:8] from back end application
     * 6-7 = RESERVED
     */
    Uint8           rx_dest_tag_hi_sel;
    /** This field specifies the source for bits 7:0 of the source tag field in the output packet descriptor.
     * This field is encoded as follows:
     * 0 = do not overwrite
     * 1 = overwrite with value given in rx_src_tag_lo
     * 2 = overwrite with flow_id[7:0] from back end application
     * 3 = RESERVED
     * 4 = overwrite with src_tag[7:0] from back end application
     * 5 = RESERVED
     * 6-7 = RESERVED
     */
    Uint8           rx_src_tag_lo_sel;
    /** This field specifies the source for bits 15:8 of the source tag field in the output packet descriptor.
     * This field is encoded as follows:
     * 0 = do not overwrite
     * 1 = overwrite with value given in rx_src_tag_hi
     * 2 = overwrite with flow_id[7:0] from back end application
     * 3 = RESERVED
     * 4 = overwrite with src_tag[7:0] from back end application
     * 5 = RESERVED
     * 6-7 = RESERVED
     */
    Uint8           rx_src_tag_hi_sel;    
 
}PktDma_RxFlowCfg;

/*Queue Manager definition*/
extern CSL_Qm_configRegs * gpQM_configRegs;
extern QMSS_DescriptorMemoryRegionRegs * gpQM_descriptorRegions; 

/*queue access registers through VBUSP configration bus*/
extern QueueManageRegs * gpQueueManageRegs;
extern QueueManageRegs * gpQueueManageProxyRegs;

/*queue access data space through VBUSM data bus*/
extern QueueManageRegs * gpQueueManageVBUSM;

extern QueueStatusConfigRegs * gpQueueStatusConfigRegs;
extern volatile Uint32 * gpQueueThresholdStatus;
extern CSL_Qm_intdRegs * gpQM_INTD_regs;

/*PDSP definition*/
extern volatile Uint32 * gpQM_PDSP_IRAM[];
extern CSL_PdspRegs * gpQM_PDSP_CtrlRegs[];
extern volatile Uint32 * gpQM_PDSP_Cmd[];

/*PacketDMA definition*/
extern CSL_Cppidma_global_configRegs * gpQM_DMA_CfgRegs;
extern CSL_Cppidma_rx_channel_configRegs * gpQM_DMA_RxChCfgRegs;
extern CSL_Cppidma_rx_flow_configRegs * gpQM_DMA_RxFlowCfgRegs;
extern CSL_Cppidma_tx_channel_configRegs * gpQM_DMA_TxChCfgRegs;
extern volatile Uint32 * gpQM_DMA_TxChPriority;
extern PKT_DMA_Regs gpQM_PKTDMA_regs;

extern CSL_Cppidma_global_configRegs * gpSRIO_DMA_CfgRegs;
extern CSL_Cppidma_rx_channel_configRegs * gpSRIO_DMA_RxChCfgRegs;
extern CSL_Cppidma_rx_flow_configRegs * gpSRIO_DMA_RxFlowCfgRegs;
extern CSL_Cppidma_tx_channel_configRegs * gpSRIO_DMA_TxChCfgRegs;
extern volatile Uint32 * gpSRIO_DMA_TxChPriority;
extern PKT_DMA_Regs gpSRIO_PKTDMA_regs;

extern CSL_Cppidma_global_configRegs * gpNetCP_DMA_CfgRegs;
extern CSL_Cppidma_rx_channel_configRegs * gpNetCP_DMA_RxChCfgRegs;
extern CSL_Cppidma_rx_flow_configRegs * gpNetCP_DMA_RxFlowCfgRegs;
extern CSL_Cppidma_tx_channel_configRegs * gpNetCP_DMA_TxChCfgRegs;
extern volatile Uint32 * gpNetCP_DMA_TxChPriority;
extern PKT_DMA_Regs gpNetCP_PKTDMA_regs;
/*use QMSS internal linking RAM for region 0
if linkingRAM1!=NULL, use linkingRAM1 for linking RAM region 1*/
extern void KeyStone_QMSS_Linking_RAM_init(unsigned long long * linkingRAM1,
	Uint32 linkingRMA1NumEntry);

/*initialize descriptor regions according to "descMemRegionCfg",
"uiNumDescRegion" is the number of regions to be initialized*/
extern void KeyStone_QMSS_Descriptor_Regions_init(
	Qmss_DescMemRegionCfg * descMemRegionCfg, Uint32 uiNumDescRegion);

/*initialize host descriptors and queues according to "hostQuCfg",
"uiQuCfgNumber" is number of Free Descriptor queues to be initalized*/
extern void KeyStone_Host_Descriptor_Queues_init(
	FreeHostQueueCfg *hostQuCfg, Uint32 uiQuCfgNumber);

/*initialize monolithic descriptors and queues according to "monoQuCfg",
"uiQuCfgNumber" is number of Free Descriptor queues to be initalized*/
extern void KeyStone_Mono_Descriptor_Queues_init(
	FreeMonoQueueCfg *monoQuCfg, Uint32 uiQuCfgNumber);

/*Copy descriptors from one FDQ to a new FDQ
normally used for create a small FDQ from a large FDQ pool*/
extern void KeyStone_Copy_FDQ(Uint32 dstFDQ, Uint32 srcFDQ, Uint32 numDesc);

extern void KeyStone_pktDma_configureRxFlow (
	CSL_Cppidma_rx_flow_configRx_flow_configRegs * rxFlowCfgRegs,	
	PktDma_RxFlowCfg * cfg, Uint32 uiFlowNum);

/*Downloads the PDSP firmware to PDSP. */
extern void KeyStone_Qmss_Download_Firmware (
	Qmss_PdspId pdspId, void *image, Uint32 size);

extern Uint32 KeyStone_Qmss_Config_Acc_Channel (
	Qmss_PdspId pdspId, Qmss_AccCmdCfg *cfg);
extern Uint32 KeyStone_Qmss_disable_Acc_Channel (
	Qmss_PdspId pdspId, Uint8 channel);
extern Uint32 KeyStone_Qmss_config_Acc_Timer (
	Qmss_PdspId pdspId, Uint16 timerConstant);
extern Uint32 KeyStone_Qmss_Config_Reclaim_Queue (
	Qmss_PdspId pdspId, Uint32 uiQueueNumber);

extern void KeyStone_pktDma_Global_Control (
	CSL_Cppidma_global_configRegs * pktDmaCfgRegs,
	Uint32 uiStarvationWaitCycles, Uint32 rxPriority, Uint32 txPriority);

extern void KeyStone_pktDma_TxCh_config(PKT_DMA_Regs * pktDmaRegs, Uint32 uiChNum,
	PktDma_TxChCfg * pktDmaTxChCfg);

static inline void KeyStone_pktDma_TxCh_enable(
	CSL_Cppidma_tx_channel_configRegs * pktDmaTxChCfgRegs, Uint32 uiChNum)
{
	pktDmaTxChCfgRegs->TX_CHANNEL_GLOBAL_CONFIG[uiChNum].TX_CHANNEL_GLOBAL_CONFIG_REG_A |= 
		(CSL_CPPIDMA_TX_CHANNEL_CONFIG_TX_CHANNEL_GLOBAL_CONFIG_REG_A_TX_ENABLE_MASK);
}

static inline void KeyStone_pktDma_TxCh_disable(
	CSL_Cppidma_tx_channel_configRegs * pktDmaTxChCfgRegs, Uint32 uiChNum)
{
	pktDmaTxChCfgRegs->TX_CHANNEL_GLOBAL_CONFIG[uiChNum].TX_CHANNEL_GLOBAL_CONFIG_REG_A &= 
		(~CSL_CPPIDMA_TX_CHANNEL_CONFIG_TX_CHANNEL_GLOBAL_CONFIG_REG_A_TX_ENABLE_MASK);
}

static inline void KeyStone_pktDma_TxCh_pause(
	CSL_Cppidma_tx_channel_configRegs * pktDmaTxChCfgRegs, Uint32 uiChNum)
{
	pktDmaTxChCfgRegs->TX_CHANNEL_GLOBAL_CONFIG[uiChNum].TX_CHANNEL_GLOBAL_CONFIG_REG_A |= 
		(CSL_CPPIDMA_TX_CHANNEL_CONFIG_TX_CHANNEL_GLOBAL_CONFIG_REG_A_TX_PAUSE_MASK);
}

/*return 1 when fail, return 0 when complete without error,
TSC must be enabled if need wait completion, the wait time is in cycles*/
static inline int KeyStone_pktDma_TxCh_teardown(
	CSL_Cppidma_tx_channel_configRegs * pktDmaTxChCfgRegs, 
	Uint32 uiChNum, PacketDMA_Wait wait)
{
	Uint32 startTSC= TSCL;
	
	pktDmaTxChCfgRegs->TX_CHANNEL_GLOBAL_CONFIG[uiChNum].TX_CHANNEL_GLOBAL_CONFIG_REG_A |= 
		(CSL_CPPIDMA_TX_CHANNEL_CONFIG_TX_CHANNEL_GLOBAL_CONFIG_REG_A_TX_TEARDOWN_MASK);

	if(PKTDMA_NO_WAIT==wait)
		return 0;

	/*wait the teardown is complete by examining the Rx/Tx Channel Global
	Configuration Register to check the enable status.*/
	while(pktDmaTxChCfgRegs->TX_CHANNEL_GLOBAL_CONFIG[uiChNum].TX_CHANNEL_GLOBAL_CONFIG_REG_A & 
		(CSL_CPPIDMA_TX_CHANNEL_CONFIG_TX_CHANNEL_GLOBAL_CONFIG_REG_A_TX_ENABLE_MASK))
	{
		if(PKTDMA_WAIT_FOREVER==wait)
			continue;

		if(TSC_getDelay(startTSC)>wait)
			return 1;
	}

	return 0;
}

static inline void KeyStone_pktDma_RxCh_enable(
	CSL_Cppidma_rx_channel_configRegs * pktDmaRxChCfgRegs, Uint32 uiChNum)
{
	pktDmaRxChCfgRegs->RX_CHANNEL_GLOBAL_CONFIG[uiChNum].RX_CHANNEL_GLOBAL_CONFIG_REG |= 
		(CSL_CPPIDMA_RX_CHANNEL_CONFIG_RX_CHANNEL_GLOBAL_CONFIG_REG_RX_ENABLE_MASK);
}

static inline void KeyStone_pktDma_RxCh_disable(
	CSL_Cppidma_rx_channel_configRegs * pktDmaRxChCfgRegs, Uint32 uiChNum)
{
	pktDmaRxChCfgRegs->RX_CHANNEL_GLOBAL_CONFIG[uiChNum].RX_CHANNEL_GLOBAL_CONFIG_REG &= 
		(~CSL_CPPIDMA_RX_CHANNEL_CONFIG_RX_CHANNEL_GLOBAL_CONFIG_REG_RX_ENABLE_MASK);
}

static inline void KeyStone_pktDma_RxCh_pause(
	CSL_Cppidma_rx_channel_configRegs * pktDmaRxChCfgRegs, Uint32 uiChNum)
{
	pktDmaRxChCfgRegs->RX_CHANNEL_GLOBAL_CONFIG[uiChNum].RX_CHANNEL_GLOBAL_CONFIG_REG |= 
		(CSL_CPPIDMA_RX_CHANNEL_CONFIG_RX_CHANNEL_GLOBAL_CONFIG_REG_RX_PAUSE_MASK);
}

/*return 1 when fail, return 0 when complete without error,
TSC must be enabled if need wait completion, the wait time is in cycles*/
static inline int KeyStone_pktDma_RxCh_teardown(
	CSL_Cppidma_rx_channel_configRegs * pktDmaRxChCfgRegs, 
	Uint32 uiChNum, PacketDMA_Wait wait)
{
	Uint32 startTSC= TSCL;
	
	pktDmaRxChCfgRegs->RX_CHANNEL_GLOBAL_CONFIG[uiChNum].RX_CHANNEL_GLOBAL_CONFIG_REG |= 
		(CSL_CPPIDMA_RX_CHANNEL_CONFIG_RX_CHANNEL_GLOBAL_CONFIG_REG_RX_TEARDOWN_MASK);

	if(PKTDMA_NO_WAIT==wait)
		return 0;

	/*wait the teardown is complete by examining the Rx/Tx Channel Global
	Configuration Register to check the enable status.*/
	while(pktDmaRxChCfgRegs->RX_CHANNEL_GLOBAL_CONFIG[uiChNum].RX_CHANNEL_GLOBAL_CONFIG_REG & 
		(CSL_CPPIDMA_RX_CHANNEL_CONFIG_RX_CHANNEL_GLOBAL_CONFIG_REG_RX_ENABLE_MASK))
	{
		if(PKTDMA_WAIT_FOREVER==wait)
			continue;

		if(TSC_getDelay(startTSC)>wait)
			return 1;
	}

	return 0;
}

/**
 *  @b Description
 *  @n  
 *      This function pushes a descriptor onto a queue specified by the queue number. 
 *      The "descSize" is used to specify the size of the descriptor being pushed.
 *      The optional parameter "packetSize" is used specify the size of packet during pop 
 *      operation. 
 *      The optional parameter "location" is used to override the default(tail) and push the packet 
 *      to the head of the queue.
 *
 *      **No validation is done on the input parameters**.
 *
 *  @param[in]  descAddr
 *      Memory address of the descriptor. Should be a global address.
 * 
 *  @param[in]  packetSize
 *      Size of packet pointed to by the descriptor.
 * 
 *  @param[in]  descSize
 *      Size of the descriptor. Minimum size is 16 bytes. Maximum size is 256 bytes
 * 
 *  @param[in]  location
 *      0 - Tail.
 *      1 - Head
 *
 */
static inline void KeyStone_queuePush_Size_Loc (Uint32 queueNum, 
	Uint32 descAddr,Uint32 packetSize, Uint32 descSize, Qmss_Location location)
{
    Uint32            regc = 0, regd = 0;
    unsigned long long            dWord = 0;
    volatile unsigned long long   *regCregDPtr;
 
    CSL_FINS (regc, QM_QUEUE_MANAGEMENT_QUEUE_REG_C_HEAD_TAIL, location);
    
    CSL_FINS (regc, QM_QUEUE_MANAGEMENT_QUEUE_REG_C_PACKET_SIZE, packetSize);
   
    regd = (descAddr | ((descSize >> 4) - 1));

#ifdef _BIG_ENDIAN
    dWord = _itoll (regc, regd);
#else
    dWord = _itoll (regd, regc);
#endif

    regCregDPtr = (volatile unsigned long long *) 
    	(& gpQueueManageVBUSM[queueNum].REG_C_HeadTail_PacketSize);
    *regCregDPtr = dWord;
    
    return;
}

/**
 *  @b Description
 *  @n  
 *      It pushes a descriptor onto a queue specified by the queue number. Does not allow
 *      specifying optional parameters. The descriptor size is not written to the queue. This 
 *      function should be used to push descriptors that will not be prefetched by the CPDMA.
 *
 *  @param[in]  descAddr
 *      Memory address of the descriptor. Should be a global address.
 * 
 *  @pre  
 *      Qmss_queueOpen function should be called before calling this function.
 *
 *  @retval
 *      None
 */
static inline void KeyStone_queuePush (Uint32 queueNum, Uint32 descAddr)
{
    gpQueueManageVBUSM[queueNum].REG_D_Descriptor= descAddr;
}

/**
 *  @b Description
 *  @n  
 *      It pushes a descriptor onto a queue specified by the queue number. Does not allow
 *      specifying optional parameters.
 *
 *      The "descSize" is used to specify the size of the descriptor being pushed. This 
 *      function should be used to push descriptors that will be prefetched by the CPDMA.
 *
 *      **No validation is done on the input parameters**.
 *
 *  @param[in]  descAddr
 *      Memory address of the descriptor. Should be a global address.
 * 
 *  @param[in]  descSize
 *      Size of the descriptor. Minimum size is 16 bytes. Maximum size is 256 bytes
 */
static inline void KeyStone_queuePush_DescSize (Uint32 queueNum, 
	Uint32 descAddr, Uint32 descSize)
{
    gpQueueManageVBUSM[queueNum].REG_D_Descriptor = ((Uint32) descAddr | ((descSize >> 4) - 1));
}

/**
 *  @b Description
 *  @n  
 *      This function pop's a descriptor off the queue specified by the queue number.
 *
 *      The lower 4 bits of the descriptor address contain the size of the descriptor 
 *      that was specified during the queue push operation. Which is used by PktDMA,
 *      it is masked out since this function is called by DSP core.
 *
 *      **No validation is done on the input parameters**
  */
static inline Uint32 KeyStone_queuePop (Uint32 queueNum)
{
    return (gpQueueManageRegs[queueNum].REG_D_Descriptor&0xFFFFFFF0);
}

/**
 *  @b Description
 *  @n  
 *      This function pop's a descriptor off the queue specified by the queue number.
 *      It also returns the packet size of the popped decriptor. The packet size is 
 *      available only if it was specified during the push operation. 
 *
 *      **This function is not multicore/multithread safe.**
 *
 *      It is possible that the descriptor is popped by another core/task between the time taken 
 *      to read the packet size and the descriptor address by the first core/task. 
 *      
 *       The caller should provide appropriate locks.
 *
 *      The packet size field is part of the descriptor and should be used to ensure correctness.
 *
 *      The lower 4 bits of the descriptor address contain the size of the descriptor 
 *      that was specified during the queue push operation.
 *      Caller should mask the lower order 4 bits before using the descriptor.
 *
 *      **No validation is done on the input parameters**
 *      
 *  @param[out]  descAddr
 *      Descriptor address and size.
 *
 *  @param[out]  packetSize
 *      Packet size in bytes.
 */
static inline Uint32 KeyStone_queuePop_Size (Uint32 queueNum, Uint32 *packetSize)
{
    *packetSize = gpQueueStatusConfigRegs[queueNum].REG_C_PacketSize&
    	CSL_QM_QUEUE_STATUS_CONFIG_QUEUE_STATUS_CONFIG_REG_C_PACKET_SIZE_MASK;
    return gpQueueManageRegs[queueNum].REG_D_Descriptor;
}

//get the number of entries in a queue
static inline Uint32 KeyStone_GetQueueEntryCount (Uint32 queueNum)
{
	return gpQueueStatusConfigRegs[queueNum].REG_A_EntryCount;
}

//Return and descriptor to its return queue
static inline void KeyStone_ReturnQueue (Uint32 descAddr)
{
	MonolithicPacketDescriptor * Descriptor= 
		(MonolithicPacketDescriptor *)descAddr;

	Uint32 uiReturnQ= (Descriptor->pkt_return_qmgr<<12)|
		Descriptor->pkt_return_qnum;
		
	KeyStone_queuePush(uiReturnQ, descAddr);
}

//empty the specified Queue by writing Reg_D
static inline void KeyStone_queueEmpty (Uint32 queueNum)
{
	gpQueueManageRegs[queueNum].REG_D_Descriptor = (Uint32) 0x0;
    return;
}
#endif
