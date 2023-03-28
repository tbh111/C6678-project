/******************************************************************************

  Copyright (C), 2001-2012, Texas Instrument.

 ******************************************************************************
  File Name     : KeyStone_SRIO_Init_drv.c
  Version       : Initial Draft
  Author        : Brighton Feng
  Created       : 2011/6/11
  Last Modified :
  Description   :  example for SRIO configuration and transfer driver on KeyStone DSP

  Function List :
              KeyStone_map_SRIO_RX_message
              KeyStone_SRIO_Build_Type11_Msg_Desc
              KeyStone_SRIO_Build_Type9_Msg_Desc
              KeyStone_SRIO_CSR_CAR_Config
              KeyStone_SRIO_Datastreaming_init
              KeyStone_SRIO_DirectIO
              KeyStone_SRIO_disable_all_blocks
              KeyStone_SRIO_DoorBell
              KeyStone_SRIO_enable_blocks
              KeyStone_SRIO_Error_Capture_Enable
              KeyStone_SRIO_Flow_Control
              KeyStone_SRIO_Garbage_Queue_Cfg
              KeyStone_SRIO_get_LSU_completion_context_code
              KeyStone_SRIO_GlobalEnable
              KeyStone_SRIO_Init
              KeyStone_SRIO_Interrupt_init
              KeyStone_SRIO_little_endian_swap
              KeyStone_SRIO_LSU_transfer
              KeyStone_SRIO_Maintenance
              KeyStone_SRIO_match_ACK_ID
              KeyStone_SRIO_MulticastID_Cfg
              KeyStone_SRIO_packet_forwarding_Cfg
              KeyStone_SRIO_Prioirity_Permission_Setup
              KeyStone_SRIO_RxMode_Setup
              KeyStone_SRIO_set_1x2x4x_Path
              KeyStone_SRIO_set_device_ID
              KeyStone_SRIO_soft_reset
              KeyStone_SRIO_Timeout_Config
              KeyStone_SRIO_TX_Queue_Cfg
              KeyStone_SRIO_wait_LSU_completion
  History       :
  1.Date        : 2011/6/11
    Author      : Brighton Feng
    Modification: Created file

  2.Date         : 2011/6/13
    Author       : Zhan
    Modification : Update
******************************************************************************/
#include <stdio.h>
#include "csl_srioAux.h"
#include "csl_pscAux.h"
#include "KeyStone_SRIO_init_drv.h"
#include "KeyStone_Navigator_init_drv.h"

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

CSL_SrioRegs *  gpSRIO_regs =   (CSL_SrioRegs *)CSL_SRIO_CONFIG_REGS;
SerdesRegs * srioSerdesRegs;
/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/



/*****************************************************************************
 Prototype    : KeyStone_SRIO_GlobalEnable
 Description  : enable globally used blocks including MMR block in SRIO
 Input        : void  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Zhan
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_GlobalEnable(void)
{
  	gpSRIO_regs->RIO_GBL_EN = 1;   
	gpSRIO_regs->BLOCK_ENABLE_STATUS[0].RIO_BLK_EN= 1; //MMR_EN

	//wait for enable completion
	while(0x3 != (gpSRIO_regs->RIO_GBL_EN_STAT&0x3));

}

/*****************************************************************************
 Prototype    : KeyStone_SRIO_enable_blocks
 Description  : Enable SRIO blocks
 Input        : SRIO_Block_Enable * blockEn  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_enable_blocks(
	SRIO_Block_Enable * blockEn)
{
	gpSRIO_regs->BLOCK_ENABLE_STATUS[5].RIO_BLK_EN= blockEn->bBLK5_8_Port_Datapath_EN[0];
	gpSRIO_regs->BLOCK_ENABLE_STATUS[6].RIO_BLK_EN= blockEn->bBLK5_8_Port_Datapath_EN[1];
	gpSRIO_regs->BLOCK_ENABLE_STATUS[7].RIO_BLK_EN= blockEn->bBLK5_8_Port_Datapath_EN[2];
	gpSRIO_regs->BLOCK_ENABLE_STATUS[8].RIO_BLK_EN= blockEn->bBLK5_8_Port_Datapath_EN[3];

	gpSRIO_regs->BLOCK_ENABLE_STATUS[1].RIO_BLK_EN= blockEn->bBLK1_LSU_EN  ;
	gpSRIO_regs->BLOCK_ENABLE_STATUS[2].RIO_BLK_EN= blockEn->bBLK2_MAU_EN  ;
	gpSRIO_regs->BLOCK_ENABLE_STATUS[3].RIO_BLK_EN= blockEn->bBLK3_TXU_EN  ;
	gpSRIO_regs->BLOCK_ENABLE_STATUS[4].RIO_BLK_EN= blockEn->bBLK4_RXU_EN  ;

	while(gpSRIO_regs->BLOCK_ENABLE_STATUS[5].RIO_BLK_EN_STAT != blockEn->bBLK5_8_Port_Datapath_EN[0]);
	while(gpSRIO_regs->BLOCK_ENABLE_STATUS[6].RIO_BLK_EN_STAT != blockEn->bBLK5_8_Port_Datapath_EN[1]);
	while(gpSRIO_regs->BLOCK_ENABLE_STATUS[7].RIO_BLK_EN_STAT != blockEn->bBLK5_8_Port_Datapath_EN[2]);
	while(gpSRIO_regs->BLOCK_ENABLE_STATUS[8].RIO_BLK_EN_STAT != blockEn->bBLK5_8_Port_Datapath_EN[3]);

	while(gpSRIO_regs->BLOCK_ENABLE_STATUS[1].RIO_BLK_EN_STAT != blockEn->bBLK1_LSU_EN  );
	while(gpSRIO_regs->BLOCK_ENABLE_STATUS[2].RIO_BLK_EN_STAT != blockEn->bBLK2_MAU_EN  );
	while(gpSRIO_regs->BLOCK_ENABLE_STATUS[3].RIO_BLK_EN_STAT != blockEn->bBLK3_TXU_EN  );
	while(gpSRIO_regs->BLOCK_ENABLE_STATUS[4].RIO_BLK_EN_STAT != blockEn->bBLK4_RXU_EN  );

}

/*****************************************************************************
 Prototype    : KeyStone_SRIO_disable_all_blocks
 Description  : Disable all SRIO blocks
 Input        : None
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_disable_all_blocks()
{
	gpSRIO_regs->BLOCK_ENABLE_STATUS[1].RIO_BLK_EN= 0; //LSU_EN  
	gpSRIO_regs->BLOCK_ENABLE_STATUS[2].RIO_BLK_EN= 0; //MAU_EN  
	gpSRIO_regs->BLOCK_ENABLE_STATUS[3].RIO_BLK_EN= 0; //TXU_EN  
	gpSRIO_regs->BLOCK_ENABLE_STATUS[4].RIO_BLK_EN= 0; //RXU_EN  

	gpSRIO_regs->BLOCK_ENABLE_STATUS[5].RIO_BLK_EN= 0; //PORT0_EN
	gpSRIO_regs->BLOCK_ENABLE_STATUS[6].RIO_BLK_EN= 0; //PORT1_EN
	gpSRIO_regs->BLOCK_ENABLE_STATUS[7].RIO_BLK_EN= 0; //PORT2_EN
	gpSRIO_regs->BLOCK_ENABLE_STATUS[8].RIO_BLK_EN= 0; //PORT3_EN

	gpSRIO_regs->BLOCK_ENABLE_STATUS[0].RIO_BLK_EN= 0; //MMR_EN

	gpSRIO_regs->RIO_GBL_EN = 0;

	//wait for disable completion
	while(gpSRIO_regs->RIO_GBL_EN_STAT&1);	
	
}

/*****************************************************************************
 Prototype    : KeyStone_SRIO_soft_reset
 Description  : soft shutdown and reset SRIO
 Input        : None
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_soft_reset()
{
	int i, j, k;
	
	/*shut down TXU/RXU transaction*/
	for(i=0; i<SRIO_PKTDMA_MAX_CH_NUM; i++)
	{
		KeyStone_pktDma_RxCh_teardown(gpSRIO_DMA_RxChCfgRegs, i, PKTDMA_WAIT_FOREVER);
		KeyStone_pktDma_TxCh_teardown(gpSRIO_DMA_TxChCfgRegs, i, PKTDMA_WAIT_FOREVER);
	}

	for(i= 0; i<SRIO_MAX_LSU_NUM ; i++)
	{
		/*flash LSU transfer for all Source ID*/
		for(j=0; j< SRIO_MAX_DEVICEID_NUM; j++)
		{
			gpSRIO_regs->LSU_CMD[i].RIO_LSU_REG6 = 
				CSL_SRIO_RIO_LSU_REG6_FLUSH_MASK| /*flash*/
				(j<<CSL_SRIO_RIO_LSU_REG6_SCRID_MAP_SHIFT); 

			/*This can take more than one cycle to do the flush. 
			wait for a while*/
			for(k=0; k< 100; k++)
				asm(" nop 5");
		}
	}

	/*disable the PEREN bit of the PCR register to stop all
	new logical layer transactions.*/
	gpSRIO_regs->RIO_PCR &= (~CSL_SRIO_RIO_PCR_PEREN_MASK);

	/*Wait one second to finish any current DMA transfer.*/
	for(i=0; i< 100000000; i++)
		asm(" nop 5");

	//reset all logic blocks in SRIO
	KeyStone_SRIO_disable_all_blocks();

	//disable Serdes
	KeyStone_Serdes_disable(srioSerdesRegs, 4);
	
	//disable SRIO through PSC
	KeyStone_disable_PSC_module(CSL_PSC_PD_SRIO, CSL_PSC_LPSC_SRIO);
	KeyStone_disable_PSC_Power_Domain(CSL_PSC_PD_SRIO);
}

/*****************************************************************************
 Prototype    : KeyStone_SRIO_set_1x2x4x_Path
 Description  : configure SRIO 1x 2x or 4x path mode
 Input        : SRIO_1x2x4x_Path_Control srio_1x2x4x_path_control  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_set_1x2x4x_Path(
	SRIO_1x2x4x_Path_Control srio_1x2x4x_path_control)
{
	/*This register is a global register, even though it can be accessed 
	from any port. So you do not need to program from each port, it is 
	basically a single register.
	The PathID is a RO value, that is driven by the H/W. You cannot modify it*/
	gpSRIO_regs->RIO_PLM[0].RIO_PLM_SP_PATH_CTL= 
		(gpSRIO_regs->RIO_PLM[0].RIO_PLM_SP_PATH_CTL&(~SRIO_1x2x4x_PATH_CONTROL_MASK))|
		srio_1x2x4x_path_control;

}

