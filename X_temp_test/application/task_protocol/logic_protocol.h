/*******************************************************************************
            Copyright (C), 2016, GoerTek Technology Co., Ltd.
*******************************************************************************/
/**
  *  @defgroup	logic_linker module name
  *  @brief 	logic_linker
  *  @file 		logic_linker
  *  @author 	vigoss.wang
  *  @version 	1.0
  *  @date    	2016/12/17
  *  @{
  *//*
  *  FileName      : logic_linker
  *  Description   : header of logic linker
  *  FunctionList  :
  * History        :
  * 1.Date         : 2016/12/17
  *    Author      : vigoss.wang
  *    Modification: Created file
*******************************************************************************/
#ifndef _LOG_PROTOCOL_H_
#define _LOG_PROTOCOL_H_
/*----------------------------------------------*
 * Include File                                 *
 *----------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
//#include "ad_nvms.h"
//#include "osal.h"
/*----------------------------------------------*
 * Const Defination                             *
 *----------------------------------------------*/
#define ERROR_CODE_TYPE          0x7F

#define OTHERS_MSG_ALTER        0XEF
#define INCOMING_REMINDER       0x1
#define SMS_REMINDER            0x2
#define WECHAT_REMINDER         0x3
#define LONG_SIT_REMINDER       0x4
#define SYSTEM_EVENT_REMINDER   0x5
#define EVENT_ALARM_REMINDER    0x6
#define FLY_PLAN_REMINDER       0x7
#define HIGHWAY_REMINDER        0x8
#define TIRED_REMINDER          0x9
#define WEATHER_REMINDER        0xa
#define QQ_REMINDER             0xb
#define HANGUP_INCOMINGCALL     0xc
#define ANSWER_INCOMINGCALL     0xd
#define ANSWER_MISSCALL         0xe
#define EMAIL_NOTIFY            0xf
#define INCOMING_MUTE_REMINDER  0x10  //????
#define MAX_ANCS_MSG_MASK       0x5 //local stores 5 ancs message

#define MSG_LENGTH              111//165

#define MAX_BOND_MAC_LEN        17  //add for iot
const static uint32_t GeneralError[36] =
{
    100000,     //success   0
    100001,     //unknown error 1
    100002,
    100003,
    100004,
    100005,     //system busy 5
    100006,
    100007,     //parameter_error 7
    100008,     //malloc_fail 8
    100009,     //response_timeout 9
    100010,
    100011,
    100012,
    100013,
    101001,
    101002,
    101003,
    102001,
    104001,
    107001,
    107002,
    108001,
    109001,
    109002,     //low power ,can not OTA
    109003,
    109004,
    109005,
    109006,
    109007,
    109008,
    109009,
    109010,
    109011,
    112001,
    122001,
    123001
};
/**************************************************************88
*message_type:0x2-new sms reminder  0x3-weixin reminder
*dot_matrix:0x2-message content text dot matrix 0x4-contact content text dot matrix
*text_type:1-message content text 2-contact text
*font_size_value = 10;
*text_word_space_value = 10;
*text_line_space_value = 5;
*text_wrap_value = 1;
*dot_metrix_height_value = 0x0200;
*dot_metrix_width_value = 0x0200;
*dot_metrix_color_value = 1;
*dot_metrix_method_value = 1;
*text_format_support_value = 1;
*text_content_length_value = 20;

*/
static uint8_t infor_restriction[197] =
{
    0x81,0x81,0x42,0x82,0x5f,0x03,0x01,0x03,0x84,0x42,0x85,0x1f,0x06,0x01,0x04,0x87,0x0c,0x08,0x01,0x44,0x09,0x01,0x00,0x0A,0x01,
    0x69,0x0B,0x01,0x00,0x0C,0x02,0x78,0x05,0x0D,0x02,0xc5,0x00,0x0E,0x01,0x14,0x0F,0x01,0xa6,0x85,0x1f,0x06,0x01,0x04,0x87,0x0c,
    0x08,0x01,0x44,0x09,0x01,0x00,0x0A,0x01,0x69,0x0B,0x01,0x00,0x0C,0x02,0x78,0x05,0x0D,0x02,0xc5,0x00,0x0E,0x01,0x14,0x0F,0x01,
    0xa6,0x90,0x16,0x91,0x09,0x12,0x01,0x02,0x13,0x01,0xc5,0x14,0x01,0x14,0x91,0x09,0x12,0x01,0x02,0x13,0x01,0xc5,0x14,0x01,0x14,
    0x82,0x5f,0x03,0x01,0x03,0x84,0x42,0x85,0x1f,0x1c,0x1,0x4,0x9f,0xc,0x20,0x1,0x44,0x21,0x1,0x00,0x22,0x01,0x69,0x0B,0x01,0x00,
    0x0C,0x02,0x78,0x05,0x0D,0x02,0xc5,0x00,0x0E,0x01,0x14,0x0F,0x01,0xa6,0x85,0x1f,0x06,0x01,0x04,0x87,0x0c,0x08,0x01,0x44,0x09,
    0x01,0x00,0x0A,0x01,0x69,0x0B,0x01,0x00,0x0C,0x02,0x78,0x05,0x0D,0x02,0xc5,0x00,0x0E,0x01,0x14,0x0F,0x01,0xa6,0x90,0x16,0x91,
    0x09,0x12,0x01,0x02,0x13,0x01,0xc5,0x14,0x01,0x14,0x91,0x09,0x12,0x01,0x02,0x13,0x01,0xc5,0x14,0x01,0x14,
};
/*----------------------------------------------*
 * Micro Defination                             *
 *----------------------------------------------*/

