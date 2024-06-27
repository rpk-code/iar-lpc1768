/*
 * FreeRTOS+TCP <DEVELOPMENT BRANCH>
 * Copyright (C) 2022 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/* Standard includes. */
#include <stdint.h>
#include <assert.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS+TCP includes. */
#include "NetworkBufferManagement.h"

/* Driver includes. */
#include "LPC17xx.h"
#include "EMAC_LPC17xx.h"
#include "EPHY_KS8721.h"

#define ETH_INSTANCE           (0U)

#define RND_NUM_SEED           (64203)

#define ENET_IRQ_PRIO          (4U)

#define ETH_BUF_SIZE           (1536U)

#define ISR_HDLR_TASK_NAME     "emac_isrh"
#define ISR_HDLR_STACK_SIZE    (128)
#define ISR_HDLR_TASK_PRIO     (configMAX_PRIORITIES - 1)

#define MAX_TX_ATTEMPTS        (5U)
#define TX_BUFFER_FREE_WAIT    (pdMS_TO_MIN_TICKS(2))

/*-----------------------------------------------------------*/
static const uint8_t ucMACAddress[6]       = {0xBC, 0x15, 0xFA, 0xF5, 0x4D, 0xAC};
static const uint8_t ucIPAddress[4]        = {172, 20, 10, 1};
static const uint8_t ucNetMask[4]          = {255, 255, 255, 0 };
static const uint8_t ucGatewayAddress[4]   = {172, 20, 10, 8};
static const uint8_t ucDNSServerAddress[4] = {0, 0, 0, 0};

static NetworkInterface_t xInterfaces[1];
static NetworkEndPoint_t xEndPoints[1];

static TaskHandle_t emac_isr_hdlr_task;
static NetworkInterface_t * pxMyInterface;

static uint32_t rnd_num_cnt = 0;

static volatile uint32_t isr_event = 0;

extern ARM_DRIVER_ETH_MAC Driver_ETH_MAC0;
extern ARM_DRIVER_ETH_PHY Driver_ETH_PHY0;

/*-----------------------------------------------------------*/
static NetworkInterface_t * pxlpc17xx_Eth_FillInterfaceDescriptor(BaseType_t xEMACIndex,
    NetworkInterface_t *pxInterface);
static BaseType_t xlpc17xx_Eth_NetworkInterfaceInitialize(NetworkInterface_t *pxInterface);
static BaseType_t xlpc17xx_Eth_GetPhyLinkStatus(NetworkInterface_t *pxInterface);
static BaseType_t xlpc17xx_Eth_NetworkInterfaceOutput(NetworkInterface_t * pxInterface,
    NetworkBufferDescriptor_t * const pxDescriptor, BaseType_t xReleaseAfterSend);
static void emac_isr_cb(uint32_t event);
static void emac_isr_handler(void *arg);

/*-----------------------------------------------------------*/
static NetworkInterface_t * pxlpc17xx_Eth_FillInterfaceDescriptor(BaseType_t xEMACIndex,
    NetworkInterface_t *pxInterface)
{
    char name[] = "ethx";

    BaseType_t ret = xTaskCreate(emac_isr_handler, ISR_HDLR_TASK_NAME,
        ISR_HDLR_STACK_SIZE, NULL, ISR_HDLR_TASK_PRIO, &emac_isr_hdlr_task);
    assert(ret);

    name[3] = (xEMACIndex - 0) + 48;
    memset(pxInterface, 0, sizeof(*pxInterface));
    pxInterface->pcName = name;
    pxInterface->pvArgument = (void*)xEMACIndex;
    pxInterface->pfInitialise = xlpc17xx_Eth_NetworkInterfaceInitialize;
    pxInterface->pfOutput = xlpc17xx_Eth_NetworkInterfaceOutput;
    pxInterface->pfGetPhyLinkStatus = xlpc17xx_Eth_GetPhyLinkStatus;

    FreeRTOS_AddNetworkInterface(pxInterface);
    pxMyInterface = pxInterface;

    return pxInterface;
}

