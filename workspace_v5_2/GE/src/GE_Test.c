/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2013
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
* ============================================================================
Example to show the usage and performance of GE on KeyStone DSP.
VLAN, CPTS, PA, SA are not covered in this example.
This example can run on single DSP with internal loopback.
With two DSPs, external loopback or transfer between two DSPs can be tested. 
  You should run this project on the second core of the second DSP firstly, 
  and then the first core of the first DSP
* =============================================================================
 *  Revision History
 *  ===============
 *  July 3, 2013 Kevin Cai   File Created
 *  August 25, 2013 Brighton Feng, support C6670, C6678 and TCI6614
 * ============================================================================
 */
#include <stdio.h>
#include <string.h>

#include "Keystone_common.h"
#include "Keystone_DDR_Init.h"
#include "KeyStone_GE_Init_drv.h"
#include "KeyStone_Serdes_init.h"
#include "GE_debug.h"
#include "GE_test.h"
#include "GE_Interrupts.h"
#include "GE_PktDMA_init.h"
#include "GE_loopback_Test.h"
#include "GE_2DSP_Test.h"

/*select between internal/external loopback test or test between two DSPs*/
GE_Test_Data_Path test_data_path= GE_TEST_SERDES_LOOPBACK;

/*select between 10/100/1000Mbps or auto negotiation mode*/
Ethernet_Mode ethernet_mode = ETHERNET_1000M_FULLDUPLEX;

//The port connection state for the test
GE_Port_Connection port_connect[GE_NUM_ETHERNET_PORT]=
{
	GE_PORT_NO_CONNECT, //SGMII port 0
	GE_PORT_CABLE_CONNECT //SGMII port 1
};

/*use long long type (8 bytes) for MAC address, but only lower 6 bytes are valid.
Please note the byte order, MAC address byte 5 is in the lowest bits.
Each MAC address corresponding to a Ethernet port*/
unsigned long long Source_MAC_address[GE_NUM_ETHERNET_PORT]=
{
	0x888888000001,
	0x888888000002
};
unsigned long long Dest_MAC_address[GE_NUM_ETHERNET_PORT]=
{
	0x888888000101,
	0x888888000102
};

Ethernet_Port_Config ethernet_port_cfg[GE_NUM_ETHERNET_PORT]; 

Ethernet_ALE_Config ale_cfg;

Ethernet_MDIO_Config mdio_cfg;

SerdesLinkSetup serdesLinkSetup;

KeyStone_GE_Config ge_cfg;

char * GE_mode_str[]=
{
	"auto negotiation (slave)",
	"auto negotiation (master)",
	"10M fullduplex",
	"100M fullduplex",
	"1000M fullduplex",
	"10G fullduplex"
};

/*check if the port is OK during the test*/
Bool Port_OK(Uint32 uiPortNum)
{
	if(GE_PORT_NOT_USED==port_connect[uiPortNum])
	{
		printf("SGMII%d port is not usable for test\n", uiPortNum);
		return FALSE;
	}

	if(GE_TEST_EXTERNAL_FIFO_LOOPBACK<=test_data_path)
	{
		if(GE_PORT_NO_CONNECT==port_connect[uiPortNum])
		{
			printf("Unconnected SGMII%d port can only be used for internal loopback test\n", uiPortNum);
			return FALSE;
		}
		else if(GE_PORT_SGMII_CONNECT==port_connect[uiPortNum])
		{
			if(0==(gpSGMII_regs[uiPortNum]->STATUS_REG
				&CSL_CPSGMII_STATUS_REG_LINK_MASK))
			{
				printf("link SGMII%d is NOT up\n", uiPortNum);
				return FALSE;
			}
		}
		else if(GE_PORT_CABLE_CONNECT==port_connect[uiPortNum])
		{
			if((gpMDIO_regs->CONTROL_REG&CSL_MDIO_CONTROL_REG_ENABLE_MASK)
				&&(0==(gpMDIO_regs->LINK_REG&(1<<uiPortNum))))
			{
				printf("link PHY%d is NOT up\n", uiPortNum);
				return FALSE;
			}
		}
	}	
	
	return TRUE;
}

/*wait for PHY ready for the ports connected through ethernet cable*/
void Wait_PHY_link()
{
	int i;
	Uint32 uiStartTSC;
	
	for(i=0; i<GE_NUM_ETHERNET_PORT; i++)
	{
		if(GE_PORT_CABLE_CONNECT != port_connect[i])
			continue;

		uiStartTSC= TSCL;
		while(0==(gpMDIO_regs->LINK_REG&(1<<i)))
		{
			if(TSC_count_cycle_from(uiStartTSC)>0x3FFFFFFF)
			{
				printf("Wait for port %d PHY link...\n", i);
				uiStartTSC= TSCL;
			}
		}

		/* Clear Interrupt events in MDIO*/
		gpMDIO_regs->LINK_INT_RAW_REG= 1<<i;
	}		
}

