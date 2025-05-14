	.file	"stackFrames.c"
	.text
	.globl	_bar
	.def	_bar;	.scl	2;	.type	32;	.endef
_bar:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	imull	12(%ebp), %eax
	movl	%eax, 8(%ebp)
	movl	8(%ebp), %eax
	popl	%ebp
	ret
	.globl	_foo
	.def	_foo;	.scl	2;	.type	32;	.endef
_foo:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	$10, -4(%ebp)
	movl	$20, 4(%esp)
	movl	-4(%ebp), %eax
	movl	%eax, (%esp)
	call	_bar
	nop
	leave
	ret
	.globl	_baz
	.def	_baz;	.scl	2;	.type	32;	.endef
_baz:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	$10, -12(%ebp)
	movl	$20, -8(%ebp)
	movl	$30, -4(%ebp)
	movl	8(%ebp), %ecx
	movl	-12(%ebp), %eax
	movl	-8(%ebp), %edx
	movl	%eax, (%ecx)
	movl	%edx, 4(%ecx)
	movl	-4(%ebp), %eax
	movl	%eax, 8(%ecx)
	movl	8(%ebp), %eax
	leave
	ret
	.def	___main;	.scl	2;	.type	32;	.endef
	.globl	_main
	.def	_main;	.scl	2;	.type	32;	.endef
_main:
	pushl	%ebp
	movl	%esp, %ebp
	andl	$-16, %esp
	subl	$32, %esp
	call	___main
	movl	$10, 28(%esp)
	movl	$10, 4(%esp)
	movl	28(%esp), %eax
	movl	%eax, (%esp)
	call	_bar
	movl	%eax, 28(%esp)
	movl	$0, %eax
	leave
	ret
	.ident	"GCC: (Rev6, Built by MSYS2 project) 13.1.0"
