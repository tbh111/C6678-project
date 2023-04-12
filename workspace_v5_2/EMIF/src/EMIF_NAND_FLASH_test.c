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
#include "stdio.h"
#include <tistdtypes.h>
#include "flash_nand.h"
#include "flash.h"
#include "EMIF_NAND_FLASH_test.h"
#include "EMIF_FLASH_mem_test.h"
#include "KeyStone_common.h"

/*for ONFI compliant NAND FLASH, some parameters will be get from 
parameter table in the device*/
NAND_InfoObj gNandInfo =
{/*configuration for NAND512R3A2DZA6E*/
	0,	//CSOffset;       
	FLASH_BUS_WIDTH_1_BYTE,	//busWidth;       
	4096,	//numBlocks;         
	32,		//pagesPerBlock;     
	512,	//dataBytesPerPage;  
	16,		//spareBytesPerPage;   
	1, 		//numOpsPerPage;     
	512,	//dataBytesPerOp;      
	16, 	//spareBytesPerOp;   
	NAND_NO_ECC 	//ECC_mode;          
};

int NAND_FLASH_erase_blocks(
	unsigned int flashAddress, unsigned int byteNumber)
{
	// Erase the NAND flash to accomadate the file size
	if (E_FAIL==NAND_eraseBlocks( &gNandInfo, 
		flashAddress>>gNandInfo.blkShift, 
		byteNumber>>gNandInfo.blkShift))
	{
		puts("\tERROR: Erasing NAND failed.");
		return E_FAIL;
	}
	return E_PASS;
}
int NAND_FLASH_write_block(unsigned int srcDatAddress, 
	unsigned int dstFlashAddress, unsigned int byteNumber)
{
	Int32 i;
	Uint32 uiNumPage= byteNumber>>gNandInfo.pageShift;
	Uint32 uiPageNum= (dstFlashAddress>>gNandInfo.pageShift)&(gNandInfo.pagesPerBlock-1);
	Uint32 uiBlockNum= dstFlashAddress>>gNandInfo.blkShift;
	for(i=0; i< uiNumPage; i++)
	{
		// Write the data to the flash
		if (E_FAIL==NAND_writePage( &gNandInfo, uiBlockNum, 
			uiPageNum, (Uint8 *)srcDatAddress))
		{
			puts("\tERROR: Writing NAND failed.");
			return E_FAIL;
		}
		srcDatAddress+= gNandInfo.dataBytesPerPage;
		uiPageNum++;
		if(uiPageNum==gNandInfo.pagesPerBlock)
		{
			uiPageNum= 0;
			uiBlockNum++;
		}
	}
	return E_PASS;
}

int NAND_FLASH_read_block(unsigned int dstDatAddress, 
	unsigned int srcFlashAddress, unsigned int byteNumber)
{
	Int32 i;
	Uint32 uiNumPage= byteNumber>>gNandInfo.pageShift;
	Uint32 uiPageNum= (srcFlashAddress>>gNandInfo.pageShift)&(gNandInfo.pagesPerBlock-1);
	Uint32 uiBlockNum= srcFlashAddress>>gNandInfo.blkShift;
	for(i=0; i< uiNumPage; i++)
	{
		// read the data from the flash
		if (E_FAIL==NAND_readPage( &gNandInfo, uiBlockNum, 
			uiPageNum, (Uint8 *)dstDatAddress))
		{
			puts("\tERROR: Reading NAND failed.");
			return E_FAIL;
		}
		dstDatAddress+= gNandInfo.dataBytesPerPage;
		uiPageNum++;
		if(uiPageNum==gNandInfo.pagesPerBlock)
		{
			uiPageNum= 0;
			uiBlockNum++;
		}
	}
	return E_PASS;
}

void NAND_FLASH_test(unsigned int uiBufAddress,
	unsigned int uiBufByteSize)
{
	int i;
	Uint32 testBlocksPerIteration, blockSize, testStartBlock;
	
	if (NAND_init(&gNandInfo))
	{
		puts("\tERROR: NAND Initialization failed." );
		return;
	}

	//call back function for FLASH erase
	FLASH_erase_blocks= &NAND_FLASH_erase_blocks;

	//call back function for FLASH write
	FLASH_write_block= &NAND_FLASH_write_block;

	//call back function for FLASH read
	FLASH_read_block= &NAND_FLASH_read_block;

	testBlocksPerIteration= 16;
	//testStartBlock= gNandInfo.numBlocks- testBlocksPerIteration;
	//testStartBlock= gNandInfo.numBlocks/2;
	testStartBlock= gNandInfo.numBlocks*7/8;
	blockSize= gNandInfo.pagesPerBlock*gNandInfo.dataBytesPerPage;
	for(i=testStartBlock; i<gNandInfo.numBlocks; i+=testBlocksPerIteration)
	{
		FLASH_MEM_Test(i*blockSize, (i+testBlocksPerIteration)*blockSize,
			uiBufAddress, uiBufByteSize);
	}
}

