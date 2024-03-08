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
// OS
#include "FreeRTOS.h"
#include "task.h"

// APPS
#include "hbeat.h"

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

/************************************
 * GLOBAL FUNCTIONS
 ************************************/
int main(void)
{
    // Initialize apps
    hbeat_init();

    vTaskStartScheduler();

    // Execution should never reach here
    while(1);
}