/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2012
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
 example for SPI NOR FLASH driver on KeyStone DSP. 
 Tested on AT225640B and N25Q128.
 * =============================================================================
 *  Revision History
 *  ===============
 *  Feb 20, 2012 Brighton Feng  file created
 * =============================================================================*/
#ifndef _SPI_NOR_FLASH_DRV_H
#define _SPI_NOR_FLASH_DRV_H

#include <tistdtypes.h>

typedef enum {
	SPI_NOR_FLASH_WRSR =1, /*Write Status Register      */
	SPI_NOR_FLASH_WRITE=2, /*Write Data to Memory Array */
	SPI_NOR_FLASH_READ =3, /*Read Data from Memory Array*/
	SPI_NOR_FLASH_WRDI =4, /*Reset Write Enable Latch   */
	SPI_NOR_FLASH_RDSR =5, /*Read Status Register       */
	SPI_NOR_FLASH_RDFSR =0x70, /*Read Status Register       */
	SPI_NOR_FLASH_WREN =6,  /*Set Write Enable Latch     */
	SPI_NOR_FLASH_RDID         =0x9F,  /*read ID*/
	SPI_NOR_FLASH_EN4B         =0xB7,  /*enter 4-byte mode*/
	SPI_NOR_FLASH_SUB_SECTOR_ERASE =0x20,  /*Sub-Sector (4KB) Erase*/
	SPI_NOR_FLASH_SECTOR_ERASE =0xD8,  /*Sector (64KB) Erase  */
	SPI_NOR_FLASH_BULK_ERASE   =0xC7,  /*Bulk Erase    */
	SPI_NOR_FLASH_RESET_ENABLE  =0x66,  
	SPI_NOR_FLASH_RESET_MEMORY  =0x99
}SPI_NOR_FLASH_Instruction;

extern SPI_Data_Format FlashDataFormat;
extern SPI_Transfer_Param FlashTransferParam;

extern Uint32 uiSPI_NOR_FLASH_page_size; 	/*must be power of 2*/
extern Uint32 uiSPI_NOR_FLASH_sector_size;	/*must be power of 2*/
extern Bool needEraseBeforeWrite; 	

/*write a commond to NOR FLASH*/
static inline Uint32 SPI_NOR_FLASH_command(Uint8 command) 
{ 
	Uint8 spiBuffer[4]; 

	spiBuffer[0]= command;	/*command*/
	return KeyStone_SPI_TxRx(spiBuffer, 0, 1,	
		NULL, 0, 0, &FlashTransferParam);
}

/*read "uiByteCount" data to "ucBuffer" from "data_address" of SPI NOR FLASH.
return number of bytes transfered*/
extern Uint32 SPI_NOR_FLASH_read(Uint32 uiAddress, 
	Uint32 address_width, Uint32 uiByteCount, Uint8 * ucBuffer);

/*write "uiByteCount" data in "ucBuffer" to address "data_address" 
in the SPI NOR FLASH.*/
extern Uint32 SPI_NOR_FLASH_write(Uint32 uiAddress, 
	Uint32 address_width, Uint32 uiByteCount, Uint8 * ucBuffer);

/*erase the whole NOR FLASH*/
extern Uint32 SPI_NOR_FLASH_bulk_erase();

/*erase a section in NOR FLASH*/
extern Uint32 SPI_NOR_FLASH_erase(Uint32 start_address, 
	Uint32 byteCount, Uint32 address_width);

/*read the ID of the NOR FLASH*/
extern Uint32 SPI_NOR_FLASH_ID_read(Uint8 * ucBuffer); 

#endif

