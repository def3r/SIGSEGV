	   MVI C,0A
	   LXI H,4000
	   LXI D,5000

LOOP:	   MOV A,M
	   MOV B,A
	   LDAX D
	   MOV M,A
	   MOV A,B
	   STAX D
	   INX H
	   INX D
	   DCR C
	   JNZ LOOP
	   RST 1
