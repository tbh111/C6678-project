/******************************************************************************

  Copyright (C), 2001-2012, Texas Instrument.

 ******************************************************************************
  File Name     : KeyStone_Serdes_init.h
  Version       : Initial Draft
  Author        : Brighton Feng
  Created       : 2011/6/10
  Last Modified :
  Description   : KeyStone_Serdes_init.c header file, example for Serdes configuration on KeyStone DSP
  Function List :
  History       :
  1.Date        : 2011/6/10
    Author      : Brighton Feng
    Modification: Created file

  2.Date         : 2013/8/25
    Author       : Brighton Feng
    Modification : Add the SGMII SERDES configuration
******************************************************************************/
#ifndef _KEYSTONE_SERDES_INIT_H_
#define _KEYSTONE_SERDES_INIT_H_

#include <tistdtypes.h>
#include <csl_bootcfgaux.h>
#include "KeyStone_common.h"


/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/
#define SERDES_PLL_CFG_CLKBYP_SHIFT         13
#define SERDES_PLL_CFG_LOOPBANDWIDTH_SHIFT  11  
#define SERDES_PLL_CFG_SLEEPPLL_SHIFT       10
#define SERDES_PLL_CFG_VRANGE_SHIFT         9
#define SERDES_PLL_CFG_MPY_SHIFT            1
#define SERDES_PLL_CFG_ENPLL_SHIFT          0

#define SERDES_RX_CFG_TESTPATTERN_SHIFT 25
#define SERDES_RX_CFG_LOOPBACK_SHIFT    23
#define SERDES_RX_CFG_ENOC_SHIFT        22
#define SERDES_RX_CFG_EQ_SHIFT          18
#define SERDES_RX_CFG_CDR_SHIFT         15
#define SERDES_RX_CFG_LOS_SHIFT         12
#define SERDES_RX_CFG_ALIGN_SHIFT       10
#define SERDES_RX_CFG_TERM_SHIFT        7
#define SERDES_RX_CFG_INVPAIR_SHIFT     6
#define SERDES_RX_CFG_RATE_SHIFT        4
#define SERDES_RX_CFG_BUSWIDTH_SHIFT    1
#define SERDES_RX_CFG_ENRX_SHIFT        0

#define SGMII_SERDES_TX_CFG_LOOPBACK_SHIFT  20
#define SGMII_SERDES_TX_CFG_RDTCT_SHIFT     18
#define SGMII_SERDES_TX_CFG_ENIDL_SHIFT     17
#define SGMII_SERDES_TX_CFG_MYSNC_SHIFT     16
#define SGMII_SERDES_TX_CFG_DEMPHASIS_SHIFT 12
#define SGMII_SERDES_TX_CFG_SWING_SHIFT     8
#define SGMII_SERDES_TX_CFG_CM_SHIFT        7

#define SERDES_TX_CFG_TESTPATTERN_SHIFT 23
#define SERDES_TX_CFG_LOOPBACK_SHIFT    21
#define SERDES_TX_CFG_MYSNC_SHIFT       20
#define SERDES_TX_CFG_FIRUPT_SHIFT      19
#define SERDES_TX_CFG_TWPST1_SHIFT      14
#define SERDES_TX_CFG_TWPRE_SHIFT       11
#define SERDES_TX_CFG_SWING_SHIFT       7

#define SERDES_TX_CFG_INVPAIR_SHIFT     6
#define SERDES_TX_CFG_RATE_SHIFT        4
#define SERDES_TX_CFG_BUSWIDTH_SHIFT    1
#define SERDES_TX_CFG_ENTX_SHIFT        0

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

/**
 * Use this symbol to specify the  link rate
 *  */
typedef enum 
{
	SRIO_SERDES_LINK_RATE_x4 =0,
	SRIO_SERDES_LINK_RATE_x2,
	SRIO_SERDES_LINK_RATE_x1,
	SRIO_SERDES_LINK_RATE_div2,

	HYPERLINK_SERDES_LINK_RATE_x4 =0,
	HYPERLINK_SERDES_LINK_RATE_x2,
	HYPERLINK_SERDES_LINK_RATE_x1,
	HYPERLINK_SERDES_LINK_RATE_div2,

	SGMII_SERDES_LINK_RATE_x2 =0,
	SGMII_SERDES_LINK_RATE_x1,
	SGMII_SERDES_LINK_RATE_div2,
	SGMII_SERDES_LINK_RATE_div16
} SerdesLinkRate;

/**
 * Use this symbol to specify the Serdes Rx termination
 **/
 
typedef enum
{
    /** This configuration is for DC coupled systems using CML transmitters*/
    SERDES_RX_TERM_COMMON_POINT_VDDT = 0,
    /** This configuration is for AC coupled systems. The transmitter has no effect on the receiver common mode*/
    SERDES_RX_TERM_COMMON_POINT_AC_COUPLE = 1,
    /** This configuration is for DC coupled systems which require the common mode voltage to be determined by the transmitter only.*/
    SERDES_RX_TERM_COMMON_POINT_FLOATING = 3
} SerdesRxTerm;

