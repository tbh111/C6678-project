/*  ============================================================================
 *   Copyright (c) Texas Instruments Inc 2012
 *
 *   Use of this software is controlled by the terms and conditions found in the
 *   license agreement under which this software has been supplied.
 *   ===========================================================================
 */
/** ============================================================================
 example for I2C initialization and read/write driver on KeyStone DSP
 * =============================================================================
 *  Revision History
 *  ===============
 *  Jan 9, 2012 Brighton Feng  file created
 *  Aug 13, 2013 Brighton Feng  add loopback test code
 *  Dec 13, 2014 Brighton Feng  add KeyStone_I2C_read_follow_write()
 * =============================================================================*/

#include <stdio.h>
#include <string.h>
#include <csl_semaux.h>
#include "KeyStone_common.h"
#include "KeyStone_I2C_init_drv.h"

CSL_I2cRegs * gpI2C_regs = (CSL_I2cRegs *)CSL_I2C_DATA_CONTROL_REGS;

/*I2C output clock <= 400 KHz*/
Uint32 I2C_speed_KHz= 400;

/*Initialize I2C as master*/
void I2C_Master_Init(Uint32 i2c_speed_KHz)
{
	Uint32 module_speed_Hz, clk_div, actualSpeed_KHz;
	if(i2c_speed_KHz>400)
	{
		puts("ERROR: I2C speed can not be higher than 400KHz!");
		return;
	}

	/*Place I2C in reset (clear IRS = 0 in ICMDR).
	The prescaler must be initialized only while the I2C module is in the reset state*/
	gpI2C_regs->ICMDR= 0;
	
	I2C_speed_KHz= i2c_speed_KHz;

	/*I2C internal input clock is (DSP core clock)/6,
	it should be Prescale to 7~12MHz for I2C internal working clock*/
	gpI2C_regs->ICPSC= gDSP_Core_Speed_Hz/6/(1000*I2C_MODULE_FREQ_KHZ)+1;

	module_speed_Hz= gDSP_Core_Speed_Hz/6/gpI2C_regs->ICPSC;
	clk_div= module_speed_Hz/(i2c_speed_KHz*1000)+1;
	actualSpeed_KHz= module_speed_Hz/clk_div/1000;
	if(actualSpeed_KHz!=i2c_speed_KHz)
		printf("I2C expected speed = %dKHz, effective speed = %dKHz\n",
			i2c_speed_KHz, actualSpeed_KHz);

	/*I2C output clock <= 400 KHz*/
	gpI2C_regs->ICCLKL= clk_div/2-6;
	gpI2C_regs->ICCLKH= (clk_div-clk_div/2)-6;

	/*Master mode. The I2C is a master and generates the serial clock on the SCL pin.*/
	gpI2C_regs->ICMDR= gpI2C_regs->ICMDR|
		(1<<CSL_I2C_ICMDR_MST_SHIFT)|
		(1<<CSL_I2C_ICMDR_FREE_SHIFT);

	/*Take I2C controller out of reset: 
	enable I2C controller (set IRS bit = 1 in ICMDR).*/
	gpI2C_regs->ICMDR= gpI2C_regs->ICMDR|
		(1<<CSL_I2C_ICMDR_IRS_SHIFT);

}

/*I2C read/write operations can not re-enter, so before any I2C operation, 
this function should be called to block other I2C operations*/
void I2C_block()
{
	while(0==CSL_semAcquireDirect(SEMAPHORE_FOR_I2C));

	/*in real system indirect request may be used and then switch to ohter tasks
	when the I2C is occupied by other module. And interrupt may be used to
	switch back to this task.*/
}

/*after complete an I2C operation, free I2C for other operations*/
void I2C_free()
{
	CSL_semReleaseSemaphore(SEMAPHORE_FOR_I2C); 
}

char * I2C_stat_str[]=
{
	"Arbitration-lost",
	"NACK received",
	"Register-access-ready",
	"Receive-data-ready",
	"Transmit-ready",
	"Stop condition detected",
	"Reserved-6",
	"Reserved-7",
	"address of all zeros",
	"Addressed-as-slave",
	"Transmit Underflow",
	"Receive overflow",
	"Bus busy",
	"NACK sent",
	"slave-transmitter"
};

void I2C_print_stat(Uint32 stat)
{
	int i;
	for(i=0; i<15; i++)
	{
		if((stat>>i)&1)
			printf("  %s\n", I2C_stat_str[i]);
	}
}

