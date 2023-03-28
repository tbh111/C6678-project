/******************************************************************************

  Copyright (C), 2013, Texas Instrument.

 ******************************************************************************
  File Name     : KeyStone_GE_Init_drv.h
  Version       : Initial Draft
  Author        : Kevin Cai
  Created       : July 3, 2013
  Last Modified :
  Description   : example for Gigbit ethernet configuration and 
                  transfer driver on KeyStone DSP

  History       :
  1.Date        : July 3, 2013
    Author      : Kevin Cai
    Modification: Created file
  2.Date        : August 25, 2013
    Author      : Brighton Feng
    Modification: Update SGMII Serdes configuration
******************************************************************************/

#ifndef _KEYSTONE_GE_INIT_DRV_H_
#define _KEYSTONE_GE_INIT_DRV_H_

#include <tistdtypes.h>
#include <csl_cpsw_3gf.h>
#include <cslr_cpgmac_sl.h>
#include <cslr_cpsw_3gf.h>
#include <cslr_cpsgmii.h>
#include <cslr_pa_ss.h>
#include <cslr_mdio.h>
#include "KeyStone_Serdes_init.h"   

#define GE_NUM_ETHERNET_PORT 		2
#define GE_DIRECT_TX_QUEUE 			648
#define GE_DIRECT_TX_CHANNEL 		8
#define GE_DIRECT_RX_PORT1_CHANNEL 	22
#define GE_DIRECT_RX_PORT2_CHANNEL 	23


/*Ethernet MAC header length in bytes*/
#define EMAC_HEADER_LEN 			14
/*Ethernet MAC CRC length in bytes*/
#define EMAC_CRC_LEN 				4

typedef enum
{
	ETHERNET_IPV4_PACKET = 0x0800,
	ETHERNET_ARP_PACKET  = 0x0806,
	ETHERNET_IPV6_PACKET = 0x86DD 
}Ethernet_Packet_Type;

typedef enum
{
	ETHERNET_LOOPBACK_DISABLE = 0,

	/*MAC, SGMII and Serdes loopback path is TX -> RX*/
	ETHERNET_MAC_LOOPBACK,
	ETHERNET_SGMII_LOOPBACK,
	ETHERNET_SERDES_LOOPBACK,

	/*Each packet received in a port is turned around and sent out on 
	the same port¡¯s transmit path: RX -> TX*/
	ETHERNET_PORT_FIFO_LOOPBACK
}Ethernet_Loopback_Mode;

typedef enum
{
	/*SGMII will be slave in auto negotiation mode*/
	ETHERNET_AUTO_NEGOTIAT_SLAVE = 0,
	/*SGMII will be master in auto negotiation mode*/
	ETHERNET_AUTO_NEGOTIAT_MASTER,

	/*SGMII will be in master force mode for following modes*/
	//ETHERNET_10M_HALFDUPLEX,
	ETHERNET_10M_FULLDUPLEX,
	//ETHERNET_100M_HALFDUPLEX,
	ETHERNET_100M_FULLDUPLEX,
	ETHERNET_1000M_FULLDUPLEX,
	ETHERNET_10G_FULLDUPLEX
}Ethernet_Mode;

typedef enum
{
	/*disable promiscous mode*/
	ETHERNET_PROMISCOUS_DISABLE = 0,

	/*Enables frames containing errors to be transferred to memory.*/
	ETHERNET_RX_CEF_EN= CSL_CPGMAC_SL_MACCONTROL_REG_RX_CEF_EN_MASK,

	/*Enables frames or fragments shorter than 64 bytes to be copied to memory*/
	ETHERNET_RX_CSF_EN= CSL_CPGMAC_SL_MACCONTROL_REG_RX_CSF_EN_MASK,

	/*Enables MAC control frames to be transferred to memory.*/
	ETHERNET_RX_CMF_EN= CSL_CPGMAC_SL_MACCONTROL_REG_RX_CMF_EN_MASK,

	/*enable all frames to be transfered to memory*/
	ETHERNET_PROMISCOUS_EN= (CSL_CPGMAC_SL_MACCONTROL_REG_RX_CEF_EN_MASK
		|CSL_CPGMAC_SL_MACCONTROL_REG_RX_CSF_EN_MASK
		|CSL_CPGMAC_SL_MACCONTROL_REG_RX_CMF_EN_MASK)
}Ethernet_Promiscuous_Mode;

