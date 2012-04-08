	.file	1 "halt.c"

 # GNU C 2.4.5 [AL 1.1, MM 40] DECstation running ultrix compiled by CC

 # Cc1 defaults:

 # Cc1 arguments (-G value = 0, Cpu = default, ISA = 1):
 # -G -quiet -dumpbase -o

gcc2_compiled.:
__gnu_compiled_c:
	.rdata
	.align	2
$LC0:
	.ascii	"../test/testfiles\000"
	.text
	.align	2
	.globl	main

	.loc	1 19
	.ent	main
main:
	.frame	$fp,24,$31		# vars= 0, regs= 2/0, args = 16, extra= 0
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	sw	$31,20($sp)
	sw	$fp,16($sp)
	move	$fp,$sp
	jal	__main
	la	$4,$LC0
	li	$5,0x00000011		# 17
	jal	Exec
$L1:
	.set	noreorder
	move	$sp,$fp			# sp not trusted here
	lw	$31,20($sp)
	lw	$fp,16($sp)
	j	$31
	addu	$sp,$sp,24
	.set	reorder
	.end	main

	.comm	a,16

	.comm	b,8

	.comm	c,8
