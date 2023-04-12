/* --------------------------------------------------------------------------
  FILE      : nor.c 				                             	 	        
  PURPOSE   : NOR driver file
  PROJECT   : DaVinci flashing Utilities
  AUTHOR    : Daniel Allred
  DESC	    : Generic NOR driver file for DaVinci EMIFA peripheral
  Update    : Brighton Feng 2012-1-19
  DESC	    : Generic NOR driver file for KeyStone(C66x) EMIFA peripheral
-------------------------------------------------------------------------- */

// General type include
#include <tistdtypes.h>
#include <stdio.h>

// This module's header file  
#include "flash_nor.h"

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
* Local Typedef Declarations                                *
************************************************************/


/************************************************************
* Local Function Declarations                               *
************************************************************/

static void NOR_flashWriteCmd (NOR_InfoHandle hNorInfo, Uint32 blkAddr, Uint32 offset, Uint32 cmd);
static void NOR_flashWriteData(NOR_InfoHandle hNorInfo, Uint32 address, Uint32 data);
static void NOR_flashWriteDatabuffer(NOR_InfoHandle hNorInfo, Uint32* address, void* data, Uint32 numBytes);
static Uint32 NOR_flashVerifyDataBuffer(NOR_InfoHandle hNorInfo, Uint32 address, void* data, Uint32 numBytes);
static Uint32 NOR_flashReadData(NOR_InfoHandle hNorInfo, Uint32 address, Uint32 offset);
static Bool NOR_flashDataIsEqual (NOR_InfoHandle hNorInfo, Uint32 blkAddr, Uint32 offset, Uint32 val);
static Bool NOR_flashIsSetAll (NOR_InfoHandle hNorInfo, Uint32 blkAddr, Uint32 offset, Uint8 mask);
static Bool NOR_flashIsSetSome (NOR_InfoHandle hNorInfo, Uint32 blkAddr, Uint32 offset, Uint8 mask);
static Uint32 NOR_flashReadCFIBytes (NOR_InfoHandle hNorInfo, Uint32 offset, Uint8 numBytes);
static Bool NOR_flashCFIIsEqual (NOR_InfoHandle hNorInfo, Uint32 offset, Uint8 val);

// Flash Identification and discovery
static Uint32   NOR_flashQueryCFI( NOR_InfoHandle hNorInfo );

// Generic commands that will point to either AMD or Intel command set
static Uint32   (* Flash_Write)(NOR_InfoHandle, Uint32, volatile Uint32);
static Uint32   (* Flash_BufferWrite)(NOR_InfoHandle, Uint32, volatile Uint8[], Uint32);
static Uint32   (* Flash_Erase)(NOR_InfoHandle, Uint32);
static Uint32   (* Flash_ID)(NOR_InfoHandle);

// Empty commands for when neither command set is used
static Uint32   Unsupported_Erase(NOR_InfoHandle hNorInfo, Uint32 address);
static Uint32   Unsupported_Write(NOR_InfoHandle hNorInfo, Uint32 address, volatile Uint32 data);
static Uint32   Unsupported_BufferWrite(NOR_InfoHandle hNorInfo, Uint32, volatile Uint8[], Uint32 );
static Uint32   Unsupported_ID(NOR_InfoHandle hNorInfo);

//Intel pointer-mapped commands
static Uint32   Intel_Erase( NOR_InfoHandle hNorInfo, volatile Uint32 blkAddr);
static Uint32   Intel_Write( NOR_InfoHandle hNorInfo, Uint32 address, volatile Uint32 data );
static Uint32   Intel_BufferWrite( NOR_InfoHandle hNorInfo, Uint32 address, volatile Uint8 data[], Uint32 numBytes );
static Uint32   Intel_ID( NOR_InfoHandle hNorInfo );
// Misc. Intel commands
static Uint32   LOCAL_IntelClearLock(NOR_InfoHandle hNorInfo, Uint32 blkAddr);
static Uint32   LOCAL_IntelSetLock(NOR_InfoHandle hNorInfo, Uint32 blkAddr);
static Uint32   LOCAL_IntelLockStatusCheck(NOR_InfoHandle hNorInfo);
static void     LOCAL_IntelSoftReset(NOR_InfoHandle hNorInfo);
static void     LOCAL_IntelClearStatus(NOR_InfoHandle hNorInfo);
static void     LOCAL_IntelWaitForStatusComplete(NOR_InfoHandle hNorInfo);