/*wait a flag in ICSTR, 
retun 1 when the ICSTR&flag_mask=expect, return 0 when timeout*/
Int32 I2C_wait_flag(Uint32 flag_mask, Uint32 expect)
{
	Uint32 startTSC;
	Uint32 delay;
	Uint32 theoryCycleForOneByte;
	volatile Uint32 flag;

	startTSC= TSCL;
	theoryCycleForOneByte= (gDSP_Core_Speed_Hz/1000/I2C_speed_KHz)*9;

	flag= gpI2C_regs->ICSTR&flag_mask;

	/*Wait until I2C flag= expect value*/
	while(flag!= expect)
	{
		/*if wait time is much larger than theoretical transfer time of
		a byte, then it is looked as timeout.*/		
		delay= TSC_getDelay(startTSC);
		if(delay> 100*theoryCycleForOneByte)
		{
			printf("I2C wait following flag timeout at device 0x%x:\n", gpI2C_regs->ICSAR);
			I2C_print_stat(flag_mask);
			puts("Current I2C state is:");
			I2C_print_stat(gpI2C_regs->ICSTR);

			/*reset and reinitialize I2C when timeout happens*/
			gpI2C_regs->ICMDR= 0;
			I2C_Master_Init(I2C_speed_KHz);

			return 0;
		}
		flag= gpI2C_regs->ICSTR&flag_mask;

	};

	return 1;
}

/*read one byte "data" from I2C*/
Uint32 I2C_read_one_byte(Uint8 * dataPointer)
{
	/*Wait until data is received (ICRRDY = 1 in ICSTR).*/
	if(0==I2C_wait_flag(CSL_I2C_ICSTR_ICRRDY_MASK, CSL_I2C_ICSTR_ICRRDY_MASK))
	{
		/*after complete an I2C operation, free I2C for other operations*/
		I2C_free();
		return 0;
	}	
	
	/*read data from ICDRR.*/
	*dataPointer=gpI2C_regs->ICDRR;

	return 1;
}

/*read "uiByteCount" data from I2C device with "slaveAddress",
data save in buffer pointed by "ucBuffer".
if "wait", polling until data trasfer complete, otherwise, let interrupt
handle the data.
return number of bytes received.*/
Uint32 I2C_read( Uint32 slaveAddress, Uint32 uiByteCount, 
	Uint8 * ucBuffer, I2C_Wait wait)
{
	int i;

	/*I2C read/write operations can not re-enter, so before any I2C operation, 
	this function should be called to block other I2C operations*/
	I2C_block();
	
	/*Place I2C in reset (clear IRS = 0 in ICMDR)*/
	gpI2C_regs->ICMDR= 0;
	
	/*Configure I2C as Master (MST = 1)
	Data Receiver (TRX = 0)*/
	gpI2C_regs->ICMDR= 
		(1<<CSL_I2C_ICMDR_FREE_SHIFT)|
		(1<<CSL_I2C_ICMDR_MST_SHIFT);
		
	/*Take I2C controller out of reset: enable I2C controller (set IRS bit = 1 in ICMDR).*/
	gpI2C_regs->ICMDR= gpI2C_regs->ICMDR| 
		(1<<CSL_I2C_ICMDR_IRS_SHIFT);

	gpI2C_regs->ICCNT= uiByteCount;
	gpI2C_regs->ICSAR= slaveAddress;

	/*Make sure the interrupt status register (ICSTR) is cleared*/
	/*Read ICSTR and write it back (write 1 to clear) ICSTR = ICSTR*/
	gpI2C_regs->ICSTR= gpI2C_regs->ICSTR;
	/*Read ICIVR until it is zero*/
	while(gpI2C_regs->ICIVR);

	/*Wait until bus busy bit is cleared (BB = 0 in ICSTR).*/
	if(0==I2C_wait_flag(CSL_I2C_ICSTR_BB_MASK, 0))
	{
		/*after complete an I2C operation, free I2C for other operations*/
		I2C_free();
		return 0;
	}	
	/*Generate a START event(set STT = 1 in ICMDR).*/
	/*End transfer/release bus when transfer is done. 
	Generate a STOP event (set STP = 1 in ICMDR).*/
	gpI2C_regs->ICMDR= gpI2C_regs->ICMDR| 
		(1<<CSL_I2C_ICMDR_STT_SHIFT)|
		(1<<CSL_I2C_ICMDR_STP_SHIFT);

	if(I2C_NOWAIT==wait)
	{
		/*exits after programmation of the control registers,
		interrupt service routine should be used to handle the data.*/
		return 0;
	}

	for(i= 0; i< uiByteCount; i++)
	{
		if(0==I2C_read_one_byte(&ucBuffer[i]))
			return 0;
	}

	/*Wait until bus busy bit is cleared (BB = 0 in ICSTR).*/
	if(0==I2C_wait_flag(CSL_I2C_ICSTR_BB_MASK, 0))
		uiByteCount= 0;

	/*after complete an I2C operation, free I2C for other operations*/
	I2C_free(); 

	return uiByteCount;
}


