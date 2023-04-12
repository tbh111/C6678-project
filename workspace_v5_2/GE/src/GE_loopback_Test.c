/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2013
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
* ============================================================================
 Examples to show the usage and performance of GE on KeyStone DSP
 these tests are loopback test
* =============================================================================
 *  Revision History
 *  ===============
 *  June 11, 2013 Brighton Feng   File Created
 * ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include "KeyStone_GE_Init_drv.h"
#include "Keystone_common.h"
#include "GE_PktDMA_Init.h"
#include "GE_test.h"
#include "GE_Interrupts.h"
#include "GE_loopback_Test.h"
#include "GE_debug.h"

//for this test, dest flow ID determines ethernet port number, and destination MAC address
Uint32 message_src_dest[][2]=
{
    {DDR_HOST_SIZE1_FDQ,        GE_RX_FLOW_DDR      },  /*DDR->DDR*/
    {DDR_HOST_SIZE1_FDQ,        GE_RX_FLOW_SL2      },  /*DDR->SL2*/
    {SL2_HOST_SIZE1_FDQ,        GE_RX_FLOW_DDR      },  /*SL2->DDR*/
    {SL2_HOST_SIZE1_FDQ,        GE_RX_FLOW_SL2      }   /*SL2->SL2*/
};

/*Please note, total number of packets transfered over multiple port is limited
by FDQ entries number. For this test, please set the total packet number NOT
greater than 16*/
GE_Transfer_Param multiPortTest[GE_NUM_ETHERNET_PORT]=
{
	/*numPackets, sourceQueue, destFlow,    payloadNumBytes*/
	{4,    SL2_HOST_SIZE1_FDQ, GE_RX_FLOW_DDR,       1500},
	{4,    DDR_HOST_SIZE1_FDQ, GE_RX_FLOW_SL2,       46  }
};

Uint8 ucTestID= 1;
/*fill data into the payload the GE packet before test*/
void InitPayload(Uint8 * ucpBuffer, Uint32 uiNumBytes)
{
	if(ucpBuffer)
	{
		memset(ucpBuffer, ucTestID, uiNumBytes);

        WritebackAllCache((void *)ucpBuffer);
        
		ucTestID++;
	}
}

/*verify data packet after test*/
void VerifyPacket(Uint8 * ucpSrc, Uint8 * ucpDst, Uint32 uiNumBytes)
{
	int i;

    WritebackInvalidAllCache((void *)ucpSrc);
    WritebackInvalidAllCache((void *)ucpDst);
    
    CSL_XMC_invalidatePrefetchBuffer();

	for(i= 0; i< uiNumBytes; i++)
	{
		if(ucpSrc[i]!= ucpDst[i])
		{
			printf("data mismatch at unit %d, 0x%x (at 0x%x) != 0x%x (at 0x%x)\n", 
				i, ucpSrc[i], ucpSrc, ucpDst[i], ucpDst);
			return;
		}
	}
}

/*wait for packet RX interrupt, which will update the "GE_INT_TSCL",
check the TSCL against "uiStartCycles" for time out*/
void WaitForRxInterrupt(Uint32 uiStartCycles)
{
	while(0==GE_INT_TSCL) 	//wait for receive interrupt
	{
		asm(" NOP 5");
#if 1 	//for debug
		if(TSC_count_cycle_from(uiStartCycles)>0x3FFFFFFF) 	//timeout
		{
			puts("waiting for GE RX interrupt timeout!");
			print_GE_status_error(); 	//for debug
			while(1); 	//trap
		}
#endif
	}
}	