/*****************************************************************************
 Prototype    : KeyStone_SRIO_Timeout_Config
 Description  : SRIO timeout configuration in microsecond
 Input        : SRIO_Config * srio_cfg            
                Uint32 logicalRepsonseTimeout_us  
                Uint32 physicalPortTimeout_us     
                Uint32 linkInitSilenceTimeout_us  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_Timeout_Config(SRIO_Config * srio_cfg,
	Uint32 logicalRepsonseTimeout_us, 
	Uint32 physicalPortTimeout_us, 
	Uint32 linkInitSilenceTimeout_us)
{
	Uint32 uiTimeout, uiMaxTimeout;
	Uint32 byteClockMHz;

	/*PRESCALAR_SELECT is used to divide VBUSM clock(normally 333 to 400MHz, 
	here use 350MHz), (VBUS_M clock)/(PRESCALAR_SELECT+1), 
	to get about 50MHz clock with about 20ns period.*/
	gpSRIO_regs->RIO_PER_SET_CNTL= (gpSRIO_regs->RIO_PER_SET_CNTL
		&(~CSL_SRIO_RIO_PER_SET_CNTL_PRESCALER_SELECT_MASK))
		|((gDSP_Core_Speed_Hz/1000000/3/50-1)<<CSL_SRIO_RIO_PER_SET_CNTL_PRESCALER_SELECT_SHIFT);

	/*logical layer response timeout
	logicalRepsonseTimeout = 15 x (scaled VBUS_M clock period) x TIMEOUT_VALUE),
	TIMEOUT_VALUE = logicalRepsonseTimeout/(scaled VBUS_M clock period)/15 */
	uiTimeout= logicalRepsonseTimeout_us*1000/20/15;
	uiMaxTimeout=CSL_SRIO_RIO_SP_RT_CTL_TVAL_MASK
		>>CSL_SRIO_RIO_SP_RT_CTL_TVAL_SHIFT;
	if(uiTimeout>uiMaxTimeout)
		uiTimeout= uiMaxTimeout;
	if(0==uiTimeout)
		uiTimeout= 1;
	gpSRIO_regs->RIO_SP_RT_CTL= uiTimeout<<CSL_SRIO_RIO_SP_RT_CTL_TVAL_SHIFT;

	/*SRV_CLK should be scaled to about 10MHz (about 100ns period).
	SRV_CLK= (a SRIO internal IP clock)/ PRESCALAR_SRV_CLK.
	The SRIO internal IP clock is the byte clock of 
	one of the lanes (selected by SYS_CLK_SEL). 
	(Byte clock) = (link rate)/20.*/
	byteClockMHz= 
		srio_cfg->serdes_cfg->linkSetup[srio_cfg->SYS_CLK_SEL]->linkSpeed_GHz*1000/20;
	gpSRIO_regs->RIO_PRESCALAR_SRV_CLK= byteClockMHz/10;

	/*physical layer response timeout.
	physicalPortTimeout = SRV_CLK period * TIMEOUT_VALUE * 3,
	TIMEOUT_VALUE = physicalPortTimeout/SRV_CLK period/3 */
	uiTimeout= physicalPortTimeout_us*1000/100/3;
	uiMaxTimeout=CSL_SRIO_RIO_SP_LT_CTL_TVAL_MASK
		>>CSL_SRIO_RIO_SP_LT_CTL_TVAL_SHIFT;
	if(uiTimeout>uiMaxTimeout)
		uiTimeout= uiMaxTimeout;
	if(0==uiTimeout)
		uiTimeout= 1;
	gpSRIO_regs->RIO_SP_LT_CTL= uiTimeout<<CSL_SRIO_RIO_SP_LT_CTL_TVAL_SHIFT;
	
	/*port silence timeout
	The SRIO starts in the SILENT state. The link output driver is disabled 
	to force the link partner to initialize regardless of its current state. 
	The duration of the SILENT state is controlled by the silence_timer. 
	The duration must be long enough to ensure that the link partner detects 
	the silence (as a loss of lane_sync) and is forced to initialize but short 
	enough that it is readily distinguished from a link break.
	linkInitSilenceTimeout is SRV_CLK period X 410 X SILENCE_TIMER,
	SILENCE_TIMER= linkInitSilenceTimeout/SRV_CLK period/410*/
	uiTimeout= linkInitSilenceTimeout_us*1000/100/410;
	uiMaxTimeout=CSL_SRIO_RIO_PLM_SP_SILENCE_TIMER_SILENCE_TIMER_MASK
		>>CSL_SRIO_RIO_PLM_SP_SILENCE_TIMER_SILENCE_TIMER_SHIFT;
	if(uiTimeout>uiMaxTimeout)
		uiTimeout= uiMaxTimeout;
	if(0==uiTimeout)
		uiTimeout= 1;
	uiTimeout= uiTimeout
		<<CSL_SRIO_RIO_PLM_SP_SILENCE_TIMER_SILENCE_TIMER_SHIFT;
	if(srio_cfg->blockEn.bLogic_Port_EN[0])
		gpSRIO_regs->RIO_PLM[0].RIO_PLM_SP_SILENCE_TIMER= uiTimeout;
	if(srio_cfg->blockEn.bLogic_Port_EN[1])
		gpSRIO_regs->RIO_PLM[1].RIO_PLM_SP_SILENCE_TIMER= uiTimeout;
	if(srio_cfg->blockEn.bLogic_Port_EN[2])
		gpSRIO_regs->RIO_PLM[2].RIO_PLM_SP_SILENCE_TIMER= uiTimeout;
	if(srio_cfg->blockEn.bLogic_Port_EN[3])
		gpSRIO_regs->RIO_PLM[3].RIO_PLM_SP_SILENCE_TIMER= uiTimeout;
	
	
}