static BaseType_t xlpc17xx_Eth_NetworkInterfaceInitialize(NetworkInterface_t *pxInterface)
{
    int32_t ret;

    /* Initialize and power on the EMAC */
    ret = Driver_ETH_MAC0.Initialize(emac_isr_cb);
    assert(ret == ARM_DRIVER_OK);
    NVIC_SetPriority(ENET_IRQn, ENET_IRQ_PRIO);
    ret = Driver_ETH_MAC0.PowerControl(ARM_POWER_FULL);
    assert(ret == ARM_DRIVER_OK);

    /* Set MAC address */
    ARM_ETH_MAC_ADDR mac_addr;
    memcpy(mac_addr.b, pxInterface->pxEndPoint->xMACAddress.ucBytes, sizeof(mac_addr.b));
    ret = Driver_ETH_MAC0.SetMacAddress(&mac_addr);
    assert(ret == ARM_DRIVER_OK);

    /* Initialize and power on PHY */
    ret = Driver_ETH_PHY0.Initialize(Driver_ETH_MAC0.PHY_Read, Driver_ETH_MAC0.PHY_Write);
    assert(ret == ARM_DRIVER_OK);
    ret = Driver_ETH_PHY0.PowerControl(ARM_POWER_FULL);
    assert(ret == ARM_DRIVER_OK);

    /* Set PHY interface */
    ARM_ETH_MAC_CAPABILITIES emac_caps = Driver_ETH_MAC0.GetCapabilities();
    ret = Driver_ETH_PHY0.SetInterface(emac_caps.media_interface);
    assert(ret == ARM_DRIVER_OK);

    /* Perform auto-negotiation and confirm link UP */
    ret = Driver_ETH_PHY0.SetMode(ARM_ETH_PHY_AUTO_NEGOTIATE);
    assert(ret == ARM_DRIVER_OK);
    ARM_ETH_LINK_STATE link_state = Driver_ETH_PHY0.GetLinkState();
    assert(link_state == ARM_ETH_LINK_UP);
    ARM_ETH_LINK_INFO link_info = Driver_ETH_PHY0.GetLinkInfo();

    /* Set MAC parameters */
    uint32_t arg = 0;
    arg |= (link_info.speed == ARM_ETH_SPEED_100M) ? ARM_ETH_MAC_SPEED_100M : ARM_ETH_MAC_SPEED_10M;
    arg |= (link_info.duplex == ARM_ETH_DUPLEX_FULL) ? ARM_ETH_MAC_DUPLEX_FULL : ARM_ETH_MAC_DUPLEX_HALF;
    arg |= ARM_ETH_MAC_ADDRESS_BROADCAST;
    ret = Driver_ETH_MAC0.Control(ARM_ETH_MAC_CONFIGURE, arg);
    assert(ret == ARM_DRIVER_OK);

    /* Enable communication */
    ret = Driver_ETH_MAC0.Control(ARM_ETH_MAC_CONTROL_TX, 1);
    assert(ret == ARM_DRIVER_OK);
    ret = Driver_ETH_MAC0.Control(ARM_ETH_MAC_CONTROL_RX, 1);
    assert(ret == ARM_DRIVER_OK);

    return pdTRUE;
}

static BaseType_t xlpc17xx_Eth_GetPhyLinkStatus(NetworkInterface_t *pxInterface)
{
    ARM_ETH_LINK_STATE link_state = Driver_ETH_PHY0.GetLinkState();

    return (link_state == ARM_ETH_LINK_UP) ? pdTRUE : pdFALSE;
}

static BaseType_t xlpc17xx_Eth_NetworkInterfaceOutput(NetworkInterface_t * pxInterface,
    NetworkBufferDescriptor_t * const pxDescriptor, BaseType_t xReleaseAfterSend)
{
    if ((pxDescriptor == NULL) || (pxDescriptor->pucEthernetBuffer == NULL) || (pxDescriptor->xDataLength == 0)) {
        return pdFALSE;
    }

    for (uint8_t i = 0; i < MAX_TX_ATTEMPTS; i++) {
        int32_t ret = Driver_ETH_MAC0.SendFrame(pxDescriptor->pucEthernetBuffer, pxDescriptor->xDataLength, 0);
        if (ret == ARM_DRIVER_OK) {
#if ipconfigZERO_COPY_TX_DRIVER == ipconfigDISABLE
            if (xReleaseAfterSend) {
                vReleaseNetworkBufferAndDescriptor(pxDescriptor);
            }
#endif

            return pdTRUE;
        } else {
            vTaskDelay(TX_BUFFER_FREE_WAIT);
        }
    }

    return pdFALSE;
}

