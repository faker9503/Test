#ifndef AM_APP_UTILS_H
#define AM_APP_UTILS_H
#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"

// Enable the debug hooks to output task activity to GPIO pins
#define AM_APP_UTIL_DEBUG_OUT 0


#define AM_APP_UTIL_DEBUG_OUT_INIT()		am_app_util_debug_out_init()
#define AM_APP_UTIL_DEBUG_OUT_SET(n)		am_app_util_debug_out_set(n)
#define AM_APP_UTIL_DEBUG_OUT_GET()			am_app_util_debug_out_get()



extern void am_app_util_tick_clock_set(bool on);
extern void am_app_util_init_swo_debug(void);
extern void am_app_util_uart_init(uint32_t ui32Module, am_hal_uart_config_t* uart_config, uint16_t uart_buff_size, uint8_t* uart_rx_buffer, uint8_t* uart_tx_buffer, uint8_t* ProcID);
extern void am_app_util_uart_enable(uint32_t ui32Module);
extern void am_app_util_uart_disable(uint32_t ui32Module);
extern void am_app_util_uart_transmit_delay(int32_t i32Module);
extern void am_app_util_uart_send(uint32_t ui32Module, char* str);
extern int am_app_util_uart_receive(uint32_t ui32Module, char* str, uint16_t maxlen, bool echo);
extern void am_app_util_pwr_config(void);
extern void am_app_util_button_handler(uint8_t button, am_hal_gpio_handler_t pfnHandler);
extern void am_app_util_gpio_handler(uint8_t gpio_pin, am_hal_gpio_handler_t pfnHandler);
extern void am_app_util_button_init(void);
extern void am_app_util_led_init(void);
extern void am_app_util_debug_out_init(void);
extern void am_app_util_debug_out_set(uint8_t indx);
extern uint8_t am_app_util_debug_out_get(void);

//*****************************************************************************
//
// Interrupt handler for the GPIO pins.
//
//*****************************************************************************

#define AM_APP_UTIL_GPIO_ISR() \
void am_gpio_isr(void) \
{\
    uint64_t ui64Status;\
		uint8_t previous_out;\
		previous_out = AM_APP_UTIL_DEBUG_OUT_GET();\
		AM_APP_UTIL_DEBUG_OUT_SET(AM_ISR_GPIO);\
    ui64Status = am_hal_gpio_int_status_get(false);\
    am_hal_gpio_int_clear(ui64Status);\
    am_hal_gpio_int_service(ui64Status);\
		AM_APP_UTIL_DEBUG_OUT_SET(previous_out);\
}


//*****************************************************************************
//
// UART1 Interrupt Service Routine
//
//*****************************************************************************

#define AM_APP_UTIL_UART0_ISR(handler) \
void am_uart_isr(void)\
{\
	uint32_t status;\
  uint8_t previous_out;\
	previous_out = AM_APP_UTIL_DEBUG_OUT_GET();\
	AM_APP_UTIL_DEBUG_OUT_SET(AM_ISR_UART);\
	status = am_hal_uart_int_status_get(0, false);\
\
\
	if(status & (AM_HAL_UART_INT_RX_TMOUT | AM_HAL_UART_INT_TX | AM_HAL_UART_INT_RX))\
	{\
		am_hal_uart_service_buffered(0, status);\
	}\
\
  if(status & (AM_HAL_UART_INT_RX_TMOUT))\
  {\
		handler();\
  }\
  if(status & (AM_HAL_UART_INT_RX))\
  {\
		handler();\
  }\
	AM_APP_UTIL_DEBUG_OUT_SET(previous_out);\
\
}
#endif

#define AM_APP_UTIL_IOM_ISR_NB(x)                               \
void am_iomaster##x##_isr(void)                             \
{                                                           \
    uint32_t ui32IntStatus;                                 \
		uint8_t previous_out;\
		previous_out = AM_APP_UTIL_DEBUG_OUT_GET();\
	  AM_APP_UTIL_DEBUG_OUT_SET(AM_ISR_IOM##x##);							\
    g_iom_error_status = am_hal_iom_error_status_get(x);    \
    ui32IntStatus = am_hal_iom_int_status_get(x, false);    \
    am_hal_iom_int_clear(x, ui32IntStatus);                 \
    am_hal_iom_int_service(x, ui32IntStatus);               \
		AM_APP_UTIL_DEBUG_OUT_SET(previous_out);\
}
