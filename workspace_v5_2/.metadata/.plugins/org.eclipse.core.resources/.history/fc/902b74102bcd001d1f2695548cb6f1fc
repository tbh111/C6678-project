-heap  0x800
-stack 0x1000

MEMORY
{
	/* Local L2, 0.5~1MB*/
	VECTORS: 	o = 0x00800000  l = 0x00000200
	LL2: 	o = 0x00800200  l = (0x00080000-0x200)
}

SECTIONS
{
	vecs       	>    VECTORS

	GROUP
	{
		.stack
		.far
		.sysmem
	} > LL2

	GROUP
	{
		.text
		.switch
	} > LL2

	GROUP
	{
		.bss
		.neardata
		.rodata
	} > LL2

	GROUP
	{
		.fardata
		.const
	} > LL2

	GROUP
	{
		.cinit
		.cio
	} > LL2
}


