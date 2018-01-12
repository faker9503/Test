//*****************************************************************************
//
//! @file am_app_utils.c
//!
//! @brief Collection of am_osal routines to abstract the HAL
//!
//! This module contains several common routines for setup using sequences of
//! calls to the AmbiqSuite HAL.
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2017, Ambiq Micro
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision v1.2.8-386-g1923177-wearable-dev-framework of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "stdbool.h"
#include "FreeRTOS.h"
#include "ambiq_core_config.h"
#include "am_bsp.h"
#include "task_protocol.h"
#include "am_app_utils.h"

static am_hal_uart_config_t* g_uart_config;
static uint16_t g_uart_buff_size;
static uint8_t* g_uart_rx_buffer; 
static uint8_t* g_uart_tx_buffer; 
static uint8_t* g_ProcID;
static uint8_t debug_indx = 0;

void am_app_util_tick_clock_set(bool on)
{
	
	if(on)
	{
//		vPortSetupTimerInterrupt(); // Let FreeRTOS implementation reconfigure the tick
	}
	else
	{
		// OFF - Disable the clock entirely using the HAL for low power demo (circumventing the RTOS)
		#ifdef AM_FREERTOS_USE_STIMER_FOR_TICK
			am_hal_stimer_config(0); // Disable
			am_hal_stimer_int_disable(AM_HAL_STIMER_INT_COMPAREA);
			am_hal_interrupt_disable(AM_HAL_INTERRUPT_STIMER_CMPR0);
		#else
			am_hal_ctimer_stop(0, AM_HAL_CTIMER_TIMERA);
			am_hal_ctimer_int_disable(AM_HAL_CTIMER_INT_TIMERA0);
			am_hal_interrupt_disable(AM_HAL_INTERRUPT_CTIMER);		
		#endif
	}
}

void am_app_util_init_swo_debug(void)
{
	//
  // Initialize the printf interface for ITM/SWO output.
  //
  am_util_stdio_printf_init((am_util_stdio_print_char_t) am_bsp_itm_string_print);
    //
    // Initialize the SWO GPIO pin
    //
    am_bsp_pin_enable(ITM_SWO);

    //
    // Enable the ITM.
    //
    am_hal_itm_enable();

    //
    // Enable debug printf messages using ITM on SWO pin
    //
    am_bsp_debug_printf_enable();	
}

static void am_app_util_uart_init_internal(uint32_t ui32Module)
{
	//TODO: Refactor to make re-entrant!!
		am_util_stdio_sprintf((char *)g_ProcID, "INF,APOLLO,0.00\n");
#ifdef AM_PART_APOLLO
	am_util_stdio_sprintf((char *)g_ProcID, "INF,APOLLO,%f\n", (float)AM_CORECLK_MHZ);
#endif
#ifdef AM_PART_APOLLO2
	am_util_stdio_sprintf((char *)g_ProcID, "INF,APOLLO2,%f\n", (float)AM_CORECLK_MHZ);
#endif

    //
    // Make sure the UART RX and TX pins are enabled.
    //
    am_bsp_pin_enable(COM_UART_TX);
    am_bsp_pin_enable(COM_UART_RX);

    //
    // Power on the selected UART
    //
    am_hal_uart_pwrctrl_enable(ui32Module);

    //
    // Start the UART interface, apply the desired configuration settings, and
    // enable the FIFOs.
    //
    am_hal_uart_clock_enable(ui32Module);

    //
    // Disable the UART before configuring it.
    //
    am_hal_uart_disable(ui32Module);

    //
    // Configure the UART.
    //
    am_hal_uart_config(ui32Module, g_uart_config);

    //
    // Configure the UART FIFO.
    //
    am_hal_uart_fifo_config(ui32Module, AM_HAL_UART_TX_FIFO_1_2 | AM_HAL_UART_RX_FIFO_1_2);

    //
    // Initialize the UART queues.
    //
    am_hal_uart_init_buffered(ui32Module,g_uart_rx_buffer,g_uart_buff_size,
    										g_uart_tx_buffer, g_uart_buff_size);
}

