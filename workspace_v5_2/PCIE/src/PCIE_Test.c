/****************************************************************************\
 *     Copyright (C) 2011, 2012, 2013 Texas Instruments Incorporated.       *
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
Example to show memory access and interrupt throughput PCIE
In this example, DSP1's memory are mapped to DSP0 through PCIE  
so, DSP0 accesses DSP 1 through the PCIE memory window just like 
access other memory space. 
To run 2 DSP test, You should run this project on the second core of the second 
DSP firstly, and then the first core of the first DSP
Internal loopback is also supported in this example, for this case, 
DSP0 actually access its own local memory through PCIE memory window.
 ****************************************************************************
 * Written by :                                                             *
 *            Brighton Feng                                                 *
 *            Texas Instruments                                             *
 *            June 23, 2013                                                 *
 *  December 29, 2013 Thomas Yang,  add two DSPs test
 ***************************************************************************/
#include <C6x.h>
#include <stdio.h>
#include <string.h>
#include <csl_xmcAux.h>
#include "KeyStone_common.h"
#include "common_test.h"
#include "KeyStone_DDR_init.h"
#include "KeyStone_PCIE_init_drv.h"
#include "PCIE_intc.h"
#include "PCIE_test.h"

PCIE_Loopback_Mode loop_mode= PCIE_PHY_LOOPBACK;
PCIE_Address_Width address_width= PCIE_ADDRESS_32_BITS;

#define PCIE_PREFETCH_BASE_ADDRESS 	0x80000000
#define PCIE_NONFETCH_BASE_ADDRESS 	0x10000000

/*RC BAR0 directly map to RC's PCIE application registers,
EP may write this space to trigger MSI to RC*/
#define PCIE_RC_BAR0_ADDRESS 		0x01000000 	//should >=0x400000???

//PCIE remote test address
#pragma DATA_SECTION(PcieRemoteTestAddr,".far:PCIEInit")
PCIERemoteTestAddress PcieRemoteTestAddr=
{
	PCIE_OUTBOUND_DATA_WINDOW+0x0000000,//DDR_SRC_ADDR;
	PCIE_OUTBOUND_DATA_WINDOW+0x1000000,//DDR_DST_ADDR;
	PCIE_OUTBOUND_DATA_WINDOW+0x4000000,//SL2_SRC_ADDR;
	PCIE_OUTBOUND_DATA_WINDOW+0x4040000,//SL2_DST_ADDR;
	PCIE_OUTBOUND_DATA_WINDOW+0x4100000,//LL2_SRC_ADDR;
	PCIE_OUTBOUND_DATA_WINDOW+0x4120000,//LL2_DST_ADDR;
};

/*Used for inbound configuration, as inbound offset*/
#pragma DATA_SECTION(memory_regions,".far:PCIEInit")
PCIE_Memory_Region memory_regions[]=
{
	{0x80000000, 64*1024*1024}, //DDR3
	{0x0C100000, 1*1024*1024}, 	//SL2
	{0x11820000, 256*1024} 	//LL2
};

/*outbound PCIE address, only used for 2 DSP test for EP to access RC*/
#pragma DATA_SECTION(EP_OB_PCIE_address,".far:PCIEInit")
unsigned long long EP_OB_PCIE_address[]=
{//EP OB configuration should be done manually
	PCIE_RC_BAR0_ADDRESS,   
	0x0000000080000000,
	0x0000000090000000
	//to do
};

/*Following two buffers are used for two devices test. Please note, they must
be allocated explicitly in link command file of this test project*/
#pragma DATA_ALIGN(uadEpRxBuf, 256);
//put this section at 0x11840000 at EP side
#pragma DATA_SECTION(uadEpRxBuf,"pcie_ep_rcv_buf")
Uint32 uadEpRxBuf[PCIE_BUFSIZE_APP+1];//The last one is for flag

