/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2011
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
* ============================================================================
*Example to show the usage and performance of SRIO on KeyStone DSP.
*This example can run on C6678 EVM with digital loopback or Serdes loopback.
*On dual Nyquist EVM, external loopback or transfer between two DSPs 
 can be tested. You should run this project on second DSP firstly and then the 
 first DSP
* =============================================================================
 *  Revision History
 *  ===============
 *  June 13, 2011 Brighton Feng   File Created
 * ============================================================================
 */
#include <stdio.h>
#include <string.h>
#include "KeyStone_SRIO_init_drv.h"
#include "KeyStone_DDR_init.h"
#include "SRIO_test.h"
#include "SRIO_Interrupts.h"
#include "SRIO_PktDMA_init.h"
#include "SRIO_internal_loopback_test.h"
#include "SRIO_external_line_loopback_test.h"
#include "SRIO_external_forward_back_test.h"
#include "SRIO_2DSP_test.h"

SRIO_Loopback_Mode loopback_mode= SRIO_SERDES_LOOPBACK;

/*up to 16 deviceID can be setup here*/
SRIO_Device_ID_Routing_Config dsp0_device_ID_routing_config[]=
{
     /*idPattern 	idMatchMask 	routeMaintenance*/
	{DSP0_SRIO_BASE_ID+0, 	0xFFFF, 	1},
	{DSP0_SRIO_BASE_ID+1, 	0xFFFF, 	1},
	{DSP0_SRIO_BASE_ID+2, 	0xFFFF, 	1},
	{DSP0_SRIO_BASE_ID+3, 	0xFFFF, 	1},
	{DSP0_SRIO_BASE_ID+4, 	0xFFFF, 	1},
	{DSP0_SRIO_BASE_ID+5, 	0xFFFF, 	1},
	{DSP0_SRIO_BASE_ID+6, 	0xFFFF, 	1},
	{DSP0_SRIO_BASE_ID+7, 	0xFFFF, 	1},
};

/*up to 16 deviceID can be setup here*/
SRIO_Device_ID_Routing_Config dsp1_device_ID_routing_config[]=
{
     /*idPattern 	idMatchMask 	routeMaintenance*/
	{DSP1_SRIO_BASE_ID+0, 	0xFFFF, 	1},
	{DSP1_SRIO_BASE_ID+1, 	0xFFFF, 	1},
	{DSP1_SRIO_BASE_ID+2, 	0xFFFF, 	1},
	{DSP1_SRIO_BASE_ID+3, 	0xFFFF, 	1},
	{DSP1_SRIO_BASE_ID+4, 	0xFFFF, 	1},
	{DSP1_SRIO_BASE_ID+5, 	0xFFFF, 	1},
	{DSP1_SRIO_BASE_ID+6, 	0xFFFF, 	1},
	{DSP1_SRIO_BASE_ID+7, 	0xFFFF, 	1},
};

