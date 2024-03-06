/*
* FreeRTOS Kernel <DEVELOPMENT BRANCH>
* Copyright (C) 2024 Synopsys, Inc. or its affiliates.  All Rights Reserved.
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
 * @ingroup	ARC_HAL_MISC_TIMER
 * @brief  implementation of internal timer related functions
 * @todo RTC support should be improved if RTC is enabled
 */
#include "arc/arc_timer.h"
#include "arc/arc_exception.h"

/**
 * @brief  check whether the specific timer present
 * @param[in] no timer number
 * @return 1 present, 0 not present
 */
int32_t arc_timer_present(const uint32_t no)
{
    uint32_t bcr = arc_aux_read(AUX_BCR_TIMERS);

    switch (no) {
      case TIMER_0:
          bcr = (bcr >> 8) & 1;
          break;

      case TIMER_1:
          bcr = (bcr >> 9) & 1;
          break;

      case RTC_TIMER:
          bcr = (bcr >> 10) & 1;
          break;

      case SECURE_TIMER_0:
          bcr = (bcr >> 11) & 1;
          break;

      case SECURE_TIMER_1:
          bcr = (bcr >> 12) & 1;
          break;

      default:
          bcr = 0;
          break;
    }

    return (int)bcr;
}

static void arc_timer0_start(const uint32_t mode, const uint32_t val) {
    arc_aux_write(AUX_TIMER0_CTRL, 0);
    arc_aux_write(AUX_TIMER0_LIMIT, val);
    arc_aux_write(AUX_TIMER0_CTRL, mode);
    arc_aux_write(AUX_TIMER0_CNT, 0);
}

static void arc_timer1_start(const uint32_t mode, const uint32_t val) {
    arc_aux_write(AUX_TIMER1_CTRL, 0);
    arc_aux_write(AUX_TIMER1_LIMIT, val);
    arc_aux_write(AUX_TIMER1_CTRL, mode);
    arc_aux_write(AUX_TIMER1_CNT, 0);
}

static void arc_rtc_start(const uint32_t mode) {
    /* Enable with bit 0, Clear count registers (AUX_RTC_LOW, AUX_RTC_HIGH) with bit 1, Accept A1 & A0 from mode bits 31 & 30 */
    arc_aux_write(AUX_RTC_CTRL, (mode & 0xC0000003));
}

static void arc_secure_timer0_start(const uint32_t mode, const uint32_t val) {
    arc_aux_write(AUX_SECURE_TIMER0_CTRL, 0);
    arc_aux_write(AUX_SECURE_TIMER0_LIMIT, val);
    arc_aux_write(AUX_SECURE_TIMER0_CTRL, mode);
    arc_aux_write(AUX_SECURE_TIMER0_CNT, 0);
}

static void arc_secure_timer1_start(const uint32_t mode, const uint32_t val) {
    arc_aux_write(AUX_SECURE_TIMER1_CTRL, 0);
    arc_aux_write(AUX_SECURE_TIMER1_LIMIT, val);
    arc_aux_write(AUX_SECURE_TIMER1_CTRL, mode);
    arc_aux_write(AUX_SECURE_TIMER1_CNT, 0);
}

/**
 * @brief  start the specific timer
 * @param[in] no	timer number
 * @param[in] mode	timer mode
 * @param[in] val	timer limit value (not for RTC)
 * @return 0 success, -1 failure
 */
int32_t arc_timer_start(const uint32_t no, const uint32_t mode, const uint32_t val)
{
    switch (no) {
        case TIMER_0:
            arc_timer0_start(mode, val);
            return 0;

        case TIMER_1:
            arc_timer1_start(mode, val);
            return 0;

        case RTC_TIMER:
            arc_rtc_start(mode);
            return 0;

        case SECURE_TIMER_0:
            arc_secure_timer0_start(mode, val);
            return 0;

        case SECURE_TIMER_1:
            arc_secure_timer1_start(mode, val);
            return 0;
    }

    return -1;
}

