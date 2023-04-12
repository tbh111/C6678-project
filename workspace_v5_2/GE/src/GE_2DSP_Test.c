/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2013
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
* ============================================================================
 Examples to show the usage of GE on KeyStone DSP
 these tests showe GE transfer between 2 DSPs or between DSP and PC
* =============================================================================
 *  Revision History
 *  ===============
 *  July 4, 2013 Kevin Cai   File Created
 * ============================================================================
 */
#include <stdio.h>
#include <string.h>
#include <csl_qm_queue.h>
#include "KeyStone_common.h"
#include "KeyStone_GE_Init_drv.h"
#include "GE_test.h"
#include "GE_debug.h"
#include "GE_PktDMA_Init.h"
#include "GE_2DSP_test.h"

/*delay some time between packets in case the other side does not 
support flow control*/
#define DELAY_BETWEEN_PACKET_MS 	1

/*Fllowing table specifies the packet transfered on 2 ports,
number of packets of each port should be less than 64 for this test*/
GE_2DSP_Transfer_Param test_2DSP_cfg[GE_NUM_ETHERNET_PORT]=
{
    /*payloadNumBytes, dataPattern, numPackets*/
    {1500,              0x55,        2},  /*SGMII port0*/
    {46,                0x01,        32}  /*SGMII port1*/
};

Uint32 Get_TX_Good_Frames()
{
	Uint32 uiTxGoodFrames;

	uiTxGoodFrames= gpStats_regs[GE_STATSB]->TXGOODFRAMES;

	return uiTxGoodFrames;
}

Uint32 TxDescriptorTempBuffer[DDR_SIZE1_PKT_BUF_NUM+DDR_SIZE0_PKT_BUF_NUM];
void GE_2DSP_Test()
{
	int i, j;
	GE_2DSP_Transfer_Param * transferParam;
	Uint32 uiFDQ, uiPayloadNumBytes;
	Uint32 uiTotalBytes, uiTotalNumPackets, uiCycles, uiTxGoodFrames;
	Uint8 * ucpBuffer;
	HostPacketDescriptor * hostDescriptor;

	uiTotalBytes= 0;
	uiTotalNumPackets= 0;
	for(j= 0; j< GE_NUM_ETHERNET_PORT; j++)
	{
		if(FALSE==Port_OK(j))
			continue;

		transferParam= &test_2DSP_cfg[j];
		uiPayloadNumBytes= transferParam->payloadNumBytes;
		if((uiPayloadNumBytes<46)||(uiPayloadNumBytes>9216))
		{
			printf("invalid packet payload size %d for port %d\n", uiPayloadNumBytes, j+1);
			continue;
		}

		if(uiPayloadNumBytes>DDR_PACKET_BUFFER_SIZE0)
			uiFDQ= DDR_HOST_SIZE1_FDQ;
		else
			uiFDQ= DDR_HOST_SIZE0_FDQ;

		for(i=0; i<transferParam->numPackets; i++)
		{
			hostDescriptor= (HostPacketDescriptor *)KeyStone_queuePop(uiFDQ);
			if(NULL==hostDescriptor)
			{
				printf("Source queue %d is NULL\n", uiFDQ);
				GE_Check_Free_Queues(); 	//for debug
				break;
			}
		
			/*invalid cache before read descriptor RAM*/
			InvalidCache((void *)hostDescriptor, 64);

			/*Directed packet to port. Setting these bits to a non-zero value 
			indicates that the packet is a directed packet. Packets with the 
			these bits set will bypass the ALE and send the packet directly 
			to the port indicated.*/
			hostDescriptor->ps_flags= j+1;

			/*initialize the source buffer*/
			ucpBuffer= (Uint8 *)hostDescriptor->buffer_ptr;

			/*fill MAC header*/
			Fill_EMAC_header(ucpBuffer, ETHERNET_IPV4_PACKET, Source_MAC_address[j],
				Dest_MAC_address[j]);

			/*fill data pattern*/
			memset(ucpBuffer+EMAC_HEADER_LEN, transferParam->dataPattern, uiPayloadNumBytes);

			hostDescriptor->packet_length= uiPayloadNumBytes+ EMAC_HEADER_LEN;

			/*write back data from cache to descriptor RAM*/
			WritebackCache((void *)hostDescriptor, 64);
			WritebackCache((void *)ucpBuffer, uiPayloadNumBytes+EMAC_HEADER_LEN);

			//save descriptors to temp buffer
			TxDescriptorTempBuffer[uiTotalNumPackets]= (Uint32)hostDescriptor;

			uiTotalBytes += uiPayloadNumBytes;
			uiTotalNumPackets++;
		}
	}

	//TSC_delay_ms(10000);
	uiTxGoodFrames= Get_TX_Good_Frames();
	
	uiCycles= TSCL;
	for(i=0; i< uiTotalNumPackets; i++)
	{
		/*push the packet descriptor to Packet DMA TX queue*/
		KeyStone_queuePush(GE_DIRECT_TX_QUEUE,
			TxDescriptorTempBuffer[i]|FETCH_SIZE_64);

		//delay to avoid potentail overflow, for debug only
		TSC_delay_ms(DELAY_BETWEEN_PACKET_MS);
	}

	/*wait all packets have been send out successfully*/
	while((Get_TX_Good_Frames()-uiTxGoodFrames)<uiTotalNumPackets)
	{
		asm(" NOP 5");
#if 1 	//for debug
		if(TSC_count_cycle_from(uiCycles)>0x3FFFFFFF) 	//timeout
		{
			puts("waiting for transfer complete timeout!");
			print_GE_status_error(); 	//for debug
			while(1); 	//trap
		}
#endif
	}

	uiCycles= TSC_count_cycle_from(uiCycles);
	printf("Transferred %8d bytes with %8d cycles, throughput= %4dMbps.\n", 
		uiTotalBytes, uiCycles, 
		(unsigned long long)uiTotalBytes*8*gDSP_Core_Speed_Hz/uiCycles/1000000);

}

