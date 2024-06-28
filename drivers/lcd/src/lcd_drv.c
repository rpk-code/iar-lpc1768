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
#include "SPI_LPC17xx.h"

//  OS
#include "FreeRTOS.h"
#include "task.h"

/************************************
 * EXTERN VARIABLES
 ************************************/

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/
// RESET pin
#define LCD_RST_PORT        (3U)
#define LCD_RST_PIN         (25U)
// RESET control
#define LCD_RST_ACTIVE      (0U)
#define LCD_RST_INACTIVE    (1U)

// BL pin
#define LCD_BL_PORT         (3U)
#define LCD_BL_PIN          (26U)
// BL control
#define LCD_BL_ON           (1U)
#define LCD_BL_OFF          (0U)

#define SPI_IRQ_PRIO        (8U)

// LCD driver task parameters
#define LCD_DRV_TASK_NAME    "lcd_drv"
#define LCD_DRV_STACK_SIZE   (128U)
#define LCD_DRV_TASK_PRIO    (6U)

// LCD driver registers
#define DISON       0xAF    // Display on
#define DISOFF      0xAE    // Display off
#define DISNOR      0xA6    // Normal display
#define DISINV      0xA7    // Inverse display
#define COMSCN      0xBB    // Common scan dir
#define DISCTL      0xCA    // Display control
#define SLPIN       0x95    // Sleep in
#define SLPOUT      0x94    // Sleep out
#define PASET       0x75    // Page address se
#define CASET       0x15    // Column address 
#define DATCTL      0xBC    // Data scan direc
#define RGBSET8     0xCE    // 256-color posit
#define RAMWR       0x5C    // Writing to memo
#define RAMRD       0x5D    // Reading from me
#define PTLIN       0xA8    // Partial display
#define PTLOUT      0xA9    // Partial display
#define RMWIN       0xE0    // Read and modify
#define RMWOUT      0xEE    // End
#define ASCSET      0xAA    // Area scroll set
#define SCSTART     0xAB    // Scroll start se
#define OSCON       0xD1    // Internal oscill
#define OSCOFF      0xD2    // Internal oscill
#define PWRCTR      0x20    // Power control
#define VOLCTR      0x81    // Electronic volu
#define VOLUP       0xD6    // Increment elect
#define VOLDOWN     0xD7    // Decrement elect
#define TMPGRD      0x82    // Temperature gra
#define EPCTIN      0xCD    // Control EEPROM
#define EPCOUT      0xCC    // Cancel EEPROM c
#define EPMWR       0xFC    // Write into EEPR
#define EPMRD       0xFD    // Read from EEPRO
#define EPSRRD1     0x7C    // Read register 1
#define EPSRRD2     0x7D    // Read register 2
#define NOP         0x25    // NOP instruction


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

static void lcd_spi_init(void)
{
    GPIO_SetDir(1, 20, GPIO_DIR_OUTPUT);
    GPIO_PinWrite(1, 20, 1);
    GPIO_SetDir(1, 21, GPIO_DIR_OUTPUT);
    GPIO_PinWrite(1, 21, 1);
    GPIO_SetDir(1, 24, GPIO_DIR_OUTPUT);
    GPIO_PinWrite(1, 24, 1);
}

static void lcd_spi_write(bool data, uint8_t val)
{
    uint16_t spi_data = val;
    uint16_t spi_mask = 0x100;

    if (data) {
        spi_data |= spi_mask;
    }

    GPIO_PinWrite(1, 21, 0);
    for(uint8_t i = 0; i < 9; i++) {
        GPIO_PinWrite(1, 20, 0);

        if (spi_data & spi_mask) {
            GPIO_PinWrite(1, 24, 1);
        } else {
            GPIO_PinWrite(1, 24, 0);
        }
        spi_mask >>= 1;

        GPIO_PinWrite(1, 20, 1);
    }
    GPIO_PinWrite(1, 21, 1);

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
    lcd_spi_write(pdTRUE, 0x1);
    lcd_spi_write(pdTRUE, 0x0);
    lcd_spi_write(pdTRUE, 0x2);

    lcd_spi_write(pdFALSE, VOLCTR);
    lcd_spi_write(pdTRUE, 32);
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
    lcd_spi_write(pdTRUE, 50);
    lcd_spi_write(pdTRUE, 100);

    lcd_spi_write(pdFALSE, CASET);
    lcd_spi_write(pdTRUE, 50);
    lcd_spi_write(pdTRUE, 100);

    lcd_spi_write(pdFALSE, RAMWR);
    for(int i = 0; i < ((131 * 131) / 2); i++) {
        lcd_spi_write(pdTRUE, (CYAN >> 4) & 0xFF);
        lcd_spi_write(pdTRUE, ((CYAN & 0xF) << 4) | ((CYAN >> 8) & 0xF));
        lcd_spi_write(pdTRUE, CYAN & 0xFF);
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