/*PHY selected to trigger MDIO link interrupt*/
typedef enum
{
	MDIO_INT_SELECT_PHY_0= 0, 
	MDIO_INT_SELECT_PHY_1 ,
	MDIO_INT_SELECT_PHY_2 ,
	MDIO_INT_SELECT_PHY_3 ,
	MDIO_INT_SELECT_PHY_4 ,
	MDIO_INT_SELECT_PHY_5 ,
	MDIO_INT_SELECT_PHY_6 ,
	MDIO_INT_SELECT_PHY_7 ,
	MDIO_INT_SELECT_PHY_8 ,
	MDIO_INT_SELECT_PHY_9 ,
	MDIO_INT_SELECT_PHY_10,
	MDIO_INT_SELECT_PHY_11,
	MDIO_INT_SELECT_PHY_12,
	MDIO_INT_SELECT_PHY_13,
	MDIO_INT_SELECT_PHY_14,
	MDIO_INT_SELECT_PHY_15,
	MDIO_INT_SELECT_PHY_16,
	MDIO_INT_SELECT_PHY_17,
	MDIO_INT_SELECT_PHY_18,
	MDIO_INT_SELECT_PHY_19,
	MDIO_INT_SELECT_PHY_20,
	MDIO_INT_SELECT_PHY_21,
	MDIO_INT_SELECT_PHY_22,
	MDIO_INT_SELECT_PHY_23,
	MDIO_INT_SELECT_PHY_24,
	MDIO_INT_SELECT_PHY_25,
	MDIO_INT_SELECT_PHY_26,
	MDIO_INT_SELECT_PHY_27,
	MDIO_INT_SELECT_PHY_28,
	MDIO_INT_SELECT_PHY_29,
	MDIO_INT_SELECT_PHY_30,
	MDIO_INT_SELECT_PHY_31,
	MDIO_INT_SELECT_PHY_NONE
}MDIO_Link_INT_PHY_Select;

/*Select Statistics module*/
typedef enum
{
	GE_STATSA= 0,
	GE_STATSB,
	GE_NUM_STAT_MODULE
}GE_Statistics_Module;

typedef struct
{
    Ethernet_Mode               mode;

    /*This value is contained in the CPPI Info Word 0 SRC_ID field for packets 
    received on a port. Used to determine which port the packet come from*/
    Uint8                       CPPI_Src_ID;
    
    /*Receive FIFO Maximum Blocks: 3~15.
    TX_MAX_BLKS= 20 - RX_MAX_BLKS*/
    Uint8                       RX_FIFO_Max_blocks; 

    Bool                        RX_flow_control_enable;
    Bool                        TX_flow_control_enable;
    /*MAC address used in pause frame. 
    unsigned long long type (8 bytes) is used, but only lower 6 bytes are valid.
    Please note the byte order, MAC address byte 5 is in the lowest bits.*/
    unsigned long long          flow_control_MAC_Address;

    /*enabled the statisics in the ethernet port*/
    Bool                        ethenet_port_statistics_enable;
    /*enabled the statisics in host port to/from this ethernet port*/
    Bool                        host_port_statistics_enable;

    Ethernet_Promiscuous_Mode   prmiscuous_mode;
}Ethernet_Port_Config;