/*For all tests, the ALE is setup to receive all packets.
For these tests, transmit will use direct packet (indpendent of ALE).*/
CSL_CPSW_3GF_ALE_UNICASTADDR_ENTRY ALE_entries[GE_NUM_ETHERNET_PORT];

//setup ALE entries for the 4 destination MAC addresses
void ALE_Entries_Init(ALE_Test_Mode ALE_test_mode)
{
	int i;

	if(ALE_BYPASS==ALE_test_mode)
		return;

	for(i=0; i<GE_NUM_ETHERNET_PORT; i++)
	{
		ALE_entries[i].macAddress[0]= (_hill(Dest_MAC_address[i])>>8)&0xFF;
		ALE_entries[i].macAddress[1]= (_hill(Dest_MAC_address[i])>>0)&0xFF;
		ALE_entries[i].macAddress[2]= (_loll(Dest_MAC_address[i])>>24)&0xFF;
		ALE_entries[i].macAddress[3]= (_loll(Dest_MAC_address[i])>>16)&0xFF;
		ALE_entries[i].macAddress[4]= (_loll(Dest_MAC_address[i])>>8)&0xFF;
		ALE_entries[i].macAddress[5]= (_loll(Dest_MAC_address[i])>>0)&0xFF;

		ALE_entries[i].portNumber= 0; //all packect are received to host port (0)
	
		ALE_entries[i].ucastType    = ALE_UCASTTYPE_UCAST_NOAGE;  
		ALE_entries[i].secureEnable = 0;  
		ALE_entries[i].blockEnable  = 0;  
	}

	ale_cfg.unicastEntries= ALE_entries;
	ale_cfg.num_unicastEntries= GE_NUM_ETHERNET_PORT;
	ge_cfg.ale_cfg= &ale_cfg;
}

/*internal Loopback test: DSP0 TX -> DSP0 RX
input parameter: loopback mode of MAC, SGMII or SERDES*/
void GE_internal_loopback_test(Ethernet_Loopback_Mode loopback_mode)
{
	ge_cfg.loopback_mode= loopback_mode;

	/*TX will use direct packet (indpendent of ALE),
	the loopbacked packet will forward to host port by ALE*/
	ALE_Entries_Init(ALE_RECEIVE_ALL);

	KeyStone_GE_Init(&ge_cfg);
	GE_PktDMA_init();
	GE_Interrupts_Init();

	GE_signle_port_loopback_test();
	GE_multiple_port_transfer();

	print_GE_status_error();
}

/*Loopback test between 2 DSPs: DSP0 TX -> DSP1 RX -> DSP1 TX -> DSP0 RX*/
void GE_external_loopback_test()
{
	int i;

	/*for this test use DSP core number as DSP number,
	so, the program should be run on core 0 of DSP0 and core 1 of DSP1*/
	Uint32 uiDspNum= KeyStone_Get_DSP_Number();

	if(0==uiDspNum)
	{
		//disable internal loopback of DSP0
		ge_cfg.loopback_mode= ETHERNET_LOOPBACK_DISABLE;

		/*DSP0 TX will use direct packet (indpendent of ALE).
		the loopbacked packet will forward to host port by ALE.*/
		ALE_Entries_Init(ALE_RECEIVE_ALL);
	}
	else
	{
		/*DSP1 is in FIFO loopback (RX->TX), disable DSP1 RX flow control */
		ge_cfg.loopback_mode= ETHERNET_PORT_FIFO_LOOPBACK;

		for(i=0; i<GE_NUM_ETHERNET_PORT; i++)
		{
			if(GE_PORT_NOT_USED==port_connect[i])
				continue;
			ethernet_port_cfg[i].RX_flow_control_enable= FALSE;
		}
	}
	
	KeyStone_GE_Init(&ge_cfg);
	Wait_PHY_link();
	
	GE_PktDMA_init();
	GE_Interrupts_Init();

	if(0==uiDspNum)
	{
		GE_signle_port_loopback_test();
		GE_multiple_port_transfer();

		print_GE_status_error();
	}
	else
	{
		puts("DSP1 ready for loopback test...");
		while(1);
	}
}

