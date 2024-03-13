/*
 * FreeRTOS Kernel <DEVELOPMENT BRANCH>
 * Copyright (C) 2024 Synopsys, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
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
 */

/**
 * \file
 * \ingroup OS_FREERTOS
 * \brief  freertos support for arc processor
 *  like task dispatcher, interrupt handler
 */
/** @cond OS_FREERTOS_ASM_ARC_SUPPORT */

/*
 * core-dependent part in assemble language (for arc)
 */
#define __ASSEMBLY__
#include "arc/arc.h"
#include "arc/arc_asm_common.h"

/*
 *  task dispatcher
 *
 */
    .text
    .align 4
    .global dispatch
dispatch:
/*
 *  the pre-conditions of this routine are task context, CPU is
 *  locked, dispatch is enabled.
 */
    SAVE_NONSCRATCH_REGS                    /* save callee save registers */
    mov     r1, dispatch_r
    PUSH    r1                              /* save return address */
    bl      arconnect_get_core_id           /* r0 = xCoreId */
    bl      dispatcher

/* return routine when task dispatch happened in task context */
dispatch_r:
    RESTORE_NONSCRATCH_REGS                 /* recover registers */
    j       [blink]

/*
 *  start dispatch
 */
    .global start_dispatch
    .align 4
start_dispatch:
/*
 *  this routine is called in the non-task context during the startup of the kernel
 *  , and all the interrupts are locked.
 *
 *  when the dispatcher is called, the cpu is locked, no nest exception (CPU exception/interrupt).
 *  In target_initialize, all interrupt priority mask should be cleared, cpu should be
 *  locked, the interrupts outside the kernel such as fiq can be
 *  enabled.
 */
    clri
    bl      arconnect_get_core_id           /* r0 = xCoreId */
    mov     r1, exc_nest_count
    add2    r2, r1, r0                      /* r2 = &exc_nest_count[xCoreId] */
    st      0, [r2]                         /* *r2 = 0 */
    b       dispatcher_0
/*
 *  dispatcher: xCoreId (r0)
 */
dispatcher:
    ld.as   r1, [pxCurrentTCBs, r0]         /* r1 = pxCurrentTCBs[xCoreId] */
    ld.as   r2, [ulCriticalNesting, r0]     /* r2 = ulCriticalNesting[xCoreId] */
    PUSH    r2                              /* save critical nesting */
    st      sp, [r1]                        /* save stack pointer of current task, r1 = pxCurrentTCBs[xCoreId] */
    jl      vTaskSwitchContext              /* change the value of pxCurrentTCBs[xCoreId] */
    bl      arconnect_get_core_id           /* r0 = xCoreId */
/*
 *  before dispatcher is called, task context | cpu locked | dispatch enabled
 *  should be satisfied. In this routine, the processor will jump
 *  into the entry of next to run task
 *
 *  i.e. kernel mode, IRQ disabled, dispatch enabled
 *
 * dispatcher_0: xCoreId (r0)
 */
dispatcher_0:
    ld.as   r1, [pxCurrentTCBs, r0]
    ld.as   sp, [r1]                        /* recover task stack from pxCurrentTCBs[xCoreId] */
    mov     r1, r0
#if ARC_FEATURE_STACK_CHECK
  #if ARC_FEATURE_SEC_PRESENT
    lr      r0, [AUX_SEC_STAT]
    bclr    r0, r0, AUX_SEC_STAT_BIT_SSC
    sflag   r0
  #else
    lr      r0, [AUX_STATUS32]
    bclr    r0, r0, AUX_STATUS_BIT_SC
    kflag   r0
  #endif
    PUSH    r1                              /* save xCoreId */
    jl      vPortSetStackCheck
    POP     r1                              /* restore xCoreId */
  #if ARC_FEATURE_SEC_PRESENT
    lr      r0, [AUX_SEC_STAT]
    bset    r0, r0, AUX_SEC_STAT_BIT_SSC
    sflag   r0
  #else
    lr      r0, [AUX_STATUS32]
    bset    r0, r0, AUX_STATUS_BIT_SC
    kflag   r0
  #endif