/*To choose EP OB Offset[2]*/
unsigned int PcieEpToRcTestAddr = CSL_PCIE_REGS+0x1120000;
#pragma DATA_ALIGN(gaudRcRxBuf, 256);
//put this section at 0x90120000 at RC side
#pragma DATA_SECTION(gaudRcRxBuf,"pcie_rc_rcv_buf")
Uint32 gaudRcRxBuf[PCIE_BUFSIZE_APP+1];

#pragma DATA_ALIGN(srcBuf, 256);
Uint32 srcBuf[PCIE_BUFSIZE_APP+1];

#pragma DATA_SECTION(prefetch_regions,".far:PCIEInit")
PCIE_Memory_Regions prefetch_regions; 

#pragma DATA_SECTION(nonfetch_regions,".far:PCIEInit")
PCIE_Memory_Regions nonfetch_regions;

#pragma DATA_SECTION(inbound_memory_regions,".far:PCIEInit")
PCIE_Inbound_Memory_Regions inbound_memory_regions;

#pragma DATA_SECTION(outbound_memory_regions,".far:PCIEInit")
PCIE_Outbound_Memory_Regions outbound_memory_regions;

#pragma DATA_SECTION(rc_cfg,".far:PCIEInit")
PCIE_RC_Config rc_cfg;

#pragma DATA_SECTION(PCIE_int_cfg,".far:PCIEInit")
PCIE_Interrupt_Config PCIE_int_cfg;

#pragma DATA_SECTION(remote_cfg_setup,".far:PCIEInit")
PCIE_Remote_CFG_SETUP remote_cfg_setup;

#pragma DATA_SECTION(PCIE_cfg,".far:PCIEInit")
KeyStone_PCIE_Config PCIE_cfg;

extern PCIE_DSP_core_test(PCIERemoteTestAddress* remoteAddr);
extern PCIE_edma_test(PCIERemoteTestAddress* remoteAddr);

/*simple memory test to verfiy basic function of PCIE */
unsigned int PCIE_Mem_Test(unsigned int uiStartAddress,
	unsigned int uiTotalByteCount, unsigned int uiFillByteCount)
{
	unsigned int uiFailCount;
	
	uiFailCount = Memory_quick_test(uiStartAddress, uiTotalByteCount, 
		uiFillByteCount, 128);

	if(0==uiFailCount)
		printf("PCIE memory test passed at address 0x%x\n",
			uiStartAddress);
			
    return uiFailCount; 
}

/*test data transfer between RC and EP. This function execute on RC.
RC writes data to EP, wait for EP sends back the data to RC, verify the data*/
void PCIE_Remoteloopback_Test(PCIERemoteTestAddress * remoteAddr)
{
	Uint32 i;
	Uint8 ucErrorFlag = 0;
	/*Core 1 LL2*/
	//PCIE_Mem_Test(remoteAddr->LL2_DST_ADDR, 0x10000, 1);
    Uint32 *pudDestPCIEAddr;
    
    pudDestPCIEAddr = (Uint32*)remoteAddr->LL2_DST_ADDR;

    /* add dstOffset to pcieBase for data transfer */
	for (i=0; i<PCIE_BUFSIZE_APP; i++)
	{
		gaudRcRxBuf[i] = 0;
		srcBuf[i] = i;
	}
	puts("#########RC starts sent data to EP!#########");
    for (i=0; i<PCIE_BUFSIZE_APP; i++)
    {
      *((volatile Uint32 *)pudDestPCIEAddr + i) = srcBuf[i];
    }

    /* Mark that the buffer is full, so EP can process it */
    *((volatile Uint32 *)pudDestPCIEAddr + PCIE_BUFSIZE_APP) = PCIE_EXAMPLE_BUF_FULL;

    /* Note on cache coherence: Write back is not necessary because pcieBase is in
       peripheral address space instead of physical memory*/

    /* Data sent to EP.
       RC waits for the loopback to be completed and
       receive data back from EP */

    do {

		      /*  Cleanup the prefetch buffer also. */
		      CSL_XMC_invalidatePrefetchBuffer();
		
		      CACHE_invL1d ((void *)gaudRcRxBuf,  PCIE_EXAMPLE_DSTBUF_BYTES, CACHE_FENCE_WAIT);
		      CACHE_invL2  ((void *)gaudRcRxBuf,  PCIE_EXAMPLE_DSTBUF_BYTES, CACHE_FENCE_WAIT);

    } while(gaudRcRxBuf[PCIE_BUFSIZE_APP]!= PCIE_EXAMPLE_BUF_FULL);
    puts ("RC has received echo data from EP!");
    puts("#########RC is verifying the received data!#########");

    /* check all the data */
    for (i=0; i<PCIE_BUFSIZE_APP; i++)
    {
      if(gaudRcRxBuf[i] != srcBuf[i])
      {
	        printf ("Received data = %d\nTransmited data = %d\nIndex = %d.\n\nTest failed.\n",
	        		gaudRcRxBuf[i], srcBuf[i], i);
	        ucErrorFlag = 1;
      }
    }
    if(!ucErrorFlag)
    {
    	puts("All the data are correct, Rc->EP->RC echo test passed!");
  	}	

}