//AMD pointer-mapped commands
static Uint32   AMD_Erase(NOR_InfoHandle hNorInfo, Uint32 blkAddr);
static Uint32   AMD_Write(NOR_InfoHandle hNorInfo, Uint32 address, volatile Uint32 data );
static Uint32   AMD_BufferWrite(NOR_InfoHandle hNorInfo, Uint32 address, volatile Uint8 data[], Uint32 numBytes );
static Uint32   AMD_ID(NOR_InfoHandle hNorInfo);
// Misc. AMD commands
static void     LOCAL_AMDSoftReset(NOR_InfoHandle hNorInfo);
static void     LOCAL_AMDPrefixCommands(NOR_InfoHandle hNorInfo);
static void     LOCAL_AMDWriteBufAbortReset(NOR_InfoHandle hNorInfo);


/************************************************************
* Local Variable Definitions                                *
************************************************************/


/************************************************************
* Global Variable Definitions                               *
************************************************************/


/************************************************************
* Global Function Definitions                               *
************************************************************/

Uint32 NOR_init(NOR_InfoHandle hNorInfo)
{
    
  // Specify number of devices
  hNorInfo->numberDevices = hNorInfo->busWidth/hNorInfo->chipOperatingWidth;

  // Perform CFI Query
  if (NOR_flashQueryCFI(hNorInfo) == E_FAIL)
  {
    puts("CFI query failed.");
    return E_FAIL;
  }
    
  // Setup function pointers
  puts("NOR Initialization:");
    
  printf("\tCommand Set: ");    
  switch (hNorInfo->commandSet)
  {
    case AMD_BASIC_CMDSET:
    case AMD_EXT_CMDSET:
      Flash_Erase          = &AMD_Erase;
      Flash_BufferWrite    = &AMD_BufferWrite;
      Flash_Write          = &AMD_Write;
      Flash_ID             = &AMD_ID;
      puts("AMD");
      break;
    case INTEL_BASIC_CMDSET:
    case INTEL_EXT_CMDSET:
      Flash_Erase          = &Intel_Erase;
      Flash_BufferWrite    = &Intel_BufferWrite;
      Flash_Write          = &Intel_Write;
      Flash_ID             = &Intel_ID;
      puts("Intel");
      break;
    default:
      Flash_Write          = &Unsupported_Write;
      Flash_BufferWrite    = &Unsupported_BufferWrite;
      Flash_Erase          = &Unsupported_Erase;
      Flash_ID             = &Unsupported_ID;
      puts("Unknown");
      break;
  }
    
  if ( (*Flash_ID)(hNorInfo) != E_PASS)
  {
    puts("NOR ID failed.");
    return E_FAIL;
  }
        
  printf("\tManufacturer: ");
  switch(hNorInfo->manfID)
  {
    case AMD:
      puts("AMD");
      break;
    case FUJITSU:
      puts("FUJITSU");
      break;
    case INTEL:
      puts("INTEL");
      break;
    case MICRON:
      puts("MICRON");
      break;
    case SAMSUNG:
      puts("SAMSUNG");
      break;
    case SHARP:
      puts("SHARP");
      break;
    default:
      puts("Unknown");
      break;
  }
  printf("\t FLASH Size: %dMB\n", hNorInfo->flashSize>>20);
    
  return E_PASS;
}

// Get info on block address and sizes
Uint32 NOR_getBlockInfo(NOR_InfoHandle hNorInfo, Uint32 address, Uint32* blockSize, Uint32* blockAddr)
{
  Int32 i;
  Uint32 currRegionAddr, nextRegionAddr;
        
  currRegionAddr = (Uint32) hNorInfo->flashBase;
  if ((address < currRegionAddr) || (address >= (currRegionAddr+hNorInfo->flashSize)))
  {
    return E_FAIL;
  }
    
  for(i=0; i< (hNorInfo->numberRegions); i++)
  {
    nextRegionAddr = currRegionAddr + (hNorInfo->blockSize[i] * hNorInfo->numberBlocks[i]);
    if ( (currRegionAddr <= address) && (nextRegionAddr > address) )
    {
      *blockSize = hNorInfo->blockSize[i];
      *blockAddr = address & (~((*blockSize) - 1));
      break;
    }
    currRegionAddr = nextRegionAddr;
  }
  return E_PASS;
}

//Global Erase NOR Flash
Uint32 NOR_globalErase(NOR_InfoHandle hNorInfo)
{
  return NOR_erase( hNorInfo, (Uint32) hNorInfo->flashBase, (Uint32) hNorInfo->flashSize );
}

// Erase Flash Block
Uint32 NOR_erase(NOR_InfoHandle hNorInfo, Uint32 start_address, Uint32 size)
{
	volatile Uint32 addr  = start_address;
	volatile Uint32 range = start_address + size;
	Uint32 blockSize, blockAddr;

	puts("Erasing the NOR Flash...");

	while (addr < range)
	{
		if (NOR_getBlockInfo(hNorInfo, addr, &blockSize, &blockAddr) != E_PASS)
		{
			puts("Address out of range");
			return E_FAIL;
		}

		//Increment to the next block
		if ( (*Flash_Erase)(hNorInfo, blockAddr) != E_PASS)
		{
			printf("Erase failure at block address 0x%x\n", blockAddr);
			return E_FAIL;
		}
		addr = blockAddr + blockSize;

		// Show progress
		if((0 == (addr&0xFFFFF))||(addr == range))
			printf("Erased through 0x%x\n", addr);

	}

	return(E_PASS);
}

