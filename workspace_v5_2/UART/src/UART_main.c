/******************************************************************************

  Copyright (C), 2013, Texas Instrument.

 ******************************************************************************
  File Name     : UART_main.c
  Version       : Initial Draft
  Author        : Vincent Han
  Created       : 2013/5/17
  Last Modified :
  Description   :  UART test code 

  History       :
  1.Date        : May 17, 2013
    Author      : Vincent Han
    Modification: Created file for KeyStone
  2.Date        : July 27, 2013
    Author      : Brighton Feng
    Modification: code clean up

******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <csl_bootcfgAux.h>
#include <csl_edma3.h>
#include <csl_edma3.h>
#include "KeyStone_common.h"
#include "KeyStone_UART_Init_drv.h"
#include "UART_interrupt.h"

//the number of the UART be tested
#define TEST_UART_NUM 	0

//default baud rate used for tests except for loopback
#define DEFAULT_BAUD_RATE 115200

/*internal loopback test
comment following line to disable this test*/
#define UART_INTERNAL_LOOPBACK_TEST

/*external loopback test, user should connect the TX pin and RX pin 
on their board before run this test. 
comment following line to disable this test*/
//#define UART_EXTERNAL_LINK_LOOPBACK_TEST

/*send character code from 0 to 255.
comment following line to disable this test*/
#define UART_TX_INCREASE_SEQUENCE

/*define for continuously transmit fixed data pattern.
normally, used for hardware signal test.
comment following line to disable this test*/
//#define UART_CONTINUE_TX_DATA_PATTERN 	0x55

/*interact with PC, echo back characters from PC or other host.
comment following line to disable this test*/
//#define	UART_INTERACT_WITH_PC

UART_Config    gUARTCfg;

#define UART_TEST_BUF_BYTE_SIZE (1024)

#pragma DATA_ALIGN(UART_Test_Rx_Buf,16);
unsigned char UART_Test_Rx_Buf[UART_TEST_BUF_BYTE_SIZE]; /* Need to be the same with Rx FIFO trigger level size */
#pragma DATA_ALIGN(UART_Test_Tx_Buf,16);
unsigned char UART_Test_Tx_Buf[UART_TEST_BUF_BYTE_SIZE];

unsigned char info[] = {"Please input on keyboard of your PC: \r\n"};

char * parityStr[]=
{
    "Parity Disable", "Odd Parity Enable Set1", "Even Parity Enable Set1", "Stick Parity Enable Set", "Stick Parity Enable Clear"
};
char * flowCtrlStr[]=
{
    "Auto Flow Control Disabled", "Auto Flow Control CTS Enabled", "Auto Flow Control CTS&RTS Enabled"
};

char * dataLenStr[]=
{
    "5", "6", "7", "8"
};

char * stopBitStr[]=
{
    "1 Stop Bit", "WLS Stop Bit"
};

/*UART configure*/
void KeyStone_UART_config(Uint32 baudRate, Bool bLoopBackEnable, 
	UART_Tx_Master txMaster)
{
	int i;
	
	if(bLoopBackEnable)
		printf("\nInit UART%d at %dbps in loopback mode...\n", TEST_UART_NUM, baudRate);
	else
		printf("\nInit UART%d at %dbps...\n", TEST_UART_NUM, baudRate);

	//clear the configuration data structure
	memset(&gUARTCfg, 0, sizeof(gUARTCfg));
	
	gUARTCfg.baudRate = baudRate;
	gUARTCfg.DSP_Core_Speed_Hz = gDSP_Core_Speed_Hz;
	gUARTCfg.dataLen = DATA_LEN_8BIT;
	gUARTCfg.parityMode = PARITY_DISABLE;
	gUARTCfg.stopMode = ONE_STOP_BIT;
    gUARTCfg.autoFlow = AUTO_FLOW_DIS;
	gUARTCfg.osmSel = OVER_SAMPLING_16X;
    gUARTCfg.fifoRxTriBytes = TRIGGER_LEVEL_14BYTE;   
    gUARTCfg.txMaster = txMaster;
	gUARTCfg.bLoopBackEnable = bLoopBackEnable;

	KeyStone_UART_init(&gUARTCfg, TEST_UART_NUM);

	if(gUARTCfg.txMaster == UART_USE_EDMA_TO_TX)
	{
		EDMA_init();
		
		for(i=0; i< CSL_UART_PER_CNT; i++)
		{
			EDMA_channel_TC_cfg(UART_EDMA_TX_channels[i].CC_num, 
				UART_EDMA_TX_channels[i].channel_num, 
				UART_EDMA_TX_channels[i].TC_num);
			EDMA_interrupt_enable(UART_EDMA_TX_channels[i].CC_num, 
				UART_EDMA_TX_channels[i].channel_num);
		}
		//disable EDMA CC2 error (to ignore event miss exception)
		gpCIC0_regs->ENABLE_CLR_INDEX_REG= 16;
	}
}

