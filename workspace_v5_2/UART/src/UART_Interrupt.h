/******************************************************************************

  Copyright (C), 2013, Texas Instrument.

 ******************************************************************************
  File Name     : UART_Interrupt.h
  Version       : Initial Draft
  Author        : Vincent Han
  Created       : 2013/5/17
  Last Modified :
  Description   :  UART interrupt setup and handling 

  History       :
  1.Date        : May 17, 2013
    Author      : Vincent Han
    Modification: Created file for KeyStone
  2.Date        : July 27, 2013
    Author      : Brighton Feng
    Modification: code clean up

******************************************************************************/
#ifndef _UART_INTERRUPT_H_
#define _UART_INTERRUPT_H_

#define RX_BUF_BYTE_SIZE    (16)

extern volatile Bool bUartEchoBack; //Echo the received data back

extern void KeyStone_UART_Interrupts_Init(Bool bRxIntEnable, Bool bEDMAInterruptEnable);

#endif