/*construct a packet to "uiPortNum" according to "transferParam",
return the pointer of the descriptor*/
HostPacketDescriptor * ConstructPacket(GE_Transfer_Param * transferParam)
{
	Uint32 uiPortNum;
	Uint8 * ucpBuffer;
	HostPacketDescriptor * hostDescriptor;
	Uint32 uiPayloadNumBytes = transferParam->payloadNumBytes;

	hostDescriptor= (HostPacketDescriptor *)
		KeyStone_queuePop(transferParam->sourceQueue);
	if(NULL==hostDescriptor)
	{
		printf("Source queue %d descriptor is NULL\n", transferParam->sourceQueue);
		GE_Check_Free_Queues(); 	//for debug
		return NULL;
	}

	//for this test, flow ID determines port number, and destination MAC address
	uiPortNum= transferParam->destFlow; 	
	
	/*invalid cache before read descriptor RAM*/
	InvalidCache((void *)hostDescriptor, 64);

	/*the SRC_TAG_LO field in the Tx descriptor is used as RX flow ID*/
	hostDescriptor->src_tag_lo= 
		(transferParam->destFlow+22);//for PktDMA loopback test

	/*Directed packet to port. Setting these bits to a non-zero value 
	indicates that the packet is a directed packet. Packets with the 
	these bits set will bypass the ALE and send the packet directly 
	to the port indicated.*/
	hostDescriptor->ps_flags= uiPortNum+1;

	/*initialize the source buffer*/
	ucpBuffer= (Uint8 *)hostDescriptor->buffer_ptr;

	/*fill MAC header*/
	Fill_EMAC_header(ucpBuffer, ETHERNET_IPV4_PACKET, 
		Source_MAC_address[uiPortNum], Dest_MAC_address[uiPortNum]);

	InitPayload(ucpBuffer+EMAC_HEADER_LEN, uiPayloadNumBytes);

	hostDescriptor->packet_length= uiPayloadNumBytes+EMAC_HEADER_LEN;
	hostDescriptor->buffer_len   = uiPayloadNumBytes+EMAC_HEADER_LEN;
	hostDescriptor->ret_push_policy= 0 ; //return to queue tail
	
	/*write back data from cache to descriptor RAM*/
	WritebackCache((void *)hostDescriptor, 64);
	WritebackCache((void *)ucpBuffer, uiPayloadNumBytes+EMAC_HEADER_LEN);

	return hostDescriptor;
}

/*send "uiNumPackets", their descriptors are saved in the "descriptorBuffer"*/
void SendPackets(Uint32 uiNumPackets, Uint32 * descriptorBuffer)
{
	int i;
	
	for(i=0; i< uiNumPackets; i++)
	{
		/*push the packet descriptor to Packet DMA TX queue*/
		KeyStone_queuePush(GE_DIRECT_TX_QUEUE,
			descriptorBuffer[i]|FETCH_SIZE_32);
	}
}

/*transfer through single GE port*/
void GE_signle_port_transfer(GE_Transfer_Param * transferParam)
{
	int i;
	Uint32 uiTotalNumPackets, uiCycles;
	Uint8 * ucpSrc, * ucpDst;
	Uint32 uiPayloadNumBytes;
	HostPacketDescriptor * hostDescriptor;
	Uint32 descriptorTempBuffer[32];

	uiPayloadNumBytes = transferParam->payloadNumBytes;

	if((uiPayloadNumBytes<46)||(uiPayloadNumBytes>9216))
	{
		printf("Error: invalid packet payload size %d\n", uiPayloadNumBytes);
		return;
	}

	uiTotalNumPackets=0;
	for(i=0; i<transferParam->numPackets; i++)
	{
		hostDescriptor= ConstructPacket(transferParam);
		if(NULL==hostDescriptor)
			return;

		//record the source buffer pointer for data verfication
		ucpSrc= (Uint8 *)hostDescriptor->buffer_ptr;
		
		//save descriptors to temp buffer
		descriptorTempBuffer[uiTotalNumPackets]= (Uint32)hostDescriptor;

		uiTotalNumPackets++;
		if(uiTotalNumPackets>=32)
		{
			puts("Error: Temp buffer can not save more than 32 packets");
			return;
		}
	}

	/*clear GE interrupt timestamp to 0, 
	it will be updated when receive interrupt happens*/
	GE_INT_TSCL = 0; 	

	uiCycles= TSCL;

	SendPackets(uiTotalNumPackets, descriptorTempBuffer);

	WaitForRxInterrupt(uiCycles);

	uiCycles= GE_INT_TSCL-uiCycles;

	/*check the last packet in the accumulation list*/
	hostDescriptor= (HostPacketDescriptor *)
		(uiaDescriptorAccumulationList[((uiAccPingPong-1)&1)*(uiAccPageSize+1)+uiTotalNumPackets]&0xFFFFFFF0);

	/*invalid cache before read descriptor RAM*/
	InvalidCache((void *)hostDescriptor, 64);

	/*get the destination buffer*/
	ucpDst= (Uint8 *)hostDescriptor->buffer_ptr;

	if((hostDescriptor->packet_length != (uiPayloadNumBytes+EMAC_HEADER_LEN+EMAC_CRC_LEN))
		&&(hostDescriptor->packet_length != (uiPayloadNumBytes+EMAC_HEADER_LEN)))
	{
		printf("received byte count %d != sent byte count %d (or %d)\n",
			hostDescriptor->packet_length, uiPayloadNumBytes+EMAC_HEADER_LEN,
			uiPayloadNumBytes+EMAC_HEADER_LEN+EMAC_CRC_LEN);
	}

	printf("GE transfer from 0x%8x to 0x%8x, %2d packets x %4d bytes, %8d cycles, %5lld Mbps\n", 
		ucpSrc, ucpDst, uiTotalNumPackets, uiPayloadNumBytes, uiCycles,
		(unsigned long long)uiTotalNumPackets*uiPayloadNumBytes*8*gDSP_Core_Speed_Hz/uiCycles/1000000);

	VerifyPacket(ucpSrc, ucpDst, uiPayloadNumBytes+EMAC_HEADER_LEN);

	GE_Check_TxRx_Queues(); 	//for debug
	GE_Check_Free_Queues(); 	//for debug

}