#endif
    POP     r0                              /* get critical nesting */
    mov     r2, ulCriticalNesting
    add2    r3, r2, r1                      /* r3 = &ulCriticalNesting[xCoreId] */
    st      r0, [r3]                        /* *r3 = r0 (ulCriticalNesting[xCoreId] = r0) */
    POP     r0
    j       [r0]

/*
 *  task startup routine
 *
 */
    .text
    .global start_r
    .align 4
start_r:
    seti                                    /* unlock cpu */
    mov     blink, vPortEndTask             /* set return address */
    POP     r1                              /* get task function body */
    POP     r0                              /* get task parameters */
    j       [r1]

/****** exceptions and interrupts handing ******/
/****** entry for exception handling ******/
    .global exc_entry_cpu
    .align 4
exc_entry_cpu:
    EXCEPTION_PROLOGUE
    mov     r3, sp                          /* as exception handler's para(p_excinfo) */
    bl      arconnect_get_core_id           /* r0 = xCoreId */
    mov     blink, sp
    mov     r1, exc_nest_count
    ld.as   r2, [exc_nest_count, r0]        /* r2 = exc_nest_count[xCoreId] */
    add     r5, r2, 1                       /* r5 = r2 + 1 */
    add2    r4, r1, r0                      /* r4 = &exc_nest_count[xCoreId] */
    st      r5, [r4]                        /* *r4 = r5 (exc_nest_count[xCoreId] = r5) */
    brne    r2, 0, exc_handler_1

exc_handler_1:
    PUSH    blink
    lr      r0, [AUX_ECR]
    lsr     r0, r0, 16
    mov     r1, exc_int_handler_table
    ld.as   r2, [r1, r0]
    mov     r0, r3
    jl      [r2]                            /* !!!!jump to exception handler where interrupts are not allowed! */

/* interrupts are not allowed */
ret_exc:
    POP     sp

    bl      arconnect_get_core_id           /* r0 = xCoreId */
    mov     r1, exc_nest_count
    add2    r3, r1, r0                      /* r3 = &exc_nest_count[xCoreId] */
    ld      r2, [r3]                        /* r2 = exc_nest_count[xCoreId] */
    sub     r2, r2, 1                       /* r2-- */
    st      r2, [r3]                        /* *r3 = r2 (exc_nest_count[xCoreId] = r2) */
    brne    r2, 0, ret_exc_1                /* nest exception case */

    lr      r1, [AUX_IRQ_ACT]               /* nest interrupt case */
    brne    r1, 0, ret_exc_1

    ld.as   r1, [context_switch_reqflg, r0] /* r1 = context_switch_reqflg[xCoreId] */
    brne    r1, 0, ret_exc_2

ret_exc_1:                                  /* return from non-task context, interrupts or exceptions are nested */
    EXCEPTION_EPILOGUE
    rtie

/* there is a dispatch request: xCoreId (r0) (from ret_exc) */
ret_exc_2:
    /* clear dispatch request */
    mov     r1, context_switch_reqflg
    add2    r2, r1, r0                      /* r2 = &context_switch_reqflg[xCoreId] */
    st      0, [r2]                         /* *r2 = 0 */

    ld.as   r1, [pxCurrentTCBs, r0]         /* r1 = pxCurrentTCBs[xCoreId] */
    breq    r1, 0, ret_exc_1

    SAVE_CALLEE_REGS    /* save callee save registers */

    lr      r1, [AUX_STATUS32]
    bclr    r1, r1, AUX_STATUS_BIT_AE       /* clear exception bit */
    kflag   r1

    mov     r1, ret_exc_r                   /* save return address */
    PUSH    r1

    /* r0 = xCoreId */
    bl      dispatcher

ret_exc_r:
    /* recover exception status */
    lr      r0, [AUX_STATUS32]
    bset    r0, r0, AUX_STATUS_BIT_AE
    kflag   r0

    RESTORE_CALLEE_REGS                     /* recover registers */
    EXCEPTION_EPILOGUE
    rtie

/****** entry for normal interrupt exception handling ******/
    .global exc_entry_int                   /* entry for interrupt handling */
    .align 4
exc_entry_int:
#if ARC_FEATURE_FIRQ == 1
  #if ARC_FEATURE_RGF_NUM_BANKS > 1
    lr      r0, [AUX_IRQ_ACT]               /* check whether it is P0 interrupt */
    btst    r0, 0
    jnz     exc_entry_firq
  #else
    PUSH    r10
    lr      r10, [AUX_IRQ_ACT]
    btst    r10, 0
    POP     r10
    jnz     exc_entry_firq
  #endif
