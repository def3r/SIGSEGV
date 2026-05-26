	.file	"simd.c"
	.text
	.globl	main
	.type	main, @function
main:
.LFB509:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$56, %rsp
	movl	$4, -96(%rbp)
	movl	$3, -92(%rbp)
	movl	$2, -88(%rbp)
	movl	$1, -84(%rbp)
	movl	-96(%rbp), %eax
	vmovd	-92(%rbp), %xmm0
	vpinsrd	$1, %eax, %xmm0, %xmm1
	movl	-88(%rbp), %eax
	vmovd	-84(%rbp), %xmm0
	vpinsrd	$1, %eax, %xmm0, %xmm0
	vpunpcklqdq	%xmm1, %xmm0, %xmm0
	vmovdqa	%xmm0, -80(%rbp)
	movl	$10, -112(%rbp)
	movl	$9, -108(%rbp)
	movl	$8, -104(%rbp)
	movl	$7, -100(%rbp)
	movl	-112(%rbp), %eax
	vmovd	-108(%rbp), %xmm0
	vpinsrd	$1, %eax, %xmm0, %xmm1
	movl	-104(%rbp), %eax
	vmovd	-100(%rbp), %xmm0
	vpinsrd	$1, %eax, %xmm0, %xmm0
	vpunpcklqdq	%xmm1, %xmm0, %xmm0
	vmovdqa	%xmm0, -64(%rbp)
	vmovdqa	-80(%rbp), %xmm0
	vmovdqa	%xmm0, -32(%rbp)
	vmovdqa	-64(%rbp), %xmm0
	vmovdqa	%xmm0, -16(%rbp)
	vmovdqa	-32(%rbp), %xmm1
	vmovdqa	-16(%rbp), %xmm0
	vpaddd	%xmm0, %xmm1, %xmm0
	vmovdqa	%xmm0, -48(%rbp)
	vmovdqa	-48(%rbp), %xmm0
	vpextrw	$0, %xmm0, %eax
	movzwl	%ax, %eax
	movl	%eax, -164(%rbp)
	vmovdqa	-48(%rbp), %xmm0
	vpextrw	$1, %xmm0, %eax
	movzwl	%ax, %eax
	movl	%eax, -160(%rbp)
	movl	-160(%rbp), %eax
	sall	$16, %eax
	movl	%eax, %edx
	movl	-164(%rbp), %eax
	addl	%edx, %eax
	movl	%eax, -156(%rbp)
	vmovdqa	-48(%rbp), %xmm0
	vpextrw	$2, %xmm0, %eax
	movzwl	%ax, %eax
	movl	%eax, -152(%rbp)
	vmovdqa	-48(%rbp), %xmm0
	vpextrw	$3, %xmm0, %eax
	movzwl	%ax, %eax
	movl	%eax, -148(%rbp)
	movl	-148(%rbp), %eax
	sall	$16, %eax
	movl	%eax, %edx
	movl	-152(%rbp), %eax
	addl	%edx, %eax
	movl	%eax, -144(%rbp)
	vmovdqa	-48(%rbp), %xmm0
	vpextrw	$4, %xmm0, %eax
	movzwl	%ax, %eax
	movl	%eax, -140(%rbp)
	vmovdqa	-48(%rbp), %xmm0
	vpextrw	$5, %xmm0, %eax
	movzwl	%ax, %eax
	movl	%eax, -136(%rbp)
	movl	-136(%rbp), %eax
	sall	$16, %eax
	movl	%eax, %edx
	movl	-140(%rbp), %eax
	addl	%edx, %eax
	movl	%eax, -132(%rbp)
	vmovdqa	-48(%rbp), %xmm0
	vpextrw	$6, %xmm0, %eax
	movzwl	%ax, %eax
	movl	%eax, -128(%rbp)
	vmovdqa	-48(%rbp), %xmm0
	vpextrw	$7, %xmm0, %eax
	movzwl	%ax, %eax
	movl	%eax, -124(%rbp)
	movl	-124(%rbp), %eax
	sall	$16, %eax
	movl	%eax, %edx
	movl	-128(%rbp), %eax
	addl	%edx, %eax
	movl	%eax, -120(%rbp)
	movl	-156(%rbp), %edx
	movl	-144(%rbp), %eax
	addl	%eax, %edx
	movl	-132(%rbp), %eax
	addl	%eax, %edx
	movl	-120(%rbp), %eax
	addl	%edx, %eax
	movl	%eax, -116(%rbp)
	movl	-116(%rbp), %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE509:
	.size	main, .-main
	.ident	"GCC: (GNU) 16.1.1 20260430"
	.section	.note.GNU-stack,"",@progbits
