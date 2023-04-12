/*  ============================================================================
 *     Copyright (C) 2013 Texas Instruments Incorporated.       *
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
EDMA test for SPI
 * =============================================================================
 *  Revision History
 *  ===============
 *  23-July-2013 Cheng  file created
 *  19-October-2013 Brighton  add more test cases
 * =============================================================================
 */
#ifndef _SPI_EDMA_TEST_H_
#define _SPI_EDMA_TEST_H_

#include <tistdtypes.h>
#include "KeyStone_SPI_Init_drv.h"
#include "KeyStone_common.h"
#include <stdio.h>
#include <csl_edma3.h>



extern SPI_Data_Format EdmaDataFormat;
extern SPI_Transfer_Param EdmaTransferParam;

#define SPI_EDMA_TEST_BUF_SIZE 	(64*1024)

extern Uint16 spiEdmaRxBuf[];
extern Uint16 spiEdmaTxBuf[];
extern Uint32 SPIDAT1_EdmaBuf[];

extern void SPI_EDMA_Init();
extern void SPI_EDMA_test();

#endif
