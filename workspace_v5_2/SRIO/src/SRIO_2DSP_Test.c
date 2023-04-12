/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2011
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
* ============================================================================
 Examples to show the usage of SRIO on KeyStone DSP
 these tests showe SRIO transfer between 2 DSPs
* =============================================================================
 *  Revision History
 *  ===============
 *  June 19, 2011 Brighton Feng   File Created
 * ============================================================================
 */
#include <stdio.h>
#include <csl_qm_queue.h>
#include "SRIO_test.h"
#include "SRIO_PktDMA_Init.h"
#include "SRIO_debug.h"

/*Please note, dual Nyquist EVM only connects port 2 and 3 between 2 DSPs.*/
SRIO_Multiple_Test_Config test_2DSP_cfg=
{
    SRIO_PATH_CTL_1xLaneA_1xLaneB_1xLaneC_1xLaneD, /*multiple_port_path*/
    /*packet_type               source                                                   dest                                                size*/
    {{0,                        0,                                                      0,                                                   0},   /*port0 is not availible for this case*/
    {0,                         0,                                                      0,                                                   0},   /*port1 is not availible for this case*/
    {SRIO_PKT_TYPE9_STREAM,     CORE0_LL2_HOST_SIZE1_FDQ,                               DSP1_SRIO_BASE_ID+SRIO_RX_FLOW_CORE1_LL2,            4096},   /*port2*/
    {SRIO_PKT_TYPE_SWRITE,      ((Uint32)&packetBuffer_LL2_Size1[0][0])+0x10000000,     ((Uint32)&packetBuffer_LL2_Size1[1][0])+0x10000000, LL2_PACKET_BUFFER_SIZE1}}    /*port3*/
};

void InitLsuDoorbell(SRIO_LSU_Transfer * lsuTransfer, 
	SRIO_Transfer_Param * transferParam)
{
	lsuTransfer->doorbellValid= 1;
	if(SRIO_PKT_TYPE_NREAD==transferParam->packet_type)
		lsuTransfer->doorbellInfo= SRIO_DOORBELL_MSG_NREAD_DONE;
	else if(SRIO_PKT_TYPE_NWRITE==transferParam->packet_type)
		lsuTransfer->doorbellInfo= SRIO_DOORBELL_MSG_NWRITE_DONE;
	else if(SRIO_PKT_TYPE_NWRITE_R==transferParam->packet_type)
		lsuTransfer->doorbellInfo= SRIO_DOORBELL_MSG_NWRITE_R_DONE;
	else if(SRIO_PKT_TYPE_SWRITE==transferParam->packet_type)
		lsuTransfer->doorbellInfo= SRIO_DOORBELL_MSG_SWRITE_DONE;
	else
		lsuTransfer->doorbellValid= 0;
}

