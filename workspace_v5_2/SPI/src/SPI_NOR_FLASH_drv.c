/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2012
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
 example for SPI NOR FLASH driver on KeyStone DSP. 
 Tested on AT225640B, N25Q032 and N25Q128.
 * =============================================================================
 *  Revision History
 *  ===============
 *  Feb 20, 2012 Brighton Feng  file created
 *  Aug 8, 2013, Brighton Feng, update for MX66L51235F(MACRONIX)
 * =============================================================================*/

#include <stdio.h>
#include "KeyStone_common.h"
#include "KeyStone_SPI_init_drv.h"
#include "SPI_NOR_FLASH_drv.h"

#define USE_FSR 	0

/*data format for NOR FLASH test*/
SPI_Data_Format FlashDataFormat =
{
	/*.delayBetweenTrans_ns=*/0,   
	/*.ShifDirection       =*/SPI_MSB_SHIFT_FIRST,
	/*.disable_CS_timing   =*/0,
	/*.clockPolarity       =*/SPI_CLOCK_LOW_INACTIVE,
	/*.clockPhase          =*/1,
	/*.clockSpeedKHz       =*/30000,
	/*.wordLength          =*/8
};

SPI_Transfer_Param FlashTransferParam =
{
	0,     /*Chip select number*/
	0,  /*select one of the 4 SPI formats*/
	SPI_CS_NO_LAST_HOLD, /*hold CS between multiple words*/
	TRUE,  /*Enable the delay counter at the end of the current transaction*/
	1   /*number of bytes per SPI word*/
};


Uint32 uiSPI_NOR_FLASH_page_size=256; 	/*must be power of 2*/
Uint32 uiSPI_NOR_FLASH_sector_size=64*1024;	/*must be power of 2*/
Bool needEraseBeforeWrite= TRUE; 	

void SPI_NOR_FLASH_fill_address(Uint32 data_address, 
	Uint32 address_width,Uint8 * ucBuffer)
{
	if(address_width>24)
	{
		ucBuffer[0]= data_address>>24; 	/*highest 8 bits of the address*/
		ucBuffer[1]= data_address>>16; 	/*higher 8 bits of the address*/
		ucBuffer[2]= data_address>>8; 	/*middle 8 bits of the address*/
		ucBuffer[3]= data_address; 	/*lower 8 bits of the address*/
	}
	else if(address_width>16)
	{
		ucBuffer[0]= data_address>>16; 	/*higher 8 bits of the address*/
		ucBuffer[1]= data_address>>8; 	/*middle 8 bits of the address*/
		ucBuffer[2]= data_address; 	/*lower 8 bits of the address*/
	}
	else
	{
		ucBuffer[0]= data_address>>8; 	/*higher 8 bits of the address*/
		ucBuffer[1]= data_address; 	/*lower 8 bits of the address*/
	}
}

/*read the value of status register form SPI NOR FLASH*/
Uint8 SPI_NOR_FLASH_read_status()
{
	Uint8 txBuffer[4], rxBuffer[4]; 

#if USE_FSR
	txBuffer[0]= SPI_NOR_FLASH_RDFSR;        /*instruction*/ 
#else
	txBuffer[0]= SPI_NOR_FLASH_RDSR;        /*instruction*/ 
#endif
	KeyStone_SPI_TxRx(txBuffer, 0, 1, 
	                rxBuffer, 1, 1, &FlashTransferParam); 

	return rxBuffer[0]; 
}

/*read "uiByteCount" data to "ucBuffer" from "data_address" of SPI NOR FLASH.
return number of bytes transfered*/
Uint32 SPI_NOR_FLASH_read(Uint32 data_address, 
	Uint32 address_width, Uint32 uiByteCount, Uint8 * ucBuffer)
{
	Uint8 txBuf[8];
	Uint32 byteSuccess=0;
	
	txBuf[0]= SPI_NOR_FLASH_READ;	/*instruction*/
	SPI_NOR_FLASH_fill_address(data_address, address_width, &txBuf[1]);
	/*read data*/
	byteSuccess= KeyStone_SPI_TxRx(txBuf, 0, address_width/8+1,	
		ucBuffer, address_width/8+1, uiByteCount, 
		&FlashTransferParam);

	if(byteSuccess<address_width/8+1)
		byteSuccess= 0;
	else
		byteSuccess -= address_width/8+1;
	return byteSuccess;
}

