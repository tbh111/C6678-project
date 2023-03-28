/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2013
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
* ============================================================================
 Example to show the configuration of PCIE on KeyStone DSP
* =============================================================================
 *  Revision History
 *  ===============
 *  June 13, 2013 Thomas Yang   File Created
 *  June 22, 2013 Brighton Feng   Update APIs
 * ============================================================================
 */
 
#ifndef _KEYSTONE_PCIE_INIT_H_
#define _KEYSTONE_PCIE_INIT_H_

#include <cslr_device.h>
#include <cslr_pcie_cfg_space_endpoint.h>
#include <cslr_pcie_cfg_space_rootcomplex.h>
#include <cslr_pciess_app.h>
#include "KeyStone_common.h"
#include "KeyStone_Serdes_init.h"

#define PCIE_SERDES_CFG_TX_LOOPBACK_SHIFT    19
#define PCIE_SERDES_CFG_TX_MSYNC_SHIFT       18
#define PCIE_SERDES_CFG_TX_CM_SHIFT          17
#define PCIE_SERDES_CFG_TX_INVPAIR_SHIFT     16
#define PCIE_SERDES_CFG_RX_LOOPBACK_SHIFT    14
#define PCIE_SERDES_CFG_RX_ENOC_SHIFT        13
#define PCIE_SERDES_CFG_RX_EQ_SHIFT          9 
#define PCIE_SERDES_CFG_RX_CDR_SHIFT         6 
#define PCIE_SERDES_CFG_RX_LOS_SHIFT         3 
#define PCIE_SERDES_CFG_RX_ALIGN_SHIFT       1 
#define PCIE_SERDES_CFG_RX_INVPAIR_SHIFT     0 
/**************************************************************************\
* Register Overlay Structure
PCIE capabilities, implementation specific registers
\**************************************************************************/
typedef struct  {
    volatile Uint32 PMCAP;
    volatile Uint32 PM_CTL_STAT;
    volatile Uint8 RSVD0[8];
    volatile Uint32 MSI_CAP;
    volatile Uint32 MSI_LOW32;
    volatile Uint32 MSI_UP32;
    volatile Uint32 MSI_DATA;
    volatile Uint8 RSVD1[16];
    volatile Uint32 PCIES_CAP;
    volatile Uint32 DEVICE_CAP;
    volatile Uint32 DEV_STAT_CTRL;
    volatile Uint32 LINK_CAP;
    volatile Uint32 LINK_STAT_CTRL;
    volatile Uint32 SLOT_CAP;
    volatile Uint32 SLOT_STAT_CTRL;
    volatile Uint32 ROOT_CTRL_CAP;
    volatile Uint32 ROOT_STATUS;
    volatile Uint32 DEV_CAP2;
    volatile Uint32 DEV_STAT_CTRL2;
    volatile Uint8 RSVD2[4];
    volatile Uint32 LINK_CTRL2;
    volatile Uint8 RSVD3[92];
    volatile Uint32 PCIE_EXTCAP;
    volatile Uint32 PCIE_UNCERR;
    volatile Uint32 PCIE_UNCERR_MASK;
    volatile Uint32 PCIE_UNCERR_SVRTY;
    volatile Uint32 PCIE_CERR;
    volatile Uint32 PCIE_CERR_MASK;
    volatile Uint32 PCIE_ACCR;
    volatile Uint32 HDR_LOG[4];
    volatile Uint32 RC_ERR_CMD;
    volatile Uint32 RC_ERR_ST;
    volatile Uint32 ERR_SRC_ID;
    volatile Uint8 RSVD4[1480];
    volatile Uint32 PL_ACKTIMER;
    volatile Uint32 PL_OMSG;
    volatile Uint32 PL_FORCE_LINK;
    volatile Uint32 ACK_FREQ;
    volatile Uint32 PL_LINK_CTRL;
    volatile Uint32 LANE_SKEW;
    volatile Uint32 SYM_NUM;
    volatile Uint32 SYMTIMER_FLTMASK;
    volatile Uint32 FLT_MASK2;
    volatile Uint8 RSVD5[4];
    volatile Uint32 DEBUG0;
    volatile Uint32 DEBUG1;
    volatile Uint8 RSVD6[220];
    volatile Uint32 PL_GEN2;
} PCIE_CAP_Implement_Regs;

typedef struct  {
    volatile Uint32 MSI_CAP;
    volatile Uint32 MSI_LOW32;
    volatile Uint32 MSI_UP32;
    volatile Uint32 MSI_DATA;
} PCIE_MSI_Regs;

