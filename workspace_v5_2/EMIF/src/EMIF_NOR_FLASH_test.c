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
This example shows NOR FLASH test through EMIF
 ****************************************************************************
 * Created by :                                                             *
 *            Brighton Feng                                                 *
 *            Texas Instruments                                             * 
 *            Jan 18, 2012                                                 *
***************************************************************************/
#include "stdio.h"
#include <tistdtypes.h>
#include <cslr_device.h>
#include "flash_nor.h"
#include "flash.h"
#include "EMIF_NOR_FLASH_test.h"
#include "EMIF_FLASH_mem_test.h"
#include "KeyStone_common.h"

NOR_InfoObj gNorInfo =
{
	CSL_EMIF16_data_REGS,/*flashBase;                  */  
	FLASH_BUS_WIDTH_2_BYTES,/*busWidth;                   */  
	FLASH_BUS_WIDTH_2_BYTES,/*chipOperatingWidth;         */  
	FLASH_BUS_WIDTH_2_BYTES/*maxTotalWidth;              */  
};

int NOR_FLASH_erase_blocks(
	unsigned int flashAddress, unsigned int byteNumber)
{
	// Erase the NOR flash to accomadate the file size
	if (E_FAIL==NOR_erase( &gNorInfo, flashAddress, byteNumber ))
	{
		puts("\tERROR: Erasing NOR failed.");
		return E_FAIL;
	}
	return E_PASS;
}

int NOR_FLASH_write_block(unsigned int srcDatAddress, 
	unsigned int dstFlashAddress, unsigned int byteNumber)
{
	// Write the application data to the flash
	if (E_FAIL==NOR_writeBytes( &gNorInfo, dstFlashAddress, byteNumber, srcDatAddress))
	{
		puts("\tERROR: Writing NOR failed.");
		return E_FAIL;
	}
	return E_PASS;
}

void NOR_FLASH_test(unsigned int uiBufAddress,
	unsigned int uiBufByteSize)
{
	if (NOR_init(&gNorInfo))
	{
		puts("\tERROR: NOR Initialization failed." );
		return;
	}

	//call back function for FLASH erase
	FLASH_erase_blocks= &NOR_FLASH_erase_blocks;

	//call back function for FLASH write
	FLASH_write_block= &NOR_FLASH_write_block;

	//call back function for FLASH read
	FLASH_read_block= NULL; /*NOR FLASH can be read directly*/

	FLASH_MEM_Test(gNorInfo.flashBase+ gNorInfo.flashSize*7/8, 
		gNorInfo.flashBase+ gNorInfo.flashSize,
		uiBufAddress, uiBufByteSize);
	
}

