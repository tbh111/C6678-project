/******************************************************************************

  Copyright (C), 2013, Texas Instrument.

 ******************************************************************************
  File Name     : KeyStone_UART_Init_drv.h
  Version       : Initial Draft
  Author        : Vincent Han
  Created       : 2013/5/17
  Last Modified :
  Description   :  UART configuration and driver on KeyStone 

  History       :
  1.Date        : May 17, 2013
    Author      : Vincent Han
    Modification: Created file for KeyStone
  2.Date        : July 27, 2013
    Author      : Brighton Feng
    Modification: update APIs

******************************************************************************/
#ifndef _KEYSTONE_UART_INIT_H_
#define _KEYSTONE_UART_INIT_H_

#include <cslr_device.h>
#include <cslr_uart.h>
#include <tistdtypes.h>
#include "KeyStone_common.h"

/*the master used for TX*/
typedef enum {
	UART_USE_CORE_TO_TX = 0,
	UART_USE_EDMA_TO_TX = 1
}UART_Tx_Master;

typedef enum {
	PARITY_DISABLE = 0,
	ODD_PARITY_ENABLE_SET1 = 1,
	EVEN_PARITY_ENABLE_SET1 = 2,
	STICK_PARITY_ENABLE_SET = 3,
	STICK_PARITY_ENABLE_CLR = 4
}UART_parityMode;

typedef enum{
	ONE_STOP_BIT = CSL_UART_LCR_STB_1BIT,
	WLS_STOP_BIT = CSL_UART_LCR_STB_WLS
}UART_stopMode;

typedef enum{
	
	DATA_LEN_5BIT = CSL_UART_LCR_WLS_5BITS,
	DATA_LEN_6BIT = CSL_UART_LCR_WLS_6BITS,
	DATA_LEN_7BIT = CSL_UART_LCR_WLS_7BITS,
	DATA_LEN_8BIT = CSL_UART_LCR_WLS_8BITS
}UART_dataLength;

typedef enum{	
	AUTO_FLOW_DIS = 0,
	AUTO_FLOW_CTS_EN = 1,
	AUTO_FLOW_RTS_CTS_EN = 2
}UART_autoFlow;

typedef enum{
	OVER_SAMPLING_16X = CSL_UART_MDR_OSM_SEL_16XOVERSAMPLING,
	OVER_SAMPLING_13X = CSL_UART_MDR_OSM_SEL_13XOVERSAMPLING
}UART_overSampling;

typedef enum{
	TRIGGER_LEVEL_1BYTE = CSL_UART_FCR_RXFIFTL_CHAR1,
	TRIGGER_LEVEL_4BYTE = CSL_UART_FCR_RXFIFTL_CHAR4,
	TRIGGER_LEVEL_8BYTE = CSL_UART_FCR_RXFIFTL_CHAR8,
	TRIGGER_LEVEL_14BYTE = CSL_UART_FCR_RXFIFTL_CHAR14
}UART_triLevel;
typedef struct{
	Uint32 				baudRate;
	Uint32 				DSP_Core_Speed_Hz;
	UART_parityMode 	parityMode;
	UART_stopMode   	stopMode;
	UART_dataLength 	dataLen;
	UART_autoFlow   	autoFlow;	
	UART_overSampling	osmSel;
	UART_triLevel   	fifoRxTriBytes;
    UART_Tx_Master      txMaster; 	//use EDMA or core to transmit
	Bool 				bLoopBackEnable;
}UART_Config;

extern EDMA_CC_Chanel_TC UART_EDMA_TX_channels[CSL_UART_PER_CNT];
extern CSL_UartRegs * gpUartRegs[CSL_UART_PER_CNT];

extern void KeyStone_UART_init(UART_Config *pUARTCfg, Uint32 uartNum);

/*write number of "byteLen" data in the "buffer" through UART of "uartNum".
EDMA or DSP core is used depends on the "txMaster" configuration*/
extern void KeyStone_UART_write(unsigned char *buffer, 
	unsigned int byteLen, Uint32 uartNum);

//wait for TX complete
extern void KeyStone_UART_TX_wait(Uint32 uartNum);

/*read data from the UART RX FIFO to the receive "buffer".
Return when FIFO is empty or the receive "buffer" is full.
Number of byte read is returned*/
extern unsigned int KeyStone_UART_read(unsigned char *buffer, 
	unsigned int buffByteLen, Uint32 uartNum);

//check and print UART error status
extern void KeyStone_UART_Error_Handler(Uint32 uartNum);

#endif

