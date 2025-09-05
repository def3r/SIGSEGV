	   LXI H,2000
	   MOV C,M
	   DCR C

PLOOP:	   LXI H,2001

CLOOP:	   MOV A,M
	   INX H
	   CMP M
	   JC CONT
	   JZ CONT
	   MOV B,A
	   MOV A,M
	   DCX H
	   MOV M,A
	   INX H
	   MOV M,B

CONT:	   DCR C
	   JNZ CLOOP
	   MOV C,L
	   DCR C
	   DCR C
	   JNZ PLOOP
	   RST 1
