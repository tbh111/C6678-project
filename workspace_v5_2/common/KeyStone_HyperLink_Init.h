/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2011
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
 example for HyperLink configuration on KeyStone DSP
 * =============================================================================
 *  Revision History
 *  ===============
 *  Nov 4, 2011 Brighton Feng  file created
 * =============================================================================
 */
#ifndef _KEYSTONE_HyperLink_INIT_H_
#define _KEYSTONE_HyperLink_INIT_H_

#include <cslr_vusr.h>
#include "KeyStone_common.h"
#include "KeyStone_Serdes_init.h"

#define NUM_MPS 	8 	/*Number of supported Micro Processor*/

/*select which bits of the RxAddress are used to select which 
Segment/Length array element value to Add/Check to the Address signal.*/
typedef enum
{
	RX_SEG_SEL_0, 	/*always use 0 as index to lookup segment/length table*/
	RX_SEG_SEL_ADDR_22_17,
	RX_SEG_SEL_ADDR_23_18,
	RX_SEG_SEL_ADDR_24_19,
	RX_SEG_SEL_ADDR_25_20,
	RX_SEG_SEL_ADDR_26_21,
	RX_SEG_SEL_ADDR_27_22,
	RX_SEG_SEL_ADDR_28_23,
	RX_SEG_SEL_ADDR_29_24,
	RX_SEG_SEL_ADDR_30_25,
	RX_SEG_SEL_ADDR_31_26,
	RX_SEG_SEL_ADDR_31_27,
	RX_SEG_SEL_ADDR_31_28,
	RX_SEG_SEL_ADDR_31_29,
	RX_SEG_SEL_ADDR_31_30,
	RX_SEG_SEL_ADDR_31   
}HyperLink_RxSegSel;

typedef enum
{
	RX_SEG_LEN_0x0_0000_0002,
	RX_SEG_LEN_0x0_0000_0004,
	RX_SEG_LEN_0x0_0000_0008,
	RX_SEG_LEN_0x0_0000_0010,
	RX_SEG_LEN_0x0_0000_0020,
	RX_SEG_LEN_0x0_0000_0040,
	RX_SEG_LEN_0x0_0000_0080,
	RX_SEG_LEN_0x0_0000_0100,
	RX_SEG_LEN_0x0_0000_0200,
	RX_SEG_LEN_0x0_0000_0400,
	RX_SEG_LEN_0x0_0000_0800,
	RX_SEG_LEN_0x0_0000_1000,
	RX_SEG_LEN_0x0_0000_2000,
	RX_SEG_LEN_0x0_0000_4000,
	RX_SEG_LEN_0x0_0000_8000,
	RX_SEG_LEN_0x0_0001_0000,
	RX_SEG_LEN_0x0_0002_0000,
	RX_SEG_LEN_0x0_0004_0000,
	RX_SEG_LEN_0x0_0008_0000,
	RX_SEG_LEN_0x0_0010_0000,
	RX_SEG_LEN_0x0_0020_0000,
	RX_SEG_LEN_0x0_0040_0000,
	RX_SEG_LEN_0x0_0080_0000,
	RX_SEG_LEN_0x0_0100_0000,
	RX_SEG_LEN_0x0_0200_0000,
	RX_SEG_LEN_0x0_0400_0000,
	RX_SEG_LEN_0x0_0800_0000,
	RX_SEG_LEN_0x0_1000_0000,
	RX_SEG_LEN_0x0_2000_0000,
	RX_SEG_LEN_0x0_4000_0000,
	RX_SEG_LEN_0x0_8000_0000,
	RX_SEG_LEN_0x1_0000_0000
}HyperLink_RxSegLen;

typedef struct{
	Uint32 Seg_Base_Addr; 	/*low 16 bits must be 0*/
	HyperLink_RxSegLen Seg_Length; 	/*must be power of 2, 0~0x100000000*/
}HyperLink_Rx_Address_Segment;

