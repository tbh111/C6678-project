/* --------------------------------------------------------------------------
  FILE      : nand.c                                                   
  PURPOSE   : NAND driver file
  PROJECT   : DaVinci User Boot-Loader and Flasher
  AUTHOR    : Daniel Allred
  DESC      : Generic NAND driver file for DaVinci EMIFA peripheral
-------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------
  HISTORY
    v1.00 - DJA - 02-Nov-2007
      Initial release
    v1.10 - DJA & SG - 15-Jan-2008
      Revision to add support for 4-bit ECC used for MLC NAND on DM355 EVM
    v1.2 - Brighton Feng - 6-Mar-2012
      Update for C66x DSP
-------------------------------------------------------------------------- */
#include <stdio.h>

// General type include
#include <tistdtypes.h>
#include <cslr_device.h>

// This module's header file 
#include "flash_nand.h"

// Device specific CSL


// Misc. utility function include
#include "flash.h"

// Project specific debug functionality
#include "KeyStone_EMIF16_init.h"
#include "KeyStone_common.h"


/************************************************************
* Explicit External Declarations                            *
************************************************************/


/************************************************************
* Local Macro Declarations                                  *
************************************************************/


/************************************************************
* Local Function Declarations                               *
************************************************************/

static void LOCAL_flashWriteAddr (NAND_InfoHandle hNandInfo, Uint32 addr);
static void LOCAL_flashWriteCmd (NAND_InfoHandle hNandInfo, Uint32 cmd);
static void LOCAL_flashWriteBytes (NAND_InfoHandle hNandInfo, void *pSrc, Uint32 numBytes);
static void LOCAL_flashWriteAddrCycles(NAND_InfoHandle hNandInfo, Uint32 block, Uint32 page);
static void LOCAL_flashWriteAddrBytes(NAND_InfoHandle hNandInfo, Uint32 numAddrBytes, Uint32 addr);
static void LOCAL_flashWriteRowAddrBytes(NAND_InfoHandle hNandInfo, Uint32 block, Uint32 page);
static void LOCAL_flashWriteData(NAND_InfoHandle hNandInfo, Uint32 offset, Uint32 data);
static Uint32 LOCAL_flashReadData(NAND_InfoHandle hNandInfo);
static void LOCAL_flashReadBytes(NAND_InfoHandle hNandInfo, void *pDest, Uint32 numBytes);
static Uint32 LOCAL_flashGetDetails(NAND_InfoHandle hNandInfo);

// ECC functionality
static void LOCAL_flashECCStart (NAND_InfoHandle hNandInfo);
static void LOCAL_flashECCStopAndRead(NAND_InfoHandle hNandInfo, Uint32 *eccValue);
static Uint32 LOCAL_flashECCCorrection(NAND_InfoHandle hNandInfo, Uint32 *ECCold, Uint32 *ECCnew, Uint8 *data);
static void  LOCAL_flashECCManipulate(NAND_InfoHandle hNandInfo, Uint32* in, Uint32* out, NAND_ECCManipulateMethods method);

// Wait for ready signal seen at NANDFSCR
static Uint32 LOCAL_flashWaitForRdy(Uint32 timeout);

// Wait for status result from device to read good */
static Uint32 LOCAL_flashWaitForStatus(NAND_InfoHandle hNandInfo, Uint32 timeout);


/************************************************************
* Local Variable Definitions                                *
************************************************************/


/************************************************************
* Global Variable Definitions                               *
************************************************************/


/************************************************************
* Global Function Definitions                               *
************************************************************/

// Initialze NAND interface and find the details of the NAND used
Uint32 NAND_init(NAND_InfoHandle hNandInfo)
{
	puts("Initializing NAND flash...");

	TSCL= 0; 	//enable TSCL for timeout checking

	hNandInfo->flashBase = CSL_EMIF16_data_REGS + 
		(DEVICE_EMIF_INTER_CE_REGION_SIZE * hNandInfo->CSOffset);

	// NAND enable for CSx
	EMIF16_Regs->NANDFCTL |= (0x1 << (hNandInfo->CSOffset));        

	LOCAL_flashECCStart(hNandInfo);

	// Send reset command to NAND
	LOCAL_flashWriteCmd( hNandInfo, NAND_RESET );
	if ( LOCAL_flashWaitForRdy(NAND_TIMEOUT) != E_PASS )
		return E_FAIL;

	// Get and set device details
	if ( LOCAL_flashGetDetails(hNandInfo) != E_PASS )
		return E_FAIL;

	return E_PASS;
}

