/**
 ********************************************************************************
 * @file    hbeat.c
 * @author  prashanth kannan
 * @date    2/28/24
 * @brief   LED heart beat app
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "stdint.h"
#include "stdbool.h"
#include "assert.h"

// Drivers
#include "LPC17xx.h"
#include "GPIO_LPC17xx.h"
#include "SSP_LPC17xx.h"

//  OS
#include "FreeRTOS.h"
#include "task.h"

#include "lcd_reg.h"
#include "bmp.h"

/************************************
 * EXTERN VARIABLES
 ************************************/

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/
// RESET pin
#define LCD_RST_PORT            (3U)
#define LCD_RST_PIN             (25U)
// RESET control
#define LCD_RST_ACTIVE          (0U)
#define LCD_RST_INACTIVE        (1U)

// BL pin
#define LCD_BL_PORT             (3U)
#define LCD_BL_PIN              (26U)
// BL control
#define LCD_BL_ON               (1U)
#define LCD_BL_OFF              (0U)

// LCD driver task parameters
#define LCD_DRV_TASK_NAME       "lcd_drv"
#define LCD_DRV_STACK_SIZE      (128U)
#define LCD_DRV_TASK_PRIO       (6U)

// LCP SSP IRQ priority
#define SSP_IRQ_PRIO            (8U)

// 12-bit color definitions
#define WHITE      0xFFF
#define BLACK      0x000
#define RED        0xF00
#define GREEN      0x0F0
#define BLUE       0x00F
#define CYAN       0x0FF
#define MAGENTA    0xF0F
#define YELLOW     0xFF0
#define BROWN      0xB22
#define ORANGE     0xFA0
#define PINK       0xF6A

/************************************
 * PRIVATE TYPEDEFS
 ************************************/

/************************************
 * STATIC VARIABLES
 ************************************/
static TaskHandle_t lcd_drv_task_h;

/************************************
 * GLOBAL VARIABLES
 ************************************/

/************************************
 * STATIC FUNCTION PROTOTYPES
 ************************************/

/************************************
 * STATIC FUNCTIONS
 ************************************/
static void lcd_gpio_init(void)
{
    GPIO_SetDir(LCD_RST_PORT, LCD_RST_PIN, GPIO_DIR_OUTPUT);
    GPIO_SetDir(LCD_BL_PORT, LCD_BL_PIN, GPIO_DIR_OUTPUT);
}

static void lcd_hw_reset(void)
{
    GPIO_PinWrite(LCD_RST_PORT, LCD_RST_PIN, LCD_RST_ACTIVE);
    vTaskDelay(pdMS_TO_TICKS(10));
    GPIO_PinWrite(LCD_RST_PORT, LCD_RST_PIN, LCD_RST_INACTIVE);
    vTaskDelay(pdMS_TO_TICKS(10));
}

static void lcd_bl_control(bool state)
{
    GPIO_PinWrite(LCD_BL_PORT, LCD_BL_PIN, state);
}

static void lcd_spi_irq_cb(uint32_t event)
{
    if (event == ARM_SPI_EVENT_TRANSFER_COMPLETE)
    {
        BaseType_t xHighPrioTaskAwaken;

        vTaskNotifyGiveFromISR(lcd_drv_task_h, &xHighPrioTaskAwaken);
        portYIELD_FROM_ISR(xHighPrioTaskAwaken);
    }
}

static void lcd_spi_init(void)
{
    int32_t ret;

    ret = Driver_SPI0.Initialize(lcd_spi_irq_cb);
    assert(ret == ARM_DRIVER_OK);

    NVIC_SetPriority(SSP0_IRQn, SSP_IRQ_PRIO);
    ret = Driver_SPI0.PowerControl(ARM_POWER_FULL);
    assert(ret == ARM_DRIVER_OK);

    ret = Driver_SPI0.Control(ARM_SPI_MODE_MASTER | ARM_SPI_SS_MASTER_HW_OUTPUT |
        ARM_SPI_CPOL1_CPHA1 | ARM_SPI_DATA_BITS(9), 32000000);
    assert(ret == ARM_DRIVER_OK);
}

static void lcd_spi_write(bool data, uint8_t val)
{
    uint8_t spi_data[2];

    spi_data[0] = val;
    spi_data[1] = data ? 1 : 0;

    int32_t ret = Driver_SPI0.Send(spi_data, 1);
    assert(ret == ARM_DRIVER_OK);

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

static void lcd_power_on(void)
{
    lcd_hw_reset();

    lcd_bl_control(LCD_BL_ON);

    lcd_spi_write(pdFALSE, DISCTL);
    lcd_spi_write(pdTRUE, 0x0);
    lcd_spi_write(pdTRUE, 0x20);
    lcd_spi_write(pdTRUE, 0x0);

    lcd_spi_write(pdFALSE, COMSCN);
    lcd_spi_write(pdTRUE, 0x1);

    lcd_spi_write(pdFALSE, OSCON);

    lcd_spi_write(pdFALSE, SLPOUT);

    lcd_spi_write(pdFALSE, PWRCTR);
    lcd_spi_write(pdTRUE, 0xF);

    lcd_spi_write(pdFALSE, DISINV);

    lcd_spi_write(pdFALSE, DATCTL);
    lcd_spi_write(pdTRUE, 0x0);
    lcd_spi_write(pdTRUE, 0x0);
    lcd_spi_write(pdTRUE, 0x2);

    lcd_spi_write(pdFALSE, VOLCTR);
    lcd_spi_write(pdTRUE, 28);
    lcd_spi_write(pdTRUE, 3);

    vTaskDelay(pdMS_TO_TICKS(100));

    lcd_spi_write(pdFALSE, DISON);

    lcd_spi_write(pdFALSE, PASET);
    lcd_spi_write(pdTRUE, 0);
    lcd_spi_write(pdTRUE, 131);

    lcd_spi_write(pdFALSE, CASET);
    lcd_spi_write(pdTRUE, 0);
    lcd_spi_write(pdTRUE, 131);

    lcd_spi_write(pdFALSE, RAMWR);
    for(int i = 0; i < ((131 * 131) / 2); i++) {
        lcd_spi_write(pdTRUE, (BLACK >> 4) & 0xFF);
        lcd_spi_write(pdTRUE, ((BLACK & 0xF) << 4) | ((BLACK >> 8) & 0xF));
        lcd_spi_write(pdTRUE, BLACK & 0xFF);
    }

    lcd_spi_write(pdFALSE, PASET);
    lcd_spi_write(pdTRUE, 0);
    lcd_spi_write(pdTRUE, 131);

    lcd_spi_write(pdFALSE, CASET);
    lcd_spi_write(pdTRUE, 0);
    lcd_spi_write(pdTRUE, 131);

    lcd_spi_write(pdFALSE, RAMWR);
    for(int i  = 0; i < sizeof(bmp); i++) {
        lcd_spi_write(pdTRUE, bmp[i]);
    }
}

static void lcd_drv_task(void *arg)
{
    lcd_gpio_init();
    lcd_spi_init();
    lcd_power_on();

    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/************************************
 * GLOBAL FUNCTIONS
 ************************************/
void lcd_drv_init(void)
{
    BaseType_t ret = xTaskCreate(lcd_drv_task, LCD_DRV_TASK_NAME, LCD_DRV_STACK_SIZE,
        NULL, LCD_DRV_TASK_PRIO, &lcd_drv_task_h);
    assert(ret);
}
