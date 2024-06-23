/* -------------------------------------------------------------------------- 
 * Copyright (c) 2024 Prashanth Kannan. All 
 * rights reserved.
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
 *
 *
 * $Date:        21. June 2024
 * $Revision:    V1.00
 *
 * Project:      Ethernet Phy Access (PHY) Definitions for Micrel KS8721BL
 * -------------------------------------------------------------------------- */

#ifndef __EPHY_KS8721_H
#define __EPHY_KS8721_H

#if defined(RTE_FREERTOS)
#include "FreeRTOS.h"
#include "task.h"
#endif

#include "Driver_ETH_PHY.h"

/* Basic Registers */
#define BMCR            0x00        /* Basic Mode Control Register       */
#define BMSR            0x01        /* Basic Mode Status Register        */
#define PHYIDR1         0x02        /* PHY Identifier 1                  */
#define PHYIDR2         0x03        /* PHY Identifier 2                  */

/* Basic Mode Control Register */
#define BMCR_RESET          0x8000      /* Software Reset                    */
#define BMCR_LOOPBACK       0x4000      /* Loopback mode                     */
#define BMCR_SPEED_SEL      0x2000      /* Speed Select (1=100Mb/s)          */
#define BMCR_ANEG_EN        0x1000      /* Auto Negotiation Enable           */
#define BMCR_POWER_DOWN     0x0800      /* KS8721 Power Down                 */
#define BMCR_ISOLATE        0x0400      /* Isolate Media interface           */
#define BMCR_REST_ANEG      0x0200      /* Restart Auto Negotiation          */
#define BMCR_DUPLEX         0x0100      /* Duplex Mode (1=Full duplex)       */
#define BMCR_COL_TEST       0x0080      /* Enable Collision Test             */
#define BCMR_TX_DIS         0x0001      /* Disable transmit                  */

/* Basic Status Register */
#define BMSR_100B_T4        0x8000      /* 100BASE-T4 Capable                */
#define BMSR_100B_TX_FD     0x4000      /* 100BASE-TX Full Duplex Capable    */
#define BMSR_100B_TX_HD     0x2000      /* 100BASE-TX Half Duplex Capable    */
#define BMSR_10B_T_FD       0x1000      /* 10BASE-T Full Duplex Capable      */
#define BMSR_10B_T_HD       0x0800      /* 10BASE-T Half Duplex Capable      */
#define BMSR_ANEG_COMPL     0x0020      /* Auto Negotiation Complete         */
#define BMSR_REM_FAULT      0x0010      /* Remote Fault                      */
#define BMSR_ANEG_ABIL      0x0008      /* Auto Negotiation Ability          */
#define BMSR_LINK_STAT      0x0004      /* Link Status (1=established)       */
#define BMSR_JABBER_DET     0x0002      /* Jaber Detect                      */
#define BMSR_EXT_CAPAB      0x0001      /* Extended Capability               */

/* PHY Identifier Registers */
#define PHY_ID1             0x0022      /* KS8721 Device Identifier MSB    */
#define PHY_ID2             0x1619      /* KS8721 Device Identifier LSB    */

/* PHY Timing Parameters */
#define PHY_RESET_TOUT      10          /* Soft reset timeout in ms        */
#define PHY_ANEG_TOUT       100         /* Auto negotiation timeout in ms  */

/* PHY Driver State Flags */
#define PHY_INIT            0x01U       /* Driver initialized                */
#define PHY_POWER           0x02U       /* Driver power is on                */

#define PHY_ADDR            0x1         /* PHY address                       */

/* PHY Driver Control Structure */
typedef struct {
  ARM_ETH_PHY_Read_t  reg_rd;           /* PHY register read function        */
  ARM_ETH_PHY_Write_t reg_wr;           /* PHY register write function       */
  uint16_t            bmcr;             /* BMCR register value               */
  uint8_t             flags;            /* Control flags                     */
  uint8_t             rsvd;             /* Reserved                          */
} PHY_CTRL;

#endif
