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
	lrl   r10, [MACRO_ARG(aux)]
	PUSHL r10
.endm

.macro ASM_MACRO1(POPAX, aux)
	POPL r10
	srl  r10, [MACRO_ARG(aux)]
.endm

/*--------------------------------------------------------------
 * Helpers to save/restore callee-saved regs:
 * used by several macros below
 *-------------------------------------------------------------*/
.macro SAVE_CALLEE_REGS
	PUSHL	r13
	PUSHL	r14
	PUSHL	r15
	PUSHL	r16
	PUSHL	r17
	PUSHL	r18
	PUSHL	r19
	PUSHL	r20
	PUSHL	r21
	PUSHL	r22
	PUSHL	r23
	PUSHL	r24
	PUSHL	r25
.endm

.macro RESTORE_CALLEE_REGS
	POPL	r25
	POPL	r24
	POPL	r23
	POPL	r22
	POPL	r21
	POPL	r20
	POPL	r19
	POPL	r18
	POPL	r17
	POPL	r16
	POPL	r15
	POPL	r14
	POPL	r13
.endm

.macro SAVE_LP_REGS
	PUSHL	 r60
	PUSHAX AUX_LP_START
	PUSHAX AUX_LP_END
.endm

.macro RESTORE_LP_REGS
	POPAX	AUX_LP_END
	POPAX	AUX_LP_START
	POPL	r10
/* must not use the LP_COUNT register(r60) as the destination of multi-cycle instruction */
	movl	r60, r10

.endm

/* macro to save r0 to r12 */
.macro SAVE_R0_TO_R12
	PUSHL	r0
	PUSHL	r1
	PUSHL	r2
	PUSHL	r3
	PUSHL	r4
	PUSHL	r5
	PUSHL	r6
	PUSHL	r7
	PUSHL	r8
	PUSHL	r9
	PUSHL	r10
	PUSHL	r11
	PUSHL	r12
.endm

/* macro to restore r0 to r12 */
.macro RESTORE_R0_TO_R12
	POPL	r12
	POPL	r11
	POPL	r10
	POPL	r9
	POPL	r8
	POPL	r7
	POPL	r6
	POPL	r5
	POPL	r4
	POPL	r3
	POPL	r2
	POPL	r1
	POPL	r0
.endm

/* macro to save all non-caller saved regs */
.macro SAVE_NONSCRATCH_REGS
/* caller saved regs are saved by caller function */
	PUSHL	gp
	PUSHL	fp
	PUSHL	blink
	SAVE_CALLEE_REGS
.endm

/* macro to restore all non-caller saved regs */
.macro RESTORE_NONSCRATCH_REGS
	RESTORE_CALLEE_REGS
	POPL	blink
	POPL	fp
	POPL	gp
.endm

/* normal interrupt prologue, pc, status and r0-r11 are saved by hardware */
.macro INTERRUPT_PROLOGUE
	PUSHL	r12
	PUSHL	gp
	PUSHL	fp
	PUSHL	ilink
	PUSHL	r30

	subl  sp, sp, 8 /* skip bta */
.endm


/* normal interrupt epilogue, pc, status and r0-r11 are restored by hardware */
.macro INTERRUPT_EPILOGUE
	addl  sp, sp, 8 /* skip bta */

	POPL	r30
	POPL	ilink
	POPL	fp
	POPL	gp
	POPL	r12
.endm

/* exception prologue, create the same frame of interrupt manually */
.macro EXCEPTION_PROLOGUE
	stl.as	r10, [sp, -8]
	PUSHAX	AUX_ERSTATUS
	PUSHAX	AUX_ERRET

	SAVE_LP_REGS

	PUSHL	blink

	PUSHL	r11
	subl	sp, sp, 8 /* r10 is  pushed before */
	PUSHL	r9
	PUSHL	r8
	PUSHL	r7
	PUSHL	r6
	PUSHL	r5
	PUSHL	r4
	PUSHL	r3
	PUSHL	r2
	PUSHL	r1
	PUSHL	r0

	PUSHL	r12
	PUSHL	gp
	PUSHL	fp
	PUSHL	ilink
	PUSHL	r30

	PUSHAX	AUX_ERBTA
.endm

/* exception epilogue, restore the same frame of interrupt manually */
.macro EXCEPTION_EPILOGUE
	POPAX	AUX_ERBTA

	POPL	r30
	POPL	ilink
	POPL	fp
	POPL	gp
	POPL	r12

	POPL	r0
	POPL	r1
	POPL	r2
	POPL	r3
	POPL	r4
	POPL	r5
	POPL	r6
	POPL	r7
	POPL	r8
	POPL	r9
	addl	sp, sp, 8 /* r10 will be popped finally */
	POPL	r11

	POPL	blink

	RESTORE_LP_REGS

	POPAX	AUX_ERRET
	POPAX	AUX_ERSTATUS

	ldl.as	r10, [sp, -8]
.endm

#endif  /* __ASSEMBLY__ */
#endif	/* _ARC_HAL_ASM_COMMON_H */
/** @endcond */
