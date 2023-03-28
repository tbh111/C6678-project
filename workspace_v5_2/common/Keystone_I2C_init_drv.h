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
 * =============================================================================*/
#ifndef _KEYSTONE_I2C_H
#define _KEYSTONE_I2C_H

#include <tistdtypes.h>
#include <cslr_i2c.h>

/*I2C internal clock should be 7~12MHz*/
#define I2C_MODULE_FREQ_KHZ  	10000
/*use a semaphore to access I2C mutexly*/
#define SEMAPHORE_FOR_I2C 		31

extern CSL_I2cRegs * gpI2C_regs;
extern Uint32 I2C_speed_KHz;

typedef enum 
{
    /** No blocking, the call exits after programmation of the 
     *  control registers, interrupt service routine should be used.
     */
    I2C_NOWAIT = 0,
                         
    /** Blocking Call, polling status flag until transfer complete.
     */
    I2C_WAIT   = 1
}I2C_Wait;

/*Initialize I2C as master*/
extern void I2C_Master_Init(Uint32 i2c_speed_KHz);

/*read "uiByteCount" data from I2C device with "slaveAddress",
data save in buffer pointed by "ucBuffer".
if "wait", polling until data trasfer complete, otherwise, let interrupt
handle the data.
return number of bytes received.*/
extern Uint32 I2C_read(Uint32 slaveAddress, Uint32 uiByteCount, 
	Uint8 * ucBuffer, I2C_Wait wait);

/*transfer "uiByteCount" data from "ucBuffer" to I2C device with address
"slaveAddress". if "wait", polling until data trasfer complete, otherwise, 
let interrupt handle the data.
return number of bytes transfered.*/
extern Uint32 I2C_write(Uint32 slaveAddress, Uint32 uiByteCount, 
	Uint8 * ucBuffer, I2C_Wait wait);

/*write fistly, and then read, without stop between write and read.
return number of bytes received.*/
extern Uint32 KeyStone_I2C_read_follow_write(Uint32 slaveAddress, 
	Uint32 uiNumWriteBytes, Uint8 * ucWriteBuffer, 
	Uint32 uiNumReadBytes, Uint8 * ucReadBuffer);

/*transfer "uiByteCount" data from "ucTxBuffer" to I2C, 
loopback data is received intot the "uxRxBuffer"
return number of bytes transfered.*/
extern Uint32 I2C_loopback(Uint32 slaveAddress, Uint32 uiByteCount, 
	Uint8 * ucTxBuffer, Uint8 * ucRxBuffer);
	
#endif