/*map RX message to flow or queue according to specific fields in the message.
In this test, we only use destination device ID to determine the RX flow ID, 
destination queue is configured in flow conifgurations*/
/*up to 64 map entries can be setup here*/
SRIO_RX_Message_Map DSP0_message_map[]=
{
    /*flowId*/              /*destQuID*/    /*dstId*/                               /*dstProm*/ /*srcId*/ /*srcProm*/ /*tt*/ /*mbx*/ /*mbxMask*/ /*ltr*/ /*ltrMask*/ /*segMap*/ /*cos*/ /*cosMask*/ /*streamId*/ /*streamMask*/         
    {SRIO_RX_FLOW_CORE0_LL2,0x1FFF,         DSP0_SRIO_BASE_ID+SRIO_RX_FLOW_CORE0_LL2,   0,          0,      1,          0,      0,      0,          0,      0,          0,          0,      0,          0,          0}, /*single segement*/
    {SRIO_RX_FLOW_CORE0_LL2,0x1FFF,         DSP0_SRIO_BASE_ID+SRIO_RX_FLOW_CORE0_LL2,   0,          0,      1,          0,      0,      0,          0,      0,          1,          0,      0,          0,          0}, /*multiple segements*/
    {SRIO_RX_FLOW_CORE1_LL2,0x1FFF,         DSP0_SRIO_BASE_ID+SRIO_RX_FLOW_CORE1_LL2,   0,          0,      1,          0,      0,      0,          0,      0,          0,          0,      0,          0,          0}, /*single segement*/
    {SRIO_RX_FLOW_CORE1_LL2,0x1FFF,         DSP0_SRIO_BASE_ID+SRIO_RX_FLOW_CORE1_LL2,   0,          0,      1,          0,      0,      0,          0,      0,          1,          0,      0,          0,          0}, /*multiple segements*/
    {SRIO_RX_FLOW_CORE2_LL2,0x1FFF,         DSP0_SRIO_BASE_ID+SRIO_RX_FLOW_CORE2_LL2,   0,          0,      1,          0,      0,      0,          0,      0,          0,          0,      0,          0,          0}, /*single segement*/
    {SRIO_RX_FLOW_CORE2_LL2,0x1FFF,         DSP0_SRIO_BASE_ID+SRIO_RX_FLOW_CORE2_LL2,   0,          0,      1,          0,      0,      0,          0,      0,          1,          0,      0,          0,          0}, /*multiple segements*/
    {SRIO_RX_FLOW_CORE3_LL2,0x1FFF,         DSP0_SRIO_BASE_ID+SRIO_RX_FLOW_CORE3_LL2,   0,          0,      1,          0,      0,      0,          0,      0,          0,          0,      0,          0,          0}, /*single segement*/
    {SRIO_RX_FLOW_CORE3_LL2,0x1FFF,         DSP0_SRIO_BASE_ID+SRIO_RX_FLOW_CORE3_LL2,   0,          0,      1,          0,      0,      0,          0,      0,          1,          0,      0,          0,          0}, /*multiple segements*/
    {SRIO_RX_FLOW_SL2,      0x1FFF,         DSP0_SRIO_BASE_ID+SRIO_RX_FLOW_SL2,         0,          0,      1,          0,      0,      0,          0,      0,          0,          0,      0,          0,          0}, /*single segement*/
    {SRIO_RX_FLOW_SL2,      0x1FFF,         DSP0_SRIO_BASE_ID+SRIO_RX_FLOW_SL2,         0,          0,      1,          0,      0,      0,          0,      0,          1,          0,      0,          0,          0}, /*multiple segements*/
    {SRIO_RX_FLOW_DDR,      0x1FFF,         DSP0_SRIO_BASE_ID+SRIO_RX_FLOW_DDR,         0,          0,      1,          0,      0,      0,          0,      0,          0,          0,      0,          0,          0}, /*single segement*/
    {SRIO_RX_FLOW_DDR,      0x1FFF,         DSP0_SRIO_BASE_ID+SRIO_RX_FLOW_DDR,         0,          0,      1,          0,      0,      0,          0,      0,          1,          0,      0,          0,          0}  /*multiple segements*/
};

/*up to 64 map entries can be setup here*/
SRIO_RX_Message_Map DSP1_message_map[]=
{
    /*flowId*/              /*destQuID*/    /*dstId*/                               /*dstProm*/ /*srcId*/ /*srcProm*/ /*tt*/ /*mbx*/ /*mbxMask*/ /*ltr*/ /*ltrMask*/ /*segMap*/ /*cos*/ /*cosMask*/ /*streamId*/ /*streamMask*/         
    {SRIO_RX_FLOW_CORE0_LL2,0x1FFF,         DSP1_SRIO_BASE_ID+SRIO_RX_FLOW_CORE0_LL2,   0,          0,      1,          0,      0,      0,          0,      0,          0,          0,      0,          0,          0}, /*single segement*/
    {SRIO_RX_FLOW_CORE0_LL2,0x1FFF,         DSP1_SRIO_BASE_ID+SRIO_RX_FLOW_CORE0_LL2,   0,          0,      1,          0,      0,      0,          0,      0,          1,          0,      0,          0,          0}, /*multiple segements*/
    {SRIO_RX_FLOW_CORE1_LL2,0x1FFF,         DSP1_SRIO_BASE_ID+SRIO_RX_FLOW_CORE1_LL2,   0,          0,      1,          0,      0,      0,          0,      0,          0,          0,      0,          0,          0}, /*single segement*/
    {SRIO_RX_FLOW_CORE1_LL2,0x1FFF,         DSP1_SRIO_BASE_ID+SRIO_RX_FLOW_CORE1_LL2,   0,          0,      1,          0,      0,      0,          0,      0,          1,          0,      0,          0,          0}, /*multiple segements*/
    {SRIO_RX_FLOW_CORE2_LL2,0x1FFF,         DSP1_SRIO_BASE_ID+SRIO_RX_FLOW_CORE2_LL2,   0,          0,      1,          0,      0,      0,          0,      0,          0,          0,      0,          0,          0}, /*single segement*/
    {SRIO_RX_FLOW_CORE2_LL2,0x1FFF,         DSP1_SRIO_BASE_ID+SRIO_RX_FLOW_CORE2_LL2,   0,          0,      1,          0,      0,      0,          0,      0,          1,          0,      0,          0,          0}, /*multiple segements*/
    {SRIO_RX_FLOW_CORE3_LL2,0x1FFF,         DSP1_SRIO_BASE_ID+SRIO_RX_FLOW_CORE3_LL2,   0,          0,      1,          0,      0,      0,          0,      0,          0,          0,      0,          0,          0}, /*single segement*/
    {SRIO_RX_FLOW_CORE3_LL2,0x1FFF,         DSP1_SRIO_BASE_ID+SRIO_RX_FLOW_CORE3_LL2,   0,          0,      1,          0,      0,      0,          0,      0,          1,          0,      0,          0,          0}, /*multiple segements*/
    {SRIO_RX_FLOW_SL2,      0x1FFF,         DSP1_SRIO_BASE_ID+SRIO_RX_FLOW_SL2,         0,          0,      1,          0,      0,      0,          0,      0,          0,          0,      0,          0,          0}, /*single segement*/
    {SRIO_RX_FLOW_SL2,      0x1FFF,         DSP1_SRIO_BASE_ID+SRIO_RX_FLOW_SL2,         0,          0,      1,          0,      0,      0,          0,      0,          1,          0,      0,          0,          0}, /*multiple segements*/
    {SRIO_RX_FLOW_DDR,      0x1FFF,         DSP1_SRIO_BASE_ID+SRIO_RX_FLOW_DDR,         0,          0,      1,          0,      0,      0,          0,      0,          0,          0,      0,          0,          0}, /*single segement*/
    {SRIO_RX_FLOW_DDR,      0x1FFF,         DSP1_SRIO_BASE_ID+SRIO_RX_FLOW_DDR,         0,          0,      1,          0,      0,      0,          0,      0,          1,          0,      0,          0,          0}  /*multiple segements*/
};