// Routine to read a page from NAND
Uint32 NAND_readPage(NAND_InfoHandle hNandInfo, Uint32 block, Uint32 page, Uint8 *dest)
{
	Uint32 i;
	Uint32 eccValue[4], spareValue[4];

	// Write read command
	LOCAL_flashWriteCmd(hNandInfo,NAND_LO_PAGE);

	// Write address bytes
	LOCAL_flashWriteAddrCycles(hNandInfo, block, page);

	// Additional confirm command for big_block devices
	if(hNandInfo->bigBlock)  
		LOCAL_flashWriteCmd(hNandInfo, NAND_READ_30H);

	// Wait for data to be available
	if(LOCAL_flashWaitForRdy(NAND_TIMEOUT) != E_PASS)
		return E_FAIL;

	if(NAND_4_BITS_ECC==hNandInfo->ECC_mode)
	{
		for (i=0; i < hNandInfo->numOpsPerPage; i++)
		{
			LOCAL_flashECCStart(hNandInfo);
			LOCAL_flashReadBytes(hNandInfo, &dest[hNandInfo->dataBytesPerOp*i], hNandInfo->dataBytesPerOp);
			LOCAL_flashECCStopAndRead(hNandInfo, eccValue);

			// Read ECC bytes
			LOCAL_flashReadBytes(hNandInfo, &dest[hNandInfo->dataBytesPerPage+(hNandInfo->spareBytesPerOp*i)], hNandInfo->spareBytesPerOp);
			LOCAL_flashECCManipulate( hNandInfo,
			      (Uint32 *)&dest[hNandInfo->dataBytesPerPage + (hNandInfo->spareBytesPerOp*i) + 6],
			      spareValue,
			      NAND_RBL_8TO10);
			if (LOCAL_flashECCCorrection(hNandInfo,spareValue,eccValue,&dest[hNandInfo->dataBytesPerOp*i]) != E_PASS)
			{
				puts("NAND ECC failure!");
				return E_FAIL;
			}
		}
	}
	else if(NAND_1_BIT_ECC==hNandInfo->ECC_mode)
	{
		for (i=0; i < hNandInfo->numOpsPerPage; i++)
		{
			// Starting the ECC calcualtion
			LOCAL_flashECCStart(hNandInfo);

			// Actually read bytes
			LOCAL_flashReadBytes(hNandInfo, &dest[hNandInfo->dataBytesPerOp*i], hNandInfo->dataBytesPerOp);

			// Stop the ECC calculation and get the ECC value
			LOCAL_flashECCStopAndRead(hNandInfo, &eccValue[i]);
		}

		// Check ECCs
		for (i=0; i < hNandInfo->numOpsPerPage; i++)
		{
			LOCAL_flashReadBytes(hNandInfo, (void*)(spareValue), hNandInfo->spareBytesPerOp);
			LOCAL_flashECCManipulate(hNandInfo,
				(Uint32*)(spareValue + hNandInfo->ECCOffset),
				(Uint32*)(spareValue + hNandInfo->ECCOffset),
				NAND_RBL_SWAP);

			// Verify ECC values
			if(eccValue[i] != spareValue[hNandInfo->ECCOffset])
			{
				if (LOCAL_flashECCCorrection(
					hNandInfo,
					&spareValue[hNandInfo->ECCOffset],
					&eccValue[i],
					dest+(i*hNandInfo->dataBytesPerOp) ) != E_PASS)
				{
					puts("NAND ECC failure!");
					return E_FAIL;
				}
			}
		}
	}
	else	/*NAND_NO_ECC*/
	{
		for (i=0; i < hNandInfo->numOpsPerPage; i++)
		{
			// Actually read bytes
			LOCAL_flashReadBytes(hNandInfo, &dest[hNandInfo->dataBytesPerOp*i], hNandInfo->dataBytesPerOp);
		}
	}

	// Return status check result
	return LOCAL_flashWaitForStatus(hNandInfo, NAND_TIMEOUT);
}