/*select which bits of the RxAddress are used to select which PrivID 
array element value to drive to the PrivID signal.*/
typedef enum
{
	RX_PRIVID_0, 	/*always use 0 as privID*/
	RX_PRIVID_SEL_ADDR_20_17,
	RX_PRIVID_SEL_ADDR_21_18,
	RX_PRIVID_SEL_ADDR_22_19,
	RX_PRIVID_SEL_ADDR_23_20,
	RX_PRIVID_SEL_ADDR_24_21,
	RX_PRIVID_SEL_ADDR_25_22,
	RX_PRIVID_SEL_ADDR_26_23,
	RX_PRIVID_SEL_ADDR_27_24,
	RX_PRIVID_SEL_ADDR_28_25,
	RX_PRIVID_SEL_ADDR_29_26,
	RX_PRIVID_SEL_ADDR_30_27,
	RX_PRIVID_SEL_ADDR_31_28,
	RX_PRIVID_SEL_ADDR_31_29,
	RX_PRIVID_SEL_ADDR_31_30,
	RX_PRIVID_SEL_ADDR_31
}HyperLink_RxPrivIdSel;

/*Specifies where the PrivID is placed in the outgoing TxAddress.*/
typedef enum
{
	TX_PRIVID_NO_OVERLAY,
	TX_PRIVID_OVL_ADDR_20_17,
	TX_PRIVID_OVL_ADDR_21_18,
	TX_PRIVID_OVL_ADDR_22_19,
	TX_PRIVID_OVL_ADDR_23_20,
	TX_PRIVID_OVL_ADDR_24_21,
	TX_PRIVID_OVL_ADDR_25_22,
	TX_PRIVID_OVL_ADDR_26_23,
	TX_PRIVID_OVL_ADDR_27_24,
	TX_PRIVID_OVL_ADDR_28_25,
	TX_PRIVID_OVL_ADDR_29_26,
	TX_PRIVID_OVL_ADDR_30_27,
	TX_PRIVID_OVL_ADDR_31_28,
	TX_PRIVID_OVL_ADDR_31_29,
	TX_PRIVID_OVL_ADDR_31_30,
	TX_PRIVID_OVL_ADDR_31
}HyperLink_TxPrivIdOvl;

/*create the mask that is logically anded to the incoming address to create 
the address sent to the remote.*/
typedef enum
{
	TX_ADDR_MASK_0x0001FFFF,
	TX_ADDR_MASK_0x0003FFFF,
	TX_ADDR_MASK_0x0007FFFF,
	TX_ADDR_MASK_0x000FFFFF,
	TX_ADDR_MASK_0x001FFFFF,
	TX_ADDR_MASK_0x003FFFFF,
	TX_ADDR_MASK_0x007FFFFF,
	TX_ADDR_MASK_0x00FFFFFF,
	TX_ADDR_MASK_0x01FFFFFF,
	TX_ADDR_MASK_0x03FFFFFF,
	TX_ADDR_MASK_0x07FFFFFF,
	TX_ADDR_MASK_0x0FFFFFFF,
	TX_ADDR_MASK_0x1FFFFFFF,
	TX_ADDR_MASK_0x3FFFFFFF,
	TX_ADDR_MASK_0x7FFFFFFF,
	TX_ADDR_MASK_0xFFFFFFFF
}HyperLink_TxAddrMask;

typedef struct{
	/*create the mask that is logically anded to the incoming address to create 
	the address sent to the remote.*/
	HyperLink_TxAddrMask tx_addr_mask;

	/*Specifies where the PrivID is placed in the outgoing TxAddress.*/
	HyperLink_TxPrivIdOvl tx_priv_id_ovl;

	/*The rxsegsel value is used to select which bits of the RxAddress 
	are used to select which Segment/Length array element value to 
	Add/Check to the Address signal.*/
	HyperLink_RxSegSel rx_seg_sel;

	HyperLink_Rx_Address_Segment rx_addr_segs[64];

	/*The rxprividsel value is used to select which bits of the 
	RxAddress are used to select which PrivID array element
	value to drive to the PrivID signal.*/
	HyperLink_RxPrivIdSel rx_priv_id_sel;

	Uint8 rx_priv_id_map[16];	
	
}HyperLink_Address_Map;

