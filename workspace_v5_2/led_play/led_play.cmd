-c -heap 0x41000 -stack 0xa000
/* memory map*/
MEMORY
{
	L1PSRAM  (RWX) : org = 0x0E00000,  len = 0x7FFF
	L1DSRAM  (RWX) : org = 0x0F00000,  len = 0x7FFF
	L2SRAM   (RWX) : org = 0x0800000,  len = 0x080000
	MSMCSRAM (RWX) : org = 0xC000000,  len = 0x200000
	DDR3     (RWX) : org = 0x80000000, len = 0x10000000
}
SECTIONS
{
	.csl_vect	>	MSMCSRAM
	.text		>	MSMCSRAM
	GROUP (NEAR_DP)
	{
		.neardata
		.rodata
		.bss
	} load		>	MSMCSRAM
	.stack		>	MSMCSRAM
	.cinit		>	MSMCSRAM
	.cio		>	MSMCSRAM
	.const		>	MSMCSRAM
	.data		>	MSMCSRAM
	.switch		>	MSMCSRAM
	.sysmem		>	MSMCSRAM
	.far		>	MSMCSRAM
	.testMem	>	MSMCSRAM
	.fardata	>	MSMCSRAM
	platform_lib>	MSMCSRAM
}