void SRIO_2DSP_Test()
{
	int j;
	SRIO_Transfer_Param * transferParam;
	SRIO_LSU_Transfer lsuTransfer;
	Int32 uiCompletionCode;
	Uint32 cycles;
	Uint32 uiFreeQueueEntryCount;
	Uint32 * uipSrc, * uipDst, uiByteCount, uiDestID;
	SRIO_Packet_Type packet_type;
	HostPacketDescriptor * hostDescriptor;

	serdesLinkSetup.linkSpeed_GHz= SRIO_DEFAULT_TEST_SPEED;
	srio_cfg.srio_1x2x4x_path_control= test_2DSP_cfg.multiple_port_path;
	srio_identify_used_ports_lanes(srio_cfg.srio_1x2x4x_path_control);

	/*On dual Nyquist EVM only lane 2 and 3 are connected between 2 DSPs*/
	srio_cfg.blockEn.bLogic_Port_EN[0]= FALSE;
	srio_cfg.blockEn.bLogic_Port_EN[1]= FALSE;
			
	KeyStone_SRIO_Init(&srio_cfg);
   	SRIO_PktDM_init();

	if(srio_cfg.blockEn.bLogic_Port_EN[2])
		KeyStone_SRIO_match_ACK_ID(2, DSP1_SRIO_BASE_ID, 2);
	if(srio_cfg.blockEn.bLogic_Port_EN[3])
		KeyStone_SRIO_match_ACK_ID(3, DSP1_SRIO_BASE_ID, 3);

	for(j= 2; j< 4; j++)
	{
		transferParam= &test_2DSP_cfg.transfer_param[j];
		uiByteCount= transferParam->byteCount;
		if(0==uiByteCount)
			continue;

		packet_type= transferParam->packet_type;
		if(packet_type<0x90) 	//directIO
		{
			uipSrc = (Uint32 *)transferParam->source;
			uipDst= (Uint32 *)transferParam->dest;

			/*initialize buffer*/
			InitDataBuffer(uipSrc, uipDst, uiByteCount);

			InitLsuTransfer(&lsuTransfer, transferParam, j, 
				DSP1_SRIO_BASE_ID, j);

			/*setup a doorbell after each DirectIO operation
			to notify the other side of SRIO*/
			InitLsuDoorbell(&lsuTransfer, transferParam);

			KeyStone_SRIO_LSU_transfer(&lsuTransfer);

			uiCompletionCode= KeyStone_SRIO_wait_LSU_completion(j,
				lsuTransfer.transactionID, lsuTransfer.contextBit);

			printf("%s from 0x%8x to 0x%8x, %6d bytes, completion code = %d\n", 
				get_packet_type_string(transferParam->packet_type),
				uipSrc, uipDst, uiByteCount,
				uiCompletionCode);
		}
		else 	//message
		{
			uiFreeQueueEntryCount= 
				KeyStone_GetQueueEntryCount(transferParam->source);

			hostDescriptor= (HostPacketDescriptor *)
				KeyStone_queuePop(transferParam->source);
			if(NULL==hostDescriptor)
			{
				printf("Source queue descriptor %d is NULL\n", 
					transferParam->source);
				SRIO_Check_Free_Queues(); 	//for debug
				return;
			}
		
			/*invalid cache before read descriptor RAM*/
			InvalidCache((void *)hostDescriptor, 64);

			/*initialize the source buffer*/
			uipSrc= (Uint32 *)hostDescriptor->buffer_ptr;
			InitDataBuffer(uipSrc, NULL, uiByteCount);

			uiDestID= transferParam->dest;
			if(SRIO_PKT_TYPE9_STREAM==packet_type)
			{
				KeyStone_SRIO_Build_Type9_Msg_Desc(hostDescriptor,
					DSP0_SRIO_BASE_ID, uiDestID, uiByteCount, 0xAAAA, 0x55);
			}
			else if(SRIO_PKT_TYPE11_MESSAGE==packet_type)
			{
				KeyStone_SRIO_Build_Type11_Msg_Desc(hostDescriptor,
					DSP0_SRIO_BASE_ID, uiDestID, uiByteCount, 0, 0);
			}
			else
			{
				printf("Invalid packet type %d\n", packet_type);
				return;
			}

			/*write back data from cache to descriptor RAM*/
			WritebackCache((void *)hostDescriptor, 64);

			cycles= TSCL;
			/*push the packet descriptor to Packet DMA TX queue*/
			KeyStone_queuePush(QMSS_SRIO_QUEUE_BASE+j, 
				(Uint32)hostDescriptor|FETCH_SIZE_64);
			_mfence(); 	//force all memory operation complete
			_mfence(); 	//force all memory operation complete

			/*wait until descriptor return to free queue*/
			while(uiFreeQueueEntryCount!= 
				KeyStone_GetQueueEntryCount(transferParam->source))
			{
				if(TSC_count_cycle_from(cycles)>0x3FFFFFFF) 	//timeout
				{
					printf("timeout for packet at 0x%x\n", (Uint32)uipSrc);
					SRIO_Check_TxRx_Queues(); 	//for debug
					SRIO_Check_Free_Queues(); 	//for debug
					return;
				}
			}
			
			printf("%s from 0x%8x, %6d bytes transfer complete.\n", 
				get_packet_type_string(transferParam->packet_type),
				uipSrc, uiByteCount);
		}
	}

	//print_SRIO_status_error(&srio_cfg.blockEn);
}

