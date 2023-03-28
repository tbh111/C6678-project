/******************************************************************************

  Copyright (C), 2014, Texas Instrument.

 ******************************************************************************
  File Name     : common_test.c
  Version       : Initial Draft
  Author        : Brighton Feng
  Created       : June 5, 2014
  Last Modified : 
  Description   : KeyStone common test functions and definitions
  History       :
  1.Date        : June 5, 2014
    Author      : Brighton Feng
    Modification: Created file

******************************************************************************/

#include <stdio.h>
#include <csl_edma3.h>
#include "KeyStone_common.h"
#include "common_test.h"

/*****************************************************************************
 Prototype    : Memory_Address_Test
 Description  : this function write the address to corresponding memory
                unit and readback for verification
 Input        : unsigned int uiStartAddress  
                unsigned int uiByteCount     
                unsigned int iStep: step (index offset) of next access 
                                    from the privous one
                uiByteCount and iStep should be multiple of 8 bytes
                unsigned int uiMaxFails: return if uiMaxFails happen
 Output       : None
 Return Value : number of failed units
 
  History        :
  1.Date         : May 25, 2013
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
unsigned int Memory_Address_Test(unsigned int uiStartAddress, 
	unsigned int uiByteCount, unsigned int uiMaxFails, int iStep)
{
    unsigned int i, uiFailCount=0;
    volatile unsigned long long *ulpAddressPointer;
    volatile unsigned long long ulReadBack;

	ulpAddressPointer = (unsigned long long *)uiStartAddress;
	for(i=0; i<uiByteCount/8; i++)
	{
		/* fill with address value */
        *ulpAddressPointer = _itoll(((unsigned int)ulpAddressPointer)+4, 
        	(unsigned int)ulpAddressPointer);
        ulpAddressPointer += (iStep/8);
    }

	WritebackInvalidCache((void *)uiStartAddress, uiByteCount);

	ulpAddressPointer = (unsigned long long *)uiStartAddress;
	for(i=0; i<uiByteCount/8; i++)
	{
        ulReadBack = *ulpAddressPointer;
        if ( ulReadBack != _itoll(((unsigned int)ulpAddressPointer)+4, 
        	(unsigned int)ulpAddressPointer)) /* verify data */
        {
			printf("Memory address Test fails at 0x%8x, Write 0x%016llx, Readback 0x%016llx\n", 
				(Uint32)ulpAddressPointer, _itoll(((unsigned int)ulpAddressPointer)+4, (unsigned int)ulpAddressPointer), ulReadBack);
			Memory_error_double_check((volatile unsigned long long *)ulpAddressPointer);
            uiFailCount++;
			if(uiFailCount>=uiMaxFails)
				return uiFailCount;
        }
        ulpAddressPointer += (iStep/8);
    }

    return uiFailCount; 
}

/*****************************************************************************
 Prototype    : Memory_Fill_Test
 Description  : this function fill memory with a pattern
                and readback for verification
 Input        : unsigned int uiStartAddress  
                unsigned int uiByteCount     
                unsigned int uiPattern       
                unsigned int iStep: step (index offset) of next access 
                                    from the privous one
                uiByteCount and iStep should be multiple of 8 bytes
                unsigned int uiMaxFails: return if uiMaxFails happen
 Output       : None
 Return Value : number of failed units
 
  History        :
  1.Date         : May 25, 2013
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
unsigned int Memory_Fill_Test(unsigned int uiStartAddress, 
	unsigned int uiByteCount, unsigned int uiPattern, 
	unsigned int uiMaxFails, int iStep)
{
    unsigned int i, uiFailCount=0;
    volatile unsigned long long *ulpAddressPointer;
    volatile unsigned long long ulReadBack;

	ulpAddressPointer = (unsigned long long *)uiStartAddress;
	for(i=0; i<uiByteCount/8; i++)
	{
		/* fill with address value */
        *ulpAddressPointer = _itoll(uiPattern, uiPattern);	  
        ulpAddressPointer+= (iStep/8);
    }

	WritebackInvalidCache((void *)uiStartAddress, uiByteCount);

	ulpAddressPointer = (unsigned long long *)uiStartAddress;
	for(i=0; i<uiByteCount/8; i++)
	{
        ulReadBack = *ulpAddressPointer;
        if ( ulReadBack != _itoll(uiPattern, uiPattern)) /* verify data */
        {
			printf("Memory pattern filling Test fails at 0x%8x, Write 0x%016llx, Readback 0x%016llx\n", 
				(Uint32)ulpAddressPointer, _itoll(uiPattern, uiPattern), ulReadBack);
			Memory_error_double_check((volatile unsigned long long *)ulpAddressPointer);
            uiFailCount++;
			if(uiFailCount>=uiMaxFails)
				return uiFailCount;
        }
        ulpAddressPointer += (iStep/8);
    }

    return uiFailCount; 
}

