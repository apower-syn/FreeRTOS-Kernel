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
 * @brief C Implementation of exception and interrupt management
 */
#include "arc/arc_exception.h"

/**
 * @addtogroup ARC_HAL_EXCEPTION_CPU
 * @{
 * @var exc_entry_table
 * @brief exception entry table
 *
 * install exception entry table to ARC_AUX_INT_VECT_BASE in startup.
 * According to ARCv2 ISA, vectors are fetched in instruction space and thus
 * may be present in ICCM, Instruction Cache, or
 * main memory accessed by instruction fetch logic.
 * So it is put into a specific section .vector.
 *
 * Please note that the exc_entry_table maybe cached in ARC. Some functions is
 * defined in .s files.
 *
 */

/**
 * @ingroup ARC_HAL_EXCEPTION_CPU
 * @brief  default cpu exception handler
 * @param p_excinf pointer to the exception frame
 */
static void exc_handler_default(void *p_excinf)
{
    arc_halt();
}

/**
 * @ingroup ARC_HAL_EXCEPTION_INTERRUPT
 * @brief  default interrupt handler
 * @param[in] p_excinf	information for interrupt handler
 */
static void int_handler_default(void *p_excinf)
{
    arc_halt();
}

#if ARC_FEATURE_USE_DEFAULT_EXC_ENTRY_TABLE == 1
__attribute__ ((aligned(2048), section(".vector")))
EXC_ENTRY_T exc_entry_table[NUM_EXC_ALL] = {
    [0] = _arc_reset,
    [1 ... NUM_EXC_CPU - 1] = exc_entry_cpu,
    [NUM_EXC_CPU ... NUM_EXC_ALL - 1] = exc_entry_int
};
#endif

#if ARC_FEATURE_USE_DEFAULT_EXC_INT_HANDLER_TABLE == 1
/**
 * @var exc_int_handler_table
 * @brief the cpu exception and interrupt exception handler table
 * called in exc_entry_default and exc_entry_int
 */
EXC_HANDLER_T exc_int_handler_table[NUM_EXC_ALL] = {
    [0 ... NUM_EXC_CPU - 1] = exc_handler_default,
    [NUM_EXC_CPU ... NUM_EXC_ALL - 1] = int_handler_default
};
#endif

typedef struct aux_irq_ctrl_field {
    /* note: little endian */
    uint32_t save_nr_gpr_pairs : 5;         /** Indicates number of general-purpose register pairs saved, from 0 to 8/16 */
    uint32_t res : 4;                       /** Reserved */
    uint32_t save_blink : 1;                /** Indicates whether to save and restore BLINK */
    uint32_t save_lp_regs : 1;              /** Indicates whether to save and restore loop registers (LP_COUNT, LP_START, LP_END) */
    uint32_t save_u_to_u : 1;               /** Indicates if user context is saved to user stack */
    uint32_t res2 : 1;                      /** Reserved */
    uint32_t save_idx_regs : 1;             /** Indicates whether to save and restore code-density registers (EI_BASE, JLI_BASE, LDI_BASE) */
    uint32_t res3 : 18;                     /** Reserved */
} aux_irq_ctrl_field_t;

typedef union {
    aux_irq_ctrl_field_t bits;
    uint32_t value;
} aux_irq_ctrl_t;

/**
 * @ingroup ARC_HAL_EXCEPTION_CPU ARC_HAL_EXCEPTION_INTERRUPT
 * @brief  initialize the exception and interrupt handling
 */
void exc_int_init(void)
{
    uint32_t i;
    uint32_t status;
    aux_irq_ctrl_t ictrl;

    ictrl.value = 0;

    ictrl.bits.save_nr_gpr_pairs = 6;       /* r0 to r11 (r12 saved manually) */

    ictrl.bits.save_blink = 1;
    ictrl.bits.save_lp_regs = 1;            /* LP_COUNT, LP_START, LP_END */
    ictrl.bits.save_u_to_u = 0;             /* user ctxt saved on kernel stack */

    status = arc_lock_save();
    for (i = NUM_EXC_CPU; i < NUM_EXC_ALL; i++) {
        /* interrupt level triggered, disabled, priority is the lowest */
        arc_aux_write(AUX_IRQ_SELECT, i);
        arc_aux_write(AUX_IRQ_ENABLE, 0);
        arc_aux_write(AUX_IRQ_TRIGGER, 0);
        arc_aux_write(AUX_IRQ_PRIORITY, INT_PRI_MAX - INT_PRI_MIN);
    }
    arc_aux_write(AUX_IRQ_CTRL, ictrl.value);

    arc_unlock_restore(status);

    /** ipm should be set after cpu unlock restore to avoid reset of the status32 value */
    arc_int_ipm_set((INT_PRI_MAX - INT_PRI_MIN));
}

/**
 * @ingroup ARC_HAL_EXCEPTION_CPU
 * @brief  install an exception handler
 * @param[in] excno	exception number
 * @param[in] handler the handler of exception to install
 */
/**
 * warning: only work in simulation and it won't work on any real platform with caches, including
 * nsim with cache enabled.
 */
static int32_t exc_handler_install(const uint32_t excno, EXC_HANDLER_T handler)
{
    if (excno < NUM_EXC_ALL && handler != NULL) {
        exc_int_handler_table[excno] = handler;
        return 0;
    }

    return -1;
}

/**
 * @brief disable the specific interrupt
 *
 * @param[in] intno interrupt number
 */
int32_t int_disable(const uint32_t intno)
{
    if (intno >= NUM_EXC_CPU && intno < NUM_EXC_ALL) {
        arc_int_disable(intno);
        return 0;
    }

    return -1;
}

/**
 * @brief  enable the specific int
 *
 * @param[in] intno interrupt number
 */
int32_t int_enable(const uint32_t intno)
{
    if (intno >= NUM_EXC_CPU && intno < NUM_EXC_ALL) {
        arc_int_enable(intno);
        return 0;
    }

    return -1;
}

/**
 * @brief set interrupt priority
 *
 * @param[in] intno interrupt number
 * @param[in] intpri interrupt priority
 * @return  <0 error, 0 ok
 */
int32_t int_pri_set(const uint32_t intno, int32_t intpri)
{
    uint32_t status;

    if (intno >= NUM_EXC_CPU && intno < NUM_EXC_ALL) {
        status = cpu_lock_save();
        intpri = intpri - INT_PRI_MIN;
        arc_int_pri_set(intno, (uint32_t)intpri);
        cpu_unlock_restore(status);
        return 0;
    }
    return -1;
}

/**
 * @brief  lock cpu and return status
 *
 * @returns cpu status
 */
uint32_t cpu_lock_save(void)
{
    return arc_lock_save();
}

/**
 * @brief  unlock cpu with the specific status
 *
 * @param[in] status  cpu status saved by cpu_lock_save
 */
void cpu_unlock_restore(const uint32_t status)
{
    arc_unlock_restore(status);
}

/**
 * @ingroup ARC_HAL_EXCEPTION_INTERRUPT
 * @brief  install an interrupt handler
 * @param[in] intno	interrupt number
 * @param[in] handler interrupt handler to install
 */
int32_t int_handler_install(const uint32_t intno, INT_HANDLER_T handler)
{
    /*!< @todo parameter check ? */
    if (intno >= NUM_EXC_CPU) {
        return exc_handler_install(intno, handler);
    }

    return -1;
}

/** @} end of group ARC_HAL_EXCEPTION_CPU */