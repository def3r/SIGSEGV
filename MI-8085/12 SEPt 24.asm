	   MVI B,0A
	   LXI D,2000
	   LDAX D
	   MOV C,A

LOOP:	   LDAX D
	   CMP C
	   JNC CONTINUE
	   JZ CONTINUE
	   LDAX D
	   MOV C,A

CONTINUE:	   INX D
	   DCR B
	   JNZ LOOP
	   MOV A,C
	   STAX D
	   RST 1
