/*
 * FreeRTOS+TCP <DEVELOPMENT BRANCH>
 * Copyright (C) 2022 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */


/*****************************************************************************
*
* See the following URL for configuration information.
* http://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_IP_Configuration.html
*
*****************************************************************************/

#ifndef FREERTOS_IP_CONFIG_H
#define FREERTOS_IP_CONFIG_H

#include <stdio.h>

/*===========================================================================*/
/*                             TCP/IP TASK CONFIG                            */
/*===========================================================================*/
#define ipconfigIP_TASK_STACK_SIZE_WORDS    (128U)
#define ipconfigUSE_NETWORK_EVENT_HOOK      (ipconfigENABLE)

/*===========================================================================*/
/*                        DEBUG/TRACE/LOGGING CONFIG                         */
/*===========================================================================*/
#define ipconfigHAS_DEBUG_PRINTF    (ipconfigENABLE)
#define FreeRTOS_debug_printf(X)    printf X

/*===========================================================================*/
/*                               DRIVER CONFIG                               */
/*===========================================================================*/
#define ipconfigBYTE_ORDER                             (pdFREERTOS_LITTLE_ENDIAN)
#define ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES    (ipconfigENABLE)
#define ipconfigETHERNET_MINIMUM_PACKET_BYTES          (42U)
#define ipconfigNETWORK_MTU                            (1500U)
#define ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS         (25U)
#define ipconfigZERO_COPY_RX_DRIVER                    (ipconfigENABLE)
#define ipconfigZERO_COPY_TX_DRIVER                    (ipconfigENABLE)

/*===========================================================================*/
/*                                TCP CONFIG                                 */
/*===========================================================================*/
#define ipconfigUSE_TCP    (ipconfigDISABLE)

/*===========================================================================*/
/*                                ARP CONFIG                                 */
/*===========================================================================*/
#define ipconfigARP_CACHE_ENTRIES    (3U)

/*===========================================================================*/
/*                               DHCP CONFIG                                 */
/*===========================================================================*/
#define ipconfigUSE_DHCP    (ipconfigDISABLE)
#define ipconfigUSE_DNS     (ipconfigDISABLE)

/*===========================================================================*/
/*                                 IP CONFIG                                 */
/*===========================================================================*/
#define ipconfigUSE_IPv6    (ipconfigDISABLE)

#endif