typedef struct
{
	/*each pointer points to a table with num_xxx ALE entries 
	NULL pointer or num_xxx=0 means no entry need be set for that type of ALE*/
    CSL_CPSW_3GF_ALE_MCASTADDR_ENTRY        * multicastEntries;
    CSL_CPSW_3GF_ALE_VLANMCASTADDR_ENTRY    * VLAN_multicastEntries;
    CSL_CPSW_3GF_ALE_UNICASTADDR_ENTRY      * unicastEntries;
    CSL_CPSW_3GF_ALE_OUIADDR_ENTRY          * OUI_entries;
    CSL_CPSW_3GF_ALE_VLANUNICASTADDR_ENTRY  * VLAN_UnicastEntries;
    CSL_CPSW_3GF_ALE_VLAN_ENTRY             * VLAN_entires;
    Uint16                                num_multicastEntries;
    Uint16                                num_VLAN_multicastEntries;
    Uint16                                num_unicastEntries;
    Uint16                                num_OUI_entries;
    Uint16                                num_VLAN_UnicastEntries;
    Uint16                                num_VLAN_entires;
}Ethernet_ALE_Config;

typedef struct
{
	/*The Peripheral clock frequency is driven by the SERDES line rate
	divided by 10 on Keystone I devices.The peripheral clock frequency 
	is driven by NETCP clock on Keystone II devices.*/
 	Uint16 clock_div;

	/*select one PHY to trigger the MIDO link interrupt*/
	MDIO_Link_INT_PHY_Select link_INT0_PHY_select;
	MDIO_Link_INT_PHY_Select link_INT1_PHY_select;
}Ethernet_MDIO_Config;

typedef struct  { 
    Ethernet_Loopback_Mode  loopback_mode;

	SerdesSetup_2links      serdes_cfg;

    Uint16                  RX_MAX_length; /*The maximum value is 9504*/

    /*pointers to port configuration structure.
    first pointer is for port 1, the sencond pointer for port 2, and so on.
    null pointer means the port is not used*/
    Ethernet_Port_Config *  ethernet_port_cfg[GE_NUM_ETHERNET_PORT]; 

    /*pointer to ALE configuration structure.
    null pointer means ALE bypass*/
    Ethernet_ALE_Config *   ale_cfg;

    /*pointer to MDIO configuration structure.
    null pointer means MDIO is not used*/
    Ethernet_MDIO_Config *  mdio_cfg;

} KeyStone_GE_Config;

