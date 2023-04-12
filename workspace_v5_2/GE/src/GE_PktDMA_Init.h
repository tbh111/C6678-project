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
#ifndef _GE_PKTDMA_INIT_H_
#define _GE_PKTDMA_INIT_H_

#include "KeyStone_Navigator_init_drv.h"

//MAX accumulation page size
#define MAX_ACCU_PAGE_SIZE 	64

/*size of descriptor must be multiple of 16B*/
#define GE_DESCRIPTOR_SIZE 		32

/*the linking RAM entry number for each descriptor region 
must be power of 2, and no less than 32.
Actual descriptor number may less than linking entry size, this may 
leave hole in linking RAM*/
#define SL2_HOST_DESCRIPTOR_NUM 		(96)
#define SL2_HOST_DESC_LINK_ENTRY_NUM 	(128)
#define LL2_HOST_DESCRIPTOR_NUM 		(96)
#define LL2_HOST_DESC_LINK_ENTRY_NUM 	(128)
#define DDR_HOST_DESCRIPTOR_NUM 		(768)
#define DDR_HOST_DESC_LINK_ENTRY_NUM 	(1024)

#define SL2_PACKET_BUFFER_SIZE0 	(1536)
#define SL2_PACKET_BUFFER_SIZE1 	(10*1024)
#define SL2_SIZE0_PKT_BUF_NUM 		(64)
#define SL2_SIZE1_PKT_BUF_NUM 		(32)
#define SL2_SIZE1_DESC_START_IDX 	(SL2_SIZE0_PKT_BUF_NUM)

#define LL2_PACKET_BUFFER_SIZE0 	(1536)
#define LL2_PACKET_BUFFER_SIZE1 	(10*1024)
#define LL2_SIZE0_PKT_BUF_NUM 		(32)
#define LL2_SIZE1_PKT_BUF_NUM 		(16)
#define LL2_SIZE1_DESC_START_IDX 	(LL2_SIZE0_PKT_BUF_NUM)

#define DDR_PACKET_BUFFER_SIZE0 	(1536)
#define DDR_PACKET_BUFFER_SIZE1 	(10*1024)
#define DDR_SIZE0_PKT_BUF_NUM 		(512)
#define DDR_SIZE1_PKT_BUF_NUM 		(256)
#define DDR_SIZE1_DESC_START_IDX 	(DDR_SIZE0_PKT_BUF_NUM)

#define TEST_QUEUE_BASE 			2048

#define SL2_HOST_SIZE0_FDQ          (TEST_QUEUE_BASE+0 )
#define SL2_HOST_SIZE1_FDQ          (TEST_QUEUE_BASE+1 )
#define CORE0_LL2_HOST_SIZE0_FDQ    (TEST_QUEUE_BASE+2 )
#define CORE0_LL2_HOST_SIZE1_FDQ    (TEST_QUEUE_BASE+3 )
#define CORE1_LL2_HOST_SIZE0_FDQ    (TEST_QUEUE_BASE+4 )
#define CORE1_LL2_HOST_SIZE1_FDQ    (TEST_QUEUE_BASE+5 )
#define CORE2_LL2_HOST_SIZE0_FDQ    (TEST_QUEUE_BASE+6 )
#define CORE2_LL2_HOST_SIZE1_FDQ    (TEST_QUEUE_BASE+7 )
#define CORE3_LL2_HOST_SIZE0_FDQ    (TEST_QUEUE_BASE+8 )
#define CORE3_LL2_HOST_SIZE1_FDQ    (TEST_QUEUE_BASE+9 )
#define DDR_HOST_SIZE0_FDQ          (TEST_QUEUE_BASE+10)
#define DDR_HOST_SIZE1_FDQ          (TEST_QUEUE_BASE+11)
#define RECLAMATION_QUEUE           (TEST_QUEUE_BASE+12)

/*Low priority accumulation will be used for these queue,
so, their index start at 32 boundary*/
#define GE_RX_DST_Q         (TEST_QUEUE_BASE+32)

#define GE_RX_FLOW_DDR 				0
#define GE_RX_FLOW_SL2 				1
#define GE_RX_FLOW_CORE0_LL2 		2
#define GE_RX_FLOW_CORE1_LL2 		3

/*Descriptor accumulation buffer*/
extern Uint32 uiaDescriptorAccumulationList[];
extern Uint32 uiAccPingPong; 	
extern Uint32 uiAccPageSize;

extern Uint32 uiUsedDescRegionNum;
extern Uint32 uiInitialHostFdqNum;
extern Uint32 uiUsedRxFlowNum;

extern Uint8 packetBuffer_SL2_Size0[SL2_SIZE0_PKT_BUF_NUM][SL2_PACKET_BUFFER_SIZE0];
extern Uint8 packetBuffer_SL2_Size1[SL2_SIZE1_PKT_BUF_NUM][SL2_PACKET_BUFFER_SIZE1];

extern Uint8 packetBuffer_LL2_Size0[LL2_SIZE0_PKT_BUF_NUM][LL2_PACKET_BUFFER_SIZE0];
extern Uint8 packetBuffer_LL2_Size1[LL2_SIZE1_PKT_BUF_NUM][LL2_PACKET_BUFFER_SIZE1];

extern Uint8 packetBuffer_DDR_Size0[DDR_SIZE0_PKT_BUF_NUM][DDR_PACKET_BUFFER_SIZE0];
extern Uint8 packetBuffer_DDR_Size1[DDR_SIZE1_PKT_BUF_NUM][DDR_PACKET_BUFFER_SIZE1];

extern char * FDQ_str[];
extern char * flow_str[];

/*must be called after GE PSC enabled*/
extern void GE_PktDMA_init();
extern void GE_QMSS_Accumulation_config(Uint8 channel, Uint16 pageSize,
	Qmss_AccPacingMode  interruptPacingMode);

/*check the descriptors in the queue, for debug purpose*/
extern void GE_Check_Free_Queues();
extern void GE_Check_TxRx_Queues();
extern void GE_Check_Queue(Uint32 uiQIdx, Uint32 uiExpectEntryCount);

#endif