/*up to 16 TX queues can be setup here*/
SRIO_TX_Queue_Sch_Info TX_Queue_Sch_Info[]=
{
    /*outputPort*/  /*priority*/    /*CRF*/
    {0,             0,              0},
    {1,             0,              0},
    {2,             0,              0},
    {3,             0,              0},
    {0,             0,              1},
    {1,             0,              1},
    {2,             0,              1},
    {3,             0,              1},
    {0,             1,              0},
    {1,             1,              0},
    {2,             1,              0},
    {3,             1,              0},
    {0,             1,              1},
    {1,             1,              1},
    {2,             1,              1},
    {3,             1,              1}
};

/*For SRIO_EXTERNAL_FORWARD_BACK test, DSP1 is setup to forwarding back all
packets with ID of DSP0*/
/*up to 8 entries can be setup here*/
SRIO_PktForwarding_Cfg DSP1_PktForwarding_Cfg[]=
{
    /*forwarding ID 8 up*/  /* forwarding ID 8 lo */   /*forwarding ID 16 up*/  /*forwarding ID 16 lo*/    /*outport */
    {DSP0_SRIO_BASE_ID+0,   DSP0_SRIO_BASE_ID+1,         DSP0_SRIO_BASE_ID+0,     DSP0_SRIO_BASE_ID+1,         2},
    {DSP0_SRIO_BASE_ID+2,   DSP0_SRIO_BASE_ID+2,         DSP0_SRIO_BASE_ID+2,     DSP0_SRIO_BASE_ID+2,         2},
    {DSP0_SRIO_BASE_ID+3,   DSP0_SRIO_BASE_ID+3,         DSP0_SRIO_BASE_ID+3,     DSP0_SRIO_BASE_ID+3,         3},
    {DSP0_SRIO_BASE_ID+4,   DSP0_SRIO_BASE_ID+7,         DSP0_SRIO_BASE_ID+4,     DSP0_SRIO_BASE_ID+7,         3}
};

SRIO_RX_Mode rxMode;

SerdesSetup_4links serdes_cfg;
SerdesLinkSetup serdesLinkSetup;
SRIO_Message_Cfg msg_cfg;
SRIO_Config srio_cfg;