#endif
    INTERRUPT_PROLOGUE
    clri                                    /* disable interrupt */
    bl      arconnect_get_core_id           /* r0 = xCoreId */
    mov     blink, sp
    mov     r1, exc_nest_count
    add2    r2, r1, r0                      /* r2 = &exc_nest_count[xCoreId] */
    ld      r3, [r2]                        /* r3 = exc_nest_count[xCoreId] */
    add     r4, r3, 1                       /* r4 = r3 + 1 */
    st      r4, [r2]                        /* *r2 = r4 (exc_nest_count[xCoreId] = r4) */
    seti                                    /* enable higher priority interrupt */
    brne    r3, 0, irq_handler_1

#if ARC_FEATURE_STACK_CHECK
  #if ARC_FEATURE_SEC_PRESENT
    lr      r0, [AUX_SEC_STAT]
    bclr    r0, r0, AUX_SEC_STAT_BIT_SSC
    sflag   r0
  #else
    lr      r0, [AUX_STATUS32]
    bclr    r0, r0, AUX_STATUS_BIT_SC
    kflag   r0
  #endif
#endif

irq_handler_1:
    PUSH    blink
    lr      r0, [AUX_IRQ_CAUSE]
    mov     r1, exc_int_handler_table
    ld.as   r2, [r1, r0]                    /* r2 = exc_int_handler_table + irqno *4 */
/* handle software triggered interrupt */
    lr      r3, [AUX_IRQ_HINT]
    cmp     r3, r0
    bne.d   irq_hint_handled
    xor     r3, r3, r3
    sr      r3, [AUX_IRQ_HINT]
irq_hint_handled:
    jl      [r2]                            /* jump to interrupt handler */

/* no interrupts are allowed from here */
ret_int:
    clri                                    /* disable interrupt */
    POP     sp
    bl      arconnect_get_core_id           /* r0 = xCoreId */
    mov     r4, r0                          /* r4 = xCoreId */
    mov     r1, exc_nest_count
    ld.as   r2, [exc_nest_count, r4]        /* r2 = exc_nest_count[xCoreId] */
    sub     r2, r2, 1                       /* r2-- */
    add2    r3, r1, r4                      /* r3 = &exc_nest_count[xCoreId] */
    st      r2, [r3]                        /* *r3 = r2 (exc_nest_count[xCoreId] = r2) */

/* if there are multi-bits set in IRQ_ACT, it's still in nest interrupt */
    lr      r0, [AUX_IRQ_CAUSE]
    sr      r0, [AUX_IRQ_SELECT]
    lr      r3, [AUX_IRQ_PRIORITY]
    lr      r1, [AUX_IRQ_ACT]
    bclr    r2, r1, r3
    brne    r2, 0, ret_int_1

    ld.as   r0, [context_switch_reqflg, r4] /* r0 = context_switch_reqflg[xCoreId] */
    brne    r0, 0, ret_int_2
ret_int_1:  /* return from non-task context */
    INTERRUPT_EPILOGUE
    rtie
/* there is a dispatch request: xCoreId (r4) (from ret_int) */
ret_int_2:
    /* clear dispatch request */
    mov     r0, context_switch_reqflg
    add2    r3, r0, r4                      /* r3 = &context_switch_reqflg[xCoreId] */
    st      0, [r3]                         /* *r3 = 0 */
    ld.as   r0, [pxCurrentTCBs, r4]         /* r0 = pxCurrentTCBs[xCoreId] */
    breq    r0, 0, ret_int_1

/* r1 has old AUX_IRQ_ACT */
    PUSH    r1
/* clear related bits in IRQ_ACT manually to simulate a irq return  */
    sr      r2, [AUX_IRQ_ACT]
    SAVE_CALLEE_REGS                        /* save callee save registers */
    mov     r1, ret_int_r                   /* save return address */
    PUSH    r1
    mov     r0, r4                          /* r0 = xCoreId */
    bl      dispatcher

ret_int_r:
    RESTORE_CALLEE_REGS                     /* recover registers */
    POPAX   AUX_IRQ_ACT
    INTERRUPT_EPILOGUE
    rtie

