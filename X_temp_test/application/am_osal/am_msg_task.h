//*****************************************************************************
//
//! @file am_msg_task.h
//!
//! @brief RTOS task factory / encapsulation which supports corresponding queues
//!	with tasks.
//!
//! This module allows the creation of multiple tasks with corresponding queues.
//! The encapsulation offered in this module greatly simplifies the main application 
//!
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

#ifndef AM_MSG_TASK_H
#define AM_MSG_TASK_H

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "am_tasks.h"

//*****************************************************************************
//
// Typedefs
//
//*****************************************************************************


typedef struct
{
	TaskHandle_t task;
	QueueHandle_t queue;
	am_msg_task_enum_t parent;
} am_msg_task_t;

typedef struct
{
	//Task Setup
	am_msg_task_enum_t indx; //Specify this task's index.
	
	TaskFunction_t pxTaskCode; //FreeRTOS function pointer to task
	const char* const pcName; // FreeRTOS name
	const uint16_t usStackDepth; // Stack Size
	void * const pvParameters; // FreeRTOS task parameter mechanism
	UBaseType_t uxPriority; // FreeRTOS Task Priority
	
	//Queue Setup
	const UBaseType_t uxQueueLength;
	//const UBaseType_t uxItemSize; //sizeof (queue_element)
	
	//Parent Task
	am_msg_task_enum_t parent; //Specify parent task.
}am_msg_task_setup_t;


// QUEUE entry for all corresponding queues
typedef struct
{
	am_msg_task_enum_t Source; // The sender lets the receiver know the source.
	uint32_t	MessageType; // May be redefined per task to index different uses of *pData.
	void *pData; // Pointer to Relevant Data
}am_msg_task_queue_element_t;

extern bool am_msg_task_init(void);
extern bool am_msg_task_create(am_msg_task_setup_t setup);
extern bool am_msg_task_send(am_msg_task_enum_t Source, am_msg_task_enum_t Dest, uint32_t MessageType, void *pData);
extern bool am_msg_task_send_fromISR(am_msg_task_enum_t Source, am_msg_task_enum_t Dest, uint32_t MessageType, void *pData);
extern bool am_msg_task_send_up(am_msg_task_enum_t Source, uint32_t MessageType, void *pData);
extern bool am_msg_task_send_up_fromISR(am_msg_task_enum_t Source, uint32_t MessageType, void *pData);
extern bool am_msg_task_read(am_msg_task_enum_t indx, am_msg_task_queue_element_t *Element);
extern TaskHandle_t am_msg_task_get_task_handle(am_msg_task_enum_t indx);
extern void am_msg_task_create_all_tasks(const am_msg_task_setup_t *task_array, uint8_t task_count);
extern void am_msg_task_suspend(am_msg_task_enum_t indx);
extern void am_msg_task_resume(am_msg_task_enum_t indx);
#endif
