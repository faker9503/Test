#ifndef AM_APP_TASKS_H
#define AM_APP_TASKS_H

// Customize this enum with indices between AM_TASK_NONE and AM_MAX_TASK
// where each index corresponds to a TASK or ISR.
// Naming convention for a task: AM_TASK_xxx
// Naming convention for an ISR: AM_ISR_xxx
typedef enum
{
	AM_TASK_NONE = 0, // The enum must begin with this value as named.
	AM_TASK_BLE,
    AM_TASK_PROTOCOL,
	AM_MAX_TASK // The enum must end with this value as named.
} am_msg_task_enum_t;

#endif
