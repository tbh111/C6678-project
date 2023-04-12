/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2011
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
* ============================================================================
 Examples to show the usage and performance of SRIO on KeyStone DSP
 these tests are external loopback test on dual Nyquist EVM
 Please note, dual Nyquist EVM only connects port 2 and 3 between 2 DSPs.
* =============================================================================
 *  Revision History
 *  ===============
 *  July 12, 2011 Brighton Feng   File Created
 * ============================================================================
 */
#include <stdio.h>
#include "SRIO_PktDMA_Init.h"
#include "SRIO_test.h"
#include "SRIO_debug.h"
#include "SRIO_loopback_test.h"

/*Please note, dual Nyquist EVM only connects port 2 and 3 between 2 DSPs.
and packet forwording does not support message*/
SRIO_Multiple_Test_Config external_multi_port_forward_test_cfg[]=
{
    {SRIO_PATH_CTL_1xLaneA_1xLaneB_1xLaneC_1xLaneD, /*multiple_port_path*/
        /*packet_type               source                                                   dest                                                size*/
        {{0,                        0,                                                      0,                                                  0},   /*port0 is not availible for this case*/
        {0,                         0,                                                      0,                                                  0},   /*port1 is not availible for this case*/
        {SRIO_PKT_TYPE_SWRITE,     ((Uint32)&packetBuffer_LL2_Size1[2][0])+0x10000000,     ((Uint32)&packetBuffer_LL2_Size1[2][0])+0x11000000, LL2_PACKET_BUFFER_SIZE1},   /*port2*/
        {SRIO_PKT_TYPE_NREAD,      ((Uint32)&packetBuffer_LL2_Size1[3][0])+0x10000000,     ((Uint32)&packetBuffer_LL2_Size1[3][0])+0x11000000, LL2_PACKET_BUFFER_SIZE1}}    /*port3*/
    },
    {SRIO_PATH_CTL_1xLaneA_1xLaneB_1xLaneC_1xLaneD,               /*multiple_port_path*/
        /*packet_type               source                                                  dest                                                 size*/
        {{0,                        0,                                                      0,                                                  0},   /*port0 is not availible for this case*/
        {0,                         0,                                                      0,                                                  0},   /*port1 is not availible for this case*/
        {SRIO_PKT_TYPE_NWRITE,     ((Uint32)&packetBuffer_LL2_Size1[2][0])+0x10000000,     ((Uint32)&packetBuffer_LL2_Size1[2][0])+0x11000000, LL2_PACKET_BUFFER_SIZE1},   /*port2*/
        {SRIO_PKT_TYPE_NWRITE_R,   ((Uint32)&packetBuffer_LL2_Size1[3][0])+0x10000000,     ((Uint32)&packetBuffer_LL2_Size1[3][0])+0x11000000, LL2_PACKET_BUFFER_SIZE1}}    /*port3*/
    }
};

/*test SRIO transfer through single port*/
void srio_signle_port_external_forward_back_test(SRIO_Loopback_Mode loopback_mode)
{
	printf("SRIO link speed is %.3fGbps\n", SRIO_DEFAULT_TEST_SPEED);
	serdesLinkSetup.linkSpeed_GHz= SRIO_DEFAULT_TEST_SPEED;

	srio_cfg.srio_1x2x4x_path_control= SRIO_PATH_CTL_1xLaneA_1xLaneB_1xLaneC_1xLaneD;                
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
	
	srio_signle_port_loopback_test(2, loopback_mode);
}

/*test SRIO transfer through multiple ports in parallel*/
void srio_multiple_ports_external_forward_back_test()
{
	int i;
	Uint32 uiNumTestCases;

	SRIO_Multiple_Test_Config * test_cfg;

	printf("SRIO mulitple ports test with link speed %.3fGbps\n", SRIO_DEFAULT_TEST_SPEED);
	serdesLinkSetup.linkSpeed_GHz= SRIO_DEFAULT_TEST_SPEED;

	//test different cases
	uiNumTestCases = 
		sizeof(external_multi_port_forward_test_cfg)/sizeof(SRIO_Multiple_Test_Config);
	for(i= 0; i< uiNumTestCases; i++)
	{
		test_cfg= &external_multi_port_forward_test_cfg[i];
   		srio_cfg.srio_1x2x4x_path_control= test_cfg->multiple_port_path;            
		srio_identify_used_ports_lanes(srio_cfg.srio_1x2x4x_path_control);

		/*On dual Nyquist EVM only lane 2 and 3 are connected between 2 DSPs*/
		srio_cfg.blockEn.bLogic_Port_EN[0]= FALSE;
		srio_cfg.blockEn.bLogic_Port_EN[1]= FALSE;
	
		KeyStone_SRIO_Init(&srio_cfg);
	   	SRIO_PktDM_init();

		KeyStone_SRIO_match_ACK_ID(2, DSP1_SRIO_BASE_ID, 2);
		KeyStone_SRIO_match_ACK_ID(3, DSP1_SRIO_BASE_ID, 3);
		
		srio_multiple_port_transfer(test_cfg);
		//print_SRIO_status_error(&srio_cfg.blockEn);

		puts("");
	}
}