// NOR_WriteBytes
Uint32 NOR_writeBytes( NOR_InfoHandle hNorInfo, Uint32 writeAddress, Uint32 numBytes, Uint32 readAddress)
{
	Int32   i;
	Uint32  retval = E_PASS;

	if(FLASH_BUS_WIDTH_2_BYTES == hNorInfo->busWidth)
	{
		if (writeAddress & 0x00000001)
		{
			puts("address is not aligned to 2-byte boundary");
			return E_FAIL;
		}

		if (numBytes & 0x00000001)
		{
			puts("number of bytes is not even");
			return E_FAIL;
		}
	}

	while (numBytes > 0)
	{
		if( (numBytes < hNorInfo->bufferSize) || (writeAddress & (hNorInfo->bufferSize-1) ))
		{
			if ((*Flash_Write)(hNorInfo, writeAddress, NOR_flashReadData(hNorInfo,readAddress,0) ) != E_PASS)
			{
				puts("Normal write failed.");
				retval = E_FAIL;
			}
			else
			{
				numBytes     -= hNorInfo->busWidth;
				writeAddress += hNorInfo->busWidth;
				readAddress  += hNorInfo->busWidth;
			}
		}
		else
		{
			// Try to use buffered writes
			if((*Flash_BufferWrite)(hNorInfo, writeAddress, (volatile Uint8 *)readAddress, hNorInfo->bufferSize) == E_PASS)
			{
				numBytes -= hNorInfo->bufferSize;
				writeAddress += hNorInfo->bufferSize;
				readAddress  += hNorInfo->bufferSize;
			}
			else
			{
				puts("Buffered write failed. Trying normal write");
				// Try normal writes as a backup
				for(i = 0; i<hNorInfo->bufferSize; i+= hNorInfo->busWidth)
				{
					if ((*Flash_Write)(hNorInfo, writeAddress, NOR_flashReadData(hNorInfo,readAddress,0) ) != E_PASS)
					{
						puts("Normal write also failed");
						retval = E_FAIL;
						break;
					}
					else
					{
						numBytes     -= hNorInfo->busWidth;
						writeAddress += hNorInfo->busWidth;
						readAddress  += hNorInfo->busWidth;
					}
				}
			}
		}

		// Output status info on the write operation
		if (retval == E_PASS)
		{    
			/*print progress*/
			if  ( (0==(writeAddress & 0xFFFFF)) || (numBytes == 0) )
			{
				printf("NOR Write OK through 0x%x\n", writeAddress);
			}
		}
		else
		{
			puts( "NOR Write Failed...Aborting!");
			return E_FAIL;
		}
	}
	
	return retval;
}


/************************************************************
* Local Function Definitions                                *
************************************************************/

static void NOR_flashWriteCmd (NOR_InfoHandle hNorInfo, Uint32 blkAddr, Uint32 offset, Uint32 cmd)
{
	Uint32 addr= blkAddr + offset*hNorInfo->busWidth;
	if (FLASH_BUS_WIDTH_1_BYTE==hNorInfo->busWidth)
		*(Uint8 *)addr = (Uint8)cmd;
	else /*FLASH_BUS_WIDTH_2_BYTES*/
		*(Uint16 *)addr = (Uint16)cmd;
}

static void NOR_flashWriteData(NOR_InfoHandle hNorInfo, Uint32 address, Uint32 data)
{
	if (FLASH_BUS_WIDTH_1_BYTE==hNorInfo->busWidth)
		*(Uint8 *)address = (Uint8)data;
	else /*FLASH_BUS_WIDTH_2_BYTES*/
		*(Uint16 *)address = (Uint16)data;
}

/* Used only twice */
static void NOR_flashWriteDatabuffer(NOR_InfoHandle hNorInfo, Uint32* address, void* data, Uint32 numBytes)
{
	volatile NOR_Ptr pAddr, pData;
	volatile Uint8* endAddress;

	pData.cp = (volatile Uint8*) data;
	pAddr.cp = (volatile Uint8*) *address;
	endAddress =(volatile Uint8*)((*address)+numBytes);

	while (pAddr.cp < endAddress)
	{
		if (FLASH_BUS_WIDTH_1_BYTE==hNorInfo->busWidth)
			*pAddr.cp++ = *pData.cp++;
		else /*FLASH_BUS_WIDTH_2_BYTES*/
			*pAddr.wp++ = *pData.wp++;
	}

	// Put last data written at start of data buffer - For AMD verification???
	if (FLASH_BUS_WIDTH_1_BYTE==hNorInfo->busWidth)
		*address = (Uint32)(endAddress-1);
	else /*FLASH_BUS_WIDTH_2_BYTES*/
		*address = (Uint32)(endAddress-2);
}