typedef enum{
	PCIE_LOOPBACK_DISABLE = 0,
	PCIE_PHY_LOOPBACK 	/*PHY loopback is only vaild with RC mode*/
}PCIE_Loopback_Mode;

/** These are the possible values for PCIe mode */
typedef enum 
{
	PCIE_EP_MODE = 0,    
	PCIE_LEGACY_EP_MODE, 
	PCIE_RC_MODE   
}PCIE_Mode;

typedef enum{
	PCIE_ADDRESS_32_BITS = 0,
	PCIE_ADDRESS_64_BITS
}PCIE_Address_Width;

/*endian swap for PCIE access*/
typedef enum 
{
	PCIE_ENDIAN_SWAP_ON_1_BYTE = 0,    
	PCIE_ENDIAN_SWAP_ON_2_BYTES, 
	PCIE_ENDIAN_SWAP_ON_4_BYTES,
	PCIE_ENDIAN_SWAP_ON_8_BYTES
}PCIE_Endian_Swap;

/*PCIE outbound region size*/
typedef enum 
{
	PCIE_OB_SIZE_1MB= 0,
	PCIE_OB_SIZE_2MB,
	PCIE_OB_SIZE_4MB,
	PCIE_OB_SIZE_8MB
}PCIE_Outbound_Region_Size;

/*LTSSM STATE*/
typedef enum 
{
	LTSSM_STAT_DETECT_QUIET      = 0x00,
	LTSSM_STAT_DETECT_ACT        = 0x01,
	LTSSM_STAT_POLL_ACTIVE       = 0x02,
	LTSSM_STAT_POLL_COMPLIANCE   = 0x03,
	LTSSM_STAT_POLL_CONFIG       = 0x04,
	LTSSM_STAT_PRE_DETECT_QUIET  = 0x05,
	LTSSM_STAT_DETECT_WAIT       = 0x06,
	LTSSM_STAT_CFG_LINKWD_START  = 0x07,
	LTSSM_STAT_CFG_LINKWD_ACEPT  = 0x08,
	LTSSM_STAT_CFG_LANENUM_WAIT  = 0x09,
	LTSSM_STAT_CFG_LANENUM_ACEPT = 0x0A,
	LTSSM_STAT_CFG_COMPLETE      = 0x0B,
	LTSSM_STAT_CFG_IDLE          = 0x0C,
	LTSSM_STAT_RCVRY_LOCK        = 0x0D,
	LTSSM_STAT_RCVRY_SPEED       = 0x0E,
	LTSSM_STAT_RCVRY_RCVRCFG     = 0x0F,
	LTSSM_STAT_RCVRY_IDLE        = 0x10,
	LTSSM_STAT_L0                = 0x11,
	LTSSM_STAT_L0s               = 0x12,
	LTSSM_STAT_L123_SEND_EIDLE   = 0x13,
	LTSSM_STAT_L1_IDLE           = 0x14,
	LTSSM_STAT_L2_IDLE           = 0x15,
	LTSSM_STAT_L2_WAKE           = 0x16,
	LTSSM_STAT_DISABLED_ENTRY    = 0x17,
	LTSSM_STAT_DISABLED_IDLE     = 0x18,
	LTSSM_STAT_DISABLED          = 0x19,
	LTSSM_STAT_LPBK_ENTRY        = 0x1A,
	LTSSM_STAT_LPBK_ACTIVE       = 0x1B,
	LTSSM_STAT_LPBK_EXIT         = 0x1C,
	LTSSM_STAT_LPBK_EXIT_TIMEOUT = 0x1D,
	LTSSM_STAT_HOT_RESET_ENTRY   = 0x1E,
	LTSSM_STAT_HOT_RESET         = 0x1F
}PCIE_LTSSM_State;

/*the memory region be accessed through PCIE.*/
typedef struct{
	Uint32 uiTargetAddress;//the address in target memory
	Uint32 uiNumBytes;
}PCIE_Memory_Region;

/*multiple memory regions with same prefetchable property be accessed 
through PCIE, these regions may be mapped to same BAR*/
typedef struct{
	PCIE_Memory_Region * memory_regions;
	Uint32 uiNumRegions;
	Bool   bPrefetchable;

	/*below are not input parameters. These are calculated during 
	initialization according to the size of all regions*/
	unsigned long long ullTotalSize;/*total size of the memory regions*/
	unsigned long long ullBAR_Mask; /*BAR mask for corresponding BAR*/
}PCIE_Memory_Regions;