// Generic routine to write a page of data to NAND
Uint32 NAND_writePage(NAND_InfoHandle hNandInfo, Uint32 block, Uint32 page, Uint8 *src)
{
	Uint32 eccValue[4];
	Uint32 spareValue[4];
	Uint8 i;

	// Blank bytes
	spareValue[0] = 0xFFFFFFFF;
	spareValue[1] = 0xFFFFFFFF;
	spareValue[2] = 0xFFFFFFFF;
	spareValue[3] = 0xFFFFFFFF;

	// Make sure the NAND page pointer is at start of page
	LOCAL_flashWriteCmd(hNandInfo,NAND_LO_PAGE);

	// Write program command
	LOCAL_flashWriteCmd(hNandInfo, NAND_PGRM_START);

	// Write address bytes
	LOCAL_flashWriteAddrCycles(hNandInfo, block, page);

	if(NAND_4_BITS_ECC==hNandInfo->ECC_mode)
	{
		for (i=0; i<hNandInfo->numOpsPerPage; i++)  
		{
			LOCAL_flashECCStart(hNandInfo);

			LOCAL_flashWriteBytes(hNandInfo, &src[hNandInfo->dataBytesPerOp*i], hNandInfo->dataBytesPerOp);

			LOCAL_flashECCStopAndRead(hNandInfo,eccValue);

			LOCAL_flashECCManipulate(hNandInfo,
				eccValue,
				(Uint32 *)(((Uint8 *)spareValue)+6),
				NAND_RBL_10TO8);

			// Writing 0xFF for 6 bytes in spare area, and the 80-bits of ECC data
			LOCAL_flashWriteBytes(hNandInfo, spareValue, hNandInfo->spareBytesPerOp);
		}
	}
	else if(NAND_1_BIT_ECC==hNandInfo->ECC_mode)
	{
		for (i=0; i<hNandInfo->numOpsPerPage; i++)
		{
			// Starting the ECC calculation
			LOCAL_flashECCStart(hNandInfo);

			// Write the bytes
			LOCAL_flashWriteBytes(hNandInfo, &src[hNandInfo->dataBytesPerOp*i], hNandInfo->dataBytesPerOp);    

			// Stopping the ECC calculation and reading the ECC value
			LOCAL_flashECCStopAndRead(hNandInfo,&eccValue[i]);
		}

		for (i=0; i<hNandInfo->numOpsPerPage; i++)
		{
			// Swap the bytes for how the ROM needs them
			LOCAL_flashECCManipulate(hNandInfo, &(eccValue[i]),&(eccValue[i]),NAND_RBL_SWAP);

			// Place the ECC values where the ROM read routine expects them    
			spareValue[hNandInfo->ECCOffset] = eccValue[i];

			// Actually write the Spare Bytes               
			LOCAL_flashWriteBytes(hNandInfo, (void*)(spareValue), hNandInfo->spareBytesPerOp);
		}
	}   
	else	/*NAND_NO_ECC*/
	{
		for (i=0; i<hNandInfo->numOpsPerPage; i++)
		{
			// Write the bytes
			LOCAL_flashWriteBytes(hNandInfo, &src[hNandInfo->dataBytesPerOp*i], hNandInfo->dataBytesPerOp);    
		}
	}

	// Write program end command
	LOCAL_flashWriteCmd(hNandInfo, NAND_PGRM_END);

	// Wait for the device to be ready
	if (LOCAL_flashWaitForRdy(NAND_TIMEOUT) != E_PASS)
		return E_FAIL;

	// Return status check result  
	return LOCAL_flashWaitForStatus(hNandInfo, NAND_TIMEOUT);
}

// Verify data written by reading and comparing byte for byte
Uint32 NAND_verifyPage(NAND_InfoHandle hNandInfo, Uint32 block, Uint32 page, Uint8* src, Uint8* dest)
{
  Uint32 i;

  if (NAND_readPage(hNandInfo, block, page, dest) != E_PASS)
    return E_FAIL;
    
  for (i=0; i< (hNandInfo->dataBytesPerPage>>2); i++)
  {
    // Check for data read errors
    if ( ((Uint32 *)src)[i] != ((Uint32 *)dest)[i] )
    {
      puts("Data mismatch! Verification failed.");
      return E_FAIL;
    }
  }
  return E_PASS;
}

// Verify data written by reading and comparing byte for byte
Uint32 NAND_verifyBlockErased(NAND_InfoHandle hNandInfo, Uint32 block, Uint8* dest)
{
  Uint32 i,j;
  
  for (j=0; j<hNandInfo->pagesPerBlock; j++)
  {
    if (NAND_readPage(hNandInfo, block, j, dest) != E_PASS)
      return E_FAIL;
    
    for (i=0; i<(hNandInfo->dataBytesPerPage>>2); i++)
    {
      // Check for data read errors
      if (((Uint32 *)dest)[i] += 0xFFFFFFFF)
      {
        printf("Erase verification failed! Block: %d,  page: %d. First Failing Byte: %d\n",
        	block, j, i);
        return E_FAIL;
      }
    }
  }
  return E_PASS;
}

//Global Erase NOR Flash
Uint32 NAND_globalErase(NAND_InfoHandle hNandInfo)
{
  // We don't erase block 0, and possibly some ending blocks reserved for BBT
  return NAND_eraseBlocks( hNandInfo, 1, (hNandInfo->numBlocks - 1 - NAND_NUM_BLOCKS_RESERVED_FOR_BBT) );
}

