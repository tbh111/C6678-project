/* --------------------------------------------------------------------------
  FILE        : util.h
  PURPOSE     : Misc. utility header file
  PROJECT     : DaVinci Flashing Utilities
  AUTHOR      : Daniel Allred
  DESC        : Header for FLASH module
 ----------------------------------------------------------------------------- */ 

#ifndef _FLASH_H_
#define _FLASH_H_

#include <tistdtypes.h>

// Prevent C++ name mangling
#ifdef __cplusplus
extern far "c" {
#endif

/***********************************************************
* Global Macro Declarations                                *
***********************************************************/
#ifndef CSL_EMIF16_data_REGS
#define CSL_EMIF16_data_REGS             (0x70000000)
#endif
#define DEVICE_EMIF_INTER_CE_REGION_SIZE  (0x4000000)
#define DEVICE_EMIF_NUMBER_CE_REGION		(4)

#define FLASH_BUS_WIDTH_1_BYTE 		1
#define FLASH_BUS_WIDTH_2_BYTES 	2

#ifndef E_PASS
  #define E_PASS    (0x00000000u)
#endif
#ifndef E_FAIL
  #define E_FAIL    (0x00000001u)
#endif
#ifndef E_TIMEOUT
  #define E_TIMEOUT (0x00000002u)
#endif

 
/***********************************************************
* Global Typedef declarations                              *
***********************************************************/


/***********************************************************
* Global Function Declarations                             *
***********************************************************/


/***********************************************************
* End file                                                 *
***********************************************************/

#ifdef __cplusplus
}
#endif

#endif //_UTIL_H_

/* --------------------------------------------------------------------------
  HISTORY
	  v1.00  -  DJA  -  07-Nov-2007
      Initial release 
 ----------------------------------------------------------------------------- */