/*multiple memory regions be accessed through PCIE inbound interface.
Please note, number of inbound memory regions can not be greater than 4.
In RC mode, there is only one memory BAR with 32-bit address.*/
typedef struct{
	//prefetchable memory regions (will be mapped to one BAR)
	PCIE_Memory_Regions * prefetch_regions; 

	//non-prefetchable memory regions (will be mapped to one BAR)
	PCIE_Memory_Regions * nonfetch_regions;
}PCIE_Inbound_Memory_Regions;

/*PCIE outbound regions configure*/
typedef struct{
	/*uiNumRegions and address_offset are optional,
	if they are NULL, RC will configure outbound memory via enumeration.*/
	unsigned long long * address_offset;
	Uint32 uiNumRegions; 

	PCIE_Outbound_Region_Size OB_size;
}PCIE_Outbound_Memory_Regions;

/*PCIE BAR configure*/
typedef struct{
	unsigned long long ullMask; //Mask represent size of the BAR
	PCIE_Address_Width address_width;
	Bool   bPrefetchable;
	Bool   bIO;	//is IO space BAR

	unsigned long long ullStart_address;
}PCIE_BAR_Config;

/*Remote Configuration Transaction Setup,
select the bus, device and function number of the target*/
typedef struct
{
	Uint8 config_type; //type 0 is EP; type 1 is RC
	Uint8 config_bus;
	Uint8 config_device;
	Uint8 config_function;
}PCIE_Remote_CFG_SETUP;

/*PCIE RC mode specific configuration*/
typedef struct{
	/*the space inside (base, limit) range are for EP,
	access outside of this range will go to RC*/
	Uint32 memory_base;
	Uint32 memory_limit;

	/*Though this parameter is 64-bit, the higher 32 bits are 
	ignored in 32-bit mode.*/
	unsigned long long prefetch_memory_base;
	unsigned long long prefetch_memory_limit;

	/*RC BAR0 directly map to RC's PCIE application registers*/
	Uint32 BAR0_address;
}PCIE_RC_Config;

/*The configuration for internal bus between PCIE and memory subsystem*/
typedef struct{
	Bool bSupervisor; //PCIE has supervisor privilege on memory subsystem
	Uint8 priority;

	/*endian swap for PCIE access*/
	PCIE_Endian_Swap endian_swap;	
}PCIE_Internal_Bus_Config;

/*number of MSI vectors need for this EP*/
typedef enum 
{
	PCIE_1_MSI=0,
	PCIE_2_MSI,
	PCIE_4_MSI,
	PCIE_8_MSI,
	PCIE_16_MSI,
	PCIE_32_MSI,
	PCIE_NO_TX_MSI 	/*no MSI is generated from this EP*/
}PCIE_number_MSI;

/*interrupt configuration*/
typedef struct{
	
	Uint32 MSI_rx_enable_mask; /*each bit enable reception of one MSI*/
	Bool   Err_rx_enable; 	/*enable error interrupt reception*/
	Bool   PMRST_rx_enable; 	/*enable power and reset interrupt reception*/
	PCIE_number_MSI number_tx_MSI; /*number of MSI may generate from this EP*/
}PCIE_Interrupt_Config;

/*PCIE Error check, dectction, report configuration*/
typedef struct{
	Bool bErrorEnable;
	
	//to do
}PCIE_Error_Config;

typedef enum 
{
    PCIE_DE_EMPHASIS_6_DB = 0,
    PCIE_DE_EMPHASIS_3P5_DB = 1
}PCIE_De_emphasis;

typedef enum 
{
    PCIE_TX_SWING_FULL = 0,
    PCIE_TX_SWING_LOW = 1
}PCIE_Tx_Swing;

//PCIE TX Serdes configuration defined in PCIE specification
typedef struct
{
	PCIE_Tx_Swing swing;
	Uint8 		tx_margin; /*0~7: 0->highest; 7->lowest*/
	PCIE_De_emphasis de_emphasis;
	PCIE_De_emphasis EP_de_emphasis;
	PCIE_De_emphasis EP_5G_de_emphasis;
}PCIE_SerDesTxConfig;