Bool NAND_isBadBlock(NAND_InfoHandle hNandInfo, Uint32 block)
{
	Uint8 spareData[16];
#if 1 	/*identify bad block*/
	if(hNandInfo->bONFI)
	{
		// Write read command
		LOCAL_flashWriteCmd(hNandInfo,NAND_LO_PAGE);

		// Write address bytes
		LOCAL_flashWriteAddrCycles(hNandInfo, block, 0);

		LOCAL_flashWriteCmd(hNandInfo, NAND_READ_30H);
	}
	else
	{
		// Write read command
		LOCAL_flashWriteCmd(hNandInfo,NAND_EXTRA_PAGE);

		// Write address bytes
		LOCAL_flashWriteAddrCycles(hNandInfo, block, 0);
	}
	
	// Wait for data to be available
	if(LOCAL_flashWaitForRdy(NAND_TIMEOUT) != E_PASS)
		return E_FAIL;

	if(hNandInfo->bONFI)
	{
		// Write read command
		LOCAL_flashWriteCmd(hNandInfo,NAND_READ_05H);

		// Write address bytes
	    LOCAL_flashWriteAddrBytes(hNandInfo, 
	    	hNandInfo->numColAddrBytes, hNandInfo->dataBytesPerPage);

		LOCAL_flashWriteCmd(hNandInfo, NAND_READ_E0H);
	}
	
	LOCAL_flashReadBytes(hNandInfo, spareData, 8);

	if(hNandInfo->bONFI)
	{/*For ONFI FLASH, 0 in first spare byte is bad block mark*/
		if(0==spareData[0])
		{
			printf("block %d is marked as bad!\n", block);
			return TRUE;
		}
	}
	else
	{
		if(0xFF!=spareData[5])
		{/*In NAND512R3A2D, the 6th byte is bad block identification*/
			printf("block %d is marked as bad!\n", block);
			return TRUE;
		}
	}
#endif
	return FALSE;
}

// NAND Flash erase block function
Uint32 NAND_eraseBlocks(NAND_InfoHandle hNandInfo, Uint32 startBlkNum, Uint32 blkCnt)
{  
	Uint32 i;
	Uint32 endBlkNum = startBlkNum + blkCnt - 1;

	// Do bounds checking
	if ( endBlkNum >= hNandInfo->numBlocks )
		return E_FAIL;

	// Output info about what we are doing
	printf("Erasing blocks %d through %d\n", startBlkNum, endBlkNum);

	for (i = 0; i < blkCnt; i++)
	{
		if(NAND_isBadBlock(hNandInfo, (startBlkNum+i)))
			return E_FAIL;
			
		// Start erase command
		LOCAL_flashWriteCmd(hNandInfo, NAND_BERASEC1);

		// Write the row addr bytes only
		LOCAL_flashWriteRowAddrBytes(hNandInfo, (startBlkNum+i), 0);

		// Confirm erase command
		LOCAL_flashWriteCmd(hNandInfo, NAND_BERASEC2);

		// Wait for the device to be ready
		if (LOCAL_flashWaitForRdy(NAND_TIMEOUT) != E_PASS)
			return E_FAIL;

		// verify the op succeeded by reading status from flash
		if (LOCAL_flashWaitForStatus(hNandInfo, NAND_TIMEOUT) != E_PASS)
			return E_FAIL;
	}

	return E_PASS;
}

// NAND Flash unprotect command
Uint32 NAND_unProtectBlocks(NAND_InfoHandle hNandInfo, Uint32 startBlkNum, Uint32 blkCnt)
{
  Uint32 endBlkNum = startBlkNum + blkCnt - 1;

  // Do bounds checking
  if (endBlkNum >= hNandInfo->numBlocks)
    return E_FAIL;

  // Output info about what we are doing
  printf("Unprotecting blocks %d through %d\n", startBlkNum, endBlkNum);

  LOCAL_flashWriteCmd(hNandInfo, NAND_UNLOCK_START);
  LOCAL_flashWriteRowAddrBytes(hNandInfo, startBlkNum, 0);
  
  LOCAL_flashWriteCmd(hNandInfo, NAND_UNLOCK_END);
  LOCAL_flashWriteRowAddrBytes(hNandInfo, endBlkNum, 0);
    
  return E_PASS;
}

// NAND Flash protect command
void NAND_protectBlocks(NAND_InfoHandle hNandInfo)
{
  puts("Protecting the entire NAND flash.");
  LOCAL_flashWriteCmd(hNandInfo, NAND_LOCK);
}


/************************************************************
* Local Function Definitions                                *
************************************************************/

// Generic Low-level NAND access functions
static void LOCAL_flashWriteData(NAND_InfoHandle hNandInfo, Uint32 offset, Uint32 data)
{
	Uint32 addr= hNandInfo->flashBase + offset;
	if (FLASH_BUS_WIDTH_1_BYTE==hNandInfo->busWidth)
		*(Uint8 *)addr = data;
	else /*FLASH_BUS_WIDTH_2_BYTES*/
		*(Uint16 *)addr = (Uint16)data;
}

static void LOCAL_flashWriteCmd (NAND_InfoHandle hNandInfo, Uint32 cmd)
{
  LOCAL_flashWriteData(hNandInfo, NAND_CLE_OFFSET, cmd);
}

