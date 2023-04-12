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
  You should run this project on second DSP firstly and then the first DSP
* =============================================================================
 *  Revision History
 *  ===============
 *  July 5, 2013 Kevin Cai   File Created
 * ============================================================================
 */
#ifndef _GE_TEST_H_
#define _GE_TEST_H_

#include <tistdtypes.h>

typedef enum
{
	/*DSP0 MAC TX -> DSP0 MAC RX*/
	GE_TEST_EMAC_LOOPBACK= 0,

	/*DSP0 SGMII TX-> DSP0 SGMII RX*/
	GE_TEST_SGMII_LOOPBACK,

	/*DSP0 SERDES TX -> DSP0 SERDES RX*/
	GE_TEST_SERDES_LOOPBACK,

	/*DSP0 TX -> DSP1 RX FIFO-> DSP1 TX FIFO-> DSP0 RX*/
	GE_TEST_EXTERNAL_FIFO_LOOPBACK,

	/*DSP0 TX -> DSP0 RX (or PC)*/
	GE_TEST_DSP0_TO_DSP1
}GE_Test_Data_Path;

//The port connection state for the test
typedef enum
{
	GE_PORT_NOT_USED = 0,

	/*the port is not connnected, can only run internal loopback test*/
	GE_PORT_NO_CONNECT, 	//recieve all packets to port 0

	/*the port connect through SGMII to another device*/
	GE_PORT_SGMII_CONNECT, 	//recieve all packets to port 0

	/*The port connect through PHY and ethernet cable to another device*/
	GE_PORT_CABLE_CONNECT 	
}GE_Port_Connection;

//ALE Test mode
typedef enum
{
	ALE_BYPASS = 0, 	//bypass ALE
	ALE_RECEIVE_ALL
}ALE_Test_Mode;

extern GE_Test_Data_Path test_data_path;
extern unsigned long long Source_MAC_address[];
extern unsigned long long Dest_MAC_address[];

Bool Port_OK(Uint32 uiPortNum);

#endif