/**2.1	Device Management Service CommandID */
typedef enum
{
    DMS_CMD_CONNECT_PARA = 1,
    DMS_CMD_SERVIVE_CAPABILITY,
    DMS_CMD_COMMAND_CAPABILITY,
    DMS_CMD_DATETIME_FORMAT,
    DMS_CMD_SET_DATETIME,
    DMS_CMD_GET_DATETIME,
    DMS_CMD_GET_VERSION,
    DMS_CMD_GET_BATTERY,
    DMS_CMD_SET_AUTO_LIGNT_STATUS,
    DMS_CMD_SET_AVOID_DISTURB,
    DMS_CMD_DEL_AVOID_DISTURB,
    DMS_CMD_MODIFY_AVOID_DISTURB,
    DMS_CMD_FACTORY_RESET,
    DMS_CMD_REQ_BT_BOND,
    DMS_CMD_GET_BT_BOND_STATUS,
    DMS_CMD_REQ_HANDSET_INFO,
    DMS_CMD_NOTIFY_INTERVAL,
    DMS_CMD_GET_MOTION_TYPE,
    DMS_CMD_GET_LEGITIMACY,
    DMS_CMD_GET_GOLD_CARD,
    DMS_CMD_NOTIFY_OPEN_APP = 0x15
}DMS_COMMAND_ID_E;

/**2.2	Notification Service CommandID */
typedef enum
{
    NS_CMD_SET_SERVE_REMINDER = 1,
    NS_CMD_GET_SERVE_REMINDER,
    NS_CMD_SET_SWITCH_STATE = 4,
    NS_CMD_QUERY_SUPPORT_TYPE
}NS_COMMAND_ID_E;

/**2.3	Hands Free Call Service CommandID */
typedef enum
{
    HCS_CMD_CALLER_APPEAR = 1,

}HCS_COMMAND_ID_E;

/**2.4	Fitness Service CommandID */
typedef enum
{
    FIT_CMD_SET_MOTION_GOAL = 1,
    FIT_CMD_SET_USER_SIGN,
    FIT_CMD_GET_INTRADAY_MOTION_TOTAL,
    FIT_CMD_GET_ACTIVE_REMIND_MSG = 0x06,
    FIT_CMD_SET_ACTIVE_REMIND_MSG = 0x07,
    FIT_CMD_GET_MOTION_SAMPLE_FRAME = 0x0a,
    FIT_CMD_GET_SAMPLE_DATA,
    FIT_CMD_GET_STATE_DATA_FRAME,
    FIT_CMD_GET_STATE_DATA_PORT,
    FIT_CMD_SET_MOTION_DATA_THRESHOLD,
    FIT_CMD_GET_MOTION_INFORM_APPEAR,
    FIT_CMD_SET_SHOW_COLLECT_SWITCH,
    FIT_CMD_REVERSE_SYNCHRODATA,
    FIT_CMD_TRIGGER_REVERSE_SYNCHRODATA,
    FIT_CMD_SET_HEART_PARAMETER
}FIT_COMMAND_ID_E;
/**2.5	Alarm Service CommandID */
typedef enum
{
    ALA_CMD_SET_ALARM_CLOCK = 1,
    ALA_CMD_SET_CAPACITY_ALARM_CLOCK,

}ALA_COMMAND_ID_E;

/**2.6	OTA Service CommandID */
typedef enum
{
    OTA_CMD_QUERY_UPGRADE_PERMIT = 1,
    OTA_CMD_UPGRADE_PARA_CONSULT,
    OTA_CMD_DATA_REPORT_ACTIVELY,
    OTA_CMD_UPDATE_CONTENT_TRANS,
    OTA_CMD_UPDATE_SIZE_APPEAR,
    OTA_CMD_UPDATE_CHECK_RESULT,
    OTA_CMD_UPDATE_STATE_APPEAR,
    OTA_CMD_UPDATE_CANCEL_COMMAND,
    OTA_CMD_APP_UPDATE_STATE
}OTA_COMMAND_ID_E;

