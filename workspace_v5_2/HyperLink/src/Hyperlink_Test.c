/****************************************************************************\
 *           Copyright (C) 2011, 2012 Texas Instruments Incorporated.             *
 *                           All Rights Reserved                            *
 *                                                                          *
 * GENERAL DISCLAIMER                                                       *
 * -------------------------------------------------------------------      *
 * All software and related documentation is provided "AS IS" and without   *
 * warranty or support of any kind and Texas Instruments expressly disclaims*
 * all other warranties, express or implied, including, but not limited to, *
 * the implied warranties of merchantability and fitness for a particular   *
 * purpose.  Under no circumstances shall Texas Instruments be liable for   *
 * any incidental, special or consequential damages that result from the    *
 * use or inability to use the software or related documentation, even if   *
 * Texas Instruments has been advised of the liability.                     *
 ****************************************************************************
Example to show memory access and interrupt throughput HyperLink
In this example, DSP1's memory are mapped to DSP0 through HyperLink 
so, DSP0 accesses DSP 1 through the HyperLink memory window just like 
access other memory space. Internal loopback is also supported in this 
example, for this case, DSP0 actually access its own local memory through 
HyperLink memory window.
 ****************************************************************************
 * Written by :                                                             *
 *            Brighton Feng                                                   *
 *            Texas Instruments                                             *
 *            Nov 5, 2011                                                 *
 ***************************************************************************/
#include <C6x.h>
#include <stdio.h>
#include <string.h>
#include "HyperLink_debug.h"
#include "HyperLink_intc.h"
#include "KeyStone_common.h"
#include "common_test.h"
#include "KeyStone_Serdes_init.h"
#include "KeyStone_ddr_init.h"
#include "KeyStone_HyperLink_init.h"

#define HYPERLINK_LOOPBACK_TEST 	1

/*HyperLink test speed at GHz*/
#define HYPERLINK_SPEED_GHZ 	5.0f

/*the base address of the DDR can be accessed through Hyperlink*/
#define DDR_SPACE_ACCESSED_BY_HYPERLINK 	0x88000000
#define HW_EVENT_FOR_INT_TEST 				0

#pragma DATA_SECTION(serdesLinkSetup,".far:HyperLinkInit")
#pragma DATA_SECTION(hyperLink_cfg,".far:HyperLinkInit")
SerdesLinkSetup serdesLinkSetup;
HyperLink_Config hyperLink_cfg;

extern HyperLink_DSP_core_test();
extern HyperLink_edma_test();