static void LOCAL_flashWriteAddr (NAND_InfoHandle hNandInfo, Uint32 addr)
{
  LOCAL_flashWriteData(hNandInfo, NAND_ALE_OFFSET, addr);
}

static void LOCAL_flashWriteBytes(NAND_InfoHandle hNandInfo, void* pSrc, Uint32 numBytes)
{
	Uint32 i;
	Uint16 * uspSrc= (Uint16 *)pSrc;
	Uint8 * ucpSrc= (Uint8 *)pSrc;
	Uint32 destAddr= hNandInfo->flashBase + NAND_DATA_OFFSET;

	if(FLASH_BUS_WIDTH_1_BYTE==hNandInfo->busWidth)
	{
		for(i=0;i<( numBytes );i++)
			*(Uint8 *)destAddr = *ucpSrc++;
	}
	else /*FLASH_BUS_WIDTH_2_BYTES*/
	{
		for(i=0;i<( numBytes >> 1);i++)
			*(Uint16 *)destAddr = *uspSrc++;
	}
}

static void LOCAL_flashWriteAddrCycles(NAND_InfoHandle hNandInfo, Uint32 block, Uint32 page)
{
    LOCAL_flashWriteAddrBytes(hNandInfo, hNandInfo->numColAddrBytes, 0x00000000);
    LOCAL_flashWriteRowAddrBytes(hNandInfo, block, page);
}

static void LOCAL_flashWriteAddrBytes(NAND_InfoHandle hNandInfo, Uint32 numAddrBytes, Uint32 addr)
{    
	Uint32 i, uiAddressMask=0;
	for (i=0; i<numAddrBytes; i++)
	{
		uiAddressMask <<= 8;
		uiAddressMask |= 0xFF;
	}
	addr &= uiAddressMask;

	if (FLASH_BUS_WIDTH_1_BYTE==hNandInfo->busWidth)
		for (i=0; i<numAddrBytes; i++)
		{
			LOCAL_flashWriteAddr(hNandInfo, ( (addr >> (8*i) ) & 0xff) );
		}
	else /*FLASH_BUS_WIDTH_2_BYTES*/
		for (i=0; i<(numAddrBytes+1)/2; i++)
		{
			LOCAL_flashWriteAddr(hNandInfo, ( (addr >> (16*i) ) & 0xffff) );
		}
}

static void LOCAL_flashWriteRowAddrBytes(NAND_InfoHandle hNandInfo, Uint32 block, Uint32 page)
{
    Uint32 row_addr;
  row_addr = (block << (hNandInfo->blkShift - hNandInfo->pageShift)) | page;
  LOCAL_flashWriteAddrBytes(hNandInfo, hNandInfo->numRowAddrBytes, row_addr);
}

static Uint32 LOCAL_flashReadData (NAND_InfoHandle hNandInfo)
{
	Uint32 addr= hNandInfo->flashBase + NAND_DATA_OFFSET;
	if (FLASH_BUS_WIDTH_1_BYTE==hNandInfo->busWidth)
		return *(Uint8 *)addr;
	else /*FLASH_BUS_WIDTH_2_BYTES*/
		return *(Uint16 *)addr;
}

static void LOCAL_flashReadBytes(NAND_InfoHandle hNandInfo, void* pDest, Uint32 numBytes)
{
	Uint32 i;
	Uint32 srcAddr= hNandInfo->flashBase + NAND_DATA_OFFSET;
	Uint16 * uspDest= (Uint16 *)pDest;
	Uint8 * ucpDest= (Uint8 *)pDest;

	if(FLASH_BUS_WIDTH_1_BYTE==hNandInfo->busWidth)
	{
		for(i=0;i<( numBytes );i++)
			*ucpDest++ = *(Uint8 *)srcAddr;
	}
	else /*FLASH_BUS_WIDTH_2_BYTES*/
	{
		for(i=0;i<( numBytes >> 1);i++)
			*uspDest++ = *(Uint16 *)srcAddr;
	}
}

// Poll bit of NANDFSR to indicate ready
static Uint32 LOCAL_flashWaitForRdy(Uint32 timeout)
{
	Uint32 delay;
	Uint32 startTSC= TSCL;

	TSC_delay_us(1); 	/*Delay between Write command to Ready/Busy valid*/
	while( !(EMIF16_Regs->NANDFSR & NAND_NANDFSR_READY))
	{
		delay= TSC_getDelay(startTSC);
		if(delay>timeout)
		{
			puts("Wait for NAND flash ready Timeout!");
			return E_FAIL;
		}
	}
	return E_PASS;
}