/*****************************************************************************
 Prototype    : KeyStone_SRIO_CSR_CAR_Config
 Description  : configure SRIO standard Command Status Capability registers
 Input        : SRIO_Config * srio_cfg  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_CSR_CAR_Config(SRIO_Config * srio_cfg)
{
	int i;
	Uint32 uiRegisterValue;
	Uint32 uiSpeed_MHz;
	
    gpSRIO_regs->RIO_DEV_ID	= (0x009D << CSL_SRIO_RIO_DEV_ID_DEV_ID_SHIFT)
                             |(0x0030 << CSL_SRIO_RIO_DEV_ID_DEV_VEN_ID_SHIFT);

	/*The lower 4b should match the 4b from the JTAG Variant field of 
	the DeviceID register.*/
    gpSRIO_regs->RIO_DEV_INFO	= gpBootCfgRegs->DEVICE_ID_REG0>>28; 
    
    gpSRIO_regs->RIO_PE_FEAT	= 
		(0 << CSL_SRIO_RIO_PE_FEAT_BRDG_SHIFT) 	/*PE is not a bridge*/
		|(0 << CSL_SRIO_RIO_PE_FEAT_MEM_SHIFT) 	/*PE is not a memory*/
		|(1 << CSL_SRIO_RIO_PE_FEAT_PROC_SHIFT)	/*PE is a processor*/
		|(0 << CSL_SRIO_RIO_PE_FEAT_SW_SHIFT) 	/*PE is not a switch*/
		|(0 << CSL_SRIO_RIO_PE_FEAT_MULT_P_SHIFT) 	//???
		|(0 << CSL_SRIO_RIO_PE_FEAT_FLOW_ARB_SHIFT) //???
		|(0 << CSL_SRIO_RIO_PE_FEAT_MC_SHIFT) 		//???
		|(0 << CSL_SRIO_RIO_PE_FEAT_ERTC_SHIFT) 	//???
		|(1 << CSL_SRIO_RIO_PE_FEAT_SRTC_SHIFT) 	//???
		|(1 << CSL_SRIO_RIO_PE_FEAT_FLOW_CTRL_SHIFT)/*PE supports flow control*/
		|(1 << 6) 	//???
		|(1 << CSL_SRIO_RIO_PE_FEAT_CRF_SHIFT) 		/*PE supports CRF Function*/
		|(1 << CSL_SRIO_RIO_PE_FEAT_CTLS_SHIFT) 	/*supports common transport large systems*/
		|(1 << CSL_SRIO_RIO_PE_FEAT_EXT_FEA_SHIFT)  /*PE has extended features list*/
		|(1 << CSL_SRIO_RIO_PE_FEAT_EXT_AS_SHIFT); 	/*PE supports 34 bit addresses*/

    gpSRIO_regs->RIO_SW_PORT    = (0 << CSL_SRIO_RIO_SW_PORT_PORT_TOTAL_SHIFT) //???
                              |(4 << CSL_SRIO_RIO_SW_PORT_PORT_NUM_SHIFT); 	//???
    
    gpSRIO_regs->RIO_SRC_OP     = 
		(0 << CSL_SRIO_RIO_SRC_OP_G_READ_SHIFT) 	//???
		|(0 << CSL_SRIO_RIO_SRC_OP_G_IREAD_SHIFT) 	//???
		|(0 << CSL_SRIO_RIO_SRC_OP_G_READ_OWN_SHIFT)//???
		|(0 << CSL_SRIO_RIO_SRC_OP_G_DC_INVALIDATE_SHIFT)	//???
		|(0 << CSL_SRIO_RIO_SRC_OP_G_CASTOUT_SHIFT) 		//???
		|(0 << CSL_SRIO_RIO_SRC_OP_G_DC_FLUSH_SHIFT) 		//???
		|(0 << CSL_SRIO_RIO_SRC_OP_G_IO_READ_SHIFT) 		//???
		|(0 << CSL_SRIO_RIO_SRC_OP_G_IC_INVALIDATE_SHIFT) 	//???
		|(0 << CSL_SRIO_RIO_SRC_OP_G_TLB_INVALIDATE_SHIFT) 	//???
		|(0 << CSL_SRIO_RIO_SRC_OP_G_TLB_SYNC_SHIFT) 		//???
		|(0 << CSL_SRIO_RIO_SRC_OP_G_RIO_RSVD_10_SHIFT) 	//???
		|(0 << CSL_SRIO_RIO_SRC_OP_G_RIO_RSVD_11_SHIFT) 	//???
		|(0 << CSL_SRIO_RIO_SRC_OP_DS_TM_SHIFT) 		//???
		|(1 << CSL_SRIO_RIO_SRC_OP_DS_SHIFT) 			//???
		|(0 << CSL_SRIO_RIO_SRC_OP_IMPLEMENT_DEF_SHIFT) //???
		|(1 << CSL_SRIO_RIO_SRC_OP_READ_SHIFT) 		/*support a read operation*/
		|(1 << CSL_SRIO_RIO_SRC_OP_WRITE_SHIFT) 	/*support a write operation*/
		|(1 << CSL_SRIO_RIO_SRC_OP_STRM_WR_SHIFT) 	/*support a streaming write operation*/
		|(1 << CSL_SRIO_RIO_SRC_OP_WR_RES_SHIFT) 	/*support a write with response operation*/
		|(1 << CSL_SRIO_RIO_SRC_OP_D_MSG_SHIFT) 	/*support a data message operation*/
		|(1 << CSL_SRIO_RIO_SRC_OP_DBELL_SHIFT) 	/*support a doorbell operation*/
		|(0 << CSL_SRIO_RIO_SRC_OP_ACSWAP_SHIFT) 	//???
		|(1 << CSL_SRIO_RIO_SRC_OP_ATSWAP_SHIFT) 	/*support an atomic test-and-swap operation*/
		|(1 << CSL_SRIO_RIO_SRC_OP_A_INC_SHIFT) 	/*support an atomic increment operation*/
		|(1 << CSL_SRIO_RIO_SRC_OP_A_DEC_SHIFT) 	/*support an atomic decrement operation*/
		|(1 << CSL_SRIO_RIO_SRC_OP_A_SET_SHIFT) 	/*support an atomic set operation*/
		|(1 << CSL_SRIO_RIO_SRC_OP_A_CLEAR_SHIFT) 	/*support an atomic clear operation*/
		|(0 << CSL_SRIO_RIO_SRC_OP_A_SWAP_SHIFT) 	//???
		|(1 << CSL_SRIO_RIO_SRC_OP_PORT_WR_SHIFT)	/*support a port-write generation*/
		|(0 << CSL_SRIO_RIO_SRC_OP_IMPLEMENT_DEF2_SHIFT); //???     0x0004FDF4;


    gpSRIO_regs->RIO_DEST_OP    = 
		(0 << CSL_SRIO_RIO_DEST_OP_G_READ_SHIFT)       	  //???
		|(0 << CSL_SRIO_RIO_DEST_OP_G_IREAD_SHIFT)          //???
		|(0 << CSL_SRIO_RIO_DEST_OP_G_READ_OWN_SHIFT)       //???
		|(0 << CSL_SRIO_RIO_DEST_OP_G_DC_INVALIDATE_SHIFT)  //???
		|(0 << CSL_SRIO_RIO_DEST_OP_G_CASTOUT_SHIFT)        //???
		|(0 << CSL_SRIO_RIO_DEST_OP_G_DC_FLUSH_SHIFT)       //???
		|(0 << CSL_SRIO_RIO_DEST_OP_G_IO_READ_SHIFT)        //???
		|(0 << CSL_SRIO_RIO_DEST_OP_G_IC_INVALIDATE_SHIFT)  //???
		|(0 << CSL_SRIO_RIO_DEST_OP_G_TLB_INVALIDATE_SHIFT) //???
		|(0 << CSL_SRIO_RIO_DEST_OP_G_TLB_SYNC_SHIFT)       //???
		|(0 << CSL_SRIO_RIO_DEST_OP_G_RIO_RSVD_10_SHIFT)    //???
		|(0 << CSL_SRIO_RIO_DEST_OP_G_RIO_RSVD_11_SHIFT)    //???
		|(0 << CSL_SRIO_RIO_DEST_OP_DS_TM_SHIFT)            //???
		|(0 << CSL_SRIO_RIO_DEST_OP_DS_SHIFT)               //???
		|(0 << CSL_SRIO_RIO_DEST_OP_IMPLEMENT_DEF_SHIFT)    //???
		|(1 << CSL_SRIO_RIO_DEST_OP_READ_SHIFT)      /*support a read operation*/                  
		|(1 << CSL_SRIO_RIO_DEST_OP_WRITE_SHIFT)     /*support a write operation*/                 
		|(1 << CSL_SRIO_RIO_DEST_OP_STRM_WR_SHIFT)   /*support a streaming write operation*/       
		|(1 << CSL_SRIO_RIO_DEST_OP_WR_RES_SHIFT)    /*support a write with response operation*/   
		|(1 << CSL_SRIO_RIO_DEST_OP_D_MSG_SHIFT)     /*support a data message operation*/          
		|(1 << CSL_SRIO_RIO_DEST_OP_DBELL_SHIFT)     /*support a doorbell operation*/              
		|(0 << CSL_SRIO_RIO_DEST_OP_ACSWAP_SHIFT)    //???                                         
		|(0 << CSL_SRIO_RIO_DEST_OP_ATSWAP_SHIFT)    /*support an atomic test-and-swap operation*/ 
		|(0 << CSL_SRIO_RIO_DEST_OP_A_INC_SHIFT)     /*support an atomic increment operation*/     
		|(0 << CSL_SRIO_RIO_DEST_OP_A_DEC_SHIFT)     /*support an atomic decrement operation*/     
		|(0 << CSL_SRIO_RIO_DEST_OP_A_SET_SHIFT)     /*support an atomic set operation*/           
		|(0 << CSL_SRIO_RIO_DEST_OP_A_CLEAR_SHIFT)   /*support an atomic clear operation*/         
		|(0 << CSL_SRIO_RIO_DEST_OP_A_SWAP_SHIFT) 	//???
		|(1 << CSL_SRIO_RIO_DEST_OP_PORT_WR_SHIFT) 	/*support a port-write operation*/
		|(0 << CSL_SRIO_RIO_DEST_OP_IMPLEMENT_DEF2_SHIFT); //??? 0x0000FC04;      

	/*PE supports 34 bit addresses*/
	gpSRIO_regs->RIO_PE_LL_CTL= (1<<CSL_SRIO_RIO_PE_LL_CTL_EXT_ADDR_CTL_SHIFT);

	/*The host base device ID lock CSR contains the base device ID value for 
	the processing element in the system that is responsible for initializing
	this processing element.*/
    gpSRIO_regs->RIO_HOST_BASE_ID_LOCK	= 
    	srio_cfg->device_ID_routing_config[0].idPattern;

	/*Software defined component tag for the PE. 
	Useful for devices without device IDs.*/
	gpSRIO_regs->RIO_COMP_TAG	= 0 ;  // not touched
    
    /* port general control */
    gpSRIO_regs->RIO_SP_GEN_CTL 		=  // agent, master, undiscovered
		(0 << CSL_SRIO_RIO_SP_GEN_CTL_HOST_SHIFT)
		|(1 << CSL_SRIO_RIO_SP_GEN_CTL_MAST_EN_SHIFT)
		|(0 << CSL_SRIO_RIO_SP_GEN_CTL_DISC_SHIFT);

	/*port control*/
	for(i= 0; i< SRIO_MAX_PORT_NUM; i++)
	{
		if(FALSE==srio_cfg->blockEn.bLogic_Port_EN[i])
			continue;

		uiRegisterValue= 
			(0<<CSL_SRIO_RIO_SP_CTL_PORT_DIS_SHIFT    )
			|(1<<CSL_SRIO_RIO_SP_CTL_OTP_EN_SHIFT      )
			|(1<<CSL_SRIO_RIO_SP_CTL_INP_EN_SHIFT      )
			|(0<<CSL_SRIO_RIO_SP_CTL_STOP_FAIL_EN_SHIFT)
			|(1<<CSL_SRIO_RIO_SP_CTL_PTYP_SHIFT        );
		if(NULL!=srio_cfg->flowControlID)
			uiRegisterValue|= 
				(1<<CSL_SRIO_RIO_SP_CTL_FLOW_CTRL_SHIFT   )
				|(1<<CSL_SRIO_RIO_SP_CTL_FLOW_ARB_SHIFT    );
		gpSRIO_regs->RIO_SP[i].RIO_SP_CTL = uiRegisterValue;

		uiSpeed_MHz= (Uint32)(srio_cfg->serdes_cfg->linkSetup[i]->linkSpeed_GHz*1000);
		if(1250==uiSpeed_MHz)
			gpSRIO_regs->RIO_SP[i].RIO_SP_CTL2 = 
				1<<CSL_SRIO_RIO_SP_CTL2_GB_1P25_EN_SHIFT;
		if(2500==uiSpeed_MHz)
			gpSRIO_regs->RIO_SP[i].RIO_SP_CTL2 = 
				1<<CSL_SRIO_RIO_SP_CTL2_GB_2P5_EN_SHIFT;
		if(3125==uiSpeed_MHz)
			gpSRIO_regs->RIO_SP[i].RIO_SP_CTL2 = 
				1<<CSL_SRIO_RIO_SP_CTL2_GB_3P125_EN_SHIFT;
		if(5000==uiSpeed_MHz)
			gpSRIO_regs->RIO_SP[i].RIO_SP_CTL2 = 
				1<<CSL_SRIO_RIO_SP_CTL2_GB_5P0_EN_SHIFT;
		if(6250==uiSpeed_MHz)
			gpSRIO_regs->RIO_SP[i].RIO_SP_CTL2 = 
				1<<CSL_SRIO_RIO_SP_CTL2_GB_6P25_EN_SHIFT;

		/*disable port write*/
		gpSRIO_regs->RIO_PLM[i].RIO_PLM_SP_ALL_PW_EN = 0;
	}
	
	/*clear port write dest Id, port write not used*/
    gpSRIO_regs->RIO_PW_TGT_ID= 0;
	/*disable port write*/
    gpSRIO_regs->RIO_EM_DEV_PW_EN = 0;
    
    /* Register Reset Control 
    Allows the SELF_RST and PWDN_PORT resets to clear sticky register bits 
    in addition to the normal configuration registers.*/
    gpSRIO_regs->RIO_REG_RST_CTL = 
    	1 << CSL_SRIO_RIO_REG_RST_CTL_CLEAR_STICKY_SHIFT;


}

