#include "freertos_ancs.h"
#include "task_protocol.h"
#include "bsp_init.h"
#include "am_msg_task.h"

extern void ProtocolTask(void *pvParameters);

static const am_msg_task_setup_t Task_Setup[] = 
{
	{AM_TASK_BLE, RadioTask, "BLE", 512, 0, 3, 16, AM_TASK_NONE},
	{AM_TASK_PROTOCOL, ProtocolTask, "PROTOCOL", 512, 0, 3, 16, AM_TASK_NONE},
};

#define TASK_INIT_COUNT (sizeof(Task_Setup)/sizeof(am_msg_task_setup_t))

void am_app_start_tasks(void)
{
	am_msg_task_init();

	RadioTaskSetup();
	
	am_msg_task_create_all_tasks(Task_Setup, TASK_INIT_COUNT);
	
	am_hal_interrupt_master_enable();	
}

void am_rtos_start(void)
{
	vTaskStartScheduler();
}

void enable_print_interface(void)
{
    am_hal_itm_enable();
    am_bsp_debug_printf_enable();
    am_util_debug_printf_init(am_hal_itm_print);
    am_util_stdio_terminal_clear();
}

int main(void)
{
    am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_MAX);

    am_hal_cachectrl_enable(&am_hal_cachectrl_defaults);

    am_hal_sysctrl_fpu_enable();
	
    am_hal_sysctrl_fpu_stacking_enable(true);

    am_bsp_low_power_init();
	
	BspInit();

	/* Not turning off the Flash as it may be needed to download the image */
    //am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEMEN_FLASH512K);
	
	/* Turn off unused Flash & SRAM */
    am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEMEN_SRAM64K);

	am_app_start_tasks();
	
	am_rtos_start();

	for(;;);
}