//*****************************************************************************
//
// Initialize the UART
//
//*****************************************************************************
void am_app_util_uart_init(uint32_t ui32Module, am_hal_uart_config_t* uart_config, uint16_t uart_buff_size, uint8_t* uart_rx_buffer, uint8_t* uart_tx_buffer, uint8_t* ProcID)
{
	//TODO: Should place these parameters in a struct passed by pointer to make this module re-entrant!
	g_uart_config = uart_config;
	g_uart_buff_size = uart_buff_size;
	g_uart_rx_buffer = uart_rx_buffer;
	g_uart_tx_buffer = uart_tx_buffer;
	g_ProcID = ProcID;
	am_app_util_uart_init_internal(ui32Module);

}




//*****************************************************************************
//
// Enable the UART
//
//*****************************************************************************
void am_app_util_uart_enable(uint32_t ui32Module)
{

    //
    // Power on the selected UART
    //
    ////am_hal_uart_pwrctrl_enable(ui32Module);
	  am_app_util_uart_init_internal(ui32Module);


    //
    // Enable the UART clock.
    //
    am_hal_uart_clock_enable(ui32Module);

    //
    // Enable the UART.
    //
    am_hal_uart_enable(ui32Module);
    am_hal_uart_int_enable(ui32Module, AM_HAL_UART_INT_RX_TMOUT |
    								   AM_HAL_UART_INT_RX );//|
									   //AM_HAL_UART_INT_TXCMP);

    //
    // Enable the UART pins.
    //
    am_bsp_pin_enable(COM_UART_TX);
    am_bsp_pin_enable(COM_UART_RX);

    am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_UART + ui32Module, configMAX_SYSCALL_INTERRUPT_PRIORITY);
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_UART + ui32Module);
		
		am_util_delay_ms(150); // Delay to allow UART to stabilize and transmit garbage from FIFO.
	        					   // TODO: Is there a way to flush the FIFO cleanly?
}

//*****************************************************************************
//
// Disable the UART
//
//*****************************************************************************
void am_app_util_uart_disable(uint32_t ui32Module)
{

	//
	// Power on the selected UART
	//
	am_hal_uart_pwrctrl_disable(ui32Module);

      //
      // Clear all interrupts before sleeping as having a pending UART interrupt
      // burns power.
      //
      am_hal_uart_int_clear(ui32Module, 0xFFFFFFFF);

      //
      // Disable the UART.
      //
      am_hal_uart_disable(ui32Module);

      //
      // Disable the UART pins.
      //
      am_bsp_pin_disable(COM_UART_TX);
      am_bsp_pin_disable(COM_UART_RX);

      //
      // Disable the UART clock.
      //
      am_hal_uart_clock_disable(ui32Module);
}

//*****************************************************************************
//
// Transmit delay waits for busy bit to clear to allow
// for a transmission to fully complete before proceeding.
//
//*****************************************************************************
void am_app_util_uart_transmit_delay(int32_t i32Module)
{
  //
  // Wait until busy bit clears to make sure UART fully transmitted last byte
  //
  while ( am_hal_uart_flags_get(i32Module) & AM_HAL_UART_FR_BUSY );
}


void am_app_util_uart_send(uint32_t ui32Module, char* str)
{
	am_hal_uart_string_transmit_buffered(ui32Module, str);
}

int am_app_util_uart_receive(uint32_t ui32Module, char* str, uint16_t maxlen, bool echo)
{
		int len;
		// Lets move the ring buffer contents into our local buffer. We may need to retain this command
		// for awhile if, for example, individual key presses are being received from a command shell.
		len = am_hal_uart_char_receive_buffered(ui32Module, str, maxlen);
		
		// Echo if requested.
		if(len && echo) {
			am_app_util_uart_send(ui32Module, str);
		}
		return len;
}

//*****************************************************************************
//
// Sets up the initial configuration on startup, ensuring low power consumption.
// This is intended to be called once before the OS Scheduler begins.
//
//*****************************************************************************
void am_app_util_pwr_config(void)
{
	    //
    // Set the clock frequency
    //
	  am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_MAX);
    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_enable(&am_hal_cachectrl_defaults);

    //
    // Initialize the board.
    //
    am_bsp_low_power_init();




#ifdef AM_PART_APOLLO
 //
 // SRAM bank power setting.
 //