/*****************************************************************************
 Prototype    : KeyStone_SRIO_Error_Capture_Enable
 Description  : enable error capture for SRIO
 Input        : SRIO_Block_Enable * blockEn  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_Error_Capture_Enable(SRIO_Block_Enable * blockEn)
{
	Int32 i;

	/*Enable specific error reporting, save and lock capture information 
	in the appropriate Logical/Transport Layer Capture CSRs*/
    gpSRIO_regs->RIO_ERR_EN= 
		(1 << CSL_SRIO_RIO_ERR_EN_IO_ERR_RESP_EN_SHIFT) 	/*"ERROR" response for an direct IO*/
		|(1 << CSL_SRIO_RIO_ERR_EN_MSG_ERR_RESP_EN_SHIFT) 	/*"ERROR" response for an message*/
		|(0 << CSL_SRIO_RIO_ERR_EN_GSM_ERR_RESP_EN_SHIFT) 	
		|(1 << CSL_SRIO_RIO_ERR_EN_MSG_FMT_ERR_EN_SHIFT) 	/*received message with invalid fromat*/
		|(1 << CSL_SRIO_RIO_ERR_EN_ILL_TRANS_DECODE_EN_SHIFT) 	/*Illegal transaction decode*/
		|(1 << CSL_SRIO_RIO_ERR_EN_ILL_TRANS_TGT_ERR_EN_SHIFT) 	/*Illegal transaction target error*/
		|(1 << CSL_SRIO_RIO_ERR_EN_MSG_REQ_TIMEOUT_EN_SHIFT) 	/*Message Request Time-out*/
		|(1 << CSL_SRIO_RIO_ERR_EN_PKT_RESP_TIMEOUT_EN_SHIFT) 	/*Packet Response Time-out*/
		|(1 << CSL_SRIO_RIO_ERR_EN_UNSOLICITED_RESP_EN_SHIFT) 	/*unsolicited/unexpected Response*/
		|(1 << CSL_SRIO_RIO_ERR_EN_UNSUPPORTED_TRANS_EN_SHIFT) 	/*Unsupported Transaction*/
		|(1 << CSL_SRIO_RIO_ERR_EN_PDU_LEN_ERR_EN_SHIFT) 		/*Data streaming PDU length error*/
		|(1 << CSL_SRIO_RIO_ERR_EN_SHORT_STREAM_SEG_EN_SHIFT) 	/*Short data streaming segment*/
		|(1 << CSL_SRIO_RIO_ERR_EN_LONG_STREAM_SEG_EN_SHIFT) 	/*Long data streaming segment*/
		|(1 << CSL_SRIO_RIO_ERR_EN_OPEN_STREAM_CONTEXT_EN_SHIFT) 	/*Open existing data streaming context*/
		|(1 << CSL_SRIO_RIO_ERR_EN_MISSING_STREAM_CONTEXT_EN_SHIFT) /*Missing data streaming context*/
		|(1 << CSL_SRIO_RIO_ERR_EN_CPPI_SECURITY_VIOLATION_EN_SHIFT)/*RX CPPI Security Violation*/
		|(1 << CSL_SRIO_RIO_ERR_EN_RX_DMA_ERR_EN_SHIFT); 			/*RX I/O DMA Access Error*/

	/*Error detect register, write 0 to clear it.*/
    gpSRIO_regs->RIO_ERR_DET= 0 ;

	/*clear error capture registers*/
    gpSRIO_regs->RIO_H_ADDR_CAPT= 0;
    gpSRIO_regs->RIO_ADDR_CAPT= 0 ;
    gpSRIO_regs->RIO_ID_CAPT= 0 ;
    gpSRIO_regs->RIO_CTRL_CAPT= 0;

	//enable physical layer error capture
	for(i=0; i< SRIO_MAX_PORT_NUM; i++)
	{
		if(FALSE == blockEn->bLogic_Port_EN[i])
			continue;

		/*Enable error rate counting*/
		gpSRIO_regs->RIO_SP_ERR[i].RIO_SP_RATE_EN= 
			(1<<CSL_SRIO_RIO_SP_RATE_EN_IMP_SPEC_EN_SHIFT     )
			|(1<<CSL_SRIO_RIO_SP_RATE_EN_CS_CRC_EN_SHIFT       )
			|(1<<CSL_SRIO_RIO_SP_RATE_EN_CS_ILL_ID_EN_SHIFT    )
			|(1<<CSL_SRIO_RIO_SP_RATE_EN_CS_NOT_ACC_EN_SHIFT   )
			|(1<<CSL_SRIO_RIO_SP_RATE_EN_PKT_ILL_ACKID_EN_SHIFT)
			|(1<<CSL_SRIO_RIO_SP_RATE_EN_PKT_CRC_ERR_EN_SHIFT  )
			|(1<<CSL_SRIO_RIO_SP_RATE_EN_PKT_ILL_SIZE_EN_SHIFT )
			|(1<<CSL_SRIO_RIO_SP_RATE_EN_DSCRAM_LOS_EN_SHIFT   )
			|(1<<CSL_SRIO_RIO_SP_RATE_EN_LR_ACKID_ILL_EN_SHIFT )
			|(1<<CSL_SRIO_RIO_SP_RATE_EN_PROT_ERR_EN_SHIFT     )
			|(1<<CSL_SRIO_RIO_SP_RATE_EN_DELIN_ERR_EN_SHIFT    )
			|(1<<CSL_SRIO_RIO_SP_RATE_EN_CS_ACK_ILL_EN_SHIFT   )
			|(1<<CSL_SRIO_RIO_SP_RATE_EN_LINK_TO_EN_SHIFT      );

		/*the threshold value for reporting an error condition*/
		gpSRIO_regs->RIO_SP_ERR[i].RIO_SP_ERR_THRESH= 
			(0xF<<CSL_SRIO_RIO_SP_ERR_THRESH_ERR_RFT_SHIFT)
			|(0x4<<CSL_SRIO_RIO_SP_ERR_THRESH_ERR_RDT_SHIFT);

		/*Error detect register, write 0 to clear it.*/
	    gpSRIO_regs->RIO_SP_ERR[i].RIO_SP_ERR_DET= 0;

		/*clear error capture registers*/
	    gpSRIO_regs->RIO_SP_ERR[i].RIO_SP_ERR_ATTR_CAPT= 0;
	    gpSRIO_regs->RIO_SP_ERR[i].RIO_SP_ERR_CAPT_0= 0;
	    gpSRIO_regs->RIO_SP_ERR[i].RIO_SP_ERR_CAPT_1= 0;
	    gpSRIO_regs->RIO_SP_ERR[i].RIO_SP_ERR_CAPT_2= 0;
	    gpSRIO_regs->RIO_SP_ERR[i].RIO_SP_ERR_CAPT_3= 0;

		/*clear error rate counter*/
	    gpSRIO_regs->RIO_SP_ERR[i].RIO_SP_ERR_RATE= 0;

		/*read this register to clear the error count in it*/
		i = gpSRIO_regs->RIO_LANE[i].RIO_LANE_STAT0;
#if 0
		gpSRIO_regs->RIO_PORT_OPTION[i].SP_CTL_INDEP= gpSRIO_regs->PORT_OPTION[i].SP_CTL_INDEP|
			(CSL_SRIO_SP_CTL_INDEP_ILL_TRANS_EN_ENABLE<<CSL_SRIO_SP_CTL_INDEP_ILL_TRANS_EN_SHIFT)|
			(CSL_SRIO_SP_CTL_INDEP_MAX_RETRY_EN_ENABLE<<CSL_SRIO_SP_CTL_INDEP_MAX_RETRY_EN_SHIFT)|
			(255<<CSL_SRIO_SP_CTL_INDEP_MAX_RETRY_THR_SHIFT)|
			(CSL_SRIO_SP_CTL_INDEP_IRQ_EN_ENABLE<<CSL_SRIO_SP_CTL_INDEP_IRQ_EN_SHIFT);
#endif
	}
}

/*
port write feature should be disabled with code like below:
    gpSRIO_regs->SP_IP_MODE |=  
                        CSL_FMKT(SRIO_SP_IP_MODE_PW_DIS, DISABLE) |
*/

/*****************************************************************************
 Prototype    : KeyStone_SRIO_set_device_ID
 Description  : configure SRIO device ID
 Input        : SRIO_Device_ID_Routing_Config * device_id_routing_config  
                Uint32 uiDeviceIdNum                                      
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_set_device_ID(
	SRIO_Device_ID_Routing_Config * device_id_routing_config,
	Uint32 uiDeviceIdNum)
{
	int i;

	/*The TLM_SP(n)_BRR_x_PATTERN_MATCH registers hold the 15 allowable DestIDs, 
	note that the first register is not used.  We use the RIO_BASE_ID register 
	to hold the first ID */
	gpSRIO_regs->RIO_BASE_ID= device_id_routing_config[0].idPattern| 	/*Large ID*/
		((device_id_routing_config[0].idPattern&0xFF)<<16); 		/*small ID*/

	uiDeviceIdNum= _min2(SRIO_MAX_DEVICEID_NUM, uiDeviceIdNum);
	for(i= 1; i<uiDeviceIdNum; i++)
	{
		/*please note, SRIO block 5~8 must be eanbled for corresponding
		RIO_TLM[0:3] taking effect*/
	    gpSRIO_regs->RIO_TLM[i/4].brr[i&3].RIO_TLM_SP_BRR_CTL = 
			(device_id_routing_config[i].routeMaintenance<<
				CSL_SRIO_RIO_TLM_SP_BRR_1_CTL_ROUTE_MR_TO_LLM_SHIFT)|
			(0<<CSL_SRIO_RIO_TLM_SP_BRR_1_CTL_PRIVATE_SHIFT)|
			(1<<CSL_SRIO_RIO_TLM_SP_BRR_1_CTL_ENABLE_SHIFT);

	    gpSRIO_regs->RIO_TLM[i/4].brr[i&3].RIO_TLM_SP_BRR_PATTERN_MATCH = 
			(device_id_routing_config[i].idPattern<<
				CSL_SRIO_RIO_TLM_SP_BRR_1_PATTERN_MATCH_PATTERN_SHIFT)|
			(device_id_routing_config[i].idMatchMask<<
				CSL_SRIO_RIO_TLM_SP_BRR_1_PATTERN_MATCH_MATCH_SHIFT);

	}	
}

/*****************************************************************************
 Prototype    : KeyStone_map_SRIO_RX_message
 Description  : configure the map between message and PacketDMA flow and
                queue
 Input        : SRIO_RX_Message_Map * srio_message_map  
                Uint32 uiNumMessageMap                  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_map_SRIO_RX_message(SRIO_RX_Message_Map * srio_message_map,
	Uint32 uiNumMessageMap)
{
	int i;
	
	uiNumMessageMap= _min2(SRIO_MAX_MSG_MAP_ENTRY_NUM, uiNumMessageMap);
	for(i=0; i< uiNumMessageMap; i++)
	{
		gpSRIO_regs->RXU_MAP[i].RIO_RXU_MAP_L= 
			(srio_message_map[i].ltrMask<<CSL_SRIO_RIO_RXU_MAP_L_LTR_MASK_SHIFT)|
			(srio_message_map[i].mbxMask<<CSL_SRIO_RIO_RXU_MAP_L_MBX_MASK_SHIFT)|
			(srio_message_map[i].ltr<<CSL_SRIO_RIO_RXU_MAP_L_LTR_SHIFT)|
			(srio_message_map[i].mbx<<CSL_SRIO_RIO_RXU_MAP_L_MBX_SHIFT)|
			(srio_message_map[i].srcId<<CSL_SRIO_RIO_RXU_MAP_L_SRCID_SHIFT);

		gpSRIO_regs->RXU_MAP[i].RIO_RXU_MAP_H= 
			(srio_message_map[i].dstId<<CSL_SRIO_RIO_RXU_MAP_H_DEST_ID_SHIFT)|
			((srio_message_map[i].dstProm)<<CSL_SRIO_RIO_RXU_MAP_H_DEST_PROM_SHIFT)|
			(srio_message_map[i].srcProm<<CSL_SRIO_RIO_RXU_MAP_H_SRC_PROM_SHIFT)|
			(srio_message_map[i].segMap<<CSL_SRIO_RIO_RXU_MAP_H_SEG_MAP_SHIFT)|
			(srio_message_map[i].tt<<CSL_SRIO_RIO_RXU_MAP_H_TT_SHIFT);

		gpSRIO_regs->RXU_TYPE9_MAP[i].RIO_RXU_TYPE9_MAP0= 
			(srio_message_map[i].cosMask<<CSL_SRIO_RIO_RXU_TYPE9_MAP0_COS_MASK_SHIFT)|
			(srio_message_map[i].cos<<CSL_SRIO_RIO_RXU_TYPE9_MAP0_COS_SHIFT)|
			(srio_message_map[i].srcId<<CSL_SRIO_RIO_RXU_TYPE9_MAP0_SRCID_SHIFT);

		gpSRIO_regs->RXU_TYPE9_MAP[i].RIO_RXU_TYPE9_MAP1= 
			(srio_message_map[i].dstId<<CSL_SRIO_RIO_RXU_TYPE9_MAP1_DEST_ID_SHIFT)|
			(srio_message_map[i].dstProm<<CSL_SRIO_RIO_RXU_TYPE9_MAP1_DEST_PROM_SHIFT)|
			(srio_message_map[i].tt<<CSL_SRIO_RIO_RXU_TYPE9_MAP1_TT_SHIFT)|
			(srio_message_map[i].srcProm<<CSL_SRIO_RIO_RXU_TYPE9_MAP1_SRC_PROM_SHIFT);

		gpSRIO_regs->RXU_TYPE9_MAP[i].RIO_RXU_TYPE9_MAP2= 
			(srio_message_map[i].streamMask<<CSL_SRIO_RIO_RXU_TYPE9_MAP2_STRM_MASK_SHIFT)|
			(srio_message_map[i].streamId<<CSL_SRIO_RIO_RXU_TYPE9_MAP2_STRM_ID_SHIFT);
			
		gpSRIO_regs->RXU_MAP[i].RIO_RXU_MAP_QID =
			(srio_message_map[i].flowId<<CSL_SRIO_RIO_RXU_MAP_QID_FLOWID_SHIFT)|
			(srio_message_map[i].destQuID<<CSL_SRIO_RIO_RXU_MAP_QID_DEST_QID_SHIFT);

	}
}
/*****************************************************************************
 Prototype    : KeyStone_SRIO_Datastreaming_init
 Description  : Configure the datastreaming mapping
 Input        : SRIO_Datastreaming_Cfg * datastreaming_cfg  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_Datastreaming_init(SRIO_Datastreaming_Cfg * datastreaming_cfg)
{

	if(NULL==datastreaming_cfg)
		return;
		
    gpSRIO_regs->RIO_DS_INFO = 
		(datastreaming_cfg->MaxPDU << CSL_SRIO_RIO_DS_INFO_MAX_PDU_SHIFT)
		|(datastreaming_cfg->SegSupport << CSL_SRIO_RIO_DS_INFO_SEG_SUPPORT_SHIFT);  

    gpSRIO_regs->RIO_DS_LL_CTL = 
    	datastreaming_cfg->MTU << CSL_SRIO_RIO_DS_LL_CTL_MTU_SHIFT;

    gpSRIO_regs->RIO_PER_SET_CNTL1 = (gpSRIO_regs->RIO_PER_SET_CNTL1
    	&(~(CSL_SRIO_RIO_PER_SET_CNTL1_COS_EN_MASK 	/*clear COS_EN bit*/
    	|CSL_SRIO_RIO_PER_SET_CNTL1_CRF_MASK))) 	/*Cleare CRF bit*/
    	|(datastreaming_cfg->COS_EN<<CSL_SRIO_RIO_PER_SET_CNTL1_COS_EN_SHIFT);
}
/*****************************************************************************
 Prototype    : KeyStone_SRIO_TX_Queue_Cfg
 Description  : SRIO TXQ configuration
 Input        : SRIO_TX_Queue_Sch_Info * TX_Queue_Sch_Info  
                Uint32 uiNumTxQueue                         
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_TX_Queue_Cfg(
	SRIO_TX_Queue_Sch_Info * TX_Queue_Sch_Info,
	Uint32 uiNumTxQueue)
{
	int i;
	Uint32 uiMask, uiShift, uiRegIndex;

	/*For SRIO, priority 3 is highest, 0 is lowest
	For PktDMA channel, priority 0 is highest, 3 is lowest*/
	Uint32 mapSrioPriToTxChPri[4]={3,2,1,0};
	
	uiNumTxQueue= _min2(SRIO_PKTDMA_MAX_CH_NUM, uiNumTxQueue);
	for(i=0; i< uiNumTxQueue; i++)
	{
		uiRegIndex= i/4;
		uiShift= (i&3)*8;
		uiMask= 0xFF<<uiShift;
		gpSRIO_regs->RIO_TX_QUEUE_SCH_INFO[uiRegIndex] =
			(gpSRIO_regs->RIO_TX_QUEUE_SCH_INFO[uiRegIndex]&(~uiMask)) 	/*clear the field*/
			|((TX_Queue_Sch_Info[i].CRF
			|(TX_Queue_Sch_Info[i].outputPort<<4))<<uiShift);

		/*PRIO field in TX_QUEUE_SCH_INFOx is read only,
		the priority information comes from the PKTDMA TX channel 
		actually takes effect*/
		gpSRIO_DMA_TxChPriority[i] = 
			mapSrioPriToTxChPri[TX_Queue_Sch_Info[i].priority];
	}
}

