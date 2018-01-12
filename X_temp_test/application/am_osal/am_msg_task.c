#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "am_msg_task.h"
#include <stdint.h>
#include <stdbool.h>
#include "am_tasks.h"
#include "am_app_utils.h"

am_msg_task_t tasks[AM_MAX_TASK];

bool am_msg_task_init(void)
{
	for(int i = 0; i < AM_MAX_TASK; i++)
	{
		tasks[i].task = NULL;
		tasks[i].queue = NULL;
		tasks[i].parent = AM_TASK_NONE;
	}
	
	return true;
}

//*****************************************************************************
//
// Create the RTOS task and queue specified by indx, with the specified setup.
//
// This may block and if it does it blocks forever.
//
// Returns true if message successfully sent.
//
//*****************************************************************************
bool am_msg_task_create(am_msg_task_setup_t setup)
{
	bool retval;
	
	if (setup.indx >= AM_MAX_TASK)
		return true;
	
    xTaskCreate(setup.pxTaskCode, setup.pcName, setup.usStackDepth, setup.pvParameters, setup.uxPriority, &(tasks[setup.indx].task));
	
	retval = (tasks[setup.indx].task == NULL);
	tasks[setup.indx].queue = xQueueCreate ( setup.uxQueueLength, sizeof(am_msg_task_queue_element_t));  
	retval |= (tasks[setup.indx].queue == NULL);
	tasks[setup.indx].parent = setup.parent;
	
	vTaskSetApplicationTaskTag( tasks[setup.indx].task, ( TaskHookFunction_t) setup.indx );
	
	return (retval);											 
}

//*****************************************************************************
//
// Send a message from the Source task to Dest queue
//
// This may block and if it does it blocks forever.
//
// Returns true if message successfully sent.
//
//*****************************************************************************
bool am_msg_task_send(am_msg_task_enum_t Source, am_msg_task_enum_t Dest, uint32_t MessageType, void *pData)
{
		BaseType_t retval = pdFAIL;
		am_msg_task_queue_element_t Element;
		Element.Source = Source;
		Element.MessageType = MessageType;
		Element.pData = pData;
	
		if(tasks[Dest].queue != NULL)
		{
			retval = xQueueSendToBack(tasks[Dest].queue, &Element, 0 );
		}
		
		return (retval != pdFAIL);
}

//*****************************************************************************
//
// Send a message from the Source task to the Dest Queue, from an ISR
//
// This may block and if it does it blocks forever.
//
// Returns true if message successfully sent.
//
//*****************************************************************************
bool am_msg_task_send_fromISR(am_msg_task_enum_t Source, am_msg_task_enum_t Dest, uint32_t MessageType, void *pData)
{
	BaseType_t retval = pdFAIL;
	am_msg_task_queue_element_t Element;
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = 0;

	Element.Source = Source;
	Element.MessageType = MessageType;
	Element.pData = pData;
	
	if(tasks[Dest].queue != NULL)
	{
		retval = xQueueSendFromISR( tasks[Dest].queue , &Element, &xHigherPriorityTaskWoken );
	}

    if (retval != pdFAIL)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    return (retval != pdFAIL);
}

//*****************************************************************************
//
// Send a message to the parent of the Source task
//
// This may block and if it does it blocks forever.
//
// Returns true if message successfully sent.
//
//*****************************************************************************
bool am_msg_task_send_up(am_msg_task_enum_t Source, uint32_t MessageType, void *pData)
{
		BaseType_t retval = pdFAIL;
		am_msg_task_queue_element_t Element;
		Element.Source = Source;
		Element.MessageType = MessageType;
		Element.pData = pData;
		if(tasks[tasks[Source].parent].queue != NULL)
		{
			return xQueueSendToBack(tasks[tasks[Source].parent].queue, &Element, 0 );
		}
		
		return (retval != pdFAIL);
}

//*****************************************************************************
//
// Send a message to the parent of the Source task, from an ISR
//
// This may block and if it does it blocks forever.
//
// Returns true if message successfully sent.
//
//*****************************************************************************
bool am_msg_task_send_up_fromISR(am_msg_task_enum_t Source, uint32_t MessageType, void *pData)
{
	BaseType_t retval = pdFAIL;
	am_msg_task_queue_element_t Element;
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = 0;

	Element.Source = Source;
	Element.MessageType = MessageType;
	Element.pData = pData;
	if(tasks[tasks[Source].parent].queue != NULL)
	{
		retval = xQueueSendFromISR( tasks[tasks[Source].parent].queue, &Element, &xHigherPriorityTaskWoken );
	}

    if (retval != pdFAIL)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    return (retval != pdFAIL);
}

bool am_msg_task_read(am_msg_task_enum_t indx, am_msg_task_queue_element_t *Element)
{
	BaseType_t retval = pdFAIL;
	if(tasks[indx].queue != NULL)
	{
		AM_APP_UTIL_DEBUG_OUT_SET(0);
		retval = xQueueReceive(tasks[indx].queue, Element, portMAX_DELAY );
		AM_APP_UTIL_DEBUG_OUT_SET(indx);
	}

	return retval; 
}

TaskHandle_t am_msg_task_get_task_handle(am_msg_task_enum_t indx)
{
	return tasks[indx].task;
}

void am_msg_task_create_all_tasks(const am_msg_task_setup_t *setup_array, uint8_t task_count)
{
	for(uint8_t task = 0; task < task_count; task++)
	{
		am_msg_task_create(setup_array[task]);
	}
}

void am_msg_task_resume(am_msg_task_enum_t indx)
{
	if(tasks[indx].task != NULL)
	{
		vTaskResume(tasks[indx].task);
	}
}

void am_msg_task_suspend(am_msg_task_enum_t indx)
{
	if(tasks[indx].task != NULL)
	{
		vTaskSuspend(tasks[indx].task);
	}
}