/*test data transfer between RC and EP. This function execute on EP.
Wait for RC writes data to EP, EP sends back the data to RC*/
void PCIE_Remote_Loopback_DataEcho(PCIERemoteTestAddress * remoteAddr)
{

	Uint32  i;
    Uint32 *pudDestPCIEAddr;
    //Uint32 *pudRCOutofRange;
    pudDestPCIEAddr = (Uint32*)PcieEpToRcTestAddr;

	/* EP waits for the data received from RC */
	do {
	  unsigned int key;

	  /* Disable Interrupts */
	  key = _disable_interrupts();

	  /*  Cleanup the prefetch buffer also. */
	  CSL_XMC_invalidatePrefetchBuffer();

	  CACHE_invL1d ((void *)uadEpRxBuf,  PCIE_EXAMPLE_DSTBUF_BYTES, CACHE_FENCE_WAIT);
	  CACHE_invL2  ((void *)uadEpRxBuf,  PCIE_EXAMPLE_DSTBUF_BYTES, CACHE_FENCE_WAIT);

	  /* Reenable Interrupts. */
	  _restore_interrupts(key);

	}	 while(uadEpRxBuf[PCIE_BUFSIZE_APP] != PCIE_EXAMPLE_BUF_FULL);

	puts("EP has received data from RC!");
	/* Loopback to RC what was written in the DST buffer.
	   Write from EP to RC */
	puts("EP is sending received data back to RC....");
	for (i=0; i<PCIE_BUFSIZE_APP; i++)
	{
	  *((volatile uint32_t *)pudDestPCIEAddr + i) = uadEpRxBuf[i];
	}

	/* Mark that the buffer is full, so RC can process it */
	*((volatile uint32_t *)pudDestPCIEAddr + PCIE_BUFSIZE_APP) = PCIE_EXAMPLE_BUF_FULL;

	puts("EP has sent data back to RC, completed the loopback test on EP!");
}

void PCIE_integrity_Test(PCIERemoteTestAddress * remoteAddr)
{
	/*DDR*/
	PCIE_Mem_Test(remoteAddr->DDR_DST_ADDR, 0x800000, 0x10000);

	/*SL2*/
	PCIE_Mem_Test(remoteAddr->SL2_DST_ADDR, 0x040000, 0x10000);

	/*Core 1 LL2*/
	PCIE_Mem_Test(remoteAddr->LL2_DST_ADDR, 0x020000, 0x10000);
}