/*****************************************************************************
 Prototype    : KeyStone_SRIO_Garbage_Queue_Cfg
 Description  : Setup garbage queue for error packet
 Input        : SRIO_Message_Cfg * msg_cfg  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/10
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_Garbage_Queue_Cfg(
	SRIO_Message_Cfg * msg_cfg)
{
	if(msg_cfg)	
	{
		gpSRIO_regs->RIO_GARBAGE_COLL_QID0=
			(msg_cfg->rx_size_error_garbage_Q
				<<CSL_SRIO_RIO_GARBAGE_COLL_QID0_GARBAGE_QID_LEN_SHIFT)
			|(msg_cfg->rx_timeout_garbage_Q
				<<CSL_SRIO_RIO_GARBAGE_COLL_QID0_GARBAGE_QID_TOUT_SHIFT);
			
		gpSRIO_regs->RIO_GARBAGE_COLL_QID1=
			(msg_cfg->tx_excessive_retries_garbage_Q
				<<CSL_SRIO_RIO_GARBAGE_COLL_QID1_GARBAGE_QID_RETRY_SHIFT)
			|(msg_cfg->tx_error_garbage_Q
				<<CSL_SRIO_RIO_GARBAGE_COLL_QID1_GARBAGE_QID_TRANS_ERR_SHIFT);

		gpSRIO_regs->RIO_GARBAGE_COLL_QID2=
			(msg_cfg->tx_size_error_garbage_Q
				<<CSL_SRIO_RIO_GARBAGE_COLL_QID2_GARBAGE_QID_SSIZE_SHIFT);
	}
}

/*****************************************************************************
 Prototype    : KeyStone_SRIO_packet_forwarding_Cfg
 Description  : configure SRIO packet forwarding
 Input        : SRIO_PktForwarding_Cfg * PktForwardingEntry_cfg  
                Uint32 pktForwardingEntryNum                     
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_packet_forwarding_Cfg(
	SRIO_PktForwarding_Cfg * PktForwardingEntry_cfg,
	Uint32 pktForwardingEntryNum)
{
    int i = 0;

	pktForwardingEntryNum= _min2(SRIO_MAX_FORWARDING_ENTRY_NUM, 
		pktForwardingEntryNum);
    for(i=0; i<pktForwardingEntryNum; i++)
    {
		gpSRIO_regs->PF_CNTL[i].RIO_PF_16B_CNTL =
			(PktForwardingEntry_cfg[i].forwardingID_up_16 
				<< CSL_SRIO_RIO_PF_16B_CNTL_DEVID_16B_UP_SHIFT)
			|(PktForwardingEntry_cfg[i].forwardingID_lo_16 
				<< CSL_SRIO_RIO_PF_16B_CNTL_DEVID_16B_LO_SHIFT);

		gpSRIO_regs->PF_CNTL[i].RIO_PF_8B_CNTL = 
			(PktForwardingEntry_cfg[i].forwardingID_up_8 
				<< CSL_SRIO_RIO_PF_8B_CNTL_DEVID_8B_UP_SHIFT)
			|(PktForwardingEntry_cfg[i].forwardingID_lo_8 
				<< CSL_SRIO_RIO_PF_8B_CNTL_DEVID_8B_LO_SHIFT)
			|(PktForwardingEntry_cfg[i].outport 
				<< CSL_SRIO_RIO_PF_8B_CNTL_OUT_PORT_SHIFT);
    }  
}


/*****************************************************************************
 Prototype    : KeyStone_SRIO_MulticastID_Cfg
 Description  : configure SRIO MulticastID
 Input        : SRIO_Multicast_ID_Cfg * multicastID  
                Uint32 uiNumMulticastID              
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_MulticastID_Cfg(
    SRIO_Multicast_ID_Cfg * multicastID,
    Uint32 uiNumMulticastID)
{
    int i = 0;

	uiNumMulticastID= _min2(SRIO_MAX_MULTICAST_ID_NUM, uiNumMulticastID);
	for(i=0; i<uiNumMulticastID; i++)
	{
		gpSRIO_regs->RIO_MULTIID_REG[i] =
			(multicastID[i].multicast_8b_ID
				<< CSL_SRIO_RIO_MULTIID_REG_16B_NODEID_SHIFT)
			|(multicastID[i].multicast_8b_ID
				<< CSL_SRIO_RIO_MULTIID_REG_8B_NODEID_SHIFT);
	}  
}

/*****************************************************************************
 Prototype    : KeyStone_SRIO_RxMode_Setup
 Description  : Rx Mode configuration
 Input        : SRIO_RX_Mode * rxMode  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_RxMode_Setup(SRIO_RX_Mode * rxMode)
{
	int i;
	
	if(rxMode)
	{
		gpSRIO_regs->RIO_PER_SET_CNTL = (gpSRIO_regs->RIO_PER_SET_CNTL&
			(~CSL_SRIO_RIO_PER_SET_CNTL_LOG_TGT_ID_DIS_MASK))|
			(rxMode->accept_data_with_any_ID
				<<CSL_SRIO_RIO_PER_SET_CNTL_LOG_TGT_ID_DIS_SHIFT);

		/*set RX mode for all ports*/
		for(i=0; i<SRIO_MAX_PORT_NUM; i++)
		{
			gpSRIO_regs->RIO_TLM[i].RIO_TLM_SP_CONTROL = 
				(gpSRIO_regs->RIO_TLM[i].RIO_TLM_SP_CONTROL&
				(~(CSL_SRIO_RIO_TLM_SP_CONTROL_TGT_ID_DIS_MASK
				|CSL_SRIO_RIO_TLM_SP_CONTROL_MTC_TGT_ID_DIS_MASK)))
				|(rxMode->port_rx_mode[i].accept_maintenance_with_any_ID
				<<CSL_SRIO_RIO_TLM_SP_CONTROL_MTC_TGT_ID_DIS_SHIFT)
				|(rxMode->port_rx_mode[i].support_multicast_forwarding
				<<CSL_SRIO_RIO_TLM_SP_CONTROL_TGT_ID_DIS_SHIFT);
		}
	}
}

/*****************************************************************************
 Prototype    : KeyStone_SRIO_little_endian_swap
 Description  : Little endian swap
 Input        : SRIO_Little_Endian_Swap_Mode leSwapMode  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_little_endian_swap(
	SRIO_Little_Endian_Swap_Mode leSwapMode)
{
#ifdef _LITTLE_ENDIAN
	//??? not sure how endian match with another processor in big-endian
	CSL_SRIO_SetLSUSwappingMode(gpSRIO_regs, leSwapMode);
	CSL_SRIO_SetTXURXUSwappingMode(gpSRIO_regs, leSwapMode);
#endif
}
/*****************************************************************************
 Prototype    : KeyStone_SRIO_Interrupt_init
 Description  : SRIO interrupt configuration
 Input        : SRIO_Interrupt_Cfg * interrupt_cfg  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_Interrupt_init(
	SRIO_Interrupt_Cfg * interrupt_cfg)
{
    Uint32 i;
    Uint32 reg, shift;
	volatile Uint32 * ICRR= (volatile Uint32 *)gpSRIO_regs->DOORBELL_ICRR;

	if(NULL == interrupt_cfg)
		return;
		
    /* Clear all the interrupts */
    for(i=0; i<2; i++)
    {
        gpSRIO_regs->LSU_ICSR_ICCR[i].RIO_LSU_ICCR	= 0xFFFFFFFF ;  
    }
    for(i=0; i<4; i++)
    {
        gpSRIO_regs->DOORBELL_ICSR_ICCR[i].RIO_DOORBELL_ICCR = 0xFFFFFFFF;        
    }
	gpSRIO_regs->RIO_ERR_RST_EVNT_ICCR = 0xFFFFFFFF;

    if(NULL != interrupt_cfg->interrupt_map)
    {
	    for(i=0; i<interrupt_cfg->uiNumInterruptMap; i++)
	    {
	        /* Get register index for the interrupt source*/
	        reg = interrupt_cfg->interrupt_map[i].interrupt_event >> 16;

	        /* Get shift value for the interrupt source*/
	        shift = interrupt_cfg->interrupt_map[i].interrupt_event & 0x0000FFFF;

	    	ICRR[reg]= (ICRR[reg]&(~(0xF<<shift))) 	/*clear the field*/
	    		|(interrupt_cfg->interrupt_map[i].INTDST_number<<shift);

	    }
	}

	gpSRIO_regs->RIO_INTERRUPT_CTL = interrupt_cfg->doorbell_route_ctl;

	/*disable interrupt rate control*/
	gpSRIO_regs->RIO_INTDST_RATE_DIS= 0xFFFF;
	for(i= 0; i< 16; i++)
	{
		gpSRIO_regs->RIO_INTDST_RATE_CNT[i]= 0;
	}
	
	if(NULL != interrupt_cfg->interrupt_rate)
	{
		/*setup interrupt rate for specific INTDST*/
		for(i= 0; i<interrupt_cfg->uiNumInterruptRateCfg; i++)
		{
			/*enable rate control for this INTDST*/
			gpSRIO_regs->RIO_INTDST_RATE_DIS &= 
				~(1<<interrupt_cfg->interrupt_rate[i].INTDST_number);

			/*set interrupt rate counter for this INTDST*/
			gpSRIO_regs->RIO_INTDST_RATE_CNT[i]= 
				interrupt_cfg->interrupt_rate[i].interrupt_rate_counter;
		}
	}
	
    return;
}
/*****************************************************************************
 Prototype    : KeyStone_SRIO_Prioirity_Permission_Setup
 Description  : Priority setup
 Input        : SRIO_priority_permission * priority_permission  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_Prioirity_Permission_Setup(
	SRIO_priority_permission * priority_permission)
{
	if(NULL != priority_permission)
	{
		gpSRIO_regs->RIO_PER_SET_CNTL = (gpSRIO_regs->RIO_PER_SET_CNTL
			&(~(CSL_SRIO_RIO_PER_SET_CNTL_CBA_TRANS_PRI_MASK))) 	/*clear the field*/
			|(priority_permission->uiVbusPriority
				<<CSL_SRIO_RIO_PER_SET_CNTL_CBA_TRANS_PRI_SHIFT);
		gpSRIO_regs->RIO_SUPERVISOR_ID = 
			(priority_permission->supervisorHostID_8b<<
				CSL_SRIO_RIO_SUPERVISOR_ID_8B_SUPRVSR_ID_SHIFT)
			|(priority_permission->supervisorHostID_16b<<
				CSL_SRIO_RIO_SUPERVISOR_ID_16B_SUPRVSR_ID_SHIFT);
	}
}