/*UART loopback test and measure the throughput at different baudrate*/
void KeyStone_UART_loopback_with_throughput(Bool bInternalLoopback)
{
	int i;
    unsigned int rxCount, correct;
    unsigned int startCycle, cycles;
    Uint32 throughput;
    Uint32 baudRate;
    Uint8 testData=0;
    
    CSL_UartRegs *localUartRegs = gpUartRegs[TEST_UART_NUM];

    KeyStone_UART_Interrupts_Init(FALSE, TRUE);

	for(baudRate= DEFAULT_BAUD_RATE; baudRate<5000000; baudRate*=2)
	{
	    KeyStone_UART_config(baudRate, bInternalLoopback, UART_USE_EDMA_TO_TX);

	    /* Fill the Tx and Rx buffer with inversed data*/
#if 1
	    for(i=0; i<UART_TEST_BUF_BYTE_SIZE; i++)
	    {
	    	UART_Test_Tx_Buf[i]= testData+i;
	    	UART_Test_Rx_Buf[i]= ~(testData+i);
	    }
#else
	    memset(UART_Test_Tx_Buf, testData, sizeof(UART_Test_Tx_Buf));
	    memset(UART_Test_Rx_Buf, ~testData, sizeof(UART_Test_Rx_Buf));
#endif

	    /* Send the data through EDMA */
	    KeyStone_UART_write(UART_Test_Tx_Buf, UART_TEST_BUF_BYTE_SIZE, TEST_UART_NUM);

	    startCycle = TSCL;

	    /* poll data from Rx side */
	    rxCount= 0;
	    correct= 0;
	    while(1)
	    {
	        while(!(localUartRegs->LSR & CSL_UART_LSR_DR_MASK))
	        {
				//timeout if the time is more than 4 times of the expected time
		        if(TSC_count_cycle_from(startCycle)>
		        	4*UART_TEST_BUF_BYTE_SIZE*10*(gDSP_Core_Speed_Hz/baudRate))
		        {
		        	printf("UART%d timeout: Tx %d bytes, Rx %d bytes\n", 
		        		TEST_UART_NUM, UART_TEST_BUF_BYTE_SIZE, rxCount);
		        	return;
		        }
	        }	        

	        UART_Test_Rx_Buf[rxCount++] = localUartRegs->RBR;
	        
	        if(rxCount == UART_TEST_BUF_BYTE_SIZE)
	            break;

	    }

	    cycles = TSC_count_cycle_from(startCycle);

		//verfiy the receive data
	    for(i = 0; i < rxCount; i++)
	    {
	        if(UART_Test_Rx_Buf[i] == UART_Test_Tx_Buf[i])
	        {
	            correct++; //count the number of correct data
	        }
	    }

	    throughput = (unsigned long long)rxCount*8*gDSP_Core_Speed_Hz/
	    	((unsigned long long)cycles*1000);

	    printf("UART%d: Tx %d bytes, Rx %d bytes, %d correct bytes. Consumes %d cycles, throughput is %dKbps\n", 
	    	TEST_UART_NUM, UART_TEST_BUF_BYTE_SIZE, rxCount, correct, cycles, throughput);

		testData++; //change test data
	}
}

/*send character code from 0 to 255*/
void UART_TX_Increase_Sequence()
{
	int i;

	//configure for non-loopback test
    KeyStone_UART_config(DEFAULT_BAUD_RATE, FALSE, UART_USE_EDMA_TO_TX);

    KeyStone_UART_Interrupts_Init(TRUE, TRUE);
    
	printf("Start print character code from 0 to 255 over UART%d...\n", TEST_UART_NUM);
	
	for(i=0; i<128; i++)
	{
		UART_Test_Tx_Buf[i]= i;
	}
	UART_Test_Tx_Buf[i]=10; 	//new line
	UART_Test_Tx_Buf[i+1]=13; 	//return
	
	KeyStone_UART_write(UART_Test_Tx_Buf, 128+2, TEST_UART_NUM);
	KeyStone_UART_TX_wait(TEST_UART_NUM);
}