/* Used only once */
static Uint32 NOR_flashVerifyDataBuffer(NOR_InfoHandle hNorInfo, Uint32 address, void* data, Uint32 numBytes)
{
	volatile NOR_Ptr pAddr, pData;
	volatile Uint8* endAddress;

	pData.cp = (volatile Uint8*) data;
	pAddr.cp = (volatile Uint8*) address;
	endAddress =(volatile Uint8*)(address+numBytes);

	while (pAddr.cp < endAddress)
	{
		if (FLASH_BUS_WIDTH_1_BYTE==hNorInfo->busWidth)
		{
			if ( (*pAddr.cp++) != (*pData.cp++) )
			return E_FAIL;
		}
		else /*FLASH_BUS_WIDTH_2_BYTES*/
		{
			if ( (*pAddr.wp++) != (*pData.wp++) )
			return E_FAIL;
		}
	}
	return E_PASS;
}

static Uint32 NOR_flashReadData(NOR_InfoHandle hNorInfo, Uint32 address, Uint32 offset)
{
	Uint32 data;
	Uint32 addr= address + offset*hNorInfo->busWidth;

	if (FLASH_BUS_WIDTH_1_BYTE==hNorInfo->busWidth)
		data = *(Uint8 *)addr;
	else /*FLASH_BUS_WIDTH_2_BYTES*/
		data = *(Uint16 *)addr;

	return data;
}

/* Used only once */
static Bool NOR_flashDataIsEqual (NOR_InfoHandle hNorInfo, Uint32 blkAddr, Uint32 offset, Uint32 val)
{
	Uint32 data;

	TSC_delay_us(100000);
	data = NOR_flashReadData(hNorInfo, blkAddr, offset);

	return (data == val);
}

static Bool NOR_flashIsSetAll (NOR_InfoHandle hNorInfo, Uint32 blkAddr, Uint32 offset, Uint8 mask)
{
	Uint32 retVal;
	Uint32 addr= blkAddr + offset*hNorInfo->busWidth;

	if (FLASH_BUS_WIDTH_1_BYTE==hNorInfo->busWidth)
		retVal = ((((Uint8)mask) & (*(Uint8 *)addr)) == ((Uint8)mask));
	else /*FLASH_BUS_WIDTH_2_BYTES*/
		retVal = ((((Uint16)mask) & (*(Uint16 *)addr)) == ((Uint16)mask));

	return retVal;
}

static Bool NOR_flashIsSetSome (NOR_InfoHandle hNorInfo, Uint32 blkAddr, Uint32 offset, Uint8 mask)
{
	Uint32 retVal;
	Uint32 addr= blkAddr + offset*hNorInfo->busWidth;

	if (FLASH_BUS_WIDTH_1_BYTE==hNorInfo->busWidth)
		retVal = (((Uint8)mask) & (*(Uint8 *)addr));
	else /*FLASH_BUS_WIDTH_2_BYTES*/
		retVal = (((Uint16)mask) & (*(Uint16 *)addr));
		
	return retVal;
}

static Uint32 NOR_flashReadCFIBytes (NOR_InfoHandle hNorInfo, Uint32 offset, Uint8 numBytes)
{
	Int32 i;
	Uint32 addr;
	Uint32 readword;

	readword= 0;

	for (i = 0; i < numBytes; i++)
	{
		addr = hNorInfo->flashBase + hNorInfo->busWidth*(offset+i);
		if (FLASH_BUS_WIDTH_1_BYTE==hNorInfo->busWidth)
			readword |= (*(Uint8 *)addr)<<(8*i);
		else /*FLASH_BUS_WIDTH_2_BYTES*/
			readword |= ((*(Uint16 *)addr)&0xFF)<<(8*i);
	}
	return readword;
}

static Bool NOR_flashCFIIsEqual (NOR_InfoHandle hNorInfo, Uint32 offset, Uint8 val)
{
	Uint32 addr= hNorInfo->flashBase + offset*hNorInfo->busWidth;
	Bool retVal = TRUE;

	if (FLASH_BUS_WIDTH_1_BYTE==hNorInfo->busWidth)
		retVal = (((Uint8)val) == (*(Uint8 *)addr));
	else /*FLASH_BUS_WIDTH_2_BYTES*/
		retVal = (((Uint16)val) == (*(Uint16 *)addr));
		
	return retVal;
}

