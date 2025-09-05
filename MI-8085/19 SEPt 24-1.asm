	   LXI H,2000
	   MOV B,M
	   MOV C,M
	   MVI A,00
	   MVI D,00

MULT:	   ADD B
	   JNC CONT
	   INR D

CONT:	   DCR C
	   JNZ MULT
	   INX H
	   MOV M,A
	   INX H
	   MOV M,D

	   RST 1