/*****************************************************************************
 Prototype    : KeyStone_SRIO_Flow_Control
 Description  : SRIO flow control setup
 Input        : SRIO_Flow_Control_ID * flowControlID  
                Uint32 uiNumFlowControlID             
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_Flow_Control(
    SRIO_Flow_Control_ID * flowControlID,
    Uint32 uiNumFlowControlID)
{
	int i;
	Uint32 uiFlowMask = 0;

	if(NULL != flowControlID)
	{
		uiNumFlowControlID= _min2(14, uiNumFlowControlID);
		for(i= 0; i< uiNumFlowControlID; i++)
			gpSRIO_regs->RIO_FLOW_CNTL[i]= 
				(flowControlID[i].tt<<CSL_SRIO_RIO_FLOW_CNTL_TT_SHIFT)
				|flowControlID[i].flow_control_ID;

		uiFlowMask= _set(uiFlowMask, 0, uiNumFlowControlID-1);
		uiFlowMask= _pack2(uiFlowMask, uiFlowMask);

		/*enable flow control in all TX modules*/
		for(i= 0; i< SRIO_MAX_LSU_NUM/2; i++)
			gpSRIO_regs->RIO_LSU_FLOW_MASKS[i]= uiFlowMask;

		for(i= 0; i< SRIO_PKTDMA_MAX_CH_NUM/2; i++)
			gpSRIO_regs->RIO_TX_CPPI_FLOW_MASKS[i]= uiFlowMask;
	}		
}

/*****************************************************************************
 Prototype    : KeyStone_SRIO_Init
 Description  : SRIO initialization
 Input        : SRIO_Config * srio_cfg  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

  2.Date         : 2011/6/13
    Author       : Zhan
    Modification : Modify the LSU setup and SERDES configuration
*****************************************************************************/
void KeyStone_SRIO_Init(SRIO_Config * srio_cfg)
{
    Uint32 cfgValue = 0;
    Uint32 i = 0;
    
	srioSerdesRegs = (SerdesRegs *)&gpBootCfgRegs->SRIO_SERDES_CFGPLL;

    if ((CSL_PSC_getPowerDomainState(CSL_PSC_PD_SRIO) == PSC_PDSTATE_ON) &&
        (CSL_PSC_getModuleState (CSL_PSC_LPSC_SRIO) == PSC_MODSTATE_ENABLE))
	{
		if(gpSRIO_regs->RIO_PCR&CSL_SRIO_RIO_PCR_PEREN_MASK)
			KeyStone_SRIO_soft_reset();	//soft reset SRIO if it is already enabled
	}
	
	//enable SRIO power and clock domain
	KeyStone_enable_PSC_module(CSL_PSC_PD_SRIO, CSL_PSC_LPSC_SRIO);

    /*Clear BOOT_COMPLETE bit*/
    gpSRIO_regs->RIO_PER_SET_CNTL &= (~(1 << CSL_SRIO_RIO_PER_SET_CNTL_BOOT_COMPLETE_SHIFT));

	if(srio_cfg->msg_cfg)
	{
		/*These registers can only be written to while the peripheral is disabled.*/
		KeyStone_SRIO_TX_Queue_Cfg(srio_cfg->msg_cfg->TX_Queue_Sch_Info,
			srio_cfg->msg_cfg->uiNumTxQueue);
	}

	/*enable globally used blocks including MMR block in SRIO*/
    KeyStone_SRIO_GlobalEnable();

    /*The LSU setup registers are only programmable
    while the LSU is disabled while the peripheral is enabled.*/
	if(srio_cfg->lsu_cfg)
	{
	    /*setup the shadow registers allocation between LSU*/
	    gpSRIO_regs->RIO_LSU_SETUP_REG0 = 
	    	(srio_cfg->lsu_cfg->lsuGrp0ShadowRegsSetup << CSL_SRIO_RIO_LSU_SETUP_REG0_SHADOW_GRP0_SHIFT)|
	        (srio_cfg->lsu_cfg->lsuGrp1ShadowRegsSetup << CSL_SRIO_RIO_LSU_SETUP_REG0_SHADOW_GRP1_SHIFT);

	    /*setup LSU interrupt based on LSU number or Source ID*/
	    cfgValue = 0;
	    for(i=0; i<SRIO_MAX_LSU_NUM; i++)
	    {
	        cfgValue |= srio_cfg->lsu_cfg->lsuIntSetup[i] << i;         
	    }
	    gpSRIO_regs->RIO_LSU_SETUP_REG1 = cfgValue;
	}

	/*enable other optional blocks*/
	KeyStone_SRIO_enable_blocks(&srio_cfg->blockEn);

	if(SRIO_SERDES_LOOPBACK==srio_cfg->loopback_mode)
	{
		if(srio_cfg->serdes_cfg->linkSetup[0])
			srio_cfg->serdes_cfg->linkSetup[0]->loopBack = SERDES_LOOPBACK_ENABLE;
		if(srio_cfg->serdes_cfg->linkSetup[1])
			srio_cfg->serdes_cfg->linkSetup[1]->loopBack = SERDES_LOOPBACK_ENABLE;
		if(srio_cfg->serdes_cfg->linkSetup[2])
			srio_cfg->serdes_cfg->linkSetup[2]->loopBack = SERDES_LOOPBACK_ENABLE;
		if(srio_cfg->serdes_cfg->linkSetup[3])
			srio_cfg->serdes_cfg->linkSetup[3]->loopBack = SERDES_LOOPBACK_ENABLE;
	}

	KeyStone_SRIO_Serdes_init(srio_cfg->serdes_cfg, srioSerdesRegs);
	Wait_SRIO_PLL_Lock();

	KeyStone_SRIO_set_1x2x4x_Path(srio_cfg->srio_1x2x4x_path_control);

	KeyStone_SRIO_set_device_ID(srio_cfg->device_ID_routing_config, 
		srio_cfg->uiNumDeviceId);

    KeyStone_SRIO_CSR_CAR_Config(srio_cfg);

	/*Allocates ingress Data Nodes and Tags based on priority.
	These registers must only be changed while boot_complete is 
	deasserted or while the port is held in reset.*/
	for(i=0;i<SRIO_MAX_PORT_NUM;i++)
	{
		if(FALSE == srio_cfg->blockEn.bBLK5_8_Port_Datapath_EN[i])
			continue;

		/*maximum data nodes and tags are 72 (0x48).
		 Each data node stores 32 bytes of data*/
		gpSRIO_regs->RIO_PBM[i].RIO_PBM_SP_IG_WATERMARK0 = 
			(36<<CSL_SRIO_RIO_PBM_SP_IG_WATERMARK0_PRIO0_WM_SHIFT)
			|(32<<CSL_SRIO_RIO_PBM_SP_IG_WATERMARK0_PRIO0CRF_WM_SHIFT);
		gpSRIO_regs->RIO_PBM[i].RIO_PBM_SP_IG_WATERMARK1 = 
			(28<<CSL_SRIO_RIO_PBM_SP_IG_WATERMARK1_PRIO1_WM_SHIFT)
			|(24<<CSL_SRIO_RIO_PBM_SP_IG_WATERMARK1_PRIO1CRF_WM_SHIFT);
		gpSRIO_regs->RIO_PBM[i].RIO_PBM_SP_IG_WATERMARK2 = 
			(20<<CSL_SRIO_RIO_PBM_SP_IG_WATERMARK2_PRIO2_WM_SHIFT)
			|(16<<CSL_SRIO_RIO_PBM_SP_IG_WATERMARK2_PRIO2CRF_WM_SHIFT);
		gpSRIO_regs->RIO_PBM[i].RIO_PBM_SP_IG_WATERMARK3 = 
			(12<<CSL_SRIO_RIO_PBM_SP_IG_WATERMARK3_PRIO3_WM_SHIFT)
			|(8<<CSL_SRIO_RIO_PBM_SP_IG_WATERMARK3_PRIO3CRF_WM_SHIFT);
	}
	
	/*setup timeout value in microsecond*/
	KeyStone_SRIO_Timeout_Config(srio_cfg, 500000, 50, 100);

	if(srio_cfg->msg_cfg)
	{
	    KeyStone_SRIO_Datastreaming_init(srio_cfg->msg_cfg->datastreaming_cfg);

		/*Setup garbage queue for error packet*/
		KeyStone_SRIO_Garbage_Queue_Cfg(srio_cfg->msg_cfg);

		/*The mapping is programmable and must be configured after device reset.*/
		KeyStone_map_SRIO_RX_message(srio_cfg->msg_cfg->message_map, 
			srio_cfg->msg_cfg->uiNumMessageMap);
	}
	
    KeyStone_SRIO_RxMode_Setup(srio_cfg->rxMode);
    
    KeyStone_SRIO_packet_forwarding_Cfg(srio_cfg->PktForwardingEntry_cfg,
        srio_cfg->uiNumPktForwardingEntry);

    KeyStone_SRIO_MulticastID_Cfg(srio_cfg->multicastID,
        srio_cfg->uiNumMulticastID);

    KeyStone_SRIO_Flow_Control(srio_cfg->flowControlID,
    	srio_cfg->uiNumFlowControlID);
    
	//KeyStone_SRIO_CSR_CAR_Config(srio_cfg);
	
	KeyStone_SRIO_Prioirity_Permission_Setup(
		srio_cfg->priority_permission);

    KeyStone_SRIO_Interrupt_init(srio_cfg->interrupt_cfg);

	if(SRIO_DIGITAL_LOOPBACK==srio_cfg->loopback_mode)
	{
		gpSRIO_regs->RIO_PER_SET_CNTL1 |=
			(0xF<<CSL_SRIO_RIO_PER_SET_CNTL1_LOOPBACK_SHIFT);
	}
	else if(SRIO_EXTERNAL_LINE_LOOPBACK==srio_cfg->loopback_mode)
	{
		for(i=0; i<SRIO_MAX_PORT_NUM; i++)
		{
			gpSRIO_regs->RIO_PLM[i].RIO_PLM_SP_IMP_SPEC_CTL=
				(1<<CSL_SRIO_RIO_PLM_SP_IMP_SPEC_CTL_LLB_EN_SHIFT);
		}
	}

    /*Set BOOT_COMPLETE bit*/
    gpSRIO_regs->RIO_PER_SET_CNTL |= (1 << CSL_SRIO_RIO_PER_SET_CNTL_BOOT_COMPLETE_SHIFT);

    /*This should be the last enable bit to toggle when bringing the 
    device out of reset to begin normal operation.*/
    gpSRIO_regs->RIO_PCR|= CSL_SRIO_RIO_PCR_PEREN_MASK;

	/*---------wait all enabled ports OK-------------*/
	for(i=0; i<SRIO_MAX_PORT_NUM; i++)
	{
		if(srio_cfg->blockEn.bLogic_Port_EN[i])
		{
			while(0==(gpSRIO_regs->RIO_SP[i].RIO_SP_ERR_STAT&
				CSL_SRIO_RIO_SP_ERR_STAT_PORT_OK_MASK));
		}
	}

	//for debug
    KeyStone_SRIO_Error_Capture_Enable(&srio_cfg->blockEn);

}