// Query the chip to check for CFI table and data
static Uint32 NOR_flashQueryCFI( NOR_InfoHandle hNorInfo )
{                
  Int32 i;
  Uint32 blkVal; 
                    
  // Enter the CFI Query mode
  NOR_flashWriteCmd (hNorInfo, hNorInfo->flashBase, 0, CFI_EXIT_CMD);
  NOR_flashWriteCmd (hNorInfo, hNorInfo->flashBase, CFI_QRY_CMD_ADDR, CFI_QRY_CMD);

  // Check for Query QRY values
  if ( NOR_flashCFIIsEqual ( hNorInfo, CFI_Q, 'Q') && 
  NOR_flashCFIIsEqual ( hNorInfo, CFI_R, 'R') && 
  NOR_flashCFIIsEqual ( hNorInfo, CFI_Y, 'Y') )
		{               
    hNorInfo->commandSet = (NOR_CmdSet) (NOR_flashReadCFIBytes(hNorInfo,CFI_CMDSET,2));
    hNorInfo->flashSize = 0x1 << NOR_flashReadCFIBytes(hNorInfo,CFI_DEVICESIZE,1) * hNorInfo->numberDevices;
    hNorInfo->numberRegions = NOR_flashReadCFIBytes(hNorInfo,CFI_NUMBLKREGIONS,1);
    hNorInfo->bufferSize = 0x1 << NOR_flashReadCFIBytes(hNorInfo,CFI_WRITESIZE,2) * hNorInfo->numberDevices;
            
    // Get info on sector sizes in each erase region of device
    for (i = 0;i < hNorInfo->numberRegions; i++)
    {    
      blkVal = NOR_flashReadCFIBytes(hNorInfo,(CFI_BLKREGIONS+i*CFI_BLKREGIONSIZE),4);
      hNorInfo->numberBlocks[i] = (blkVal&0x0000FFFF) + 1;
      hNorInfo->blockSize[i]    = ((blkVal&0xFFFF0000) ? ( ((blkVal>>16)&0xFFFF) * 256) : 128) * hNorInfo->numberDevices;
    }
            
    // Exit CFI mode 
    NOR_flashWriteCmd (hNorInfo,hNorInfo->flashBase, 0, CFI_EXIT_CMD);
		    
    return E_PASS;
  }
    
  NOR_flashWriteCmd (hNorInfo,hNorInfo->flashBase, 0, CFI_EXIT_CMD);   
  return E_FAIL;
}


// ------------------------ Default empty commands ---------------------------

static Uint32 Unsupported_Write( NOR_InfoHandle hNorInfo, Uint32 address, volatile Uint32 data)
{
    return E_FAIL;
}
static Uint32 Unsupported_BufferWrite(NOR_InfoHandle hNorInfo, Uint32 address, volatile Uint8 data[], Uint32 length )
{
    return E_FAIL;
}
static Uint32 Unsupported_Erase(NOR_InfoHandle hNorInfo, Uint32 address)
{
    return E_FAIL;
}
static Uint32 Unsupported_ID(NOR_InfoHandle hNorInfo)
{
    return E_FAIL;
}


// ------------------- Begin of Intel specific commands ----------------------
//ID flash
static Uint32 Intel_ID( NOR_InfoHandle hNorInfo )
{
    // Intel Exit back to read array mode
    LOCAL_IntelSoftReset(hNorInfo);
    
    // Write ID command
    NOR_flashWriteCmd(hNorInfo,hNorInfo->flashBase, 0, INTEL_ID_CMD);
        
    //Read Manufacturer's ID
    hNorInfo->manfID = (NOR_ManfID) NOR_flashReadData(hNorInfo, hNorInfo->flashBase, INTEL_MANFID_ADDR);
    
    // Read Device ID
    hNorInfo->devID1 = (Uint16) (NOR_ManfID) NOR_flashReadData(hNorInfo, hNorInfo->flashBase, INTEL_DEVID_ADDR);
    hNorInfo->devID2 = 0x0000;
        
    // Intel Exit back to read array mode
    LOCAL_IntelSoftReset(hNorInfo); 
    
    return E_PASS;
}

// Reset back to Read array mode
static void LOCAL_IntelSoftReset(NOR_InfoHandle hNorInfo)
{
    // Intel Exit back to read array mode
    NOR_flashWriteCmd(hNorInfo, hNorInfo->flashBase, 0, INTEL_RESET);
}

// Clear status register
static void LOCAL_IntelClearStatus(NOR_InfoHandle hNorInfo)
{
    // Intel clear status
    NOR_flashWriteCmd(hNorInfo, hNorInfo->flashBase,0,INTEL_CLEARSTATUS_CMD);
}

// Remove block write protection
static Uint32 LOCAL_IntelClearLock(NOR_InfoHandle hNorInfo, Uint32 blkAddr)
{

	// Write the Clear Lock Command
    NOR_flashWriteCmd(hNorInfo, blkAddr,0,INTEL_LOCK_CMD0);

    NOR_flashWriteCmd(hNorInfo, blkAddr,0,INTEL_UNLOCK_BLOCK_CMD);

    // Check Status
	return LOCAL_IntelLockStatusCheck(hNorInfo);
}