/*****************************************************************************
 Prototype    : Memory_data_bus_test
 Description  : this function test the data bus to detect bit stuck, and
                interference between bits.
 Input        : unsigned int uiBaseAddress 
                unsigned int uiBusWidth, number of bits of the data bus.
 Output       : None
 Return Value : fail count
 
  History        :
  1.Date         : September 18, 2013
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
unsigned int Memory_Data_Bus_Test(unsigned int uiBaseAddress,
	unsigned int uiBusWidth)
{
    int i, j;
    unsigned int uiNumBitShift, uiBusWidth_64bit;
    unsigned int uiFailCount, oldMAR, uiMAR_index;
    volatile unsigned long long *ulpAddressPointer;
    volatile unsigned long long ulReadBack;
    unsigned long long ulBitMask;
    unsigned long long ulDataPattern[7]={0xaaaaaaaaaaaaaaaa, 
    	0xcccccccccccccccc, 0xf0f0f0f0f0f0f0f0, 0xff00ff00ff00ff00, 
    	0xffff0000ffff0000, 0xffffffff00000000, 0};

    uiBusWidth_64bit= (uiBusWidth+63)/64; //bus width in number of 64-bit
	uiNumBitShift= uiBusWidth<64?uiBusWidth:64;

    /*cache should be disabled for this test, otherwise, the write and 
    read back data may stay in the cache, and can not test real bus*/
	WritebackInvalidCache((void *)uiBaseAddress, (8+uiNumBitShift)*uiBusWidth_64bit*8);
	uiMAR_index= uiBaseAddress/16/1024/1024;
	if(uiMAR_index>=16)
		oldMAR= gpCGEM_regs->MAR[uiMAR_index]; //save MAR
	else
		oldMAR= 0;
	if(oldMAR)
	{
		gpCGEM_regs->MAR[uiMAR_index]=0; //disable cache
	}
	
	ulpAddressPointer = (unsigned long long *)uiBaseAddress;

	uiFailCount=0;

	/*----------write/readback pattern to detect stuck bit-----------*/
    for (i = 0; i < 7; i++)
    {
        for(j=0; j<uiBusWidth_64bit; j++)
        {
	        //write a pattern
	        *ulpAddressPointer = ulDataPattern[i];
	        ulpAddressPointer[uiBusWidth_64bit]= ~ulDataPattern[i]; //avoid floating bus
			ulReadBack= *ulpAddressPointer;
	        if (ulReadBack != ulDataPattern[i])
			{
				printf(" Data pattern Test fails at 0x%8x, Write 0x%016llx, Readback 0x%016llx\n", (unsigned int)ulpAddressPointer, (ulDataPattern[i]), ulReadBack);
				Memory_error_double_check((volatile unsigned long long *)ulpAddressPointer);
	            uiFailCount++;
			}	            

	        //write the inversion of the pattern
	        *ulpAddressPointer = ~ulDataPattern[i];
	        ulpAddressPointer[uiBusWidth_64bit]= ulDataPattern[i]; //avoid floating bus
			ulReadBack= *ulpAddressPointer;
	        if (ulReadBack != (~ulDataPattern[i]))
			{
				printf(" Data pattern Test fails at 0x%8x, Write 0x%016llx, Readback 0x%016llx\n", (unsigned int)ulpAddressPointer, (~ulDataPattern[i]), ulReadBack);
				Memory_error_double_check((volatile unsigned long long *)ulpAddressPointer);
	            uiFailCount++;
			}

			ulpAddressPointer++;
		}
    }

	/*-------bit walking to detect interference and stuck betweeen bits--------*/
    ulBitMask = 1;
    for (i = 0; i < uiNumBitShift; i++)
    {
        for(j=0; j<uiBusWidth_64bit; j++)
        {
	        // Test "1" in bit position i
	        *ulpAddressPointer = ulBitMask;
	        ulpAddressPointer[uiBusWidth_64bit]= ~ulBitMask; //avoid floating bus
			ulReadBack= *ulpAddressPointer;
	        if (ulReadBack != ulBitMask)
			{
				printf(" Data bit walking Test fails at 0x%8x, Write 0x%016llx, Readback 0x%016llx\n", (unsigned int)ulpAddressPointer, (ulBitMask), ulReadBack);
				Memory_error_double_check((volatile unsigned long long *)ulpAddressPointer);
	            uiFailCount++;
			}	            

	        // Test "0" in bit position i
	        *ulpAddressPointer = ~ulBitMask;
	        ulpAddressPointer[uiBusWidth_64bit]= ulBitMask; //avoid floating bus
			ulReadBack= *ulpAddressPointer;
	        if (ulReadBack != (~ulBitMask))
			{
				printf(" Data bit walking Test fails at 0x%8x, Write 0x%016llx, Readback 0x%016llx\n", (unsigned int)ulpAddressPointer, (~ulBitMask), ulReadBack);
				Memory_error_double_check((volatile unsigned long long *)ulpAddressPointer);
	            uiFailCount++;
			}

			ulpAddressPointer++;
		}

		ulBitMask = ulBitMask << 1;
    }

	if(oldMAR)
		gpCGEM_regs->MAR[uiMAR_index]= oldMAR; //restore MAR

    return uiFailCount;
}

