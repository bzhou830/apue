.section .text
.global _start

msg:
	.ascii "hello, world!\n"

msg_end:
	.equ len, msg_end - msg
	.equ SYS_write, 1
	.equ SYS_exit, 60

_start:
	mov $SYS_write, %rax
	mov $1,%rdi
	mov $msg,%rsi
	mov $len,%rdx
	syscall
	
	mov $SYS_exit,%rax
	mov $0,%rdi
	syscall
