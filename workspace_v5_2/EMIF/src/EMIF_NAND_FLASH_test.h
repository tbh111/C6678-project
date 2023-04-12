/****************************************************************************\
 *           Copyright (C) 2012 Texas Instruments Incorporated.             *
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
This example shows NAND FLASH test through EMIF
verfied on NAND512R3A2DZA6E on Shannon EVM
          MT29F1G08ABBDAHC on Appleton EVM.
 ****************************************************************************
 * Created by :                                                             *
 *            Brighton Feng                                                 *
 *            Texas Instruments                                             * 
 *            March 9, 2012                                                 *
***************************************************************************/

#ifndef _EMIF_NAND_FLASH_TEST_H_
#define _EMIF_NAND_FLASH_TEST_H_

extern void NAND_FLASH_test(unsigned int uiBufAddress,
	unsigned int uiBufByteSize);

#endif 

