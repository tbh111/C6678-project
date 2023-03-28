/*
memory test with DMA
Author: Brighton Feng
Created on 2010-10-27
last modified on 2010-11-5
*/

#ifndef _DMA_MEM_TEST_H
#define _DMA_MEM_TEST_H

/*allocate EDMA TCs between cores sequentially*/
extern void allocate_EDMA_TC(unsigned int core_num, unsigned int number_of_cores);
 
/*memory test with EDMA*/
extern int EDMA_MEM_Test(unsigned int uiStartAddress, unsigned int uiStopAddress,
                        unsigned int  uiDmaBufAddress,
                        unsigned int uiDmaBufSize);
extern int IDMA_MEM_Test(unsigned int uiStartAddress, unsigned int uiStopAddress,
                        unsigned int  uiDmaBufAddress,
                        unsigned int uiDmaBufSize);

 #endif

