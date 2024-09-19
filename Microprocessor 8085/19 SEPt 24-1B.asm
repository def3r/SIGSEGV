	   LXI H,2000
	   MOV B,M
	   MOV C,M
	   MVI D,00
	   MOV E,B
	   DCR B

FACT:	   MOV C,B
	   MVI A,00
	   CALL MULT	// B * C , C->COUNTER
	   MOV E,A
	   DCR B
	   JNZ FACT

BREAK:	   INX H
	   MOV M,A
	   INX H
	   MOV M,D
	   RST 1

MULT:	   ADD E
	   JNC CONT
	   INR D // SHORT BY 1

CONT:	   DCR C
	   JNZ MULT
	   RET
