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
 * \ingroup ARC_HAL_CORE_COMM
 * \brief  header file including common core definitions
 */

/**
 * \addtogroup ARC_HAL_CORE_COMM
 * @{
 */

#ifndef _ARC_HAL_CORE_H_
#define _ARC_HAL_CORE_H_

#include "arc_feature_config.h"

#define AUX_BCR_TIMERS                  (0x75)                      /*!< build configuration for processor timers */
#define AUX_STATUS32                    (0xa)
#define AUX_STATUS_BIT_AE               (5)                         /*!< processor is in an exception */
#define AUX_STATUS_BIT_IE               (31)                        /*!< interrupt enable */
#define AUX_STATUS_BIT_HALT             (0)                         /*!< halt bit */
#define AUX_STATUS_BIT_SC               (14)                        /*!< stack check bit */
#define AUX_STATUS_BIT_US               (20)                        /*!< user sleep mode enable bit */
#define AUX_STATUS_MASK_IE              (1 << AUX_STATUS_BIT_IE)    /*!< mask of AUX_STATUS_BIT_IE */
#define AUX_STATUS_MASK_HALT            (1 << AUX_STATUS_BIT_HALT)  /*!< mask of AUX_STATUS_BIT_HALT */
#define AUX_STATUS_MASK_US              (1 << AUX_STATUS_BIT_US)    /*!< mask of AUX_STATUS_BIT_US */
#define STATUS32_RESET_VALUE            (AUX_STATUS_MASK_US)
#define AUX_LP_START                    (0x2)                       /*!< loop start address (32-bit) */
#define AUX_LP_END                      (0x3)                       /*!< loop end address (32-bit) */
#define AUX_KSTACK_TOP                  (0x264)
#define AUX_KSTACK_BASE                 (0x265)
#define AUX_TIMER0_CNT                  (0x21)                      /*!< timer 0 count value */
#define AUX_TIMER0_CTRL                 (0x22)                      /*!< timer 0 control value */
#define AUX_TIMER0_LIMIT                (0x23)                      /*!< timer 0 limit value */
#define AUX_TIMER1_CNT                  (0x100)                     /*!< timer 1 count value */
#define AUX_TIMER1_CTRL                 (0x101)                     /*!< timer 1 control value */
#define AUX_TIMER1_LIMIT                (0x102)                     /*!< timer 1 limit value */
#define AUX_RTC_CTRL                    (0x103)                     /*!< RTC control value */
#define AUX_RTC_LOW                     (0x104)                     /*!< RTC count low value */
#define AUX_RTC_HIGH                    (0x105)                     /*!< RTC count high value */
#define AUX_SECURE_TIMER0_CNT           (0x106)                     /*!< secure timer 0 count value */
#define AUX_SECURE_TIMER0_CTRL          (0x107)                     /*!< secure timer 0 control value */
#define AUX_SECURE_TIMER0_LIMIT         (0x108)                     /*!< secure timer 0 limit value */
#define AUX_SECURE_TIMER1_CNT           (0x109)                     /*!< secure timer 1 count value */
#define AUX_SECURE_TIMER1_CTRL          (0x10a)                     /*!< secure timer 1 control value */
#define AUX_SECURE_TIMER1_LIMIT         (0x10b)                     /*!< secure timer 1 limit value */
#define AUX_ERRET                       (0x400)                     /*!< exception return address */
#define AUX_ERBTA                       (0x401)                     /*!< BTA saved on exception entry */
#define AUX_ERSTATUS                    (0x402)                     /*!< STATUS32 saved on exception */
#define AUX_ECR                         (0x403)                     /*!< exception cause register */
#define AUX_IRQ_CTRL                    (0xe)                       /*!< interrupt context saving control register */
#define AUX_INT_VECT_BASE               (0x25)                      /*!< interrupt vector base register */
#define AUX_IRQ_ACT                     (0x43)                      /*!< active interrupts register */
#define AUX_IRQ_CAUSE                   (0x40a)                     /*!< interrupt cause register */
#define AUX_IRQ_SELECT                  (0x40b)                     /*!< interrupt select register */
#define AUX_IRQ_PRIORITY                (0x206)                     /*!< interrupt priority register */
#define AUX_IRQ_ENABLE                  (0x40c)                     /*!< interrupt enable register */
#define AUX_IRQ_TRIGGER                 (0x40d)                     /*!< interrupt trigger: level or pulse */
#define AUX_IRQ_HINT                    (0x201)                     /*!< software interrupt trigger */

#ifndef __ASSEMBLY__

#include <stdint.h>     /* C99 standard lib */
#include <limits.h>     /* C99 standard lib */
#include <stddef.h>     /* C99 standard lib */
#include <stdbool.h>    /* C99 standard lib */

#define Inline  static __inline__               /* inline function */

#define Asm     __asm__ volatile                /* inline asm (no optimization) */

#endif  /* __ASSEMBLY__ */

#endif  /* _ARC_HAL_CORE_H_ */

// /**  @} */