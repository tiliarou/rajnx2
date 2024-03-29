/*
 * Copyright (c) 2018-2019 Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
.macro CLEAR_GPR_REG_ITER
    mov r\@, #0
.endm

.section .text.start, "ax", %progbits
.arm
.align 5
.global _start
.type   _start, %function
_start:
    /* Switch to system mode, mask all interrupts, clear all flags */
    msr cpsr_cxsf, #0xDF

    /* Relocate ourselves if necessary */
    ldr r2, =__start__
    adr r3, _start
    cmp r2, r3
    bne _relocation_loop_end

    ldr r4, =__bss_start__
    sub r4, r4, r2          /* size >= 32, obviously, and we've declared 32-byte-alignment */
    _relocation_loop:
        ldmia r3!, {r5-r12}
        stmia r2!, {r5-r12}
        subs  r4, #0x20
        bne _relocation_loop

    ldr r12, =_relocation_loop_end
    bx  r12

    _relocation_loop_end:
    /* Set the stack pointer */
    ldr  sp, =__stack_top__
    mov  fp, #0
    bl  __program_init

    /* Set r0 to r12 to 0 (for debugging) & call main */
    .rept 13
    CLEAR_GPR_REG_ITER
    .endr
    ldr r0, =__program_argc
    ldr r1, =__program_argv
    ldr lr, =__program_exit
    ldr r0, [r0]
    ldr r1, [r1]
    b   main

/* No need to include this in normal programs: */
.section .chainloader.text.start, "ax", %progbits
.arm
.align 5
.global relocate_and_chainload
.type   relocate_and_chainload, %function
relocate_and_chainload:
    ldr sp, =__stack_top__
    b   relocate_and_chainload_main