/*****************************************************************************
 Prototype    : KeyStone_SRIO_LSU_transfer
 Description  : Initialize the LSU transfer
                the maximum size per LSU transfer block is 1MB, if transfer-
                ->byteCount 
                larger than 1MB, this function only submit transfer for
                1MB. After transfer
                submission, the transfer->byteCount is decreased to indicate
                the remaining 
                bytes, the transfer addresses are increased to indicate
                the start address
                for next possible transfer. The caller can check these to
                take proper action.
                Transcation ID and context bit are recorded in transfer->tr-
                ansactionID
                and transfer->contextBit for caller to check completion
                status.
                
 Input        : SRIO_LSU_Transfer * transfer  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_LSU_transfer(SRIO_LSU_Transfer * transfer)
{
	Uint32   lsuNum= transfer->lsuNum;
	Uint32   uiByteCount;
	Uint32   uiReg6;
	unsigned long long ullRioAddress;

	/*check if LSU is busy or full*/
	uiReg6 = gpSRIO_regs->LSU_CMD[lsuNum].RIO_LSU_REG6;
	while(uiReg6&(CSL_SRIO_RIO_LSU_REG6_BUSY_MASK
		|CSL_SRIO_RIO_LSU_REG6_FULL_MASK))
	{
		if(FALSE==transfer->waitLsuReady) /*should we wait?*/
			return;

		uiReg6 = gpSRIO_regs->LSU_CMD[lsuNum].RIO_LSU_REG6;
	}

	/*record the transcation ID and context bit*/
	transfer->transactionID= uiReg6&
		CSL_SRIO_RIO_LSU_REG6_LTID_MASK;
	transfer->contextBit= (uiReg6&CSL_SRIO_RIO_LSU_REG6_LCB_MASK)>>
		CSL_SRIO_RIO_LSU_REG6_LCB_SHIFT;

	/*the maximum size per LSU transfer block is 1MB, if transfer->byteCount 
	larger than 1MB, this function only submit transfer for 1MB*/
	if(transfer->bytecount>=1024*1024)
		uiByteCount= 1024*1024;
	else
		uiByteCount= transfer->bytecount;

	/*submit transfer*/
	gpSRIO_regs->LSU_CMD[lsuNum].RIO_LSU_REG0= transfer->rioAddressMSB;
	gpSRIO_regs->LSU_CMD[lsuNum].RIO_LSU_REG1= transfer->rioAddressLSB_ConfigOffset;
	gpSRIO_regs->LSU_CMD[lsuNum].RIO_LSU_REG2= transfer->localDspAddress;

	gpSRIO_regs->LSU_CMD[lsuNum].RIO_LSU_REG3= 
    	((uiByteCount&0xFFFFF)<<CSL_SRIO_RIO_LSU_REG3_BYTE_COUNT_SHIFT)|
    	(transfer->doorbellValid<<CSL_SRIO_RIO_LSU_REG3_DRBLL_VALUE_SHIFT);

	gpSRIO_regs->LSU_CMD[lsuNum].RIO_LSU_REG4= 
    	(transfer->intrRequest<<CSL_SRIO_RIO_LSU_REG4_INT_REQ_SHIFT)|
    	(transfer->srcIDMap<<CSL_SRIO_RIO_LSU_REG4_SRCID_MAP_SHIFT)|
    	(transfer->supGoodInt<<CSL_SRIO_RIO_LSU_REG4_SUP_GINT_SHIFT)|
    	(transfer->dstID<<CSL_SRIO_RIO_LSU_REG4_DESTID_SHIFT)|
    	(transfer->idSize<<CSL_SRIO_RIO_LSU_REG4_ID_SIZE_SHIFT)|
    	(0<<CSL_SRIO_RIO_LSU_REG4_XAMBS_SHIFT)|
    	(transfer->priority<<CSL_SRIO_RIO_LSU_REG4_PRIORITY_SHIFT)|
    	(transfer->outPortID<<CSL_SRIO_RIO_LSU_REG4_OUTPORTID_SHIFT);

	gpSRIO_regs->LSU_CMD[lsuNum].RIO_LSU_REG5= transfer->packetType|
		(transfer->hopCount<<CSL_SRIO_RIO_LSU_REG5_HOP_COUNT_SHIFT)|
		(transfer->doorbellInfo<<CSL_SRIO_RIO_LSU_REG5_DRBLL_INFO_SHIFT);

	/*update the byte count and address after submit the transfer*/
	transfer->bytecount -= uiByteCount;
	transfer->localDspAddress += uiByteCount;
	ullRioAddress= _itoll(transfer->rioAddressMSB, 
		transfer->rioAddressLSB_ConfigOffset)+ uiByteCount;
	transfer->rioAddressLSB_ConfigOffset= _loll(ullRioAddress);
	transfer->rioAddressMSB= _hill(ullRioAddress);

}