void Hyperlink_config(void)
{
	int iPrvId, iSeg;
	
#if HYPERLINK_LOOPBACK_TEST	
	printf("HyperLink internal loopback test at %.3fGHz...\n", 
		HYPERLINK_SPEED_GHZ);
	hyperLink_cfg.loopback_mode = HyperLink_LOOPBACK;
#else
	printf("HyperLink test at %.3fGHz...\n", 
		HYPERLINK_SPEED_GHZ);
	hyperLink_cfg.loopback_mode = HyperLink_LOOPBACK_DISABLE;
#endif

    /*------------------Initialize Hyperlink Serdes-------------------------*/ 
	serdesLinkSetup.linkSpeed_GHz    = HYPERLINK_SPEED_GHZ;
	serdesLinkSetup.testPattern      = SERDES_TEST_DISABLED; 
	serdesLinkSetup.txOutputSwing    = 15; /*0~15 represents between 100 and 850 mVdfpp  */
	serdesLinkSetup.txInvertPolarity = SERDES_TX_NORMAL_POLARITY; 
	serdesLinkSetup.rxAlign          = SERDES_RX_COMMA_ALIGNMENT_ENABLE; 
    serdesLinkSetup.rxCDR            = 5;
	serdesLinkSetup.rxInvertPolarity = SERDES_RX_NORMAL_POLARITY; 
	serdesLinkSetup.rxTermination    = SERDES_RX_TERM_COMMON_POINT_AC_COUPLE; 
	serdesLinkSetup.rxEqualizerConfig= SERDES_RX_EQ_ADAPTIVE; 
	if(hyperLink_cfg.loopback_mode == HyperLink_LOOPBACK)
		serdesLinkSetup.loopBack= SERDES_LOOPBACK_ENABLE;

	/*all Serdes Link use same configuration*/
	hyperLink_cfg.serdes_cfg.linkSetup[0]= &serdesLinkSetup;
	hyperLink_cfg.serdes_cfg.linkSetup[1]= &serdesLinkSetup;
	hyperLink_cfg.serdes_cfg.linkSetup[2]= &serdesLinkSetup;
	hyperLink_cfg.serdes_cfg.linkSetup[3]= &serdesLinkSetup;

	hyperLink_cfg.serdes_cfg.commonSetup.loopBandwidth= SERDES_PLL_LOOP_BAND_MID;

	/*----------------Initialize Hyperlink address map----------------------*/ 
	/*use 28 bits address for TX (256 MB) */
	hyperLink_cfg.address_map.tx_addr_mask = TX_ADDR_MASK_0x0FFFFFFF;

	/*overlay PrivID to higher 4 bits of address for TX*/
	hyperLink_cfg.address_map.tx_priv_id_ovl = TX_PRIVID_OVL_ADDR_31_28;

	/*Select higher 4 bits of address as PrivID for RX*/
	hyperLink_cfg.address_map.rx_priv_id_sel = RX_PRIVID_SEL_ADDR_31_28;

	/*map remote PrviID 0~7 to loccal ID 13*/
	hyperLink_cfg.address_map.rx_priv_id_map[0] = 13;
	hyperLink_cfg.address_map.rx_priv_id_map[1] = 13;
	hyperLink_cfg.address_map.rx_priv_id_map[2] = 13;
	hyperLink_cfg.address_map.rx_priv_id_map[3] = 13;
	hyperLink_cfg.address_map.rx_priv_id_map[4] = 13;
	hyperLink_cfg.address_map.rx_priv_id_map[5] = 13;
	hyperLink_cfg.address_map.rx_priv_id_map[6] = 13;
	hyperLink_cfg.address_map.rx_priv_id_map[7] = 13;

	/*map remote PrviID 8~15 to loccal ID 14*/
	hyperLink_cfg.address_map.rx_priv_id_map[8] = 14;
	hyperLink_cfg.address_map.rx_priv_id_map[9] = 14;
	hyperLink_cfg.address_map.rx_priv_id_map[10] = 14;
	hyperLink_cfg.address_map.rx_priv_id_map[11] = 14;
	hyperLink_cfg.address_map.rx_priv_id_map[12] = 14;
	hyperLink_cfg.address_map.rx_priv_id_map[13] = 14;
	hyperLink_cfg.address_map.rx_priv_id_map[14] = 14;
	hyperLink_cfg.address_map.rx_priv_id_map[15] = 14;

	/*use bit 24~29 (4 bits (24~27) MSB address, 2 bits (28~29) 
	remote PriviID) as index to lookup segment/length table*/
	hyperLink_cfg.address_map.rx_seg_sel = RX_SEG_SEL_ADDR_29_24;

	/*for this test, 28 bits address space (256 MB) is splitted into 16 segments
	segment 0 -> core 0 local memory (same for all remote PrviID)
	segment 1 -> core 1 local memory (same for all remote PrviID)
	segment 2 -> core 2 local memory (same for all remote PrviID)
	......
	segment 8 -> DDR3 (same for all remote PrviID)
	segment 9 -> DDR3+0x01000000 (same for all remote PrviID)
	segment A -> DDR3+0x02000000 (same for all remote PrviID)
	segment B -> DDR3+0x03000000 (same for all remote PrviID)
	segment C -> SL2 (same for all remote PrviID)
	segment D -> DDR3+0x04000000+(PrviID[1:0]*3+0)*0x01000000
	segment E -> DDR3+0x04000000+(PrviID[1:0]*3+1)*0x01000000
	segment F -> DDR3+0x04000000+(PrviID[1:0]*3+2)*0x01000000

	Since the Hyperlink memory map window start at 0x40000000, 
	the memory map on a remote master is:
	0x40800000 	Core 0 LL2
	0x41800000 	Core 1 LL2
	0x42800000 	Core 2 LL2
	......
	0x48000000~0x4BFFFFFF 	DDR3
	0x4C000000 	SL2
	0x4D000000~0x4FFFFFFF 	DDR3 (different master may access different section)
	*/

	/*map local memory into the same segments for all PrivID (remote masters)*/
	for(iSeg= 0; iSeg<8; iSeg++)
	for(iPrvId=0; iPrvId<4; iPrvId++)
	{
		hyperLink_cfg.address_map.rx_addr_segs[(iPrvId<<4)|iSeg].Seg_Base_Addr= 
			0x10000000+iSeg*0x01000000;
		hyperLink_cfg.address_map.rx_addr_segs[(iPrvId<<4)|iSeg].Seg_Length= 
			RX_SEG_LEN_0x0_0100_0000;
	}

	/*map a part of DDR3 into the same segments for all PrvID (remote masters)*/
	for(iSeg= 8; iSeg<0xC; iSeg++)
	for(iPrvId=0; iPrvId<4; iPrvId++)
	{
		hyperLink_cfg.address_map.rx_addr_segs[(iPrvId<<4)|iSeg].Seg_Base_Addr= 
			DDR_SPACE_ACCESSED_BY_HYPERLINK+(iSeg-8)*0x01000000;
		hyperLink_cfg.address_map.rx_addr_segs[(iPrvId<<4)|iSeg].Seg_Length= 
			RX_SEG_LEN_0x0_0100_0000;
	}

	/*map SL2 into same segement for all PrvID (remote masters)*/
	for(iPrvId=0; iPrvId<4; iPrvId++)
	{
		hyperLink_cfg.address_map.rx_addr_segs[(iPrvId<<4)|0xC].Seg_Base_Addr= 
			0x0C000000;
		hyperLink_cfg.address_map.rx_addr_segs[(iPrvId<<4)|0xC].Seg_Length= 
			RX_SEG_LEN_0x0_0100_0000;
	}

	/*map different DDR3 sections into the segements 
	of different PrvID (remote masters)*/
	for(iPrvId=0; iPrvId<4; iPrvId++)
	for(iSeg= 0xD; iSeg<=0xF; iSeg++)
	{
		hyperLink_cfg.address_map.rx_addr_segs[(iPrvId<<4)|iSeg].Seg_Base_Addr= 
			DDR_SPACE_ACCESSED_BY_HYPERLINK+0x04000000+(iPrvId*3+iSeg-0xD)*0x01000000;
		hyperLink_cfg.address_map.rx_addr_segs[(iPrvId<<4)|iSeg].Seg_Length= 
			RX_SEG_LEN_0x0_0100_0000;
	}

    /*------------------Initialize Hyperlink interrupt----------------------*/ 
	/*map Hyperlink error/status interrupt to interrupt vector 0*/
    hyperLink_cfg.interrupt_cfg.sts_int_enable = 1;
    hyperLink_cfg.interrupt_cfg.sts_int_vec= 0;

	/*interrupt to remote DSP to interrupt vector 1*/
    hyperLink_cfg.interrupt_cfg.int_event_cntl[HW_EVENT_FOR_INT_TEST].si_en = 1;
    hyperLink_cfg.interrupt_cfg.int_event_cntl[HW_EVENT_FOR_INT_TEST].mps = 0;
    hyperLink_cfg.interrupt_cfg.int_event_cntl[HW_EVENT_FOR_INT_TEST].vector = 1;

	/*generate interrupt packet to remote DSP when local interrupt event happens*/
    hyperLink_cfg.interrupt_cfg.int_local= 0;
	/*route interrupt packet from remote DSP to interrupt pending register*/
    hyperLink_cfg.interrupt_cfg.int2cfg = 1;
#if 1
	if(0!=KeyStone_Get_DSP_Number()) //for debug
	{
		TSC_delay_ms(3000);
	}
#endif	
    KeyStone_HyperLink_Init(&hyperLink_cfg);

}