/**
 * Use this symbol to specify the Sd Rx adaptive equalizer
 * */
typedef enum
{
    /** Selects the maximum gain, no equalizer */
    SERDES_RX_EQ_MAXIMUM = 0,
    /** Selects the adaptive Receiver equalizer*/
    SERDES_RX_EQ_ADAPTIVE
} SerdesRxEqConfig;

/**
 * Use this symbol to specify the Sd Rx invert polarity
 * */
typedef enum
{
    /** Selects the Receive pair normal polarity*/
    SERDES_RX_NORMAL_POLARITY = 0,
    /** Selects the Receive pair inverted polarity*/
    SERDES_RX_INVERTED_POLARITY
} SerdesRxInvertPolarity;

typedef enum
{
    /** No symbol alignment will be performed whilst this setting is selected, or when switching to this selection from another*/
    SERDES_RX_ALIGNMENT_DISABLE = 0,
    /** Symbol alignment will be performed whenever a misaligned comma symbol is received. */
    SERDES_RX_COMMA_ALIGNMENT_ENABLE,
    /** The symbol alignment will be adjusted by one bit position when this mode is selected */
    SERDES_RX_ALIGNMENT_JOG
}SerdesRxAlign;

typedef enum
{
    /** Loss of signal detection disabled*/
    SERDES_RX_LOS_DISABLE = 0,
    /** Loss of signal detection enabled. */
    SERDES_RX_LOS_ENABLE = 1
}SerdesRxLos;


/**
 * Use this symbol to specify the Sd Tx polarity
 * */
typedef enum
{
    /** Selects Tx pair normal polarity */
    SERDES_TX_NORMAL_POLARITY = 0,
    /** Selects Tx pair inverted polarity */
    SERDES_TX_INVERTED_POLARITY
} SerdesTxInvertPolarity;

typedef enum
{
    /** Test mode disabled */
    SERDES_TEST_DISABLED = 0,
    /** Alternating 0/1 Pattern. An alternating 0/1 pattern with a period of 2UI  */
    SERDES_ALTERNATING_0_1,
    /** Generate or Verify 27 . 1 PRBS. Uses a 7-bit LFSR with feedback polynomial x7 + x6 + 1 */
    SERDES_PRBS_7BIT_LFSR,
    /** Generate or Verify 223.1 PRBS. Uses a 23-bit LFSR with feedback polynomial x23 + x18 + 1 */
    SERDES_PRBS_23BIT_LFSR,
    /** Generate or Verify 223.1 PRBS. Uses a 23-bit LFSR with feedback polynomial x23 + x18 + 1 */
    SERDES_PRBS_31BIT_LFSR
} SerdesTestPattern;

/* Use this symbol to specify the Serdes PLL multiply factor
 * */
typedef enum 
{
    /** Select 4x PLL multiply factor */
    SERDES_PLL_MPY_FACTOR_4X = 0x10,
    /** Select 5x PLL multiply factor */
    SERDES_PLL_MPY_FACTOR_5X = 0x14,
    /** Select 6x PLL multiply factor */
    SERDES_PLL_MPY_FACTOR_6X = 0x18,
    /** Select 8x PLL multiply factor */
    SERDES_PLL_MPY_FACTOR_8X = 0x20,
    /** Select 8.25x PLL multiply factor */
    SERDES_PLL_MPY_FACTOR_8_25X = 0x21,
    /** Select 10x PLL multiply factor */
    SERDES_PLL_MPY_FACTOR_10X = 0x28,
    /** Select 12x PLL multiply factor */
    SERDES_PLL_MPY_FACTOR_12X = 0x30,
    /** Select 12.5x PLL multiply factor */
    SERDES_PLL_MPY_FACTOR_12_5X = 0x32,
    /** Select 15x PLL multiply factor */
    SERDES_PLL_MPY_FACTOR_15X = 0x3C,
     /** Select 16x PLL multiply factor */
    SERDES_PLL_MPY_FACTOR_16X = 0x40,
     /** Select 16.5x PLL multiply factor */
    SERDES_PLL_MPY_FACTOR_16_5X = 0x42,
    /** Select 20x PLL multiply factor */
    SERDES_PLL_MPY_FACTOR_20X = 0x50,
     /** Select 22x PLL multiply factor */
    SERDES_PLL_MPY_FACTOR_22X = 0x58,
    /** Select 25x PLL multiply factor */
    SERDES_PLL_MPY_FACTOR_25X = 0x64
}SerdesPllMpyFactor;

/**
 * Use this symbol to specify the Sd PLL voltage range
 * */
typedef enum 
{
    SERDES_PLL_VCO_RANGE_LOW = 0,
    SERDES_PLL_VCO_RANGE_HIGH
}SerdesVcoRange;


/**
 * Use this symbol to specify the Sd PLL sleep mode
 * */
typedef enum 
{
    /** PLL awake  */
    SERDES_PLL_AWAKE = 0,
    /** PLL sleep */
    SERDES_PLL_SLEEP
}SerdesSleepPll;