/*path configuration strings*/
char * path_str[]=
{
	"1xLaneA                        ", 
	"1xLaneA_1xLaneB                ",
	"2xLaneAB                       ",
	"1xLaneA_1xLaneB_1xLaneC_1xLaneD",  
	"2xLaneAB_1xLaneC_1xLaneD       ",
	"1xLaneA_1xLaneB_2xLaneCD       ",
	"2xLaneAB_2xLaneCD              ",
	"4xLaneABCD                     "
};
char * get_path_string(SRIO_1x2x4x_Path_Control srio_1x2x4x_path_control)
{
	if(SRIO_PATH_CTL_1xLaneA                             == srio_1x2x4x_path_control) return path_str[0];
	else if(SRIO_PATH_CTL_1xLaneA_1xLaneB                == srio_1x2x4x_path_control) return path_str[1];
	else if(SRIO_PATH_CTL_2xLaneAB                       == srio_1x2x4x_path_control) return path_str[2];
	else if(SRIO_PATH_CTL_1xLaneA_1xLaneB_1xLaneC_1xLaneD== srio_1x2x4x_path_control) return path_str[3]; 
	else if(SRIO_PATH_CTL_2xLaneAB_1xLaneC_1xLaneD       == srio_1x2x4x_path_control) return path_str[4];
	else if(SRIO_PATH_CTL_1xLaneA_1xLaneB_2xLaneCD       == srio_1x2x4x_path_control) return path_str[5];
	else if(SRIO_PATH_CTL_2xLaneAB_2xLaneCD              == srio_1x2x4x_path_control) return path_str[6];
	else if(SRIO_PATH_CTL_4xLaneABCD                     == srio_1x2x4x_path_control) return path_str[7];
	else return NULL;
}

/* identify used ports lanes according to 1x 2x 4x path configuration*/
void srio_identify_used_ports_lanes(
	SRIO_1x2x4x_Path_Control srio_1x2x4x_path_control)
{
	int i;
	Uint32 uiPathConfig, uiPathMode;
	Uint32 uiLaneEnableMask=0, uiLogicPortEnableMask=0;

	/* SRIO 1x 2x 4x path mode configuration:
	In a configuration 1, the following operating mode is available:
	 Mode 0: One independent 1x port in lane A

	In Configuration 2, a maximum of 2 active ports and 2 lanes per path are supported as follows:
	 Mode 0: Two independent 1x ports in lanes A and B
	 Mode 1: One independent 2x port in lanes {A, B}

	In Configuration 4, a maximum of 4 active ports and 4 lanes per path are supported as follows:
	 Mode 0: Four independent 1x ports in lanes A, B, C, and D
	 Mode 1: One independent 2x port in lanes {A,B}, and two independent 1x ports in lanes C and D
	 Mode 2: Two independent 1x ports in lanes A and B, and one independent 2x port in lanes {C,D}
	 Mode 3: Two independent 2x ports, occupying lanes {A,B} and {C,D} respectively
	 Mode 4: One independent 4x port in lanes {A,B,C,D}*/
	uiPathConfig= (srio_1x2x4x_path_control&
		CSL_SRIO_RIO_PLM_SP_PATH_CTL_PATH_CONFIGURATION_MASK)>>
		CSL_SRIO_RIO_PLM_SP_PATH_CTL_PATH_CONFIGURATION_SHIFT;

	uiPathMode= (srio_1x2x4x_path_control&
		CSL_SRIO_RIO_PLM_SP_PATH_CTL_PATH_MODE_MASK)>>
		CSL_SRIO_RIO_PLM_SP_PATH_CTL_PATH_MODE_SHIFT;

	if(1==uiPathConfig)
	{
		uiLaneEnableMask= 0x1; 	/*0001*/

		uiLogicPortEnableMask= 0x1; 	/*0001*/
	}
	else if(2==uiPathConfig)
	{
		uiLaneEnableMask= 0x3; 	/*0011*/

		if(0==uiPathMode)
			uiLogicPortEnableMask= 0x3; 	/*0011*/
		else if(1==uiPathMode)
			uiLogicPortEnableMask= 0x1; 	/*0001*/
	}
	else if(4==uiPathConfig)
	{
		uiLaneEnableMask= 0xF; 	/*1111*/

		if(0==uiPathMode)
			uiLogicPortEnableMask= 0xF; 	/*1111*/
		else if(1==uiPathMode)
			uiLogicPortEnableMask= 0xD; 	/*1101*/
		else if(2==uiPathMode)
			uiLogicPortEnableMask= 0x7; 	/*0111*/
		else if(3==uiPathMode)
			uiLogicPortEnableMask= 0x5; 	/*0101*/
		else if(4==uiPathMode)
			uiLogicPortEnableMask= 0x1; 	/*0001*/
	}

	/*enable ports data path according to 1x 2x 4x path configuration*/
	srio_cfg.blockEn.bBLK5_8_Port_Datapath_EN[0]= uiLaneEnableMask&1;
	srio_cfg.blockEn.bBLK5_8_Port_Datapath_EN[1]= (uiLaneEnableMask>>1)&1;
	srio_cfg.blockEn.bBLK5_8_Port_Datapath_EN[2]= (uiLaneEnableMask>>2)&1;
	srio_cfg.blockEn.bBLK5_8_Port_Datapath_EN[3]= (uiLaneEnableMask>>3)&1;

	/*disable Serdes according to 1x 2x 4x path configuration*/
	for(i= 0; i<4; i++ )
	{
		if(uiLaneEnableMask&(1<<i))
			srio_cfg.serdes_cfg->linkSetup[i]= &serdesLinkSetup;
		else
			srio_cfg.serdes_cfg->linkSetup[i]= NULL;
	}

	/*enable loggical ports according to 1x 2x 4x path configuration*/
	srio_cfg.blockEn.bLogic_Port_EN[0]= uiLogicPortEnableMask&1;
	srio_cfg.blockEn.bLogic_Port_EN[1]= (uiLogicPortEnableMask>>1)&1;
	srio_cfg.blockEn.bLogic_Port_EN[2]= (uiLogicPortEnableMask>>2)&1;
	srio_cfg.blockEn.bLogic_Port_EN[3]= (uiLogicPortEnableMask>>3)&1;

	printf("SRIO path configuration %s\n", 
		get_path_string(srio_1x2x4x_path_control));

}