static void arc_timer0_stop() {
    arc_aux_write(AUX_TIMER0_CTRL, 0);
    arc_aux_write(AUX_TIMER0_LIMIT, 0);
    arc_aux_write(AUX_TIMER0_CNT, 0);
}

static void arc_timer1_stop() {
    arc_aux_write(AUX_TIMER1_CTRL, 0);
    arc_aux_write(AUX_TIMER1_LIMIT, 0);
    arc_aux_write(AUX_TIMER1_CNT, 0);
}

static void arc_rtc_stop() {
    /* Zero the Enable bit 0, Clear count registers (AUX_RTC_LOW, AUX_RTC_HIGH) with bit 1, Reserved bits ignored on write, Preserve A1, A0 in bits 31 & 30 */
    uint32_t mode = arc_aux_read(AUX_RTC_CTRL) & ~0x1;
    arc_aux_write(AUX_RTC_CTRL, mode);
}

static void arc_secure_timer0_stop() {
    arc_aux_write(AUX_SECURE_TIMER0_CTRL, 0);
    arc_aux_write(AUX_SECURE_TIMER0_LIMIT, 0);
    arc_aux_write(AUX_SECURE_TIMER0_CNT, 0);
}

static void arc_secure_timer1_stop() {
    arc_aux_write(AUX_SECURE_TIMER1_CTRL, 0);
    arc_aux_write(AUX_SECURE_TIMER1_LIMIT, 0);
    arc_aux_write(AUX_SECURE_TIMER1_CNT, 0);
}

/**
 * @brief  stop and clear the specific timer
 *
 * @param[in] no timer number
 * @return 0 success, -1 failure
 */
int32_t arc_timer_stop(const uint32_t no)
{
    switch (no) {
        case TIMER_0:
            arc_timer0_stop();
            return 0;

        case TIMER_1:
            arc_timer1_stop();
            return 0;

        case RTC_TIMER:
            arc_rtc_stop();
            return 0;

        case SECURE_TIMER_0:
            arc_secure_timer0_stop();
            return 0;

        case SECURE_TIMER_1:
            arc_secure_timer1_stop();
            return 0;
    }

    return -1;
}

static void arc_timer0_int_clear() {
    uint32_t val;

    val = arc_aux_read(AUX_TIMER0_CTRL);
    val &= ~TIMER_CTRL_IP;
    arc_aux_write(AUX_TIMER0_CTRL, val);
}

static void arc_timer1_int_clear() {
    uint32_t val;

    val = arc_aux_read(AUX_TIMER1_CTRL);
    val &= ~TIMER_CTRL_IP;
    arc_aux_write(AUX_TIMER1_CTRL, val);
}

static void arc_secure_timer0_int_clear() {
    uint32_t val;

    val = arc_aux_read(AUX_SECURE_TIMER0_CTRL);
    val &= ~TIMER_CTRL_IP;
    arc_aux_write(AUX_SECURE_TIMER0_CTRL, val);
}

static void arc_secure_timer1_int_clear() {
    uint32_t val;

    val = arc_aux_read(AUX_SECURE_TIMER1_CTRL);
    val &= ~TIMER_CTRL_IP;
    arc_aux_write(AUX_SECURE_TIMER1_CTRL, val);
}

/**
 * @brief  clear the interrupt pending bit of timer
 *
 * @param[in] no timer number
 * @return 0 success, -1 failure
 */
int32_t arc_timer_int_clear(const uint32_t no)
{
    switch (no) {
        case TIMER_0:
            arc_timer0_int_clear();
            return 0;

        case TIMER_1:
            arc_timer1_int_clear();
            return 0;

        case SECURE_TIMER_0:
            arc_secure_timer0_int_clear();
            return 0;

        case SECURE_TIMER_1:
            arc_secure_timer1_int_clear();
            return 0;
    }

    return -1;
}

/**
 * @brief  init internal timer
 */
void arc_timer_init(void)
{
    if (arc_timer_present(TIMER_0)) {
        arc_timer_stop(TIMER_0);
    }
}