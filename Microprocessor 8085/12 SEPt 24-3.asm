// USR INPUT @ 2000H
	   LXI D,2000
	   LDAX D
	   MOV C,A
	   INX D

PLOOP:	   MOV L,E
	   MOV H,D
	   MOV B,C

CLOOP:	   MOV A,M
	   INX H
	   CMP M
	   JZ CONT
	   JNC CONT
	   MOV B,A
	   MOV A,M
	   DCX H
	   MOV M,A
	   MOV A,B
	   INX H
	   MOV M,A

CONT:	   DCR B
	   JNZ CLOOP
	   DCR C
	   INX D
	   JNZ PLOOP
	   RST 1
