	.file	"stackFrames.c"
	.text
	.globl	_bar
	.def	_bar;	.scl	2;	.type	32;	.endef
_bar:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	$69, -4(%ebp)
	movl	$420, -8(%ebp)
	nop
	leave
	ret
	.globl	_foo
	.def	_foo;	.scl	2;	.type	32;	.endef
_foo:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	;	The foo procedure only needs 16 bytes
	;	for local variables and args to bar, but
	;	the compiler adds some padding for the
	;	sake of alignment....
	;	
	;	+ It does not matter if the local vars
	;	are all being used after the function
	;	call at the very beginning, it allocates
	;	all the local variables in the beginning

	movl	$69, 4(%esp)
	movl	$420, (%esp)
	call	_bar
	movl	$100, -4(%ebp)
	movl	$200, -8(%ebp)
	nop
	leave
	ret
	.ident	"GCC: (Rev6, Built by MSYS2 project) 13.1.0"