/**2.7	Maintenance Service CommandID */
typedef enum
{
    MAI_CMD_GET_LOG_INFORMATION = 1,
    MAI_CMD_TRANS_PARA_CONSULT,
    MAI_CMD_QUERY_SINGAL_INFORMATION,
    MAI_CMD_APPLY_FILE_TRANS,
    MAI_CMD_FILE_TRANS_ACTIVELY,
    MAI_CMD_FILE_RECEIVE_RESULT,
    MAI_CMD_SET_LOG_RULE,
    MAI_CMD_DATA_SWITCH
}MAI_COMMAND_ID_E;


typedef enum
{
    PHO_CMD_LOST_ALARM = 1,
    PHO_CMD_LOST_REMIND = 3,

}PHONE_LOST_REMIND_ID_E;

/**2.8	Font Service CommandID */
typedef enum
{
    FON_CMD_CELLPHONE_LANGUAGE = 1,

}FON_COMMAND_ID_E;

/**2.9	Workout Service CommandID */
typedef enum
{
    WOR_CMD_INFORM_EQUIPMENT_STATUS = 1,
    WOR_CMD_APPEAR_EQUIPMENT_STATUS,
    WOR_CMD_QUERY_EQUIPMENT_STATUS,
    WOR_CMD_MOTION_REMIND = 0x06,
    WOR_CMD_GET_SINGAL_MOTION_RECORD,
    WOR_CMD_GET_SINGAL_MOTION_RECORD_DATA,
    WOR_CMD_APPEAR_SINGAL_MOTION_RECORD_DATA,
    WOR_CMD_GET_SINGAL_MOTION_RECORD_MINUTE
}WOR_COMMAND_ID_E;

/**2.10	Heart Rate Service CommandID */
typedef enum
{
    HAR_CMD_CLOSE_REAL_HEART = 1,
    HAR_CMD_QUERY_REAL_HEART,
    HAR_CMD_APPEAR_REAL_HEART,
}HAR_COMMAND_ID_E;

typedef enum
{
    DEVICE_MANAGER_SERVICE = 1,
    NOTIFICATION_SERVICE,
    HANDSFREE_CALL_SERVICE = 4,
    FITNESS_SERVICE = 7,
    ALARM_SERVICE,
    OTA_SERVICE,
    MAINTENANCE_SERVICE,
    PHONE_LOST_REMIND,
    FONT_SERVICE = 12,
    PHONE_BATTERY_SERVICE,
    EXERCISE_SERVICE = 22,
    WORKOUT_SERVICE,
    HEART_RATE_SERVICE = 25,
    READ_FLASH_DATA = 0x66,
    ERROR_LOG_SERVICE = 0x77
}SERVICE_ID_E;
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
typedef union
{
    uint32_t f;
    char c[4];
}UINT32_CONV_U;

typedef union
{
    uint16_t f;
    char c[2];
}UINT16_CONV_U;
/*----------------------------------------------*
 * Module Variables                             *
 *----------------------------------------------*/
typedef struct _TlvItem
{
    uint8_t   type;      //bit7= 1,with subtype ; bit7=0, no subtype  type value: bit6~bit0 ; general error = 127
    uint16_t  length;
    uint8_t   *value;
    struct _TlvItem  *brotherNode;
    struct _TlvItem  *childNode;
    struct _TlvItem  *parentNode;

}TlvItem_t;


typedef struct
{
    TlvItem_t motion_struct;
    TlvItem_t motion_type;
    TlvItem_t step;
    TlvItem_t calorie;
    TlvItem_t distance;
    TlvItem_t sleep_time;
}Current_Day_Total_Motion_Data_t;

#pragma pack(1)
typedef struct
{
    uint8_t Avoid_Disturb_Valid;
    uint8_t Avoid_Disturb_Index;
    uint8_t Avoid_Disturb_Switch;
    uint8_t Avoid_Disturb_Type;
    uint8_t Avoid_Disturb_Start_Hour;
    uint8_t Avoid_Disturb_Start_Minute;
    uint8_t Avoid_Disturb_End_Hour;
    uint8_t Avoid_Disturb_End_Minute;
    uint8_t Avoid_Disturb_Cycle;
}FL_Device_Avoid_Disturb;
#pragma pack()
extern FL_Device_Avoid_Disturb g_avoid_disturb;

typedef struct
{
   uint8_t msg_type;  // sms,wechat,qq
   uint8_t contact_length;
   uint8_t contact[25];
   uint8_t title_length;
   uint8_t title[25];
   uint8_t msg_length;
   uint8_t msg_buf[MSG_LENGTH];

}MSG_NODE_T;

