	   MVI B,15
	   MVI C,15
	   MVI D,00
	   MVI E,00
	   MOV H,C

MULT:	   ADD B
	   JNC CONT
	   INR D

CONT:	   DCR H
	   JNZ MULT
	   MOV E,A
	   RST 1
