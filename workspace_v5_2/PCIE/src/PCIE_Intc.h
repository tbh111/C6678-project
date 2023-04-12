/*  ============================================================================
 *     Copyright (C) 2011, 2012, 2013 Texas Instruments Incorporated.       *
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
Interrupts configuration and ISR for PCIE
 * =============================================================================
 *  Revision History
 *  ===============
 *  August 22, 2013 Brighton Feng file created
 * =============================================================================
 */
#ifndef _PCIE_INTC_H_
#define _PCIE_INTC_H_
#include <tistdtypes.h>

extern Uint32 PCIE_IntTSCL;
extern Uint32 gudInterruptFlag;

extern void PCIE_Interrupts_Init(void);

#endif
