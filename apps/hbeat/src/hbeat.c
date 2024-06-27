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
#include "assert.h"

// Drivers
#include "GPIO_LPC17xx.h"

//  OS
#include "FreeRTOS.h"
#include "task.h"

/************************************
 * EXTERN VARIABLES
 ************************************/

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/
// LED port
#define HBEAT_LED_PORT      (1U)
#define HBEAT_LED_PIN       (25U)
// LED control
#define HBEAT_LED_ON        (0U)
#define HBEAT_LED_OFF       (1U)
// Heart beat rate
#define HBEAT_LED_ON_MS     (pdMS_TO_TICKS(500))
#define HBEAT_LED_OFF_MS    (pdMS_TO_TICKS(500))

#define HBEAT_TASK_NAME     "hbeat"
#define HBEAT_TASK_PRIO     (1U)
#define HBEAT_STACK_SIZE    (32U)

/************************************
 * PRIVATE TYPEDEFS
 ************************************/

/************************************
 * STATIC VARIABLES
 ************************************/

/************************************
 * GLOBAL VARIABLES
 ************************************/

/************************************
 * STATIC FUNCTION PROTOTYPES
 ************************************/

/************************************
 * STATIC FUNCTIONS
 ************************************/
static void hbeat_task(void *arg)
{
    uint8_t state = HBEAT_LED_ON;

    // Setup LED pin
    GPIO_SetDir(HBEAT_LED_PORT, HBEAT_LED_PIN, GPIO_DIR_OUTPUT);

    while(1) {
        GPIO_PinWrite(HBEAT_LED_PORT, HBEAT_LED_PIN, state);
        (HBEAT_LED_ON == state) ? vTaskDelay(HBEAT_LED_ON_MS) :
            vTaskDelay(HBEAT_LED_OFF_MS);
        state ^= 1;
    }
}

/************************************
 * GLOBAL FUNCTIONS
 ************************************/
void hbeat_init(void)
{
    BaseType_t ret = xTaskCreate(hbeat_task, HBEAT_TASK_NAME, HBEAT_STACK_SIZE,
        NULL, HBEAT_TASK_PRIO, NULL);
    assert(ret);
}