/*****************************************************************************
 Prototype    : Memory_Address_Bus_Test
 Description  : this function test the address bus to detect bit stuck, and
                interference between bits.
Please note, this function can not be used to test DDR with ECC, because the access
             is not 64-bit aligned.
 Input        : unsigned int uiBaseAddress, must align to uiNumBytes boundary 
                unsigned int uiNumBytes, number of bytes of the memory under test.
                  this is used to determine number of address bits
                unsigned int uiMAU_bytes, Min Access Unit in bytes
 Output       : None
 Return Value : fail count
 
  History        :
  1.Date         : September 19, 2013
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
unsigned int Memory_Address_Bus_Test(unsigned int uiBaseAddress,
	unsigned int uiNumBytes, unsigned int uiMAU_bytes)
{
    int i;
    unsigned int uiFailCount;
    unsigned int uiOldFirstMAR, uiFirstMAR_index, uiOldLastMAR=0, uiLastMAR_index;
    register volatile unsigned int *uipAddressPointer;
    register unsigned int uiReadBack;
    register unsigned char ucReadBack;
    register unsigned short usReadBack;

    /*cache should be disabled for this test, otherwise, the write and 
    read back data may stay in the cache, and can not test real bus*/
	uiFirstMAR_index= uiBaseAddress/16/1024/1024;
	uiLastMAR_index= (uiBaseAddress+uiNumBytes-1)/16/1024/1024;

	if(uiFirstMAR_index>=16)
		uiOldFirstMAR= gpCGEM_regs->MAR[uiFirstMAR_index]; //save MAR
	else
		uiOldFirstMAR= 0;
	if(uiOldFirstMAR)
		gpCGEM_regs->MAR[uiFirstMAR_index]=0; //disable cache
	if(uiLastMAR_index != uiFirstMAR_index)
	{
		if(uiLastMAR_index>=16)
			uiOldLastMAR= gpCGEM_regs->MAR[uiLastMAR_index]; //save MAR
		else
			uiOldLastMAR= 0;
		if(uiOldLastMAR)
			gpCGEM_regs->MAR[uiLastMAR_index]=0; //disable cache
	}
	if(uiOldFirstMAR|uiOldLastMAR)
		WritebackInvalidAllCache((void *)uiBaseAddress);

	uiFailCount=0;

	if(1==uiMAU_bytes)
	{//test address bit 0

		*(unsigned char *)uiBaseAddress=0;
        // Test "1" in bit position 0
        *(unsigned char *)(uiBaseAddress+1) = 1;
		ucReadBack= *(unsigned char *)uiBaseAddress;
        if (ucReadBack != 0)
		{
			printf(" Address bit walking Test fails at 0x%8x, Readback 0x%2x from the first data unit\n", uiBaseAddress+1, ucReadBack);
            uiFailCount++;
		}	            

		*(unsigned char *)(uiBaseAddress+uiNumBytes-1)=0xFF;
        // Test "0" in bit position 0
        *(unsigned char *)(uiBaseAddress+uiNumBytes-2) = 0xFE;
		ucReadBack= *(unsigned char *)(uiBaseAddress+uiNumBytes-1);
        if (ucReadBack != 0xFF)
		{
			printf(" Address bit walking Test fails at 0x%8x, Readback 0x%2x from the last data unit\n", uiBaseAddress+uiNumBytes-2, ucReadBack);
            uiFailCount++;
		}
	}

	if(2>=uiMAU_bytes)
	{//test address bit 1

		*(unsigned short *)uiBaseAddress=0;
        // Test "1" in bit position 1
        *(unsigned short *)(uiBaseAddress+2) = 2;
		usReadBack= *(unsigned short *)uiBaseAddress;
        if (usReadBack != 0)
		{
			printf(" Address bit walking Test fails at 0x%8x, Readback 0x%4x from the first data unit\n", uiBaseAddress+2, usReadBack);
            uiFailCount++;
		}	            

		*(unsigned short *)(uiBaseAddress+uiNumBytes-2)=0xFFFF;
        // Test "0" in bit position 1
        *(unsigned short *)(uiBaseAddress+uiNumBytes-4) = 0xFFFC;
		usReadBack= *(unsigned short *)(uiBaseAddress+uiNumBytes-2);
        if (usReadBack != 0xFFFF)
		{
			printf(" Address bit walking Test fails at 0x%8x, Readback 0x%4x from the last data unit\n", uiBaseAddress+uiNumBytes-4, usReadBack);
            uiFailCount++;
		}

	}
	
	uipAddressPointer = (unsigned int *)uiBaseAddress;

    for (i = 0; (1<<i)*4 < uiNumBytes; i++)
    {
		uipAddressPointer[0]= 0;
        // Test "1" in bit position i+2
        uipAddressPointer[1<<i] = (unsigned int)&uipAddressPointer[1<<i];
		uiReadBack= uipAddressPointer[0];
        if (uiReadBack != 0)
		{
			printf(" Address bit walking Test fails at 0x%8x, Readback 0x%8x from the first data unit\n", (Uint32)&uipAddressPointer[1<<i], uiReadBack);
            uiFailCount++;
		}	            

		uipAddressPointer[uiNumBytes/4-1]= 0xFFFFFFFF;
        // Test "0" in bit position i+2
        uipAddressPointer[uiNumBytes/4-1-(1<<i)] = (unsigned int)&uipAddressPointer[uiNumBytes/4-1-(1<<i)];
		uiReadBack= uipAddressPointer[uiNumBytes/4-1];
        if (uiReadBack != 0xFFFFFFFF)
		{
			printf(" Address bit walking Test fails at 0x%8x, Readback 0x%8x from the last data unit\n", (Uint32)&uipAddressPointer[uiNumBytes/4-1-(1<<i)], uiReadBack);
            uiFailCount++;
		}

    }

	if(uiOldFirstMAR)
		gpCGEM_regs->MAR[uiFirstMAR_index]= uiOldFirstMAR; //restore MAR
	if((uiLastMAR_index != uiFirstMAR_index)&&uiOldLastMAR)
		gpCGEM_regs->MAR[uiLastMAR_index]= uiOldLastMAR; //restore MAR

    return uiFailCount;
}


