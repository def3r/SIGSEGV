	# RISC arch: (A + B) * (C + D)
	LOAD R1, A		;	R1 <-- [A]
	LOAD R2, B		;	R2 <-- [B]
	LOAD R3, C		;	R3 <-- [C]
	LOAD R0, D		;	R0 <-- [D]
	ADD R1, R1, R2		;	R1 <-- [R1] + [R2]
	ADD R0, R0, R3		;	R0 <-- [R0] + [R3]
	MUL R1, R1, R0		;	R1 <-- [R1] * [R0]
	STORE X, R1		;	M[X] <-- [R1]


	# R1:[] R2:[3] R3:[5]
	# Register Indirect Addressing Mode
	MOV R1, (R2)		;	R1 <-- [R3]


	# Procedure Calling
	# Jump and Jump and Link
Main:	addi	$s1, $0, 5
	add	$a0, $s1, $0	# passing arg

	jal	add10

	add	$s1, $v0, $0
	li	$v, 10

	syscall


add10:	addi	$sp, $sp, -4
	sw	$s1, 0($sp)

	addi	$s1, $a0, 10
	add	$v0, $s1, $0

	lw	$s1, 0($sp)
	add	$sp, $sp, 4

	jr	$ra


	# Radnom
	addi $t0, $t0, 0xF	# Hexadecimal 15 ig

	li $t1, 10
	li $t2, 3
	mul $t2, $t2, $2	# incorrect way to multiply
	sll $t2, $t2, 1
	add $t3, $t1, $t2

	# pseudo conversion
	int i = 2, j = 6;
	int k = i + j;
	k = k * 8;

	li $t0, 3
	slti $t1, $t0, 7
	beq $t1, $zero, Else
	addi $t1, $t0, 7
	j Endif
Else:	
	add $t1, $t0, $t0
Endif:	
	# $v0 is set to 10 as 
	# syscall needs $v0 to be set to 10
	# to terminate the program.
	li $v0, 10
	syscall


	li $s0, 0
	li $s2, 0
loop:
	slti $s1, $s2, 64
	beq $s1, $zero, end

	add ($t0, $s0), ($t1, $s0), ($t2, $s0)	# this doesn't work in MIPS at all

	add $t3, $t0, $s0
	add $t4, $t1, $s0
	add $t5, $t2, $s0
	lw $t4, 0($t4)
	lw $t5, 0($t5)
	add $t3, $t4, $t5
	sw $t3, 0($t3)

	addi $s0, $s0, 1
	j loop
end:
	li $v0, 10
	syscall

