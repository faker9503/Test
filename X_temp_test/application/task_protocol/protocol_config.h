/*******************************************************************************
            Copyright (C), 2016, GoerTek Technology Co., Ltd.
*******************************************************************************/
/**
  *  @defgroup	protocol_config module name
  *  @brief 	protocol_config
  *  @file 		protocol_config
  *  @author 	vigoss.wang
  *  @version 	1.0
  *  @date    	2016/12/17
  *  @{
  *//*
  *  FileName      : protocol_config
  *  Description   : config file of protocol
  *  FunctionList  :
  * History        :
  * 1.Date         : 2016/12/17
  *    Author      : vigoss.wang
  *    Modification: Created file
*******************************************************************************/
#ifndef _PROTOCOL_CONFIG_H_
#define _PROTOCOL_CONFIG_H_

#include <stdint.h>
#include <stdbool.h>
//#include "osal.h"
//#include "msg_queues.h"
#include "logic_linker.h"
#include "logic_protocol.h"
//#include "control_function.h"
/*----------------------------------------------*
 * Include File                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * Micro Defination                             *
 *----------------------------------------------*/

#define AES_ENCRYPT_ENABLE (1)

#define MTU_MAX_LENGTH  (20)//80
#define LINKER_FRAME_MAX_LENGTH  254

#define FRAME_BUFFER_SIZE 254
#define RECEIVE_BUFFER_SIZE 1024

/*Logic linker layer min header, which include checksum*/
#define LOGIC_LINKER_FRAME_MIN_HEADER  6

/*Logic linker layer max header, which include checksum*/
#define LOGIC_LINKER_FRAME_MAX_HEADER  7

#define FRAME_PAYLOAD_MIN (LINKER_FRAME_MAX_LENGTH - LOGIC_LINKER_FRAME_MIN_HEADER)
#define FRAME_PAYLOAD_MAX (LINKER_FRAME_MAX_LENGTH - LOGIC_LINKER_FRAME_MAX_HEADER)
/*----------------------------------------------*
 * External Declaration                         *
 *----------------------------------------------*/

/*----------------------------------------------*
 * Extern Prototype                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * Internal Declaration                         *
 *----------------------------------------------*/

/*----------------------------------------------*
 * Globle Variables                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * Module Variables                             *
 *----------------------------------------------*/
//uint8_t g_receive_frame_buffer[FRAME_BUFFER_SIZE];
//uint8_t g_send_frame_buffer[FRAME_BUFFER_SIZE];

//uint8_t g_receive_packet_buffer[RECEIVE_BUFFER_SIZE];
/*----------------------------------------------*
 * Const Defination                             *
 *----------------------------------------------*/
typedef struct
{
    uint8_t  service_id;
    uint8_t  command_id;
    uint8_t  *parameters;
}PAY_LOAD_T;

#define BLE_REC_DATA_MASK           (1 << 1)
#define PRO_GET_MOTION_DATA_FRAME   (1 << 2)
#define PRO_GET_MOTION_DATA         (1 << 3)
#define PRO_GET_MOTION_STATE_FRAME  (1 << 4)
#define PRO_GET_MOTION_STATE        (1 << 5)
#define PRO_CLOSE_SINGLE_MOTION     (1 << 6)
#define DELETE_PROTOCOL_TASK_SELF   (1 << 7)
#define PRO_GET_MOTION_SLEEP_STATISTIC_INFO (1<<8)

#define PRO_GET_SINGLE_MOTION_INFO_OK_NOTIFY (1<<9)
#define PRO_GET_SINGLE_MOTION_DETAIL_INFO_OK_NOTIFY (1<<10)

#define CFG_APP_PROTOCOL            (1)
#define BLE_PATH_TRANS_QUEUE_SIZE    16

#define REAL_TIME_HEART_SENCONDS       (5000)

//msg_queue  ble_normal_datapath_trans_queue;

void ProtocolTaskInit(void);
//TaskHandle_t GetProtocolTask(void);
/*----------------------------------------------*
 * Function Declaration                         *
 *----------------------------------------------*/


#endif /*_PROTOCOL_CONFIG_H_*/
/** @}*/