/*simple memory test to verfiy basic function of HyperLink */
unsigned int HyperLink_Mem_Test(unsigned int uiStartAddress,
	unsigned int uiTotalByteCount, unsigned int uiFillByteCount)
{
	unsigned int uiFailCount;
	
	uiFailCount = Memory_quick_test(uiStartAddress, uiTotalByteCount, 
		uiFillByteCount, 256);

	if(0==uiFailCount)
		printf("HyperLink memory test passed at address 0x%x\n",
			uiStartAddress);
			
    return uiFailCount; 
}

void HyperLink_integrity_Test()
{
	/*Core 1 LL2*/
	HyperLink_Mem_Test(0x41800000, 0x0080000, 0x10000);

	/*DDR*/
	HyperLink_Mem_Test(0x48000000, 0x1000000, 0x10000);

	/*SL2*/
	HyperLink_Mem_Test(0x4C100000, 0x0100000, 0x10000);
}

/*this interrupt test is done in loopback mode,
a hardware event is trigger manually, a interrupt packet is generated and 
loopback to this DSP and trigger interrupt to the DSP core. The latency 
between trigger and the entry of the ISR are measured*/
void HyperLink_Interrupt_Test()
{
	Uint32 uiStartTSC= TSCL;

	/*manually trigger the hardware event, which will generate 
	interrupt packet to remote side*/
	gpHyperLinkRegs->SW_INT= HW_EVENT_FOR_INT_TEST;

	/*the interrupt packet is loop back to this DSP and trigger 
	interrupt to this DSP core, here waiting for the interrupt*/
	asm(" IDLE");

	/*the time stamp at the entry of the interrupt is recorded in the ISR*/

	printf("Hyperlink interrupt latency is %d cycles\n", 
		HyperLinkIntTSCL- uiStartTSC);
}

