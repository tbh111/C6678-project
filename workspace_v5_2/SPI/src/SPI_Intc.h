/*  ============================================================================
 *     Copyright (C) 2013 Texas Instruments Incorporated.       *
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
Interrupts configuration for SPI
 * =============================================================================
 *  Revision History
 *  ===============
 *  23-July-2013 Cheng  file created
 *  19-October-2013 Brighton  update to support multiple ports
 * =============================================================================
 */
#ifndef _SPI_INTC_H_
#define _SPI_INTC_H_

#include <c6x.h>
#include <csl_cpintc.h>
#include "KeyStone_common.h"
#include "KeyStone_SPI_init_drv.h"

extern void SPI_Interrupts_Init(void);

#endif