#if ARC_FEATURE_FIRQ == 1
    .global exc_entry_firq
    .align 4
exc_entry_firq:
  #if ARC_FEATURE_STACK_CHECK && ARC_FEATURE_RGF_NUM_BANKS > 1
    #if ARC_FEATURE_SEC_PRESENT
    lr      r0, [AUX_SEC_STAT]
    bclr    r0, r0, AUX_SEC_STAT_BIT_SSC
    sflag   r0
    #else
    lr      r0, [AUX_STATUS32]
    bclr    r0, r0, AUX_STATUS_BIT_SC
    kflag   r0
    #endif
  #endif
    SAVE_FIQ_EXC_REGS
    bl      arconnect_get_core_id           /* r0 = xCoreId */
    mov     blink, sp
    mov     r1, exc_nest_count
    ld.as   r2, [exc_nest_count, r0]        /* r3 = exc_nest_count[xCoreId] */
    add     r2, r2, 1                       /* r2++ */
    add2    r3, r1, r0                      /* r3 = &exc_nest_count[xCoreId] */
    st      r2, [r3]                        /* *r3 = r2  (exc_nest_count[xCoreId] = r2) */
    brne    r3, 0, firq_handler_1

  #if ARC_FEATURE_STACK_CHECK && ARC_FEATURE_RGF_NUM_BANKS == 1
    #if ARC_FEATURE_SEC_PRESENT
    lr      r0, [AUX_SEC_STAT]
    bclr    r0, r0, AUX_SEC_STAT_BIT_SSC
    sflag   r0
    #else
    lr      r0, [AUX_STATUS32]
    bclr    r0, r0, AUX_STATUS_BIT_SC
    kflag   r0
    #endif
  #endif

firq_handler_1:
    PUSH    blink
    lr      r0, [AUX_IRQ_CAUSE]
    mov     r1, exc_int_handler_table
    ld.as   r2, [r1, r0]                    /* r2 = exc_int_handler_table[irqno] */
/* handle software triggered interrupt */
    lr      r3, [AUX_IRQ_HINT]
    brne    r3, r0, firq_hint_handled
    xor     r3, r3, r3
    sr      r3, [AUX_IRQ_HINT]
firq_hint_handled:
    jl      [r2]                            /* jump to interrupt handler */

/* no interrupts are allowed from here */
ret_firq:
    clri
    POP     sp
    bl      arconnect_get_core_id           /* r0 = xCoreId */
    mov     r4, r0                          /* r4 = xCoreId */
    mov     r1, exc_nest_count
    ld.as   r2, [exc_nest_count, r4]        /* r3 = exc_nest_count[xCoreId] */
    sub     r2, r2, 1                       /* r2-- */
    add2    r3, r1, r4                      /* r3 = &exc_nest_count[xCoreId] */
    st      r2, [r3]                        /* *r3 = r2  (exc_nest_count[xCoreId] = r2) */

/* if there are multi-bits set in IRQ_ACT, it's still in nest interrupt */
    lr      r1, [AUX_IRQ_ACT]
    bclr    r1, r1, 0
    brne    r1, 0, ret_firq_1

    ld      r0, [context_switch_reqflg, r4] /* r0 = context_switch_reqflg[xCoreId] */
    brne    r0, 0, ret_firq_2
ret_firq_1:                                 /* return from non-task context */
    RESTORE_FIQ_EXC_REGS
    rtie
/* there is a dispatch request: xCoreId (r4) (from ret_firq) */
ret_firq_2:
    /* clear dispatch request */
    mov     r0, context_switch_reqflg
    add2    r3, r0, r4                      /* r3 = &context_switch_reqflg[xCoreId] */
    st      0, [r3]                         /* *r3 = 0 */
    ld.as   r0, [pxCurrentTCBs, r4]         /* r0 = pxCurrentTCBs[xCoreId] */
    breq    r0, 0, ret_firq_1

/* reconstruct the interruptted context
 * When ARC_FEATURE_RGF_BANKED_REGS >= 16 (16, 32), sp is banked
 * so need to restore the fast irq stack.
 */
  #if ARC_FEATURE_RGF_BANKED_REGS >= 16
    RESTORE_LP_REGS
    #if ARC_FEATURE_CODE_DENSITY
    RESTORE_CODE_DENSITY
    #endif
    RESTORE_R58_R59
  #endif