typedef struct
{
    uint8_t text_type;
    uint8_t text_format;
    uint8_t *text_content;//maybe need modify size in the futrue
    uint16_t text_content_length;
}TEXT_NOTIFICATION_T;

typedef struct
{
    uint8_t  dot_metrix_type;
    uint16_t dot_metrix_height;
    uint16_t dot_metrix_width;
    uint8_t  dot_metrix_color;
    uint8_t  *dot_metrix_data;
}DOT_METRIX_T;

typedef struct
{

    TEXT_NOTIFICATION_T    g_message_text[3];
    DOT_METRIX_T           g_dot_metrix[3];
    uint16_t               g_message_id;
    uint8_t                g_message_type;
    uint8_t                g_motor_enable;
    uint8_t                g_total_text_msg_num;

}MESSAGE_PUSH_T;
extern MESSAGE_PUSH_T g_message_push;

#pragma pack(1)
typedef struct
{
    uint32_t  start_time;
    uint16_t  time_offset;
} DURATION_T;
#pragma pack()

#pragma pack(1)
typedef struct
{
    uint8_t  data_type;
    uint8_t  value_type;
    uint16_t value;
    uint8_t  action;
} THRESSHOLD_STRUCT_T;
#pragma pack()

#pragma pack(1)
typedef struct
{
    uint8_t bt_bind_os;
    uint32_t bt_bind_feature;
    uint8_t  bt_bind_id[6];

    //bind status
    uint8_t bt_bind_status;
    uint8_t bt_bind_status_info;

    uint8_t bt_bond_version;
    uint8_t bt_bond_version_flag;

    /*for IOT */
    uint8_t bt_bond_mac[MAX_BOND_MAC_LEN];
}BT_BIND_REQ_T;
#pragma pack()
extern BT_BIND_REQ_T  BtBindRequest;

#pragma pack(1)
typedef struct
{
    uint8_t  active_remider_enable;
    uint8_t  active_remider_interval;
    uint16_t active_remider_start;
    uint16_t active_remider_end;
    uint8_t  active_remider_cycle;

}ACTIVE_REMIND_MSG_T;
#pragma pack()

typedef struct
{
    uint8_t wrist_light_switch;
    uint8_t ble_reminder_enable;
    uint8_t phone_prevent_lose;
    uint8_t hrm_operator_type;
    uint8_t bnotification_enable;
    uint8_t bincoming_call_enable;
    uint8_t hrm_clash_enable;
    uint8_t hrm_clash_count;
    uint8_t hrm_measure_flag;
    uint8_t miss_call_flag;
    uint8_t ios_call_reject_flag;

}REMING_SWITCH_T;


extern REMING_SWITCH_T g_reming_switch;

#pragma pack(1)
typedef struct
{
    uint8_t  hrmsoftwareversion[14];
    uint8_t  firmware_version[5];
    uint16_t info_component_size;
    uint8_t  work_mode;
    uint16_t app_wait_Timeout;
    uint16_t device_restart_Timeout;
    uint16_t ota_uint_size;
    uint16_t interval;
    uint8_t  ack_enable;
    uint8_t  package_validity;
    uint32_t package_valid_size;
    uint32_t package_received_size;
    uint32_t ota_file_offset;
    uint32_t ota_file_length;
    uint32_t ota_file_bitmap;
//    nvms_t   ota_update_part;
    uint8_t  ota_app_ready;
    uint16_t ota_package_index;
    uint16_t ota_package_total;
    uint32_t ota_flash_address;
    uint8_t  ota_last_bytes;
//    OS_TIMER ota_timer;
    uint8_t timercnt;
    uint16_t ota_check_sum;
//    uint8_t  ota_animation_index;
    uint8_t  ota_on_going;
    uint8_t  ota_hrm_update_flag;
    uint8_t  ota_req_timeout;
    uint8_t  ota_transfer_finished;
}OTA_Struct_T;
#pragma pack()



//OTA_Struct_T gOTAData;
/*----------------------------------------------*
 * Function Declaration                         *
 *----------------------------------------------*/

extern void PhonePreventLose(uint8_t phone_prevent_lose);
extern void HcsCallerAppear(uint8_t calling_operation_type);
extern void RealTimeHeartTimerDelete(void);
extern void RealTimeHeartTimerInit(void);
extern uint16_t Uint16LtoB(uint16_t data);
extern uint32_t Uint32LtoB(uint32_t data);
extern void OTAApplyUpgradePackageData(uint32_t fileoffset,uint32_t filelength,uint32_t filebitmap);
#endif
/** @}*/
