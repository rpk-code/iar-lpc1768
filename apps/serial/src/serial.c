/**
 ********************************************************************************
 * @file    serial.c
 * @author  prashanth kannan
 * @date    2/28/24
 * @brief   serial output/input
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "stdint.h"
#include "stdbool.h"
#include "stdarg.h"
#include "string.h"
#include "assert.h"

// Drivers
#include "LPC17xx.h"
#include "UART_LPC17xx.h"

// OS
#include "FreeRTOS.h"
#include "semphr.h"

/************************************
 * EXTERN VARIABLES
 ************************************/
extern ARM_DRIVER_USART Driver_USART0;

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/
#define UART_IRQ_PRIO       (15U)
#define TX_BUF_SIZE         (128U)
#define NUM_BUF_SIZE        (12U)
#define MAX_TX_LOCK_TIME    (pdMS_TO_TICKS(5))


#define MIN(a, b)      ((a) < (b) ? (a) : (b))

/************************************
 * PRIVATE TYPEDEFS
 ************************************/
struct tx_ctx {
    char buf[TX_BUF_SIZE];
    SemaphoreHandle_t mutex;
    volatile bool completed;
};

struct serial_ctx {
    struct tx_ctx tx;
};

/************************************
 * STATIC VARIABLES
 ************************************/
static struct serial_ctx ctx;

/************************************
 * GLOBAL VARIABLES
 ************************************/

/************************************
 * STATIC FUNCTION PROTOTYPES
 ************************************/

/************************************
 * STATIC FUNCTIONS
 ************************************/
static inline uint8_t cstrcpy(char *out_buf, uint16_t *out_idx, const char *in_buf, uint32_t len)
{
    uint16_t copy_len = MIN(len, (TX_BUF_SIZE - *out_idx));

    memcpy(&out_buf[*out_idx], in_buf, copy_len);
    *out_idx = *out_idx + copy_len;

    return (*out_idx < TX_BUF_SIZE) ? 1 : 0;
}

static inline uint8_t cuitoa(char *buf, uint32_t val, uint8_t base, bool is_upper)
{
    uint8_t i = 0;

    if (!val) {
        buf[i++] = '0';
        buf[i] = 0;
    } else {
        while (val) {
            uint8_t d = val % base;

            if (d > 9) {
                d -= 10;
                buf[i++] = is_upper ? ('A' + d) : ('a' + d);
            } else {
                buf[i++] = '0' + d;
            }
            val /= base;
        }

        uint8_t x = 0;
        uint8_t y = i-1;

        while (x < y) {
            char tmp = buf[x];

            buf[x++] = buf[y];
            buf[y--] = tmp;
        }
        buf[i] = 0;
    }

    return i;
}

static inline uint8_t citoa(char *buf, int32_t val)
{
    if (val < 0) {
        val = -val;
        buf[0] = '-';
        return cuitoa(&buf[1], (uint32_t)val, 10, false) + 1;
    } else {
        return cuitoa(buf, (uint32_t)val, 10, false);
    }
}

static void usart_cb(uint32_t event)
{
    switch (event) {
        case ARM_USART_EVENT_SEND_COMPLETE:
            {
                ctx.tx.completed = 1;
            }
            break;
        default:
            break;
    }
}

static void serial_tx(const void *buf, uint32_t len)
{
    assert(ctx.tx.completed);
    if (ctx.tx.completed) {
        ctx.tx.completed = 0;
        Driver_USART0.Send(buf, len);
        while(!ctx.tx.completed);
    }
}

/************************************
 * GLOBAL FUNCTIONS
 ************************************/
void serial_init(void)
{
    int32_t ret;

    // Driver initialization
    ret = Driver_USART0.Initialize(usart_cb);
    assert(ret == ARM_DRIVER_OK);

    NVIC_SetPriority(UART0_IRQn, UART_IRQ_PRIO);
    ret = Driver_USART0.PowerControl(ARM_POWER_FULL);
    assert(ret == ARM_DRIVER_OK);

    ret = Driver_USART0.Control(ARM_USART_MODE_ASYNCHRONOUS |
        ARM_USART_DATA_BITS_8 | ARM_USART_PARITY_NONE |
        ARM_USART_STOP_BITS_1 | ARM_USART_FLOW_CONTROL_NONE, 112500);
    assert(ret == ARM_DRIVER_OK);

    ret = Driver_USART0.Control(ARM_USART_CONTROL_TX, 1);
    assert(ret == ARM_DRIVER_OK);

    // TX context initialization
    ctx.tx.mutex = xSemaphoreCreateMutex();
    assert(ctx.tx.mutex);
    ctx.tx.completed = 1;
}

int printf(const char *format, ...)
{
    // printf from ISR is not supported
    if (pdTRUE == xPortIsInsideInterrupt()) {
        return 0;
    }

    bool os_running = (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED);

    // Protect TX
    if (os_running) {
        if (pdFALSE == xSemaphoreTake(ctx.tx.mutex, MAX_TX_LOCK_TIME)) {
            return 0;
        }
    }

    // Process printf
    uint32_t i = 0;
    uint16_t n = 0;
    va_list args;

    va_start(args, format);
    while (format[i]) {
        const char *str_start;
        uint32_t str_len;

        for (str_start = &format[i], str_len = 0; (format[i] && format[i] != '%'); i++, str_len++);

        if (!cstrcpy(ctx.tx.buf, &n, str_start, str_len)) {
            break;
        }

        if (format[i] == '%') {
            char num[NUM_BUF_SIZE];

            i++;
            str_start = num;

        decode_format:
            switch (format[i++]) {
                case 'd':
                case 'i':
                    str_len = citoa(num, va_arg(args, int32_t));
                    break;
                case 'u':
                    str_len = cuitoa(num, va_arg(args, uint32_t), 10, false);
                    break;
                case 'x':
                    str_len = cuitoa(num, va_arg(args, uint32_t), 16, false);
                    break;
                case 'X':
                    str_len = cuitoa(num, va_arg(args, uint32_t), 16, true);
                    break;
                case 'c':
                    num[0] = (char)va_arg(args, int);
                    str_len = 1;
                    break;
                case 's':
                    str_start = va_arg(args, char*);
                    str_len = strlen(str_start);
                    break;
                case 'p':
                    str_len = cuitoa(num, va_arg(args, uint32_t), 16, true);
                    break;
                case 'l':
                    switch (format[i]) {
                        case 'd':
                        case 'i':
                        case 'u':
                        case 'x':
                        case 'X':
                            goto decode_format;
                            break;
                        default:
                            assert(0);
                            break;
                    }
                    break;
                default:
                    assert(0);
                    break;
            }

            assert(str_start);
            if (!cstrcpy(ctx.tx.buf, &n, str_start, str_len)) {
                break;
            }
        }
    }
    va_end(args);

    // Send data out
    serial_tx(ctx.tx.buf, n);

    // Unprotect TX
    if (os_running) {
        xSemaphoreGive(ctx.tx.mutex);
    }

    return n;
}
