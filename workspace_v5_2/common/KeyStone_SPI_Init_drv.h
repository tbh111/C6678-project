/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2012
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
 Example of SPI configuration and read/write driver
 * =============================================================================
 *  Revision History
 *  ===============
 *  Feb 19, 2012 Brighton Feng created
Dec 8, 2014, Brighton Feng fixed CS selection and polarity bug.
* =============================================================================
 */
#ifndef _KEYSTONE_SPI_INIT_H_
#define _KEYSTONE_SPI_INIT_H_

#include <tistdtypes.h>
#include <cslr_spi.h>

typedef enum {
	SPI_CLOCK_LOW_INACTIVE= 0,
	SPI_CLOCK_HIGH_INACTIVE
}SPI_Clock_Polarity;

typedef enum {
	SPI_MSB_SHIFT_FIRST= 0,
	SPI_LSB_SHIFT_FIRST
}SPI_Shift_Direction;

typedef enum {
	SPI_CS_LOW_ACTIVE= 0,
	SPI_CS_HIGH_ACTIVE
}SPI_CS_Polarity;

typedef enum {
	SPI_CS_0= 0,
	SPI_CS_1,
	SPI_CS_2,
	SPI_CS_3,
	SPI_CS_4
}SPI_CS_Select;

/*hold CS between multiple words*/
typedef enum {
	SPI_CS_NO_HOLD= 0, 	/*NO hold between words*/
	SPI_CS_ALWAYS_HOLD, /*always hold between words*/
	SPI_CS_NO_LAST_HOLD /*no hold for last word, CS hold for previous words*/
}SPI_CS_Hold;

/*EDMA TX destination register and data width*/
typedef enum {
	EDMA_TO_SPIDAT1_4_bytes= 0, 	/*EDMA writes 4 bytes to SPIDAT1*/
	EDMA_TO_SPIDAT0_2_bytes, 	/*EDMA writes 2 bytes to SPIDAT0*/
	EDMA_TO_SPIDAT0_1_byte 	/*EDMA writes 1 bytes to SPIDAT0*/
}SPI_EDMA_Dest_Width;

typedef struct  {
    /*Delay between transmissions in ns. Idle time that will be 
    applied at the end of the current transmission if the bit
    WDEL is set in the current buffer*/
    Uint32              delayBetweenTrans_ns;   
    SPI_Shift_Direction ShifDirection; 
    /*No C2TDELAY or T2CDELAY is inserted in the chip select timings.*/
    Uint8               disable_CS_timing;
    SPI_Clock_Polarity  clockPolarity;
    Uint8               clockPhase;
    /*SPI external clock speed in KHz < 66,000KHz*/
    Uint32              clockSpeedKHz;
    /*SPI data word length. Legal values are 2h (data word length = 2 bit)
    to 10h (data word length = 16).*/
    Uint8               wordLength;
} SPI_Data_Format;

typedef struct  {
    Bool    TX_interruptEnable;
    Bool    RX_interruptEnable;
    Bool    overrunInterruptEnable;
    Bool    bitErrorInterruptEnable;
    Uint8   TX_INT_map;
    Uint8   RX_INT_map;
    Uint8   overrun_INT_map;
    Uint8   bitError_INT_map;
} SPI_Interrupt_Config;

typedef struct  {

	SPI_Data_Format * dataFormat[4];
	Bool loopBack;
	Bool DMA_requestEnable;
	SPI_Interrupt_Config * interruptCfg;
	/*number of SPI pins defined as:
	3: SPISIMO, SPISOMI, SPICLK
	4: SPISIMO, SPISOMI, SPICLK, SPISCS0
	5: SPISIMO, SPISOMI, SPICLK, SPISCS0, SPISCS1
	6: SPISIMO, SPISOMI, SPICLK, SPISCS0, SPISCS1, SPISCS2
	......	*/
	Uint8 number_SPI_pins;
	/*Chip-select-active-to-transmit-start-delay. It defines a setup time 
	for the slave device that delays the data transmission from the chip 
	select active edge, in ns*/
	Uint32 C2T_delay_ns;
	/*Transmit-end-to-chip-select-inactive-delay. It defines a hold time 
	for the slave device that delays the chip select deactivation (in ns)
	after the last bit is transferred*/
	Uint32 T2C_delay_ns;
} SPI_Config;