/*quick test to verify basic functions of memory from "uiStartAddress".
"uiTotalByteCount" determines number of address bits for address bus test.
"uiFillByteCount" determines how many bytes are filled into the memory, 
the bigger the "uiFillByteCount", the longer the test time.
this function returns fail count*/
int Memory_quick_test(unsigned int uiStartAddress,
	unsigned int uiTotalByteCount, unsigned int uiFillByteCount,
	unsigned int uiDataBusWidth)
{
	unsigned int uiFailCount= 0;
	
	uiFailCount += Memory_Data_Bus_Test(uiStartAddress, uiDataBusWidth);
	uiFailCount += Memory_Address_Bus_Test(uiStartAddress, uiTotalByteCount, 1);
	uiFailCount += Memory_Fill_Test(uiStartAddress,uiFillByteCount, 0x00000000, 8, 8);
	uiFailCount += Memory_Fill_Test(uiStartAddress,uiFillByteCount, 0xFFFFFFFF, 8, 8);
	uiFailCount += Memory_Address_Test(uiStartAddress,uiFillByteCount, 8, 8);

	return uiFailCount;
}
/*double check error at a specific address to see if the error is consistent 
and hlep judge it is read or write error*/
void Memory_error_double_check(volatile unsigned long long * ullpAddress)
{
	printf("  Read data at address 0x%08x 8 times again, get: 0x%016llx, 0x%016llx, 0x%016llx, 0x%016llx, 0x%016llx, 0x%016llx, 0x%016llx, 0x%016llx\n",
		ullpAddress, *ullpAddress, *ullpAddress, *ullpAddress, *ullpAddress, *ullpAddress, *ullpAddress, *ullpAddress, *ullpAddress);

	*ullpAddress= 0;
	printf("  Write 0 again and read back 0x%016llx,", *ullpAddress);

	*ullpAddress= 0xffffffffffffffffULL;
	printf("  write 0xffffffffffffffff again and read back 0x%016llx\n", *ullpAddress);
}

