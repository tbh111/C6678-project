/******************************************************************************

  Copyright (C), 2001-2012, Texas Instrument.

 ******************************************************************************
  File Name     : KeyStone_DDR_Init.h
  Version       : Initial Draft
  Author        : Brighton Feng
  Created       : 2010/12/6
  Last Modified :
  Description   : KeyStone_DDR_Init.c header file
  Function List :
  History       :
  1.Date        : 2010/12/6
    Author      : Brighton Feng
    Modification: Created file

  2.Date         : 2011/4/17
    Author       : Brighton Feng
    Modification: for C6678 EVM

  3.Date         : 2011/7/12
    Author       : Brighton Feng
    Modification: for dual C6670 EVM    

  4.Date         : 2012/5/8
    Author       : Brighton Feng
    Modification: C6670 EVM and TCI6614 EVM

  5.Date         : 2012/11/28
    Author       : Brighton Feng
    Modification: Modify configuration according to update document
    	"sprabl2a_KeyStone DDR3 Initialization.pdf"
******************************************************************************/
#ifndef _KEYSTONE_DDR_INIT_H_
#define _KEYSTONE_DDR_INIT_H_
#include <ti\csl\cslr_emif4f.h>

typedef enum
{
	DDR_TERM_DISABLED= 0, 						
	DDR_TERM_RZQ_OVER_4, 						
	DDR_TERM_RZQ_OVER_2, 						
	DDR_TERM_RZQ_OVER_6, 						
	DDR_TERM_RZQ_OVER_12, 						
	DDR_TERM_RZQ_OVER_8 						
}T_DDR_TERM;

typedef enum
{
	DDR_DYN_ODT_DISABLED= 0, 						
	DDR_DYN_ODT_OVER_4, 						
	DDR_DYN_ODT_OVER_2 						
}T_DDR_DYN_ODT;

typedef enum
{
	SDRAM_DRIVE_RZQ_OVER_6= 0, 						
	SDRAM_DRIVE_RZQ_OVER_7 						
}T_SDRAM_DRIVE;

typedef enum
{
	DDR_CWL_5= 0, 						
	DDR_CWL_6, 						
	DDR_CWL_7, 						
	DDR_CWL_8, 						
	DDR_CWL_9, 						
	DDR_CWL_10, 						
	DDR_CWL_11, 						
	DDR_CWL_12 						
}T_DDR_CWL;

typedef enum
{
	DDR_CL_5= 2, 						
	DDR_CL_6= 4, 						
	DDR_CL_7= 6, 						
	DDR_CL_8= 8, 						
	DDR_CL_9= 10, 						
	DDR_CL_10= 12, 						
	DDR_CL_11= 14, 						
	DDR_CL_12= 1, 						
	DDR_CL_13= 3, 						
	DDR_CL_14= 5, 						
	DDR_CL_15= 7, 						
	DDR_CL_16= 9 						
}T_DDR_CL;

typedef enum
{
	DDR_BUS_WIDTH_64= 0, 						
	DDR_BUS_WIDTH_32, 						
	DDR_BUS_WIDTH_16					
}T_DDR_BUS_WIDTH;

typedef enum
{
	DDR_ROW_SIZE_9_BIT = 0,
	DDR_ROW_SIZE_10_BIT,
	DDR_ROW_SIZE_11_BIT,
	DDR_ROW_SIZE_12_BIT,
	DDR_ROW_SIZE_13_BIT,
	DDR_ROW_SIZE_14_BIT,
	DDR_ROW_SIZE_15_BIT,
	DDR_ROW_SIZE_16_BIT
}T_DDR_ROW_SIZE;

typedef enum
{
	DDR_BANK_NUM_1= 0, 						
	DDR_BANK_NUM_2, 						
	DDR_BANK_NUM_4, 						
	DDR_BANK_NUM_8					
}T_DDR_BANK_NUM;

typedef enum
{
	DDR_PAGE_SIZE_8_BIT_256_WORD = 0,
	DDR_PAGE_SIZE_9_BIT_512_WORD,
	DDR_PAGE_SIZE_10_BIT_1024_WORD,
	DDR_PAGE_SIZE_11_BIT_2048_WORD
}T_DDR_PAGE_SIZE;

typedef enum
{
	/*enable calculation for accesses outside of these address ranges*/
	EN_ECC_OUT_OF_DEFINED_RANGES= 0, 
	/*Enable ECC calculation within address ranges defined in ECC Address Range 1 and 2 registers*/
	EN_ECC_WITHIN_DEFINED_RANGES 						
}DDR_ECC_Range_Mode;


/*DDR ECC configuration*/
typedef struct
{
	/*please note, this is not DDR virtual mapping address in CPU.
	this is DDR controller internal address from 0~8GB.
	Lower 17 bits will be ignored and cleared*/
	unsigned long long startAddr;
	unsigned long long byteCnt;
}DDR_ECC_Addr_Range;

/*DDR ECC configuration*/
typedef struct
{
	DDR_ECC_Range_Mode rangeMode;
	DDR_ECC_Addr_Range addressRange[2];
}DDR_ECC_Config;

extern void KeyStone_DDR_incremental_leveling(Uint32 incremental_interval_ms);
extern int KeyStone_DDR_full_leveling();
extern void KeyStone_DDR_ECC_INT_enable();
extern void KeyStone_DDR_init(float ref_clock_MHz, unsigned int DDR_PLLM, 
	unsigned int DDR_PLLD, DDR_ECC_Config * ecc_cfg);

#endif