typedef struct
{
    float inputRefClock_MHz; 	/*input reference clock in MHz*/

    float 	linkSpeed_GHz; 	/*expected link speed in GHz*/

    Uint32 numLanes; /*number of lanes*/

	/** PLL Loop bandwidth setting  */
	SerdesLoopBandwidth     loopBandwidth;

	SerdesLoopback  	loopBack;

	/** Invert Polarity. Inverts the polarity of TXPi and TXNi. */
	SerdesTxInvertPolarity    txInvertPolarity;

	/** Rx loss of Signal */
	SerdesRxLos         rxLos;
	
	SerdesRxAlign 		rxAlign;

    /* Clock/data recovery. Configures the clock/data recovery algorithm */
    Uint8               rxCDR;

	/** polairty of Rx differential i/p - normal/inverted */
	SerdesRxInvertPolarity      rxInvertPolarity;

	/** Rx equalizer configuration */
	SerdesRxEqConfig          rxEqualizerConfig;

	/*pointer to TX configuration defined in PCIE specification.
	If it is NULL, default configuration will be used*/
	PCIE_SerDesTxConfig 	* tx_cfg;
} KeyStone_PCIE_SerdesConfig;

typedef struct  {
	PCIE_Mode PcieMode;
	PCIE_Loopback_Mode loop_mode;
	PCIE_Address_Width address_width;

	/*pointer to inbound memory configuration structure,
	NULL menas inbound access is not used*/
	PCIE_Inbound_Memory_Regions * inbound_memory_regions;

	/*pointer to outbound memory configuration structure.
	If it is NULL, in EP mode, menas outbound access is not used; in RC mode, 
	the OB size in this configuration structure must be provided*/
	PCIE_Outbound_Memory_Regions * outbound_memory_regions;

	/*pointer to PCIE RC mode specific configuration structure.
	For EP mode, assign NULL pointer to it*/
	PCIE_RC_Config * rc_cfg;

	KeyStone_PCIE_SerdesConfig serdes_cfg;

	/*this device and the device at the opposite end of the link are operating 
	with a common clock source.*/
	Bool bCommon_clock;

	/*pointer to the MSI interrupt configuration structure. 
	NULL pointer means MSI is not used*/
	PCIE_Interrupt_Config * interrupt_cfg;

	/*pointer to configuration structure for internal bus between PCIE and memory subsystem. 
	NULL pointer results in using default values*/
	PCIE_Internal_Bus_Config * bus_cfg;

	/*pointer to the PCIE Error check, dectction, report configuration structure. 
	NULL pointer results in using default values*/
	PCIE_Error_Config * error_cfg;

} KeyStone_PCIE_Config;

extern CSL_Pciess_appRegs   				*gpPCIE_app_regs;
extern CSL_Pcie_cfg_space_rootcomplexRegs 	*gpPCIE_RC_regs ;
extern CSL_Pcie_cfg_space_endpointRegs		*gpPCIE_EP_regs ;
extern PCIE_CAP_Implement_Regs 				*gpPCIE_CAP_implement_regs ;
extern CSL_Pcie_cfg_space_endpointRegs		*gpPCIE_remote_EP_regs;

/************************Function define******************************/
static inline void Wait_PCIE_PLL_Lock()
{
    while ((gpBootCfgRegs->STS_PCIE & 0x00000001) != 0x00000001);
}

extern void KeyStone_PCIE_Serdes_init(
	KeyStone_PCIE_SerdesConfig * pcie_serdes_cfg);

/*PCIE outbound memory regions initialize*/
extern void KeyStone_PCIE_Outbound_Memory_Region_Init(
	PCIE_Outbound_Memory_Regions * outbound_memory_regions);

/*initialize internal bus between PCIE and memory subsystem*/
extern void KeyStone_PCIE_Internal_Bus_Init(PCIE_Internal_Bus_Config * bus_cfg);

/*PCIE initialization*/
extern void KeyStone_PCIE_Init(KeyStone_PCIE_Config * pcie_cfg);

/*PCIE address allocation (via the PCI Express enumeration procedure).
setup outbound and inbound address mapping*/
extern void KeyStone_PCIE_Address_setup(KeyStone_PCIE_Config * pcie_cfg);

/*Remote Configuration Transaction Setup,
select the bus, device and function number of the target*/
extern void KeyStone_PCIE_remote_CFG_setup(PCIE_Remote_CFG_SETUP * cfg_setup);

/*PCIE MSI allocation for one device.
msi_regs: pointer to remote configuration space. For loopback test, pointer to local bus space
MSI_address: PCIE address for EP to write to generate MSI*/
extern void KeyStone_PCIE_RC_MSI_allocate(PCIE_MSI_Regs * msi_regs,
	unsigned long long PCIE_MSI_address);

/*generate one MSI interrupt through PCIE.
MSI_number: 0~31
MSI_address: the remapped address in PCIE window that should be written to generate the MSI*/
extern void KeyStone_PCIE_generate_MSI(Uint32 MSI_number, Uint32 * MSI_address);

//clear all interrupt status registers of PCIE
extern void KeyStone_PCIE_clear_interrupts();

#endif
