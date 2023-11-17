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
 * @file
 * @ingroup ARC_HAL_EXCEPTION_CPU ARC_HAL_EXCEPTION_INTERRUPT
 * @brief header file of exception and interrupt management module
 */

#ifndef _ARC_HAL_EXCEPTION_H_
#define _ARC_HAL_EXCEPTION_H_

#include "arc/arc.h"
#include "arc/arc_builtin.h"

#ifndef __ASSEMBLY__

/**
 * @addtogroup ARC_HAL_EXCEPTION_CPU
 * @{
 */
#ifndef NUM_EXC_CPU
/*!< number of CPU exceptions */
#define NUM_EXC_CPU             16
#endif

/*!< total number of exceptions */
#define NUM_EXC_ALL             (NUM_EXC_CPU + NUM_EXC_INT)

typedef struct int_exc_frame {
	uint32_t erbta;

	uint32_t r30;   /* r30 is useless, skipped? */
	uint32_t ilink; /* r29 is useless, skipped?*/
	/* r28 is sp, saved other place */
	uint32_t fp;    /* r27 */
	uint32_t gp;    /* r26 */

	uint32_t r12;
	uint32_t r0, r1, r2, r3;
	uint32_t r4, r5, r6, r7, r8, r9;
	uint32_t r10, r11;
	uint32_t blink; /* r31 */
	uint32_t lp_end, lp_start, lp_count;
	uint32_t ret;
	uint32_t status32;
} __attribute__((packed)) INT_EXC_FRAME_T;

typedef struct callee_frame {
	uint32_t r25;
	uint32_t r24;
	uint32_t r23;
	uint32_t r22;
	uint32_t r21;
	uint32_t r20;
	uint32_t r19;
	uint32_t r18;
	uint32_t r17;
	uint32_t r16;
	uint32_t r15;
	uint32_t r14;
	uint32_t r13;
} __attribute__((packed)) CALLEE_FRAME_T;

typedef struct processor_frame {
	CALLEE_FRAME_T callee_regs;
	INT_EXC_FRAME_T exc_frame;
} __attribute__((packed)) PROCESSOR_FRAME_T;

#define ARC_PROCESSOR_FRAME_T_SIZE        (sizeof(PROCESSOR_FRAME_T) / sizeof(uint32_t))
#define ARC_EXC_FRAME_T_SIZE              (sizeof(INT_EXC_FRAME_T) / sizeof(uint32_t))
#define ARC_CALLEE_FRAME_T_SIZE           (sizeof(CALLEE_FRAME_T) / sizeof(uint32_t))

extern uint32_t exc_nest_count;

#define INT_PRI_MAX (-1)                /*!< the maximum interrupt priority */
/**
 * @brief disable the specific interrupt
 *
 * @param[in] intno interrupt number
 */
Inline void arc_int_disable(const uint32_t intno)
{
	arc_aux_write(AUX_IRQ_SELECT, intno);
	arc_aux_write(AUX_IRQ_ENABLE, 0);
}

/**
 * @brief  enable the specific int
 *
 * @param[in] intno interrupt number
 */
Inline void arc_int_enable(const uint32_t intno)
{
	arc_aux_write(AUX_IRQ_SELECT, intno);
	arc_aux_write(AUX_IRQ_ENABLE, 1);
}

/**
 * @brief  set the interrupt priority mask
 *
 * @param[in] intpri interrupt priority
 */
Inline void arc_int_ipm_set(uint32_t intpri)
{
	volatile uint32_t status;

	status = arc_aux_read(AUX_STATUS32) & ~0x1e;

	status = status | ((intpri << 1) & 0x1e);
	/* sr cannot write AUX_STATUS32 */
	Asm("kflag %0" ::"ir" (status));
}

/**
 * @brief set interrupt priority
 *
 * @param[in] intno interrupt number
 * @param[in] intpri interrupt priority
 */
Inline void arc_int_pri_set(const uint32_t intno, uint32_t intpri)
{
	arc_aux_write(AUX_IRQ_SELECT, intno);
	arc_aux_write(AUX_IRQ_PRIORITY, intpri | (arc_aux_read(AUX_IRQ_PRIORITY) & 0xfffffff0));
}

/**
 * @brief  lock cpu, disable interrupts
 */
Inline void arc_lock(void)
{
	arc_clri();
}

/**
 * @brief  unlock cpu, enable interrupts to happen
 */
Inline void arc_unlock(void)
{
	arc_seti(0);
}

/**
 * @brief  lock cpu and status
 *
 * @returns cpu status
 */
Inline uint32_t arc_lock_save(void)
{
	return arc_clri();
}

/**
 * @brief  unlock cpu with the specific status
 *
 * @param[in] status  cpu status saved by cpu_lock_save
 */
Inline void arc_unlock_restore(const uint32_t status)
{
	arc_seti(status);
}

/** @}*/

/**
 * @addtogroup ARC_HAL_EXCEPTION_CPU
 * @{
 */
/**
 * @typedef EXC_ENTRY_T
 * @brief  the data type for exception entry
 */
typedef void (*EXC_ENTRY_T) (void);
/**
 * @typedef EXC_HANDLER_T
 * @brief  the data type for exception handler
 */
typedef void (*EXC_HANDLER_T) (void *exc_frame);
/** @}*/

/**
 * @ingroup ARC_HAL_EXCEPTION_INTERRUPT
 * @typedef INT_HANDLER_T
 * @brief  the data type for interrupt handler
 */
typedef void (*INT_HANDLER_T) (void *ptr);

extern EXC_ENTRY_T exc_entry_table[NUM_EXC_ALL];
extern EXC_HANDLER_T exc_int_handler_table[NUM_EXC_ALL];

/** @ingroup ARC_HAL_EXCEPTION_CPU
 * @{
 */
/**
 * @fn _arc_reset
 * @brief the reset entry
 */
extern void _arc_reset(void);
/**
 * @fn exc_entry_cpu
 * @brief the default CPU exception entry
 */
extern void exc_entry_cpu(void);

/**
 * @fn exc_entry_int
 * @brief the interrupt exception entry
 */
extern void exc_entry_int(void);
/** @}*/

/* exception related APIs */
extern void exc_int_init(void);

/* interrupt related APIs */
extern int32_t int_disable(const uint32_t intno);
extern int32_t int_enable(const uint32_t intno);

/**
 * @brief  get current interrupt priority mask
 *
 * @param[in] intno interrupt number
 * @return  <0 interrupt priority, 0 error
 */
extern int32_t int_pri_set(const uint32_t intno, int32_t intpri);
extern uint32_t cpu_lock_save(void);
extern void cpu_unlock_restore(const uint32_t status);
extern int32_t int_handler_install(const uint32_t intno, INT_HANDLER_T handler);

#endif  /* __ASSEMBLY__ */

#endif /* _ARC_HAL_EXCEPTION_H_*/