
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
 */

#ifndef _ARCONNECT_H_
#define _ARCONNECT_H_

#include "arc/arc.h"
#include "arc/arc_builtin.h"

typedef struct {
    /* note: little endian */
    uint32_t version : 8;   /** The ARConnect version*/
    uint32_t ext     : 1;   /** Indicates whether ARConnect supports memory-mapped transactions. */
    uint32_t ici     : 1;   /** Presence of Inter-Core Interrupt Unit */
    uint32_t ics     : 1;   /** Presence of Inter-Core Semaphore Unit */
    uint32_t icm     : 1;   /** Presence of Inter-Core Message Unit */
    uint32_t pmu     : 1;   /** Presence of Power Management Unit */
    uint32_t icd     : 1;   /** Presence of Inter-Core Debug Unit */
    uint32_t gfrc    : 1;   /** Presence of Global Free-Running Counter Unit */
    uint32_t res1    : 1;   /** Reserved */
    uint32_t corenum : 6;   /** Number of cores (ARC and non-ARC) connected to ARConnect */
    uint32_t res2    : 1;   /** Reserved */
    uint32_t idu     : 1;   /** Presence of Interrupt Distribution Unit */
    uint32_t res3    : 1;   /** Reserved */
    uint32_t pdm     : 1;   /** Presence of Power Domain Management Unit */
    uint32_t ivc     : 1;   /** Presence of Interrupt Vector base Control Unit */
    uint32_t res4    : 5;   /** Reserved */
} arconnect_bcr_t;

typedef union {
  arconnect_bcr_t fields;
  uint32_t        bits;
} arconnect_bcr_u;

void arconnect_send_command(uint8_t command, uint16_t parameter);
uint32_t arconnect_readback(void);

void arconnect_send_cmd_check_core_id(void);
void arconnect_send_cmd_intrpt_generate_irq(uint8_t xCoreId);
void arconnect_send_cmd_intrpt_generate_ack(uint8_t xCoreId);
uint32_t arconnect_send_cmd_intrpt_check_source(void);
uint8_t arconnect_send_cmd_intrpt_read_status(uint8_t xCoreId);

uint32_t arconnect_get_core_id(void);
arconnect_bcr_t arconnect_get_arconnect_bcr(void);
uint32_t arconnect_get_ici_version(void);

#endif  /* _ARCONNECT_H_ */
