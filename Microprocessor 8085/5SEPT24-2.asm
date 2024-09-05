	   LDA 3000
	   MOV C,A
	   LDA 3001
	   MOV B,A
	   MVI D,00
	   MVI A,00
	   MOV H,C

MULT:	   ADD B
	   JNC CONT
	   INR D

CONT:	   DCR H
	   JNZ MULT
	   STA 4000
	   MOV A,D
	   STA 4001
	   RST 1