/*write one byte "data" to I2C*/
Uint32 I2C_write_one_byte( Uint8 data)
{
	/*Wait until transmit is ready (ICXRDY = 1 in ICSTR).*/
	if(0==I2C_wait_flag(CSL_I2C_ICSTR_ICXRDY_MASK, CSL_I2C_ICSTR_ICXRDY_MASK))
	{
		/*I2C operation timeout, free I2C for other operations*/
		I2C_free(); 
		return 0;
	}	
	
	/*transmit data to ICDXR.*/
	gpI2C_regs->ICDXR= data;

	return 1;
}

/*transfer "uiByteCount" data from "ucBuffer" to I2C device with address
"slaveAddress". if "wait", polling until data trasfer complete, otherwise, 
let interrupt handle the data.
return number of bytes transfered.*/
Uint32 I2C_write( Uint32 slaveAddress, Uint32 uiByteCount, 
	Uint8 * ucBuffer, I2C_Wait wait)
{
	int i;

	/*I2C read/write operations can not re-enter, so before any I2C operation, 
	this function should be called to block other I2C operations*/
	I2C_block();
	
	/*Place I2C in reset (clear IRS = 0 in ICMDR)*/
	gpI2C_regs->ICMDR= 0;
	
	/*Configure I2C as Master (MST = 1)
	Data Transmiter (TRX = 1)*/
	gpI2C_regs->ICMDR= 
		(1<<CSL_I2C_ICMDR_FREE_SHIFT)|
		(1<<CSL_I2C_ICMDR_TRX_SHIFT)|
		(1<<CSL_I2C_ICMDR_MST_SHIFT);
		
	/*Take I2C controller out of reset: enable I2C controller (set IRS bit = 1 in ICMDR).*/
	gpI2C_regs->ICMDR= gpI2C_regs->ICMDR| 
		(1<<CSL_I2C_ICMDR_IRS_SHIFT);

	gpI2C_regs->ICCNT= uiByteCount;
	gpI2C_regs->ICSAR= slaveAddress;

	/*Make sure the interrupt status register (ICSTR) is cleared*/
	/*Read ICSTR and write it back (write 1 to clear) ICSTR = ICSTR*/
	gpI2C_regs->ICSTR= gpI2C_regs->ICSTR;
	/*Read ICIVR until it is zero*/
	while(gpI2C_regs->ICIVR);


	/*Wait until bus busy bit is cleared (BB = 0 in ICSTR).*/
	if(0==I2C_wait_flag(CSL_I2C_ICSTR_BB_MASK, 0))
	{
		/*after complete an I2C operation, free I2C for other operations*/
		I2C_free(); 	
		return 0;
	}	
	
	/*write the first byte to ICDXR.*/
	gpI2C_regs->ICDXR= ucBuffer[0];

	/*Generate a START event(set STT = 1 in ICMDR).*/
	/*End transfer/release bus when transfer is done. 
	Generate a STOP event (set STP = 1 in ICMDR).*/
	gpI2C_regs->ICMDR= gpI2C_regs->ICMDR| 
		(1<<CSL_I2C_ICMDR_STT_SHIFT)|
		(1<<CSL_I2C_ICMDR_STP_SHIFT);

	if(I2C_NOWAIT==wait)
	{
		/*exits after programmation of the control registers,
		interrupt service routine should be used to handle the data.*/
		return 0;
	}

	for(i= 1; i< uiByteCount; i++)
	{
		if(0==I2C_write_one_byte(ucBuffer[i]))
			return 0;
	}

	/*Wait until bus busy bit is cleared (BB = 0 in ICSTR).*/
	if(0==I2C_wait_flag(CSL_I2C_ICSTR_BB_MASK, 0))
		uiByteCount= 0;

	/*after complete an I2C operation, free I2C for other operations*/
	I2C_free(); 
	
	return uiByteCount;
}