// Wait for the status to be ready in NAND register
//      There were some problems reported in DM320 with Ready/Busy pin
//      not working with all NANDs. So this check has also been added.
static Uint32 LOCAL_flashWaitForStatus(NAND_InfoHandle hNandInfo, Uint32 timeout)
{
  volatile Uint32 cnt;
  Uint32 status;
  cnt = timeout;

  do
  {
    LOCAL_flashWriteCmd(hNandInfo,NAND_STATUS);
    status = LOCAL_flashReadData(hNandInfo) & (NAND_STATUS_ERROR | NAND_STATUS_BUSY);
    cnt--;
  }
  while((cnt>0) && !status);

  if(cnt == 0)
  {
    puts("NANDWaitForStatus() Timeout!");
    return E_FAIL;
  }

  return E_PASS;
}

static void LOCAL_flashECCStart (NAND_InfoHandle hNandInfo)
{
	volatile Uint32 temp;
	if(NAND_4_BITS_ECC==hNandInfo->ECC_mode)
	{
		// Write appropriate bit to start ECC calcualtions (bit 12 for four bit ECC)
		EMIF16_Regs->NANDFCTL |= (1<<CSL_EMIF16_NANDFCTL_4BIT_ECC_ST_SHIFT);
	}
	else if(NAND_1_BIT_ECC==hNandInfo->ECC_mode)
	{
		// Write appropriate bit to start ECC calcualtions 
		EMIF16_Regs->NANDFCTL |= (1<<(CSL_EMIF16_NANDFCTL_CE0ECC_SHIFT+ hNandInfo->CSOffset));
	}
	// Flush NANDFCTL write (by reading another CFG register)
	temp = EMIF16_Regs->RCSR;
}

// Read the current ECC calculation and restart process
static void LOCAL_flashECCStopAndRead (NAND_InfoHandle hNandInfo, Uint32 *eccValue)
{
	volatile Uint32 temp;

	// Flush data writes (by reading CS3 data region)
	temp = *((volatile Uint32*)(hNandInfo->flashBase));
	if(NAND_4_BITS_ECC==hNandInfo->ECC_mode)
	{
		// Read and mask appropriate (based on CSn space flash is in) ECC regsiter
		eccValue[0] = (EMIF16_Regs->NANDF4BECC1R & 0x03FF03FF);
		eccValue[1] = (EMIF16_Regs->NANDF4BECC2R & 0x03FF03FF);
		eccValue[2] = (EMIF16_Regs->NANDF4BECC3R & 0x03FF03FF);
		eccValue[3] = (EMIF16_Regs->NANDF4BECC4R & 0x03FF03FF);
	}
	else if(NAND_1_BIT_ECC==hNandInfo->ECC_mode)
	{
		// Read and mask appropriate (based on CSn space flash is in) ECC regsiter
		eccValue[0] = ((Uint32*)(&(EMIF16_Regs->NFECCCE0)))[hNandInfo->CSOffset] & hNandInfo->ECCMask;
	}
}