/*test between 2 DSPs: DSP0 -> DSP1 (or PC)*/
void GE_DSP0_to_DSP1_test()
{
	/*for this test use DSP core number as DSP number,
	so, the program should be run on core 0 of DSP0 and core 1 of DSP1*/
	Uint32 uiDspNum= KeyStone_Get_DSP_Number();

	ge_cfg.loopback_mode= ETHERNET_LOOPBACK_DISABLE;

	//setup ALE to receive all packets to host port (0)
	ALE_Entries_Init(ALE_RECEIVE_ALL);

	KeyStone_GE_Init(&ge_cfg);
	Wait_PHY_link();
	
	GE_PktDMA_init();
	GE_Interrupts_Init();

	if(0==uiDspNum)
	{
		//accumulation all potential receive packets
		GE_QMSS_Accumulation_config(0, 1, Qmss_AccPacingMode_LAST_INTERRUPT);
		
		GE_2DSP_Test();

		print_GE_status_error();
	}
	else
	{
		//accumulation is used on the second DSP to receive packets
		GE_QMSS_Accumulation_config(1, 4, Qmss_AccPacingMode_LAST_INTERRUPT);
		
		puts("DSP1 ready as slave for test...");
		while(1);
	}
}

void main()
{
	int i;

	/*for this test use DSP core number as DSP number,
	so, the program should be run on core 0 of DSP0 and core 1 of DSP1*/
	Uint32 uiDspNum= KeyStone_Get_DSP_Number();

	/*enable TSC, memory protection interrupts, EDC for internal RAM;
    clear cache; protect L1 as cache*/
	KeyStone_common_CPU_init();
	/*print device information. 
	Enable memory protection interrupts, EDC for MSMC RAM*/
	KeyStone_common_device_init();

	//enable exception handling
	KeyStone_Exception_cfg(TRUE);

    CACHE_setL1PSize(CACHE_L1_32KCACHE);
    CACHE_setL1DSize(CACHE_L1_32KCACHE);
    CACHE_setL2Size(CACHE_256KCACHE);

	/*make SL2 cacheable*/
	for(i=12; i<16; i++)
		gpCGEM_regs->MAR[i]= 1;

	/*make other cores local memory cacheable and prefetchable*/
	for(i=16; i<24; i++)
		gpCGEM_regs->MAR[i]=1|(1<<CSL_CGEM_MAR0_PFX_SHIFT);

	/*make DDR cacheable and prefetchable*/
	for(i=128; i<256; i++)
		gpCGEM_regs->MAR[i]=1|(1<<CSL_CGEM_MAR0_PFX_SHIFT);

	/*make other space non-cacheable and non-prefetchable*/
	for(i=24; i<128; i++)
		gpCGEM_regs->MAR[i]=0;

	/*clear configuration structrue to make sure unused field is 0 (default value)*/
	memset(ethernet_port_cfg, 0, sizeof(ethernet_port_cfg));
	memset(&ale_cfg, 0, sizeof(ale_cfg));
	memset(&mdio_cfg, 0, sizeof(mdio_cfg));
	memset(&serdesLinkSetup, 0, sizeof(serdesLinkSetup));
	memset(&ge_cfg, 0, sizeof(ge_cfg));

	if(C6678_EVM==gDSP_board_type)
	{
		//DSP core speed: 100*10/1=1000MHz
		KeyStone_main_PLL_init(100, 10, 1); 
		KeyStone_PASS_PLL_init(100, 21, 2);

		ge_cfg.serdes_cfg.commonSetup.inputRefClock_MHz = 312.5;
	}
	else if(TCI6614_EVM==gDSP_board_type
		||DUAL_NYQUIST_EVM==gDSP_board_type
		||C6670_EVM==gDSP_board_type)
	{
		//DSP core speed: 122.88*236/29= 999.9889655MHz
		KeyStone_main_PLL_init(122.88, 236, 29);
		KeyStone_PASS_PLL_init(122.88, 205, 24);

		if(C6670_EVM==gDSP_board_type)
			ge_cfg.serdes_cfg.commonSetup.inputRefClock_MHz = 250;
		else if(DUAL_NYQUIST_EVM==gDSP_board_type)
			ge_cfg.serdes_cfg.commonSetup.inputRefClock_MHz = 156.25;
		else
		{
			ge_cfg.serdes_cfg.commonSetup.inputRefClock_MHz = 312.5;
		}
	}
	else
	{
		puts("Unknown DSP board type!");
		return;
	}

	//DDR init 66.66667*20/1= 1333
	KeyStone_DDR_init (66.66667, 20, 1, NULL);

	ge_cfg.serdes_cfg.commonSetup.loopBandwidth= SERDES_PLL_LOOP_BAND_MID;

	serdesLinkSetup.txOutputSwing    = 15; /*0~15 represents between 110 and 1310mVdfpp*/
	serdesLinkSetup.txInvertPolarity = SERDES_TX_NORMAL_POLARITY; 
	serdesLinkSetup.rxLos            = SERDES_RX_LOS_DISABLE; 
	serdesLinkSetup.rxAlign          = SERDES_RX_COMMA_ALIGNMENT_ENABLE; 
	serdesLinkSetup.rxInvertPolarity = SERDES_RX_NORMAL_POLARITY; 
	serdesLinkSetup.rxEqualizerConfig= SERDES_RX_EQ_ADAPTIVE; 
    serdesLinkSetup.rxCDR            = 1;

	if((ethernet_mode == ETHERNET_AUTO_NEGOTIAT_SLAVE)||
		(ethernet_mode == ETHERNET_AUTO_NEGOTIAT_MASTER))
	{
		//MDIO is enabled in negotiation mode
		ge_cfg.mdio_cfg= &mdio_cfg;
		
		/*The MDIO clock can operate at up to 2.5 MHz, 
		but typically operates at 1.0 MHz.*/
		mdio_cfg.clock_div= 350; 	/*350MHz/350= 1MHz*/
		mdio_cfg.link_INT0_PHY_select= MDIO_INT_SELECT_PHY_0;
		mdio_cfg.link_INT1_PHY_select= MDIO_INT_SELECT_PHY_1;
	
		if( test_data_path <= GE_TEST_SERDES_LOOPBACK )
		{
			puts("SGMII negotiation is not supported for loopback test. It is forced to 1000M_FULLDUPLEX at this time!");
			ethernet_mode= ETHERNET_1000M_FULLDUPLEX;
		}

		/*for test with two DSPs, if auto negotiation is used, 
		one should be master, and the other should be slave*/
		if(uiDspNum) 	//the second DSP
		{
			//swap the master/slave for the second DSP
			if(ethernet_mode == ETHERNET_AUTO_NEGOTIAT_MASTER)
				ethernet_mode = ETHERNET_AUTO_NEGOTIAT_SLAVE;
			else
				ethernet_mode = ETHERNET_AUTO_NEGOTIAT_MASTER;
		}
	}
	
	for(i=0; i<GE_NUM_ETHERNET_PORT; i++)
	{
		if(GE_PORT_NOT_USED==port_connect[i])
			continue;

		/*Serdes lanes are enabled for internal loopback or 
		when external connection is availible*/
		if((GE_PORT_SGMII_CONNECT==port_connect[i])
			||(GE_PORT_CABLE_CONNECT==port_connect[i])
			||(GE_TEST_SERDES_LOOPBACK>=test_data_path))
			ge_cfg.serdes_cfg.linkSetup[i]= &serdesLinkSetup;
		
		ethernet_port_cfg[i].mode= ethernet_mode;
		ethernet_port_cfg[i].CPPI_Src_ID= i+1;
		ethernet_port_cfg[i].RX_FIFO_Max_blocks= 8;
		ethernet_port_cfg[i].RX_flow_control_enable= TRUE;
		ethernet_port_cfg[i].TX_flow_control_enable= TRUE;
		ethernet_port_cfg[i].flow_control_MAC_Address= Source_MAC_address[i];
		ethernet_port_cfg[i].ethenet_port_statistics_enable= TRUE;
		ethernet_port_cfg[i].host_port_statistics_enable= TRUE;
		ethernet_port_cfg[i].prmiscuous_mode = ETHERNET_RX_CMF_EN;
		ge_cfg.ethernet_port_cfg[i]= &ethernet_port_cfg[i];
	}

	ge_cfg.RX_MAX_length= 9504;

	switch(test_data_path)
	{
	case GE_TEST_SGMII_LOOPBACK:
		printf("GE %s internal SGMII loopback test...\n", GE_mode_str[ethernet_mode]);
		GE_internal_loopback_test(ETHERNET_SGMII_LOOPBACK);
		break;
	case GE_TEST_SERDES_LOOPBACK:
		printf("GE %s internal SERDES loopback test...\n", GE_mode_str[ethernet_mode]);
		GE_internal_loopback_test(ETHERNET_SERDES_LOOPBACK);
		break;
	case GE_TEST_EXTERNAL_FIFO_LOOPBACK:
		printf("GE %s external loopback test (DSP0 TX -> DSP1 RX FIFO -> DSP1 TX FIFO-> DSP0 RX)...\n", GE_mode_str[ethernet_mode]);
		GE_external_loopback_test();
		break;
	case GE_TEST_DSP0_TO_DSP1:
		printf("GE %s two DSPs test (DSP0 -> DSP1)...\n", GE_mode_str[ethernet_mode]);
		GE_DSP0_to_DSP1_test();
		break;
	default:
		printf("GE %s internal MAC loopback test...\n", GE_mode_str[ethernet_mode]);
		GE_internal_loopback_test(ETHERNET_MAC_LOOPBACK);
	}

	puts("GE test complete.\n");
	//while(1);
}


