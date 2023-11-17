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
 * \ingroup ARC_HAL_MISC_TIMER
 * \brief header file of ARC internal timer
 */

/**
 * \addtogroup ARC_HAL_MISC_TIMER
 * @{
 */

#ifndef _ARC_HAL_TIMER_H_
#define _ARC_HAL_TIMER_H_
#include "arc/arc.h"
#include "arc/arc_builtin.h"

/**
 * \name arc internal timers names
 * @{
 */
#define TIMER_0         0   /*!< macro name for arc internal timer 0 */
#define TIMER_1         1   /*!< macro name for arc internal timer 1 */
#define RTC_TIMER       2   /*!< macro name for arc internal RTC */
#define SECURE_TIMER_0  3   /*!< macro name for arc internal secure timer 0 */
#define SECURE_TIMER_1  4   /*!< macro name for arc internal secure timer 1 */

#define INTNO_TIMER0 ARC_FEATURE_TIMER0_VECTOR

/** @} */

/**
 * \name bit definition of timer CTRL reg
 * @{
 */
#define TIMER_CTRL_IE           (1 << 0)        /*!< Interrupt when count reaches limit */
#define TIMER_CTRL_NH           (1 << 1)        /*!< Count only when CPU NOT halted */
#define TIMER_CTRL_W            (1 << 2)        /*!< watchdog enable */
#define TIMER_CTRL_IP           (1 << 3)        /*!< interrupt pending */

/** @} */
extern int32_t arc_timer_present(const uint32_t no);
extern int32_t arc_timer_start(const uint32_t no, const uint32_t mode, const uint32_t val);
extern int32_t arc_timer_stop(const uint32_t no);
extern int32_t arc_timer_int_clear(const uint32_t no);
extern void arc_timer_init(void);

#endif  /* _ARC_HAL_TIMER_H_ */
/** @} */