/*Test throughput of EDMA "TC_channel" for data transfer from "src" to "dst"*/
void edma_Throughput_Test (Uint32 src, Uint32 dst, Uint32 uiACount, 
	Uint32 uiBCount, Uint32 uiIndex, CSL_TpccRegs*  EDMACCRegs, Uint32 TC_channel)
{
	unsigned int cycles;
	Uint32 loopIndex;
	unsigned long long *srcBuff=(unsigned long long *)src;
	unsigned long long *dstBuff=(unsigned long long *)dst;

	/* Initialize data buffers */
	for (loopIndex = 0; loopIndex < uiACount*uiBCount/8; loopIndex++) {
		srcBuff[loopIndex] = _itoll(dst+loopIndex*8+4,dst+loopIndex*8);
		dstBuff[loopIndex] = 0xaaaaaaaaaaaaaaaaULL;
	}      

	//Clear cache    
	CACHE_wbInvAllL1d(CACHE_WAIT);
	CACHE_wbInvAllL2(CACHE_WAIT);
	_mfence();
	_mfence();

	//clear completion flag
	EDMACCRegs->TPCC_ICR=(1<<TC_channel); 

	EDMACCRegs->PARAMSET[TC_channel].OPT= 
		CSL_EDMA3_OPT_MAKE(CSL_EDMA3_ITCCH_DIS, 
			CSL_EDMA3_TCCH_DIS, 
			CSL_EDMA3_ITCINT_DIS, 
			CSL_EDMA3_TCINT_EN,
			TC_channel,
			CSL_EDMA3_TCC_NORMAL,
			CSL_EDMA3_FIFOWIDTH_NONE, 
			CSL_EDMA3_STATIC_DIS, 
			CSL_EDMA3_SYNC_AB, 
			CSL_EDMA3_ADDRMODE_INCR, 
			CSL_EDMA3_ADDRMODE_INCR);
	EDMACCRegs->PARAMSET[TC_channel].SRC= src;
	EDMACCRegs->PARAMSET[TC_channel].A_B_CNT= CSL_EDMA3_CNT_MAKE(uiACount, uiBCount);
	EDMACCRegs->PARAMSET[TC_channel].DST= dst;
	EDMACCRegs->PARAMSET[TC_channel].SRC_DST_BIDX= CSL_EDMA3_BIDX_MAKE(uiIndex, uiIndex);
	EDMACCRegs->PARAMSET[TC_channel].LINK_BCNTRLD= CSL_EDMA3_LINKBCNTRLD_MAKE(0xFFFF, uiBCount);
	EDMACCRegs->PARAMSET[TC_channel].SRC_DST_CIDX= 0;
	EDMACCRegs->PARAMSET[TC_channel].CCNT= 1;

	/*Manually trigger the EDMA*/
	EDMACCRegs->TPCC_ESR= 1<<(TC_channel);

	cycles = TSCL; 	/*record start time*/
	/* Wait for completion */
	while ((EDMACCRegs->TPCC_IPR&(1<<(TC_channel))) ==0);
	cycles= TSC_count_cycle_from(cycles);

	printf("transfer %4d * %5d Bytes with index=%5d from 0x%8x to 0x%8x, ", uiBCount, uiACount, uiIndex, src, dst);
	printf("consumes %6d cycles, achieve throughput %5lld MB/s\n", cycles, (unsigned long long)uiACount*uiBCount*gDSP_Core_Speed_Hz/cycles/1000000);

	//clear completion flag
	EDMACCRegs->TPCC_ICR=(1<<TC_channel); 

	CSL_XMC_invalidatePrefetchBuffer();
	_mfence();
	_mfence();

	/* Verify data transfered */
	if((dst&0xffffff)>=0xe00000&&(dst&0xffffff)<0xf00000)
		return; 	//L1P can't be verified by following codes
	if(uiIndex!=uiACount)
		return; 	//non-linear transfer, do not verify it
	for (loopIndex = 0; loopIndex < uiACount*uiBCount/8; loopIndex++) {
		if(dstBuff[loopIndex] != _itoll(dst+loopIndex*8+4,dst+loopIndex*8))
		{
			printf("EDMA data transfer failed at 0x%x\n", (Uint32)(dstBuff+loopIndex));
			break;
		}
	}
	return;  

}