// Write-protect a block
static Uint32 LOCAL_IntelSetLock(NOR_InfoHandle hNorInfo, Uint32 blkAddr)
{
	// Write the Set Lock Command	
  NOR_flashWriteCmd(hNorInfo, blkAddr,0,INTEL_LOCK_CMD0);            
	
  NOR_flashWriteCmd(hNorInfo, blkAddr,0,INTEL_LOCK_BLOCK_CMD);

  // Check Status
  return LOCAL_IntelLockStatusCheck(hNorInfo);
}

static void LOCAL_IntelWaitForStatusComplete(NOR_InfoHandle hNorInfo)
{
    while ( !NOR_flashIsSetAll(hNorInfo, hNorInfo->flashBase, 0, (1<<7)) );
}

static Uint32 LOCAL_IntelLockStatusCheck(NOR_InfoHandle hNorInfo)
{
    Uint32 retval = E_PASS;
    //Uint8 status;

    LOCAL_IntelWaitForStatusComplete(hNorInfo);

    //status = flash_read_uint16((Uint32)hNorInfo->flashBase,0);
    //if ( status & (1<<5) )
    if (NOR_flashIsSetSome(hNorInfo, hNorInfo->flashBase, 0, ((1<<5) | (1<<3))))
    {
        retval = E_FAIL;
		/*if ( status & (1<<4) )
        {
			puts("Command Sequence Error");
		}
		else
		{
			puts("Clear Lock Error");
		}*/
	}
	/*if ( status & (1<<3) )
	{
		retval = E_FAIL;
		//puts("Voltage Range Error\n");
    }*/
	
	// Clear status
	LOCAL_IntelClearStatus(hNorInfo);
	
	// Put chip back into read array mode.
	LOCAL_IntelSoftReset(hNorInfo);
	
	// Set Timings back to Optimum for Read
	return retval;
}

// Erase Block
static Uint32 Intel_Erase(NOR_InfoHandle hNorInfo, volatile Uint32 blkAddr)
{
	Uint32 retval = E_PASS;
	
	// Clear Lock Bits
	retval |= LOCAL_IntelClearLock(hNorInfo,blkAddr);
	
	// Send Erase commands
	NOR_flashWriteCmd(hNorInfo,blkAddr,0,INTEL_ERASE_CMD0);
	NOR_flashWriteCmd(hNorInfo,blkAddr,0,INTEL_ERASE_CMD1);
	
	// Wait until Erase operation complete
	LOCAL_IntelWaitForStatusComplete(hNorInfo);
    
    // Verify successful erase                       
    if ( NOR_flashIsSetSome(hNorInfo,hNorInfo->flashBase, 0, (1<<5)) )
        retval = E_FAIL;
    
	// Put back into Read Array mode.
	LOCAL_IntelSoftReset(hNorInfo);
	
	return retval;
}

// Write data
static Uint32 Intel_Write(NOR_InfoHandle hNorInfo, Uint32 address, volatile Uint32 data )
{
  Uint32 retval = E_PASS;

  // Send Write command
  NOR_flashWriteCmd(hNorInfo,address,0,INTEL_WRITE_CMD);
  NOR_flashWriteData(hNorInfo,address, data);

  // Wait until Write operation complete
  LOCAL_IntelWaitForStatusComplete(hNorInfo);

  // Verify successful program
  if ( NOR_flashIsSetSome(hNorInfo, hNorInfo->flashBase, 0, ((1<<4)|(1<<3))) )
  {
    //puts("Write Op Failed.", FALSE);
    retval = E_FAIL;
  }
    
  // Lock the block
  retval |= LOCAL_IntelSetLock(hNorInfo,address);
    
  // Put back into Read Array mode.
	LOCAL_IntelSoftReset(hNorInfo);
                          
  return retval;
}

