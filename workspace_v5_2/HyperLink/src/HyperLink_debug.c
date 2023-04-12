/****************************************************************************\
 *           Copyright (C) 2011, 2012 Texas Instruments Incorporated.             *
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
   ****************************************************************************
This file print HyperLink status and error information for debug
****************************************************************************
 * Created by :                                                             *
 *            Brighton Feng                                                 *
 *            Texas Instruments                                             * 
 *            Nov 5, 2011                                                 *
***************************************************************************/
#include <stdio.h>
#include <c6x.h>
#include "KeyStone_common.h"
#include "HyperLink_debug.h"
#include "KeyStone_HyperLink_Init.h"

/*copy of HyperLink status registers*/
CSL_HyperLinkStsRegs hyperLinkStsRegs;

/*copy HyperLink error status registers to a local data structure for later check*/
void copyHyperLinkErrStsRegs()
{
    hyperLinkStsRegs.STS      = gpHyperLinkRegs->STS     ; 
    hyperLinkStsRegs.ECC_CNTR = gpHyperLinkRegs->ECC_CNTR; 
    hyperLinkStsRegs.LINK_STS = gpHyperLinkRegs->LINK_STS; 
}

/*print HyperLink status, 
copyHyperLinkErrStsRegs() function must be called before this function*/
void print_HyperLink_status()
{
	int i;
	Uint32 uiStatus, uiTxSerdesSts, uiRxSerdesSts;

	/*copy HyperLink error status registers to a local data structure for later check*/
	copyHyperLinkErrStsRegs();

	puts("-------------HyperLink status when test completes------------");
	uiStatus= hyperLinkStsRegs.STS;

	if(uiStatus&CSL_VUSR_STS_SERIAL_HALT_MASK)
		puts("the serial logic is in a halted state due to any of reset, serial_stop, pll_unlock being set.");

	if(uiStatus&CSL_VUSR_STS_PLL_UNLOCK_MASK)
		puts("the SerDes PLL is not locked to the reference clock. This will prevent any serial operations.");

	if(uiStatus&CSL_VUSR_STS_RPEND_MASK)
		puts("a remote operation is currently pending or in flight.");

	if(uiStatus&CSL_VUSR_STS_IFLOW_MASK)
		puts("a flow control enable request has been received");

	if(uiStatus&CSL_VUSR_STS_OFLOW_MASK)
		puts("the internal flow control threshold has been reached.");

	if(uiStatus&CSL_VUSR_STS_RERROR_MASK)
		puts("an ECC error is received from the management interface.");

	if(uiStatus&CSL_VUSR_STS_LERROR_MASK)
		puts("an inbound packet contains an uncorrectable ECC error.");

	if(uiStatus&CSL_VUSR_STS_NFEMPTY3_MASK)
		puts("Slave Command FIFO is not empty");

	if(uiStatus&CSL_VUSR_STS_NFEMPTY2_MASK)
		puts("Slave Data FIFO is not empty.");

	if(uiStatus&CSL_VUSR_STS_NFEMPTY1_MASK)
		puts("Master Command FIFO is not empty");

	if(uiStatus&CSL_VUSR_STS_NFEMPTY0_MASK)
		puts("Master Data FIFO is not empty.");

	if(uiStatus&CSL_VUSR_STS_SPEND_MASK)
		puts("a request has been detected on the Tx VBUSM slave interface.");

	if(uiStatus&CSL_VUSR_STS_MPEND_MASK)
		puts("a request has been asserted on the Rx VBUSM master interface.");

	if(0==(uiStatus&CSL_VUSR_STS_LINK_MASK))
		puts("serial interface initialization sequence has NOT completed successfully.");

	if(hyperLinkStsRegs.ECC_CNTR)
		printf("%d single bit error are corrected, %d double bit error are detected\n",
			((hyperLinkStsRegs.ECC_CNTR&CSL_VUSR_ECC_CNTR_SGL_ERR_COR_MASK)>>
				CSL_VUSR_ECC_CNTR_SGL_ERR_COR_SHIFT),
			((hyperLinkStsRegs.ECC_CNTR&CSL_VUSR_ECC_CNTR_DBL_ERR_DET_MASK)>>
				CSL_VUSR_ECC_CNTR_DBL_ERR_DET_SHIFT));

	uiStatus= hyperLinkStsRegs.LINK_STS;

	if(uiStatus&CSL_VUSR_LINK_STS_TX_RSYNC_MASK)
		puts("the remote device has synced to the transmit training sequence.");

	if(uiStatus&CSL_VUSR_LINK_STS_TXPLSOK_MASK)
		puts("the Tx PLS layer has linked to the remote device.");
		
	if(uiStatus&CSL_VUSR_LINK_STS_RX_LSYNC_MASK)
		puts("the receive has synced to the training sequence.");

	if(uiStatus&CSL_VUSR_LINK_STS_RX_ONE_ID_MASK)
		puts("lane zero has been identified during training");
		
	uiTxSerdesSts= hyperLinkStsRegs.LINK_STS>>CSL_VUSR_LINK_STS_TX_PHY_EN_SHIFT;
	uiRxSerdesSts= hyperLinkStsRegs.LINK_STS>>CSL_VUSR_LINK_STS_RX_PHY_EN_SHIFT;
	for(i=0; i<4; i++)
	{
		if(uiTxSerdesSts&1)
			printf("TX Serdes lane %d is enabled.\n", i);
		uiTxSerdesSts>>=1;

		if(uiRxSerdesSts&1)
			printf("RX Serdes lane %d is enabled.\n", i);
		uiRxSerdesSts>>=1;
	}
}

void clear_HyperLink_error_status()
{
	gpHyperLinkRegs->ECC_CNTR = 0;
}

