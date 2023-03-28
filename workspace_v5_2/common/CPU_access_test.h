/******************************************************************************

  Copyright (C), 2014, Texas Instrument.

 ******************************************************************************
  File Name     : CPU_access_test.h
  Version       : Initial Draft
  Author        : Brighton Feng
  Created       : June 5, 2014
  Last Modified : 
  Description   : CPU read/write test functions and definitions
  History       :
  1.Date        : June 5, 2014
    Author      : Brighton Feng
    Modification: Created file

******************************************************************************/
#ifndef _CPU_ACCESS_TEST_H_
#define _CPU_ACCESS_TEST_H_

#include <tistdtypes.h>

/************Assembley test code implemented in "CPU_LD_ST_test.asm***************
access "buff" with LDDW, STDW, LDW or STW instruction for "uiCount" times*/
extern void Asm_LDDW_Test(Uint32 buff_addr, Uint32 uiIndex, Uint32 uiCount);
extern void Asm_STDW_Test(Uint32 buff_addr, Uint32 uiIndex, Uint32 uiCount);
extern void Asm_LDW_Test(Uint32 buff_addr, Uint32 uiIndex, Uint32 uiCount);
extern void Asm_STW_Test(Uint32 buff_addr, Uint32 uiIndex, Uint32 uiCount);

#define 	LD_ST_TEST_OVERHEAD 	40
#define 	LOAD_STORE_TIMES 		(512)

/*execute LDDW test in "buff" of "uiByteCnt" with different index*/
extern void LDDWTest(Uint32 buff_addr, Uint32 uiByteCnt);
/*execute STDW test in "buff" of "uiByteCnt" with different index*/
extern void STDWTest(Uint32 buff_addr, Uint32 uiByteCnt);

#endif