/* when BANKED_REGS == 16, r4-r9 wiil be also saved in fast irq stack
 * so pop them out
 */
  #if  ARC_FEATURE_RGF_BANKED_REGS == 16 && !defined(ARC_FEATURE_RF16)
    POP     r9
    POP     r8
    POP     r7
    POP     r6
    POP     r5
    POP     r4
  #endif

/* for other cases, unbanked regs are already in interrupted context's stack,
 * so just need to save and pop the banked regs
 */

/* save the interruptted context */
  #if ARC_FEATURE_RGF_BANKED_REGS > 0
/* switch back to bank0  */
    lr      r0, [AUX_STATUS32]
    bic     r0, r0, 0x70000
    kflag   r0
  #endif

  #if ARC_FEATURE_RGF_BANKED_REGS == 4
/* r4 - r12, gp, fp, r30, blink already saved */
    PUSH    r0
    PUSH    r1
    PUSH    r2
    PUSH    r3
  #elif ARC_FEATURE_RGF_BANKED_REGS == 8
/* r4 - r9, r0, r11 gp, fp, r30, blink already saved */
    PUSH    r0
    PUSH    r1
    PUSH    r2
    PUSH    r3
    PUSH    r12
  #elif ARC_FEATURE_RGF_BANKED_REGS >= 16
/* nothing is saved, */
    SAVE_R0_TO_R12

    SAVE_R58_R59
    PUSH    gp
    PUSH    fp
    PUSH    r30                             /* general purpose */
    PUSH    blink

    #if ARC_FEATURE_CODE_DENSITY
    SAVE_CODE_DENSITY
    #endif
    SAVE_LP_REGS
  #endif
    PUSH    ilink
    lr      r0, [AUX_STATUS32_P0]
    PUSH    r0
    lr      r0, [AUX_IRQ_ACT]
    PUSH    r0
    bclr    r0, r0, 0
    sr      r0, [AUX_IRQ_ACT]

    SAVE_CALLEE_REGS                        /* save callee save registers */

    mov     r1, ret_firq_r                  /* save return address */
    PUSH    r1
    bl      arconnect_get_core_id           /* r0 = xCoreId */
    bl      dispatcher

ret_firq_r:
    RESTORE_CALLEE_REGS                     /* recover registers */
    POPAX   AUX_IRQ_ACT
    POPAX   AUX_STATUS32_P0
    POP     ilink

  #if ARC_FEATURE_RGF_NUM_BANKS > 1
    #if ARC_FEATURE_RGF_BANKED_REGS == 4
/* r4 - r12, gp, fp, r30, blink already saved */
    POP     r3
    POP     r2
    POP     r1
    POP     r0
    RESTORE_FIQ_EXC_REGS
    #elif ARC_FEATURE_RGF_BANKED_REGS == 8
/* r4 - r9, gp, fp, r30, blink already saved */
    POP     r12
    POP     r3
    POP     r2
    POP     r1
    POP     r0
    RESTORE_FIQ_EXC_REGS
    #elif ARC_FEATURE_RGF_BANKED_REGS >= 16
    RESTORE_LP_REGS
      #if ARC_FEATURE_CODE_DENSITY
    RESTORE_CODE_DENSITY
      #endif
    POP     blink
    POP     r30
    POP     fp
    POP     gp

    RESTORE_R58_R59
    RESTORE_R0_TO_R12
    #endif /* ARC_FEATURE_RGF_BANKED_REGS  */
  #else
    RESTORE_FIQ_EXC_REGS
  #endif /* ARC_FEATURE_RGF_NUM_BANKS */
    rtie
#endif

/*
 *  acquire_spin_lock
 *    r0 = address of spin lock
 */
    .text
    .global acquire_spin_lock
    .align 4
acquire_spin_lock:
    mov     r1, 0x1
wait_for_release:
    ex      r1, [r0]
    cmp     r1, 0
    bne     wait_for_release
    nop
    j       [blink]

/*
 *  release_spin_lock
 *    r0 = address of spin lock
 */
    .global release_spin_lock
    .align 4
release_spin_lock:
    st      0, [r0]
    j       [blink]

/** @endcond */