/*this interrupt test is done in loopback mode,
a MSI is trigger manually, a interrupt packet is generated and 
loopback to this DSP and trigger interrupt to the DSP core. The latency 
between trigger and the entry of the ISR are measured*/
void PCIE_Interrupt_Latency_Test()
{
	Uint32 uiStartTSC= TSCL;

	/*manually trigger MSI, which will generate interrupt packet to remote side.
	For loopback test, the MSI_IRQ in application reister space are mapped 
	through BAR0 to first outbound window*/
	KeyStone_PCIE_generate_MSI(DNUM+8, 
		(Uint32*)(CSL_PCIE_REGS+((Uint32)&gpPCIE_app_regs->MSI_IRQ)-(Uint32)gpPCIE_app_regs));

	/*the interrupt packet is loop back to this DSP and trigger 
	interrupt to this DSP core, here waiting for the interrupt*/
	asm(" IDLE");

	/*the time stamp at the entry of the interrupt is recorded in the ISR*/
	printf("PCIE interrupt latency is %d cycles\n", PCIE_IntTSCL- uiStartTSC);
}

void PCIE_Test()
{
	/*for this test use DSP core number as DSP number,
	so, the program should be run on core 0 of DSP0 and core 1 of DSP1*/
	if(0==KeyStone_Get_DSP_Number())
	{
		/*integrity test*/
	    PCIE_integrity_Test(&PcieRemoteTestAddr); 

		/*interrupt test is done in loopback mode*/
		if(PCIE_cfg.loop_mode == PCIE_PHY_LOOPBACK)
		{
		    PCIE_Interrupt_Latency_Test();
		}
		else
		{
			/*------test data transfer between RC and EP------
			RC writes data to EP, EP sends back the data to RC, RC verify the recievied data*/

			//handshake: wait for MSI from EP
			while(1)
			{
				if(1 == gudInterruptFlag)
					break;
			}
			gudInterruptFlag = 0;

			PCIE_Remoteloopback_Test(&PcieRemoteTestAddr);
		}

		/*------performance test------*/
		puts("#########RC starts core access performance test!#########");
		PCIE_DSP_core_test(&PcieRemoteTestAddr);

		puts("#########RC starts EDMA performance test!#########");
		PCIE_edma_test(&PcieRemoteTestAddr);


	}
	else
	{
		/*------test data transfer between RC and EP------
		RC writes data to EP, EP sends back the data to RC, RC verify the recievied data*/

		//handshake: send MSI to RC
		puts("#########EP starts MSI interrupt to RC!#########");
		KeyStone_PCIE_generate_MSI(8,//DNUM+8,
			(Uint32*)(CSL_PCIE_REGS+((Uint32)&gpPCIE_app_regs->MSI_IRQ)-(Uint32)gpPCIE_app_regs));

		puts("EP is waiting for receiving data from RC!");
		PCIE_Remote_Loopback_DataEcho(&PcieRemoteTestAddr);

		puts("EP standby for access by PCIE...");
		while(1) asm(" nop 5");
	}
	
	//print_PCIE_status();
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

	/*clear all configuration data structure, make sure unused parameters are 0*/
	memset(&prefetch_regions       , 0, sizeof(prefetch_regions       )); 
	memset(&nonfetch_regions       , 0, sizeof(nonfetch_regions       ));
	memset(&inbound_memory_regions , 0, sizeof(inbound_memory_regions ));
	memset(&outbound_memory_regions, 0, sizeof(outbound_memory_regions));
	memset(&PCIE_int_cfg           , 0, sizeof(PCIE_int_cfg           ));
	memset(&rc_cfg                 , 0, sizeof(rc_cfg                 ));
	memset(&remote_cfg_setup       , 0, sizeof(remote_cfg_setup       ));
	memset(&PCIE_cfg               , 0, sizeof(PCIE_cfg               ));

	PCIE_cfg.serdes_cfg.inputRefClock_MHz = 100;
	PCIE_cfg.serdes_cfg.linkSpeed_GHz = 5.f;
	PCIE_cfg.serdes_cfg.numLanes= 2;

	PCIE_cfg.serdes_cfg.loopBandwidth    = SERDES_PLL_LOOP_BAND_MID;
	PCIE_cfg.serdes_cfg.txInvertPolarity = SERDES_TX_NORMAL_POLARITY; 
	PCIE_cfg.serdes_cfg.rxInvertPolarity = SERDES_RX_NORMAL_POLARITY; 
	PCIE_cfg.serdes_cfg.rxEqualizerConfig= SERDES_RX_EQ_ADAPTIVE; 
    PCIE_cfg.serdes_cfg.rxCDR            = 1;
    PCIE_cfg.serdes_cfg.rxLos            = SERDES_RX_LOS_DISABLE;
	PCIE_cfg.serdes_cfg.rxAlign          = SERDES_RX_ALIGNMENT_DISABLE; 

	if(C6678_EVM==gDSP_board_type)
	{
		//DSP core speed: 100*10/1=1000MHz
		KeyStone_main_PLL_init(100, 10, 1); 
	}
	else if(TCI6614_EVM==gDSP_board_type
		||DUAL_NYQUIST_EVM==gDSP_board_type
		||C6670_EVM==gDSP_board_type)
	{
		//DSP core speed: 122.88*236/29= 999.9889655MHz
		KeyStone_main_PLL_init(122.88, 236, 29);

		if(TCI6614_EVM==gDSP_board_type)
			PCIE_cfg.serdes_cfg.inputRefClock_MHz = 156.25;
	}
	else
	{
		puts("Unknown DSP board type!");
		return;
	}

	//DDR init 66.66667*20/1= 1333
	KeyStone_DDR_init (66.66667, 20, 1, NULL);

	/*make other space non-cacheable and non-prefetchable*/
	for(i=24; i<128; i++)
		gpCGEM_regs->MAR[i]=0;

	/*make DDR cacheable and prefetchable*/
	for(i=128; i<256; i++)
		gpCGEM_regs->MAR[i]=1|(1<<CSL_CGEM_MAR0_PFX_SHIFT);

	PCIE_cfg.loop_mode= loop_mode;
	PCIE_cfg.address_width= address_width;

	/*enable all PCIE interrupt reception*/
	PCIE_int_cfg.MSI_rx_enable_mask= 0xFFFFFFFF;
	PCIE_int_cfg.Err_rx_enable = TRUE;
	PCIE_int_cfg.PMRST_rx_enable =TRUE;
	
	/*number of MSI may generate from this EP*/
	PCIE_int_cfg.number_tx_MSI = PCIE_16_MSI;
	PCIE_cfg.interrupt_cfg= &PCIE_int_cfg;
	
	PCIE_cfg.outbound_memory_regions= &outbound_memory_regions;
	PCIE_cfg.inbound_memory_regions= &inbound_memory_regions;

	outbound_memory_regions.OB_size= PCIE_OB_SIZE_8MB;

	rc_cfg.memory_base= PCIE_NONFETCH_BASE_ADDRESS;
	rc_cfg.memory_limit= PCIE_NONFETCH_BASE_ADDRESS+256*1024*1024-1;
	rc_cfg.prefetch_memory_base= PCIE_PREFETCH_BASE_ADDRESS;
	rc_cfg.prefetch_memory_limit= PCIE_PREFETCH_BASE_ADDRESS+256*1024*1024-1;
	rc_cfg.BAR0_address= PCIE_RC_BAR0_ADDRESS;

	//for loopback test
	if(PCIE_PHY_LOOPBACK==loop_mode)
	{//loopback only support in RC mode
		PCIE_cfg.PcieMode= PCIE_RC_MODE;
		PCIE_cfg.rc_cfg= &rc_cfg;

		/*for loopback test all memories are mapped to one prefeachable BAR
		because RC only has one memory BAR*/
		prefetch_regions.memory_regions= memory_regions;
		prefetch_regions.uiNumRegions= 3;
		prefetch_regions.bPrefetchable= TRUE;
		inbound_memory_regions.prefetch_regions= &prefetch_regions;
		printf("PCIE PHY loopback mode at %.1fGHz.\n", PCIE_cfg.serdes_cfg.linkSpeed_GHz);
	}
	else //for 2 DSP test
	{
		if(0==KeyStone_Get_DSP_Number())
		{//first DSP is the RC
			PCIE_cfg.PcieMode= PCIE_RC_MODE;
			PCIE_cfg.rc_cfg= &rc_cfg;

			/*for this test, two devices are connect directly,
			the bus, device and function number are all 0*/
			remote_cfg_setup.config_type= 0; 	//remote device is EP
			remote_cfg_setup.config_bus= 0;
			remote_cfg_setup.config_device= 0;
			remote_cfg_setup.config_function= 0;
			printf("PCIE normal and RC mode at %.1fGHz, should be running on core0.\n", PCIE_cfg.serdes_cfg.linkSpeed_GHz);
		}
		else
		{//the second DSP is the EP
			PCIE_cfg.PcieMode= PCIE_EP_MODE;

			/*for 2 DSP test, at EP side, DDR are mapped to prefetchable BAR;
			SL2 and LL2 are mapped to nonprefetchable BAR*/
			prefetch_regions.memory_regions= &memory_regions[0];
			prefetch_regions.uiNumRegions= 1;
			prefetch_regions.bPrefetchable= TRUE;
			nonfetch_regions.memory_regions= &memory_regions[1];
			nonfetch_regions.uiNumRegions= 2;
			nonfetch_regions.bPrefetchable= FALSE;
			inbound_memory_regions.prefetch_regions= &prefetch_regions;
			inbound_memory_regions.nonfetch_regions= &nonfetch_regions;

			/*in EP mode, outbound memory regions must be setup manually.
			in RC mode, outbound memory regions is setup via enumeration*/
			outbound_memory_regions.address_offset= EP_OB_PCIE_address;
			outbound_memory_regions.uiNumRegions= sizeof(EP_OB_PCIE_address)/8; 
			printf("PCIE normal and EP mode at %.1fGHz, should be running on core1.\n", PCIE_cfg.serdes_cfg.linkSpeed_GHz);
		}
	}

	//PCIE initialize
	KeyStone_PCIE_Init(&PCIE_cfg);

	/*PCIE MSI allocation for one device.*/
	if(PCIE_PHY_LOOPBACK==loop_mode)
	{
		/*for loopback test, the MSI CAP registers are accessed through local bus. 
		The MSI_IRQ in application reister space are mapped through BAR0 to 
		PCIE_RC_BAR0_ADDRESS*/
		KeyStone_PCIE_RC_MSI_allocate((PCIE_MSI_Regs *)&gpPCIE_EP_regs->MSI_CAP,
			PCIE_RC_BAR0_ADDRESS+((Uint32)&gpPCIE_app_regs->MSI_IRQ)-(Uint32)gpPCIE_app_regs);
	}
	else
	{
		if(0==KeyStone_Get_DSP_Number())
		{//first DSP is the RC

			/*Remote Configuration Transaction Setup,
			select the bus, device and function number of the target*/
			KeyStone_PCIE_remote_CFG_setup(&remote_cfg_setup);

			/*for test between two device, the RC should access MSI CAP register of EP throught its remote
			configuration space, MSI PCIE write address depends on RC's PCIE address*/
			KeyStone_PCIE_RC_MSI_allocate((PCIE_MSI_Regs *)&gpPCIE_remote_EP_regs->MSI_CAP,
				PCIE_RC_BAR0_ADDRESS+((Uint32)&gpPCIE_app_regs->MSI_IRQ)-(Uint32)gpPCIE_app_regs);
		}
	}

	/*PCIE address allocation. setup outbound and inbound address mapping*/
	KeyStone_PCIE_Address_setup(&PCIE_cfg);

	//interrupt route initialization
	PCIE_Interrupts_Init();

	PCIE_Test();

	puts("PCIE test complete.");
}