am_hal_mcuctrl_sram_power_set( AM_HAL_MCUCTRL_SRAM_POWER_DOWN_4 |
							   AM_HAL_MCUCTRL_SRAM_POWER_DOWN_5 |
							   AM_HAL_MCUCTRL_SRAM_POWER_DOWN_6 |
							   AM_HAL_MCUCTRL_SRAM_POWER_DOWN_7,
							   AM_HAL_MCUCTRL_SRAM_POWER_DOWN_4 |
							   AM_HAL_MCUCTRL_SRAM_POWER_DOWN_5 |
							   AM_HAL_MCUCTRL_SRAM_POWER_DOWN_6 |
							   AM_HAL_MCUCTRL_SRAM_POWER_DOWN_7);

 //
 // Flash bank power set.
 //
 am_hal_mcuctrl_flash_power_set(AM_HAL_MCUCTRL_FLASH_POWER_DOWN_1);
 #endif  // AM_PART_APOLLO

}

void am_app_util_button_handler(uint8_t button, am_hal_gpio_handler_t pfnHandler)
{
	uint8_t gpio_pin;
	
	switch(button)
	{
		case 0:
			gpio_pin = AM_BSP_GPIO_BUTTON0;
			break;
		case 1:
			gpio_pin = AM_BSP_GPIO_BUTTON1;
			break;
		case 2:
			gpio_pin = AM_BSP_GPIO_BUTTON2;
			break;
		default:
			return;
	}
	am_hal_gpio_pin_config(gpio_pin, AM_HAL_GPIO_INPUT);
	am_hal_gpio_int_polarity_bit_set(gpio_pin, AM_HAL_GPIO_RISING);
	am_app_util_gpio_handler(gpio_pin, pfnHandler);
	am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(gpio_pin));
	am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(gpio_pin));
}


void am_app_util_gpio_handler(uint8_t gpio_pin, am_hal_gpio_handler_t pfnHandler)
{
	am_hal_gpio_int_register(gpio_pin, pfnHandler);
}

//*****************************************************************************
//
// Initialize the interrupts for gpio button presses
// Callbacks are mapped by am_app_util_button_handler calls for each button
// The callbacks are GPIO interrupt driven. 
//
//*****************************************************************************
void am_app_util_button_init(void)
{

    //
    // Set the GPIO interrupt priority to a level that allows interacting with
    // FreeRTOS API calls.
    //
    am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_GPIO, configMAX_SYSCALL_INTERRUPT_PRIORITY);
    //
    // Enable the GPIO interrupts (global)
    //
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_GPIO);
	// TODO: Need to enable GPIO interrupt globally after all tasks are ready.
}

//*****************************************************************************
//
// Initialize the LEDs using the led device driver for the BSP.
//
//*****************************************************************************
void am_app_util_led_init(void)
{
//		am_devices_led_array_init(am_bsp_psLEDs, AM_BSP_NUM_LEDS);
//		am_devices_led_off(am_bsp_psLEDs, 0);
//    am_devices_led_off(am_bsp_psLEDs, 1);
//    am_devices_led_off(am_bsp_psLEDs, 2);
//    am_devices_led_off(am_bsp_psLEDs, 3);
}

void am_app_util_debug_out_init(void)
{
#if AM_APP_UTIL_DEBUG_OUT
	am_hal_gpio_pin_config(8, AM_HAL_GPIO_OUTPUT); 
	am_hal_gpio_pin_config(9, AM_HAL_GPIO_OUTPUT); 
	am_hal_gpio_pin_config(44, AM_HAL_GPIO_OUTPUT); 
	am_hal_gpio_pin_config(39, AM_HAL_GPIO_OUTPUT);
	am_hal_gpio_pin_config(12, AM_HAL_GPIO_OUTPUT);
	am_hal_gpio_pin_config(40, AM_HAL_GPIO_OUTPUT);
	am_hal_gpio_out_bit_clear(40);
#endif
	debug_indx = 0;
}

void am_app_util_debug_out_set(uint8_t indx)
{
#if AM_APP_UTIL_DEBUG_OUT
	uint8_t pins[] = {8, 9, 44, 39, 12};
	

	am_hal_gpio_out_bit_clear(40);

	
	for(int i=0; i<5; i++)
	{
		if(indx & 0x01)
		{
			am_hal_gpio_out_bit_set(pins[i]);
		}
		else
		{
			am_hal_gpio_out_bit_clear(pins[i]);
		}
		
		indx >>=1;
		
	}
	
	am_hal_gpio_out_bit_set(40);
#endif
	debug_indx = indx;
}

uint8_t am_app_util_debug_out_get(void)
{
	return debug_indx;
}
