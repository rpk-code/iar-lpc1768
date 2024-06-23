/*
 * Copyright (c) 2024 Prashanth Kannan. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#include "EPHY_KS8721.h"

extern ARM_DRIVER_ETH_PHY Driver_ETH_PHY0;

#define ARM_ETH_PHY_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0) /* driver version */

static PHY_CTRL PHY = {NULL, NULL, 0, 0, 0};

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_ETH_PHY_API_VERSION,
    ARM_ETH_PHY_DRV_VERSION
};

//
// Functions
//

static ARM_DRIVER_VERSION ARM_ETH_PHY_GetVersion(void)
{
    return DriverVersion;
}

static int32_t ARM_ETH_PHY_Initialize(ARM_ETH_PHY_Read_t fn_read, ARM_ETH_PHY_Write_t fn_write)
{
    if ((fn_read == NULL) || (fn_write == NULL)) {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if (!(PHY.flags & PHY_INIT)) {
        PHY.reg_rd = fn_read;
        PHY.reg_wr = fn_write;
        PHY.bmcr = 0;
        PHY.flags = PHY_INIT;
    }

    return ARM_DRIVER_OK;
}

static int32_t ARM_ETH_PHY_Uninitialize(void)
{
    PHY.reg_rd = NULL;
    PHY.reg_wr = NULL;
    PHY.bmcr = 0;
    PHY.flags = 0;

    return ARM_DRIVER_OK;
}

static int32_t ARM_ETH_PHY_PowerControl(ARM_POWER_STATE state)
{
    switch (state) {
        case ARM_POWER_FULL:
        {
            uint8_t tout = 0;
            uint16_t reg_val = 0;

            if (!(PHY.flags & PHY_INIT)) {
                return ARM_DRIVER_ERROR;
            }

            if (PHY.flags & PHY_POWER) {
                return ARM_DRIVER_OK;
            }

            PHY.reg_rd(PHY_ADDR, PHYIDR1, &reg_val);
            if (reg_val != PHY_ID1) {
                return ARM_DRIVER_ERROR_UNSUPPORTED;
            }

            PHY.reg_rd(PHY_ADDR, PHYIDR2, &reg_val);
            if (reg_val != PHY_ID2) {
                return ARM_DRIVER_ERROR_UNSUPPORTED;
            }

            PHY.bmcr = BMCR_RESET;
            if (PHY.reg_wr(PHY_ADDR, BMCR, PHY.bmcr) != ARM_DRIVER_OK) {
                return ARM_DRIVER_ERROR;
            }

            reg_val = 0;
            do {
            #if defined(RTE_FREERTOS)
                vTaskDelay(pdMS_TO_TICKS(5));
            #endif
                tout += 5;

                PHY.reg_rd(PHY_ADDR, BMCR, &reg_val);
                if (!(reg_val & (BMCR_RESET | BMCR_POWER_DOWN))) {
                    break;
                }
            } while(tout < PHY_RESET_TOUT);

            if (tout >= PHY_RESET_TOUT) {
                PHY.bmcr = 0;
                return ARM_DRIVER_ERROR;
            }

            PHY.bmcr = reg_val;
            PHY.flags |= PHY_POWER;

            return ARM_DRIVER_OK;
        }

        case ARM_POWER_OFF:
        {
            if (!(PHY.flags & PHY_INIT)) {
                return ARM_DRIVER_ERROR;
            }

            PHY.flags &= ~PHY_POWER;
            PHY.bmcr = BMCR_POWER_DOWN;

            return PHY.reg_wr(PHY_ADDR, BMCR, PHY.bmcr);
        }

        case ARM_POWER_LOW:
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
}

static int32_t ARM_ETH_PHY_SetInterface(uint32_t interface)
{
    if (!(PHY.flags & PHY_POWER)) {
        return ARM_DRIVER_ERROR;
    }

    switch (interface) {
        case ARM_ETH_INTERFACE_RMII:
            break;

        case ARM_ETH_INTERFACE_MII:
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    return ARM_DRIVER_OK;
}

static int32_t ARM_ETH_PHY_SetMode(uint32_t mode)
{
    uint16_t bmcr;

    if (!(PHY.flags & PHY_POWER)) {
        return ARM_DRIVER_ERROR;
    }

    bmcr = PHY.bmcr;
    switch (mode & ARM_ETH_PHY_SPEED_Msk) {
        case ARM_ETH_PHY_SPEED_10M:
            bmcr &= ~BMCR_SPEED_SEL;
            break;
        case ARM_ETH_PHY_SPEED_100M:
            bmcr |= BMCR_SPEED_SEL;
            break;
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    switch (mode & ARM_ETH_PHY_DUPLEX_Msk) {
        case ARM_ETH_PHY_DUPLEX_HALF:
            bmcr &= ~BMCR_DUPLEX;
            break;
        case ARM_ETH_PHY_DUPLEX_FULL:
            bmcr |= BMCR_DUPLEX;
            break;
    }

    if (mode & ARM_ETH_PHY_AUTO_NEGOTIATE) {
        bmcr |= BMCR_ANEG_EN;
    }

    if (mode & ARM_ETH_PHY_LOOPBACK) {
        bmcr |= BMCR_LOOPBACK;
    }

    if (mode & ARM_ETH_PHY_ISOLATE) {
        bmcr |= BMCR_ISOLATE;
    }

    PHY.bmcr = bmcr;
    if (PHY.reg_wr(PHY_ADDR, BMCR, PHY.bmcr) != ARM_DRIVER_OK) {
        return ARM_DRIVER_ERROR;
    }

    if (mode & ARM_ETH_PHY_AUTO_NEGOTIATE) {
        uint8_t tout = 0;
        uint16_t bmsr = 0;

        do {
        #if defined(RTE_FREERTOS)
            vTaskDelay(pdMS_TO_TICKS(10));
        #endif
            tout += 10;

            PHY.reg_rd(PHY_ADDR, BMSR, &bmsr);
            if (bmsr & BMSR_ANEG_COMPL) {
                break;
            }
        } while(tout < PHY_ANEG_TOUT);

        if (tout >= PHY_ANEG_TOUT) {
            return ARM_DRIVER_ERROR;
        }
    }

    return ARM_DRIVER_OK;
}

static ARM_ETH_LINK_STATE ARM_ETH_PHY_GetLinkState(void)
{
    uint16_t bmsr = 0;
    ARM_ETH_LINK_STATE state;

    if (PHY.flags & PHY_POWER) {
        PHY.reg_rd(PHY_ADDR, BMSR, &bmsr);
    }

    state = (bmsr & BMSR_LINK_STAT) ? ARM_ETH_LINK_UP : ARM_ETH_LINK_DOWN;

    return state;
}

static ARM_ETH_LINK_INFO ARM_ETH_PHY_GetLinkInfo(void)
{
    uint16_t bmsr = 0;
    ARM_ETH_LINK_INFO info;

    if (PHY.flags & PHY_POWER) {
        PHY.reg_rd(PHY_ADDR, BMSR, &bmsr);
    }

    info.speed = (bmsr & (BMSR_100B_TX_FD | BMSR_100B_TX_HD)) ? \
        ARM_ETH_SPEED_100M : ARM_ETH_SPEED_10M;

    info.duplex = (bmsr & (BMSR_100B_TX_FD | BMSR_10B_T_FD)) ? \
        ARM_ETH_DUPLEX_FULL : ARM_ETH_DUPLEX_HALF;

    return info;
}

ARM_DRIVER_ETH_PHY Driver_ETH_PHY0 =
{
    ARM_ETH_PHY_GetVersion,
    ARM_ETH_PHY_Initialize,
    ARM_ETH_PHY_Uninitialize,
    ARM_ETH_PHY_PowerControl,
    ARM_ETH_PHY_SetInterface,
    ARM_ETH_PHY_SetMode,
    ARM_ETH_PHY_GetLinkState,
    ARM_ETH_PHY_GetLinkInfo,
};
