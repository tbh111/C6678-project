/***************************************************************************\
 *       Copyright (C) 2013 Texas Instruments Incorporated.      *
 *                           All Rights Reserved                            *
 *                                                                          *
 * GENERAL DISCLAIMER                                                       *
 * ------------------                                                       *
 * All software and related documentation is provided "AS IS" and without   *
 * warranty or support of any kind and Texas Instruments expressly disclaims*
 * all other warranties, express or implied, including, but not limited to, *
 * the implied warranties of merchantability and fitness for a particular   *
 * purpose.  Under no circumstances shall Texas Instruments be liable for   *
 * any incidental, special or consequential damages that result from the    *
 * use or inability to use the software or related documentation, even if   *
 * Texas Instruments has been advised of the liability.                     *
   **************************************************************************
This file Print GE status and error information
****************************************************************************
 * Created by :                                                             *
 *            Kevin Cai                                                 *
 *            Texas Instruments                                             * 
 *            July 5, 2013                                                 *
  History       :
  1.Date        : Dec 8, 2014
    Author      : Brighton Feng
    Modification: fixed a SGMII_STATUS check bug
\***************************************************************************/
#include <stdio.h>
#include <tistdtypes.h>
#include <csl_cpsgmii.h>
#include <csl_cpsgmiiAux.h>
#include "KeyStone_GE_Init_drv.h"
#include "Keystone_common.h"
#include "GE_PktDMA_Init.h"
#include "GE_debug.h"

char * GE_speed_str[]=
{
    "10Mbps",
    "100Mbps",
    "1000Mbps"
};

char * GE_duplex_str[]=
{
    "Half Duplex",
    "Full Duplex"
};

void print_GE_link_ability(Uint32 uiPort)
{
	CSL_SGMII_ADVABILITY sgmiiAdvAbility;

	CSL_SGMII_getLinkPartnerAdvAbility(uiPort, &sgmiiAdvAbility);
	printf("SGMII%d Link Partner Advertised Ability %s %s\n", uiPort, 
		GE_speed_str[sgmiiAdvAbility.linkSpeed],
		GE_duplex_str[sgmiiAdvAbility.duplexMode]);
}

