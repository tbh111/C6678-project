/* --------------------------------------------------------------------------
  FILE      : nand.h
  PURPOSE   : NAND header file
  PROJECT   : DaVinci User Boot-Loader and Flasher
  AUTHOR    : Daniel Allred
  DESC      : Header file for the generic low-level NAND driver code for
                use with the DaVinci EMIFA peripheral
-------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------
  HISTORY
    v1.00 - DJA - 02-Nov-2007
      Initial release
    v1.10 - DJA & SG - 15-Jan-2008
      Revision to add support for 4-bit ECC used for MLC NAND on DM355 EVM
    v1.2 - Brighton Feng - 6-Mar-2012
      Update for C66x DSP
-------------------------------------------------------------------------- */

#ifndef _NAND_H_
#define _NAND_H_

#include <tistdtypes.h>

// Prevent C++ name mangling
#ifdef __cplusplus
extern far "c" {
#endif

/************************************************************
* Global Macro Declarations                                 *
************************************************************/

// An assumption is made that ARM-based devices use Linux and 
// therefore use the last four blocks of the NAND as space for 
// an MTD standard bad block table. DSP-based are assumed to 
// not run Linux, and bad blocks are managed in some other fashion.
#if ( defined(__TMS470__) | defined(__GNUC__) )
#define NAND_NUM_BLOCKS_RESERVED_FOR_BBT     (4)
#else
#define NAND_NUM_BLOCKS_RESERVED_FOR_BBT     (0)
#endif

// NAND flash addresses
#define NAND_DATA_OFFSET    0
#define NAND_ALE_OFFSET     (1<<13) 	/*ALE is A11 pin*/
#define NAND_CLE_OFFSET     (1<<14) 	/*CLE is A12 pin*/

// NAND timeout 
#define NAND_TIMEOUT        100000000

// NAND flash commands
#define NAND_LO_PAGE        (0x00)
#define NAND_HI_PAGE        (0x01)
#define NAND_LOCK           (0x2A)
#define NAND_UNLOCK_START   (0x23)
#define NAND_UNLOCK_END     (0x24)
#define NAND_READ_05H       (0x05)
#define NAND_READ_30H       (0x30)
#define NAND_READ_E0H       (0xE0)
#define NAND_EXTRA_PAGE     (0x50)
#define	NAND_RDID           (0x90)
#define	NAND_READ_PARAMETER (0xEC)
#define NAND_RDIDADD        (0x00)
#define	NAND_RESET          (0xFF)
#define	NAND_PGRM_START     (0x80)
#define NAND_PGRM_END       (0x10)
#define NAND_RDY            (0x40)
#define	NAND_PGM_FAIL       (0x01)
#define	NAND_BERASEC1       (0x60)
#define	NAND_BERASEC2       (0xD0)
#define	NAND_STATUS         (0x70)

#define NAND_ONFIRDIDADD        (0x20)
#define NANDONFI_STRING         (0x49464E4F)

// Status Output
#define NAND_NANDFSR_READY      (0x01)
#define NAND_STATUS_WRITEREADY  (0xC0)
#define NAND_STATUS_ERROR       (0x01)
#define NAND_STATUS_BUSY        (0x40)

#define UNKNOWN_NAND            (0xFF)    // Unknown device id
#define MAX_PAGE_SIZE           (2112)    // Including Spare Area
#define MAX_BYTES_PER_OP        (512)     // Bytes per operation (device constrained by ECC calculations)
#define MAX_BYTES_PER_OP_SHIFT  (9)       // Num of right shifts to enable division by MAX_BYTES_PER_OP
#define SPAREBYTES_PER_OP_SHIFT (5)       // Num of right shifts to get spare_bytes per op from bytes per op

// Macro gets the page size in bytes without the spare bytes 
#define NANDFLASH_PAGESIZE(x) ( ( x >> 8 ) << 8 )


/***********************************************************
* Global Typedef declarations                              *
***********************************************************/
typedef enum
{
  NAND_NO_ECC= 0,
  NAND_1_BIT_ECC,
  NAND_4_BITS_ECC
}
NAND_ECC_Mode;



// NAND_INFO structure - holds pertinent info for open driver instance
typedef struct _NAND_INFO_
{
  Uint8           CSOffset;           // 0 for CS2 space, 1 for CS3 space, 2 for CS4 space, 3 for CS5 space
  Uint8           busWidth;           // NAND width
  Uint16          numBlocks;          // block count per device
  Uint8           pagesPerBlock;      // page count per block
  Uint16          dataBytesPerPage;   // Number of data bytes in a page
  Uint8           spareBytesPerPage;  // Number of spare bytes in a page  	
  Uint8           numOpsPerPage;      // Number of operations to complete a page read/write
  Uint16          dataBytesPerOp;     // Number of bytes per operation
  Uint8           spareBytesPerOp;    // Number of bytes in spare byte area of each page   	
  NAND_ECC_Mode   ECC_mode;           // Error correction enable (should be on by default)
  Uint32          ECCMask;            // Mask for ECC register
  Uint8           ECCOffset;          // Offset in spareBytePerOp for ECC Value
  Bool            bigBlock;           // TRUE - Big block device, FALSE - small block device
  Uint8           numColAddrBytes;    // Number of Column address cycles
  Uint8           numRowAddrBytes;    // Number of Row address cycles
  Uint8           blkShift;           // Number of bits by which block address is to be shifted
  Uint8           pageShift;          // Number of bits by which page address is to be shifted
  Uint32          flashBase;          // Base address of CS memory space where NAND is connected
  Uint8           manfID;             // NAND manufacturer ID (just for informational purposes)
  Uint8           devID;              // NAND_DevTable index
  Bool            bONFI;              //is ONFI compliant device
} 
NAND_InfoObj, *NAND_InfoHandle;

typedef union
{
  Uint8 c;
  Uint16 w;
  Uint32 l;
}
NAND_Data;

typedef union
{
  volatile Uint8 *cp;
  volatile Uint16 *wp;
  volatile Uint32 *lp;
}
NAND_Ptr;

typedef enum
{
  NAND_RBL_SWAP,
  NAND_RBL_8TO10,
  NAND_RBL_10TO8
}
NAND_ECCManipulateMethods;


/************************************************************
* Global Function Declarations                              *
************************************************************/

extern Uint32 NAND_init(NAND_InfoHandle hNandInfo);
extern Uint32 NAND_readPage(NAND_InfoHandle hNandInfo, Uint32 block, Uint32 page, Uint8 *dest);
extern Uint32 NAND_writePage(NAND_InfoHandle hNandInfo, Uint32 block, Uint32 page, Uint8 *src);
extern Uint32 NAND_verifyPage(NAND_InfoHandle hNandInfo, Uint32 block, Uint32 page, Uint8 *src, Uint8* dest);
extern Bool   NAND_isBadBlock(NAND_InfoHandle hNandInfo, Uint32 block);
extern Uint32 NAND_eraseBlocks(NAND_InfoHandle hNandInfo, Uint32 startBlkNum, Uint32 blkCount);
extern Uint32 NAND_verifyBlockErased(NAND_InfoHandle hNandInfo, Uint32 block, Uint8* dest);
extern Uint32 NAND_globalErase(NAND_InfoHandle hNandInfo);
extern Uint32 NAND_unProtectBlocks(NAND_InfoHandle hNandInfo,Uint32 startBlkNum,Uint32 endBlkNum);
extern void NAND_protectBlocks(NAND_InfoHandle hNandInfo);


/***********************************************************
* End file                                                 *
***********************************************************/

#ifdef __cplusplus
}
#endif

#endif //_NAND_H_


