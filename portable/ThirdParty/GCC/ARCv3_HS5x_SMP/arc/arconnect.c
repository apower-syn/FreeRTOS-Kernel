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

/*
 * Implementation of functions defined in arconnect.h
 */

#include "arc/arc.h"
#include "arc/arc_builtin.h"
#include "arc/arconnect.h"

void arconnect_send_command(uint8_t command, uint16_t parameter)
{
    uint32_t cmd = (parameter << 8) | command;
    arc_aux_write(ARCONNECT_CMD, cmd);
}

uint32_t arconnect_readback(void)
{
    return arc_aux_read(ARCONNECT_READBACK);
}

void arconnect_send_cmd_check_core_id(void)
{
    uint8_t  command   = ARCONNECT_CMD_CHECK_CORE_ID;
    uint16_t parameter = 0;
    arconnect_send_command(command, parameter);
}

void arconnect_send_cmd_intrpt_generate_irq(uint8_t xCoreId)
{
    uint8_t  command   = ARCONNECT_CMD_INTRPT_GENERATE_IRQ;
    uint16_t parameter = xCoreId;
    arconnect_send_command(command, parameter);
}

void arconnect_send_cmd_intrpt_generate_ack(uint8_t xCoreId)
{
    uint8_t  command   = ARCONNECT_CMD_INTRPT_GENERATE_ACK;
    uint16_t parameter = xCoreId;
    arconnect_send_command(command, parameter);
}

uint32_t arconnect_send_cmd_intrpt_check_source(void)
{
    uint8_t  command   = ARCONNECT_CMD_INTRPT_CHECK_SOURCE;
    uint16_t parameter = 0;
    arconnect_send_command(command, parameter);

    uint32_t intrpt_source   = 0;
    uint32_t intrpt_readback = arconnect_readback();

    /* Assume only one core is generating the interrupt */
    for (int i = 0; i < 32; i++) {
      if ( intrpt_readback & (1 << i) ) {
        intrpt_source = i;
        return intrpt_source;
      }
    }

    return intrpt_source;
}

uint8_t arconnect_send_cmd_intrpt_read_status(uint8_t xCoreId)
{
    uint8_t  command   = ARCONNECT_CMD_INTRPT_READ_STATUS;
    uint16_t parameter = xCoreId;
    arconnect_send_command(command, parameter);

    uint32_t intrpt_read_status = arconnect_readback();
    return (intrpt_read_status & 0x01);
}

uint32_t arconnect_get_core_id(void)
{
    arconnect_send_cmd_check_core_id();

    uint32_t readback  = arconnect_readback();
    uint32_t arcCoreId = (readback & 0x1F);

    return arcCoreId;
}

arconnect_bcr_t arconnect_get_arconnect_bcr(void)
{
    arconnect_bcr_u arconnect_bcr;
    arconnect_bcr.bits = arc_aux_read(AUX_CONNECT_SYSTEM_BUILD);    
    return arconnect_bcr.fields;
}

uint32_t arconnect_get_ici_version(void)
{
    uint32_t ici_bcr = arc_aux_read(AUX_CONNECT_ICI_BUILD);
    uint32_t ici_version = ici_bcr & 0x0FF;
    return ici_version;
}