/*write fistly, and then read, without stop between write and read.
If "wait", polling until data trasfer complete, otherwise, let interrupt
handle the data.
return number of bytes received.*/
Uint32 KeyStone_I2C_read_follow_write(Uint32 slaveAddress, Uint32 uiNumWriteBytes, 
	Uint8 * ucWriteBuffer, Uint32 uiNumReadBytes, Uint8 * ucReadBuffer)
{
	int i;
	Uint32 uiByteCount;

	/*I2C read/write operations can not re-enter, so before any I2C operation, 
	this function should be called to block other I2C operations*/
	I2C_block();
	
	/*Place I2C in reset (clear IRS = 0 in ICMDR)*/
	gpI2C_regs->ICMDR= 0;
	
	/*----------------------------------write-------------------------------*/
	/*Configure I2C as Master (MST = 1)
	Data Transmiter (TRX = 1)
	repeat mod (RM = 1)*/
	gpI2C_regs->ICMDR=  
		(1<<CSL_I2C_ICMDR_FREE_SHIFT)|
		(1<<CSL_I2C_ICMDR_TRX_SHIFT)|
		(1<<CSL_I2C_ICMDR_RM_SHIFT)| 	
		(1<<CSL_I2C_ICMDR_MST_SHIFT);
		
	/*Take I2C controller out of reset: enable I2C controller (set IRS bit = 1 in ICMDR).*/
	gpI2C_regs->ICMDR= gpI2C_regs->ICMDR| 
		(1<<CSL_I2C_ICMDR_IRS_SHIFT);

	gpI2C_regs->ICSAR= slaveAddress;

	/*Make sure the interrupt status register (ICSTR) is cleared*/
	/*Read ICSTR and write it back (write 1 to clear) ICSTR = ICSTR*/
	gpI2C_regs->ICSTR= gpI2C_regs->ICSTR;
	/*Read ICIVR until it is zero*/
	while(gpI2C_regs->ICIVR);

	/*Wait until bus busy bit is cleared (BB = 0 in ICSTR).*/
	if(0==I2C_wait_flag(CSL_I2C_ICSTR_BB_MASK, 0))
	{
		/*after complete an I2C operation, free I2C for other operations*/
		I2C_free(); 	
		return 0;
	}	
	
	/*write the first byte to ICDXR.*/
	gpI2C_regs->ICDXR= ucWriteBuffer[0];

	/*Generate a START event(set STT = 1 in ICMDR).*/
	gpI2C_regs->ICMDR= gpI2C_regs->ICMDR| 
		(1<<CSL_I2C_ICMDR_STT_SHIFT);

	for(i= 1; i< uiNumWriteBytes; i++)
	{
		if(0==I2C_write_one_byte(ucWriteBuffer[i]))
			return 0;
	}

	/*Wait until transmit is ready (ICXRDY = 1 in ICSTR).*/
	if(0==I2C_wait_flag(CSL_I2C_ICSTR_ICXRDY_MASK, CSL_I2C_ICSTR_ICXRDY_MASK))
	{
		/*I2C operation timeout, free I2C for other operations*/
		I2C_free(); 
		return 0;
	}	

	/*----------------------------------read-------------------------------*/
	/* In master mode, STT is automatically cleared after the START condition has been generated*/
	/*Generate a repeat START event(set STT = 1 in ICMDR).*/
	/*Configure I2C as Master (MST = 1)
	Data Receiver (TRX = 0)*/
	gpI2C_regs->ICMDR= 
		(1<<CSL_I2C_ICMDR_FREE_SHIFT)|
		(1<<CSL_I2C_ICMDR_STT_SHIFT)|
		(1<<CSL_I2C_ICMDR_RM_SHIFT)| 	
		(1<<CSL_I2C_ICMDR_MST_SHIFT)|
		(1<<CSL_I2C_ICMDR_IRS_SHIFT);
		
	for(i= 0; i< uiNumReadBytes-1; i++)
	{
		if(0==I2C_read_one_byte(&ucReadBuffer[i]))
			return 0;
	}

	/* In master mode, STT is automatically cleared after the START condition has been generated*/
	/*Generate a STOP event(set STT = 1 in ICMDR).*/
	while(gpI2C_regs->ICMDR&CSL_I2C_ICMDR_STT_MASK); //wait start complete
	gpI2C_regs->ICMDR |=
		(1<<CSL_I2C_ICMDR_STP_SHIFT);
	
	/*Wait until data is received (ICRRDY = 1 in ICSTR).*/
	if(0==I2C_wait_flag(CSL_I2C_ICSTR_ICRRDY_MASK, CSL_I2C_ICSTR_ICRRDY_MASK))
	{
		/*after complete an I2C operation, free I2C for other operations*/
		I2C_free();
		return 0;
	}	
	
	/*read last byte from ICDRR.*/
	ucReadBuffer[uiNumReadBytes-1]=gpI2C_regs->ICDRR;

	uiByteCount= uiNumReadBytes;

	/*Wait until bus busy bit is cleared (BB = 0 in ICSTR).*/
	if(0==I2C_wait_flag(CSL_I2C_ICSTR_BB_MASK, 0))
		uiByteCount= 0;

	/*after complete an I2C operation, free I2C for other operations*/
	I2C_free(); 

	return uiByteCount;
}