/**
 * Use this symbol to specify the Sd PLL loop bandwidth
 * */
typedef enum 
{
    /** set pll loop bandwidth mid */
    SERDES_PLL_LOOP_BAND_MID = 0,
    /** set pll loop bandwidth ultra high */
    SERDES_PLL_LOOP_BAND_UHIGH,
    /** set pll loop bandwidth low */
    SERDES_PLL_LOOP_BAND_LOW,
    /** set pll loop bandwidth high */
    SERDES_PLL_LOOP_BAND_HIGH
}SerdesLoopBandwidth;

/**
 * Use this symbol to specify the Sd PLL clock bypass
 * */
typedef enum 
{
    /* Macro operates normally from the PLL. */
    SERDES_PLL_CLOCK_NO_BYPASS = 0,
    /* The macro operates functionally at low speed using TESTCLKT and TESTCLKR */
    SERDES_PLL_CLOCK_TESTCLK_OBSERVE = 2,
    /* The PLL is bypassed by REFCLKP/N */
    SERDES_PLL_CLOCK_REFCLK_OBSERVE = 3
}SerdesClockBypass;

/**
 * Use this symbol to specify the Serdes loopback
 * */
typedef enum 
{
    SERDES_LOOPBACK_DISABLE = 0,
    SERDES_LOOPBACK_ENABLE = 3
}SerdesLoopback;

typedef struct
{
    volatile Uint32 CFGRX;
    volatile Uint32 CFGTX;
}SerdesLinkRegs;

typedef struct
{
    volatile Uint32 CFGPLL;
	SerdesLinkRegs link[4];
}SerdesRegs;

/**
 * This structure is used for configuring the parameters of a SD module*/
typedef struct
{
    float inputRefClock_MHz; 	/*input reference clock in MHz*/

	/** PLL Loop bandwidth setting  */
	SerdesLoopBandwidth     loopBandwidth;

	/*follwing parameters are not input parameters, 
	they will be drived from other input parameters*/
	float pllMpy;
	float serdesPllClock_GHz;
	SerdesVcoRange vcoRange;
} SerdesCommonSetup;

/**
 * @brief This is a sub-structure in @a CSL_AifCommonLinkSetup. This structure is used for
 * configuring the parameters for Serdes params specific to a link  */
typedef struct
{
	/*the link speed must be 1x, 2x or 4x of the other link's speed*/
    float 	linkSpeed_GHz; 	//not used for SGMII, which is fixed to 1.25G

	SerdesLoopback  	loopBack;

	SerdesTestPattern 	testPattern;

	/** Output swing. 0~15 represents between 100 and 850 mVdfpp  */
	Uint32      txOutputSwing;

	/** Invert Polarity. Inverts the polarity of TXPi and TXNi. */
	SerdesTxInvertPolarity    txInvertPolarity;

	/** Rx loss of Signal */
	SerdesRxLos         rxLos;
	
	SerdesRxAlign 		rxAlign;

    /* Clock/data recovery. Configures the clock/data recovery algorithm */
    Uint8           rxCDR;

	/** polairty of Rx differential i/p - normal/inverted */
	SerdesRxInvertPolarity      rxInvertPolarity;

	/** specifies the Rx termination options for AC/DC coupled scenarios */
	SerdesRxTerm              rxTermination;

	/** Rx equalizer configuration */
	SerdesRxEqConfig          rxEqualizerConfig;
} SerdesLinkSetup;

typedef struct
{
	SerdesCommonSetup commonSetup;
	SerdesLinkSetup * linkSetup[2];
} SerdesSetup_2links;

typedef struct
{
	SerdesCommonSetup commonSetup;
	SerdesLinkSetup * linkSetup[4];
} SerdesSetup_4links;

static inline void Wait_SRIO_PLL_Lock()
{
    while ((gpBootCfgRegs->STS_SRIO & 0x00000001) != 0x00000001);
}

static inline void Wait_Hyperlink_PLL_Lock()
{
    while ((gpBootCfgRegs->STS_VUSR & 0x00000001) != 0x00000001);
}

static inline void Wait_SGMII_PLL_Lock()
{
    while ((gpBootCfgRegs->STS_SGMII& 0x00000001) != 0x00000001);
}

static inline void KeyStone_Disable_Serdes_Comma_Align(
	SerdesLinkRegs * serdesLInkRegs)
{
	serdesLInkRegs->CFGRX &= ~(3<<10);
}

extern void KeyStone_SRIO_Serdes_init(
	SerdesSetup_4links * serdes_cfg, SerdesRegs * serdesRegs);

extern void KeyStone_HyperLink_Serdes_init(
	SerdesSetup_4links * serdes_cfg, SerdesRegs * serdesRegs);

extern void Keystone_SGMII_Serdes_init(
	SerdesSetup_2links * serdes_cfg, SerdesRegs * serdesRegs);

extern void KeyStone_Serdes_disable(
	SerdesRegs * serdesRegs, Uint32 uiLinkNum);

#endif 