/* Holds the EMAC statistics. 64 bits is used to avoid quick overflow of 32-bit value, 
the 32-bit values in registers may be accumulated into this data structure.*/
typedef struct {
    /** Good Frames Received                      */        
    unsigned long long      RxGoodFrames;     

    /** Good Broadcast Frames Received            */
    unsigned long long      RxBCastFrames;    

    /** Good Multicast Frames Received            */
    unsigned long long      RxMCastFrames;    

    /** PauseRx Frames Received                   */
    unsigned long long      RxPauseFrames;    

    /** Frames Received with CRC Errors           */
    unsigned long long      RxCRCErrors;      

    /** Frames Received with Alignment/Code Errors*/
    unsigned long long      RxAlignCodeErrors;

    /** Oversized Frames Received                 */
    unsigned long long      RxOversized;      

    /** Jabber Frames Received                    */
    unsigned long long      RxJabber;         

    /** Undersized Frames Received                */
    unsigned long long      RxUndersized;     

    /** Rx Frame Fragments Received               */
    unsigned long long      RxFragments;      

    /** Reserved       */
    unsigned long long      reserved;         

    /** Reserved */
    unsigned long long      reserved2;      

    /** Total Received Bytes in Good Frames       */
    unsigned long long      RxOctets;         

    /** Good Frames Sent                          */
    unsigned long long      TxGoodFrames;     
    
    /** Good Broadcast Frames Sent                */
    unsigned long long      TxBCastFrames;    

    /** Good Multicast Frames Sent                */
    unsigned long long      TxMCastFrames;    

    /** PauseTx Frames Sent                       */
    unsigned long long      TxPauseFrames;    

    /** Frames Where Transmission was Deferred    */
    unsigned long long      TxDeferred;       

    /** Total Frames Sent With Collision          */
    unsigned long long      TxCollision;      

    /** Frames Sent with Exactly One Collision    */
    unsigned long long      TxSingleColl;     

    /** Frames Sent with Multiple Colisions       */
    unsigned long long      TxMultiColl;      

    /** Tx Frames Lost Due to Excessive Collisions*/
    unsigned long long      TxExcessiveColl;  

    /** Tx Frames Lost Due to a Late Collision    */
    unsigned long long      TxLateColl;       

    /** Tx Frames Lost with Tx Underrun Error     */
    unsigned long long      TxUnderrun;       

    /** Tx Frames Lost Due to Carrier Sense Loss  */
    unsigned long long      TxCarrierSLoss;   

    /** Total Transmitted Bytes in Good Frames    */
    unsigned long long      TxOctets;         

    /** Total Tx&Rx with Octet Size of 64         */
    unsigned long long      Frame64;          

    /** Total Tx&Rx with Octet Size of 65 to 127  */
    unsigned long long      Frame65t127;      

    /** Total Tx&Rx with Octet Size of 128 to 255 */
    unsigned long long      Frame128t255;     

    /** Total Tx&Rx with Octet Size of 256 to 511 */
    unsigned long long      Frame256t511;     

    /** Total Tx&Rx with Octet Size of 512 to 1023*/
    unsigned long long      Frame512t1023;    

    /** Total Tx&Rx with Octet Size of >=1024     */
    unsigned long long      Frame1024tUp;     

    /** Sum of all Octets Tx or Rx on the Network */
    unsigned long long      NetOctets;        

    /** Total Rx Start of Frame Overruns          */
    unsigned long long      RxSOFOverruns;    

    /** Total Rx Middle of Frame Overruns         */
    unsigned long long      RxMOFOverruns;    

    /** Total Rx DMA Overruns                     */
    unsigned long long      RxDMAOverruns;    
} Ethernet_Statistics;

extern CSL_CpsgmiiRegs *  gpSGMII_regs[];
extern CSL_Mdio_Regs *  gpMDIO_regs;     
extern CSL_Cpsw_3gfRegs *  gpCPSW_regs;
extern CSL_Cpsw_3gfPort_info_groupRegs *  gpCPSW_port_regs[];
extern CSL_Cpgmac_slRegs *  gpMAC_regs[];
extern CSL_Cpsw_3gfPort_stats_groupRegs * gpStats_regs[];
extern CSL_Pa_ssRegs * gpNetCP_regs;

//accumulation of the statistics values
extern Ethernet_Statistics statistics[];

/*Giga bit ethernet switch subsystem reset*/
extern void KeyStone_GE_soft_reset();

/*address look up table initialize*/
extern void KeyStone_GE_ALE_Init(Ethernet_ALE_Config *   ale_cfg);

/*setup MDIO for PHY controlling*/
extern void KeyStone_MDIO_Init(Ethernet_MDIO_Config *  mdio_cfg);

/*Giga bit ethernet initialization*/
extern void KeyStone_GE_Init(KeyStone_GE_Config * ge_cfg);

/*accumulate the statistics values in the registers to the software data structure.
application should call this function before the 32-bit register counters overflow*/
extern void KeyStone_GE_Accumulate_Statistics();

/*fill EMAC header to a packet buffer*/
extern void Fill_EMAC_header(Uint8 *buffer, Ethernet_Packet_Type type,
	unsigned long long sourceMAC, unsigned long long destMAC);

/*get EMAC header information in a packet buffer*/
extern void Get_EMAC_header(Uint8 *buffer, Ethernet_Packet_Type * type,
	unsigned long long * sourceMAC, unsigned long long * destMAC);

/*set "data" in to a register of a PHY*/
extern void KeyStone_MDIO_PHY_Set_Reg(Uint32 phyNum, Uint32 regNum, Uint16 data);

/*read data from a register of a PHY*/
extern Uint16 KeyStone_MDIO_PHY_Get_Reg(Uint32 phyNum, Uint32 regNum);

#endif
