 	.global Asm_LDDW_Test
 	.global Asm_STDW_Test
 	.global Asm_LDW_Test
 	.global Asm_STW_Test

	.asg 	A16, A16_Index
	.asg 	B0, B0_Cnt
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Asm_LDDW_Test(srcBuff, uiIndex, uiCount);
Asm_LDDW_Test:

 	 	SUB 	A6, 5, B0_Cnt
 	 	MV 	B4, A16_Index

 	[B0_Cnt] B LoadTestLoop
 	|| 	LDDW 	*A4++[A16_Index], A31:A30

 	[B0_Cnt] B LoadTestLoop
 	|| 	LDDW 	*A4++[A16_Index], A31:A30

 	[B0_Cnt] B LoadTestLoop
 	|| 	LDDW 	*A4++[A16_Index], A31:A30

 	[B0_Cnt] B LoadTestLoop
 	|| 	LDDW 	*A4++[A16_Index], A31:A30

 	[B0_Cnt] B LoadTestLoop
 	|| 	LDDW 	*A4++[A16_Index], A31:A30

LoadTestLoop:
 		
 	[B0_Cnt] B LoadTestLoop
 	|| 	LDDW 	*A4++[A16_Index], A31:A30
	||[B0_Cnt] 	SUB 	B0_Cnt, 1, B0_Cnt

		NOP 	
		NOP 	
		NOP 	
		NOP 	
		NOP 	
		NOP 	

 		BNOP  	B3, 5
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Asm_STDW_Test(srcBuff, uiIndex, uiCount);
Asm_STDW_Test:

 	 	SUB 	A6, 5, B0_Cnt
 	 	MV 	B4, A16_Index

 	[B0_Cnt] B StoreTestLoop
 	|| 	STDW 	A31:A30, *A4++[A16_Index]

 	[B0_Cnt] B StoreTestLoop
 	|| 	STDW 	A31:A30, *A4++[A16_Index]

 	[B0_Cnt] B StoreTestLoop
 	|| 	STDW 	A31:A30, *A4++[A16_Index]

 	[B0_Cnt] B StoreTestLoop
 	|| 	STDW 	A31:A30, *A4++[A16_Index]

 	[B0_Cnt] B StoreTestLoop
 	|| 	STDW 	A31:A30, *A4++[A16_Index]

StoreTestLoop:
 		
 	[B0_Cnt] B StoreTestLoop
 	|| 	STDW 	A31:A30, *A4++[A16_Index]
	||[B0_Cnt] 	SUB 	B0_Cnt, 1, B0_Cnt

		NOP 	
		NOP 	
		NOP 	
		NOP 	
		NOP 	
		NOP 	

 		BNOP  	B3, 5
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Asm_LDW_Test(srcBuff, uiIndex, uiCount);
Asm_LDW_Test:

 	 	SUB 	A6, 5, B0_Cnt
 	 	MV 	B4, A16_Index

 	[B0_Cnt] B LDWTestLoop
 	|| 	LDW 	*A4++[A16_Index], A30

 	[B0_Cnt] B LDWTestLoop
 	|| 	LDW 	*A4++[A16_Index], A30

 	[B0_Cnt] B LDWTestLoop
 	|| 	LDW 	*A4++[A16_Index], A30

 	[B0_Cnt] B LDWTestLoop
 	|| 	LDW 	*A4++[A16_Index], A30

 	[B0_Cnt] B LDWTestLoop
 	|| 	LDW 	*A4++[A16_Index], A30

LDWTestLoop:
 		
 	[B0_Cnt] B LDWTestLoop
 	|| 	LDW 	*A4++[A16_Index], A30
	||[B0_Cnt] 	SUB 	B0_Cnt, 1, B0_Cnt

		NOP 	
		NOP 	
		NOP 	
		NOP 	
		NOP 	
		NOP 	

 		BNOP  	B3, 5
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Asm_STW_Test(srcBuff, uiIndex, uiCount);
Asm_STW_Test:

 	 	SUB 	A6, 5, B0_Cnt
 	 	MV 	B4, A16_Index

 	[B0_Cnt] B STWTestLoop
 	|| 	STW 	A30, *A4++[A16_Index]

 	[B0_Cnt] B STWTestLoop
 	|| 	STW 	A30, *A4++[A16_Index]

 	[B0_Cnt] B STWTestLoop
 	|| 	STW 	A30, *A4++[A16_Index]

 	[B0_Cnt] B STWTestLoop
 	|| 	STW 	A30, *A4++[A16_Index]

 	[B0_Cnt] B STWTestLoop
 	|| 	STW 	A30, *A4++[A16_Index]

STWTestLoop:
 		
 	[B0_Cnt] B STWTestLoop
 	|| 	STW 	A30, *A4++[A16_Index]
	||[B0_Cnt] 	SUB 	B0_Cnt, 1, B0_Cnt

		NOP 	
		NOP 	
		NOP 	
		NOP 	
		NOP 	
		NOP 	

 		BNOP  	B3, 5
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