/*packet type strings*/
char * type_str[]=
{
	"NREAD   " ,
	"NWRITE  " ,
	"NWRITE_R" ,
	"SWRITE  " ,
	"STREAM  " ,
	"MESSAGE "
};
char * get_packet_type_string(SRIO_Packet_Type packetType)
{
	if(SRIO_PKT_TYPE_NREAD         == packetType) return type_str[0];
	else if(SRIO_PKT_TYPE_NWRITE   == packetType) return type_str[1];
	else if(SRIO_PKT_TYPE_NWRITE_R == packetType) return type_str[2];
	else if(SRIO_PKT_TYPE_SWRITE   == packetType) return type_str[3];
	else if(SRIO_PKT_TYPE9_STREAM  == packetType) return type_str[4];
	else if(SRIO_PKT_TYPE11_MESSAGE== packetType) return type_str[5];
	else return NULL;
}

Uint32 uiTestID= 1;
/*fill data before test*/
void InitDataBuffer(Uint32 * uipSrc, Uint32 * uipDst, Uint32 uiByteCount)
{
	int i;
	
	if(uipSrc)
	{
		for(i= 0; i< uiByteCount/4; i++)
		{
			uipSrc[i] = uiTestID;
		}

        WritebackCache((void *)uipSrc, uiByteCount);
        
		uiTestID++;
	}

	if(uipDst)
	{
		for(i= 0; i< uiByteCount/4; i++)
		{
			uipDst[i] = 0xFFFFFFFF;
		}
        WritebackCache((void *)uipDst, uiByteCount);
	}
}

/*verify data after test*/
void VerifyData(Uint32 * uipSrc, Uint32 * uipDst, Uint32 uiByteCount)
{
	int i;

    InvalidCache((void *)uipSrc, uiByteCount);
    InvalidCache((void *)uipDst, uiByteCount);
    
	CSL_XMC_invalidatePrefetchBuffer();

	for(i= 0; i< uiByteCount/4; i++)
	{
		if(uipSrc[i]!= uipDst[i])
		{
			printf("data mismatch at unit %d, 0x%x (at 0x%x) != 0x%x (at 0x%x)\n", 
				i, uipSrc[i], uipSrc, uipDst[i], uipDst);
			return;
		}
	}
}

void InitLsuTransfer(SRIO_LSU_Transfer * lsuTransfer, 
	SRIO_Transfer_Param * transferParam, Uint32 uiLsuNum, 
	Uint32 uiDstID, Uint32 uiSrcIDMap)
{
    lsuTransfer->rioAddressMSB=0;
	/*swap source/dest for READ*/
	if(SRIO_PKT_TYPE_NREAD==transferParam->packet_type)
	{
	    lsuTransfer->rioAddressLSB_ConfigOffset= transferParam->source;
	    lsuTransfer->localDspAddress= transferParam->dest;
	}
	else
	{
	    lsuTransfer->rioAddressLSB_ConfigOffset= transferParam->dest;
	    lsuTransfer->localDspAddress= transferParam->source;
	}
    lsuTransfer->bytecount= transferParam->byteCount; 	
    lsuTransfer->packetType= transferParam->packet_type;
    lsuTransfer->dstID= uiDstID;
    lsuTransfer->doorbellInfo= 0;
    lsuTransfer->waitLsuReady= TRUE;
    lsuTransfer->lsuNum= uiLsuNum;
    lsuTransfer->doorbellValid = 0;
    lsuTransfer->intrRequest = 0;
    lsuTransfer->supGoodInt = 0;
    lsuTransfer->priority = 0;
    lsuTransfer->outPortID = uiLsuNum;
    lsuTransfer->idSize = 0;
    lsuTransfer->srcIDMap = uiSrcIDMap;
    lsuTransfer->hopCount = 0;

}

