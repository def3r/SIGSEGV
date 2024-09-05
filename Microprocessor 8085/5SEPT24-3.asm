	   MVI C,05
	   LXI D,2000
	   LXI H,3004

LOOP:	   LDAX D
	   MOV M,A
	   INX D
	   INX H
	   DCR C
	   JNZ LOOP
	   RST 1