/*SPI transfer parameters*/
typedef struct  {
	Uint32 CS_select;    /*Chip select number*/
	Uint32 formatSelect; /*select one of the 4 formats*/
	SPI_CS_Hold CS_hold; /*hold CS between multiple words*/
	Bool delayEnable; /*Enable the delay counter at the end of the current transaction*/
	Uint32 byteOfWord;   /*number of bytes per SPI word*/
}SPI_Transfer_Param;



extern CSL_SpiRegs * gpSPI_regs;

/*make the higher 16-bit of the SPIDAT1 register*/
#define SPI_TRANSMIT_FORMAT_MAKE(CS_hold_enable, delay_enable, formatSelect, CS_select)\
(Uint32)(\
     CSL_FMKR(28,28,CS_hold_enable) \
    |CSL_FMKR(26,26,delay_enable) \
    |CSL_FMKR(25,24,formatSelect) \
    |CSL_FMKR(23,16,~(1<<CS_select))) 

extern void KeyStone_SPI_init(SPI_Config *spiCfg);

/*wait a flag in SPIFLG, 
retun 1 when the SPIFLG&flag_mask=expect, return 0 when timeout*/
extern Int32 KeyStone_SPI_wait_flag(Uint32 flag_mask, Uint32 expect);

/*Since SPI TX and RX shares same clock and control signals, SPI TX and RX 
actually happens at the same time even you only want to TX or RX. But, 
application may only care for TX or RX, or part of the TX and RX words. 
This function only TX valid data from the "firstTxWord" to "lastTxWord" words, 
the valid data are from the "txBuf"; 
only RX valid data from the "firstRxWord" to "lastRxWord" words, 
the valid data are stored to the "rxBuf".
The transfer format is control with "formatControl", its high 16 bits 
is the higher 16 bit for SPIDAT1 register.
Return number of successful words*/
extern Uint32 KeyStone_SPI_TxRx(Uint8 * txBuf, Uint32 firstTxByte, 
	Uint32 numTxByte, Uint8 * rxBuf, Uint32 firstRxByte, Uint32 numRxByte, 
	SPI_Transfer_Param * transfer);

/*set higher 16 bit of SPIDAT1.
input: transfer parameters structure pointer*/
extern void KeyStone_set_SPIDAT1_format(SPI_Transfer_Param * transfer);

/*attach higher 16-bit to orignal data in "txBuf" to constuct data according to 
SPIDAT1 register format, the output is saved in "SPIDAT1_buf".
Normally, this is used to prepare data for EDMA transfer to SPIDAT1*/
extern void KeyStone_SPIDAT1_buffer_construct(Uint8 * txBuf, Uint32 * SPIDAT1_buf, 
	Uint32 numTxWord, SPI_Transfer_Param * transfer);

/*Use EDMA to transmit and receive SPI data.
Since SPI TX and RX shares same clock and control signals, SPI TX and RX 
actually happens at the same time even you only want to TX or RX, or part of them. 
Since EDMA can not skip data, there may be unused data received at the beginning
of "rxDataBuf", it is the caller's responsibility to skip those unused data.
EDMA RX interrupt is enabled, which may be used by caller to determine the completion*/
extern void KeyStone_SPI_EDMA_TxRx(Uint8 * txDataBuf,
	Uint8 * rxDataBuf, Uint32 numDataWord, Uint32 byteOfWord, 
	SPI_EDMA_Dest_Width EDMA_dest);
	
#endif