/*write "uiByteCount" data in "ucBuffer" to "data_address" of SPI NOR FLASH.
Please note, the data must be in SPI_NOR_FLASH_PAGE_SIZE, that is,
((data_address&0xff)+uiByteCount)<=SPI_NOR_FLASH_PAGE_SIZE
return number of bytes transfered.*/
Uint32 SPI_NOR_FLASH_page_write(Uint32 data_address, 
	Uint32 address_width, Uint32 uiByteCount, Uint8 * ucBuffer)
{
	Uint8 spiBuffer[256+8];
	Uint32 byteSuccess=0;

	/*Write enable instruction*/
	SPI_NOR_FLASH_command(SPI_NOR_FLASH_WREN);

	spiBuffer[0]= SPI_NOR_FLASH_WRITE;	/*instruction*/
	SPI_NOR_FLASH_fill_address(data_address, address_width, &spiBuffer[1]);

	memcpy(spiBuffer+address_width/8+1, ucBuffer, uiByteCount);

	/*Write data*/
	byteSuccess= KeyStone_SPI_TxRx(spiBuffer, 0, address_width/8+1+uiByteCount,	
		NULL, 0, 0, &FlashTransferParam);

	if(byteSuccess<address_width/8+1)
		return 0;
	byteSuccess -= address_width/8+1;

	/*poll the status register until programming complete*/
#if USE_FSR
	while(SPI_NOR_FLASH_read_status()&0x80);
	while(SPI_NOR_FLASH_read_status()&0x80);
#else
	while(SPI_NOR_FLASH_read_status()&1);
#endif
	
	return byteSuccess;
}

/*write "uiByteCount" data in "ucBuffer" to address "data_address" 
in the SPI NOR FLASH.*/
Uint32 SPI_NOR_FLASH_write(Uint32 data_address, 
	Uint32 address_width, Uint32 uiByteCount, Uint8 * ucBuffer)
{
	Uint32 pageByteCount, pageAddrOffset;
	Uint32 byteSuccess, tempCount= uiByteCount;

	/*SPI NOR FLASH write must be in SPI_NOR_FLASH_PAGE_SIZE, write more than 
	SPI_NOR_FLASH_PAGE_SIZE a time will wrap around and overwrite privious data*/

	/*checke the address offset in a SPI_NOR_FLASH_PAGE_SIZE*/
	pageAddrOffset= data_address&(uiSPI_NOR_FLASH_page_size-1);

	/*handle non-aligned data firstly*/
	if(pageAddrOffset)
	{
		pageByteCount= uiSPI_NOR_FLASH_page_size- pageAddrOffset;
		if(pageByteCount>uiByteCount)
			pageByteCount= uiByteCount;

		byteSuccess= SPI_NOR_FLASH_page_write(data_address, 
			address_width, pageByteCount, ucBuffer);
		if(byteSuccess!= pageByteCount)
			return byteSuccess;

		data_address+= pageByteCount;
		ucBuffer+= pageByteCount;
		tempCount-= pageByteCount;
	}

	/*write the remaining data page by page*/
	while(tempCount)
	{
		if(tempCount>=uiSPI_NOR_FLASH_page_size)
			pageByteCount= uiSPI_NOR_FLASH_page_size;
		else
			pageByteCount= tempCount;

		byteSuccess= SPI_NOR_FLASH_page_write(data_address, 
			address_width, pageByteCount, ucBuffer);
		if(byteSuccess!= pageByteCount)
			return byteSuccess;

		data_address+= pageByteCount;
		ucBuffer+= pageByteCount;
		tempCount-= pageByteCount;
	}

	return uiByteCount;
}