//Copy multiple of 8 bytes data to show the max throughput of data transfer by CPU
void MemCopy8(unsigned long long * restrict dst, unsigned long long * restrict src, Uint32 uiCount)
{
	int i;
#pragma MUST_ITERATE(4,,4)
	for(i=0; i< uiCount/4; i++)
	{
		*dst++=*src++;
		*dst++=*src++;
		*dst++=*src++;
		*dst++=*src++;
	}
}

void MemCopy8Test(unsigned long long * srcBuff, unsigned long long * dstBuff, Uint32 uiByteCnt)
{
	int i;
	
	unsigned int cycles;
	Uint32 iThroughput;

	/* Initialize data buffers */
	for(i=0; i< uiByteCnt/8; i++)
	{
		srcBuff[i]= (unsigned long long)(dstBuff+i);
		dstBuff[i] = 0xaaaaaaaaaaaaaaaaULL;
	}

	//Clear cache
	CACHE_wbInvAllL2(CACHE_WAIT);
	//CACHE_invAllL1p(CACHE_WAIT);
	cycles= TSCL;
	MemCopy8(dstBuff, srcBuff, uiByteCnt/8);
	//if((Uint32)dstBuff>=0x60000000) 	//DDR
	//	CACHE_wbL2((void *)dstBuff, uiByteCnt, CACHE_WAIT);
	cycles= TSC_count_cycle_from(cycles);
	iThroughput= (unsigned long long)uiByteCnt*gDSP_Core_Speed_Hz/cycles/1000000;
	printf("%5d MB/s, copy %d bytes from 0x%x to 0x%x, consumes %ld cycles\n", iThroughput, uiByteCnt, srcBuff, dstBuff, cycles);

	//verfiry result
	for (i = 0; i < uiByteCnt/8; i++) {
		if(dstBuff[i] != (unsigned long long)(dstBuff+i))
		{
			printf("CPU memory copy failed at 0x%x\n", (Uint32)(dstBuff+i));
			break;
		}
	}
}


void RepeatMemCopy8Test(unsigned long long * srcBuff, unsigned long long * dstBuff, Uint32 uiByteCnt)
{
	int i;
	
	unsigned int cycles;
	Uint32 iThroughput;

	/* Initialize data buffers */
	for(i=0; i< uiByteCnt/8; i++)
	{
		srcBuff[i]= (unsigned long long)(dstBuff+i);
		dstBuff[i] = 0xaaaaaaaaaaaaaaaaULL;
	}

	//Clear cache
	CACHE_wbInvAllL2(CACHE_WAIT);
	//CACHE_invAllL1p(CACHE_WAIT);
	for(i=0; i<8; i++)
	{
		cycles= TSCL;
		MemCopy8(dstBuff, srcBuff, uiByteCnt/8);
		cycles= TSC_count_cycle_from(cycles);
		iThroughput= (unsigned long long)uiByteCnt*gDSP_Core_Speed_Hz/cycles/1000000;
		printf("%5d MB/s, copy %d bytes from 0x%x to 0x%x, consumes %ld cycles\n", iThroughput, uiByteCnt, srcBuff, dstBuff, cycles);
	}
	
	//verfiry result
	for (i = 0; i < uiByteCnt/8; i++) {
		if(dstBuff[i] != (unsigned long long)(dstBuff+i))
		{
			printf("CPU memory copy failed at 0x%x\n", (Uint32)(dstBuff+i));
			break;
		}
	}
	printf("\n");
}



