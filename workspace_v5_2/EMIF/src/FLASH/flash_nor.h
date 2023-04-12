/* --------------------------------------------------------------------------
  FILE      : nor.h
  PURPOSE   : NOR header file
  PROJECT   : Generic NOR Driver - DaVinci Flashing utilities
  AUTHOR    : Daniel Allred
  DESC      : Header file for the generic CFI NOR driver code for
                use with the DaVinci EMIFA peripheral  
  Update    : Brighton Feng 2012-1-19
  DESC	    : Generic NOR driver file for KeyStone(C66x) EMIFA peripheral
 ----------------------------------------------------------------------------- */

#ifndef _NOR_H_
#define _NOR_H_

#include <tistdtypes.h>

// Prevent C++ name mangling
#ifdef __cplusplus
extern far "c" {
#endif

/**************************************************
* Global Macro Declarations                       *
**************************************************/

/**************** DEFINES for AMD Basic Command Set **************/
#define AMD_CMD0                    (0xAA)        // AMD CMD PREFIX 0
#define AMD_CMD1                    (0x55)        // AMD CMD PREFIX 1
#define AMD_CMD0_ADDR               (0x555)       // AMD CMD0 Offset 
#define AMD_CMD1_ADDR               (0x2AA)       // AMD CMD1 Offset 
#define AMD_CMD2_ADDR               (0x555)       // AMD CMD2 Offset 
#define AMD_ID_CMD                  (0x90)        // AMD ID CMD
#define AMD_MANFID_ADDR             (0x00)        // Manufacturer ID offset
#define AMD_DEVID_ADDR0             (0x01)        // First device ID offset
#define AMD_DEVID_ADDR1             (0x0E)        // Offset for 2nd byte of 3 byte ID 
#define AMD_DEVID_ADDR2             (0x0F)        // Offset for 3rd byte of 3 byte ID 
#define AMD_ID_MULTI                (0x7E)        // First-byte ID value for 3-byte ID
#define AMD_RESET                   (0xF0)        // AMD Device Reset Command
#define AMD_BLK_ERASE_SETUP_CMD     (0x80)        // Block erase setup
#define AMD_BLK_ERASE_CMD           (0x30)        // Block erase confirm
#define AMD_BLK_ERASE_DONE	        (0xFFFF)      // Block erase check value
#define AMD_PROG_CMD                (0xA0)        // AMD simple Write command
#define AMD_WRT_BUF_LOAD_CMD        (0x25)        // AMD write buffer load command
#define AMD_WRT_BUF_CONF_CMD        (0x29)        // AMD write buffer confirm command

/**************** DEFINES for Intel Basic Command Set **************/
#define INTEL_ID_CMD                (0x90)        // Intel ID CMD
#define INTEL_MANFID_ADDR           (0x00)        // Manufacturer ID offset
#define INTEL_DEVID_ADDR            (0x01)        // Device ID offset
#define INTEL_RESET                 (0xFF)        // Intel Device Reset Command
#define INTEL_ERASE_CMD0            (0x20)        // Intel Erase command
#define INTEL_ERASE_CMD1            (0xD0)        // Intel Erase command
#define INTEL_WRITE_CMD             (0x40)        // Intel simple write command
#define INTEL_WRT_BUF_LOAD_CMD      (0xE8)        // Intel write buffer load command
#define INTEL_WRT_BUF_CONF_CMD      (0xD0)        // Intel write buffer confirm command
#define INTEL_LOCK_CMD0             (0x60)        // Intel lock mode command
#define INTEL_LOCK_BLOCK_CMD        (0x01)        // Intel lock command
#define INTEL_UNLOCK_BLOCK_CMD      (0xD0)        // Intel unlock command
#define INTEL_CLEARSTATUS_CMD       (0x50)        // Intel clear status command


/**************** DEFINES for CFI Commands and Table **************/

// CFI Entry and Exit commands
#define CFI_QRY_CMD             (0x98u)
#define CFI_EXIT_CMD            (0xF0u)

// CFI address locations
#define CFI_QRY_CMD_ADDR        (0x55u)

// CFI Table Offsets in Bytes
#define CFI_Q                   (0x10)
#define CFI_R                   (0x11)
#define CFI_Y                   (0x12)
#define CFI_CMDSET              (0x13)
#define CFI_CMDSETADDR          (0x15)
#define CFI_ALTCMDSET           (0x17)
#define CFI_ALTCMDSETADDR       (0x19)
#define CFI_MINVCC              (0x1B)
#define CFI_MAXVCC              (0x1C)
#define CFI_MINVPP              (0x1D)
#define CFI_MAXVPP              (0x1E)
#define CFI_TYPBYTEPGMTIME      (0x1F)
#define CFI_TYPBUFFERPGMTIME    (0x20)
#define CFI_TYPBLOCKERASETIME   (0x21)
#define CFI_TYPCHIPERASETIME    (0x22)
#define CFI_MAXBYTEPGMTIME      (0x23)
#define CFI_MAXBUFFERPGMTIME    (0x24)
#define CFI_MAXBLOCKERASETIME   (0x25)
#define CFI_MAXCHIPERASETIME    (0x26)
#define CFI_DEVICESIZE          (0x27)
#define CFI_INTERFACE           (0x28)
#define CFI_WRITESIZE           (0x2A)
#define CFI_NUMBLKREGIONS       (0x2C)
#define CFI_BLKREGIONS          (0x2D)
#define CFI_BLKREGIONSIZE       (0x04)

// Maximum number of block regions supported
#define CFI_MAXREGIONS          (0x06)


/***********************************************************
* Global Typedef declarations                              *
***********************************************************/

// Supported Flash Manufacturers
typedef enum _NOR_MANF_ID_
{
  UNKNOWN_ID    = 0x00,
  AMD           = 0x01,
  FUJITSU       = 0x04,
  INTEL         = 0x89,
  MICRON        = 0x2C,
  SAMSUNG       = 0xEC,
  SHARP         = 0xB0
}
NOR_ManfID;

// Supported CFI command sets
typedef enum _NOR_CMD_SET_
{
  UNKNOWN_CMDSET      = 0x0000,
  INTEL_EXT_CMDSET    = 0x0001,
  AMD_BASIC_CMDSET    = 0x0002,
  INTEL_BASIC_CMDSET  = 0x0003,
  AMD_EXT_CMDSET      = 0x0004,
  MITSU_BASIC_CMDSET  = 0x0100,
  MITSU_EXT_CMDSET    = 0x0101  
}
NOR_CmdSet;

// Struct to hold discovered flash parameters
typedef struct _NOR_INFO_
{
  Uint32      flashBase;                    // 32-bit address of flash start
  Uint8       busWidth;                     // 8-bit or 16-bit bus width
  Uint8       chipOperatingWidth;           // The operating width of each chip
  Uint8       maxTotalWidth;                // Maximum extent of width of all chips combined - determines offset shifts
  Uint32      flashSize;                    // Size of NOR flash regions in bytes (numberDevices * size of one device)
  Uint32      bufferSize;                   // Size of write buffer
  NOR_CmdSet  commandSet;                   // command set id (see CFI documentation)
  Uint8       numberDevices;                // Number of deives used in parallel
  Uint8       numberRegions;                // Number of regions of contiguous regions of same block size
  Uint32      numberBlocks[CFI_MAXREGIONS]; // Number of blocks in a region
  Uint32      blockSize[CFI_MAXREGIONS];    // Size of the blocks in a region
  NOR_ManfID  manfID;                       // Manufacturer's ID
  Uint16      devID1;                       // Device ID
  Uint16      devID2;                       // Used for AMD 3-byte ID devices
}
NOR_InfoObj,*NOR_InfoHandle;

typedef union
{
	Uint8 c;
	Uint16 w;
	Uint32 l;
}
NOR_Data;

typedef union
{
	volatile Uint8 *cp;
	volatile Uint16 *wp;
	volatile Uint32 *lp;
}
NOR_Ptr;


/***********************************************************
* Global Function Declarations                             *
***********************************************************/

extern NOR_InfoHandle NOR_open (Uint32 baseCSAddr);
extern Uint32 NOR_init(NOR_InfoHandle hNorInfo);

extern Uint32 NOR_writeBytes(NOR_InfoHandle hNorInfo, Uint32 writeAddress, Uint32 numBytes, Uint32 readAddress);
extern Uint32 NOR_globalErase(NOR_InfoHandle hNorInfo);
extern Uint32 NOR_erase(NOR_InfoHandle hNorInfo, Uint32 start_address, Uint32 size);
extern Uint32 NOR_getBlockInfo(NOR_InfoHandle hNorInfo, Uint32 address,Uint32* blockSize, Uint32* blockAddr);

/***********************************************************
* End file                                                 *
***********************************************************/

#ifdef __cplusplus
}
#endif

#endif //_NOR_H_

/* --------------------------------------------------------------------------
	HISTORY
    v1.00 - DJA - 07-Nov-2007
      Initial Release
    v1.1 - Brighton Feng - 20-Dec-2011
      modify for big-endian for TCI6614 test
-------------------------------------------------------------------------- */