/*continuously transmit fixed data pattern.
normally, used for hardware signal test.*/
void UART_continue_TX_data_pattern(Uint8 dataPattern)
{
	int i;

	//configure for non-loopback test
    KeyStone_UART_config(DEFAULT_BAUD_RATE, FALSE, UART_USE_EDMA_TO_TX);

    KeyStone_UART_Interrupts_Init(TRUE, TRUE);
    
	printf("Start continously transmit data pattern 0x%x over UART%d...\n", 
		dataPattern, TEST_UART_NUM);
	
	for(i=0; i<UART_TEST_BUF_BYTE_SIZE; i++)
	{
		UART_Test_Tx_Buf[i]= dataPattern;
	}

	while(1)	
		KeyStone_UART_write(UART_Test_Tx_Buf, UART_TEST_BUF_BYTE_SIZE, TEST_UART_NUM);
}

/*interact with PC, echo back characters from PC or other host.*/
void UART_Interact_with_PC()
{
	//configure for non-loopback test, and use CPU for TX
    KeyStone_UART_config(DEFAULT_BAUD_RATE, FALSE, UART_USE_CORE_TO_TX);

    KeyStone_UART_Interrupts_Init(TRUE, FALSE);
    
	printf("Start interact with PC or other host through UART%d...\n", TEST_UART_NUM);
	printf("Please connect the Serial cable to your PC with the following configurations:\n");
	printf("Baud rate   : %d \n", gUARTCfg.baudRate);
	printf("Data len    : %s bit \n", dataLenStr[gUARTCfg.dataLen]);
	printf("Parity mode : %s \n", parityStr[gUARTCfg.parityMode]);
	printf("Stop bit    : %s \n", stopBitStr[gUARTCfg.stopMode]);
	printf("Flow control: %s \n", flowCtrlStr[gUARTCfg.autoFlow]);

	KeyStone_UART_write(info, sizeof(info), TEST_UART_NUM);

	printf("Please input on UART input window of your PC:\n");

	/*let UART RX interrupt ISR echos back the characters from PC*/
	bUartEchoBack= TRUE;

	/*wait for input from the UART of PC*/
    while(1); 		
}    

void main(void)
{
	/*enable TSC, memory protection interrupts, EDC for internal RAM;
    clear cache; protect L1 as cache*/
	KeyStone_common_CPU_init();
	/*print device information. 
	Enable memory protection interrupts, EDC for MSMC RAM*/
	KeyStone_common_device_init();

	//enable exception handling
	KeyStone_Exception_cfg(TRUE);

	if(C6678_EVM==gDSP_board_type)
	{
		//DSP core speed: 100*10/1=1000MHz
		KeyStone_main_PLL_init(100, 10, 1); 
	}
	else if(TCI6614_EVM==gDSP_board_type
		||DUAL_NYQUIST_EVM==gDSP_board_type
		||C6670_EVM==gDSP_board_type)
	{
		//DSP core speed: 122.88*236/29= 999.9889655MHz
		KeyStone_main_PLL_init(122.88, 236, 29);
	}
	else
	{
		puts("Unknown DSP board type!");
		return;
	}

#ifdef UART_INTERNAL_LOOPBACK_TEST
    KeyStone_UART_loopback_with_throughput(TRUE);
#endif

#ifdef UART_EXTERNAL_LINK_LOOPBACK_TEST
    KeyStone_UART_loopback_with_throughput(FALSE);
#else

#ifdef UART_TX_INCREASE_SEQUENCE
	UART_TX_Increase_Sequence();
#endif

#ifdef UART_CONTINUE_TX_DATA_PATTERN
	UART_continue_TX_data_pattern(UART_CONTINUE_TX_DATA_PATTERN);
#endif

#ifdef UART_INTERACT_WITH_PC
	UART_Interact_with_PC();
#endif

#endif /* End of ifdef UART_EXTERNAL_LINK_LOOPBACK_TEST */
	puts("UART test complete.");
}