// Buffer write data
static Uint32 Intel_BufferWrite(NOR_InfoHandle hNorInfo, Uint32 address, volatile Uint8 data[], Uint32 numBytes )
{
  Uint32 startAddress = address;
	Uint32 retval = E_PASS;
	Uint32 timeoutCnt = 0, shift;

	// Send Write_Buff_Load command   
  do {
    NOR_flashWriteCmd(hNorInfo,address,0,INTEL_WRT_BUF_LOAD_CMD);
    timeoutCnt++;
  }while( (!NOR_flashIsSetAll(hNorInfo,hNorInfo->flashBase, 0, (1<<7))) && (timeoutCnt < 0x00010000) );

  if (timeoutCnt >= 0x10000)
  {
    //    puts("Write Op Failed.", FALSE);
    retval = E_TIMEOUT;
  }
  else
  {
    //Establish correct shift value
    shift = 0;
    while ((hNorInfo->busWidth >> shift) > 1)
      shift++;

    // Write Length (either numBytes or numBytes/2)	    
    NOR_flashWriteCmd(hNorInfo, startAddress, 0, (numBytes >> shift) - 1);

    // Write buffer length
    //NOR_flashWriteData(startAddress, (length - 1));

    // Write buffer data
    NOR_flashWriteDatabuffer(hNorInfo, &address,(void*)data,numBytes);

    // Send write buffer confirm command
    NOR_flashWriteCmd(hNorInfo, startAddress,0,INTEL_WRT_BUF_CONF_CMD);

    // Check status
    LOCAL_IntelWaitForStatusComplete(hNorInfo);
    // Verify program was successful

    if ( NOR_flashIsSetSome(hNorInfo, hNorInfo->flashBase, 0, (1<<4)) )
    {
    //    puts("Write Buffer Op Failed.", FALSE);
      retval = E_FAIL;
    }

    // Put back into Read Array mode.
    LOCAL_IntelSoftReset(hNorInfo);
  }

  return retval;
}
// -------------------- End of Intel specific commands -----------------------



// -------------------- Begin of AMD specific commands -----------------------

// Identify the Manufacturer and Device ID 
static Uint32 AMD_ID( NOR_InfoHandle hNorInfo )
{
  // Exit back to read array mode
  LOCAL_AMDSoftReset(hNorInfo);

  // Write ID commands
  LOCAL_AMDPrefixCommands(hNorInfo);
  NOR_flashWriteCmd(hNorInfo, hNorInfo->flashBase, AMD_CMD2_ADDR, AMD_ID_CMD);

  // Read manufacturer's ID
  hNorInfo->manfID = (NOR_ManfID) NOR_flashReadData(hNorInfo,hNorInfo->flashBase, AMD_MANFID_ADDR);
    
  // Read device ID
  hNorInfo->devID1 = (Uint16) NOR_flashReadData(hNorInfo,hNorInfo->flashBase, AMD_DEVID_ADDR0);
    
  // Read additional ID bytes if needed
  if ( (hNorInfo->devID1 & 0xFF ) == AMD_ID_MULTI )
    hNorInfo->devID2 = NOR_flashReadCFIBytes(hNorInfo, AMD_DEVID_ADDR1, 2);
  else
    hNorInfo->devID2 = 0x0000;
        
  // Exit back to read array mode
  LOCAL_AMDSoftReset(hNorInfo);
    
  return E_PASS;
}

static void LOCAL_AMDSoftReset(NOR_InfoHandle hNorInfo)
{
	// Reset Flash to be in Read Array Mode
	NOR_flashWriteCmd(hNorInfo,hNorInfo->flashBase,AMD_CMD2_ADDR,AMD_RESET);
  TSC_delay_us(600);
}

// AMD Prefix Commands
static void LOCAL_AMDPrefixCommands(NOR_InfoHandle hNorInfo)
{
  NOR_flashWriteCmd(hNorInfo, hNorInfo->flashBase, AMD_CMD0_ADDR, AMD_CMD0);
  NOR_flashWriteCmd(hNorInfo, hNorInfo->flashBase, AMD_CMD1_ADDR, AMD_CMD1);
}

// Erase Block
static Uint32 AMD_Erase(NOR_InfoHandle hNorInfo, Uint32 blkAddr)
{
  Uint32 retval = E_PASS;
  Uint32 cnt = 0;

  // Send commands
  LOCAL_AMDPrefixCommands(hNorInfo);
  NOR_flashWriteCmd(hNorInfo,hNorInfo->flashBase, AMD_CMD2_ADDR, AMD_BLK_ERASE_SETUP_CMD);
  LOCAL_AMDPrefixCommands(hNorInfo);
  NOR_flashWriteCmd(hNorInfo,blkAddr, AMD_CMD2_ADDR, AMD_BLK_ERASE_CMD);
	
  // Poll DQ7 and DQ15 for status
  while ( !NOR_flashIsSetAll(hNorInfo,blkAddr, 0, (1<<7)) )
  {
    cnt++;
  }

  TSC_delay_us(1000);
    
  // Check data 
  if ( !NOR_flashDataIsEqual(hNorInfo,blkAddr, 0, AMD_BLK_ERASE_DONE) )
    retval = E_FAIL;
	
  // Flash Mode: Read Array
  //LOCAL_AMDSoftReset(hNorInfo);
    
  return retval;
}

