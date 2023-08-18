# Sys V AMD64 calling convention
# Arguments: RDI, RSI, RDX, RCX, R8, R9
# Returns: RAX, RDX
# Non-Volatile (callee saved): RBX, RSP, RBP, R12-15
# Volatile (caller saved): RAX, RCX, RDX, RDI, RSI, R8-R11, RIP

.section .note.GNU-stack,"",@progbits

.text
.global cthread_asm_switch
; .global cthread_asm_start
.global cthread_asm_altstack
.global cthread_main_ret
.global cthread_exit
.global __morestack


# We want to skip the stack around and have gdb follow it. GDB looks for this particular
# symbol name as it's what gcc uses for the -fsplit-stack option.
__morestack:
	# debuggers don't like the return address being the beginning of a function
	# so insert a nop and use &__morestack + 1
	nop

	# in0 - rax - return code
	# rsp should now point to cthread structure

	# call cthread_exit
	# does not return
	# in0 - rdi - return code
	mov %rax,%rdi
	call cthread_exit
	hlt


cthread_asm_switch:
	# in0 - rdi - data argument
	# in1 - rsi - load stack
	# in2 - rdx - save stack
	# in3 - rcx - save frame

	# Save the frame details to the target thread so that debuggers can unwind it.
	# stack pointer
	lea 8(%rsp),%rax
	mov %rax,0(%rcx)
	# frame pointer
	mov %rbp,8(%rcx)
	# return address
	mov (%rsp), %rax
	mov %rax,16(%rcx)

	# Save source non-volatile registers to the source stack
	push %rbp
	push %rbx
	push %r12
	push %r13
	push %r14
	push %r15
	# store source stack so that we can restore it on yield
	mov %rsp,(%rdx)

	# restore non-volatile registers from the load stack
switch_point:
	mov %rsi, %rsp
	pop %r15
	pop %r14
	pop %r13
	pop %r12
	pop %rbx
	pop %rbp
	
	# out0 - rax - data argument
	mov %rdi,%rax

	ret

cthread_asm_altstack:
	# in0 - rdi - arg0
	# in1 - rsi - arg1
	# in2 - rdx - load stack
	# in3 - rcx - function

	# swap to the new stack
	xchg %rdx,%rsp
	# save the old stack pointer onto the new stack
	push %rdx

	# call the target function
	# in0 - rdi - argument
	# out0 - rax
	call *%rcx

	# restore the old stack
	pop %rsp

	# leave rax as return value
	ret
