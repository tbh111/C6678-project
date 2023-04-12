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
 * Created by :                                                             *
 *            Brighton Feng                                                 *
 *            Texas Instruments                                             * 
 *            Nov 5, 2011                                                 *
\***************************************************************************/
#ifndef _HyperLink_DEBUG_H
#define _HyperLink_DEBUG_H
#include <tistdtypes.h>
#include <cslr_vusr.h>

typedef struct  {
    volatile Uint32 STS;
    volatile Uint32 ECC_CNTR;
    volatile Uint32 LINK_STS;
} CSL_HyperLinkStsRegs;

extern CSL_HyperLinkStsRegs hyperLinkStsRegs;

/*copy HyperLink error status registers to a local data structure for later check*/
extern void copyHyperLinkErrStsRegs();

extern void print_HyperLink_status();

extern void clear_HyperLink_error_status();

#endif
