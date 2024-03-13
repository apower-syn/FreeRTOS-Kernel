/*
 * FreeRTOS Kernel V10.6.0
 * Copyright (C) 2023 Synopsys, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 * 1 tab == 4 spaces!
 */

/**
 * \file
 * \ingroup ARC_HAL_MISC
 * \brief common macro definitions for assembly file
 */
/** @cond ARC_HAL_ASM_COMMON */

#ifndef _ARC_HAL_ASM_COMMON_H_
#define _ARC_HAL_ASM_COMMON_H_

#include "arc/arc.h"

#ifdef __ASSEMBLY__
/* the assembly macro definitions in ARC GNU and MWDT are
 * different, so need different processing
 */
#if defined(__GNU__)
#define MACRO_ARG(x) \x
#define ASM_MACRO1(name, arg1) name arg1
#else
#define MACRO_ARG(x) x
#define ASM_MACRO1(name, arg1) name, arg1
#endif

/* Note on the LD/ST addr modes with addr reg wback
 *
 * LD.a same as LD.aw
 *
 * LD.a    reg1, [reg2, x]  => Pre Incr
 *      Eff Addr for load = [reg2 + x]
 *
 * LD.ab   reg1, [reg2, x]  => Post Incr
 *      Eff Addr for load = [reg2]
 */
.macro ASM_MACRO1(PUSHAX, aux)
	lr r10, [MACRO_ARG(aux)]
	PUSH r10
.endm

.macro ASM_MACRO1(POPAX, aux)
	POP r10
	sr r10, [MACRO_ARG(aux)]
.endm

/*--------------------------------------------------------------
 * Helpers to save/restore callee-saved regs:
 * used by several macros below
 *-------------------------------------------------------------*/
.macro SAVE_CALLEE_REGS
	PUSH	r13
	PUSH	r14
	PUSH	r15
	PUSH	r16
	PUSH	r17
	PUSH	r18
	PUSH	r19
	PUSH	r20
	PUSH	r21
	PUSH	r22
	PUSH	r23
	PUSH	r24
	PUSH	r25
.endm

.macro RESTORE_CALLEE_REGS
	POP	r25
	POP	r24
	POP	r23
	POP	r22
	POP	r21
	POP	r20
	POP	r19
	POP	r18
	POP	r17
	POP	r16
	POP	r15
	POP	r14
	POP	r13
.endm

.macro SAVE_LP_REGS
	PUSH	r60
	PUSHAX	AUX_LP_START
	PUSHAX	AUX_LP_END
.endm

.macro RESTORE_LP_REGS
	POPAX	AUX_LP_END
	POPAX	AUX_LP_START
	POP	r10
/* must not use the LP_COUNT register(r60) as the destination of multi-cycle instruction */
	mov	r60, r10

.endm

/* macro to save r0 to r12 */
.macro SAVE_R0_TO_R12
	PUSH	r0
	PUSH	r1
	PUSH	r2
	PUSH	r3
	PUSH	r4
	PUSH	r5
	PUSH	r6
	PUSH	r7
	PUSH	r8
	PUSH	r9
	PUSH	r10
	PUSH	r11
	PUSH	r12
.endm

/* macro to restore r0 to r12 */
.macro RESTORE_R0_TO_R12
	POP	r12
	POP	r11
	POP	r10
	POP	r9
	POP	r8
	POP	r7
	POP	r6
	POP	r5
	POP	r4
	POP	r3
	POP	r2
	POP	r1
	POP	r0
.endm

/* macro to save all non-caller saved regs */
.macro SAVE_NONSCRATCH_REGS
/* caller saved regs are saved by caller function */
	PUSH	gp
	PUSH	fp
	PUSH	blink
	SAVE_CALLEE_REGS
.endm

/* macro to restore all non-caller saved regs */
.macro RESTORE_NONSCRATCH_REGS
	RESTORE_CALLEE_REGS
	POP	blink
	POP	fp
	POP	gp
.endm

/* normal interrupt prologue, pc, status and r0-r11 are saved by hardware */
.macro INTERRUPT_PROLOGUE
	PUSH	r12
	PUSH	gp
	PUSH	fp
	PUSH	ilink
	PUSH	r30

	sub	sp, sp, 4 /* skip bta */
.endm


/* normal interrupt epilogue, pc, status and r0-r11 are restored by hardware */
.macro INTERRUPT_EPILOGUE
	add	sp, sp, 4 /* skip bta */

	POP	r30
	POP	ilink
	POP	fp
	POP	gp
	POP	r12
.endm

/* exception prologue, create the same frame of interrupt manually */
.macro EXCEPTION_PROLOGUE
	st.as	r10, [sp, -8]
	PUSHAX	AUX_ERSTATUS
	PUSHAX	AUX_ERRET

	SAVE_LP_REGS

	PUSH	blink

	PUSH	r11
	sub	sp, sp, 4 /* r10 is  pushed before */
	PUSH	r9
	PUSH	r8
	PUSH	r7
	PUSH	r6
	PUSH	r5
	PUSH	r4
	PUSH	r3
	PUSH	r2
	PUSH	r1
	PUSH	r0

	PUSH	r12
	PUSH	gp
	PUSH	fp
	PUSH	ilink
	PUSH	r30

	PUSHAX	AUX_ERBTA
.endm

/* exception epilogue, restore the same frame of interrupt manually */
.macro EXCEPTION_EPILOGUE
	POPAX	AUX_ERBTA

	POP	r30
	POP	ilink
	POP	fp
	POP	gp
	POP	r12

	POP	r0
	POP	r1
	POP	r2
	POP	r3
	POP	r4
	POP	r5
	POP	r6
	POP	r7
	POP	r8
	POP	r9
	add	sp, sp, 4 /* r10 will be popped finally */
	POP	r11

	POP	blink

	RESTORE_LP_REGS

	POPAX	AUX_ERRET
	POPAX	AUX_ERSTATUS

	ld.as	r10, [sp, -8]
.endm

#endif  /* __ASSEMBLY__ */
#endif	/* _ARC_HAL_ASM_COMMON_H */
/** @endcond */