/* LSU states are in 6 LSU state registers as below:
--------------------------------------------------------------------------------------------------------------
               |31      28|         24|         20|         16|         12|          8|          4|          0|
---------------|----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|
LSU_STAT_REG0  |Lsu0_Stat7| Lsu0_Stat6| Lsu0_Stat5| Lsu0_Stat4| Lsu0_Stat3| Lsu0_Stat2| Lsu0_Stat1| Lsu0_Stat0|
LSU_STAT_REG1  |Lsu2_Stat0| Lsu1_Stat5| Lsu1_Stat4| Lsu1_Stat3| Lsu1_Stat2| Lsu1_Stat1| Lsu1_Stat0| Lsu0_Stat8|
LSU_STAT_REG2  |Lsu3_Stat3| Lsu3_Stat2| Lsu3_Stat1| Lsu3_Stat0| Lsu2_Stat4| Lsu2_Stat3| Lsu2_Stat2| Lsu2_Stat1|
LSU_STAT_REG3  |Lsu4_Stat7| Lsu4_Stat6| Lsu4_Stat5| Lsu4_Stat4| Lsu4_Stat3| Lsu4_Stat2| Lsu4_Stat1| Lsu4_Stat0|
LSU_STAT_REG4  |Lsu6_Stat0| Lsu5_Stat5| Lsu5_Stat4| Lsu5_Stat3| Lsu5_Stat2| Lsu5_Stat1| Lsu5_Stat0| Lsu4_Stat8|
LSU_STAT_REG5  |Lsu7_Stat3| Lsu7_Stat2| Lsu7_Stat1| Lsu7_Stat0| Lsu6_Stat4| Lsu6_Stat3| Lsu6_Stat2| Lsu6_Stat1|
---------------------------------------------------------------------------------------------------------------
below is a table to indicate the index of state for a transaction of a LSU 
in the LSU state registers
*/
Uint8 LSU_state_index_table[SRIO_MAX_LSU_NUM][SRIO_LSU0_4_MAX_SHADOW_REG_SET]=
{
	/*LSU0*/ {0, 1, 2, 3, 4, 5, 6, 7, 8},
	/*LSU1*/ {9, 10, 11, 12, 13, 14},
	/*LSU2*/ {15, 16, 17, 18, 19},
	/*LSU3*/ {20, 21, 22, 23},
	/*LSU4*/ {24, 25, 26, 27, 28, 29, 30, 31, 32},
	/*LSU5*/ {33, 34, 35, 36, 37, 38},
	/*LSU6*/ {39, 40, 41, 42, 43},
	/*LSU7*/ {44, 45, 46, 47}
};
/*****************************************************************************
 Prototype    : KeyStone_SRIO_wait_LSU_completion
 Description  : wait LSU transfer completion, return completion code
 Input        : Uint32 lsuNum         
                Uint32 transactionID  
                Uint32 contextBit     
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
Uint32 KeyStone_SRIO_wait_LSU_completion(Uint32 lsuNum,
	Uint32 transactionID, Uint32 contextBit)
{
	Uint32 uiStateIndex= LSU_state_index_table[lsuNum][transactionID]; 
	Uint32 uiCompletionCode;

	do
	{
		uiCompletionCode=(gpSRIO_regs->LSU_STAT_REG[uiStateIndex/8]>>
			((uiStateIndex&7)*4))&0xF;
	}
	while((uiCompletionCode&1) != contextBit);

	return (uiCompletionCode>>1);
}

/*****************************************************************************
 Prototype    : KeyStone_SRIO_get_LSU_completion_context_code
 Description  : read completion code and context bit
 Input        : Uint32 lsuNum         
                Uint32 transactionID  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
Uint32 KeyStone_SRIO_get_LSU_completion_context_code(Uint32 lsuNum,
	Uint32 transactionID)
{
	Uint32 uiStateIndex= LSU_state_index_table[lsuNum][transactionID]; 

	return ((gpSRIO_regs->LSU_STAT_REG[uiStateIndex/8]>>((uiStateIndex&7)*4))&0xF);
}

/*****************************************************************************
 Prototype    : KeyStone_SRIO_DirectIO
 Description  : SRIO DirectIO operation
 Input        : Uint32 uiLocalAddress        
                Uint32 uiRemoteAddress       
                Uint32 uiDestID              
                Uint32 uiByteCount           
                Uint32 uiPort                
                Uint32 uiLSU_No              
                SRIO_Packet_Type packetType  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
Int32 KeyStone_SRIO_DirectIO(Uint32 uiLocalAddress, Uint32 uiRemoteAddress, 
	Uint32 uiDestID, Uint32 uiByteCount, Uint32 uiPort, Uint32 uiLSU_No, 
	SRIO_Packet_Type packetType)
{
	SRIO_LSU_Transfer lsuTransfer;

    lsuTransfer.rioAddressMSB=0;
    lsuTransfer.rioAddressLSB_ConfigOffset= uiRemoteAddress;
    lsuTransfer.localDspAddress= uiLocalAddress;
    lsuTransfer.bytecount= uiByteCount; 	
    lsuTransfer.packetType= packetType;
    lsuTransfer.dstID= uiDestID;
    lsuTransfer.doorbellInfo= 0;
    lsuTransfer.waitLsuReady= 1;
    lsuTransfer.lsuNum= uiLSU_No;
    lsuTransfer.doorbellValid = 0;
    lsuTransfer.intrRequest = 0;
    lsuTransfer.supGoodInt = 0;
    lsuTransfer.priority = 0;
    lsuTransfer.outPortID = uiPort;
    lsuTransfer.idSize = 0;
    lsuTransfer.srcIDMap = 0;
    lsuTransfer.hopCount = 0;

	KeyStone_SRIO_LSU_transfer(&lsuTransfer);

	return KeyStone_SRIO_wait_LSU_completion(uiLSU_No,
		lsuTransfer.transactionID, lsuTransfer.contextBit);

}

/*****************************************************************************
 Prototype    : KeyStone_SRIO_Maintenance
 Description  : SRIO maintenance operation
 Input        : Uint32 uiPort                
                Uint32 uiLSU_No              
                Uint32 uiDestID              
                Uint32 uiOffSet              
                Uint32 uiLocalAddress        
                SRIO_Packet_Type packetType  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
Int32 KeyStone_SRIO_Maintenance(Uint32 uiPort, Uint32 uiLSU_No,
	Uint32 uiDestID, Uint32 uiOffSet, Uint32 uiLocalAddress, 
	SRIO_Packet_Type packetType)
{
	SRIO_LSU_Transfer lsuTransfer;
	Uint32 uiCompletionCode;
#ifdef _LITTLE_ENDIAN
	Uint32 * uipData = (Uint32 *)uiLocalAddress;

	//swap maintenance value for little endian
	*uipData= _swap4(_packlh2(*uipData, *uipData));	
#endif

    lsuTransfer.rioAddressMSB=0;
    lsuTransfer.rioAddressLSB_ConfigOffset= uiOffSet;
    lsuTransfer.localDspAddress= uiLocalAddress;
    lsuTransfer.bytecount= 4; 	
    lsuTransfer.packetType= packetType;
    lsuTransfer.dstID= uiDestID;
    lsuTransfer.doorbellInfo= 0;
    lsuTransfer.waitLsuReady= 1;
    lsuTransfer.lsuNum= uiLSU_No;
    lsuTransfer.doorbellValid = 0;
    lsuTransfer.intrRequest = 0;
    lsuTransfer.supGoodInt = 0;
    lsuTransfer.priority = 0;
    lsuTransfer.outPortID = uiPort;
    lsuTransfer.idSize = 0;
    lsuTransfer.srcIDMap = 0;
    lsuTransfer.hopCount = 1;

	KeyStone_SRIO_LSU_transfer(&lsuTransfer);

	uiCompletionCode= KeyStone_SRIO_wait_LSU_completion(uiLSU_No,
		lsuTransfer.transactionID, lsuTransfer.contextBit);

#ifdef _LITTLE_ENDIAN
	//swap maintenance value for little endian
	*uipData= _swap4(_packlh2(*uipData, *uipData));	
#endif

	return uiCompletionCode;

}

/*****************************************************************************
 Prototype    : KeyStone_SRIO_DoorBell
 Description  : Send SRIO DoorBell packet
 Input        : Uint32 uiPort          
                Uint32 uiLSU_No        
                Uint32 uiDestID        
                Uint32 uiDoorBellInfo  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
Int32 KeyStone_SRIO_DoorBell(Uint32 uiPort, Uint32 uiLSU_No,
	Uint32 uiDestID, Uint32 uiDoorBellInfo)
{
	SRIO_LSU_Transfer lsuTransfer;

    lsuTransfer.rioAddressMSB=0;
    lsuTransfer.rioAddressLSB_ConfigOffset= 0;
    lsuTransfer.localDspAddress= 0;
    lsuTransfer.bytecount= 4; 	
    lsuTransfer.packetType= SRIO_PKT_TYPE_DOORBELL;
    lsuTransfer.dstID= uiDestID;
    lsuTransfer.doorbellInfo= uiDoorBellInfo;
    lsuTransfer.waitLsuReady= 1;
    lsuTransfer.lsuNum= uiLSU_No;
    lsuTransfer.doorbellValid = 0;
    lsuTransfer.intrRequest = 0;
    lsuTransfer.supGoodInt = 0;
    lsuTransfer.priority = 0;
    lsuTransfer.outPortID = uiPort;
    lsuTransfer.idSize = 0;
    lsuTransfer.srcIDMap = 0;
    lsuTransfer.hopCount = 0;

	KeyStone_SRIO_LSU_transfer(&lsuTransfer);

	return KeyStone_SRIO_wait_LSU_completion(uiLSU_No,
		lsuTransfer.transactionID, lsuTransfer.contextBit);

}

/*****************************************************************************
 Prototype    : KeyStone_SRIO_match_ACK_ID
 Description  : Make the ACK_ID of both sides match 
 Input        : Uint32 uiLocalPort   
                Uint32 uiDestID      
                Uint32 uiRemotePort  
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_match_ACK_ID(Uint32 uiLocalPort,
	Uint32 uiDestID, Uint32 uiRemotePort)
{
	Uint32 uiMaintenanceValue, uiResult;
	Uint32 uiLocal_In_ACK_ID, uiRemote_In_ACK_ID, uiRemote_out_ACK_ID;

	//send a "restart-from-error" commond, request the ACK_ID of the other side
	gpSRIO_regs->RIO_SP[uiLocalPort].RIO_SP_LM_REQ=4; 	

	//wait for link response
	while(0==(gpSRIO_regs->RIO_SP[uiLocalPort].RIO_SP_LM_RESP>>
		CSL_SRIO_RIO_SP_LM_RESP_RESP_VALID_SHIFT))
		asm(" nop 5");

	uiRemote_In_ACK_ID= (gpSRIO_regs->RIO_SP[uiLocalPort].RIO_SP_LM_RESP&
		CSL_SRIO_RIO_SP_LM_RESP_ACK_ID_STAT_MASK)>>
		CSL_SRIO_RIO_SP_LM_RESP_ACK_ID_STAT_SHIFT;

	//Set the local OUTBOUND_ACKID to be same as the responsed ACKID 
	gpSRIO_regs->RIO_SP[uiLocalPort].RIO_SP_ACKID_STAT= uiRemote_In_ACK_ID;
	if(uiRemote_In_ACK_ID != 0)
		printf("match_ACK_ID SP_ACKID_STAT=0x%x\n",gpSRIO_regs->RIO_SP[uiLocalPort].RIO_SP_ACKID_STAT); 	//for dubug
	
	do
	{		
		//set the remote OUTBOUND_ACKID to be same as local INBOUND_ACKID
		uiLocal_In_ACK_ID= (gpSRIO_regs->RIO_SP[uiLocalPort].RIO_SP_ACKID_STAT&
			CSL_SRIO_RIO_SP_ACKID_STAT_INB_ACKID_MASK)>>
			CSL_SRIO_RIO_SP_ACKID_STAT_INB_ACKID_SHIFT;

		uiMaintenanceValue= ((uiRemote_In_ACK_ID+1)<<
			CSL_SRIO_RIO_SP_ACKID_STAT_INB_ACKID_SHIFT)|uiLocal_In_ACK_ID;

		//set the remote ACK_ID through maintenance packet
		uiResult= KeyStone_SRIO_Maintenance(uiLocalPort, uiLocalPort, uiDestID,
			0x148+(0x20*uiRemotePort), GLOBAL_ADDR(&uiMaintenanceValue), 
			SRIO_PKT_TYPE_MTN_WRITE);

		if(uiResult) 	//fail
			continue;	
		
		//readback the remote ID
		uiResult= KeyStone_SRIO_Maintenance(uiLocalPort, uiLocalPort, 
			uiDestID, 0x148+(0x20*uiRemotePort), GLOBAL_ADDR(&uiMaintenanceValue), 
			SRIO_PKT_TYPE_MTN_READ);
		uiRemote_out_ACK_ID= uiMaintenanceValue&
			CSL_SRIO_RIO_SP_ACKID_STAT_OUTB_ACKID_MASK;
	}while(uiResult|(uiLocal_In_ACK_ID+1 != uiRemote_out_ACK_ID));
}

/*****************************************************************************
 Prototype    : KeyStone_SRIO_Build_Type11_Msg_Desc
 Description  : Build SRIO type11 message descriptor
 Input        : HostPacketDescriptor * hostDescriptor  
                Uint32 uiSrcID                         
                Uint32 uiDestID                        
                Uint32 uiByteCount                     
                Uint32 uiMailBox                       
                Uint32 uiLetter                        
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_Build_Type11_Msg_Desc(
	HostPacketDescriptor * hostDescriptor, Uint32 uiSrcID, Uint32 uiDestID, 
	Uint32 uiByteCount, Uint32 uiMailBox, Uint32 uiLetter)
{
	SRIO_Type11_Message_TX_Descriptor * type11MsgTxDesc;

	hostDescriptor->packet_type = SRIO_TYPE11_CPPI_PACKET;
	hostDescriptor->packet_length= uiByteCount;
	hostDescriptor->buffer_len= uiByteCount;
	hostDescriptor->psv_word_count= 2; 	//SRIO uses 2 Protocol Specific words

	type11MsgTxDesc= (SRIO_Type11_Message_TX_Descriptor *)
		(((Uint32)hostDescriptor)+32);
	type11MsgTxDesc->Dest_ID = uiDestID;
	type11MsgTxDesc->SRC_ID = uiSrcID;
	type11MsgTxDesc->Retry_Count= 1;
	type11MsgTxDesc->SSIZE= SRIO_SSIZE_256_BYTES;
	type11MsgTxDesc->TT = 0;
	type11MsgTxDesc->LTR= uiLetter;
	type11MsgTxDesc->MailBox= uiMailBox;			
}

/*****************************************************************************
 Prototype    : KeyStone_SRIO_Build_Type9_Msg_Desc
 Description  : Build SRIO type9 data stream message Descriptror
 Input        : HostPacketDescriptor * hostDescriptor  
                Uint32 uiSrcID                         
                Uint32 uiDestID                        
                Uint32 uiByteCount                     
                Uint32 uiStreamID                      
                Uint32 uiCOS                           
 Output       : None
 Return Value : 
 
  History        :
  1.Date         : 2011/6/11
    Author       : Brighton Feng
    Modification : Created function

*****************************************************************************/
void KeyStone_SRIO_Build_Type9_Msg_Desc(
	HostPacketDescriptor * hostDescriptor, Uint32 uiSrcID, Uint32 uiDestID, 
	Uint32 uiByteCount, Uint32 uiStreamID, Uint32 uiCOS)
{
	SRIO_Type9_Message_TX_Descriptor * type9MsgTxDesc;

	hostDescriptor->packet_type = SRIO_TYPE9_CPPI_PACKET;
	hostDescriptor->packet_length= uiByteCount;
	hostDescriptor->buffer_len= uiByteCount;
	hostDescriptor->psv_word_count= 2; 	//SRIO uses 2 Protocol Specific words

	type9MsgTxDesc= (SRIO_Type9_Message_TX_Descriptor *)
		(((Uint32)hostDescriptor)+32);
	type9MsgTxDesc->Dest_ID = uiDestID;
	type9MsgTxDesc->SRC_ID = uiSrcID;
	type9MsgTxDesc->StreamID = uiStreamID;
	type9MsgTxDesc->TT = 0;
	type9MsgTxDesc->COS = uiCOS;
}

