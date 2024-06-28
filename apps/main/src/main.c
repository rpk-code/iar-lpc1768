/**
 ********************************************************************************
 * @file    main.c
 * @author  prashanth kannan
 * @date    2/28/24
 * @brief   
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "stdio.h"

// OS
#include "FreeRTOS.h"
#include "task.h"

// APPS
#include "hbeat.h"
#include "serial.h"
#include "lcd_drv.h"

/************************************
 * EXTERN VARIABLES
 ************************************/

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/

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
static void display_system_info(void) {
    printf("******** System Info ********\n");
    printf("MCU: LPC1768\n");
    printf("OS: FreeRTOS %s\n", tskKERNEL_VERSION_NUMBER);
    printf("*****************************\n");
}

/************************************
 * GLOBAL FUNCTIONS
 ************************************/
int main(void)
{
    serial_init();
    display_system_info();

    // Initialize drivers
    lcd_drv_init();

    // Initialize apps
    hbeat_init();

    vTaskStartScheduler();

    // Execution should never reach here
    while(1);
}