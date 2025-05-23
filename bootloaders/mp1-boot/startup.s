.syntax unified
.cpu cortex-a7

.equ MODE_FIQ, 0x11
.equ MODE_IRQ, 0x12
.equ MODE_SVC, 0x13
.equ MODE_ABT, 0x17
.equ MODE_UND, 0x1B
.equ MODE_SYS, 0x1F

.section .vector_table, "x"
.global _Reset
.global _start
_Reset:
    b Reset_Handler
    b Undef_Handler 								// 0x4  Undefined Instruction 
    b SVC_Handler 									// Software Interrupt 
    b PAbt_Handler  								// 0xC  Prefetch Abort
    b DAbt_Handler 									// 0x10 Data Abort
    b . 											// 0x14 Reserved 
    b IRQ_Handler 									// 0x18 IRQ 
    b FIQ_Handler 									// 0x1C FIQ

.section .text
Reset_Handler:
	cpsid   if 										// Mask Interrupts

	mrc     p15, 0, r0, c1, c0, 0					// Read System Control register (SCTLR)
    bic     r0, r0, #0b11100000000000
    bic     r0, r0, #0b00000000000101
	mcr     p15, 0, r0, c1, c0, 0 					// Write System Control register (SCTLR)
	isb
													// Configure ACTLR
	mrc     p15, 0, r0, c1, c0, 1 					// Read CP15 Auxiliary Control Register
	orr     r0, r0, #(1 <<  1) 						// Enable L2 prefetch hint 
	mcr     p15, 0, r0, c1, c0, 1 					// Write CP15 Auxiliary Control Register

													// Set Vector Base Address Register (VBAR) to point to this application's vector table
	ldr    r0, =0x2FFC2500
	mcr    p15, 0, r0, c12, c0, 0

    												// FIQ stack: Fill with FEFF
    msr cpsr_c, MODE_FIQ
    ldr r1, =_fiq_stack_start
    ldr sp, =_fiq_stack_end
    movw r0, #0xFEFF
    movt r0, #0xFEFF
fiq_loop:
    cmp r1, sp
    strlt r0, [r1], #4
    blt fiq_loop

   													// IRQ stack: Fill will F1F1
    msr cpsr_c, MODE_IRQ
    ldr r1, =_irq_stack_start
    ldr sp, =_irq_stack_end
    movw r0, #0xF1F1
    movt r0, #0xF1F1
irq_loop:
    cmp r1, sp
    strlt r0, [r1], #4
    blt irq_loop

   													// Supervisor (SVC) stack: Fill with F5F5
    msr cpsr_c, MODE_SVC
    ldr r1, =_svc_stack_start
    ldr sp, =_svc_stack_end
    movw r0, #0xF5F5
    movt r0, #0xF5F5
svc_loop:
    cmp r1, sp
    strlt r0, [r1], #4
    blt svc_loop

													// USER and SYS mode stack: Fill with F0F0
	msr cpsr_c, MODE_SYS
    ldr r1, =_user_stack_start
	ldr sp, =_user_stack_end
    movw r0, #0xF0F0
    movt r0, #0xF0F0
usrsys_loop:
    cmp r1, sp
    strlt r0, [r1], #4
    blt usrsys_loop

    												// Copying initialization values (.data)
    ldr r0, =_text_end
    ldr r1, =_data_start
    ldr r2, =_data_end

data_loop:
    cmp r1, r2
    ldrlt r3, [r0], #4
    strlt r3, [r1], #4
    blt data_loop

    												// Initialize .bss
    mov r0, #0
    ldr r1, =_bss_start
    ldr r2, =_bss_end

bss_loop:
    cmp r1, r2
    strlt r0, [r1], #4
    blt bss_loop

	bl SystemInit 									// Setup MMU, TLB, Caches, FPU, IRQ
    bl __libc_init_array 							// libc init (static constructors)

	//Do not enable IRQ interrupts, this project doesn't use them
	//cpsie  i 

run_main:
    bl main
    b Abort_Exception

Abort_Exception:
	b .

Undef_Handler:
	b .

SVC_Handler:
	b .

PAbt_Handler:
	b .

DAbt_Handler:
    mcr     p15, 0, r0, c5, c0, 0   // Read DFSR into r0.
	b .

IRQ_Handler:
	b .

FIQ_Handler:
	b .

.global syscall_start_program
syscall_start_program:
    push    {r0}

    // Initialize translation table.
    bl MMU_CreateTranslationTable

    // System control changes.
    mrc     p15, 0, r0, c1, c0, 0       // Read SCTLR into r0.
    orr     r0, r0, #0b1100000000000    // Set bits to enable instruction cache and branch prediction.
    orr     r0, r0, #0b0000000000001    // Set bits to enable memory management unit.
    mcr     p15, 0, r0, c1, c0, 0       // Write SCTLR from r0.

    // // SIMD and floating-point.
    // mov     r0, 0x00F00000              // Prepare CPACR value in r0 to enable advanced SIMD and floating-point functionality.
    // mcr     p15, 0, r0, c1, c0, 2       // Write CPACR from r0.

    // ; mov     sp, 0x30000000              // Set the stack pointer to the top of SYSRAM.

    // Enable IRQs.
    bl      IRQ_Initialize


    // Start program.
    pop     {r0}
    mcr     p15, 0, r0, c12, c0, 0      // Set VBAR to the program entry point.
    isb                                 // Instruction synchronization barrier.
    mov     lr, 0xffffffff              // Clear out the link register to ensure it is an error to "return" to the bootloader.
    bx      r0                          // Jump to the program entry point.