/*transfer "uiByteCount" data from "ucTxBuffer" to I2C, 
loopback data is received intot the "uxRxBuffer"
return number of bytes transfered.*/
Uint32 I2C_loopback( Uint32 slaveAddress, Uint32 uiByteCount, 
	Uint8 * ucTxBuffer, Uint8 * ucRxBuffer)
{
	int i;

	/*I2C read/write operations can not re-enter, so before any I2C operation, 
	this function should be called to block other I2C operations*/
	I2C_block();
	
	/*Place I2C in reset (clear IRS = 0 in ICMDR)*/
	gpI2C_regs->ICMDR= 0;
	
	/*Configure I2C as Master (MST = 1)
	Data Receiver (TRX = 0)*/
	gpI2C_regs->ICMDR= 
		(1<<CSL_I2C_ICMDR_FREE_SHIFT)|
		(1<<CSL_I2C_ICMDR_TRX_SHIFT)|
		(1<<CSL_I2C_ICMDR_DLB_SHIFT)|
		(1<<CSL_I2C_ICMDR_MST_SHIFT);
		
	gpI2C_regs->ICCNT= uiByteCount;
	gpI2C_regs->ICSAR= slaveAddress;
	gpI2C_regs->ICOAR= slaveAddress;

	/*Make sure the interrupt status register (ICSTR) is cleared*/
	/*Read ICSTR and write it back (write 1 to clear) ICSTR = ICSTR*/
	gpI2C_regs->ICSTR= gpI2C_regs->ICSTR;
	/*Read ICIVR until it is zero*/
	while(gpI2C_regs->ICIVR);

	/*Take I2C controller out of reset: enable I2C controller (set IRS bit = 1 in ICMDR).*/
	gpI2C_regs->ICMDR= gpI2C_regs->ICMDR| 
		(1<<CSL_I2C_ICMDR_IRS_SHIFT);

	/*Wait until bus busy bit is cleared (BB = 0 in ICSTR).*/
	if(0==I2C_wait_flag(CSL_I2C_ICSTR_BB_MASK, 0))
	{
		/*after complete an I2C operation, free I2C for other operations*/
		I2C_free(); 	
		return 0;
	}	
	
	/*write the first byte to ICDXR.*/
	gpI2C_regs->ICDXR= ucTxBuffer[0];

	/*Generate a START event(set STT = 1 in ICMDR).*/
	/*End transfer/release bus when transfer is done. 
	Generate a STOP event (set STP = 1 in ICMDR).*/
	gpI2C_regs->ICMDR= gpI2C_regs->ICMDR| 
		(1<<CSL_I2C_ICMDR_STT_SHIFT)|
		(1<<CSL_I2C_ICMDR_STP_SHIFT);

	//write byte N and read byte N-1	
	for(i= 1; i< uiByteCount; i++)
	{
		if(0==I2C_write_one_byte(ucTxBuffer[i]))
			return i-1;
		if(0==I2C_read_one_byte(&ucRxBuffer[i-1]))
			return i-1;
	}
	
	//read last byte
	if(0==I2C_read_one_byte(&ucRxBuffer[uiByteCount-1]))
		return 0;

	/*Wait until bus busy bit is cleared (BB = 0 in ICSTR).*/
	if(0==I2C_wait_flag(CSL_I2C_ICSTR_BB_MASK, 0))
		uiByteCount= 0;

	/*after complete an I2C operation, free I2C for other operations*/
	I2C_free(); 
	
	return uiByteCount;
}