// Use old (write) and new (read) ECCs to correct errors
static Uint32 LOCAL_flashECCCorrection(NAND_InfoHandle hNandInfo, Uint32 *ECCold, Uint32 *ECCnew, Uint8 *data)
{
	if(NAND_4_BITS_ECC==hNandInfo->ECC_mode)
	{
		Uint8  corrState, status, numE;
		volatile Uint32 temp;
		Int32 i;
		Uint16 addOffset, corrValue;
		Uint16* syndrome10 = (Uint16 *)ECCold;

		volatile Uint32 cnt;

		// Load the syndrome10 (from 7 to 0) values 
		for(i=7;i>=0;i--)
			EMIF16_Regs->NANDF4BECCLR = syndrome10[i];

		// Read the EMIF status and version (dummy call) 
		temp = EMIF16_Regs->RCSR;

		// Check if error is detected
		if( (EMIF16_Regs->NANDF4BECC1R | EMIF16_Regs->NANDF4BECC2R | EMIF16_Regs->NANDF4BECC3R | EMIF16_Regs->NANDF4BECC4R) == 0)
		{
			return E_PASS;
		}

		// Calcuate the address if error detected
		// Set bit13 of NANDFCTL
		EMIF16_Regs->NANDFCTL |= (1<<CSL_EMIF16_NANDFCTL_ADDR_CALC_ST_SHIFT);

		// bit 11:8 gives the corr_state. After correction the states can
		// be 0000, 0001, 0010, 0011. All other states are work-in-progress
		// So, checking for bit 11:10
		cnt = NAND_TIMEOUT;
		do
		{
			status = (EMIF16_Regs->NANDFSR & 0xC00);
			cnt--;
		}
		while((cnt>0) && status);

		// After bit 11:10 become 00
		corrState = (EMIF16_Regs->NANDFSR & CSL_EMIF16_NANDFSR_CORR_STATE_MASK)>>
			CSL_EMIF16_NANDFSR_CORR_STATE_SHIFT;

		if(corrState == 1)
			return E_FAIL;
		else
		{
			// Error detected and address calculated
			// Number of errors corrected 17:16
			numE = (EMIF16_Regs->NANDFSR & CSL_EMIF16_NANDFSR_ERR_NUM_MASK) >> 
				CSL_EMIF16_NANDFSR_ERR_NUM_SHIFT;
			if (numE == 0)
				return E_PASS;
			else
			{
				// bit 9:0
				addOffset = 519 - (EMIF16_Regs->NANDFEA1R & (0x3FF));
				corrValue = EMIF16_Regs->NANDFEV1R & (0x3FF);
				data[addOffset] ^= (Uint8)corrValue;

				if(numE == 1)
					return E_PASS;
				else
				{
					// bit 25:16
					addOffset = 519 - ( (EMIF16_Regs->NANDFEA1R & (0x3FF0000))>>16 );
					corrValue = EMIF16_Regs->NANDFEV1R & (0x3FF);
					data[addOffset] ^= (Uint8)corrValue;        

					if(numE == 2)
						return E_PASS;
					else
					{
						// bit 9:0
						addOffset = 519 - (EMIF16_Regs->NANDFEA2R & (0x3FF));
						corrValue = EMIF16_Regs->NANDFEV2R & (0x3FF);
						data[addOffset] ^= (Uint8)corrValue;

						if (numE == 3)
							return E_PASS;
						else
						{
							// bit 25:16
							addOffset = 519 - ( (EMIF16_Regs->NANDFEA2R & (0x3FF0000)>>16) );
							corrValue = EMIF16_Regs->NANDFEV2R & (0x3FF);
							data[addOffset] ^= (Uint8)corrValue;
						}
					}
				}
			}    
		}
	}
	else if(NAND_1_BIT_ECC==hNandInfo->ECC_mode)
	{
		Uint16 ECCxorVal, byteAddr, bitAddr;

		ECCxorVal = (Uint16)  ((ECCold[0] & 0xFFFF0000) >> 16) ^  // write ECCo
			((ECCold[0] & 0x0000FFFF) >> 0 ) ^  // write ECCe
			((ECCnew[0] & 0xFFFF0000) >> 16) ^  // read ECCo 
			((ECCnew[0] & 0x0000FFFF) >> 0 );   // read ECCe

		if ( ECCxorVal == (0x0000FFFF & hNandInfo->ECCMask) )
		{
			// Single Bit error - can be corrected
			ECCxorVal = (Uint16) ((ECCold[0] & 0xFFFF0000) >> 16) ^ 
				((ECCnew[0] & 0xFFFF0000) >> 16);
			byteAddr = (ECCxorVal >> 3);
			bitAddr = (ECCxorVal & 0x7);
			data[byteAddr] ^= (0x1 << bitAddr);
			return E_PASS;
		}
		else
		{
			// Multiple Bit error - nothing we can do
			return E_FAIL;
		}
	}
	return E_PASS;
}

static void LOCAL_flashECCManipulate(NAND_InfoHandle hNandInfo, Uint32* in, Uint32* out, NAND_ECCManipulateMethods method)
{

	if(NAND_4_BITS_ECC==hNandInfo->ECC_mode)
	{
		if( NAND_RBL_8TO10==method)
		{
			out[0] = ((in[0]&0x000FFC00) << 6) | ((in[0]&0x000003FF)     );
			out[1] = ((in[1]&0x000000FF) <<18) | ((in[0]&0xC0000000) >>14) |
				((in[0]&0x3FF00000) >>20);
			out[2] = ((in[1]&0x0FFC0000) >> 2) | ((in[1]&0x0003FF00) >> 8);
			out[3] = ((in[2]&0x0000FFC0) <<10) | ((in[2]&0x0000003F) << 4) |
				((in[1]&0xF0000000) >>28);
		}
		else if(NAND_RBL_10TO8==method)
		{
			out[0] = ((in[1]&0x00030000) <<14) | ((in[1]&0x000003FF) <<20) |
				((in[0]&0x03FF0000) >> 6) | ((in[0]&0x000003FF)     );
			out[1] = ((in[3]&0x0000000F) <<28) | ((in[2]&0x03FF0000) << 2) |
				((in[2]&0x000003FF) << 8) | ((in[1]&0x03FC0000) >>18);
			out[2] = ((out[2]&0xFFFF0000)    ) | ((in[3]&0x03FF0000) >>10) |
				((in[3]&0x000003F0) >> 4);
		}
	}
	else if(NAND_1_BIT_ECC==hNandInfo->ECC_mode)
	{
		if(NAND_RBL_SWAP==method)
		{
			if (FLASH_BUS_WIDTH_1_BYTE==hNandInfo->busWidth)
				*out= _swap4(*in);
			else /*FLASH_BUS_WIDTH_2_BYTES*/
				*out= _packlh2(*in, *in);
		}
	}
}