typedef struct{
	/*Interrupt Enable. When set, this bit indicates that interrupts detected 
	on the Hyperlink_int_i[icidx] pin should be should be forwarded to the 
	below mps:vector interrupt vector.*/
	Uint8 Int_en;

	/*Interrupt Type. When set, this bit indicates that the 
	Hyperlink_int_i[icidx] interrupt is pulsed. When clear, this bit
	indicates that Hyperlink_int_i[icidx] is level sensitive.*/
	Uint8 Int_type;

	/*Interrupt Polarity When set, this bit indicates that the 
	Hyperlink_int_i[icidx)] interrupt is active low. When clear, this
	bit indicates that Hyperlink_int_i[icidx] is active high*/
	Uint8 Int_pol; 

	/*Software Interrupt Enable indicate if this interrupt can be issued 
	via software writing to the Generate Soft Interrupt
	Value register for this SoftInt[icidx].*/
	Uint8 si_en; 

	Uint8 mps; 	/*MicroProcessor Select*/

	/*When the local device has int2Local =1, this field indicates which bit 
	of interrupt pending register to set. When the local device has 
	int2local =0, this field is transferred to the remote device, which is 
	used to indicate which bit of the interrupt pending register to set in 
	the remote device.	*/
	Uint8 vector; 
}HyperLink_int_event_control;

typedef struct{
	/*Interrupt Local. This bit determines whether interrupts are posted in 
	the Interrupt Status/Clear Register or forwarded via the serial interface. 
	When set, interrupts are posted in the Interrupt Status/Clear Register and
	the Hyperlink_intpls_o and Hyperlink_intlvl_o pins are asserted. 
	When clear, interrupts are forwarded out the serial interface to the remote 
	device.*/
	Uint8 int_local;

	/*Interrupt Enable. This bit causes HyperLink module status interrupts to
	be posted to the Interrupt Pending/Set Register*/
	Uint8 sts_int_enable;

	/*Interrupt Vector. This field indicates which bit in the Interrupt 
	Pending/Set Register is set for HyperLink module status interrupts.*/
	Uint8 sts_int_vec; 

	/*The Interrupt pending register can be set by the remote 
	device through the interrupt packet, when the int2cfg is set to 1. 
	When int2cfg is clear, the interrupt status is extracted from the
	received interrupt packet and written to the register indicated by 
	the Interrupt Pointer Register for the particular micro processor 
	select (MPS) value.*/
	Uint8 int2cfg; 

	HyperLink_int_event_control int_event_cntl[64];

	/*The Interrupt Pointer Registers typically map to microprocessor 
	interrupt controller set registers which get set to a one to interrupt 
	that processor.*/
	Uint32 int_set_register_pointer[NUM_MPS];
}HyperLink_Interrupt_Cfg;

typedef enum
{
	HyperLink_LOOPBACK_DISABLE = 0,
	HyperLink_LOOPBACK
}HyperLink_Loopback_Mode;


typedef struct  { 
	HyperLink_Address_Map address_map;
	HyperLink_Interrupt_Cfg interrupt_cfg;
	HyperLink_Loopback_Mode loopback_mode;
	SerdesSetup_4links serdes_cfg;
} HyperLink_Config;

extern CSL_VusrRegs * gpHyperLinkRegs;
extern SerdesRegs * hyperLinkSerdesRegs;

/*soft shutdown and reset HyperLink*/
extern void KeyStone_HyperLink_soft_reset();

extern void KeyStone_HyperLink_Init(HyperLink_Config * HyperLink_cfg);

#endif
