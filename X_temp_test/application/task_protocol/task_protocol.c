#include <stdint.h>
#include <stdbool.h>

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_devices.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "portmacro.h"
#include "portable.h"
#include "freertos_ancs.h"

#include "am_msg_task.h"

extern am_msg_task_t tasks[AM_MAX_TASK];
extern void ProtocolVariateInit(void);
extern uint8_t LogicLinkerReceiveFrame(uint8_t *value, uint16_t length);

void am_ctimer_isr(void)
{
    uint32_t ui32Status;

    ui32Status = am_hal_ctimer_int_status_get(false);

    am_hal_ctimer_int_clear(ui32Status);
    am_hal_ctimer_int_service(ui32Status);
}

//*****************************************************************************
//
// Sleep function called from FreeRTOS IDLE task.
// Do necessary application specific Power down operations here
// Return 0 if this function also incorporates the WFI, else return value same
// as idleTime
//
//*****************************************************************************
uint32_t am_freertos_sleep(uint32_t idleTime)
{
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
	
    return 0;
}

//*****************************************************************************
//
// Recovery function called from FreeRTOS IDLE task, after waking up from Sleep
// Do necessary 'wakeup' operations here, e.g. to power up/enable peripherals etc.
//
//*****************************************************************************
void am_freertos_wakeup(uint32_t idleTime)
{
    return;
}

void vApplicationMallocFailedHook(void)
{
    //
    // Called if a call to pvPortMalloc() fails because there is insufficient
    // free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    // internally by FreeRTOS API functions that create tasks, queues, software
    // timers, and semaphores.  The size of the FreeRTOS heap is set by the
    // configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h.
    //
    while (1);
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    (void) pcTaskName;
    (void) pxTask;

    //
    // Run time stack overflow checking is performed if
    // configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    // function is called if a stack overflow is detected.
    //
    while (1)
    {
        __asm("BKPT #0\n") ; // Break into the debugger
    }
}

void ProtocolTask(void *pvParameters)                            
{
	am_msg_task_queue_element_t m;
	
	ProtocolVariateInit();
	
	while(1)
	{
		if(uxQueueMessagesWaiting(tasks[AM_TASK_PROTOCOL].queue))
		{
			if(am_msg_task_read(AM_TASK_PROTOCOL, &m))
			{
				LogicLinkerReceiveFrame(m.pData,sizeof(m.pData)); 
			}
		}

		vTaskDelay(1);
	}	
}