//print GE statistics
void print_GE_statistics()
{
	int i;

	//update statistics data
	KeyStone_GE_Accumulate_Statistics();

	for(i=0; i< GE_NUM_ETHERNET_PORT; i++)
	{
	    if(statistics[i].RxGoodFrames     )	printf("STATS%c.RxGoodFrames     =%llu\n", 'A'+i, statistics[i].RxGoodFrames     );
	    if(statistics[i].RxBCastFrames    ) printf("STATS%c.RxBCastFrames    =%llu\n", 'A'+i, statistics[i].RxBCastFrames    );
	    if(statistics[i].RxMCastFrames    ) printf("STATS%c.RxMCastFrames    =%llu\n", 'A'+i, statistics[i].RxMCastFrames    );
	    if(statistics[i].RxPauseFrames    ) printf("STATS%c.RxPauseFrames    =%llu\n", 'A'+i, statistics[i].RxPauseFrames    );
	    if(statistics[i].RxCRCErrors      ) printf("STATS%c.RxCRCErrors      =%llu\n", 'A'+i, statistics[i].RxCRCErrors      );
	    if(statistics[i].RxAlignCodeErrors) printf("STATS%c.RxAlignCodeErrors=%llu\n", 'A'+i, statistics[i].RxAlignCodeErrors);
	    if(statistics[i].RxOversized      ) printf("STATS%c.RxOversized      =%llu\n", 'A'+i, statistics[i].RxOversized      );
	    if(statistics[i].RxJabber         ) printf("STATS%c.RxJabber         =%llu\n", 'A'+i, statistics[i].RxJabber         );
	    if(statistics[i].RxUndersized     ) printf("STATS%c.RxUndersized     =%llu\n", 'A'+i, statistics[i].RxUndersized     );
	    if(statistics[i].RxFragments      ) printf("STATS%c.RxFragments      =%llu\n", 'A'+i, statistics[i].RxFragments      );
	    if(statistics[i].reserved         ) printf("STATS%c.reserved         =%llu\n", 'A'+i, statistics[i].reserved         );
	    if(statistics[i].reserved2        ) printf("STATS%c.reserved2        =%llu\n", 'A'+i, statistics[i].reserved2        );
	    if(statistics[i].RxOctets         ) printf("STATS%c.RxOctets         =%llu\n", 'A'+i, statistics[i].RxOctets         );
	    if(statistics[i].TxGoodFrames     ) printf("STATS%c.TxGoodFrames     =%llu\n", 'A'+i, statistics[i].TxGoodFrames     );
	    if(statistics[i].TxBCastFrames    ) printf("STATS%c.TxBCastFrames    =%llu\n", 'A'+i, statistics[i].TxBCastFrames    );
	    if(statistics[i].TxMCastFrames    ) printf("STATS%c.TxMCastFrames    =%llu\n", 'A'+i, statistics[i].TxMCastFrames    );
	    if(statistics[i].TxPauseFrames    ) printf("STATS%c.TxPauseFrames    =%llu\n", 'A'+i, statistics[i].TxPauseFrames    );
	    if(statistics[i].TxDeferred       ) printf("STATS%c.TxDeferred       =%llu\n", 'A'+i, statistics[i].TxDeferred       );
	    if(statistics[i].TxCollision      ) printf("STATS%c.TxCollision      =%llu\n", 'A'+i, statistics[i].TxCollision      );
	    if(statistics[i].TxSingleColl     ) printf("STATS%c.TxSingleColl     =%llu\n", 'A'+i, statistics[i].TxSingleColl     );
	    if(statistics[i].TxMultiColl      ) printf("STATS%c.TxMultiColl      =%llu\n", 'A'+i, statistics[i].TxMultiColl      );
	    if(statistics[i].TxExcessiveColl  ) printf("STATS%c.TxExcessiveColl  =%llu\n", 'A'+i, statistics[i].TxExcessiveColl  );
	    if(statistics[i].TxLateColl       ) printf("STATS%c.TxLateColl       =%llu\n", 'A'+i, statistics[i].TxLateColl       );
	    if(statistics[i].TxUnderrun       ) printf("STATS%c.TxUnderrun       =%llu\n", 'A'+i, statistics[i].TxUnderrun       );
	    if(statistics[i].TxCarrierSLoss   ) printf("STATS%c.TxCarrierSLoss   =%llu\n", 'A'+i, statistics[i].TxCarrierSLoss   );
	    if(statistics[i].TxOctets         ) printf("STATS%c.TxOctets         =%llu\n", 'A'+i, statistics[i].TxOctets         );
	    if(statistics[i].Frame64          ) printf("STATS%c.Frame64          =%llu\n", 'A'+i, statistics[i].Frame64          );
	    if(statistics[i].Frame65t127      ) printf("STATS%c.Frame65t127      =%llu\n", 'A'+i, statistics[i].Frame65t127      );
	    if(statistics[i].Frame128t255     ) printf("STATS%c.Frame128t255     =%llu\n", 'A'+i, statistics[i].Frame128t255     );
	    if(statistics[i].Frame256t511     ) printf("STATS%c.Frame256t511     =%llu\n", 'A'+i, statistics[i].Frame256t511     );
	    if(statistics[i].Frame512t1023    ) printf("STATS%c.Frame512t1023    =%llu\n", 'A'+i, statistics[i].Frame512t1023    );
	    if(statistics[i].Frame1024tUp     ) printf("STATS%c.Frame1024tUp     =%llu\n", 'A'+i, statistics[i].Frame1024tUp     );
	    if(statistics[i].NetOctets        ) printf("STATS%c.NetOctets        =%llu\n", 'A'+i, statistics[i].NetOctets        );
	    if(statistics[i].RxSOFOverruns    ) printf("STATS%c.RxSOFOverruns    =%llu\n", 'A'+i, statistics[i].RxSOFOverruns    );
	    if(statistics[i].RxMOFOverruns    ) printf("STATS%c.RxMOFOverruns    =%llu\n", 'A'+i, statistics[i].RxMOFOverruns    );
	    if(statistics[i].RxDMAOverruns    ) printf("STATS%c.RxDMAOverruns    =%llu\n", 'A'+i, statistics[i].RxDMAOverruns    );
	}	
}

