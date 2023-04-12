-heap  0x800
-stack 0x1000

MEMORY
{
	/* Local L2, 0.5~1MB*/
	VECTORS: 	o = 0x00800000  l = 0x00000200   
	LL2_RW_DATA: 	o = 0x00800200  l = 0x0003FE00   
	
	/* Shared L2 2~4MB*/
	SL2: 		o = 0x0C000000  l = 0x00200000   
	
	/*pcie receive data*/
	PCIE_EP_RCV_BUF : o = 0x11840000  l=0x00001000
	/* External DDR3, upto 2GB per core */
	DDR3_CODE: 	o = 0x80000000  l = 0x01000000   /*set memory protection attribitue as execution only*/
	DDR3_R_DATA: 	o = 0x81000000  l = 0x01000000 	 /*set memory protection attribitue as read only*/
	DDR3_RW_DATA: 	o = 0x82000000  l = 0x06000000   /*set memory protection attribitue as read/write*/
	PCIE_RC_RCV_BUF: o=0x90120000 l=0x00001000
}

SECTIONS
{
	vecs       	>    VECTORS 

	.text           >    SL2
	.cinit          >    SL2
	.const          >    SL2
	.switch         >    SL2

	.stack          >    LL2_RW_DATA
	GROUP
	{
		.neardata
		.rodata
		.bss
	} 		>    LL2_RW_DATA
	.far            >    LL2_RW_DATA
	.fardata        >    LL2_RW_DATA
	.cio            >    LL2_RW_DATA
	.sysmem         >    LL2_RW_DATA
	pcie_ep_rcv_buf > PCIE_EP_RCV_BUF
	pcie_rc_rcv_buf > PCIE_RC_RCV_BUF

	
}