/*erase the whole NOR FLASH*/
Uint32 SPI_NOR_FLASH_bulk_erase()
{
	Uint32 byteSuccess=0;

	if(!needEraseBeforeWrite)
		return 0;

	byteSuccess= SPI_NOR_FLASH_command(SPI_NOR_FLASH_WREN);
	if(1!=byteSuccess)
		return 0;

	byteSuccess= SPI_NOR_FLASH_command(SPI_NOR_FLASH_BULK_ERASE);
	if(1!=byteSuccess)
		return 0;

	/*poll the status register until programming complete*/
#if USE_FSR
	while(SPI_NOR_FLASH_read_status()&0x80);
	while(SPI_NOR_FLASH_read_status()&0x80);
#else
	while(SPI_NOR_FLASH_read_status()&1);
#endif
	
	return byteSuccess;
}

/*erase one sector in NOR FLASH*/
Uint32 SPI_NOR_FLASH_sector_erase(Uint32 sector_address, 
	Uint32 address_width)
{
	Uint8 spiBuffer[8];
	Uint32 byteSuccess=0;

	if(!needEraseBeforeWrite)
		return 0;

	/*Write enable instruction*/
	byteSuccess= SPI_NOR_FLASH_command(SPI_NOR_FLASH_WREN);
	if(1!=byteSuccess)
		return 0;

	spiBuffer[0]= SPI_NOR_FLASH_SECTOR_ERASE;	/*instruction*/
	SPI_NOR_FLASH_fill_address(sector_address, address_width, &spiBuffer[1]);
	byteSuccess= KeyStone_SPI_TxRx(spiBuffer, 0, address_width/8+1,	
		NULL, 0, 0, &FlashTransferParam);
	if((address_width/8+1)!=byteSuccess)
		return 0;

	/*poll the status register until programming complete*/
#if USE_FSR
	while(SPI_NOR_FLASH_read_status()&0x80);
	while(SPI_NOR_FLASH_read_status()&0x80);
#else
	while(SPI_NOR_FLASH_read_status()&1);
#endif

#if 1 /*for debug*/
	int i;
	SPI_NOR_FLASH_read(sector_address, 
		address_width, 8, spiBuffer);
	for(i=0;i<8;i++)
	{
		if(0xFF!=spiBuffer[i])
		{
			printf("erase failed at 0x%x\n", sector_address);
			return 0;
		}
	}
#endif
	
	return byteSuccess;
}

/*erase a section in NOR FLASH, return number of sectors erased*/
Uint32 SPI_NOR_FLASH_erase(Uint32 start_address, 
	Uint32 byteCount, Uint32 address_width)
{
	Uint32 uiAddress, uiSectorCount= 0;
	Uint32 uiAlignedAddress;	/*address align to sector*/
	Uint32 uiAlignedByteCount;	/*byte count be multiple of sector size*/

	if(!needEraseBeforeWrite)
		return 0;

	/*since SPI NOR FLASH is erased in sector, we need align the address 
	and byte count to sector size*/
	if(start_address&(uiSPI_NOR_FLASH_sector_size-1))
	{/*not aligned*/
		uiAlignedAddress= start_address&(~(uiSPI_NOR_FLASH_sector_size-1));
		byteCount+= (start_address&(uiSPI_NOR_FLASH_sector_size-1));
	}
	else
		uiAlignedAddress= start_address;
	
	if(byteCount&(uiSPI_NOR_FLASH_sector_size-1))
	{/*not aligned*/
		uiAlignedByteCount= (byteCount+uiSPI_NOR_FLASH_sector_size)&
			(~(uiSPI_NOR_FLASH_sector_size-1));
	}
	else
		uiAlignedByteCount= byteCount;
	
	for(uiAddress= uiAlignedAddress; 
		uiAddress< uiAlignedAddress+uiAlignedByteCount;
		uiAddress+= uiSPI_NOR_FLASH_sector_size)
	{
		if(0==SPI_NOR_FLASH_sector_erase(uiAddress, address_width))
			return uiSectorCount;
		uiSectorCount++;
	}
	
	return uiSectorCount;
}

/*read the ID of the NOR FLASH*/
Uint32 SPI_NOR_FLASH_ID_read(Uint8 * ucBuffer) 
{ 
	Uint8 spiBuffer[4]; 

	spiBuffer[0]= SPI_NOR_FLASH_RDID;        /*instruction*/ 
	KeyStone_SPI_TxRx(spiBuffer, 0, 1, 
	                ucBuffer, 1, 3, &FlashTransferParam); 

	return 1; 

}