volatile Uint32 uiDoorbell_TSC= 0; 	//timestamp when doorbell interrupt happens
void srio_doorbell_latency_test(Uint32 uiPortNum)
{
	Uint32 cycles;

	cycles= TSCL;	
	KeyStone_SRIO_DoorBell(uiPortNum, uiPortNum, 
		DSP0_SRIO_BASE_ID+uiPortNum, SRIO_DOORBELL_MSG_LATENCY_TEST);

	if(0==uiDoorbell_TSC)
		asm(" IDLE"); //waiting for doorbell interupt
	cycles= ((unsigned int)((0xFFFFFFFFl+uiDoorbell_TSC)- 
		(unsigned long long)cycles)+ 1);

	printf("doorbell latency is %4d cycles\n\n", cycles);
}

/*test mode strings*/
char * test_mode_str[]=
{
	"SRIO test between two DSPs",
	"SRIO_DIGITAL_LOOPBACK test",
	"SRIO_SERDES_LOOPBACK test",
	"SRIO_EXTERNAL_LINE_LOOPBACK test",
	"SRIO_EXTERNAL_FORWARD_BACK test"
};

void main()
{
	int i;
	Uint32 uiDspNum;

	/*enable TSC, memory protection interrupts, EDC for internal RAM;
    clear cache; protect L1 as cache*/
	KeyStone_common_CPU_init();
	/*print device information. 
	Enable memory protection interrupts, EDC for MSMC RAM*/
	KeyStone_common_device_init();

	//enable exception handling
	KeyStone_Exception_cfg(TRUE);

#if 1 	/*for debug only. Without these code you need reset DSP before your rerun the program*/
	//disable SRIO through PSC
	KeyStone_disable_PSC_module(CSL_PSC_PD_SRIO, CSL_PSC_LPSC_SRIO);
	KeyStone_disable_PSC_Power_Domain(CSL_PSC_PD_SRIO);

#endif

	printf("%s start............................................\n", test_mode_str[loopback_mode]);

    CACHE_setL1PSize(CACHE_L1_32KCACHE);
    CACHE_setL1DSize(CACHE_L1_32KCACHE);
    CACHE_setL2Size(CACHE_256KCACHE);

	/*make other cores local memory cacheable and prefetchable*/
	for(i=16; i<24; i++)
		gpCGEM_regs->MAR[i]=1|(1<<CSL_CGEM_MAR0_PFX_SHIFT);	

	/*make other space non-cacheable and non-prefetchable*/
	for(i=24; i<128; i++)
		gpCGEM_regs->MAR[i]=0;	

	/*make DDR cacheable and prefetchable*/
	for(i=128; i<256; i++)
		gpCGEM_regs->MAR[i]=1|(1<<CSL_CGEM_MAR0_PFX_SHIFT);	

	if(C6678_EVM==gDSP_board_type)
	{
		//DSP core speed: 100*10/1=1000MHz
		KeyStone_main_PLL_init(100, 10, 1); 

		serdes_cfg.commonSetup.inputRefClock_MHz = 312.5;
	}
	else if(TCI6614_EVM==gDSP_board_type
		||DUAL_NYQUIST_EVM==gDSP_board_type
		||C6670_EVM==gDSP_board_type)
	{
		//DSP core speed: 122.88*236/29= 999.9889655MHz
		KeyStone_main_PLL_init(122.88, 236, 29);

		if(C6670_EVM==gDSP_board_type)
			serdes_cfg.commonSetup.inputRefClock_MHz = 250;
		else if(DUAL_NYQUIST_EVM==gDSP_board_type)
			serdes_cfg.commonSetup.inputRefClock_MHz = 156.25;
		else
		{
			serdes_cfg.commonSetup.inputRefClock_MHz = 312.5;
		}
	}
	else
	{
		puts("Unknown DSP board type!");
		return;
	}

	//DDR init 66.66667*20/1= 1333
	KeyStone_DDR_init (66.66667, 20, 1, NULL);

	/*clear configuration structrue to make sure unused field is 0*/
	memset(&srio_cfg, 0, sizeof(srio_cfg)); 
	
	srio_cfg.blockEn.bBLK1_LSU_EN= 1;
	srio_cfg.blockEn.bBLK2_MAU_EN= 1;
	srio_cfg.blockEn.bBLK3_TXU_EN= 1;
	srio_cfg.blockEn.bBLK4_RXU_EN= 1;

	srio_cfg.loopback_mode= loopback_mode;
	
	srio_cfg.device_ID_routing_config= dsp0_device_ID_routing_config;
	srio_cfg.uiNumDeviceId= 
		sizeof(dsp0_device_ID_routing_config)/sizeof(SRIO_Device_ID_Routing_Config);

	serdes_cfg.commonSetup.loopBandwidth= SERDES_PLL_LOOP_BAND_MID;
	srio_cfg.serdes_cfg= &serdes_cfg;

	serdesLinkSetup.txOutputSwing    = 15; /*0~15 represents between 100 and 850 mVdfpp  */
	serdesLinkSetup.testPattern      = SERDES_TEST_DISABLED; 
	serdesLinkSetup.rxAlign          = SERDES_RX_COMMA_ALIGNMENT_ENABLE; 
	serdesLinkSetup.rxInvertPolarity = SERDES_RX_NORMAL_POLARITY; 
	serdesLinkSetup.rxTermination    = SERDES_RX_TERM_COMMON_POINT_AC_COUPLE; 
	serdesLinkSetup.rxEqualizerConfig= SERDES_RX_EQ_ADAPTIVE; 
    serdesLinkSetup.rxCDR            = 5;
	serdesLinkSetup.txInvertPolarity = SERDES_TX_NORMAL_POLARITY; 

	msg_cfg.message_map = DSP0_message_map;
	msg_cfg.uiNumMessageMap= 
		sizeof(DSP0_message_map)/sizeof(SRIO_RX_Message_Map);
	msg_cfg.TX_Queue_Sch_Info= TX_Queue_Sch_Info;
	msg_cfg.uiNumTxQueue = 
		sizeof(TX_Queue_Sch_Info)/sizeof(SRIO_TX_Queue_Sch_Info);
	msg_cfg.rx_size_error_garbage_Q        = SRIO_RX_SIZE_ERROR_GARBAGE_Q       ;
	msg_cfg.rx_timeout_garbage_Q           = SRIO_RX_TIMEOUT_GARBAGE_Q          ;
	msg_cfg.tx_excessive_retries_garbage_Q = SRIO_TX_EXCESSIVE_RETRIES_GARBAGE_Q;
	msg_cfg.tx_error_garbage_Q             = SRIO_TX_ERROR_GARBAGE_Q            ;
	msg_cfg.tx_size_error_garbage_Q        = SRIO_TX_SIZE_ERROR_GARBAGE_Q       ;
	msg_cfg.datastreaming_cfg= NULL; 	/*use default values*/
	srio_cfg.msg_cfg= &msg_cfg;

	SRIO_Interrupts_Init();

	uiDspNum= KeyStone_Get_DSP_Number();
	if(SRIO_DIGITAL_LOOPBACK==loopback_mode
		||SRIO_SERDES_LOOPBACK==loopback_mode) //internal loopback test
	{
		srio_signle_port_internal_loopback_test(loopback_mode);
		srio_multiple_ports_internal_loopback_test();
		srio_doorbell_latency_test(0);
	}
	else if(SRIO_EXTERNAL_LINE_LOOPBACK==loopback_mode)
	{
		/*For external loopback, DSP0 sends packet to DSP1, and DSP1 sends
		back to DSP0. So, DSP0 runs as normal, DSP1 runs in line loopback mode*/
		if(0==uiDspNum)
		{
			srio_cfg.loopback_mode= SRIO_NO_LOOPBACK;
			srio_signle_port_external_line_loopback_test(loopback_mode);
			srio_multiple_ports_external_line_loopback_test();
			srio_doorbell_latency_test(2);
		}
		else
		{
			srio_cfg.device_ID_routing_config= dsp1_device_ID_routing_config;
			srio_cfg.uiNumDeviceId= 
				sizeof(dsp1_device_ID_routing_config)/
				sizeof(SRIO_Device_ID_Routing_Config);

			serdesLinkSetup.linkSpeed_GHz= SRIO_DEFAULT_TEST_SPEED;
	   		srio_cfg.srio_1x2x4x_path_control= SRIO_PATH_CTL_1xLaneA_1xLaneB_1xLaneC_1xLaneD;            
			srio_identify_used_ports_lanes(srio_cfg.srio_1x2x4x_path_control);

			/*on the second DSP, no logical block is need for line loopback, 
			since it is loopback in physical layer*/
			srio_cfg.blockEn.bLogic_Port_EN[0]= FALSE;
			srio_cfg.blockEn.bLogic_Port_EN[1]= FALSE;
			srio_cfg.blockEn.bLogic_Port_EN[2]= FALSE;
			srio_cfg.blockEn.bLogic_Port_EN[3]= FALSE;
			
			KeyStone_SRIO_Init(&srio_cfg);
			
			puts("DSP1 ready for line loopback");
			while(1);
		}
	}
	else if(SRIO_EXTERNAL_FORWARD_BACK==loopback_mode)
	{
		/*For external loopback, DSP0 sends packet to DSP1, and DSP1 sends
		back to DSP0. So, DSP0 runs as normal, DSP1 is configured to forward
		back packets with ID of DSP0*/
		if(0==uiDspNum)
		{
			srio_cfg.loopback_mode= SRIO_NO_LOOPBACK;
			srio_signle_port_external_forward_back_test(loopback_mode);
			srio_multiple_ports_external_forward_back_test();
			srio_doorbell_latency_test(2);
		}
		else
		{
			srio_cfg.device_ID_routing_config= dsp1_device_ID_routing_config;
			srio_cfg.uiNumDeviceId= 
				sizeof(dsp1_device_ID_routing_config)/
				sizeof(SRIO_Device_ID_Routing_Config);

		    srio_cfg.PktForwardingEntry_cfg = DSP1_PktForwarding_Cfg;
		    srio_cfg.uiNumPktForwardingEntry = 
		        sizeof(DSP1_PktForwarding_Cfg)/sizeof(SRIO_PktForwarding_Cfg);

			/*clear configuration structrue to make sure unused field is 0*/
			memset(&rxMode, 0, sizeof(rxMode)); 
			rxMode.port_rx_mode[0].support_multicast_forwarding= TRUE;
			rxMode.port_rx_mode[1].support_multicast_forwarding= TRUE;
			rxMode.port_rx_mode[2].support_multicast_forwarding= TRUE;
			rxMode.port_rx_mode[3].support_multicast_forwarding= TRUE;
			srio_cfg.rxMode = &rxMode;
			
			serdesLinkSetup.linkSpeed_GHz= SRIO_DEFAULT_TEST_SPEED;
	   		srio_cfg.srio_1x2x4x_path_control= SRIO_PATH_CTL_1xLaneA_1xLaneB_1xLaneC_1xLaneD;            
			srio_identify_used_ports_lanes(srio_cfg.srio_1x2x4x_path_control);

			/*On dual Nyquist EVM only lane 2 and 3 are connected between 2 DSPs*/
			srio_cfg.blockEn.bLogic_Port_EN[0]= FALSE;
			srio_cfg.blockEn.bLogic_Port_EN[1]= FALSE;
			
			KeyStone_SRIO_Init(&srio_cfg);
			
			puts("DSP1 ready for packet forwarding");
			while(1);
		}
	}
	else if(SRIO_NO_LOOPBACK==loopback_mode) 	//test between 2 DSPs
	{
		/*DSP0 is the master for test, DSP0 DirectIO or send message to DSP1.
		DSP1 is the slave for test, it monitor the doorbell and message 
		interrupt to detect the received data*/
		if(0==uiDspNum)
		{
			SRIO_2DSP_Test();
		}
		else
		{
			srio_cfg.device_ID_routing_config= dsp1_device_ID_routing_config;
			srio_cfg.uiNumDeviceId= 
				sizeof(dsp1_device_ID_routing_config)/
				sizeof(SRIO_Device_ID_Routing_Config);

			msg_cfg.message_map = DSP1_message_map;
			msg_cfg.uiNumMessageMap= 
				sizeof(DSP1_message_map)/sizeof(SRIO_RX_Message_Map);
			srio_cfg.msg_cfg= &msg_cfg;
            
			serdesLinkSetup.linkSpeed_GHz= SRIO_DEFAULT_TEST_SPEED;
	   		srio_cfg.srio_1x2x4x_path_control= SRIO_PATH_CTL_1xLaneA_1xLaneB_1xLaneC_1xLaneD;            
			srio_identify_used_ports_lanes(srio_cfg.srio_1x2x4x_path_control);

			/*On dual Nyquist EVM only lane 2 and 3 are connected between 2 DSPs*/
			srio_cfg.blockEn.bLogic_Port_EN[0]= FALSE;
			srio_cfg.blockEn.bLogic_Port_EN[1]= FALSE;
			
			KeyStone_SRIO_Init(&srio_cfg);
			SRIO_PktDM_init();

			/*accumulation is only used for the 2nd DSP for two DSPs test*/
			QMSS_Accumulation_config();

			puts("DSP1 ready as slave for test");
			while(1);
		}
	}

	puts("SRIO test complete.\n");
}