static void emac_isr_cb(uint32_t event)
{
#if ipconfigZERO_COPY_TX_DRIVER == ipconfigDISABLE
    if (event & ARM_ETH_MAC_EVENT_RX_FRAME)
#endif
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        isr_event |= event;
        vTaskNotifyGiveFromISR(emac_isr_hdlr_task, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

static void emac_do_rx(void)
{
    uint32_t recv_len = Driver_ETH_MAC0.GetRxFrameSize();
    if ((recv_len != 0) && (recv_len != UINT32_MAX)) {
#if ipconfigZERO_COPY_RX_DRIVER == ipconfigDISABLE
        NetworkBufferDescriptor_t *desc = pxGetNetworkBufferWithDescriptor(recv_len, 0);
#else
        NetworkBufferDescriptor_t *desc = pxGetNetworkBufferWithDescriptor(ETH_BUF_SIZE, 0);
#endif

        if (desc) {
            uint32_t ret = Driver_ETH_MAC0.ReadFrame(desc->pucEthernetBuffer, recv_len);

            if (ret) {
#if ipconfigZERO_COPY_RX_DRIVER == ipconfigDISABLE
                NetworkBufferDescriptor_t *curr_desc = desc;
#else
                NetworkBufferDescriptor_t *curr_desc = pxPacketBuffer_to_NetworkBuffer((void*)ret);
#endif
                curr_desc->xDataLength = recv_len;
                curr_desc->pxInterface = pxMyInterface;
                curr_desc->pxEndPoint = FreeRTOS_MatchingEndpoint(curr_desc->pxInterface, curr_desc->pucEthernetBuffer);
                if (curr_desc->pxEndPoint) {
                    IPStackEvent_t event;

                    event.eEventType = eNetworkRxEvent;
                    event.pvData = (void*)curr_desc;
                    if (xSendEventStructToIPTask(&event, 0) == pdFALSE) {
                        vReleaseNetworkBufferAndDescriptor(curr_desc);
                    }
                } else {
                    vReleaseNetworkBufferAndDescriptor(curr_desc);
                }
            } else {
                vReleaseNetworkBufferAndDescriptor(desc);
            }
        }
    }
}

#if ipconfigZERO_COPY_TX_DRIVER == ipconfigENABLE
static void emac_do_tx(void)
{
    uint8_t *pkt = (uint8_t*)Driver_ETH_MAC0.GetTxFrame();

    if (pkt) {
        NetworkBufferDescriptor_t *desc = pxPacketBuffer_to_NetworkBuffer((void*)pkt);
        assert(desc);
        vReleaseNetworkBufferAndDescriptor(desc);
    }
}
#endif

static void emac_isr_handler(void *arg)
{
    while(1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (isr_event & ARM_ETH_MAC_EVENT_RX_FRAME) {
            emac_do_rx();
            isr_event &= ~ARM_ETH_MAC_EVENT_RX_FRAME;
        }
#if ipconfigZERO_COPY_TX_DRIVER == ipconfigENABLE
        if (isr_event & ARM_ETH_MAC_EVENT_TX_FRAME) {
            emac_do_tx();
            isr_event &= ~ARM_ETH_MAC_EVENT_TX_FRAME;
        }
#endif
    }
}

void eth_init(void)
{
    pxlpc17xx_Eth_FillInterfaceDescriptor(ETH_INSTANCE, &xInterfaces[0]);
    FreeRTOS_FillEndPoint(&xInterfaces[0], &xEndPoints[0], ucIPAddress,
        ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress);
    FreeRTOS_IPInit_Multi();
}

void vApplicationIPNetworkEventHook_Multi( eIPCallbackEvent_t eNetworkEvent,
    struct xNetworkEndPoint * pxEndPoint )
{
    switch(eNetworkEvent) {
        case eNetworkUp:
            printf("ETH UP\n");
            break;
        case eNetworkDown:
            printf("ETH DOWN\n");
            break;
        default:
            break;
    }
}

BaseType_t xApplicationGetRandomNumber( uint32_t * pulNumber )
{
    *pulNumber = RND_NUM_SEED + rnd_num_cnt;
    rnd_num_cnt++;

    return pdTRUE;
}