//print GE ethernet port error/status
void print_GE_ethernet_ports_status()
{
	int i;
	Uint32 SGMII_STATUS;
	Uint32 MAC_STATUS, Port_BLK_CNT;

	for(i=0; i< GE_NUM_ETHERNET_PORT; i++)
	{
		if(gpMAC_regs[i]->MACCONTROL_REG&CSL_CPGMAC_SL_MACCONTROL_REG_GMII_EN_MASK)
		{//only print status for enabled port
			//read status from the registers
			SGMII_STATUS      = gpSGMII_regs[i]->STATUS_REG;
			MAC_STATUS        = gpMAC_regs[i]->MACSTATUS_REG;
			Port_BLK_CNT      = gpCPSW_port_regs[i]->P_BLK_CNT_REG;

			if(0==(SGMII_STATUS&CSL_CPSGMII_STATUS_REG_LOCK_MASK))
			{
				printf("SGMII%d SerDes PLL is NOT locked!\n", i);
			}
			if(0==(SGMII_STATUS&CSL_CPSGMII_STATUS_REG_LINK_MASK))
			{
				printf("SGMII%d Link is NOT up!\n", i);
			}

			if(SGMII_STATUS&CSL_CPSGMII_STATUS_REG_MR_AN_COMPLETE_MASK)
			{//only print autonegotiation status when it is complete
				if(SGMII_STATUS&CSL_CPSGMII_STATUS_REG_AN_ERROR_MASK)
				{
					printf("SGMII%d Auto-negotiation error, occurs when halfduplex gigabit is commanded!\n", i);
				}
				print_GE_link_ability(i);
			}

			if(0==(MAC_STATUS&CSL_CPGMAC_SL_MACSTATUS_REG_IDLE_MASK))
			{
				printf("SGMII port %d is NOT in the idle state.\n", i);
			}
			if(0==(MAC_STATUS&CSL_CPGMAC_SL_MACSTATUS_REG_EXT_GIG_MASK))
			{
				printf("SGMII port %d GIG input signal from SGMII is low.\n", i);
			}
			if(0==(MAC_STATUS&CSL_CPGMAC_SL_MACSTATUS_REG_EXT_FULLDUPLEX_MASK))
			{
				printf("SGMII port %d FULLDUPLEX input signal from SGMII is low.\n", i);
			}
			if(MAC_STATUS&CSL_CPGMAC_SL_MACSTATUS_REG_RX_FLOW_ACT_MASK)
			{
				printf("SGMII port %d RX flow control is enabled and triggered.\n", i);
			}
			if(MAC_STATUS&CSL_CPGMAC_SL_MACSTATUS_REG_TX_FLOW_ACT_MASK)
			{
				printf("SGMII port %d TX flow control is enabled and triggered.\n", i);
			}
#if 0
			printf("Ethernet port%d: %d blocks allocated to the FIFO logical transmit queues.\n", 
				i+1, (Port_BLK_CNT&CSL_CPSW_3GF_P_BLK_CNT_REG_TX_BLK_CNT_MASK)>>CSL_CPSW_3GF_P_BLK_CNT_REG_TX_BLK_CNT_SHIFT);
			printf("                %d blocks allocated to the FIFO logical receive queues.\n", 
				(Port_BLK_CNT&CSL_CPSW_3GF_P_BLK_CNT_REG_RX_BLK_CNT_MASK)>>CSL_CPSW_3GF_P_BLK_CNT_REG_RX_BLK_CNT_SHIFT);
#endif
		}
	}
}

//print GE MDIO status
void print_GE_MDIO_status()
{
	if(gpMDIO_regs->CONTROL_REG&CSL_MDIO_CONTROL_REG_ENABLE_MASK)
	{//only print status when MDIO is enabled
		printf("MDIO ALIVE flag: 0x%08x\n", gpMDIO_regs->ALIVE_REG);
		printf("MDIO LINK  flag: 0x%08x\n", gpMDIO_regs->LINK_REG );
	}
}

//print GE status and errors
void print_GE_status_error()
{
	GE_Check_Free_Queues();
	GE_Check_TxRx_Queues();

	print_GE_statistics();
	print_GE_ethernet_ports_status();
	print_GE_MDIO_status();
}