// AMD Flash Write
static Uint32 AMD_Write(NOR_InfoHandle hNorInfo, Uint32 address, volatile Uint32 data )
{
  Uint32 retval = E_PASS;
	
  // Send Commands
  LOCAL_AMDPrefixCommands(hNorInfo);
  NOR_flashWriteCmd(hNorInfo,hNorInfo->flashBase, AMD_CMD2_ADDR, AMD_PROG_CMD);
  NOR_flashWriteData(hNorInfo,address, data);

  // Wait for ready.
  while(TRUE)
  {
    if ( (NOR_flashReadData(hNorInfo, address, 0 ) & ((1<<7) | (1<<15) )) == (data & ((1<<7) | (1<<15)) ) )
    {
      break;
    }
    else
    {
      if(NOR_flashIsSetAll(hNorInfo, address, 0, (1<<5)))
      {
        if ( (NOR_flashReadData(hNorInfo, address, 0 ) & ((1<<7) | (1<<15)) ) != (data & ((1<<7) | (1<<15)) ) )
        {
          puts("Timeout ocurred.");
          retval = E_FAIL;
        }
        break;				
      }
    }
  }
	
  // Return Read Mode
	//LOCAL_AMDSoftReset(hNorInfo);
	
	// Verify the data.
  if ( (retval == E_PASS) && ( NOR_flashReadData(hNorInfo, address, 0) != data) )
    retval = E_FAIL;
	
	return retval;
}

// AMD flash buffered write
static Uint32 AMD_BufferWrite(NOR_InfoHandle hNorInfo, Uint32 address, volatile Uint8 data[], Uint32 numBytes )
{
  Uint32 startAddress = address;
  Uint32 blkAddress, blkSize;
  Uint32 data_temp;
  Uint32 retval = E_PASS;
  Uint32 shift;
	
  // Get block base address and size
  NOR_getBlockInfo(hNorInfo, address, &blkSize, &blkAddress);
			
  // Write the Write Buffer Load command
  LOCAL_AMDPrefixCommands(hNorInfo);
  NOR_flashWriteCmd(hNorInfo, blkAddress, 0, AMD_WRT_BUF_LOAD_CMD);
        
  //Establish correct shift value
  shift = 0;
  while ((hNorInfo->busWidth >> shift) > 1)
    shift++;
    
  // Write Length (either numBytes or numBytes/2)	    
  NOR_flashWriteCmd(hNorInfo, blkAddress, 0, (numBytes >> shift) - 1);
	
	// Write Data
	NOR_flashWriteDatabuffer(hNorInfo, &address,(void*)data, numBytes);
		
  // Program Buffer to Flash Confirm Write
  NOR_flashWriteCmd(hNorInfo, blkAddress, 0, AMD_WRT_BUF_CONF_CMD);                  

  TSC_delay_us(10);
    
  // Read last data item                  
  data_temp = NOR_flashReadData(hNorInfo, (Uint32) (data + (address - startAddress)), 0);
        
	while(TRUE)
	{
    if( (NOR_flashReadData(hNorInfo, address, 0 ) & ((1<<7) | (1<<15))) == (data_temp & ((1<<7) | (1<<15)) ) )
    {
      break;
    }
    else
    {
      // Timeout has occurred
      if(NOR_flashIsSetAll(hNorInfo,address, 0, (1<<5)))
      {
        if( (NOR_flashReadData(hNorInfo,address, 0 ) & ((1<<7) | (1<<15))) != (data_temp & ((1<<7) | (1<<15)) ) )
        {
          puts("Timeout ocurred.");
          retval = E_FAIL;
        }
        break;
      }
      // Abort has occurred
      if(NOR_flashIsSetAll(hNorInfo, address, 0, (1<<1)))
      {
        if( (NOR_flashReadData(hNorInfo, address, 0 ) & ((1<<7) | (1<<15))) != (data_temp & ((1<<7) | (1<<15)) ) )
        {
          puts("Abort ocurred.");
          retval = E_FAIL;
          LOCAL_AMDWriteBufAbortReset (hNorInfo);
        }
        break;
      }
    }
  }
	
  // Put chip back into read array mode.
  //LOCAL_AMDSoftReset(hNorInfo);
  if (retval == E_PASS)
  {
    retval = NOR_flashVerifyDataBuffer(hNorInfo, startAddress,(void*)data, numBytes);
    if (retval != E_PASS)
      puts("Data verify failed.");
  }
  return retval;
}

// AMD Write Buf Abort Reset Flash
static void LOCAL_AMDWriteBufAbortReset(NOR_InfoHandle hNorInfo)
{
  // Reset Flash to be in Read Array Mode
  LOCAL_AMDPrefixCommands(hNorInfo);
  LOCAL_AMDSoftReset(hNorInfo);
}

// --------------------- End of AMD specific commands ------------------------


/***********************************************************
* End file                                                 *
***********************************************************/

/* --------------------------------------------------------------------------
	HISTORY
    v1.00 - DJA - 16-Aug-2007
      Initial Release
    v1.01 - DJA - 02-Nov-2007
      Code cleanup, minor mod to erase function
    v1.1 - Brighton Feng - 20-Dec-2011
      modify for big-endian for TCI6614 test
-------------------------------------------------------------------------- */

