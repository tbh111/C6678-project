-heap  0x1000
-stack 0x2000

MEMORY
{
	/* Local L2, 0.5~1MB*/
	VECTORS: 	o = 0x00800000  l = 0x00000200   /*set memory protection attribitue as execution only*/
	LL2_CODE: 	o = 0x00800200  l = 0x0000FE00   /*set memory protection attribitue as execution only*/
	LL2_R_DATA: 	o = 0x00810000  l = 0x00008000   /*set memory protection attribitue as read only*/
	LL2_RW_DATA: 	o = 0x00818000  l = 0x00048000   /*set memory protection attribitue as read/write*/

	/* Shared L2 2~4MB*/
	SL2: 		o = 0x0C000000  l = 0x00180000   
	SL2_RW_DATA: 	o = 0x18000000  l = 0x00200000   /*remapped SL2, set memory protection attribitue as read/write*/
	
	/* External DDR3, upto 2GB per core */
	DDR3_CODE: 	o = 0x80000000  l = 0x01000000   /*set memory protection attribitue as execution only*/
	DDR3_R_DATA: 	o = 0x81000000  l = 0x01000000 	 /*set memory protection attribitue as read only*/
	DDR3_RW_DATA: 	o = 0x82000000  l = 0x06000000   /*set memory protection attribitue as read/write*/
}

SECTIONS
{
	vecs       	>    VECTORS 

	.text           >    SL2
	.cinit          >    SL2
	.const          >    SL2
	.switch         >    SL2
	.cio            >    SL2
	.stack          >    LL2_RW_DATA
	GROUP
	{
		.neardata
		.rodata
		.bss
	} 		>    LL2_RW_DATA
	.far            >    LL2_RW_DATA
	.fardata        >    LL2_RW_DATA
	.sysmem         >    LL2_RW_DATA
	External_NonCache_Data 	> 	DDR3_RW_DATA
}