//Get ONFI parameters if the NAND flash support ONFI
static Uint32 LOCAL_flashGetOnfiParameters(NAND_InfoHandle hNandInfo)
{
	Uint8 parameters[128];
	
	// Issue device read ID command
	LOCAL_flashWriteCmd( hNandInfo, NAND_READ_PARAMETER);
	LOCAL_flashWriteAddr( hNandInfo, 0);

	TSC_delay_us(70);

	if ( LOCAL_flashWaitForRdy(NAND_TIMEOUT) != E_PASS )
		return E_FAIL;

	//TSC_delay_us(1);

	LOCAL_flashReadBytes(hNandInfo, parameters, 128);

	//check ONFI Parameter page signature
	if((0x4F==parameters[0])&&(0x4E==parameters[1])
		&&(0x46==parameters[2])&&(0x49==parameters[3]))
	{
		hNandInfo->bONFI = TRUE;
		
		hNandInfo->dataBytesPerPage= (parameters[83]<<24)
			|(parameters[82]<<16)|(parameters[81]<<8)|parameters[80];

		hNandInfo->spareBytesPerPage= (parameters[85]<<8)|parameters[84];

		hNandInfo->pagesPerBlock= (parameters[95]<<24)
			|(parameters[94]<<16)|(parameters[93]<<8)|parameters[92];

		hNandInfo->numBlocks= (parameters[99]<<24)
			|(parameters[98]<<16)|(parameters[97]<<8)|parameters[96];

		hNandInfo->numOpsPerPage= parameters[110];

	}
	else
	{
		hNandInfo->bONFI = FALSE;
		puts("this is not an ONFI compliant device");
	}
	
	return E_PASS;
}

// Get details of the NAND flash used from the id and the table of NAND devices
static Uint32 LOCAL_flashGetDetails(NAND_InfoHandle hNandInfo)
{
	Uint32 manfID,deviceID,j;

	// Issue device read ID command
	LOCAL_flashWriteCmd( hNandInfo, NAND_RDID);
	LOCAL_flashWriteAddr( hNandInfo, NAND_RDIDADD);

	// Read ID bytes
	manfID   = LOCAL_flashReadData( hNandInfo ) & 0xFF;
	deviceID = LOCAL_flashReadData( hNandInfo ) & 0xFF;
	j        = LOCAL_flashReadData( hNandInfo ) & 0xFF;
	j        = LOCAL_flashReadData( hNandInfo ) & 0xFF;

	hNandInfo->manfID = (Uint8) manfID;
	hNandInfo->devID = (Uint8) deviceID;
	printf("Manufacturer ID = %xh, Device ID = %xh\n", manfID, deviceID);

	LOCAL_flashGetOnfiParameters(hNandInfo);

	printf("NAND FLASH size = %d x %d x %d = %d Bytes\n",
		hNandInfo->dataBytesPerPage, hNandInfo->pagesPerBlock, hNandInfo->numBlocks, 
		hNandInfo->dataBytesPerPage*hNandInfo->pagesPerBlock*hNandInfo->numBlocks);
	
	hNandInfo->dataBytesPerOp= 
		hNandInfo->dataBytesPerPage/hNandInfo->numOpsPerPage;

	hNandInfo->spareBytesPerOp= 
		hNandInfo->spareBytesPerPage/hNandInfo->numOpsPerPage;

	// Assign the big_block flag
	hNandInfo->bigBlock = (hNandInfo->dataBytesPerPage>MAX_BYTES_PER_OP)?TRUE:FALSE;

	// Setup address shift values  
	hNandInfo->blkShift = 31-_lmbd(1, hNandInfo->pagesPerBlock);
	hNandInfo->pageShift = 31-_lmbd(1, hNandInfo->dataBytesPerPage);
	hNandInfo->blkShift += hNandInfo->pageShift;

	// Set number of column address bytes needed
	if(hNandInfo->bONFI)
	{
		hNandInfo->numColAddrBytes = (31-_lmbd(1, hNandInfo->dataBytesPerPage+
			hNandInfo->spareBytesPerPage)+7)/8;
	}
	else
		hNandInfo->numColAddrBytes = 1;

	// Set number of row address bytes needed
	hNandInfo->numRowAddrBytes = (31-_lmbd(1, hNandInfo->pagesPerBlock*
		hNandInfo->numBlocks)+7)/8;

	if(NAND_NO_ECC!=hNandInfo->ECC_mode)
	{
		// Set the ECC bit mask
		hNandInfo->ECCMask = 0x00000000;
		for (j = 0; (((hNandInfo->dataBytesPerOp*8)>>j) > 0x1); j++)
		{
			hNandInfo->ECCMask |= (0x00010001<<j);
		}

		hNandInfo->ECCOffset = (hNandInfo->bigBlock)?2:0;
	}
	
	return E_PASS;
}

/***********************************************************
* End file                                                 *
***********************************************************/