void main()
{
	int i;
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


#if 1 	/*for debug only. Without these code you need reset DSP before your re-run the program*/
	if(TCI6614_EVM==gDSP_board_type)
	{
		//reset HyperLink through PSC
		KeyStone_disable_PSC_module(CSL_PSC_PD_ALWAYSON, 3);
	}
	else
	{
		//reset HyperLink through PSC
		KeyStone_disable_PSC_module(5, 12);
		KeyStone_disable_PSC_Power_Domain(5);
	}

#endif

	/*make other space non-cacheable and non-prefetchable*/
	for(i=24; i<128; i++)
		gpCGEM_regs->MAR[i]=0;	

	/*make DDR cacheable and prefetchable*/
	for(i=128; i<256; i++)
		gpCGEM_regs->MAR[i]=1|(1<<CSL_CGEM_MAR0_PFX_SHIFT);	

	memset(&serdesLinkSetup, 0, sizeof(serdesLinkSetup));
	memset(&hyperLink_cfg, 0, sizeof(hyperLink_cfg));

	if(C6678_EVM==gDSP_board_type)
	{
		//DSP core speed: 100*10/1=1000MHz
		KeyStone_main_PLL_init(100, 10, 1); 
		hyperLink_cfg.serdes_cfg.commonSetup.inputRefClock_MHz = 312.5;
	}
	else if(TCI6614_EVM==gDSP_board_type
		||DUAL_NYQUIST_EVM==gDSP_board_type
		||C6670_EVM==gDSP_board_type)
	{
		//DSP core speed: 122.88*236/29= 999.9889655MHz
		KeyStone_main_PLL_init(122.88, 236, 29);

		if(C6670_EVM==gDSP_board_type)
			hyperLink_cfg.serdes_cfg.commonSetup.inputRefClock_MHz = 250;
		else
		{
			hyperLink_cfg.serdes_cfg.commonSetup.inputRefClock_MHz = 312.5;
		}
	}
	else
	{
		puts("Unknown DSP board type!");
		return;
	}
	//DDR init 66.66667*20/1= 1333
	KeyStone_DDR_init (66.66667, 20, 1, NULL);

    Hyperlink_config();//hyperlink configuration

	if(0==KeyStone_Get_DSP_Number())
	{
		/*integrity test*/
	    HyperLink_integrity_Test(); 

		/*interrupt test is done in loopback mode*/
		if(hyperLink_cfg.loopback_mode == HyperLink_LOOPBACK)
		{
			HyperLink_Interrupts_Init();
		    HyperLink_Interrupt_Test();
		}

		/*performance test*/
		HyperLink_DSP_core_test();

		//while(1) /*continously transfer data over HyperLink for eye-diagram test*/
		HyperLink_edma_test();
	}
	else
	{
		puts("standby for access by HyperLink...");
		while(1) asm(" nop 5");
	}
	
	print_HyperLink_status();

	//soft reset HyperLink after the test completes
	KeyStone_HyperLink_soft_reset();	

}