/*test GE transfer through single port*/
void GE_signle_port_loopback_test()
{
	int k, m, n;
	GE_Transfer_Param testParam;
	Uint32 uiSrcDstComb;

	//test with different number of packets
	for(k=1; k<=16; k*=2)
	{
		testParam.numPackets= k;

		//setup packet accumulator for receiving
		GE_QMSS_Accumulation_config(0, k, Qmss_AccPacingMode_NONE);
		
		//test different source/destination combinations
		uiSrcDstComb= sizeof(message_src_dest)/8;
		for(m=0; m< uiSrcDstComb; m++)
		{
			testParam.sourceQueue= message_src_dest[m][0];
			testParam.destFlow= message_src_dest[m][1];
			if(FALSE==Port_OK(testParam.destFlow))
				continue;
			
			//test different packet size
			for(n=64; n<= LL2_PACKET_BUFFER_SIZE1; n*=2)
			{
				testParam.payloadNumBytes = n;                            
				GE_signle_port_transfer(&testParam);
			}
		}

		/*disable accumulation channels*/
		KeyStone_Qmss_disable_Acc_Channel(Qmss_PdspId_PDSP1, 0);

	}
}

/*transfer through multiple GE ports in parallel*/
void GE_multiple_port_transfer()
{
	int i, j;
	GE_Transfer_Param * transferParam;
	Uint32 uiFDQ, uiPayloadNumBytes, uiNumPackets, uiDestFlow;
	Uint32 uiTotalBytes, uiTotalNumPackets, uiCycles;
	HostPacketDescriptor * hostDescriptor;
	Uint32 descriptorTempBuffer[64];

	puts("Multiple ports test...");

	uiTotalBytes= 0;
	uiTotalNumPackets= 0;
	for(j= 0; j< GE_NUM_ETHERNET_PORT; j++)
	{
		transferParam= &multiPortTest[j];
		uiPayloadNumBytes= transferParam->payloadNumBytes;
		if((uiPayloadNumBytes<46)||(uiPayloadNumBytes>9216))
		{
			printf("invalid packet payload size %d\n", uiPayloadNumBytes);
			continue;
		}
		uiFDQ= transferParam->sourceQueue;
		uiNumPackets= transferParam->numPackets;
		uiDestFlow= transferParam->destFlow;
		if(FALSE==Port_OK(uiDestFlow))
			continue;

		printf(" transfer %2d x %4d bytes from %s (%d) to %s (%d)\n",
			uiNumPackets, uiPayloadNumBytes, 
			FDQ_str[uiFDQ-SL2_HOST_SIZE0_FDQ], uiFDQ, 
			flow_str[uiDestFlow], uiDestFlow);

		for(i=0; i<uiNumPackets; i++)
		{
			hostDescriptor= ConstructPacket(transferParam);
			if(NULL==hostDescriptor)
				return;

			//save descriptors to temp buffer
			descriptorTempBuffer[uiTotalNumPackets]= (Uint32)hostDescriptor;

			uiTotalBytes += uiPayloadNumBytes;
			uiTotalNumPackets++;
			if(uiTotalNumPackets>=64)
			{
				puts("Error: Temp buffer can not save more than 64 packets");
				return;
			}
		}
	}

	//setup packet accumulator for receiving
	GE_QMSS_Accumulation_config(0, uiTotalNumPackets, Qmss_AccPacingMode_NONE);
		
	/*clear GE interrupt timestamp to 0, 
	it will be updated when receive interrupt happens*/
	GE_INT_TSCL = 0; 	

	uiCycles= TSCL;

	SendPackets(uiTotalNumPackets, descriptorTempBuffer);

	WaitForRxInterrupt(uiCycles);

	uiCycles= GE_INT_TSCL- uiCycles;
	
	printf("Transferred %8d bytes with %8d cycles, throughput= %5lldMbps.\n", 
		uiTotalBytes, uiCycles, 
		(unsigned long long)uiTotalBytes*8*gDSP_Core_Speed_Hz/uiCycles/1000000);

	/*disable accumulation channels*/
	KeyStone_Qmss_disable_Acc_Channel(Qmss_PdspId_PDSP1, 0);

}

