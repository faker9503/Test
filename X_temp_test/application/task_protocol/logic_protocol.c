/*******************************************************************************
            Copyright (C), 2016, GoerTek Technology Co., Ltd.
*******************************************************************************/
/**
  *  @defgroup  logic_linker module name
  *  @brief     logic_linker
  *  @file      logic_linker
  *  @author    vigoss.wang
  *  @version   1.0
  *  @date      2016/12/17
  *  @{
  *//*
  *  FileName      : logic_linker
  *  Description   : Logic linker file of protocol
  *  FunctionList  :
  * History        :
  * 1.Date         : 2016/12/17
  *    Author      : vigoss.wang
  *    Modification: Created file
*******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
//#include <time.h>
//#include "FreeRTOS.h"
//#include "task.h"
//#include "queue.h"
//#include "semphr.h"

//#include "osal.h"
//#include "msg_queues.h"
//#include "motion_process.h"
//#include "hw_usb_charger.h"
#include "ble_tlv.h"
//#include "logging.h"
//#include "normal_datapath_handler.h"
//#include "sw_calendar.h"
//#include "alarm.h"
//#include "pxp_reporter_task.h"
//#include "ui_fsm.h"
//#include "gsensor_task.h"
//#include "sys_socf.h"
//#include "app_common.h"
#include "fifo.h"
//#include "hr_task.h"
//#include "memory_task.h"
//#include "MotionSleepDataBaseOpt.h"
#include "bsp_flash.h"
#include "sha256.h"
#include "hmac.h"
#include "aes.h"
//#include "app_motor.h"
//#include "app_touch.h"
//#include "error_log.h"

DISPLY_STATE_T g_diaply_msg;
uint8_t g_bond_addr[2] = {0xFF, 0xFF}; 

OTA_Struct_T gOTAData;

extern uint8_t g_receive_packet_buffer[RECEIVE_BUFFER_SIZE];

//extern  EVENT_ALARM_T g_event_alarm[MAX_EVENT_ALARM];
//extern SMART_ALARM_T g_smart_alarm;
uint8_t g_single_motion_cnt = 0;
//single_motion_detail_t g_single_motion_buffer[5];



uint8_t  bt_bond_key[32] = {0};
uint8_t  g_decrypt_iv[16] = {0};

uint8_t  g_bt_bond_iv[16] = {0};
TlvItem_t *Tlvs_value = NULL;
TlvItem_t Tlvs_value_resp;
uint8_t   *ItemStr = NULL;
uint8_t   *CryptItemStr = NULL;

BT_BIND_REQ_T  BtBindRequest;

MESSAGE_PUSH_T g_message_push;
//DISPLY_STATE_T g_diaply_msg;
FL_Device_Avoid_Disturb g_avoid_disturb;
REMING_SWITCH_T g_reming_switch;

uint8_t  Device_SoftVersion[8]= {0};
uint32_t g_threshold_step = 0;

extern uint8_t g_bond_addr[2];

//xSemaphoreHandle semaphoreBleTx;

#define OTA_EXTERNAL_FLASH_PAGE_SIZE    (256)
#define OTA_RECEIVE_BUFFER_SIZE         (128)
#define OTA_MCU_START_OFFSET            (48)
#define OTA_UPDATE_PART_SIZE            (440*1024)//(520*1024)

static void HandleAppItem(TlvItem_t *item,uint8_t service_iden,uint8_t command_id);
/******************************************************************************/
/** Conversion little endian 32 data to big endian
  *
  * @param[in] data
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/14
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
uint32_t Uint32LtoB(uint32_t data)
{
    UINT32_CONV_U d1,d2;
    d1.f = data;
    d2.c[0] = d1.c[3];
    d2.c[1] = d1.c[2];
    d2.c[2] = d1.c[1];
    d2.c[3] = d1.c[0];
    return d2.f;
}
uint16_t Uint16LtoB(uint16_t data)
{
    UINT16_CONV_U d1,d2;
    d1.f = data;
    d2.c[0] = d1.c[1];
    d2.c[1] = d1.c[0];
    return d2.f;
}
uint8_t CharacterConversion(uint8_t *input,uint8_t *output,uint32_t Len)
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t unit;
    uint8_t decade;

    if((input == NULL)||(Len == 0))
    {
        return LOGIC_LINKER_PAR_ERROR;
    }

    for(i = 0; i < Len; i++)
    {
        decade = input[i]>>4;
        unit = input[i]&0xFu;

        if(decade)
        {
            output[j++] = decade + 0x30;
        }

        output[j++] = unit + 0x30;

        if(i != (Len-1))
        {
            output[j++] = 0x2E;
        }
    }

    return j;
}
static void GetFirmwareVersion(void)
{
//    nvms_t para_part = ad_nvms_open(NVMS_FW_UPDATE_FLAG_PART); //added by ryan.wu 20170123
//    ad_nvms_read(para_part,3,  gOTAData.firmware_version,5);//read five bytes from PARAM_PART for software version.

    if((gOTAData.firmware_version[0] == 0xFF) || (gOTAData.firmware_version[1] == 0xFF) || (gOTAData.firmware_version[2] == 0xFF))
    {
        gOTAData.firmware_version[0] = 0x00;
        gOTAData.firmware_version[1] = 0x00;
        gOTAData.firmware_version[2] = 0x00;
    }

//    for(veridx=0;veridx<5;veridx++)
//    printf("firmware_version[%d]= 0x%.2x\r\n",veridx,gOTAData.firmware_version[veridx]);

//    CharacterConversion(gOTAData.firmware_version,Device_SoftVersion,3);
//    printf("length=%d\r\n",length);
//    for(veridx=0;veridx<length;veridx++)
//    printf("Device_SoftVersion[%d]= %c\r\n",veridx,Device_SoftVersion[veridx]);
}

static uint8_t CharToHex(uint8_t bHex)
{
    if((bHex>=0)&&(bHex<=9))
    {
        bHex += 0x30;
    }

    else if((bHex>=10)&&(bHex<=15))//Capital
    {
        bHex += 0x37;
    }

    else
    {
        bHex = 0xff;
    }

    return bHex;
}
static uint8_t GainBleAddress(uint8_t *output)
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t unit;
    uint8_t decade;
    uint8_t address[6];

//    ad_ble_get_public_address(address);
	address[0]=0xAC;
	address[1]=0x37;
	address[2]=0x54;
	address[3]=0xD9;
	address[4]=0x01;
	address[5]=0x50;

    for(i = 0; i < 6; i++)
    {
        decade = address[5-i]>>4;
        unit = address[5-i]&0xFu;

        output[j++] = CharToHex(decade);
        output[j++] = CharToHex(unit);

        if(i != (6-1))
        {
            output[j++] = 0x3A;
        }
    }

    return j;
}
/******************************************************************************/
/** Send Frame Pack
  *
  * @param[in] item
  * @param[in] service_id
  * @param[in] command_id
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/20
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void SendFramePack(TlvItem_t *item,uint8_t service_id,uint8_t command_id)
{
    uint32_t cur_data_length = 0;
    uint8_t  structure[2];

    structure[0] = service_id;
    structure[1] = command_id;

    if(NULL != ItemStr)
    {
        vPortFree(ItemStr);
        ItemStr = NULL;
    }

    ItemStr = TlvItem_MallocStrFromItem(item,&cur_data_length);

    if(NULL == ItemStr)
    {
        //xSemaphoreGive(semaphoreBtChipTx);
        return;
    }

    LogicAppSendPacket(structure,2,ItemStr,cur_data_length);

    if(NULL != ItemStr)
    {
        vPortFree(ItemStr);
        ItemStr = NULL;
    }
}
/******************************************************************************/
/** Send Frame Pack Encrypt
  *
  * @param[in] item
  * @param[in] service_id
  * @param[in] command_id
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/28
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void SendFramePackEncrypt(TlvItem_t *item,uint8_t service_id,uint8_t command_id)
{
#if 1
    uint8_t encrypt_buf[ ]= {0x00,0x00,0x7c,0x01,0x01,0x7d,0x10,0x77,0x8d,0xbf,0xed,0x69,0x54,
                             0xe8,0x0b,0x3d,0xde,0xf9,0xeb,0xa3,0xee,0x0d,0xc3,0x7e,0x00,0x00
                            };
    //uint8_t payload_buf[1024]={0};
    uint32_t cur_data_length = 0;
    uint32_t encrypt_length = 0;
    uint32_t index = 0;
    uint8_t  padding = 0;

    memset(&g_receive_packet_buffer,0,1024);
    encrypt_buf[0] = service_id;
    encrypt_buf[1] = command_id;
    index = 7;
    memcpy(&encrypt_buf[index],g_decrypt_iv,16);
    index += 17;

    if(NULL != CryptItemStr)
    {
        vPortFree(CryptItemStr);
        CryptItemStr = NULL;
    }

    CryptItemStr = TlvItem_MallocStrFromItem(item,&cur_data_length);
    memcpy(&g_receive_packet_buffer,CryptItemStr,cur_data_length);
    encrypt_length = ((cur_data_length % 16) > 0) ? ((cur_data_length / 16 + 1)*16) : cur_data_length;

    padding = encrypt_length - cur_data_length;

    if(padding > 0)
    {
        for(uint8_t i = 0; i < padding ; i++)
        {
            g_receive_packet_buffer[cur_data_length+i] = padding;
        }

//        LogProtocol("16byte not \r\n");
    }

    else
    {
        for(uint8_t i = 0; i < 16 ; i++)
        {
            g_receive_packet_buffer[cur_data_length+i] = 16;
        }

        encrypt_length += 16;
//        LogProtocol("16byte \r\n");
    }

    if(encrypt_length >= 1024)
    {
        encrypt_length = 1024;
    }

    if(encrypt_length > 0x7F)
    {
        encrypt_buf[index++] = (uint8_t)((encrypt_length / BIGLEN_MASK) | BIGLEN_MASK);
        encrypt_buf[index++] = (uint8_t)(encrypt_length % BIGLEN_MASK);
    }

    else
    {
        encrypt_buf[index++] = (uint8_t)encrypt_length;
    }

//    AesCbcEncryptFunction(g_Flash_Control_Struct.g_decrypt_key,g_decrypt_iv,g_receive_packet_buffer,g_receive_packet_buffer,encrypt_length);

    LogicAppSendPacket(encrypt_buf,index,g_receive_packet_buffer,encrypt_length);

    if(NULL != CryptItemStr)
    {
        vPortFree(CryptItemStr);
        CryptItemStr = NULL;
    }

#endif

#if 0
    uint8_t encrypt_type;
    uint32_t cur_data_length = 0;

    TlvItem_t item_encrypt_enable;
    TlvItem_t item_encrypt_iv;
    TlvItem_t item_encrypt_data;

    if(NULL != CryptItemStr)
    {
        vPortFree(CryptItemStr);
        CryptItemStr = NULL;
    }

    CryptItemStr = TlvItem_MallocStrFromItem(item,&cur_data_length);
    AesCbcEncryptFunction(g_decrypt_key,g_decrypt_iv,CryptItemStr,CryptItemStr,cur_data_length);

    memset(&item_encrypt_enable,0,sizeof(TlvItem_t));
    item_encrypt_enable.type = 0x7C;
    item_encrypt_enable.length = 0x01;
    encrypt_type = 0x01;
    item_encrypt_enable.value = &encrypt_type;

    memset(&item_encrypt_iv,0,sizeof(TlvItem_t));
    item_encrypt_iv.type = 0x7D;
    item_encrypt_iv.length = 0x10;
    item_encrypt_iv.value = g_decrypt_iv;
    TlvItem_AddBrother(&item_encrypt_enable,&item_encrypt_iv);

    memset(&item_encrypt_data,0,sizeof(TlvItem_t));
    item_encrypt_data.type = 0x7E;
    item_encrypt_data.length = (uint16_t)cur_data_length;
    item_encrypt_data.value = CryptItemStr;
    TlvItem_AddBrother(&item_encrypt_enable,&item_encrypt_data);

    SendFramePack(&item_encrypt_enable,service_id,command_id);

    if(NULL != CryptItemStr)
    {
        vPortFree(CryptItemStr);
        CryptItemStr = NULL;
    }

#endif
}
/******************************************************************************/
/** ReceiveFramePackEncrypt
  *
  * @param[in] payload_total_length
  * @param[in] service_id
  * @param[in] command_id
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/2/21
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void ReceiveFramePackEncrypt(uint16_t payload_total_length,uint8_t service_id,uint8_t command_id)
{
    uint16_t encrypt_len = 0;
    uint16_t encrypt_index = 0;

    if((0x7C == g_receive_packet_buffer[2])&&(0x7D == g_receive_packet_buffer[5])
            &&(0x7E == g_receive_packet_buffer[23]))
    {
        encrypt_index += 7;
        memcpy(g_decrypt_iv,&g_receive_packet_buffer[encrypt_index],16);
        encrypt_index += 17;

        if(g_receive_packet_buffer[encrypt_index]&0x80)
        {
            encrypt_len = (0x7F) & g_receive_packet_buffer[encrypt_index];
            encrypt_index += 1;
            encrypt_len = encrypt_len * BIGLEN_MASK + (0x7F & g_receive_packet_buffer[encrypt_index]);
            encrypt_index += 1;
        }

        else
        {
            encrypt_len = g_receive_packet_buffer[encrypt_index];
            encrypt_index += 1;
        }
    	for(int i=0;i<encrypt_len;i++)
	    {
	        am_util_stdio_printf("g_receive_packet_buffer_before[%d]=%x\n",i,g_receive_packet_buffer[encrypt_index+i]);
	    }
        AesCbcDecryptFunction(g_Flash_Control_Struct.g_decrypt_key,g_decrypt_iv,&g_receive_packet_buffer[2],&g_receive_packet_buffer[encrypt_index],encrypt_len);
    	for(int i=0;i<(encrypt_len+2);i++)
	    {
	        am_util_stdio_printf("g_receive_packet_buffer_after[%d]=%x\n",i,g_receive_packet_buffer[i]);
	    }
	}

    Tlvs_value = TlvItem_MallocTree(payload_total_length-2,&g_receive_packet_buffer[2]);

    if(NULL == Tlvs_value)
        return;

    HandleAppItem(Tlvs_value,service_id,command_id);

    if(NULL != Tlvs_value)
    {
        TlvItem_FreeTree(Tlvs_value);
        Tlvs_value = NULL;
    }
}
/******************************************************************************/
/** General Error ACK
  *
  * @param[in] error_code
  * @param[in] ser_id
  * @param[in] cmd_id
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/23
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void GeneralErrorACK(uint32_t error_code,uint8_t ser_id,uint8_t cmd_id)
{
    uint32_t  ErrorCodeVaule = 0x00;

    ErrorCodeVaule = Uint32LtoB(error_code);

    memset(&Tlvs_value_resp,0,sizeof(TlvItem_t));
    Tlvs_value_resp.type = ERROR_CODE_TYPE;
    Tlvs_value_resp.length = 0x04;
    Tlvs_value_resp.value = (uint8_t *)(&ErrorCodeVaule);

    SendFramePack(&Tlvs_value_resp,ser_id,cmd_id);
}
/******************************************************************************/
/** DMS Connect Para Negotiate 2.1.1
  *
  * @param[in] item_t
  * @param[in] command_id
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/19
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void DmsConnectParaNegotiate(TlvItem_t *item)
{
    uint8_t type;
    uint8_t random_value[18]= {0x00,0x02,0x55,0xa1,0xe5,0xfa,0x11,0x99,0x13,
                               0x26,0xae,0x8b,0xd9,0xe6,0x3b,0x6c,0x57,0xb7
                              };

    TlvItem_t temp_item_protocal_version;
    TlvItem_t temp_item_max_frame_size;
    TlvItem_t temp_item_max_tr_unit;
    TlvItem_t temp_item_interval;
    TlvItem_t temp_item_random_value;

    uint8_t temp_protocal_version;
    uint16_t temp_max_frame_size;
    uint16_t temp_max_tr_unit;
    uint16_t temp_interval;
    uint16_t temp_value;

    if(NULL == item)
    {
        GeneralErrorACK(GeneralError[6],1,1);
        return;
    }

    if(NULL == TlvItem_ItemWithType(item,1))
    {
        GeneralErrorACK(GeneralError[6],1,1);
        return;
    }

    if(NULL == TlvItem_ItemWithType(item,2))
    {
        GeneralErrorACK(GeneralError[6],1,1);
        return;
    }

    if(NULL == TlvItem_ItemWithType(item,3))
    {
        GeneralErrorACK(GeneralError[6],1,1);
        return;
    }

    if(NULL == TlvItem_ItemWithType(item,4))
    {
        GeneralErrorACK(GeneralError[6],1,1);
        return;
    }

    memset(&temp_item_protocal_version,0,sizeof(TlvItem_t));
    memset(&temp_item_max_frame_size,0,sizeof(TlvItem_t));
    memset(&temp_item_max_tr_unit,0,sizeof(TlvItem_t));
    memset(&temp_item_interval,0,sizeof(TlvItem_t));
    memset(&temp_item_random_value,0,sizeof(TlvItem_t));

    temp_item_protocal_version.type = 1;
    temp_item_protocal_version.length = 1;
    temp_protocal_version = 0x02;
    temp_item_protocal_version.value = &temp_protocal_version;

    temp_item_max_frame_size.type = 2;
    temp_item_max_frame_size.length = 0x02;
    temp_value = LINKER_FRAME_MAX_LENGTH;
    temp_max_frame_size = Uint16LtoB(temp_value);
    temp_item_max_frame_size.value = (uint8_t*)&(temp_max_frame_size);
    TlvItem_AddBrother(&temp_item_protocal_version,&temp_item_max_frame_size);

    temp_item_max_tr_unit.type = 3;
    temp_item_max_tr_unit.length = 0x02;
    temp_value = MTU_MAX_LENGTH;
    temp_max_tr_unit = Uint16LtoB(temp_value);
    temp_item_max_tr_unit.value = (uint8_t*)&(temp_max_tr_unit);
    TlvItem_AddBrother(&temp_item_protocal_version,&temp_item_max_tr_unit);

    temp_item_interval.type = 4;
    temp_item_interval.length = 0x02;
    temp_value = 10;
    temp_interval = Uint16LtoB(temp_value);
    temp_item_interval.value = (uint8_t*)&(temp_interval);
    TlvItem_AddBrother(&temp_item_protocal_version,&temp_item_interval);

    temp_item_random_value.type = 5;
    temp_item_random_value.length = sizeof(random_value);
    temp_item_random_value.value = random_value;
    TlvItem_AddBrother(&temp_item_protocal_version,&temp_item_random_value);

    //BtBindRequest.bt_bind_os = 0;
    SendFramePack(&temp_item_protocal_version,1,1);
}
/******************************************************************************/
/**  Servive Capability 2.1.2
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/27
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void DmsServiveCapability(TlvItem_t *item)
{
    uint8_t item_type;
    TlvItem_t item_service_response;
    uint8_t service_response[25]= {0x01,0x01,0x00,0x01,0x00,0x00,0x01,0x01,
                                   0x01,0x00,0x01,0x01,0x00,0x00,0x00,0x00,
                                   0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,
                                   0x01
                                  };

    if(NULL == item)
    {
        return;
    }

    item_type = TlvItem_ItemType(item);

    if(1 ==item_type)
    {
        memset(&item_service_response,0,sizeof(TlvItem_t));

        item_service_response.type = 2;
        item_service_response.length = sizeof(service_response);
        item_service_response.value = service_response;

        SendFramePack(&item_service_response,1,2);
    }
}
/******************************************************************************/
/**  Command Capability  2.1.3
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/27
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void DmsCommandCapability(TlvItem_t *item)
{
    TlvItem_t item_command_request_struct;
    TlvItem_t item_service_id[15];
    TlvItem_t item_command_support[15];
    uint8_t service_id[15]= {0x01,0x02,0x03,0x04,0x07,0x08,0x09,0x0a
                             ,0x0b,0x0c,0x0f,0x16,0x17,0x18,0x19
                            };
    uint8_t *command_support = NULL;
    uint8_t command_length = 0;
    uint8_t command_support1[12]= {0x01,0x01,0x01,0x01,0x01,0x01,
                                   0x01,0x01,0x01,0x01,0x01,0x01
                                  };
    uint8_t command_support2[3]= {0x01,0x01,0x01};
    uint8_t command_support3[1]= {0x00};
    uint8_t command_support4[1]= {0x01};
    uint8_t command_support7[9]= {0x01,0x01,0x01,0x01,0x01,0x01,
                                  0x01,0x00,0x01
                                 };
    uint8_t command_support8[2]= {0x01,0x01};
    uint8_t command_support9[1]= {0x01};
    uint8_t command_support10[3]= {0x00,0x00,0x00};
    uint8_t command_support11[2]= {0x01,0x01};
    uint8_t command_support12[1]= {0x01};
    uint8_t command_support15[1]= {0x00};
    uint8_t command_support22[3]= {0x01,0x01,0x00};
    uint8_t command_support23[4]= {0x01,0x01,0x01,0x01};
    uint8_t command_support24[2]= {0x00,0x00};
    uint8_t command_support25[1]= {0x01};

    if(NULL == item)
    {
        return;
    }

    memset(&item_command_request_struct,0,sizeof(TlvItem_t));

    item_command_request_struct.type = 1 | CHILD_MASK;
    item_command_request_struct.length = 0;

    for(uint8_t i = 0; i < 15 ; i++)            //number note
    {
        memset(&item_service_id[i],0,sizeof(TlvItem_t));
        item_service_id[i].type = 0x02;
        item_service_id[i].length = 0x01;
        item_service_id[i].value = (uint8_t*)&service_id[i];
        TlvItem_AddChild(&item_command_request_struct,&item_service_id[i]);

        switch(i)
        {
            case 0 :
                command_support = command_support1;
                command_length = sizeof(command_support1);
                break;

            case 1 :
                command_support = command_support2;
                command_length = sizeof(command_support2);
                break;

            case 2 :
                command_support = command_support3;
                command_length = sizeof(command_support3);
                break;

            case 3 :
                command_support = command_support4;
                command_length = sizeof(command_support4);
                break;

            case 4 :
                command_support = command_support7;
                command_length = sizeof(command_support7);
                break;

            case 5 :
                command_support = command_support8;
                command_length = sizeof(command_support8);
                break;

            case 6 :
                command_support = command_support9;
                command_length = sizeof(command_support9);
                break;

            case 7 :
                command_support = command_support10;
                command_length = sizeof(command_support10);
                break;

            case 8 :
                command_support = command_support11;
                command_length = sizeof(command_support11);
                break;

            case 9:
                command_support = command_support12;
                command_length = sizeof(command_support12);
                break;

            case 10 :
                command_support = command_support15;
                command_length = sizeof(command_support15);
                break;

            case 11 :
                command_support = command_support22;
                command_length = sizeof(command_support22);
                break;

            case 12 :
                command_support = command_support23;
                command_length = sizeof(command_support23);
                break;

            case 13 :
                command_support = command_support24;
                command_length = sizeof(command_support24);
                break;

            case 14 :
                command_support = command_support25;
                command_length = sizeof(command_support25);
                break;

            default:
                break;
        }

        memset(&item_command_support[i],0,sizeof(TlvItem_t));
        item_command_support[i].type = 0x04;
        item_command_support[i].length = command_length;
        item_command_support[i].value = command_support;
        TlvItem_AddChild(&item_command_request_struct,&item_command_support[i]);
    }
    SendFramePack(&item_command_request_struct,1,3);
}
/******************************************************************************/
/** Set Date Time Format   2.1.4
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/24
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void DmsSetDateTimeFormat(TlvItem_t *item)
{
    TlvItem_t *child_item = NULL;
    TlvItem_t *brotheritem = NULL;

    uint8_t  date_display_type = 0;
    uint8_t  time_display_type = 0;
    uint32_t length = 0;

    if(NULL == item)
    {
        return;
    }

    child_item = TlvItem_Child(item);
    date_display_type = *TlvItem_ItemValue(child_item,&length);
    brotheritem = TlvItem_Next(child_item);
    time_display_type = *TlvItem_ItemValue(brotheritem,&length);

    GeneralErrorACK(GeneralError[0],1,4);
}
/******************************************************************************/
/** Set Device Time 2.1.5
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/20
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void DmsSetDeviceTime(TlvItem_t *item)
{
    uint8_t  *item_value=NULL;
    uint32_t time_value = 0;
    uint16_t time_zone;

    TlvItem_t *tempitem = NULL;

    tempitem = TlvItem_ItemWithType(item,1);

    if(NULL != tempitem)
    {
        TlvItem_ItemInt32Value(tempitem,&time_value,1);
    }

    item_value = item->brotherNode->value;
    *((uint8_t *)(&time_zone)+1) = *(item_value)&0xff;
    *((uint8_t *)(&time_zone)+0) = *(item_value+1);

//    SetGreenwishCalendarByTimestamps(time_value,time_zone);    // Need to do later...
    GeneralErrorACK(GeneralError[0],1,5);
}
/******************************************************************************/
/** Get Device Time Info 2.1.6
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/20
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void DmsGetDeviceTimeInfo(TlvItem_t *item)
{
    uint8_t item_type;
//    CALENDAR_T calendar_pt;                               //Need to do later...

    item_type = TlvItem_ItemType(item);

    if(2 ==item->brotherNode->type)
    {
        TlvItem_t item_timezone_type;
        uint32_t temp_time=0x00;
        int16_t  temp_zone=0x00;

//        GetGreenwishCalendar(&calendar_pt);                  //Need to do later...
        memset(&Tlvs_value_resp,0,sizeof(TlvItem_t));
        memset(&item_timezone_type,0,sizeof(TlvItem_t));

        Tlvs_value_resp.type = item_type;
        Tlvs_value_resp.length = 0x04;
//        temp_time = Uint32LtoB(calendar_pt.time_stamps);     //Need to do later...
        Tlvs_value_resp.value = (uint8_t*)&(temp_time);

        item_timezone_type.type = item->brotherNode->type;
        item_timezone_type.length = 0x02;
//        temp_zone = Uint16LtoB(calendar_pt.time_zone);       //Need to do later...
        item_timezone_type.value = (uint8_t*)&(temp_zone);

        //vTaskDelay(3);
        TlvItem_AddBrother(&Tlvs_value_resp,&item_timezone_type);

        SendFramePack(&Tlvs_value_resp,1,6);
    }
}
/******************************************************************************/
/** Get Version Information 2.1.7
  *
  * @param[in] item
  *
  * @return
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/20
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
uint8_t Device_type_AW61 = 0x0c;
void DmsGetVersionInformation(TlvItem_t *item)
{
    uint8_t bt_Version[] = "1.3.45";
    uint8_t device_version[] = "NA";
    uint8_t phone_number[] = "NA";
    uint8_t Device_BT_Mac[] = "AB:CD:EF:34:12:56";//{0};
    uint8_t device_imei[]= "NA";
    //uint8_t Device_SoftVersion[] = "01.02.01";
    uint8_t Device_OpenSource_Version[] = "1.0";
    uint8_t device_sn[] = "NA";
    uint8_t device_model[] = "Gemini";
    uint8_t device_emmc_id[] = "1.0";
    uint8_t device_swversion_length = 0;
    uint8_t ble_mac_length = 0;
    TlvItem_t temp_item_bt_version;
    TlvItem_t temp_item_device_type;
    TlvItem_t temp_item_device_version;
    TlvItem_t temp_item_phone_number;
    TlvItem_t temp_item_mac_addr;
    TlvItem_t temp_device_imei;
    TlvItem_t temp_item_device_soft_version;
    TlvItem_t temp_item_device_opensource_version;
    TlvItem_t temp_item_device_sn;
    TlvItem_t temp_item_device_model;
    TlvItem_t temp_item_device_emmc_id;

    if(NULL == item)
    {
        GeneralErrorACK(GeneralError[6],1,7);
        return;
    }

    device_swversion_length = CharacterConversion(gOTAData.firmware_version,Device_SoftVersion,3);
    memset(&temp_item_bt_version,0,sizeof(TlvItem_t));

    if(NULL != TlvItem_ItemWithType(item,1))
    {
        temp_item_bt_version.type = 1;   //bt version
        temp_item_bt_version.length =  strlen((char*)bt_Version);
        temp_item_bt_version.value = bt_Version;

        if(NULL != TlvItem_ItemWithType(item,2))
        {
            memset(&temp_item_device_type,0,sizeof(TlvItem_t));
            temp_item_device_type.type = 2;
            temp_item_device_type.length = 0x1;
            temp_item_device_type.value = &Device_type_AW61;
            TlvItem_AddBrother(&temp_item_bt_version,&temp_item_device_type);
        }
    }

    else
    {
        if(NULL != TlvItem_ItemWithType(item,2))
        {
            temp_item_bt_version.type = 2;
            temp_item_bt_version.length = 0x1;
            temp_item_bt_version.value = &Device_type_AW61;
        }
    }

    if(NULL != TlvItem_ItemWithType(item,3))
    {
        memset(&temp_item_device_version,0,sizeof(TlvItem_t));
        temp_item_device_version.type = 3;
        temp_item_device_version.length = strlen((char*)device_version);;
        temp_item_device_version.value = device_version;
        TlvItem_AddBrother(&temp_item_bt_version,&temp_item_device_version);
    }

    if(NULL != TlvItem_ItemWithType(item,4))
    {
        memset(&temp_item_phone_number,0,sizeof(TlvItem_t));
        temp_item_phone_number.type = 4;
        temp_item_phone_number.length = strlen((char*)phone_number);
        temp_item_phone_number.value = phone_number;
        TlvItem_AddBrother(&temp_item_bt_version,&temp_item_phone_number);
    }

    if(NULL != TlvItem_ItemWithType(item,5))
    {
        ble_mac_length = GainBleAddress(Device_BT_Mac);
        memset(&temp_item_mac_addr,0,sizeof(TlvItem_t));
        temp_item_mac_addr.type = 5;
        //ble_mac_length = strlen((char*)Device_BT_Mac);
        //printf("ble_mac_length=%d\r\n",ble_mac_length);

        temp_item_mac_addr.length = ble_mac_length;
        temp_item_mac_addr.value = Device_BT_Mac;

        TlvItem_AddBrother(&temp_item_bt_version,&temp_item_mac_addr);
    }

    if(NULL != TlvItem_ItemWithType(item,6))
    {
        memset(&temp_device_imei,0,sizeof(TlvItem_t));
        temp_device_imei.type = 6;
        temp_device_imei.length = strlen((char*)device_imei);;
        temp_device_imei.value = device_imei;
        TlvItem_AddBrother(&temp_item_bt_version,&temp_device_imei);
    }

    if(NULL != TlvItem_ItemWithType(item,7))
    {
        memset(&temp_item_device_soft_version,0,sizeof(TlvItem_t));
        temp_item_device_soft_version.type = 7;
        temp_item_device_soft_version.length = device_swversion_length;//strlen((char*)Device_SoftVersion);
        temp_item_device_soft_version.value = Device_SoftVersion;
        TlvItem_AddBrother(&temp_item_bt_version,&temp_item_device_soft_version);
    }

    if(NULL != TlvItem_ItemWithType(item,8))
    {
        memset(&temp_item_device_opensource_version,0,sizeof(TlvItem_t));
        temp_item_device_opensource_version.type = 8;
        temp_item_device_opensource_version.length = strlen((char*)Device_OpenSource_Version);;
        temp_item_device_opensource_version.value = Device_OpenSource_Version;
        TlvItem_AddBrother(&temp_item_bt_version,&temp_item_device_opensource_version);
    }

    if(NULL != TlvItem_ItemWithType(item,9))
    {
        memset(&temp_item_device_sn,0,sizeof(TlvItem_t));
        temp_item_device_sn.type = 9;
        temp_item_device_sn.length = strlen((char*)device_sn);
        temp_item_device_sn.value = device_sn;
        TlvItem_AddBrother(&temp_item_bt_version,&temp_item_device_sn);
    }

    if(NULL != TlvItem_ItemWithType(item,10))
    {
        memset(&temp_item_device_model,0,sizeof(TlvItem_t));
        temp_item_device_model.type = 10;
        temp_item_device_model.length = strlen((char*)device_model);
        temp_item_device_model.value = device_model;
        TlvItem_AddBrother(&temp_item_bt_version,&temp_item_device_model);
    }

    if(NULL != TlvItem_ItemWithType(item,11))
    {
        memset(&temp_item_device_emmc_id,0,sizeof(TlvItem_t));
        temp_item_device_emmc_id.type = 11;
        temp_item_device_emmc_id.length = strlen((char*)device_emmc_id);
        temp_item_device_emmc_id.value = device_emmc_id;
        TlvItem_AddBrother(&temp_item_bt_version,&temp_item_device_emmc_id);
    }

    SendFramePack(&temp_item_bt_version,1,7);
}
/******************************************************************************/
/** Get Battery PercentInfo 2.1.8
  *
  * @param[in] item_type
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/20
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void DmsGetBatteryPercentInfo(TlvItem_t *item)
{
    uint8_t item_type;
    uint8_t batteryLevelPercent;

    item_type = TlvItem_ItemType(item);
    memset(&Tlvs_value_resp,0,sizeof(TlvItem_t));
    Tlvs_value_resp.type = item_type;
    Tlvs_value_resp.length = 0x01;

    batteryLevelPercent = GetBatteryLevel();//donna

    Tlvs_value_resp.value = (uint8_t*)&(batteryLevelPercent);

    SendFramePack(&Tlvs_value_resp,1,8);
}
/******************************************************************************/
/** Set Auto Light Status 2.1.9
  *
  * @param[in] item_t
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/23
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void DmsSetAutoLightStatus(TlvItem_t *item)
{
    uint32_t temp_length;
    uint8_t auto_light_screen_status;

    if(NULL == item)
    {
        return;
    }

    if(NULL == TlvItem_ItemWithType(item,1))
    {
        GeneralErrorACK(GeneralError[6],1,9);
        return;
    }

    auto_light_screen_status = *TlvItem_ItemValue(item,&temp_length);

    if(auto_light_screen_status == 1)
    {
        g_reming_switch.wrist_light_switch = 1;
        GeneralErrorACK(GeneralError[0],1,9);

    }

    else if(auto_light_screen_status == 0)
    {
        g_reming_switch.wrist_light_switch = 0;
        GeneralErrorACK(GeneralError[0],1,9);
    }

    else
    {
        GeneralErrorACK(GeneralError[7],1,9);
    }
}
/******************************************************************************/
/** New Add Device DisturbInfo  2.1.10
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/20
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void DmsNewAddDeviceDisturbInfo(TlvItem_t *item)
{
//    TlvItem_t  *temp_tlv=NULL;
//    uint8_t *item_value=NULL;

//    memset(&g_avoid_disturb,0,sizeof(FL_Device_Avoid_Disturb));

//    temp_tlv = item->childNode;

//    if(temp_tlv != NULL)
//    {
//        if(g_avoid_disturb.Avoid_Disturb_Valid == 0)
//        {
//            return;
//        }

//        g_avoid_disturb.Avoid_Disturb_Valid = 0x01;

//        item_value = temp_tlv->value;
//        g_avoid_disturb.Avoid_Disturb_Index = *item_value ;

//        temp_tlv = temp_tlv->brotherNode;
//        item_value = temp_tlv->value;
//        g_avoid_disturb.Avoid_Disturb_Switch = *item_value ;

//        temp_tlv = temp_tlv->brotherNode;
//        item_value = temp_tlv->value;
//        g_avoid_disturb.Avoid_Disturb_Type = *item_value;

//        temp_tlv = temp_tlv->brotherNode;
//        item_value = temp_tlv->value;
//        g_avoid_disturb.Avoid_Disturb_Start_Hour = *item_value;
//        g_avoid_disturb.Avoid_Disturb_Start_Minute = *(item_value+1);

//        temp_tlv = temp_tlv->brotherNode;
//        item_value = temp_tlv->value;
//        g_avoid_disturb.Avoid_Disturb_End_Hour = *item_value;
//        g_avoid_disturb.Avoid_Disturb_End_Minute = *(item_value+1);

//        temp_tlv = temp_tlv->brotherNode;
//        item_value = temp_tlv->value;
//        g_avoid_disturb.Avoid_Disturb_Cycle = (*item_value);

//        //xSemaphoreTake(gAppData.semaphoreBtChipTx, 60);
//        SendFramePack(GeneralError[0],1,10);
//    }

//    else
//    {
//        GeneralErrorACK(GeneralError[5],1,10);   //request error
//    }

//    AppScanNoDisturb();
}
/******************************************************************************/
/** Delet Device AvoidDisturb 2.1.11
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/20
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void DmsDeletDeviceAvoidDisturb(TlvItem_t *item)
{
//    TlvItem_t  *temp_tlvs = NULL;

//    temp_tlvs = item->childNode ; //avoid_disturb_index

//    if(temp_tlvs !=  NULL)
//    {
//        memset(&g_avoid_disturb,0,sizeof(FL_Device_Avoid_Disturb));
//        g_avoid_disturb.Avoid_Disturb_Index = *(temp_tlvs->value);

//        GeneralErrorACK(GeneralError[0],1,11);   //success
//    }

//    else
//    {
//        GeneralErrorACK(GeneralError[5],1,11);   //request error
//    }

//    AppScanNoDisturb();
}
/******************************************************************************/
/**  Modify Device AvoidDisturb 2.1.12
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/20
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void DmsModifyDeviceAvoidDisturb(TlvItem_t *item)
{
//    TlvItem_t *temp_tlvs = NULL;
//    uint8_t childnode_type = 0x00;
//    uint8_t *item_value = NULL;

//    memset(&g_avoid_disturb,0,sizeof(FL_Device_Avoid_Disturb));

//    temp_tlvs = item->childNode ; //avoid_disturb_index

//    if(temp_tlvs !=  NULL)
//    {
//        item_value = temp_tlvs->value;//avoid_disturb_index
//        g_avoid_disturb.Avoid_Disturb_Index = *item_value;

//        temp_tlvs = temp_tlvs->brotherNode;//switch,start_time,or endtime,or cycle

//        while(temp_tlvs !=  NULL)
//        {
//            childnode_type= temp_tlvs->type;

//            switch(childnode_type)
//            {
//                case 3:
//                    g_avoid_disturb.Avoid_Disturb_Switch= *(temp_tlvs->value);
//                    break;

//                case 4:
//                    g_avoid_disturb.Avoid_Disturb_Type= *(temp_tlvs->value);
//                    break;

//                case 5 :
//                    g_avoid_disturb.Avoid_Disturb_Start_Hour=(*temp_tlvs->value) ;
//                    g_avoid_disturb.Avoid_Disturb_Start_Minute=(*(temp_tlvs->value+1));
//                    break;

//                case 6:
//                    g_avoid_disturb.Avoid_Disturb_End_Hour=(*temp_tlvs->value) ;
//                    g_avoid_disturb.Avoid_Disturb_End_Minute=(*(temp_tlvs->value+1));
//                    break;

//                case 7:
//                    g_avoid_disturb.Avoid_Disturb_Cycle =*(temp_tlvs->value);
//                    break;

//                default:
//                    break;
//            }

//            g_avoid_disturb.Avoid_Disturb_Valid = 0x01 ;
//            temp_tlvs = temp_tlvs->brotherNode;
//        }

//        GeneralErrorACK(GeneralError[0],1,12);   //success
//    }

//    else
//    {
//        GeneralErrorACK(GeneralError[5],1,12);   //request error
//    }

//    AppScanNoDisturb();
}
/******************************************************************************/
/**  Factory Reset  2.1.13
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/1/7
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void DmsFactoryReset(TlvItem_t *item)
{
    uint8_t  factory_reset;
    uint32_t temp_length;

    if(NULL == item)
    {
        return;
    }

    if(NULL == TlvItem_ItemWithType(item,1))
    {
        GeneralErrorACK(GeneralError[6],1,13);
        return;
    }

    factory_reset = *TlvItem_ItemValue(item,&temp_length);

    if(0x01 == factory_reset)
    {
        GeneralErrorACK(GeneralError[0],1,13);
//        OS_DELAY_MS(200);                          //Need to do later...
//        FactoryReset();                            //Need to do later...
    }

    else
    {
        GeneralErrorACK(GeneralError[7],1,13);
    }
}

/******************************************************************************/
/** BT Bond Require 2.1.14
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/23
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void WirstBandBindResponse(uint8_t enable)
{
    if(0x01 == enable)
    {
        GetAESRecodedKey(bt_bond_key, sizeof(bt_bond_key),g_bt_bond_iv);
        g_Flash_Control_Struct.ble_unpair_flag = 1;
//      WriteContorlBytesToGenericFlash();//donna
    }

    memset(&Tlvs_value_resp,0,sizeof(TlvItem_t));

    Tlvs_value_resp.type = 0x02;
    Tlvs_value_resp.length = 0x01;
    Tlvs_value_resp.value = &enable;    //1:bind enable 0:bind disable

    SendFramePack(&Tlvs_value_resp,1,14);
}

void WristBandBindEnable(uint8_t enable)
{
//    if(1 == enable)
//    {
//        // add for iot
//        g_Flash_Control_Struct.bond_addr[0] = g_bond_addr[0];
//        g_Flash_Control_Struct.bond_addr[1] = g_bond_addr[1];
//        if(0 == BtBindRequest.bt_bind_os) //android
//        {
//            memcpy(g_Flash_Control_Struct.bt_bind_id,BtBindRequest.bt_bind_id,6);
//            g_Flash_Control_Struct.bt_system_flag = 0;
//        }

//        else //ios
//        {
//            OS_TASK_NOTIFY(GetBleTask(), IOS_ACCEPT_PAIR_NOTIFY, OS_NOTIFY_SET_BITS);
//            memset(g_Flash_Control_Struct.bt_bind_id,0,6);
//            g_Flash_Control_Struct.bt_system_flag = 1;
//        }

//        OS_DELAY_MS(1);
//        WirstBandBindResponse(enable);
//        OS_DELAY_MS(1);
//        WirstBandBindResponse(enable);
//    }

//    else
//    {
//        WirstBandBindResponse(enable);
//        OS_DELAY_MS(1);
//        WirstBandBindResponse(enable);

//        ble_gap_disconnect(g_connect_idx, BLE_HCI_ERROR_REMOTE_USER_TERM_CON);
//    }
}

static void DmsBTBondRequire(TlvItem_t *item)
{
//    Msg_t msg;

    uint32_t temp_length = 0;
    uint32_t temp_value = 0;

    TlvItem_t *temp_item;

    temp_item = TlvItem_ItemWithType(item,3);

    if(NULL != temp_item)
    {
        BtBindRequest.bt_bind_os = *TlvItem_ItemValueWithType(item,3,&temp_length);
        g_Flash_Control_Struct.bt_bind_os = BtBindRequest.bt_bind_os;
    }

    temp_item = TlvItem_ItemWithType(item,4);

    if(NULL != temp_item)
    {
        TlvItem_ItemInt32Value(temp_item,&temp_value,1);
        BtBindRequest.bt_bind_feature = temp_value;
    }

    temp_item = TlvItem_ItemWithType(item,6);

    if(NULL != temp_item)
    {
        memcpy(bt_bond_key,temp_item->value,temp_item->length);
    }

    temp_item = TlvItem_ItemWithType(item,7);

    if(NULL != temp_item)
    {
        memcpy(g_bt_bond_iv,temp_item->value,temp_item->length);
    }

    temp_item = TlvItem_ItemWithType(item,5);

    if(NULL == temp_item)
    {
        if((0x01 == BtBindRequest.bt_bind_status) && (0x01 == BtBindRequest.bt_bind_status_info))
        {
            WirstBandBindResponse(1);
//			am_util_delay_ms(500);
        }

        else
        {
//            msg = MSG_TSK_BLE_PAIR;
//            xQueueSend(g_queue_display,&msg,(portTickType)0);
        }
    }

    else  //Android device
    {
        //memcpy(BtBindRequest.bt_bind_id,temp_item->value,6);
        //if(0 == strncmp((char*)g_Flash_Control_Struct.bt_bind_id,(char*)BtBindRequest.bt_bind_id,6))
        memcpy(BtBindRequest.bt_bind_id,temp_item->value,6);

        if(0 == strncmp((char*)g_Flash_Control_Struct.bt_bind_id,(char*)BtBindRequest.bt_bind_id,6))    
        {
            WirstBandBindResponse(1);
//			am_util_delay_ms(500);
        }

        else
        {
//            msg = MSG_TSK_BLE_PAIR;
//            xQueueSend(g_queue_display,&msg,(portTickType)0);

            //memcpy(BtBindRequest.bt_bind_id,ble_bind_id,6);
            //WirstBandBindResponse(1);
			WirstBandBindResponse(1);
//			am_util_delay_ms(500);
        }
    }
}
/******************************************************************************/
/** GetBtBindStatusInfo 2.1.15
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/23
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
void MCUFeedBackBindStatus(void)
{
    uint8_t Band_Firmware_Version = 0;
    uint8_t ble_mac_length = 0;
    uint8_t Device_BT_Mac[] = "80:EA:CA:88:77:77";
    uint32_t bt_cmd_cnt = 0;
	
    TlvItem_t temp_item;
    TlvItem_t temp_item_bond_version;
    TlvItem_t temp_item_bt_service_mtu;
    TlvItem_t temp_item_mac_addr;
    TlvItem_t temp_item_bt_cmd_cnt;

    uint16_t  mtu_length;
    uint16_t  temp_mtu_length;

    memset(&Tlvs_value_resp,0,sizeof(TlvItem_t));
    Tlvs_value_resp.type = 1;
    Tlvs_value_resp.length = 0x1;
    Tlvs_value_resp.value = &BtBindRequest.bt_bind_status;

    memset(&temp_item,0,sizeof(TlvItem_t));
    temp_item.type = 2;
    temp_item.length = 0x1;
    temp_item.value = &BtBindRequest.bt_bind_status_info;
    TlvItem_AddBrother(&Tlvs_value_resp,&temp_item);

#if AES_ENCRYPT_ENABLE
    memset(&temp_item_bond_version,0,sizeof(TlvItem_t));
    temp_item_bond_version.type = 4;
    temp_item_bond_version.length = 0x1;
    Band_Firmware_Version = 2;
    temp_item_bond_version.value = &Band_Firmware_Version;
    TlvItem_AddBrother(&Tlvs_value_resp,&temp_item_bond_version);
#else
    memset(&temp_item_bond_version,0,sizeof(TlvItem_t));
    temp_item_bond_version.type = 4;
    temp_item_bond_version.length = 0x1;
    Band_Firmware_Version = 1;
    temp_item_bond_version.value = &Band_Firmware_Version;
    TlvItem_AddBrother(&Tlvs_value_resp,&temp_item_bond_version);
#endif
    mtu_length = MTU_MAX_LENGTH;
    memset(&temp_item_bt_service_mtu,0,sizeof(TlvItem_t));
    temp_item_bt_service_mtu.type = 5;
    temp_item_bt_service_mtu.length = 0x2;
    temp_mtu_length = (mtu_length & 0xff00)>>8 | (mtu_length & 0xff) << 8;
    temp_item_bt_service_mtu.value = (uint8_t*)(&temp_mtu_length);
    TlvItem_AddBrother(&Tlvs_value_resp,&temp_item_bt_service_mtu);

    ble_mac_length = GainBleAddress(Device_BT_Mac);
    memset(&temp_item_mac_addr,0,sizeof(TlvItem_t));
    temp_item_mac_addr.type = 8;
    temp_item_mac_addr.length = ble_mac_length;
    temp_item_mac_addr.value = Device_BT_Mac;
    TlvItem_AddBrother(&Tlvs_value_resp,&temp_item_mac_addr);

#if AES_ENCRYPT_ENABLE
    memset(&temp_item_bt_cmd_cnt,0,sizeof(TlvItem_t));
    bt_cmd_cnt = 0xf3;
    bt_cmd_cnt = Uint32LtoB(bt_cmd_cnt);
    temp_item_bt_cmd_cnt.type = 9;
    temp_item_bt_cmd_cnt.length = 0x04;
    temp_item_bt_cmd_cnt.value = (uint8_t *)&bt_cmd_cnt;
    TlvItem_AddBrother(&Tlvs_value_resp,&temp_item_bt_cmd_cnt);
#endif
    SendFramePack(&Tlvs_value_resp,1,15);
}

static void DmsGetBtBindStatusInfo(TlvItem_t *item)
{
    TlvItem_t *temp_item;
//    Msg_t msg;

    if(NULL == item)
    {
        return;
    }

    temp_item = TlvItem_ItemWithType(item,3);

    if(NULL == temp_item)  //IOS device
    {
        if((0x2 == BtBindRequest.bt_bind_status) && (0x4 == BtBindRequest.bt_bind_status_info))
        {
            BtBindRequest.bt_bind_status = 0;
            BtBindRequest.bt_bind_status_info = 3;
        }

        else
        {
            /*the upair flag is for factory reset,the ble bond area is not erase*/
            if((0x00 == g_Flash_Control_Struct.bt_system_flag)||(g_Flash_Control_Struct.ble_unpair_flag == 0xD3/*BLE_UNPAIR_FLAG*/))
            {
                BtBindRequest.bt_bind_status = 0;
                BtBindRequest.bt_bind_status_info = 2;
            }
        }

        BtBindRequest.bt_bond_version_flag = 0;
        MCUFeedBackBindStatus();
        //BtBindRequest.bt_bind_os = 1;
    }

    else
    {
        //memcpy(BtBindRequest.bt_bind_id,temp_item->value,6);
        memcpy(BtBindRequest.bt_bind_id,temp_item->value,6);

        temp_item = TlvItem_ItemWithType(item,4);

        if(NULL != temp_item)
        {
            BtBindRequest.bt_bond_version = *(temp_item->value);
            BtBindRequest.bt_bond_version_flag = 1;
        }

        //if(0 == strncmp((char*)g_Flash_Control_Struct.bt_bind_id,(char*)BtBindRequest.bt_bind_id,6))
        if(0 == strncmp((char*)g_Flash_Control_Struct.bt_bind_id,(char*)BtBindRequest.bt_bind_id,6))
        {
            BtBindRequest.bt_bind_status = 1;
            BtBindRequest.bt_bind_status_info = 1;
            /*ble connect icon*/
//            gUIContext.ble_connect = TRUE;
//            msg = MSG_TSK_REFRESH_TIME;
//            xQueueSend(g_queue_display,&msg,(portTickType)10);
        }

        else
        {
            BtBindRequest.bt_bind_status = 0;
            BtBindRequest.bt_bind_status_info = 3;
        }

        MCUFeedBackBindStatus();
    }
    //add for iot
    temp_item = TlvItem_ItemWithType(item, 0x07);
    if(NULL != temp_item)
    {
        uint8_t addr_idx =0;
        uint8_t tmep_bt_addr[MAX_BOND_MAC_LEN] = {0};
        memcpy(BtBindRequest.bt_bond_mac, temp_item->value, MAX_BOND_MAC_LEN);
//        for(uint8_t i=0;i< temp_item->length;i++)
//        {
//            printf("data=%x\r\n",temp_item->value[i]);
//        }
        for(addr_idx = 0; addr_idx < MAX_BOND_MAC_LEN; addr_idx++)
        {
            if(BtBindRequest.bt_bond_mac[addr_idx] >= 0x30 && BtBindRequest.bt_bond_mac[addr_idx] <= 0x39)
            {
                tmep_bt_addr[addr_idx] = BtBindRequest.bt_bond_mac[addr_idx] - 0x30;    /*0-9*/
            }
            else if(BtBindRequest.bt_bond_mac[addr_idx] >= 0x41 && BtBindRequest.bt_bond_mac[addr_idx] <= 0x46)
            {
                tmep_bt_addr[addr_idx] = BtBindRequest.bt_bond_mac[addr_idx] - 0x37;    /*A-F*/
            }
            else if(BtBindRequest.bt_bond_mac[addr_idx] >= 0x61 && BtBindRequest.bt_bond_mac[addr_idx] <= 0x66)
            {
                tmep_bt_addr[addr_idx] = BtBindRequest.bt_bond_mac[addr_idx] - 0x57;    /*a-f*/
            }
            else
            {
                tmep_bt_addr[addr_idx] = 0xFF;
            }
        }

        g_bond_addr[0] = tmep_bt_addr[12] << 4 | tmep_bt_addr[13];
        g_bond_addr[1] = tmep_bt_addr[15] << 4 | tmep_bt_addr[16];
//        printf("BT bond status a=%x,b=%x\r\n",g_bond_addr[0],g_bond_addr[1]);

    }
    else
    {
        //for ios is null
        memset(g_bond_addr, 0xFF, sizeof(g_bond_addr));
//        printf("0xff 0xff...\r\n");
    }

    if ((1 == BtBindRequest.bt_bind_status))
        //&& (0x0 == g_Flash_Control_Struct.bond_addr[0])
        //&& (0x0 == g_Flash_Control_Struct.bond_addr[1]))
    {
        g_Flash_Control_Struct.bond_addr[0] = g_bond_addr[0];
        g_Flash_Control_Struct.bond_addr[1] = g_bond_addr[1];
//        printf("set the flashflag a=%x,b=%x\r\n",g_Flash_Control_Struct.bond_addr[0],g_Flash_Control_Struct.bond_addr[1]);
    }
}
/******************************************************************************/
/** Notify Interval  2.1.17
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/1/7
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void DmsNotifyInterval(void)
{
    uint16_t interval = 0;
    uint16_t temp_interval = 0;

    memset(&Tlvs_value_resp,0,sizeof(TlvItem_t));
    Tlvs_value_resp.type = 0x01;
    Tlvs_value_resp.length = 0x02;

    temp_interval = Uint16LtoB(interval);
    Tlvs_value_resp.value = (uint8_t*)&(temp_interval);

    SendFramePack(&Tlvs_value_resp,1,17);
}
/******************************************************************************/
/** Get Motion Type   2.1.18
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/1/5
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void DmsGetMotionType(void)
{
    uint8_t i = 0;
    uint8_t supported_activity_type[5]= {0x04,0x07,0x09,0x0A,0x0B};
    //uint8_t avtivity_data_type[5]= {0x0E,0x0E,0x0C,0x05,0x05};
    uint8_t avtivity_data_type[5]= {0x4E,0x4E,0x4C,0x25,0x25};

    TlvItem_t item_supported_activity_type[5];
    TlvItem_t item_avtivity_data_type[5];

    memset(&Tlvs_value_resp,0,sizeof(TlvItem_t));
    Tlvs_value_resp.type = 1 | CHILD_MASK;
    Tlvs_value_resp.length = 0;

    for(i = 0; i < 5 ; i++)
    {
        memset(&item_supported_activity_type[i],0,sizeof(TlvItem_t));
        item_supported_activity_type[i].type = 2;
        item_supported_activity_type[i].length = 0x01;
        item_supported_activity_type[i].value = &supported_activity_type[i];
        TlvItem_AddChild(&Tlvs_value_resp,&item_supported_activity_type[i]);

        memset(&item_avtivity_data_type[i],0,sizeof(TlvItem_t));
        item_avtivity_data_type[i].type = 3;
        item_avtivity_data_type[i].length = 0x01;
        item_avtivity_data_type[i].value = &avtivity_data_type[i];
        TlvItem_AddChild(&Tlvs_value_resp,&item_avtivity_data_type[i]);
    }

    SendFramePack(&Tlvs_value_resp,1,0x12);
}
/******************************************************************************/
/** DmsLegalApprove 2.1.19
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/1/21
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void DmsLegalApprove(TlvItem_t *item)
{
    TlvItem_t *temp_item;

    uint8_t Rand[32] = {0x55,0xa1,0xe5,0xfa,0x11,0x99,0x13,0x26,
                        0xae,0x8b,0xd9,0xe6,0x3b,0x6c,0x57,0xb7
                       };
#if 0
    uint8_t CAK[] = {0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
                     0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
                     0x01,0x10
                    };
#else
    uint8_t CAK[] =
    {
        0x93,0xAC,0xDE,0xF7,0x6A,0xCB,0x09,0x85,
        0x7D,0xBF,0xE5,0x26,0x1A,0xAB,0xCD,0x78,
        0x01,0x10
    };
#endif
    uint8_t MK[32] = {0};
    uint8_t Token[32] = {0};

    if(NULL == item)
    {
        return;
    }

    temp_item = TlvItem_ItemWithType(item,2);

    if(NULL != temp_item)
    {
        memcpy(Rand+16,temp_item->value+2,16);
    }

    Hmac hmac;
    HmacSetKey(&hmac, SHA256, CAK, sizeof(CAK));
    HmacUpdate(&hmac, Rand, sizeof(Rand));
    HmacFinal(&hmac, MK);

    HmacSetKey(&hmac, SHA256, MK, sizeof(MK));
    HmacUpdate(&hmac, Rand, sizeof(Rand));
    HmacFinal(&hmac, Token);

    memset(&Tlvs_value_resp,0,sizeof(TlvItem_t));
    Tlvs_value_resp.type = 0x01;
    Tlvs_value_resp.length = sizeof(Token);
    Tlvs_value_resp.value = Token;

    SendFramePack(&Tlvs_value_resp,0x01,0x13);
}
/******************************************************************************/
/** DmsGetGoldCardMember
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/3/4
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void DmsGetGoldCardMember(void)
{
    uint8_t gold_card_member = 0;

    memset(&Tlvs_value_resp,0,sizeof(TlvItem_t));
    Tlvs_value_resp.type = 0x01;
    Tlvs_value_resp.length = 0x01;
    Tlvs_value_resp.value = &gold_card_member;

    SendFramePack(&Tlvs_value_resp,1,0x14);
}
/******************************************************************************/
/** Notify Open App 2.1.20
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/1/7
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void DmsNotifyOpenApp(void)
{
    uint8_t device_open_app = 0;

    memset(&Tlvs_value_resp,0,sizeof(TlvItem_t));
    Tlvs_value_resp.type = 0x01;
    Tlvs_value_resp.length = 0x01;
    Tlvs_value_resp.value = &device_open_app;

    SendFramePack(&Tlvs_value_resp,1,21);
}
/******************************************************************************/
/** Device Management Service 2.1
  *
  * @param[in] item_t
  * @param[in] command_id
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/19
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void DeviceManagerServiceType(TlvItem_t *item_t,uint8_t command_iden)
{
    DMS_COMMAND_ID_E command_id;
    command_id = (DMS_COMMAND_ID_E)command_iden;

    switch(command_id)
    {
        case DMS_CMD_CONNECT_PARA:
            /**2.1.1 */
            DmsConnectParaNegotiate(item_t);
            break;

        case DMS_CMD_SERVIVE_CAPABILITY:
            /**2.1.2*/
            DmsServiveCapability(item_t);
            break;

        case DMS_CMD_COMMAND_CAPABILITY:
            /**2.1.3 */
            DmsCommandCapability(item_t);
            break;

        case DMS_CMD_DATETIME_FORMAT:
            /**2.1.4*/
            DmsSetDateTimeFormat(item_t);
            break;

        case DMS_CMD_SET_DATETIME:
            /**2.1.5 */
            DmsSetDeviceTime(item_t);
            break;

        case DMS_CMD_GET_DATETIME:
            /**2.1.6*/
            DmsGetDeviceTimeInfo(item_t);
            break;

        case DMS_CMD_GET_VERSION:
            /**2.1.7*/
            DmsGetVersionInformation(item_t);
            break;

        case DMS_CMD_GET_BATTERY:
            /**2.1.8*/
            DmsGetBatteryPercentInfo(item_t);
            break;

        case DMS_CMD_SET_AUTO_LIGNT_STATUS:
            /**2.1.9 */
            DmsSetAutoLightStatus(item_t);
            break;

        case DMS_CMD_SET_AVOID_DISTURB:
            /**2.1.10 */
            DmsNewAddDeviceDisturbInfo(item_t);        //Need to do later...
            break;

        case DMS_CMD_DEL_AVOID_DISTURB:
            /**2.1.11 */
            DmsDeletDeviceAvoidDisturb(item_t);        //Need to do later...
            break;

        case DMS_CMD_MODIFY_AVOID_DISTURB:
            /**2.1.12 */
            DmsModifyDeviceAvoidDisturb(item_t);       //Need to do later...
            break;

        case DMS_CMD_FACTORY_RESET:
            /**2.1.13 */
            DmsFactoryReset(item_t);
            break;

        case DMS_CMD_REQ_BT_BOND:
            /**2.1.14 */
            DmsBTBondRequire(item_t);
            break;

        case DMS_CMD_GET_BT_BOND_STATUS:
            /**2.1.15 */
            DmsGetBtBindStatusInfo(item_t);
            break;

        case DMS_CMD_REQ_HANDSET_INFO:
            /**2.1.16 */
            break;

        case DMS_CMD_NOTIFY_INTERVAL:
            /**2.1.17 */
            DmsNotifyInterval();
            break;

        case DMS_CMD_GET_MOTION_TYPE:
            /**2.1.18 */
            DmsGetMotionType();
            break;

        case DMS_CMD_GET_LEGITIMACY:
            /**2.1.19 */
            DmsLegalApprove(item_t);
            break;

        case DMS_CMD_GET_GOLD_CARD:

            DmsGetGoldCardMember();
            break;


        case DMS_CMD_NOTIFY_OPEN_APP:
            /**2.1.20*/
            DmsNotifyOpenApp();
            break;

        default:
            break;
    }
}
static void NsHandleTextType(MSG_NODE_T *message_node, TEXT_NOTIFICATION_T *text_struct, uint8_t message_type)
{
//    uint8_t unknown_app[6] = {0xE6,0xB6,0x88,0xE6,0x81,0xAF};
//    uint8_t unknown_app_en[7] = {'M','e','s','s','a','g','e'};
//    switch(message_type)
//    {
//        case INCOMING_REMINDER:
//            {
//                if(text_struct->text_content_length > 25)
//                {
//                    gUIContext.incoming_call_length = 25;
//                }

//                else
//                {
//                    gUIContext.incoming_call_length = text_struct->text_content_length;
//                }

//                memcpy(gUIContext.incoming_call_title,text_struct->text_content, (gUIContext.incoming_call_length>25)?25:gUIContext.incoming_call_length);
//            }
//            break;

//        case ANSWER_MISSCALL:
//            {
//                if(text_struct->text_content_length > 25)
//                {
//                    gUIContext.missed_call_length = 25;
//                }

//                else
//                {
//                    gUIContext.missed_call_length = text_struct->text_content_length;
//                }

//                memcpy(gUIContext.missed_call_title, text_struct->text_content, (gUIContext.missed_call_length>25)?25:gUIContext.missed_call_length);
//            }
//            break;

//        case SMS_REMINDER:
//        case WECHAT_REMINDER:
//        case QQ_REMINDER:
//        case EMAIL_NOTIFY:

//            if(0x01 == text_struct->text_type)  // content, max length=90
//            {
//                if(text_struct->text_content_length > MSG_LENGTH)
//                {
//                    message_node->msg_length = MSG_LENGTH;
//                }

//                else
//                {
//                    message_node->msg_length = text_struct->text_content_length;
//                }

//                memcpy(message_node->msg_buf, text_struct->text_content, (message_node->msg_length>MSG_LENGTH)?MSG_LENGTH:message_node->msg_length);
//            }

//            else if(0x02 == text_struct->text_type) // contact, max length=25
//            {
//                if(text_struct->text_content_length > 25)
//                {
//                    message_node->contact_length = 25;
//                }

//                else
//                {
//                    message_node->contact_length = text_struct->text_content_length;
//                }

//                memcpy(message_node->contact, text_struct->text_content, (message_node->contact_length>25)?25:message_node->contact_length);
//            }

//            else if(0x03 == text_struct->text_type)  //title, max length=25
//            {
//                if(text_struct->text_content_length > 25)
//                {
//                    message_node->title_length = 25;
//                }

//                else
//                {
//                    message_node->title_length = text_struct->text_content_length;
//                }

//                memcpy(message_node->title, text_struct->text_content, (message_node->title_length>25)?25:message_node->title_length);
//            }

//            break;

//        default:
//            break;
//    }

//    if(((LONG_SIT_REMINDER <= message_type) && (WEATHER_REMINDER >= message_type))
//            ||(message_type > INCOMING_MUTE_REMINDER))
//    {
//        switch(text_struct->text_type)
//        {
//            case 0x1://message content

//                if(text_struct->text_content_length > MSG_LENGTH)
//                {
//                    message_node->msg_length = MSG_LENGTH;
//                }

//                else
//                {
//                    message_node->msg_length = text_struct->text_content_length;
//                }

//                memcpy(message_node->msg_buf, text_struct->text_content, (message_node->msg_length>MSG_LENGTH)?MSG_LENGTH:message_node->msg_length);

//                break;

//            case 0x2://title
//            case 0x3://contact

//                if(text_struct->text_content_length > 25)
//                {
//                    message_node->contact_length = 25;
//                }
//                /*if the contact or title length is null,the set the content to default:"mseeage"*/
//                else if(text_struct->text_content_length == 0)
//                {
//                    if(strncmp((char *)gUIContext.PhoneLanguage,"zh-CN",2) == 0)
//                    {
//                        message_node->contact_length = 6;
//                        memcpy(text_struct->text_content,&unknown_app[0],6);
//                    }
//                    else
//                    {
//                        message_node->contact_length = 7;
//                        memcpy(text_struct->text_content,&unknown_app_en[0],7);
//                    }
//                }
//                else
//                {
//                    message_node->contact_length= text_struct->text_content_length;
//                }

//                memcpy(message_node->contact, text_struct->text_content, (message_node->contact_length>25)?25:message_node->contact_length);

//                break;

//            default:
//                break;
//        }
//    }
}
/******************************************************************************/
/** 2.2.1 Set Server Massage Remind Infomation
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/28
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void NsSetServerMsgRemindInfo(TlvItem_t *item)
{
//    Msg_t msg;
//    TlvItem_t *child_item = NULL;
//    TlvItem_t *brother_item = NULL;
//    TlvItem_t *temp_item = NULL;
//    uint32_t length = 0;
//    uint8_t  text_id = 0;
//    MSG_NODE_T *new_msg_node;

//    new_msg_node = (MSG_NODE_T*)pvPortMalloc(sizeof(MSG_NODE_T));

//    if(new_msg_node == NULL)
//    {
//        return;
//    }

//    for(text_id = 0; text_id < 3; text_id++)
//    {
//        memset(&g_message_push.g_dot_metrix[text_id],0,sizeof(DOT_METRIX_T));
//        memset(&g_message_push.g_message_text[text_id],0,sizeof(TEXT_NOTIFICATION_T));
//    }

//    memset(new_msg_node, 0, sizeof(MSG_NODE_T));

//    TlvItem_ItemInt16Value(item,&g_message_push.g_message_id,1);

//    brother_item = TlvItem_Next(item); //point to message_type struct
//    g_message_push.g_message_type = *TlvItem_ItemValue(brother_item,&length);
//    new_msg_node->msg_type = g_message_push.g_message_type;

//    brother_item = TlvItem_Next(brother_item);  //point to motor enable
//    g_message_push.g_motor_enable = *TlvItem_ItemValue(brother_item,&length);

//    brother_item = TlvItem_Next(brother_item);  //point to message content struct

//    child_item = TlvItem_ItemWithType(brother_item,0x05); //point to dot_metrix_list

//    if(NULL != child_item)
//    {
//        text_id = 0;

//        child_item = TlvItem_ItemWithType(brother_item,0x06); //point to dot metrix struct

//        while(NULL != child_item)
//        {
//            temp_item = TlvItem_ItemWithType(child_item,0x07);

//            if(NULL != temp_item)
//            {
//                g_message_push.g_dot_metrix[text_id].dot_metrix_type = *TlvItem_ItemValue(temp_item,&length);
//            }

//            temp_item = TlvItem_ItemWithType(child_item,0x08);

//            if(NULL != temp_item)
//            {
//                TlvItem_ItemInt16Value(temp_item,&g_message_push.g_dot_metrix[text_id].dot_metrix_height,1);
//            }

//            temp_item = TlvItem_ItemWithType(child_item,0x09);

//            if(NULL != temp_item)
//            {
//                TlvItem_ItemInt16Value(temp_item,&g_message_push.g_dot_metrix[text_id].dot_metrix_width,1);
//            }

//            temp_item = TlvItem_ItemWithType(child_item,0x0A);
//            {
//                g_message_push.g_dot_metrix[text_id].dot_metrix_color = *TlvItem_ItemValue(temp_item,&length);
//            }

//            temp_item = TlvItem_ItemWithType(child_item,0x0B);
//            {
//                g_message_push.g_dot_metrix[text_id].dot_metrix_data = TlvItem_ItemValue(temp_item,&length);
//            }
//            text_id++;

//            child_item =  TlvItem_Next(child_item);

//            //
//            if(text_id >= 3)
//            {
//                text_id = 0;
//            }
//        }
//    }

//    child_item = TlvItem_ItemWithType(brother_item,0x0C); //point to text_list

//    if(NULL != child_item)
//    {
//        text_id = 0;
//        child_item = TlvItem_ItemWithType(brother_item,0x0D); //point to text_struct

//        while(NULL != child_item)
//        {
//            temp_item = TlvItem_ItemWithType(child_item,0x0E); //point to text_type

//            if(NULL != temp_item)
//            {
//                g_message_push.g_message_text[text_id].text_type = *TlvItem_ItemValue(temp_item,&length);
//            }

//            temp_item = TlvItem_ItemWithType(child_item,0x0F); //point to text_format

//            if(NULL != temp_item)
//            {
//                g_message_push.g_message_text[text_id].text_format = *TlvItem_ItemValue(temp_item,&length);
//            }

//            temp_item = TlvItem_ItemWithType(child_item,0x10); //point to text_content

//            if(NULL != temp_item)
//            {
//                g_message_push.g_message_text[text_id].text_content_length = temp_item->length;
//                g_message_push.g_message_text[text_id].text_content = TlvItem_ItemValueWithType(temp_item,0x10,&length);

//                if(g_message_push.g_message_text[text_id].text_content != NULL)
//                {
//                    NsHandleTextType(new_msg_node, &g_message_push.g_message_text[text_id], g_message_push.g_message_type);
//                }
//            }

//            text_id++;
//            g_message_push.g_total_text_msg_num = text_id;
//            child_item =  TlvItem_Next(child_item);

//            //
//            if(text_id >= 3)
//            {
//                text_id = 0;
//            }
//        }
//    }

//#if 1

//    if(((SMS_REMINDER == g_message_push.g_message_type) ||(WECHAT_REMINDER == g_message_push.g_message_type)
//            ||(QQ_REMINDER == g_message_push.g_message_type) ||(EMAIL_NOTIFY == g_message_push.g_message_type))
//            ||((LONG_SIT_REMINDER <= g_message_push.g_message_type) && (WEATHER_REMINDER >= g_message_push.g_message_type))
//            ||(g_message_push.g_message_type > INCOMING_MUTE_REMINDER))
//#else
//    if((SMS_REMINDER == g_message_push.g_message_type) ||(WECHAT_REMINDER == g_message_push.g_message_type)
//            ||(QQ_REMINDER == g_message_push.g_message_type))
//#endif
//    {

//        //if((g_reming_switch.bnotification_enable)&&(!g_ndb_info.ndb_message))

//        fifo_add(&g_diaply_msg.message_fifo, new_msg_node);

//        //lambert:add for alarm  and incoming call motor close by message@20170403
//        // printf(">>>>gUIContext->current_state.state = [%d]\n\r",gUIContext.current_state->last_state);
//        if((!g_ndb_info.ndb_message)&&(UsbAttachOrNot() == 0) \
//                && (STATE_ALARM != gUIContext.current_state->state) \
//                && (STATE_INCOMING_CALL != gUIContext.current_state->state))
//        {
//            // printf(">>>>>> from protocol to MOTOR_CONTROL_VIBRATE\n\r");
//            msg = MOTOR_CONTROL_VIBRATE;
//            OS_QUEUE_PUT(g_motor_queue,&msg,10);
//        }

//        else if(gUIContext.current_state->last_state == STATE_MISS_CALL)
//        {

//            // printf(">>>>>> from protocol to MOTOR_CONTROL_VIBRATE\n\r");
//            msg = MOTOR_CONTROL_VIBRATE;
//            OS_QUEUE_PUT(g_motor_queue,&msg,10);
//        }

//        msg = MSG_TSK_IT_MESSAGE_NOTIFICATION;
//        xQueueSend(g_queue_display,&msg,(portTickType)10);
//    }

//    else
//    {
//        vPortFree(new_msg_node);/**<Free the message struct */

//        if(INCOMING_REMINDER == g_message_push.g_message_type)
//        {
//            //if((g_reming_switch.bincoming_call_enable)&&(!g_ndb_info.ndb_incoming_call))
//            if(!g_ndb_info.ndb_incoming_call)
//            {
//                gUIContext.incoming_call_set_flag = 1;
//                msg = MSG_TSK_IT_CALL_INCOMING;
//                xQueueSend(g_queue_display,&msg,(portTickType)10);
//            }
//        }

//        else if(HANGUP_INCOMINGCALL == g_message_push.g_message_type)
//        {
//            msg = MSG_TSK_HANGUP_INCOMINGCALL;
//            xQueueSend(g_queue_display,&msg,(portTickType)10);
//        }

//        else if(ANSWER_INCOMINGCALL == g_message_push.g_message_type)
//        {
//            msg = MSG_TSK_HANGUP_INCOMINGCALL;
//            xQueueSend(g_queue_display,&msg,(portTickType)10);
//        }

//        else if(ANSWER_MISSCALL == g_message_push.g_message_type)
//        {
//            g_reming_switch.miss_call_flag = 1;
//            msg = MSG_TSK_IT_CALL_MISSED;
//            xQueueSend(g_queue_display,&msg,(portTickType)10);

//        }

//        else
//        {
//            //msg = MSG_TSK_IT_NOTIFICATION;
//        }
//    }

//    //if (msg != MSG_TSK_UI_NOTIFICATION)
//    //{
//    //gDisplay.msg_timeout_flag = 0; /*clear flag, show the message when OLED is on*/       //@Bill Fix Bug 8994
//    //xQueueSend(gAppData.queueDisplay, &msg, 0);
//    //}

//    GeneralErrorACK(GeneralError[0],2,1);

}
/******************************************************************************/
/**  Get Restriction ListInfo  2.2.2
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/26
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void NsGetRestrictionListInfo(void)
{
    uint8_t cur_data_length;
    uint8_t  structure[2];

    structure[0] = 0x02;
    structure[1] = 0x02;

    cur_data_length = sizeof(infor_restriction);
    /*    ItemStr = (uint8_t*)pvPortMalloc(sizeof(infor_restriction));


        if(NULL == ItemStr)
        {
            log_printf(LOG_NOTICE, 3, "cur_data_length: %d\r\n",cur_data_length);
            return;
        }

        memcpy(ItemStr,infor_restriction,sizeof(infor_restriction));
     */

    LogicAppSendPacket(structure,2,infor_restriction,cur_data_length);
    /*    if(NULL != ItemStr)
        {
            vPortFree(ItemStr);
            ItemStr = NULL;
        }*/
}
/******************************************************************************/
/** Setting Msg Remind Switch Status 2.2.3
  *
  * @param[in] item
  *
  * @return
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/23
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
void NsSetMsgRemindSwitchStatus(TlvItem_t *item)
{
    TlvItem_t *temp_item_child;

    if(NULL == item)
    {
        return;
    }

    temp_item_child = TlvItem_ItemWithType(item,2);

    if(NULL != TlvItem_ItemWithType(item,2))
    {
        g_reming_switch.bnotification_enable = *(temp_item_child->value);

        if((0 != g_reming_switch.bnotification_enable) && (0x01 != g_reming_switch.bnotification_enable))
        {
            GeneralErrorACK(GeneralError[6],2,4);
            return;
        }
    }

    temp_item_child = TlvItem_ItemWithType(item,3);

    if(NULL != temp_item_child)
    {
        g_reming_switch.bincoming_call_enable = *(temp_item_child->value);

        if((0 != g_reming_switch.bincoming_call_enable) && (0x1 != g_reming_switch.bincoming_call_enable))
        {
            GeneralErrorACK(GeneralError[6],2,4);  //parameter error
            return;
        }
    }

    GeneralErrorACK(GeneralError[0],2,4);
}
/******************************************************************************/
/** Query Device Support Type 2.2.4
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/23
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void NsQueryDeviceSupportType(TlvItem_t *item)
{
    uint8_t item_type;
    uint8_t support_type;


    item_type = TlvItem_ItemType(item);
    memset(&Tlvs_value_resp,0,sizeof(TlvItem_t));
    Tlvs_value_resp.type = item_type;
    Tlvs_value_resp.length = 0x01;
    support_type = 0x01;
    Tlvs_value_resp.value = (uint8_t*)(&support_type);

    SendFramePack(&Tlvs_value_resp,2,5);
}
/******************************************************************************/
/** Notification Service Type 2.2
  *
  * @param[in] item_t
  * @param[in] command_id
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/19
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void NotificationServiceType(TlvItem_t *item_t,uint8_t command_iden)
{
    NS_COMMAND_ID_E command_id;
    command_id = (NS_COMMAND_ID_E)command_iden;

    switch(command_id)
    {
        case NS_CMD_SET_SERVE_REMINDER:

            /**2.2.1 */
            if(0x00 == gOTAData.ota_on_going)
            {
                NsSetServerMsgRemindInfo(item_t);    //Need to do later...
            }

            break;

        case NS_CMD_GET_SERVE_REMINDER:
            /**2.2.2 */
            NsGetRestrictionListInfo();
            break;

        case NS_CMD_SET_SWITCH_STATE:
            /**2.2.3 */
            NsSetMsgRemindSwitchStatus(item_t);
            break;

        case NS_CMD_QUERY_SUPPORT_TYPE:
            /**2.2.4 */
            NsQueryDeviceSupportType(item_t);
            break;

        default:
            break;
    }
}
/******************************************************************************/
/** Caller Appear 2.3.1
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/1/7
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
void HcsCallerAppear(uint8_t calling_operation_type)
{
    memset(&Tlvs_value_resp,0,sizeof(TlvItem_t));
    Tlvs_value_resp.type = 0x01;
    Tlvs_value_resp.length = 0x01;
    Tlvs_value_resp.value = &calling_operation_type;

    SendFramePack(&Tlvs_value_resp,4,1);
}
/******************************************************************************/
/** Hands Free Call Service Type 2.3
  *
  * @param[in] item_t
  * @param[in] command_id
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/19
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void HandsFreeCallServiceType(TlvItem_t *item_t,uint8_t command_id)
{
    switch(command_id)
    {
        case HCS_CMD_CALLER_APPEAR:
            /**2.3.1 */
            //HcsCallerAppear(1);
            break;

        default:
            break;
    }
}
/******************************************************************************/
/**  Set Motion TargetInfo 2.4.1
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/20
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
typedef struct
{
    uint8_t  goal_type;
    uint8_t  motion_type;
    uint16_t goal_time;
    uint32_t goal_step;
    uint32_t goal_calorie;
    uint32_t goal_distance;
}MOTION_GOAL_T;                         // Added for test,need to delete later 
MOTION_GOAL_T g_motion_goal[3];         // Added for test,need to delete later

uint8_t g_day_goal_flag;                // Added for test,need to delete later 
uint8_t g_first_goal_step;              // Added for test,need to delete later 


extern uint8_t g_day_goal_flag;
extern uint8_t g_first_goal_step;//donna
static void FitSetMotionTargetInfo(TlvItem_t *item)
{
    uint32_t temp_value = 0;
    uint32_t length = 0;
    TlvItem_t *childitem = NULL;
    TlvItem_t *childchilditem = NULL;
    TlvItem_t *brotheritem = NULL;
    TlvItem_t *tempitem = NULL;

    if(NULL == item)
    {
        return;
    }

    childitem = TlvItem_Child(item);  //point to goal_struct item

    childchilditem = TlvItem_Child(childitem); //point to goal type item
    g_motion_goal[0].goal_type = *TlvItem_ItemValue(childchilditem,&length);

    if(1 != g_motion_goal[0].goal_type)
    {
        GeneralErrorACK(GeneralError[6],7,1);  //parameter error
        return;
    }

    brotheritem = TlvItem_Next(childchilditem);  // point to motion type item
    g_motion_goal[0].motion_type = *TlvItem_ItemValue(brotheritem,&length);
    tempitem = TlvItem_ItemWithType(brotheritem,5);

    if(NULL != tempitem)
    {
        TlvItem_ItemInt32Value(tempitem,&temp_value,1);

        if((g_first_goal_step == 1) && (g_Flash_Control_Struct.goal_step_before_restart != 0xFFFFFFFF))//??????????,?????????????????
        {
                if(temp_value != g_Flash_Control_Struct.goal_step_before_restart)//??????????????????????,???????????
                {
                   g_day_goal_flag = 0;
                }
                else//?????????????????????,????????
                {
                   g_day_goal_flag = 1;
                }
        }
        else//?????????,??????????
        {
                if(temp_value != g_motion_goal[0].goal_step)
                {
                     g_day_goal_flag = 0;
                }
        }

        g_motion_goal[0].goal_step = temp_value;
        g_first_goal_step = 0;//???????????????????,??????????????????1,?????0.

        g_Flash_Control_Struct.goal_step_before_restart = 0xFFFFFFFF;
    }

    GeneralErrorACK(GeneralError[0],7,1);   //success
}

typedef struct
{
	unsigned int weight;							
	unsigned int height;						
	unsigned int age;									
	unsigned int sex;									
}user_information;      //Added for test,need to delete later...
/******************************************************************************/
/** Set User Body FeatureInfo 2.4.2
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/20
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void FitSetUserBodyFeatureInfo(TlvItem_t *item)
{
    user_information  user_infor;

    uint8_t height_value = 0;
    uint8_t weight_value = 0;
    uint8_t age_value = 0;
    uint8_t gender_value = 0;
    uint8_t walk_step_value = 0;
    uint8_t run_step_value = 0;
    uint32_t length = 0;

    if(NULL == item)
    {
        return;
    }

    user_infor.height = *TlvItem_ItemValueWithType(item,1,&length);
    height_value = user_infor.height;

    user_infor.weight = *TlvItem_ItemValueWithType(item,2,&length);
    weight_value = user_infor.weight;

    user_infor.age = *TlvItem_ItemValueWithType(item,3,&length);
    age_value = user_infor.age;

    user_infor.sex = *TlvItem_ItemValueWithType(item,5,&length);
    gender_value = user_infor.sex;
#if 0
    user_infor.walk_step = *TlvItem_ItemValueWithType(item,6,&length);
    walk_step_value = user_infor.walk_step;

    user_infor.run_step = *TlvItem_ItemValueWithType(item,7,&length);
    run_step_value = user_infor.run_step;
#endif
    //msg = MSG_SET_USER_INFOR;
    //xQueueSend(gAppData.queueGSensor, &msg, (portTickType)0);

    if((0 == (height_value && weight_value && age_value && gender_value)) || (gender_value > 2) || (0 == walk_step_value) || (0 == run_step_value) || (0 == length))
    {
        //parameter error
        //GeneralErrorACK(GeneralError[6],7,2);
    }

    else
    {
//        SetUserFeatureMessage(&user_infor);   //Need to do later...

    }

    GeneralErrorACK(GeneralError[0],7,2);
}
/******************************************************************************/
/**  Get Tota lMotion Data 2.4.3
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/20
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void FitGetTotalMotionData(void)
{
//    TlvItem_t tlv_total_calorie;
//    Current_Day_Total_Motion_Data_t  tlv_total_motion_struct[5];

//    uint32_t total_calorie = 0;
//    uint8_t  motion_type[5] = {0x01,0x02,0x04,0x06,0x07};

//    uint32_t walk_step = 0;
//    uint16_t walk_calorie = 0;
//    uint32_t walk_distance = 0;

//    uint32_t run_step = 0;
//    uint16_t run_calorie = 0;
//    uint32_t run_distance = 0;

//    uint16_t ride_calorie = 0;

//    uint16_t shallow_sleep_time = 0;
//    uint16_t deep_sleep_time = 0;

////    xSemaphoreTake(semaphoreBtChipTx, 60);
//    memset(&Tlvs_value_resp,0,sizeof(TlvItem_t));
//    Tlvs_value_resp.type = 1 | CHILD_MASK;
//    Tlvs_value_resp.length = 0;

//    memset(&tlv_total_calorie,0,sizeof(TlvItem_t));
//    tlv_total_calorie.type = 0x02;
//    tlv_total_calorie.length = 0x04;
//    total_calorie = GetCurrentTotalCalorie();
//    total_calorie = Uint32LtoB(total_calorie);
//    tlv_total_calorie.value = (uint8_t*)(&total_calorie);
//    TlvItem_AddChild(&Tlvs_value_resp,&tlv_total_calorie);

//    for(uint8_t i = 0; i < 5; i++)
//    {
//        memset(&tlv_total_motion_struct[i],0,sizeof(Current_Day_Total_Motion_Data_t));

//        switch(i)
//        {
//            case 0x00: //walk
//                {
//                    tlv_total_motion_struct[i].motion_struct.type = 3 | CHILD_MASK;
//                    tlv_total_motion_struct[i].motion_struct.length = 0;

//                    tlv_total_motion_struct[i].motion_type.type = 0x04;
//                    tlv_total_motion_struct[i].motion_type.length = 0x01;
//                    tlv_total_motion_struct[i].motion_type.value = (uint8_t*)&motion_type[i];
//                    TlvItem_AddChild(&tlv_total_motion_struct[i].motion_struct,&tlv_total_motion_struct[i].motion_type);

//                    tlv_total_motion_struct[i].step.type = 0x05;
//                    tlv_total_motion_struct[i].step.length = 0x04;
//                    walk_step = GetWalkSteps();
//                    //walk_step = GetCurrentTotalSteps();
//                    walk_step = Uint32LtoB(walk_step);
//                    tlv_total_motion_struct[i].step.value = (uint8_t *)&walk_step;
//                    TlvItem_AddChild(&tlv_total_motion_struct[i].motion_struct,&tlv_total_motion_struct[i].step);

//                    tlv_total_motion_struct[i].calorie.type = 0x06;
//                    tlv_total_motion_struct[i].calorie.length = 0x02;
//                    walk_calorie = GetWalkCalorie();
//                    //walk_calorie = GetCurrentTotalCalorie();
//                    walk_calorie = Uint16LtoB(walk_calorie);
//                    tlv_total_motion_struct[i].calorie.value = (uint8_t*)&walk_calorie;
//                    TlvItem_AddChild(&tlv_total_motion_struct[i].motion_struct,&tlv_total_motion_struct[i].calorie);

//                    tlv_total_motion_struct[i].distance.type = 0x07;
//                    tlv_total_motion_struct[i].distance.length = 0x04;
//                    walk_distance = GetWalkDistance();
//                    //walk_distance = GetCurrentTotalDistance();
//                    walk_distance = Uint32LtoB(walk_distance);
//                    tlv_total_motion_struct[i].distance.value = (uint8_t*)&walk_distance;
//                    TlvItem_AddChild(&tlv_total_motion_struct[i].motion_struct,&tlv_total_motion_struct[i].distance);
//                }
//                break;

//            case 0x01: //run
//                {
//                    tlv_total_motion_struct[i].motion_struct.type = 3 | CHILD_MASK;
//                    tlv_total_motion_struct[i].motion_struct.length = 0;

//                    tlv_total_motion_struct[i].motion_type.type = 0x04;
//                    tlv_total_motion_struct[i].motion_type.length = 0x01;
//                    tlv_total_motion_struct[i].motion_type.value = (uint8_t*)&motion_type[i];
//                    TlvItem_AddChild(&tlv_total_motion_struct[i].motion_struct,&tlv_total_motion_struct[i].motion_type);

//                    tlv_total_motion_struct[i].step.type = 0x05;
//                    tlv_total_motion_struct[i].step.length = 0x04;
//                    run_step = GetRunSteps();
//                    run_step = Uint32LtoB(run_step);
//                    tlv_total_motion_struct[i].step.value = (uint8_t*)&run_step;
//                    TlvItem_AddChild(&tlv_total_motion_struct[i].motion_struct,&tlv_total_motion_struct[i].step);

//                    tlv_total_motion_struct[i].calorie.type = 0x06;
//                    tlv_total_motion_struct[i].calorie.length = 0x02;
//                    run_calorie = GetRunCalorie();
//                    run_calorie = Uint16LtoB(run_calorie);
//                    tlv_total_motion_struct[i].calorie.value = (uint8_t*)&run_calorie;
//                    TlvItem_AddChild(&tlv_total_motion_struct[i].motion_struct,&tlv_total_motion_struct[i].calorie);

//                    tlv_total_motion_struct[i].distance.type = 0x07;
//                    tlv_total_motion_struct[i].distance.length = 0x04;
//                    run_distance = GetRunDistance();
//                    run_distance = Uint32LtoB(run_distance);
//                    tlv_total_motion_struct[i].distance.value = (uint8_t*)&run_distance;
//                    TlvItem_AddChild(&tlv_total_motion_struct[i].motion_struct,&tlv_total_motion_struct[i].distance);
//                }
//                break;

//            case 0x02://ride
//                {
//                    tlv_total_motion_struct[i].motion_struct.type = 3 | CHILD_MASK;
//                    tlv_total_motion_struct[i].motion_struct.length = 0;

//                    tlv_total_motion_struct[i].motion_type.type = 0x04;
//                    tlv_total_motion_struct[i].motion_type.length = 0x01;
//                    tlv_total_motion_struct[i].motion_type.value = (uint8_t*)&motion_type[i];
//                    TlvItem_AddChild(&tlv_total_motion_struct[i].motion_struct,&tlv_total_motion_struct[i].motion_type);

//                    tlv_total_motion_struct[i].calorie.type = 0x06;
//                    tlv_total_motion_struct[i].calorie.length = 0x02;
//                    ride_calorie = GetRideCalorie();
//                    ride_calorie = Uint16LtoB(ride_calorie);
//                    tlv_total_motion_struct[i].calorie.value = (uint8_t*)&ride_calorie;
//                    TlvItem_AddChild(&tlv_total_motion_struct[i].motion_struct,&tlv_total_motion_struct[i].calorie);
//                }
//                break;

//            case 0x03: //light sleep
//                {
//                    tlv_total_motion_struct[i].motion_struct.type = 3 | CHILD_MASK;
//                    tlv_total_motion_struct[i].motion_struct.length = 0;

//                    tlv_total_motion_struct[i].motion_type.type = 0x04;
//                    tlv_total_motion_struct[i].motion_type.length = 0x01;
//                    tlv_total_motion_struct[i].motion_type.value = (uint8_t*)&motion_type[i];
//                    TlvItem_AddChild(&tlv_total_motion_struct[i].motion_struct,&tlv_total_motion_struct[i].motion_type);

//                    tlv_total_motion_struct[i].sleep_time.type = 0x08;
//                    tlv_total_motion_struct[i].sleep_time.length = 0x02;
//                    shallow_sleep_time = GetShallowSleepTimes();
//                    shallow_sleep_time = Uint16LtoB(shallow_sleep_time);
//                    tlv_total_motion_struct[i].sleep_time.value = (uint8_t*)&shallow_sleep_time;
//                    TlvItem_AddChild(&tlv_total_motion_struct[i].motion_struct,&tlv_total_motion_struct[i].sleep_time);
//                }
//                break;

//            case 0x04://deep sleep
//                {
//                    tlv_total_motion_struct[i].motion_struct.type = 3 | CHILD_MASK;
//                    tlv_total_motion_struct[i].motion_struct.length = 0;

//                    tlv_total_motion_struct[i].motion_type.type = 0x04;
//                    tlv_total_motion_struct[i].motion_type.length = 0x01;
//                    tlv_total_motion_struct[i].motion_type.value = (uint8_t*)&motion_type[i];
//                    TlvItem_AddChild(&tlv_total_motion_struct[i].motion_struct,&tlv_total_motion_struct[i].motion_type);

//                    tlv_total_motion_struct[i].sleep_time.type = 0x08;
//                    tlv_total_motion_struct[i].sleep_time.length = 0x02;
//                    deep_sleep_time = GetDeepSleepTimes();
//                    deep_sleep_time = Uint16LtoB(deep_sleep_time);
//                    tlv_total_motion_struct[i].sleep_time.value = (uint8_t*)&deep_sleep_time;
//                    TlvItem_AddChild(&tlv_total_motion_struct[i].motion_struct,&tlv_total_motion_struct[i].sleep_time);
//                }
//                break;
//        }

//        TlvItem_AddChild(&Tlvs_value_resp,&tlv_total_motion_struct[i].motion_struct);
//    }

//    SendFramePack(&Tlvs_value_resp,0x07,0x03);
}
/******************************************************************************/
/** Get Active Remind Message
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/1/12
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void FitGetActiveRemindMessage(TlvItem_t *item)
{
    ACTIVE_REMIND_MSG_T  active_reminder_struct= {0};

    TlvItem_t item_remider_enable;
    TlvItem_t item_remider_interval;
    TlvItem_t item_remider_start;
    TlvItem_t item_remider_end;
    TlvItem_t item_remider_cycle;
    uint16_t  remider_start;
    uint16_t  remider_end;

    if(NULL == item)
    {
        GeneralErrorACK(GeneralError[5],0x07,0x06);
        return;
    }

    memset(&Tlvs_value_resp,0,sizeof(TlvItem_t));
    Tlvs_value_resp.type = 1 | CHILD_MASK;
    Tlvs_value_resp.length = 0;

    memset(&item_remider_enable,0,sizeof(TlvItem_t));
    item_remider_enable.type = 0x02;
    item_remider_enable.length = 0x01;
    item_remider_enable.value = (uint8_t*)&(active_reminder_struct.active_remider_enable);
    TlvItem_AddChild(&Tlvs_value_resp,&item_remider_enable);

    memset(&item_remider_interval,0,sizeof(TlvItem_t));
    item_remider_interval.type = 0x03;
    item_remider_interval.length = 0x01;
    item_remider_interval.value = (uint8_t*)&(active_reminder_struct.active_remider_interval);
    TlvItem_AddChild(&Tlvs_value_resp,&item_remider_interval);

    memset(&item_remider_start,0,sizeof(TlvItem_t));
    item_remider_start.type = 0x04;
    item_remider_start.length = 0x02;
    remider_start = Uint16LtoB(active_reminder_struct.active_remider_start);
    item_remider_start.value = (uint8_t*)&(remider_start);
    TlvItem_AddChild(&Tlvs_value_resp,&item_remider_start);

    memset(&item_remider_end,0,sizeof(TlvItem_t));
    item_remider_end.type = 0x05;
    item_remider_end.length = 0x02;
    remider_end = Uint16LtoB(active_reminder_struct.active_remider_start);
    item_remider_end.value = (uint8_t*)&(remider_end);
    TlvItem_AddChild(&Tlvs_value_resp,&item_remider_end);

    memset(&item_remider_cycle,0,sizeof(TlvItem_t));
    item_remider_cycle.type = 0x06;
    item_remider_cycle.length = 0x01;
    item_remider_cycle.value = (uint8_t*)&(active_reminder_struct.active_remider_cycle);
    TlvItem_AddChild(&Tlvs_value_resp,&item_remider_cycle);

    SendFramePack(&Tlvs_value_resp,0x07,0x06);
}
/******************************************************************************/
/** Set Active Remind Message
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/1/12
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void FitSetActiveRemindMessage(TlvItem_t *item)
{
//    ACTIVE_REMIND_MSG_T  active_reminder_struct;
//    uint32_t length = 0;
//    uint16_t temp_value = 0;

//    TlvItem_t *childitem = NULL;

//    if(NULL == item)
//    {
//        return;
//    }

//    childitem = TlvItem_ItemWithType(item,2);

//    if(NULL != childitem)
//    {
//        active_reminder_struct.active_remider_enable = *TlvItem_ItemValue(childitem,&length);
//        g_Flash_Control_Struct.long_time_sit_check_flag = active_reminder_struct.active_remider_enable;
//    }

//    childitem = TlvItem_ItemWithType(item,3);

//    if(NULL != childitem)
//    {
//        active_reminder_struct.active_remider_interval = *TlvItem_ItemValue(childitem,&length);
//    }

//    childitem = TlvItem_ItemWithType(item,4);

//    if(NULL != childitem)
//    {
//        TlvItem_ItemInt16Value(childitem,&temp_value,1);
//        active_reminder_struct.active_remider_start = *TlvItem_ItemValue(childitem,&length);
//    }

//    childitem = TlvItem_ItemWithType(item,5);

//    if(NULL != childitem)
//    {
//        TlvItem_ItemInt16Value(childitem,&temp_value,1);
//        active_reminder_struct.active_remider_end = *TlvItem_ItemValue(childitem,&length);
//    }

//    childitem = TlvItem_ItemWithType(item,6);

//    if(NULL != childitem)
//    {
//        active_reminder_struct.active_remider_cycle = *TlvItem_ItemValue(childitem,&length);
//    }

//    GeneralErrorACK(GeneralError[0],0x07,0x07);
}

/******************************************************************************/
/** 2.4.4 Upload Motion Sample data Frame Number
  *
  * @param[in] sample_frame_start_time
  * @param[in] sample_frame_end_time
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/27
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void FitUploadMotionSampleFrameNum(void)
{
//    TlvItem_t item_sample_frame_list;
//    TlvItem_t item_sample_frame_count;

//    uint16_t  temp_value = 0;

//    memset(&item_sample_frame_list,0,sizeof(TlvItem_t));
//    item_sample_frame_list.type = 0x01 | CHILD_MASK;
//    item_sample_frame_list.length = 0;

//    memset(&item_sample_frame_count,0,sizeof(TlvItem_t));
//    item_sample_frame_count.type = 0x02;
//    item_sample_frame_count.length = 0x2;
//    temp_value = g_tMotionSleepDb.page_cnt;
//    temp_value = Uint16LtoB(temp_value);
//    item_sample_frame_count.value = (uint8_t*)&temp_value;
//    TlvItem_AddChild(&item_sample_frame_list,&item_sample_frame_count);

//    SendFramePack(&item_sample_frame_list,0x07,0x0A);
}
/******************************************************************************/
/** 2.4.4 Fet Motion Sample Frame Count
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/27
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void FitGetMotionSampleFrame(TlvItem_t *item)
{
//    Msg_t msg;
//    uint32_t sample_frame_start_time = 0;
//    uint32_t sample_frame_end_time = 0;

//    TlvItem_t *item_start_time = NULL;
//    TlvItem_t *item_end_time = NULL;

//    if(NULL == item)
//    {
//        GeneralErrorACK(GeneralError[5],0x07,0x0A);
//        return;
//    }

//    item_start_time = TlvItem_ItemWithType(item,3);

//    if(NULL != item_start_time)
//    {
//        TlvItem_ItemInt32Value(item_start_time,&sample_frame_start_time,1);
//    }

//    g_tMotionSleepDb.start_timestamp_request = sample_frame_start_time;
//    item_end_time = TlvItem_ItemWithType(item,4);

//    if(NULL != item_end_time)
//    {
//        TlvItem_ItemInt32Value(item_end_time,&sample_frame_end_time,1);
//    }

//    g_tMotionSleepDb.end_timestamp_request = sample_frame_end_time;
//    /*updata threshold step*/
//    g_threshold_step = GetCurrentTotalSteps();

//    msg = MSG_PRO_GET_MOTION_DATA_FRAME;
//    xQueueSend(g_queue_memory,&msg,(portTickType)0);
}
/******************************************************************************/
/** 2.4.5 Upload Real Sample Data
  *
  * @param[in]
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/27
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void FitUploadSampleData(uint16_t frame_index)
{
//    uint8_t  i = 0;
//    uint16_t temp_uint16_value=0;
//    uint32_t temp_uint32_value=0;
//    uint32_t page_sample_data_cnt = g_tMotionSleepDb.tlv_len;
//    uint32_t data_cnt = 0;
//    page_motion_sample_t *page_sample = (page_motion_sample_t*)g_tMotionSleepDb.tlv_buffer;

//    TlvItem_t item_sample_frame;
//    TlvItem_t item_frame_index;
//    TlvItem_t item_start_time;
//    TlvItem_t item_sample_info[MAX_MINUTE_SINGLE_FRAME];
//    TlvItem_t item_sample_time_offset[MAX_MINUTE_SINGLE_FRAME];
//    TlvItem_t item_base_parameter[MAX_MINUTE_SINGLE_FRAME];
//    TlvItem_t item_update_flag[5];
//    uint8_t update_flag[5];
//    uint8_t item_update_pos = 0;

//    memset(&item_sample_frame,0,sizeof(TlvItem_t));
//    item_sample_frame.type = 0x01 | CHILD_MASK;
//    item_sample_frame.length = 0;

//    memset(&item_frame_index,0,sizeof(TlvItem_t));
//    item_frame_index.type = 0x02;
//    item_frame_index.length = 0x02;
//    temp_uint16_value = g_tMotionSleepDb.page_index_request;
//    temp_uint16_value = Uint16LtoB(temp_uint16_value);
//    item_frame_index.value = (uint8_t *)&(temp_uint16_value);
//    TlvItem_AddChild(&item_sample_frame,&item_frame_index);

//    memset(&item_start_time,0,sizeof(TlvItem_t));
//    item_start_time.type = 0x03;
//    item_start_time.length = 0x04;
//    temp_uint32_value = g_tMotionSleepDb.page_timestamp;
//    temp_uint32_value = Uint32LtoB(temp_uint32_value);
//    item_start_time.value = (uint8_t *)&(temp_uint32_value);
//    TlvItem_AddChild(&item_sample_frame,&item_start_time);

//    while(data_cnt != page_sample_data_cnt)
//    {
//        memset(&item_sample_info[i],0,sizeof(TlvItem_t));
//        item_sample_info[i].type = 0x04 | CHILD_MASK;
//        item_sample_info[i].length = 0;

//        memset(&item_sample_time_offset[i],0,sizeof(TlvItem_t));
//        item_sample_time_offset[i].type = 0x05;
//        item_sample_time_offset[i].length = 0x01;
//        item_sample_time_offset[i].value = (uint8_t*)&(page_sample->buffer[0]);
//        TlvItem_AddChild(&item_sample_info[i],&item_sample_time_offset[i]);

//        memset(&item_base_parameter[i],0,sizeof(TlvItem_t));
//        item_base_parameter[i].type = 0x06;
//        item_base_parameter[i].length = page_sample->len-2;
//        item_base_parameter[i].value = (uint8_t*)&(page_sample->buffer[1]);
//        TlvItem_AddChild(&item_sample_info[i],&item_base_parameter[i]);

//        if((page_sample->buffer[page_sample->len-1] == 1) && (item_update_pos < 5))
//        {
//            memset(&item_update_flag[item_update_pos],0,sizeof(TlvItem_t));
//            item_update_flag[item_update_pos].type = 0x08;
//            item_update_flag[item_update_pos].length = 1;
//            update_flag[item_update_pos] = 1;
//            item_update_flag[item_update_pos].value = &update_flag[item_update_pos];
//            TlvItem_AddChild(&item_sample_info[i],&item_update_flag[item_update_pos]);

//            item_update_pos++;
//        }

//        TlvItem_AddChild(&item_sample_frame,&item_sample_info[i]);

//        data_cnt += (page_sample->len + 1);
//        page_sample = (page_motion_sample_t*)(g_tMotionSleepDb.tlv_buffer + data_cnt);
//        i++;

//        if(i>50)
//        {
//            break;
//        }
//    }

//    SendFramePack(&item_sample_frame,0x07,0x0B);
//    //SendFramePackEncrypt(&item_sample_frame,0x07,0x0B);
}
/******************************************************************************/
/** 2.4.5 Get Sample Data Interface
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/27
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void FitGetSampleDataPort(TlvItem_t *item)
{
//    Msg_t msg;
//    uint16_t temp_value = 0;
//    uint16_t frame_index;
//    TlvItem_t *childitem = NULL;

//    if(NULL == item)
//    {
//        GeneralErrorACK(GeneralError[5],0x07,0x0B);
//        return;
//    }

//    childitem = TlvItem_Child(item); //point to frame_index

//    if(NULL == childitem)
//    {
//        GeneralErrorACK(GeneralError[5],0x07,0x0B);
//        return;
//    }

//    TlvItem_ItemInt16Value(childitem,&temp_value,1);
//    g_tMotionSleepDb.page_index_request = temp_value;

//    msg = MSG_PRO_GET_MOTION_DATA;
//    xQueueSend(g_queue_memory,&msg,(portTickType)0);


//    //FitUploadSampleData(frame_index);
}
/******************************************************************************/
/** 2.4.6 Upload Status Data Frame
  *
  * @param[in] status_frame_start_time
  * @param[in] status_frame_end_time
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/27
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void FitUploadStateDataFrame(void)
{
//    TlvItem_t item_status_frame_list;
//    TlvItem_t item_status_frame_count;

//    uint16_t  status_value = 0;

//    memset(&item_status_frame_list,0,sizeof(TlvItem_t));
//    item_status_frame_list.type = 0x01 | CHILD_MASK;
//    item_status_frame_list.length = 0;

//    memset(&item_status_frame_count,0,sizeof(TlvItem_t));
//    item_status_frame_count.type = 0x02;
//    item_status_frame_count.length = 0x2;
//    status_value = g_tMotionSleepDb.page_cnt;
//    status_value = Uint16LtoB(status_value);
//    item_status_frame_count.value = (uint8_t*)&status_value;
//    TlvItem_AddChild(&item_status_frame_list,&item_status_frame_count);

//    SendFramePack(&item_status_frame_list,0x07,0x0C);
}
/******************************************************************************/
/** 2.4.6 Get State Data Frame Interface
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/27
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void  FitGetStateDataFrame(TlvItem_t *item)
{
//    Msg_t msg;
//    uint32_t status_frame_start_time = 0;
//    uint32_t status_frame_end_time = 0;

//    TlvItem_t *item_start_time = NULL;
//    TlvItem_t *item_end_time = NULL;

//    if(NULL == item)
//    {
//        GeneralErrorACK(GeneralError[5],0x07,0x0C);
//        return;
//    }

//    item_start_time = TlvItem_ItemWithType(item,3);

//    if(NULL != item_start_time)
//    {
//        TlvItem_ItemInt32Value(item_start_time,&status_frame_start_time,1);
//    }

//    g_tMotionSleepDb.start_timestamp_request = status_frame_start_time;
//    item_end_time = TlvItem_ItemWithType(item,4);

//    if(NULL != item_end_time)
//    {
//        TlvItem_ItemInt32Value(item_end_time,&status_frame_end_time,1);
//    }

//    g_tMotionSleepDb.end_timestamp_request = status_frame_end_time;


//    msg = MSG_PRO_GET_MOTION_STATE_FRAME;
//    xQueueSend(g_queue_memory,&msg,(portTickType)0);
}
/******************************************************************************/
/** 2.4.7  Upload State Data Interface
  *
  * @param[in]
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/27
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void FitUploadStateDataPort(uint16_t status_frame_index)
{
//    uint16_t temp_uint16_value=0;

//    DURATION_T duration[MAX_MINUTE_SINGLE_FRAME]= {0};

//    frame_motion_status_t *frame_status = (frame_motion_status_t*)g_tMotionSleepDb.tlv_buffer;
//    TlvItem_t item_status_frame_list;
//    TlvItem_t item_status_frame_index;
//    TlvItem_t item_status_info[MAX_MINUTE_SINGLE_FRAME];
//    TlvItem_t item_status_type[MAX_MINUTE_SINGLE_FRAME];
//    TlvItem_t item_duration[MAX_MINUTE_SINGLE_FRAME];
//    TlvItem_t item_update_flag[5];
//    uint8_t update_flag[5];
//    uint8_t item_update_pos = 0;

//    memset(&item_status_frame_list,0,sizeof(TlvItem_t));
//    item_status_frame_list.type = 0x01 | CHILD_MASK;
//    item_status_frame_list.length = 0;

//    memset(&item_status_frame_index,0,sizeof(TlvItem_t));
//    item_status_frame_index.type = 0x02;
//    item_status_frame_index.length = 0x02;
//    temp_uint16_value = Uint16LtoB(status_frame_index);
//    item_status_frame_index.value = (uint8_t *)&(temp_uint16_value);
//    TlvItem_AddChild(&item_status_frame_list,&item_status_frame_index);

//    for(uint8_t i = 0; i < g_tMotionSleepDb.tlv_len / sizeof(frame_motion_status_t); i++)
//    {
//        memset(&item_status_info[i],0,sizeof(TlvItem_t));
//        item_status_info[i].type = 0x03 | CHILD_MASK;
//        item_status_info[i].length = 0;

//        memset(&item_status_type[i],0,sizeof(TlvItem_t));
//        item_status_type[i].type = 0x04;
//        item_status_type[i].length = 0x01;
//        item_status_type[i].value = &(frame_status->status_type);
//        TlvItem_AddChild(&item_status_info[i],&item_status_type[i]);

//        memset(&item_duration[i],0,sizeof(TlvItem_t));
//        item_duration[i].type = 0x05;
//        item_duration[i].length = 0x06;
//        duration[i].start_time = Uint32LtoB(frame_status->start_time);
//        duration[i].time_offset = Uint16LtoB(frame_status->duration);
//        item_duration[i].value = (uint8_t*)&(duration[i]);
//        TlvItem_AddChild(&item_status_info[i],&item_duration[i]);

//        if((frame_status->update_flag == 1) && (item_update_pos < 5))
//        {
//            memset(&item_update_flag[item_update_pos],0,sizeof(TlvItem_t));
//            item_update_flag[item_update_pos].type = 0x06;
//            item_update_flag[item_update_pos].length = 1;
//            update_flag[item_update_pos] = 1;
//            item_update_flag[item_update_pos].value = &update_flag[item_update_pos];
//            TlvItem_AddChild(&item_status_info[i],&item_update_flag[item_update_pos]);

//            item_update_pos++;
//        }

//        TlvItem_AddChild(&item_status_frame_list,&item_status_info[i]);
//        frame_status++;
//    }

//    SendFramePack(&item_status_frame_list,0x07,0x0D);
//    //SendFramePackEncrypt(&item_status_frame_list,0x07,0x0D);
}
/******************************************************************************/
/** 2.4.7 Get State Data Interface
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/27
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void FitGetStateDataPort(TlvItem_t *item)
{
//    Msg_t msg;
//    uint16_t temp_value = 0;
//    TlvItem_t *childitem = NULL;

//    if(NULL == item)
//    {
//        GeneralErrorACK(GeneralError[5],0x07,0x0D);
//        return;
//    }

//    childitem = TlvItem_Child(item);

//    if(NULL == childitem)
//    {
//        GeneralErrorACK(GeneralError[5],0x07,0x0D);
//        return;
//    }

//    TlvItem_ItemInt16Value(childitem,&temp_value,1);
//    g_tMotionSleepDb.page_index_request = temp_value;

//    msg = MSG_PRO_GET_MOTION_STATE;
//    xQueueSend(g_queue_memory,&msg,(portTickType)0);
//    //FitUploadStateDataPort(status_frame_index);
}
/******************************************************************************/
/** 2.4.8 Set Motion Data Threshold
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/28
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void FitSetMotionDataThreshold(TlvItem_t *item)
{
    uint8_t threshold_temp_index=0x00;
    THRESSHOLD_STRUCT_T threshold_struct[5];
    TlvItem_t *temp_threshold_struct=NULL;


    for(threshold_temp_index=0; threshold_temp_index<5; threshold_temp_index++)
    {
        memset(&(threshold_struct[threshold_temp_index]),0,sizeof(THRESSHOLD_STRUCT_T));
    }

    threshold_temp_index = 0;

    if(NULL == item)
    {
        //GeneralErrorACK(GeneralError[5],0x07,0x0E);
        return;
    }

    temp_threshold_struct = item->childNode;/*item = atmo_list, temp_atmo_struct = item->childNode*/

    if(NULL == temp_threshold_struct)
    {
        //GeneralErrorACK(GeneralError[5],0x07,0x0E);
        return;
    }

    else
    {
        while(temp_threshold_struct != NULL)
        {
            threshold_struct[threshold_temp_index].data_type = *(temp_threshold_struct->value);
            threshold_struct[threshold_temp_index].value_type = *(temp_threshold_struct->value + 1);
            threshold_struct[threshold_temp_index].value = (*(temp_threshold_struct->value + 2)<<8) | (*(temp_threshold_struct->value+3));
            threshold_struct[threshold_temp_index].action= *(temp_threshold_struct->value + 4);

            threshold_temp_index++;

            if(threshold_temp_index >= 5)
            {
                threshold_temp_index = 0;
                break;
            }

            temp_threshold_struct = temp_threshold_struct->brotherNode;
        }
    }

    GeneralErrorACK(GeneralError[0],0x07,0x0E);   //success
}
/******************************************************************************/
/** 2.4.9 Motion Data Notification Active Report
  *
  * @param[in]
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/28
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
void FitNotificationActionData()
{
    TlvItem_t item_action;
    uint8_t action = 3;

    memset(&item_action,0,sizeof(TlvItem_t));

    item_action.type = 0x01;
    item_action.length = 0x01;
    item_action.value = &action;
    SendFramePack(&item_action,0x07,0x0F);
}
/******************************************************************************/
/** Set Show Switch 2.4.10
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/1/6
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void FitSetShowSwitch(TlvItem_t *item)
{
    uint8_t  cloud_summary_enable;
    uint32_t temp_length;

    if(NULL == item)
    {
        return;
    }

    if(NULL == TlvItem_ItemWithType(item,1))
    {
        GeneralErrorACK(GeneralError[6],0x07,0x10);
        return;
    }

    cloud_summary_enable = *TlvItem_ItemValue(item,&temp_length);

    GeneralErrorACK(GeneralError[0],0x07,0x10);
}
/******************************************************************************/
/** Reverse Sync data  2.4.11
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/1/7
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void FitReverseSyncdata(TlvItem_t *item)
{
    GeneralErrorACK(GeneralError[0],0x07,0x11);
}
/******************************************************************************/
/** Trigger Reverse  2.4.12
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/1/7
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void FitTriggerReverse(void)
{
    uint8_t reverse_sync_trigger = 1;

    memset(&Tlvs_value_resp,0,sizeof(TlvItem_t));
    Tlvs_value_resp.type = 0x01;
    Tlvs_value_resp.length = 0x01;
    Tlvs_value_resp.value = &reverse_sync_trigger;

    SendFramePack(&Tlvs_value_resp,1,0x12);
}
/******************************************************************************/
/** 2.4.13 Set Heart Rate Zone Paremeter
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/28
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void FitSetHeartParameter(TlvItem_t *item)
{
//    uint32_t length = 0;

//    if(NULL == item)
//    {
//        GeneralErrorACK(GeneralError[5],0x07,0x13);
//        return;
//    }

//    g_Flash_Control_Struct.hrm_min_threshold =   *TlvItem_ItemValueWithType(item,2,&length);

//    g_Flash_Control_Struct.hrm_warm_up_threshold =   *TlvItem_ItemValueWithType(item,3,&length);

//    g_Flash_Control_Struct.hrm_fat_burn_threshold =  *TlvItem_ItemValueWithType(item,4,&length);

//    g_Flash_Control_Struct.hrm_aerobic_threshold =   *TlvItem_ItemValueWithType(item,5,&length);

//    g_Flash_Control_Struct.hrm_anaerobic_threshold = *TlvItem_ItemValueWithType(item,6,&length);

//    g_Flash_Control_Struct.hrm_max_threshold =   *TlvItem_ItemValueWithType(item,7,&length);

//    g_Flash_Control_Struct.hrm_warn_switch =      *TlvItem_ItemValueWithType(item,8,&length);

//    g_Flash_Control_Struct.hrm_war_max =         *TlvItem_ItemValueWithType(item,9,&length);

//    GeneralErrorACK(GeneralError[0],0x07,0x13);
}
/******************************************************************************/
/** Fitness Service Type 2.4
  *
  * @param[in] item_t
  * @param[in] command_id
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/19
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void FitnessServiceType(TlvItem_t *item_t,uint8_t command_iden)
{
    FIT_COMMAND_ID_E command_id;
    command_id = (FIT_COMMAND_ID_E)command_iden;

//    Msg_t msg;                                               //Need to do later...

    switch(command_id)
    {
        case FIT_CMD_SET_MOTION_GOAL:
            /**2.4.1 */
            FitSetMotionTargetInfo(item_t);
            break;

        case FIT_CMD_SET_USER_SIGN:
            /**2.4.2 */
            FitSetUserBodyFeatureInfo(item_t);
            break;

        case FIT_CMD_GET_INTRADAY_MOTION_TOTAL:
            /**2.4.3 */
            //FitGetTotalMotionData();

//            msg = MSG_PRO_GET_MOTION_SLEEP_STATISTIC_INFO;        //Need to do later...
//            xQueueSend(g_queue_memory,&msg,(portTickType)5000);   //Need to do later...
            break;

        case FIT_CMD_GET_ACTIVE_REMIND_MSG:

            FitGetActiveRemindMessage(item_t);
            break;

        case FIT_CMD_SET_ACTIVE_REMIND_MSG:

            FitSetActiveRemindMessage(item_t);
            break;

        case FIT_CMD_GET_MOTION_SAMPLE_FRAME:
            /**2.4.4 */
            FitGetMotionSampleFrame(item_t);
            break;

        case FIT_CMD_GET_SAMPLE_DATA:
            /**2.4.5 */
            FitGetSampleDataPort(item_t);
            break;

        case FIT_CMD_GET_STATE_DATA_FRAME:
            /**2.4.6 */
            FitGetStateDataFrame(item_t);
            break;

        case FIT_CMD_GET_STATE_DATA_PORT:
            /**2.4.7 */
            FitGetStateDataPort(item_t);
            break;

        case FIT_CMD_SET_MOTION_DATA_THRESHOLD:
            /**2.4.8 */
            FitSetMotionDataThreshold(item_t);
            break;

        case FIT_CMD_GET_MOTION_INFORM_APPEAR:
            /**2.4.9 */
            FitNotificationActionData();
            break;

        case FIT_CMD_SET_SHOW_COLLECT_SWITCH:
            /**2.4.10 */
            FitSetShowSwitch(item_t);
            break;

        case FIT_CMD_REVERSE_SYNCHRODATA:
            /**2.4.11 */
            FitReverseSyncdata(item_t);
            break;

        case FIT_CMD_TRIGGER_REVERSE_SYNCHRODATA:
            /**2.4.12 */
            FitTriggerReverse();
            break;

        case FIT_CMD_SET_HEART_PARAMETER:
            /**2.4.13 */
            FitSetHeartParameter(item_t);
            break;

        default:
            break;
    }
}
/******************************************************************************/
/** Set Alarm Clock List 2.5.1
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/23
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void AlaSetAlarmClockList(TlvItem_t *item)
{
//    SET_EVENT_ALARM_T Event_Alarm[5];

//    uint8_t alarm_index=0;
//    uint8_t alarm_temp_index=0;
//    // uint8_t  alarm_name_length =0;
//    uint16_t alarm_time = 0;
//    TlvItem_t *temp_alarm_struct=NULL;
//    TlvItem_t *temp_alarm=NULL;

//    temp_alarm_struct = item->childNode;/*item = alarm_list, temp_alarm_struct = item->childNode*/

//    memset(&(Event_Alarm[0]),0,sizeof(SET_EVENT_ALARM_T));
//    memset(&(Event_Alarm[1]),0,sizeof(SET_EVENT_ALARM_T));
//    memset(&(Event_Alarm[2]),0,sizeof(SET_EVENT_ALARM_T));
//    memset(&(Event_Alarm[3]),0,sizeof(SET_EVENT_ALARM_T));
//    memset(&(Event_Alarm[4]),0,sizeof(SET_EVENT_ALARM_T));

//    for(int i =0; i<5; ++i)
//    {
//        Event_Alarm[i].time = ALARM_TIME_NONE;
//    }

//    if(temp_alarm_struct == NULL)
//    {
//        GeneralErrorACK(GeneralError[5],0x08,0x01);
//    }

//    else
//    {
//        while(temp_alarm_struct !=NULL)
//        {
//            temp_alarm = temp_alarm_struct->childNode;/* alarm_index */
//            alarm_index = *(temp_alarm->value);
//            Event_Alarm[alarm_temp_index].index = alarm_index-1;
//            temp_alarm = temp_alarm->brotherNode; /* alarm_enable alarm_time alarm_cycle alarm_name */

//            while(temp_alarm != NULL)
//            {
//                switch(temp_alarm->type)
//                {
//                    case 4 :
//                        Event_Alarm[alarm_temp_index].enable=(uint8_t)(*(temp_alarm->value));
//                        break;

//                    case 5:
//                        *((uint8_t *)(&alarm_time)+1) = *(temp_alarm->value);
//                        *((uint8_t *)(&alarm_time)+0) = *((temp_alarm->value)+1);
//                        Event_Alarm[alarm_temp_index].time = alarm_time;
//                        break;

//                    case 6:
//                        Event_Alarm[alarm_temp_index].cycle=(uint8_t)(*(temp_alarm->value));
//                        break;

//                    case 7:
//#if 0
//                        alarm_name_length = temp_alarm->length;
//                        memcpy(Event_Alarm[alarm_temp_index].name,temp_alarm->value,(alarm_name_length>24)?24:alarm_name_length);
//#endif

//                        break;

//                    default:
//                        break;
//                }

//                temp_alarm = temp_alarm->brotherNode;
//            }



//            if(g_Flash_Control_Struct.bt_bind_os == 0x01)
//            {
//                for(uint8_t  i = alarm_index; i<5; ++i)
//                {
//                    g_event_alarm[i].set_event_alarm.enable = ALARM_DISABLE;
//                    g_event_alarm[i].set_event_alarm.time = ALARM_TIME_NONE;
//                    g_event_alarm[i].set_event_alarm.cycle = 0;
//                    g_event_alarm[i].alarm_next_time = ALARM_TIME_NONE;
//                    g_event_alarm[i].alarm_next_numbers = ALARM_WORK_TIME_OUT;
//                    LogRTC("-------IOS need init the event alarm which not exist in app  \n\r");
//                }

//            }


//            SetNewEventAlarm(&Event_Alarm[alarm_temp_index]);
//            memcpy(&g_Flash_Control_Struct.event_alarm_save[alarm_temp_index],&Event_Alarm[alarm_temp_index],sizeof(SET_EVENT_ALARM_T));
//            WriteContorlBytesToGenericFlash();
//            alarm_temp_index++ ;
//            temp_alarm_struct = temp_alarm_struct->brotherNode ;
//        }

//        GeneralErrorACK(GeneralError[0],0x08,0x01);   //success
//    }

}
/******************************************************************************/
/**  Set Smart Alarm Clock List 2.5.2
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/23
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void AlaSetSmartAlarmClockList(TlvItem_t *item)
{
//    //SET_SMART_ALARM_T Smart_Alarm[5];
//    SET_SMART_ALARM_T Smart_Alarm[1];

//    uint8_t smart_alarm_index=0;
//    uint8_t smart_alarm_temp_index=0;
//    uint16_t alarm_time = 0;
//    TlvItem_t *temp_smart_alarm_struct=NULL;
//    TlvItem_t *temp_smart_alarm=NULL;
//    temp_smart_alarm_struct = item->childNode;//temp = smart_alarm_struct

//    memset(&(Smart_Alarm[0]),0,sizeof(SET_SMART_ALARM_T));
//    Smart_Alarm[0].time = ALARM_TIME_NONE;
//    // g_smart_alarm.alarm_next_numbers = ALARM_WORK_TIME_OUT;

//    if(temp_smart_alarm_struct == NULL)
//    {
//        GeneralErrorACK(GeneralError[5],0x08,0x02);
//    }

//    else
//    {
//        while(temp_smart_alarm_struct !=NULL)
//        {
//            temp_smart_alarm = temp_smart_alarm_struct->childNode; /* alarm_index */
//            smart_alarm_index =(*(temp_smart_alarm->value));
//            Smart_Alarm[smart_alarm_temp_index].index = smart_alarm_index-1;
//            temp_smart_alarm = temp_smart_alarm->brotherNode;

//            while(temp_smart_alarm != NULL)
//            {
//                switch(temp_smart_alarm->type)
//                {
//                    case 4:
//                        Smart_Alarm[smart_alarm_temp_index].enable =(uint8_t)(*(temp_smart_alarm->value));
//                        break;

//                    case 5:
//                        *((uint8_t *)(&alarm_time)+1) = *(temp_smart_alarm->value);
//                        *((uint8_t *)(&alarm_time)+0) = *((temp_smart_alarm->value)+1);

//                        Smart_Alarm[smart_alarm_temp_index].time = alarm_time;
//                        break;

//                    case 6:
//                        Smart_Alarm[smart_alarm_temp_index].cycle=(uint8_t)(*(temp_smart_alarm->value));
//                        break;

//                    case 7:
//                        Smart_Alarm[smart_alarm_temp_index].ahead_time=(uint8_t)(*(temp_smart_alarm->value));
//                        break;

//                    default:
//                        break;
//                }

//                temp_smart_alarm = temp_smart_alarm->brotherNode ;
//            }


//            if(!g_smart_alarm.set_smart_alarm.enable \
//                    && (!g_smart_alarm.set_smart_alarm.cycle) \
//                    && (ALARM_PAUSE_ONE_TIME == g_smart_alarm.alarm_next_numbers))
//            {

//                LogRTC("-----just ignore the Android auto close single smart\n\r");

//            }


//            else
//            {
//                SetNewSmartAlarm(&Smart_Alarm[smart_alarm_temp_index]);
//                memcpy(&g_Flash_Control_Struct.smart_alarm_save,&Smart_Alarm[smart_alarm_temp_index],sizeof(SET_SMART_ALARM_T));
//                WriteContorlBytesToGenericFlash();

//                // smart_alarm_temp_index ++;
//                temp_smart_alarm_struct = temp_smart_alarm_struct->brotherNode ;
//            }
//        }

//        GeneralErrorACK(GeneralError[0],0x08,0x02);
//    }
}
/******************************************************************************/
/** Alarm Service Type 2.5
  *
  * @param[in] item_t
  * @param[in] command_id
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/19
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void AlarmServiceType(TlvItem_t *item_t,uint8_t command_iden)
{
    ALA_COMMAND_ID_E command_id;
    command_id = (ALA_COMMAND_ID_E)command_iden;

    switch(command_id)
    {
        case ALA_CMD_SET_ALARM_CLOCK:
            /**2.5.1 */
            AlaSetAlarmClockList(item_t);
            break;

        case ALA_CMD_SET_CAPACITY_ALARM_CLOCK:
            /**2.5.2 */
            AlaSetSmartAlarmClockList(item_t);
            break;

        default:
            break;
    }
}

static void OTAReplyDeviceUpdateOrNot(void)
{
    TlvItem_t item_battery_Level;
    uint8_t batterythreshold;
    uint8_t batteryLevelPercent;
    uint32_t  ErrorCodeVaule = 0x00;

    batterythreshold = 30;
    batteryLevelPercent = GetBatteryLevel();
//    LogOta("batteryLevelPercent=%d\r\n",batteryLevelPercent);
//    LogOta("gOTAData.work_mode=%d\r\n",gOTAData.work_mode);



    if(batteryLevelPercent < batterythreshold)
    {
        ErrorCodeVaule = Uint32LtoB(GeneralError[23]);//
    }

    else//> 30% batery level can ota
    {
        if(gOTAData.work_mode == 2)//do not allow ota in background mode
        {
//            LogOta("GeneralError[24]\r\n");
            ErrorCodeVaule = Uint32LtoB(GeneralError[24]);
        }

        else if(gOTAData.work_mode == 0)//normal mode,can ota
        {
//            LogOta("GeneralError[0]\r\n");
            ErrorCodeVaule = Uint32LtoB(GeneralError[0]);
        }
    }

    memset(&Tlvs_value_resp,0,sizeof(TlvItem_t));
    Tlvs_value_resp.type = ERROR_CODE_TYPE;
    Tlvs_value_resp.length = 0x04;
    Tlvs_value_resp.value = (uint8_t *)(&ErrorCodeVaule);

    memset(&item_battery_Level,0,sizeof(TlvItem_t));
    item_battery_Level.type = 0x04;
    item_battery_Level.length = 0x01;
    item_battery_Level.value = (uint8_t*)&(batterythreshold);
    TlvItem_AddBrother(&Tlvs_value_resp,&item_battery_Level);
    SendFramePack(&Tlvs_value_resp,OTA_SERVICE,OTA_CMD_QUERY_UPGRADE_PERMIT);

}



/******************************************************************************/
/** OTACheckDeviceUpdateOrNot 2.6.1
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/01/09
  *   Author       : ryan.wu
  *   Modification : Created function

*******************************************************************************/
static void OTACheckDeviceUpdateOrNot(TlvItem_t *item)
{
    TlvItem_t *childitem=NULL;

    if(NULL == item)
    {
        GeneralErrorACK(GeneralError[6],OTA_SERVICE,OTA_CMD_QUERY_UPGRADE_PERMIT);
        return;
    }

    else
    {
//        LogOta("OTACheckDeviceUpdateOrNot\r\n");
        childitem = TlvItem_ItemWithType(item,0x01);

        if(NULL != childitem)
        {
//            LogOta("App Send softwareversion=%d\r\n",childitem->length);
            //memcpy(gOTAData.softwareversion,childitem->value,strlen((char*)gOTAData.softwareversion));
            //for(ryanindex = 0; ryanindex <14; ryanindex++)
            //{
            //    printf("0x%.2x ",  gOTAData.softwareversion[ryanindex]);
            //}
        }

        childitem = TlvItem_ItemWithType(item,0x02);

        if(NULL != childitem)
        {
            //TlvItem_ItemInt16Value(childitem,&gOTAData.info_component_size,1);
        }

//        LogOta("gOTAData.info_component_size=%d\r\n", gOTAData.info_component_size);
        childitem = TlvItem_ItemWithType(item,0x03);

        if(NULL != childitem)
        {
            gOTAData.work_mode = *(childitem->value);
        }

//        LogOta("gOTAData.work_mode=%d\r\n", gOTAData.work_mode);
        OTAReplyDeviceUpdateOrNot();
    }
}
static void OTAVariableInit(void)
{
    gOTAData.ota_file_offset = 0x0;
    gOTAData.ota_file_bitmap = 0x0;
    gOTAData.ota_file_length = 0x0;
    gOTAData.ota_package_index = 0x0;
    gOTAData.ota_flash_address =0x0;
    gOTAData.ota_last_bytes =0x0;
    gOTAData.package_received_size =0x0;
    gOTAData.package_validity =0x0;
    gOTAData.package_valid_size =0x0;
    gOTAData.ota_package_total =0x0;
    gOTAData.timercnt =0x0;
    gOTAData.ota_req_timeout = 0x00;
    gOTAData.ota_transfer_finished = 0x00;
    gOTAData.ota_check_sum =0x0;
    //gOTAData.ota_animation_index =9;//the 9th update picture
    gOTAData.ota_on_going = 0;//the 9th update picture
    gOTAData.ota_hrm_update_flag = 0;
}
//static void OTATimerCallBack(OS_TIMER timer)
//{
////    gOTAData.timercnt++;
////    gOTAData.ota_req_timeout++;

////    if(gOTAData.timercnt == 13)
////    {
////        g_Flash_Control_Struct.baterry_level = GetBatteryLevel();
////        g_Flash_Control_Struct.dead_zone = GetLocalTimeZone();
////        g_Flash_Control_Struct.goal_step_before_restart = g_motion_goal[0].goal_step;//donna:??????
////        WriteContorlBytesToGenericFlash();
////        OS_DELAY_MS(150);
////        hw_cpm_reboot_system();
////    }

////    if(gOTAData.ota_transfer_finished == 0x00)
////    {
////        if(gOTAData.ota_req_timeout >= 4)
////        {
////            gOTAData.ota_req_timeout = 0;

////            if(gOTAData.ota_package_index == 0)
////            {
////                OTAApplyUpgradePackageData(gOTAData.ota_file_offset,128,gOTAData.ota_file_bitmap);
////            }
////            if(gOTAData.ota_package_index < (gOTAData.ota_package_total-1)) //??????,??128,?????????
////            {
////                OTAApplyUpgradePackageData(gOTAData.ota_file_offset,128,gOTAData.ota_file_bitmap);
////            }

////            else if(gOTAData.ota_package_index == (gOTAData.ota_package_total-1))
////            {
////                gOTAData.ota_last_bytes = (gOTAData.package_valid_size - gOTAData.ota_package_index * 128) ;
////                LogOta("gOTAData.ota_last_bytes=%d\r\n",gOTAData.ota_last_bytes);
////                OTAApplyUpgradePackageData(gOTAData.ota_file_offset,gOTAData.ota_last_bytes,gOTAData.ota_file_bitmap);
////            }
////        }
////    }
//}
void OTATimerInit(void)
{
//    gOTAData.ota_timer= OS_TIMER_CREATE("ota timer",
//                                        OS_MS_2_TICKS(2*1000),
//                                        OS_TIMER_SUCCESS, /*repeated*/
//                                        (void *) OS_GET_CURRENT_TASK(),
//                                        OTATimerCallBack);
//    OS_TIMER_START(gOTAData.ota_timer,OS_TIMER_FOREVER);
}
/******************************************************************************/
/** OTAParameterNegotiate 2.6.2
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/01/09
  *   Author       : ryan.wu
  *   Modification : Created function

*******************************************************************************/
static void OTAParameterNegotiate(void)
{
    uint16_t app_wait_timeout;
    uint16_t device_restart_timeout;
    uint16_t ota_unit_size;
    uint16_t ota_interval;

    TlvItem_t tlv_device_restart_timeout;
    TlvItem_t tlv_ota_unit_size;
    TlvItem_t tlv_interval;
    TlvItem_t tlv_ackenable;
//    LogOta("OTAParameterNegotiate\r\n");

    gOTAData.app_wait_Timeout = 30;//second
    app_wait_timeout = Uint16LtoB(gOTAData.app_wait_Timeout) ;
    memset(&Tlvs_value_resp,0,sizeof(TlvItem_t));
    Tlvs_value_resp.type = 0x01;
    Tlvs_value_resp.length = 0x02;
    Tlvs_value_resp.value = (uint8_t*)&(app_wait_timeout);

    gOTAData.device_restart_Timeout = 240;//second
    device_restart_timeout = Uint16LtoB(gOTAData.device_restart_Timeout);
    memset(&tlv_device_restart_timeout,0,sizeof(TlvItem_t));
    tlv_device_restart_timeout.type = 0x02;
    tlv_device_restart_timeout.length = 0x02;
    tlv_device_restart_timeout.value =(uint8_t*)&(device_restart_timeout);
    TlvItem_AddBrother(&Tlvs_value_resp,&tlv_device_restart_timeout);

    gOTAData.ota_uint_size = 139;//Bytes
    ota_unit_size = Uint16LtoB(gOTAData.ota_uint_size);
    memset(&tlv_ota_unit_size,0,sizeof(TlvItem_t));
    tlv_ota_unit_size.type = 0x03;
    tlv_ota_unit_size.length = 0x02;
    tlv_ota_unit_size.value = (uint8_t*)&(ota_unit_size);
    TlvItem_AddBrother(&Tlvs_value_resp,&tlv_ota_unit_size);

    gOTAData.interval = 60;//ms
    ota_interval = Uint16LtoB(gOTAData.interval);
    memset(&tlv_interval,0,sizeof(TlvItem_t));
    tlv_interval.type = 0x04;
    tlv_interval.length = 0x02;
    tlv_interval.value = (uint8_t*)&(ota_interval);
    TlvItem_AddBrother(&Tlvs_value_resp,&tlv_interval);

    gOTAData.ack_enable = 0;//APP dont need to ack
    memset(&tlv_ackenable,0,sizeof(TlvItem_t));
    tlv_ackenable.type = 0x05;
    tlv_ackenable.length = 0x01;
    tlv_ackenable.value = &(gOTAData.ack_enable);
    TlvItem_AddBrother(&Tlvs_value_resp,&tlv_ackenable);

    SendFramePack(&Tlvs_value_resp,OTA_SERVICE,OTA_CMD_UPGRADE_PARA_CONSULT);

}
/******************************************************************************/
/** OTA PackageValidity 2.6.6
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/01/09
  *   Author       : ryan.wu
  *   Modification : Created function

*******************************************************************************/
static void OTAAppReadyOrNot(TlvItem_t *item)
{
    TlvItem_t *childitem=NULL;

    if(NULL == item)
    {
        GeneralErrorACK(GeneralError[6],OTA_SERVICE,OTA_CMD_APP_UPDATE_STATE);
        return;
    }

    else
    {
//        LogOta("OTAAppReadyOrNot\r\n");
        childitem = TlvItem_ItemWithType(item,0x01);

        if(NULL != childitem)
        {
            gOTAData.ota_app_ready = *(childitem->value);
//            LogOta("App ota_status=%d\r\n",gOTAData.ota_app_ready);
        }
    }
}
static void OTANotifyDeviceReboot(void)
{
//    uint8_t error;
//    uint8_t timeout_count = 0;

//    LogOta("OTANotifyDeviceReboot111\r\n");

//    if((gOTAData.package_validity == 0x01) && (gOTAData.ota_hrm_update_flag == 0x01))
//    {
//        gOTAData.timercnt = 0;
//        gOTAData.ota_req_timeout = 0;
//        LogOta("OTANotifyHrmUpdate \r\n");
//        error = StartHrmUpdate();
//        LogOta("OTANotifyHrmUpdate error=%d gOTAData.timercnt=%d\r\n",error,gOTAData.timercnt);

//        if(error == 0) //??????
//        {
//            //????????
//            uint8_t hrmsoftversion[5];

//            nvms_t Hrm_part_info = ad_nvms_open(NVMS_FW_UPDATE_FLAG_PART);

//            if(Hrm_part_info == NULL)
//            {
//                LogOta("Hrm_part_info == NULL\r\n");
//            }

//            else
//            {
//                timeout_count = 0;
//                do
//                {
//                    ad_nvms_write(Hrm_part_info, FTY_APP_FW_VERSION_OFFSET, gOTAData.hrmsoftwareversion,5);
//                    OS_DELAY_MS(100);
//                    ad_nvms_read(Hrm_part_info, FTY_APP_FW_VERSION_OFFSET, hrmsoftversion,5);

//                    if(++timeout_count > 10)
//                    {
//                        timeout_count = 0;
//                        break;
//                    }
//                }while(memcmp(gOTAData.hrmsoftwareversion,hrmsoftversion,5));
//            }
//        }

//        g_Flash_Control_Struct.baterry_level = GetBatteryLevel();
//        g_Flash_Control_Struct.dead_zone = GetLocalTimeZone();
//        g_Flash_Control_Struct.goal_step_before_restart = g_motion_goal[0].goal_step;//donna:??????
//        WriteContorlBytesToGenericFlash();
//        OS_DELAY_MS(150);
//        hw_cpm_reboot_system();
//    }

//    if(gOTAData.package_validity == 0x01)
//    {
//        if(gOTAData.package_valid_size>  200*1024)
//        {
//            LogOta("OTANotifyDeviceReboot333\r\n");
//            LogOta("OTAData.package_validity OK\r\n");
//            uint8_t needupdate[2];
//            uint8_t readneedupdate[2] = {0};
//            needupdate[0] =0xaa;
//            needupdate[1] =0xaa;

//            nvms_t para_part = ad_nvms_open(NVMS_FW_UPDATE_FLAG_PART);//added by ryan.wu 20170204

//            if(para_part == NULL)
//            {
//                LogOta("para_part == NULL\r\n");
//            }

//            else
//            {
//                timeout_count = 0;
//                do
//                {
//                    ad_nvms_write(para_part,0,needupdate,2);
//                    OS_DELAY_MS(100);
//                    ad_nvms_read(para_part,0,readneedupdate,2);

//                    if(++timeout_count > 10)
//                    {
//                        timeout_count = 0;
//                        break;
//                    }
//                }while(memcmp(needupdate,readneedupdate,2));
//            }

//            LogOta("Enterbootloader\r\n");
//            g_Flash_Control_Struct.baterry_level = GetBatteryLevel();
//            g_Flash_Control_Struct.dead_zone = GetLocalTimeZone();
//            g_Flash_Control_Struct.goal_step_before_restart = g_motion_goal[0].goal_step;//donna:??????
//            WriteContorlBytesToGenericFlash();
//            OS_DELAY_MS(150);
//            hw_cpm_reboot_system();
//        }

//        else
//        {
//            g_Flash_Control_Struct.baterry_level = GetBatteryLevel();
//            g_Flash_Control_Struct.dead_zone = GetLocalTimeZone();
//            g_Flash_Control_Struct.goal_step_before_restart = g_motion_goal[0].goal_step;//donna:??????
//            WriteContorlBytesToGenericFlash();
//            OS_DELAY_MS(150);
//            hw_cpm_reboot_system();
//        }
//    }

//    else
//    {
//        LogOta("OTAData.package_validity false\r\n");
//        uint8_t needupdate[2];
//        needupdate[0] =0x55;
//        needupdate[1] =0x55;
//        nvms_t para_part = ad_nvms_open(NVMS_FW_UPDATE_FLAG_PART);//added by ryan.wu 20170204

//        if(para_part == NULL)
//        {
//            LogOta("para_part == NULL\r\n");
//        }

//        else
//        {
//            ad_nvms_write(para_part,0,needupdate,2);
//        }

//        LogOta("update fail\r\n");
//        g_Flash_Control_Struct.baterry_level = GetBatteryLevel();
//        g_Flash_Control_Struct.dead_zone = GetLocalTimeZone();
//        g_Flash_Control_Struct.goal_step_before_restart = g_motion_goal[0].goal_step;//donna:??????
//        WriteContorlBytesToGenericFlash();
//        OS_DELAY_MS(150);
//        hw_cpm_reboot_system();
//    }
}
#if 0
uint8_t FirmwareCalcCheckSum(nvms_t src_part)
{

    uint32_t ReadExternalCnt=0;
    uint16_t LoopIndex=0;
    uint8_t  LastFrameByteLeft=0;
    uint16_t CheckSum=0;
    uint16_t index=0;
    uint8_t  Cache_Buffer[256];
    uint16_t CheckSumReadFromFlash;
    uint32_t u32FlashAddr=0;
    uint32_t CacheDataLength=0;

    OTAPackageVerifyRead(src_part,0,Cache_Buffer,48);

    CheckSumReadFromFlash = ((Cache_Buffer[25]) | (Cache_Buffer[26] << 8));


    u32FlashAddr = ((Cache_Buffer[17]) | (Cache_Buffer[18] << 8) | (Cache_Buffer[19] << 16) | (Cache_Buffer[20] << 24));

    CacheDataLength = ((Cache_Buffer[21]) | (Cache_Buffer[22] << 8) | (Cache_Buffer[23] << 16) | (Cache_Buffer[24] << 24));

    //printf("CheckSum u32FlashAddr=0x%.8x \r\n",u32FlashAddr);

    //printf("CheckSum CacheDataLength=0x%.8x \r\n",CacheDataLength);

    LastFrameByteLeft = (CacheDataLength % OTA_EXTERNAL_FLASH_PAGE_SIZE);

    if(LastFrameByteLeft != 0)
    {

        ReadExternalCnt = (CacheDataLength / OTA_EXTERNAL_FLASH_PAGE_SIZE) + 1;

        //printf("CheckSum LastFrameByteLeft != 0 ,ReadExternalCnt =%d \r\n",ReadExternalCnt);
        for(LoopIndex=0; LoopIndex < (ReadExternalCnt-1); LoopIndex++)
        {
            //printf("CheckSum LoopIndex = %d \r\n",LoopIndex);
            OTAPackageVerifyRead(src_part,(u32FlashAddr + LoopIndex * OTA_EXTERNAL_FLASH_PAGE_SIZE) ,Cache_Buffer,OTA_EXTERNAL_FLASH_PAGE_SIZE);

            /*Calc CheckSum each time 64 Bytes*/
            for(index = 0; index < OTA_EXTERNAL_FLASH_PAGE_SIZE; index++)
            {
                CheckSum = (CheckSum<<8) ^ crc16tab[((CheckSum>>8) ^ Cache_Buffer[index])&0x00FF];
            }

        }

        OTAPackageVerifyRead(src_part,(u32FlashAddr + LoopIndex * OTA_EXTERNAL_FLASH_PAGE_SIZE),Cache_Buffer,LastFrameByteLeft);

        for(index = 0; index < LastFrameByteLeft; index++)
        {
            CheckSum = (CheckSum<<8) ^ crc16tab[((CheckSum>>8) ^ Cache_Buffer[index])&0x00FF];
        }


    }

    else
    {

        ReadExternalCnt = CacheDataLength / OTA_EXTERNAL_FLASH_PAGE_SIZE;

        //printf("CheckSum LastFrameByteLeft == 0 ,ReadExternalCnt =%d \r\n",ReadExternalCnt);
        for(LoopIndex=0; LoopIndex < ReadExternalCnt; LoopIndex++)
        {
            //printf("CheckSum LoopIndex = %d \r\n",LoopIndex);
            OTAPackageVerifyRead(src_part,(u32FlashAddr + LoopIndex * OTA_EXTERNAL_FLASH_PAGE_SIZE),Cache_Buffer,OTA_EXTERNAL_FLASH_PAGE_SIZE);

            /*Calc Check Sum each time 64 Bytes*/
            for(index = 0; index < OTA_EXTERNAL_FLASH_PAGE_SIZE; index++)
            {
                CheckSum = (CheckSum<<8) ^ crc16tab[((CheckSum>>8) ^ Cache_Buffer[index])&0x00FF];
            }

        }
    }

    if(CheckSumReadFromFlash == CheckSum)
    {
        //printf("CheckSum=0x%.4x \r\n",CheckSum);
        //printf("CheckSumReadFromFlash=0x%.4x \r\n",CheckSumReadFromFlash);
        return TRUE;
    }

    else
    {
        //printf("CheckSum=0x%.4x \r\n",CheckSum);
        //printf("CheckSumReadFromFlash=0x%.4x \r\n",CheckSumReadFromFlash);
        return FALSE;
    }
}
#else
//uint8_t FirmwareCalcCheckSum(nvms_t src_part)
//{
//    uint8_t  Cache_Buffer[48];
//    uint16_t CheckSumReadFromFlash;
//    OTAPackageVerifyRead(src_part,0,Cache_Buffer,48);
//    CheckSumReadFromFlash = ((Cache_Buffer[25]) | (Cache_Buffer[26] << 8));

//    if(CheckSumReadFromFlash == gOTAData.ota_check_sum)
//    {
//        LogOta("CheckSum=0x%.4x \r\n",gOTAData.ota_check_sum);
//        LogOta("CheckSumReadFromFlash=0x%.4x \r\n",CheckSumReadFromFlash);
//        return TRUE;
//    }

//    else
//    {
//        LogOta("CheckSum=0x%.4x \r\n",gOTAData.ota_check_sum);
//        LogOta("CheckSumReadFromFlash=0x%.4x \r\n",CheckSumReadFromFlash);
//        return FALSE;
//    }
//}
#endif
static void OTAPackageValidity()
{
//    gOTAData.package_validity = FirmwareCalcCheckSum(gOTAData.ota_update_part);
//    LogOta("22gOTAData.timercnt =%d\r\n",gOTAData.timercnt);
//    LogOta("OTAPackageValidity=%d\r\n",gOTAData.package_validity);
//    memset(&Tlvs_value_resp,0,sizeof(TlvItem_t));
//    Tlvs_value_resp.type = 0x01;
//    Tlvs_value_resp.length = 0x01;
//    Tlvs_value_resp.value = &(gOTAData.package_validity);
//    SendFramePack(&Tlvs_value_resp,OTA_SERVICE,OTA_CMD_UPDATE_CHECK_RESULT);

//    if(gOTAData.package_validity == 0x00)
//    {
//        LogOta("gOTAData.package_validity false\r\n");
//        LogOta("ota package invalid\r\n");
//        g_Flash_Control_Struct.baterry_level = GetBatteryLevel();
//        g_Flash_Control_Struct.dead_zone = GetLocalTimeZone();
//        g_Flash_Control_Struct.goal_step_before_restart = g_motion_goal[0].goal_step;//donna:??????
//        WriteContorlBytesToGenericFlash();
//        OS_DELAY_MS(150);
//        hw_cpm_reboot_system();
//    }


}
static void OTAPackageValidReceivedSize(uint32_t  packagevalidsize,uint32_t  packagereceivedsize)
{
    TlvItem_t tlv_file_received_size;
    uint32_t validsize;
    uint32_t receiedsize;
//    LogOta("OTAPackageValidReceivedSize\r\n");

    validsize = Uint32LtoB(packagevalidsize);
    memset(&Tlvs_value_resp,0,sizeof(TlvItem_t));
    Tlvs_value_resp.type = 0x01;
    Tlvs_value_resp.length = 0x04;
    Tlvs_value_resp.value = (uint8_t*)&(validsize);


    receiedsize = Uint32LtoB(packagereceivedsize);
    memset(&tlv_file_received_size,0,sizeof(TlvItem_t));
    tlv_file_received_size.type = 0x02;
    tlv_file_received_size.length = 0x04;
    tlv_file_received_size.value  = (uint8_t*)&(receiedsize);
    TlvItem_AddBrother(&Tlvs_value_resp,&tlv_file_received_size);

    SendFramePack(&Tlvs_value_resp,OTA_SERVICE,OTA_CMD_UPDATE_SIZE_APPEAR);
}
static void OTAUpgradeStatusReponse(void)
{
    GeneralErrorACK(GeneralError[0],OTA_SERVICE,OTA_CMD_UPDATE_STATE_APPEAR);
    return;
}
//2.6.7
static void OTAUpgradeCancel(TlvItem_t *item)
{
    if(NULL == item)
    {
        GeneralErrorACK(GeneralError[6],OTA_SERVICE,OTA_CMD_UPDATE_CANCEL_COMMAND);
        return;
    }

//    LogOta("OTAUpgradeCancel!!!!\r\n");
    OTAVariableInit();
    GeneralErrorACK(GeneralError[0],OTA_SERVICE,OTA_CMD_UPDATE_CANCEL_COMMAND);
    return;
}
void OTAApplyUpgradePackageData(uint32_t fileoffset,uint32_t filelength,uint32_t filebitmap)
{
    uint32_t ota_file_offset;
    uint32_t ota_file_length;
    uint32_t ota_file_bitmap;

    TlvItem_t tlv_ota_file_offset;
    TlvItem_t tlv_ota_file_length;
    TlvItem_t tlv_ota_file_bitmap;


    memset(&Tlvs_value_resp,0,sizeof(TlvItem_t));
    ota_file_offset = Uint32LtoB(fileoffset);
    Tlvs_value_resp.type = 0x01;
    Tlvs_value_resp.length = 0x04;
    Tlvs_value_resp.value = (uint8_t*)&(ota_file_offset);

    ota_file_length = Uint32LtoB(filelength);
    memset(&tlv_ota_file_length,0,sizeof(TlvItem_t));
    tlv_ota_file_length.type = 0x02;
    tlv_ota_file_length.length = 0x04;
    tlv_ota_file_length.value =(uint8_t*)&(ota_file_length);
    TlvItem_AddBrother(&Tlvs_value_resp,&tlv_ota_file_length);

#if 0
    ota_file_bitmap = Uint32LtoB(filebitmap);
    memset(&tlv_ota_file_bitmap,0,sizeof(TlvItem_t));
    tlv_ota_file_bitmap.type = 0x03;
    tlv_ota_file_bitmap.length = 0x04;
    tlv_ota_file_bitmap.value =(uint8_t*)&(ota_file_bitmap);
    TlvItem_AddBrother(&Tlvs_value_resp,&tlv_ota_file_bitmap);
#endif
    SendFramePack(&Tlvs_value_resp,OTA_SERVICE,OTA_CMD_DATA_REPORT_ACTIVELY);

}
uint32_t OTAProgrammFlash(uint32_t address, uint8_t *outbuffer, uint32_t writeLength)
{
//    ad_nvms_write(gOTAData.ota_update_part,address,outbuffer,writeLength);
}
uint32_t OTAReadFlash(uint32_t address, uint8_t *outbuffer, uint32_t readLength)
{
//    ad_nvms_read(gOTAData.ota_update_part,address,outbuffer,readLength);
}

//void OTAPackageVerifyRead(nvms_t handle, uint32_t address, uint8_t *outbuffer, uint32_t len)
//{
//    ad_nvms_read(handle, address, outbuffer,len);
//}
/******************************************************************************/
/** OTA Service Type 2.6
  *
  * @param[in] item_t
  * @param[in] command_id
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/19
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void OTAServiceType(TlvItem_t *item_t,uint8_t command_id)
{
//    Msg_t msg;

//    switch(command_id)
//    {
//        case OTA_CMD_QUERY_UPGRADE_PERMIT:
//            OTACheckDeviceUpdateOrNot(item_t);
//            break;

//        case OTA_CMD_UPGRADE_PARA_CONSULT:

//            break;

//        case OTA_CMD_DATA_REPORT_ACTIVELY:

//            break;

//        case OTA_CMD_UPDATE_CONTENT_TRANS:

//            break;

//        case OTA_CMD_UPDATE_SIZE_APPEAR:

//            break;

//        case OTA_CMD_UPDATE_CHECK_RESULT:

//            break;

//        case OTA_CMD_UPDATE_STATE_APPEAR:

//            break;

//        case OTA_CMD_UPDATE_CANCEL_COMMAND:
//            {

//                OTAUpgradeCancel(item_t);
//                g_Flash_Control_Struct.baterry_level = GetBatteryLevel();
//                g_Flash_Control_Struct.dead_zone = GetLocalTimeZone();
//                g_Flash_Control_Struct.goal_step_before_restart = g_motion_goal[0].goal_step;//donna:??????
//                WriteContorlBytesToGenericFlash();
//                OS_DELAY_MS(150);
//                hw_cpm_reboot_system();

//            }
//            break;

//        case OTA_CMD_APP_UPDATE_STATE:
//            OTAAppReadyOrNot(item_t);

//            if(gOTAData.ota_app_ready == 0x01)
//            {
//                conn_param_update_for_ota();
//                OTAVariableInit();

//                if(g_Flash_Control_Struct.bt_system_flag != 0)
//                {
//                    ancs_client_set_stop_push_message();
//                }

//                /*Tell memory task delte self*/
//                msg = MSG_DELETE_MEMORY_TASK;
//                xQueueSend(g_queue_memory,&msg,(portTickType)2000);
//                OS_DELAY_MS(1000);
//                /*Tell touch task delte self*/
//                OS_TASK_NOTIFY(g_touch_handle,NOTIFY_DELETE_TOUCH_TASK, OS_NOTIFY_SET_BITS);
//                /*Tell hrm task delte self*/
//                OS_TASK_NOTIFY(g_hrsensor_handle,DELETE_HRM_TASK, OS_NOTIFY_SET_BITS);
//                /*Tell gsensor task delte self*/
//                OS_TASK_NOTIFY(sensor_data_reading_task_handle,DELETE_GSENSOR_TASK_MASK,OS_NOTIFY_SET_BITS);

//                msg = MSG_TSK_OTA_BEGIN_MESSAGE;
//                xQueueSend(g_queue_display,&msg,(portTickType)0);

//                msg = MSG_TSK_OTA_BEGIN_MESSAGE;
//                OS_QUEUE_PUT(g_motor_queue,&msg,10);

//                gOTAData.ota_on_going = 0x01;
//                gOTAData.ota_update_part = ad_nvms_open(NVMS_FW_UPDATE_PART);
//                ad_nvms_erase_region(gOTAData.ota_update_part,0,OTA_UPDATE_PART_SIZE);
//                OTATimerInit();
//                OTAApplyUpgradePackageData(gOTAData.ota_file_offset,128/*gOTAData.ota_file_length*/,gOTAData.ota_file_bitmap);
//            }

//            else
//            {
//                LogOta("App not ready\r\n");
//            }

//            break;

//        default:
//            break;
//    }
}
/******************************************************************************/
/** 2.7.1 Get Log File Name
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/20
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void MAIGetLogInformation(TlvItem_t *item)
{
#if 0
    TlvItem_t temp_item_log_file_name;
    uint8_t temp_file_name[8]= {0};
    memset(&temp_item_log_file_name,0,sizeof(TlvItem_t));

    temp_item_log_file_name.type = 0x01;
    temp_item_log_file_name.length = sizeof(temp_file_name);
    temp_item_log_file_name.value = temp_file_name;

    SendFramePack(&temp_item_log_file_name,0x0a,1);
#endif
    GeneralErrorACK(GeneralError[7],0x0a,0x01); //modified by ryan.wu for 20170510 from GeneralError[4] to GeneralError[7]
}
/******************************************************************************/
/** 2.7.2 File Transmit parameter
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/20
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void MAItransParaConsult(TlvItem_t *item)
{
    uint8_t file_rotocal_version[5];
    uint8_t bitmap_enable;
    uint16_t tran_unit_size;
    uint16_t temp_tran_unit_size;
    uint32_t temp_max_apply_data_size;
    uint16_t timeout;
    uint16_t temp_timeout;

    TlvItem_t item_file_rotocal_version;
    TlvItem_t item_tran_bitmap_enable;
    TlvItem_t item_tran_unit_size;
    TlvItem_t item_max_apply_data_size;
    TlvItem_t item_timeout;

    memset(&item_file_rotocal_version,0,sizeof(TlvItem_t));
    memset(&item_tran_bitmap_enable,0,sizeof(TlvItem_t));
    memset(&item_tran_unit_size,0,sizeof(TlvItem_t));
    memset(&item_max_apply_data_size,0,sizeof(TlvItem_t));
    memset(&item_timeout,0,sizeof(TlvItem_t));

    item_file_rotocal_version.type = 0x01;
    item_file_rotocal_version.length = 1;
    item_file_rotocal_version.value = file_rotocal_version;

    item_tran_bitmap_enable.type =0x02;
    item_tran_bitmap_enable.length = 1;
    bitmap_enable = 1;
    item_tran_bitmap_enable.value = &bitmap_enable;
    TlvItem_AddBrother(&item_file_rotocal_version,&item_tran_bitmap_enable);

    item_tran_unit_size.type = 0x03;
    item_tran_unit_size.length = 0x02;
    temp_tran_unit_size = 128;
    tran_unit_size = Uint16LtoB(temp_tran_unit_size);
    item_tran_unit_size.value = (uint8_t*)&(tran_unit_size);
    TlvItem_AddBrother(&item_file_rotocal_version,&item_tran_unit_size);

    item_max_apply_data_size.type = 0x04;
    item_max_apply_data_size.length = 0x04;
    temp_max_apply_data_size = 128;
    tran_unit_size = Uint32LtoB(temp_max_apply_data_size);
    item_max_apply_data_size.value = (uint8_t*)&(tran_unit_size);
    TlvItem_AddBrother(&item_file_rotocal_version,&item_max_apply_data_size);

    item_timeout.type = 0x05;
    item_timeout.length = 0x02;
    temp_timeout = 128;
    timeout = Uint16LtoB(temp_timeout);
    item_timeout.value = (uint8_t*)&(timeout);
    TlvItem_AddBrother(&item_file_rotocal_version,&item_timeout);

    SendFramePack(&item_file_rotocal_version,0x0a,2);
}
/******************************************************************************/
/** 2.7.3 Query Single File Information
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/20
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void MAIQuerySingleInfo(TlvItem_t *item)
{
    uint8_t file_name[5];
    uint32_t file_size;
    uint32_t temp_file_size;
    uint32_t file_crc;
    uint32_t temp_file_crc;

    TlvItem_t item_file_size;
    TlvItem_t item_file_crc;

    memset(&item_file_size,0,sizeof(TlvItem_t));
    memset(&item_file_crc,0,sizeof(TlvItem_t));

    memcpy(file_name,item->value,item->length);

    item_file_size.type =0x02;
    item_file_size.length = 0x04;
    temp_file_size = 1;
    file_size = Uint32LtoB(temp_file_size);
    item_file_size.value = (uint8_t*)&file_size;

    item_file_crc.type = 0x03;
    item_file_crc.length = 0x04;
    temp_file_crc = 128;
    file_crc = Uint16LtoB(temp_file_crc);
    item_file_crc.value = (uint8_t*)&(file_crc);
    TlvItem_AddBrother(&item_file_size,&item_file_crc);

    SendFramePack(&item_file_size,0x0a,3);
}
/******************************************************************************/
/** 2.7.4 Applay File Data From Device
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/20
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void MAIApplyFileTrans(TlvItem_t *item)
{
    TlvItem_t *brotheritem = NULL;
    TlvItem_t *tempitem = NULL;
    uint8_t file_name[5];
    uint8_t apply_file_bitmap[5];
    uint32_t apply_offset;
    uint32_t apply_file_length;

    if(NULL == item)
        return;

    memcpy(file_name,item->value,item->length);

    brotheritem = TlvItem_Next(item);
    TlvItem_ItemInt32Value(brotheritem,&apply_offset,1);

    brotheritem = TlvItem_Next(brotheritem);
    TlvItem_ItemInt32Value(brotheritem,&apply_file_length,1);

    tempitem = TlvItem_ItemWithType(brotheritem,0x04);

    if(tempitem != NULL)
    {
        memcpy(apply_file_bitmap,tempitem->value,tempitem->length);
    }
}
/******************************************************************************/
/** 2.7.5
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/20
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void MAIFileTransActivety(TlvItem_t *item)
{

}
/******************************************************************************/
/** 2.7.6 File Receive Result
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/20
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void MAIFileReceiveResult(TlvItem_t *item)
{
    uint8_t file_validity_result;
    TlvItem_t item_file_validity_result;

    file_validity_result = *(item->value);
    memset(&item_file_validity_result,0,sizeof(TlvItem_t));

    if(file_validity_result)
    {
        item_file_validity_result.type = 0x01;
        item_file_validity_result.length = 0x01;
        item_file_validity_result.value = (uint8_t*)&GeneralError[0];
    }

    else
    {
        item_file_validity_result.type = 0x01;
        item_file_validity_result.length = 0x01;
        item_file_validity_result.value = (uint8_t*)&GeneralError[1];
    }

    SendFramePack(&item_file_validity_result,0x0a,0x06);
}
/******************************************************************************/
/** Maintenance Service Type 2.7
  *
  * @param[in] item_t
  * @param[in] command_id
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/19
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void MaintenanceServiceType(TlvItem_t *item_t,uint8_t command_iden)
{
    MAI_COMMAND_ID_E command_id;
    command_id = (MAI_COMMAND_ID_E)command_iden;

    switch(command_id)
    {
        case MAI_CMD_GET_LOG_INFORMATION:
            MAIGetLogInformation(item_t);
            break;

        case MAI_CMD_TRANS_PARA_CONSULT:
            MAItransParaConsult(item_t);
            break;

        case MAI_CMD_QUERY_SINGAL_INFORMATION:
            MAIQuerySingleInfo(item_t);
            break;

        case MAI_CMD_APPLY_FILE_TRANS:
            MAIApplyFileTrans(item_t);
            break;

        case MAI_CMD_FILE_TRANS_ACTIVELY:
            MAIFileTransActivety(item_t);
            break;

        case MAI_CMD_FILE_RECEIVE_RESULT:
            MAIFileReceiveResult(item_t);
            break;

        case MAI_CMD_SET_LOG_RULE:

            break;

        case MAI_CMD_DATA_SWITCH:

            break;

        default:
            break;
    }
}
/******************************************************************************/
/** Phone Prevent Lose
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/1/11
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
void PhonePreventLose(uint8_t phone_prevent_lose)
{
    memset(&Tlvs_value_resp,0,sizeof(TlvItem_t));
    Tlvs_value_resp.type = 0x01;
    Tlvs_value_resp.length = 0x01;
    Tlvs_value_resp.value = &phone_prevent_lose;

    SendFramePack(&Tlvs_value_resp,0x0b,0x01);
}
/******************************************************************************/
/** BtLoseReminderEnable
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/1/19
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void BtLoseReminderEnable(TlvItem_t *item)
{
    uint8_t  reminder_enable=0;
    uint32_t temp_length=0;

    if(NULL == item)
    {
        return;
    }

    reminder_enable = *TlvItem_ItemValue(item,&temp_length);
    g_reming_switch.ble_reminder_enable = reminder_enable;
    GeneralErrorACK(GeneralError[0],0x0b,0x03);
}
/******************************************************************************/
/** Phone Lost Remind
  *
  * @param[in] item_t
  * @param[in] command_iden
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/1/12
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void PhoneLostRemind(TlvItem_t *item_t,uint8_t command_iden)
{
    PHONE_LOST_REMIND_ID_E command_id;
    command_id = (PHONE_LOST_REMIND_ID_E)command_iden;

    switch(command_id)
    {
        case PHO_CMD_LOST_ALARM:
            //PhonePreventLose(2);
            break;

        case PHO_CMD_LOST_REMIND:
            BtLoseReminderEnable(item_t);
            break;

        default:
            break;
    }
}
/******************************************************************************/
/** Set Cell Phone Language 2.8.1
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/31
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void FonSetCellPhoneLanguage(TlvItem_t *item)
{
//    TlvItem_t *tempitem = NULL;
//    uint8_t length = 0;

//    length = item->length;
//    tempitem = TlvItem_ItemWithType(item,1);

//    if(NULL != tempitem)
//    {
//        memcpy(gUIContext.PhoneLanguage,tempitem->value,(length>6)?6:length);
//    }

//    gUIContext.PhoneLanguage[5] = '\0';

//    GeneralErrorACK(GeneralError[0],12,1);
}
/******************************************************************************/
/** Font Service Type 2.8
  *
  * @param[in] item_t
  * @param[in] command_id
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/19
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void FontServiceType(TlvItem_t *item_t,uint8_t command_iden)
{
    FON_COMMAND_ID_E command_id;
    command_id = (FON_COMMAND_ID_E)command_iden;

    switch(command_id)
    {
        case FON_CMD_CELLPHONE_LANGUAGE:
            FonSetCellPhoneLanguage(item_t);
            break;

        default:
            break;
    }
}
/******************************************************************************/
/** ????,??APP????????????,?????????
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/3/9
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void RunningpProgram(TlvItem_t *item)
{
#if 0
    uint8_t total_sign[32];
    uint8_t plan_sign[32];
    uint8_t type;
    uint8_t version[16] = {0x34,0x2e,0x34,0x2e,0x32,0x2e,0x32,0x00,
                           0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
                          };
    uint8_t sync_size[2] = {0x05,0x07};
    TlvItem_t run_plan_info_struct;
    TlvItem_t run_plan_total_sign;
    TlvItem_t run_plan_sign;
    TlvItem_t algorithm_type;
    TlvItem_t algorithm_version;
    TlvItem_t run_plan_sync_size;
    memset(&run_plan_info_struct,0,sizeof(TlvItem_t));
    run_plan_info_struct.type = 0x01 | CHILD_MASK;
    run_plan_info_struct.length = 0;
    memset(&run_plan_total_sign,0,sizeof(TlvItem_t));
    run_plan_total_sign.type = 0x02;
    run_plan_total_sign.length = 0x02;
    memset(total_sign,0x30,sizeof(total_sign));
    run_plan_total_sign.value = total_sign;
    TlvItem_AddChild(&run_plan_info_struct,&run_plan_total_sign);
    memset(&run_plan_sign,0,sizeof(TlvItem_t));
    run_plan_sign.type = 0x06;
    run_plan_sign.length = 0x04;
    memset(plan_sign,0x30,sizeof(plan_sign));
    run_plan_sign.value = plan_sign;
    TlvItem_AddChild(&run_plan_info_struct,&run_plan_sign);
    memset(&algorithm_type,0,sizeof(TlvItem_t));
    algorithm_type.type = 0x07;
    algorithm_type.length = 0x02;
    type = 1;
    algorithm_type.value = (uint8_t *)&(type);
    TlvItem_AddChild(&run_plan_info_struct,&algorithm_type);
    memset(&algorithm_version,0,sizeof(TlvItem_t));
    algorithm_version.type = 0x08;
    algorithm_version.length = 0x02;
    algorithm_version.value = (uint8_t *)&(version);
    TlvItem_AddChild(&run_plan_info_struct,&algorithm_version);
    memset(&run_plan_sync_size,0,sizeof(TlvItem_t));
    run_plan_sync_size.type = 0x08;
    run_plan_sync_size.length = 0x02;
    run_plan_sync_size.value = (uint8_t *)&(sync_size);
    TlvItem_AddChild(&run_plan_info_struct,&run_plan_sync_size);
    SendFramePack(&run_plan_info_struct,0x16,0x01);
#else

    if(NULL == item)
    {
        return;
    }

    GeneralErrorACK(GeneralError[2],0x16,0x01);
#endif
}
static void RunningpProgramPush(TlvItem_t *item)
{
    if(NULL == item)
    {
        return;
    }

    GeneralErrorACK(GeneralError[2],0x16,0x02);
}
static void RunningpProgramReminder(TlvItem_t *item)
{
    if(NULL == item)
    {
        return;
    }

    GeneralErrorACK(GeneralError[2],0x16,0x03);
}
static void ExerciseMessage(TlvItem_t *item)
{
#if 0
    uint16_t record_count = 0;
    uint16_t workout_record_id = 0;
    uint16_t workout_index_count = 0;
    uint32_t workout_id = 0;
    TlvItem_t item_workout_record_list;
    TlvItem_t item_workout_record_count;
    TlvItem_t item_workout_record_struct;
    TlvItem_t item_workout_record_id;
    TlvItem_t item_workout_index_count;
    TlvItem_t item_workout_id;
    memset(&item_workout_record_list,0,sizeof(TlvItem_t));
    item_workout_record_list.type = 0x01 | CHILD_MASK;
    item_workout_record_list.length = 0;
    memset(&item_workout_record_count,0,sizeof(TlvItem_t));
    item_workout_record_count.type = 0x02;
    item_workout_record_count.length = 0x02;
    record_count = 0;
    record_count = Uint16LtoB(record_count);
    item_workout_record_count.value = (uint8_t *)&(record_count);
    TlvItem_AddChild(&item_workout_record_list,&item_workout_record_count);
    memset(&item_workout_record_struct,0,sizeof(TlvItem_t));
    item_workout_record_struct.type = 0x05 | CHILD_MASK;
    item_workout_record_struct.length = 0;
    memset(&item_workout_id,0,sizeof(TlvItem_t));
    item_workout_id.type = 0x06;
    item_workout_id.length = 0x04;
    workout_id = 0;
    workout_id = Uint16LtoB(workout_id);
    item_workout_id.value = (uint8_t *)&(workout_id);
    TlvItem_AddChild(&item_workout_record_struct,&item_workout_id);
    memset(&item_workout_record_id,0,sizeof(TlvItem_t));
    item_workout_record_id.type = 0x07;
    item_workout_record_id.length = 0x02;
    workout_record_id = 0;
    workout_record_id = Uint16LtoB(workout_record_id);
    item_workout_record_id.value = (uint8_t *)&(workout_record_id);
    TlvItem_AddChild(&item_workout_record_struct,&item_workout_record_id);
    memset(&item_workout_index_count,0,sizeof(TlvItem_t));
    item_workout_index_count.type = 0x08;
    item_workout_index_count.length = 0x02;
    workout_index_count = 0;
    workout_index_count = Uint16LtoB(workout_index_count);
    item_workout_index_count.value = (uint8_t *)&(workout_index_count);
    TlvItem_AddChild(&item_workout_record_struct,&item_workout_index_count);
    TlvItem_AddChild(&item_workout_record_list,&item_workout_record_struct);
    SendFramePack(&item_workout_record_list,0x16,0x04);
#else
    uint16_t record_count = 0;
    TlvItem_t item_workout_record_list;
    TlvItem_t item_workout_record_count;
    memset(&item_workout_record_list,0,sizeof(TlvItem_t));
    item_workout_record_list.type = 0x01 | CHILD_MASK;
    item_workout_record_list.length = 0;
    memset(&item_workout_record_count,0,sizeof(TlvItem_t));
    item_workout_record_count.type = 0x02;
    item_workout_record_count.length = 0x02;
    record_count = 0;
    record_count = Uint16LtoB(record_count);
    item_workout_record_count.value = (uint8_t *)&(record_count);
    TlvItem_AddChild(&item_workout_record_list,&item_workout_record_count);
    SendFramePack(&item_workout_record_list,0x16,0x04);
#endif
}
static void ExerciseServiceType(TlvItem_t *item_t,uint8_t command_iden)
{
    switch(command_iden)
    {
        case 0x01:
            RunningpProgram(item_t);
            break;

        case 0x02:
            RunningpProgramPush(item_t);
            break;

        case 0x03:
            RunningpProgramReminder(item_t);
            break;

        case 0x04:
            ExerciseMessage(item_t);
            break;

        default:
            break;
    }
}
/******************************************************************************/
/** 2.9.1 Notify Device Sport Status
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/23
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void WorInformEquipmentStatus(TlvItem_t *item)
{
#if 0
    uint32_t temp_value = 0;
    uint32_t length = 0;
    uint8_t operator_type = 0;
    uint8_t sport_type = 0;
    uint32_t run_plan_start_date = 0;
    TlvItem_t *child_item = NULL;
    TlvItem_t *brotheritem = NULL;
    TlvItem_t *temp_item = NULL;

    if(NULL == item)
    {
        GeneralErrorACK(GeneralError[5],0x17,0x01);
        return;
    }

    child_item = TlvItem_Child(item); //point to operator_type
    operator_type = *TlvItem_ItemValue(child_item,&length);

    switch(operator_type)
    {
        case 1:
            SingleMotionStart(E_SINGLE_MOTION_SRC_APP);
            OS_TASK_NOTIFY(g_hrsensor_handle,HR_OPEN_MASK, OS_NOTIFY_SET_BITS);
            RealTimeHeartTimerInit();
            break;

        case 2:
            SingleMotionPause(E_SINGLE_MOTION_SRC_APP);
            RealTimeHeartTimerStop();
            break;

        case 3:
            SingleMotionStart(E_SINGLE_MOTION_SRC_APP);
            RealTimeHeartTimerStart();
            break;

        case 4:
            SingleMotionStop(E_SINGLE_MOTION_SRC_APP);
            OS_TASK_NOTIFY(g_hrsensor_handle,HR_CLOSE_MASK, OS_NOTIFY_SET_BITS);
            RealTimeHeartTimerDelete();
            break;

        default:
            break;
    }

    brotheritem = TlvItem_Next(child_item); //point to sport_type
    sport_type = *TlvItem_ItemValue(brotheritem,&length);

    if(0x01 == sport_type)
    {

    }
    else if(0x02 == sport_type)
    {

    }

    temp_item = TlvItem_ItemWithType(brotheritem,0x04);

    if(temp_item != NULL)
    {
        TlvItem_ItemInt32Value(brotheritem,&temp_value,1);
        run_plan_start_date = temp_value;
    }

    GeneralErrorACK(GeneralError[0],0x17,0x01);
#endif
}

#if HRM_VALUE_TEST_IN_APP
//donna change: 2.10.3 ?????????ê
extern uint8_t g_acc_data_length;//donna
//void GetRealtimeHeartrateDataMore(uint8_t hearrate, uint8_t SQR, uint8_t *ppg_raw_data, uint8_t *acc_infor )
void GetRealtimeHeartrateDataMore(uint8_t *ppg_raw_data,uint8_t ppg_size_len, uint8_t *acc_infor)
{
#if 0
//    printf("GetRealtimeHeartrateDataMore\r\n");//donna
    uint32_t temp_hr_time;
    uint32_t hr_time;
    uint8_t HeartRateTime[2];//donna

    TlvItem_t item_heartrate_list;
    TlvItem_t item_heartrate_struct;
    TlvItem_t item_heartrate_info;
    TlvItem_t item_time_info;

    //donna
    TlvItem_t item_ppg_raw_data;
    TlvItem_t item_acc_infor;

    memset(&item_heartrate_list,0,sizeof(TlvItem_t));

    item_heartrate_list.type = 0x01 | CHILD_MASK;//????????type??? CHILD_MASK
    item_heartrate_list.length = 0;

    memset(&item_heartrate_struct,0,sizeof(TlvItem_t));
    memset(&item_heartrate_info,0,sizeof(TlvItem_t));
    memset(&item_time_info,0,sizeof(TlvItem_t));

    memset(&item_ppg_raw_data,0,sizeof(TlvItem_t));//donna
    memset(&item_acc_infor,0,sizeof(TlvItem_t));//donna

    item_heartrate_struct.type = 0x02 | CHILD_MASK;
    item_heartrate_struct.length = 0;

    item_heartrate_info.type = 0x03;
    item_heartrate_info.length = 0x2;
    HeartRateTime[0] = hearrate;
    HeartRateTime[1] = SQR;
//        item_heartrate_info.value = &(HeartRateTime[0]);
    item_heartrate_info.value = (uint8_t *)HeartRateTime;

    TlvItem_AddChild(&item_heartrate_struct,&item_heartrate_info);

    item_time_info.type = 0x04;
    item_time_info.length = 0x4;
    hr_time = Uint32LtoB(temp_hr_time);
    item_time_info.value = (uint8_t*)&hr_time;

    TlvItem_AddChild(&item_heartrate_struct,&item_time_info);

    item_ppg_raw_data.type = 0x05;
    item_ppg_raw_data.length = 48;//donnaú??length???ì??????????ú?ì?????length??4?????value????
    item_ppg_raw_data.value = ppg_raw_data;

    TlvItem_AddChild(&item_heartrate_struct,&item_ppg_raw_data);

    item_acc_infor.type = 0x06;
    item_acc_infor.length = g_acc_data_length;
    item_acc_infor.value = acc_infor;

    TlvItem_AddChild(&item_heartrate_struct,&item_acc_infor);

//        printf("g_acc_data_length: %d\r\n",g_acc_data_length);
//        printf("acc_infor[0]: %d\r\n", acc_infor[0]);



//        printf("temp_hr_time = 0x%.8x\r\n",temp_hr_time);//donna
    TlvItem_AddChild(&item_heartrate_list,&item_heartrate_struct);

    SendFramePack(&item_heartrate_list,0x19,0x03);
#endif


    TlvItem_t item_ppg_raw_data;
    TlvItem_t item_acc_infor;
    memset(&item_ppg_raw_data,0,sizeof(TlvItem_t));//donna
    memset(&item_acc_infor,0,sizeof(TlvItem_t));//donna
    item_ppg_raw_data.type = 0x05;
    item_ppg_raw_data.length = ppg_size_len;
    item_ppg_raw_data.value = ppg_raw_data;

    item_acc_infor.type = 0x06;
    item_acc_infor.length = g_acc_data_length;
    item_acc_infor.value = acc_infor;
    TlvItem_AddBrother(&item_ppg_raw_data,&item_acc_infor);
    SendFramePack(&item_ppg_raw_data,0x19,0x03);
}

#endif
/******************************************************************************/
/** 2.9.3 Query Equipment Status
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/23
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void WorQueryEquipmentStatus(TlvItem_t *item)
{
    TlvItem_t item_operator_struct;
    TlvItem_t item_traing_monitor_status;

    uint8_t traing_monitor_status = 0;

    memset(&item_operator_struct,0,sizeof(TlvItem_t));
    memset(&item_traing_monitor_status,0,sizeof(TlvItem_t));

    if(NULL == item)
    {
        GeneralErrorACK(GeneralError[5],0x17,0x03);
        return;
    }

    item_operator_struct.type = 0x01 | CHILD_MASK;
    item_operator_struct.length = 0;

    item_traing_monitor_status.type = 0x02;
    item_traing_monitor_status.length = 0x01;
    item_traing_monitor_status.value = &traing_monitor_status;
    TlvItem_AddChild(&item_operator_struct,&item_traing_monitor_status);

    SendFramePack(&item_operator_struct,0x17,0x03);
}
/******************************************************************************/
/** 2.9.4 Workout Motion Remind
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/23
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void WorMotionRemind(TlvItem_t *item)
{
//    TlvItem_t *temp_item = NULL;
//    uint8_t  reminder_type;
//    uint8_t  run_phrase_number;
//    uint16_t run_phrase_variable[2];
//    uint32_t distance_info;
//    uint32_t time_info;
//    uint8_t  hr_value_info;
//    uint8_t  hr_status_info;

//    TlvItem_t item_reminder_struct;
//    TlvItem_t item_reminder_type;
//    TlvItem_t item_run_phrase_number;
//    TlvItem_t item_run_phrase_variable;
//    TlvItem_t item_distance_info;
//    TlvItem_t item_time_info;
//    TlvItem_t item_hr_value_info;
//    TlvItem_t item_hr_status_info;

//    memset(&item_reminder_struct,0,sizeof(TlvItem_t));
//    item_reminder_struct.type = 0x01 | CHILD_MASK;
//    item_reminder_struct.length = 0;

//    memset(&item_reminder_type,0,sizeof(TlvItem_t));
//    item_reminder_type.type = 0x03;
//    item_reminder_type.length = 0x01;
//    item_reminder_type.value = &reminder_type;
//    TlvItem_AddChild(&item_reminder_struct,&item_reminder_type);

//    memset(&item_run_phrase_number,0,sizeof(TlvItem_t));
//    item_run_phrase_number.type = 0x04;
//    item_run_phrase_number.length = 0x01;
//    item_run_phrase_number.value = &run_phrase_number;
//    TlvItem_AddChild(&item_reminder_struct,&item_run_phrase_number);

//    memset(&item_run_phrase_variable,0,sizeof(TlvItem_t));
//    item_run_phrase_variable.type = 0x05;
//    item_run_phrase_variable.length = 0x04;
//    item_run_phrase_variable.value = run_phrase_variable;
//    TlvItem_AddChild(&item_reminder_struct,&item_run_phrase_variable);

//    memset(&item_distance_info,0,sizeof(TlvItem_t));
//    item_distance_info.type = 0x06;
//    item_distance_info.length = 0x04;
//    distance_info = Uint32LtoB(distance_info);
//    item_distance_info.value = (uint8_t*)&(distance_info);
//    TlvItem_AddChild(&item_reminder_struct,&item_distance_info);

//    memset(&item_time_info,0,sizeof(TlvItem_t));
//    item_time_info.type = 0x07;
//    item_time_info.length = 0x04;
//    time_info = Uint32LtoB(time_info);
//    item_time_info.value = (uint8_t*)&(time_info);
//    TlvItem_AddChild(&item_reminder_struct,&item_time_info);

//    memset(&item_hr_value_info,0,sizeof(TlvItem_t));
//    item_hr_value_info.type = 0x08;
//    item_hr_value_info.length = 0x01;
//    item_hr_value_info.value = &hr_value_info;
//    TlvItem_AddChild(&item_reminder_struct,&item_hr_value_info);

//    memset(&item_hr_status_info,0,sizeof(TlvItem_t));
//    item_hr_status_info.type = 0x09;
//    item_hr_status_info.length = 0x01;
//    item_hr_status_info.value = &hr_status_info;
//    TlvItem_AddChild(&item_reminder_struct,&item_hr_status_info);

//    SendFramePack(&item_reminder_struct,0x17,0x06);
}
/******************************************************************************/
/** 2.9.5 Upload Singal Motion Record
  *
  * @param[in] heartreat_start_time
  * @param[in] heartreat_end_time
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/26
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void UploadSingalMotionRecord(uint32_t query_start_time,uint32_t query_end_time)
{
//    uint16_t record_count = 0;
//    uint16_t workout_record_id[5] = {0};
//    uint16_t workout_index_count[5] = {0};

//    TlvItem_t item_workout_record_list;
//    TlvItem_t item_workout_record_count;
//    TlvItem_t item_workout_record_struct[5];
//    TlvItem_t item_workout_record_id[5];
//    TlvItem_t item_workout_index_count[5];

//    //SingleMotionFlashSaveAreaInfoCntGet(query_start_time,query_end_time,g_single_motion_buffer,&g_single_motion_cnt);
//    memcpy((uint8_t*)g_single_motion_buffer,g_tMotionSleepDb.tlv_buffer,g_tMotionSleepDb.tlv_len);
//    g_single_motion_cnt = g_tMotionSleepDb.tlv_len / sizeof(single_motion_detail_t);
//    memset(&item_workout_record_list,0,sizeof(TlvItem_t));
//    item_workout_record_list.type = 0x01 | CHILD_MASK;
//    item_workout_record_list.length = 0;

//    memset(&item_workout_record_count,0,sizeof(TlvItem_t));
//    item_workout_record_count.type = 0x02;
//    item_workout_record_count.length = 0x02;
//    record_count = g_single_motion_cnt;
//    record_count = Uint16LtoB(record_count);
//    item_workout_record_count.value = (uint8_t *)&(record_count);
//    TlvItem_AddChild(&item_workout_record_list,&item_workout_record_count);

//    for(uint8_t i = 0; i < g_single_motion_cnt; i++)
//    {
//        memset(&item_workout_record_struct[i],0,sizeof(TlvItem_t));
//        item_workout_record_struct[i].type = 0x05 | CHILD_MASK;
//        item_workout_record_struct[i].length = 0;

//        memset(&item_workout_record_id[i],0,sizeof(TlvItem_t));
//        item_workout_record_id[i].type = 0x06;
//        item_workout_record_id[i].length = 0x02;
//        workout_record_id[i] = g_single_motion_buffer[i].id;
//        workout_record_id[i] = Uint16LtoB(workout_record_id[i]);
//        item_workout_record_id[i].value = (uint8_t *)&(workout_record_id[i]);
//        TlvItem_AddChild(&item_workout_record_struct[i],&item_workout_record_id[i]);

//        memset(&item_workout_index_count[i],0,sizeof(TlvItem_t));
//        item_workout_index_count[i].type = 0x07;
//        item_workout_index_count[i].length = 0x02;
//        workout_index_count[i] = g_single_motion_buffer[i].frame_cnt;
//        workout_index_count[i] = Uint16LtoB(workout_index_count[i]);
//        item_workout_index_count[i].value = (uint8_t *)&(workout_index_count[i]);
//        TlvItem_AddChild(&item_workout_record_struct[i],&item_workout_index_count[i]);

//        TlvItem_AddChild(&item_workout_record_list,&item_workout_record_struct[i]);
//    }

//    SendFramePack(&item_workout_record_list,0x17,0x07);
}
/******************************************************************************/
/** 2.9.5 Get Singal Motion Record
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/26
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void WorGetSingalMotionRecord(TlvItem_t *item)
{
//    TlvItem_t *item_start_time = NULL;
//    TlvItem_t *item_end_time = NULL;

//    uint32_t query_start_time = 0;
//    uint32_t query_end_time = 0;

//    if(NULL == item)
//    {
//        GeneralErrorACK(GeneralError[5],0x17,0x07);
//        return;
//    }

//    item_start_time = TlvItem_ItemWithType(item,3);

//    if(NULL != item_start_time)
//    {
//        TlvItem_ItemInt32Value(item_start_time,&query_start_time,1);
//    }

//    item_end_time = TlvItem_ItemWithType(item,4);

//    if(NULL != item_end_time)
//    {
//        TlvItem_ItemInt32Value(item_end_time,&query_end_time,1);
//    }

//    //UploadSingalMotionRecord(query_start_time,query_end_time);
//    g_tMotionSleepDb.start_timestamp_request = query_start_time;
//    g_tMotionSleepDb.end_timestamp_request = query_end_time;

//    if(sensor_data_reading_task_handle != NULL)
//    {
//        OS_TASK_NOTIFY(sensor_data_reading_task_handle,
//                       SINGLE_MOTION_RECORD_GET_NOTIFY,
//                       OS_NOTIFY_SET_BITS);
//    }
}
/******************************************************************************/
/** 2.9.6 Upload Singla Motion Record Data
  *
  * @param[in]
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/26
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void UploadSingalMotionRecordData(uint16_t workout_record_id)
{
//    uint8_t  workout_status = 0;
//    uint32_t workout_start_time = 0;
//    uint32_t workout_end_time = 0;
//    uint32_t workout_calorie = 0;
//    uint32_t workout_distance = 0;
//    uint32_t workout_step = 0;
//    uint32_t workout_total_time = 0;

//    uint32_t workout_exercise_duration = 0;
//    uint16_t workout_speed = 0;
//    single_motion_type_e  workout_type = 0;

//    uint8_t  workout_HrABS_peak[2];

//    TlvItem_t item_workout_record_struct;
//    TlvItem_t item_workout_record_id;
//    TlvItem_t item_workout_status;
//    TlvItem_t item_workout_start_time;
//    TlvItem_t item_workout_end_time;
//    TlvItem_t item_workout_calorie;
//    TlvItem_t item_workout_distance;
//    TlvItem_t item_workout_step;
//    TlvItem_t item_workout_tatal_time;
//    //TlvItem_t item_workout_speed;
//    TlvItem_t item_workout_hr_abs_peak;
//    TlvItem_t item_workout_exercise_duration;
//    TlvItem_t item_workout_type;
//    uint8_t index = 0;
//    uint8_t lop = 0;

//    for(lop = 0; lop < g_single_motion_cnt; lop++)
//    {
//        if(g_single_motion_buffer[lop].id == workout_record_id)
//        {
//            index = lop;
//            break;
//        }
//    }

//    if(lop >= g_single_motion_cnt)
//    {
//        index = 0;
//    }

//    //SingleMotionInfoGet(E_SINGLE_MOTION_SRC_UI, &workout_type,&workout_start_time,&workout_end_time,&motion_result);
//    workout_start_time = g_single_motion_buffer[index].single_motion_start_timestamp;
//    workout_end_time = g_single_motion_buffer[index].single_motion_stop_timestamp;
//    workout_exercise_duration = workout_end_time - workout_start_time;

//    workout_calorie    = g_single_motion_buffer[index].motion_info.motion_colorie;
//    workout_distance   = g_single_motion_buffer[index].motion_info.motion_distance;
//    workout_step       = g_single_motion_buffer[index].motion_info.motion_step;
//    workout_total_time = g_single_motion_buffer[index].motion_info.motion_duration;
//    workout_type       = g_single_motion_buffer[index].motion_type;

//    memset(&item_workout_record_struct,0,sizeof(TlvItem_t));
//    item_workout_record_struct.type = 0x01 | CHILD_MASK;
//    item_workout_record_struct.length = 0;

//    memset(&item_workout_record_id,0,sizeof(TlvItem_t));
//    item_workout_record_id.type = 0x02;
//    item_workout_record_id.length = 0x02;
//    workout_record_id = Uint16LtoB(workout_record_id);
//    item_workout_record_id.value = (uint8_t *)&(workout_record_id);
//    TlvItem_AddChild(&item_workout_record_struct,&item_workout_record_id);

//    memset(&item_workout_status,0,sizeof(TlvItem_t));
//    item_workout_status.type = 0x03;
//    item_workout_status.length = 0x01;
//    workout_status = 1;
//    item_workout_status.value = (uint8_t *)&workout_status;
//    TlvItem_AddChild(&item_workout_record_struct,&item_workout_status);

//    memset(&item_workout_start_time,0,sizeof(TlvItem_t));
//    item_workout_start_time.type = 0x04;
//    item_workout_start_time.length = 0x04;
//    workout_start_time = Uint32LtoB(workout_start_time);
//    item_workout_start_time.value = (uint8_t *)&(workout_start_time);
//    TlvItem_AddChild(&item_workout_record_struct,&item_workout_start_time);

//    memset(&item_workout_end_time,0,sizeof(TlvItem_t));
//    item_workout_end_time.type = 0x05;
//    item_workout_end_time.length = 0x04;
//    workout_end_time = Uint32LtoB(workout_end_time);
//    item_workout_end_time.value = (uint8_t *)&(workout_end_time);
//    TlvItem_AddChild(&item_workout_record_struct,&item_workout_end_time);

//    memset(&item_workout_calorie,0,sizeof(TlvItem_t));
//    item_workout_calorie.type = 0x06;
//    item_workout_calorie.length = 0x04;
//    workout_calorie = Uint32LtoB(workout_calorie);
//    item_workout_calorie.value = (uint8_t *)&(workout_calorie);
//    TlvItem_AddChild(&item_workout_record_struct,&item_workout_calorie);

//    memset(&item_workout_distance,0,sizeof(TlvItem_t));
//    item_workout_distance.type = 0x07;
//    item_workout_distance.length = 0x04;
//    workout_distance = Uint32LtoB(workout_distance);
//    item_workout_distance.value = (uint8_t *)&(workout_distance);
//    TlvItem_AddChild(&item_workout_record_struct,&item_workout_distance);

//    memset(&item_workout_step,0,sizeof(TlvItem_t));
//    item_workout_step.type = 0x08;
//    item_workout_step.length = 0x04;
//    workout_step = Uint32LtoB(workout_step);
//    item_workout_step.value = (uint8_t *)&(workout_step);
//    TlvItem_AddChild(&item_workout_record_struct,&item_workout_step);

//    memset(&item_workout_tatal_time,0,sizeof(TlvItem_t));
//    item_workout_tatal_time.type = 0x09;
//    item_workout_tatal_time.length = 0x04;
//    workout_total_time = Uint32LtoB(workout_total_time);
//    item_workout_tatal_time.value = (uint8_t *)&(workout_total_time);
//    TlvItem_AddChild(&item_workout_record_struct,&item_workout_tatal_time);

//    //memset(&item_workout_speed,0,sizeof(TlvItem_t));
//    //item_workout_speed.type = 0x0A;
//    //item_workout_speed.length = 0x02;
//    //workout_speed = 0;
//    //workout_speed = Uint16LtoB(workout_speed);
//    //item_workout_speed.value = (uint8_t *)&(workout_speed);
//    //TlvItem_AddChild(&item_workout_record_struct,&item_workout_speed);

//    memset(&item_workout_hr_abs_peak,0,sizeof(TlvItem_t));
//    item_workout_hr_abs_peak.type = 0x0C;
//    item_workout_hr_abs_peak.length = 0x02;
//    workout_HrABS_peak[0] = 110;
//    workout_HrABS_peak[1] = 100;
//    item_workout_hr_abs_peak.value = workout_HrABS_peak;
//    TlvItem_AddChild(&item_workout_record_struct,&item_workout_hr_abs_peak);

//    memset(&item_workout_exercise_duration,0,sizeof(TlvItem_t));
//    item_workout_exercise_duration.type = 0x12;
//    item_workout_exercise_duration.length = 0x04;
//    workout_exercise_duration = Uint32LtoB(workout_exercise_duration);
//    item_workout_exercise_duration.value = (uint8_t *)&(workout_exercise_duration);
//    TlvItem_AddChild(&item_workout_record_struct,&item_workout_exercise_duration);

//    memset(&item_workout_type,0,sizeof(TlvItem_t));
//    item_workout_type.type = 0x14;
//    item_workout_type.length = 0x01;
//    item_workout_type.value = (uint8_t *)&workout_type;
//    TlvItem_AddChild(&item_workout_record_struct,&item_workout_type);

//    SendFramePack(&item_workout_record_struct,0x17,0x08);
//    //SendFramePackEncrypt(&item_workout_record_struct,0x17,0x08);
}
/******************************************************************************/
/** 2.9.6 Get Singal Motion Record Data
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/26
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void WorGetSingalMotionRecordData(TlvItem_t *item)
{
    uint16_t workout_record_id = 0;
    TlvItem_t *child_item = NULL;

    if(NULL == item)
    {
        GeneralErrorACK(GeneralError[5],0x17,0x08);
        return;
    }

    child_item = TlvItem_Child(item); //point to start_time

    if(NULL == child_item)
    {
        return;
    }

    else
    {
        TlvItem_ItemInt16Value(child_item,&workout_record_id,1);
    }

    UploadSingalMotionRecordData(workout_record_id);
}
/******************************************************************************/
/** 2.9.7 Device Appear Singal Motion Data
  *
  * @param[in]
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/26
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void WorAppearSingalMotionData(uint16_t workout_record_id)
{
//    uint8_t  workout_status = 0;
//    uint32_t workout_start_time = 0;
//    uint32_t workout_end_time = 0;
//    single_motion_t motion_result;
//    single_motion_type_e  workout_type = 0;
//    uint8_t  workout_HrABS_peak[2];

//    TlvItem_t item_workout_record_struct;
//    TlvItem_t item_workout_record_id;
//    TlvItem_t item_workout_status;
//    TlvItem_t item_workout_start_time;
//    TlvItem_t item_workout_end_time;
//    TlvItem_t item_workout_calorie;
//    TlvItem_t item_workout_distance;
//    TlvItem_t item_workout_step;
//    TlvItem_t item_workout_tatal_time;
//    TlvItem_t item_workout_hr_abs_peak;

//    SingleMotionInfoGet(E_SINGLE_MOTION_SRC_UI, &workout_type,&workout_start_time,&workout_end_time,&motion_result);
//    memset(&item_workout_record_struct,0,sizeof(TlvItem_t));
//    item_workout_record_struct.type = 0x01 | CHILD_MASK;
//    item_workout_record_struct.length = 0;

//    memset(&item_workout_record_id,0,sizeof(TlvItem_t));
//    item_workout_record_id.type = 0x02;
//    item_workout_record_id.length = 0x02;
//    workout_record_id = 0;
//    workout_record_id = Uint16LtoB(workout_record_id);
//    item_workout_record_id.value = (uint8_t *)&(workout_record_id);
//    TlvItem_AddChild(&item_workout_record_struct,&item_workout_record_id);

//    memset(&item_workout_status,0,sizeof(TlvItem_t));
//    item_workout_status.type = 0x03;
//    item_workout_status.length = 0x01;
//    workout_status = 1;
//    item_workout_status.value = (uint8_t *)&workout_status;
//    TlvItem_AddChild(&item_workout_record_struct,&item_workout_status);

//    memset(&item_workout_start_time,0,sizeof(TlvItem_t));
//    item_workout_start_time.type = 0x04;
//    item_workout_start_time.length = 0x04;
//    workout_start_time = Uint32LtoB(workout_start_time);
//    item_workout_start_time.value = (uint8_t *)&(workout_start_time);
//    TlvItem_AddChild(&item_workout_record_struct,&item_workout_start_time);

//    memset(&item_workout_end_time,0,sizeof(TlvItem_t));
//    item_workout_end_time.type = 0x05;
//    item_workout_end_time.length = 0x04;
//    workout_end_time = Uint32LtoB(workout_end_time);
//    item_workout_end_time.value = (uint8_t *)&(workout_end_time);
//    TlvItem_AddChild(&item_workout_record_struct,&item_workout_end_time);

//    memset(&item_workout_calorie,0,sizeof(TlvItem_t));
//    item_workout_calorie.type = 0x06;
//    item_workout_calorie.length = 0x04;
//    motion_result.motion_colorie = Uint32LtoB(motion_result.motion_colorie);
//    item_workout_calorie.value = (uint8_t *)&(motion_result.motion_colorie);
//    TlvItem_AddChild(&item_workout_record_struct,&item_workout_calorie);

//    memset(&item_workout_distance,0,sizeof(TlvItem_t));
//    item_workout_distance.type = 0x07;
//    item_workout_distance.length = 0x04;
//    motion_result.motion_distance = Uint32LtoB(motion_result.motion_distance);
//    item_workout_distance.value = (uint8_t *)&(motion_result.motion_distance);
//    TlvItem_AddChild(&item_workout_record_struct,&item_workout_distance);

//    memset(&item_workout_step,0,sizeof(TlvItem_t));
//    item_workout_step.type = 0x08;
//    item_workout_step.length = 0x04;
//    motion_result.motion_step = Uint32LtoB(motion_result.motion_step);
//    item_workout_step.value = (uint8_t *)&(motion_result.motion_step);
//    TlvItem_AddChild(&item_workout_record_struct,&item_workout_step);

//    memset(&item_workout_tatal_time,0,sizeof(TlvItem_t));
//    item_workout_tatal_time.type = 0x09;
//    item_workout_tatal_time.length = 0x04;
//    motion_result.motion_duration = Uint32LtoB(motion_result.motion_duration);
//    item_workout_tatal_time.value = (uint8_t *)&(motion_result.motion_duration);
//    TlvItem_AddChild(&item_workout_record_struct,&item_workout_tatal_time);

//    memset(&item_workout_hr_abs_peak,0,sizeof(TlvItem_t));
//    item_workout_hr_abs_peak.type = 0x0C;
//    item_workout_hr_abs_peak.length = 0x02;
//    workout_HrABS_peak[0] = 110;
//    workout_HrABS_peak[1] = 100;
//    item_workout_hr_abs_peak.value = workout_HrABS_peak;
//    TlvItem_AddChild(&item_workout_record_struct,&item_workout_hr_abs_peak);

//    SendFramePack(&item_workout_record_struct,0x17,0x09);
}
/******************************************************************************/
/** 4.9.8 Upload Singal Motion record Detailed Information
  *
  * @param[in] workout_record_id
  * @param[in] workout_data_index
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/26
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
#if 0
static void UploadSingalmotionDetailedInfo(uint16_t workout_record_id,uint16_t workout_data_index)
{
    uint32_t workout_time = 0;

    uint8_t  workout_data_header[14]= {0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,
                                       0x05,0x00,0x18,0x04,0x00,0x07
                                      };
    uint8_t  workout_data[96]= {0x58,0x00,0x5a,0x08,0x5c,0x00,0x55,0x09,
                                0x62,0x00,0x5a,0x08,0x66,0x00,0x55,0x09,
                                0x6e,0x00,0x5a,0x08,
                               };

    TlvItem_t item_workout_data_struct;
    TlvItem_t item_workout_record_id;
    TlvItem_t item_workout_data_index;
    TlvItem_t item_workout_data_header;
    TlvItem_t item_workout_data;

    memset(&item_workout_data_struct,0,sizeof(TlvItem_t));
    memset(&item_workout_record_id,0,sizeof(TlvItem_t));
    memset(&item_workout_data_index,0,sizeof(TlvItem_t));
    memset(&item_workout_data_header,0,sizeof(TlvItem_t));
    memset(&item_workout_data,0,sizeof(TlvItem_t));

    workout_data_header[1] = (uint8_t)workout_record_id;
    workout_data_header[3] = (uint8_t)workout_data_index;
    workout_time = g_single_motion_buffer[workout_record_id].single_motion_start_timestamp;
    workout_time = Uint32LtoB(workout_time);
    memcpy(&workout_data_header[4],(uint8_t *)&workout_time,4);

    for(uint8_t i=0; i < 24; i++)
    {
        //if(g_dynamic_heart[i] != 0)
        //{
        workout_data[i*4] = g_dynamic_heart[i];
        LogProtocol("heart: %x\r\n",workout_data[i*4]);
        //}
    }

    item_workout_data_struct.type = 0x01 | CHILD_MASK;
    item_workout_data_struct.length = 0;

    item_workout_record_id.type = 0x02;
    item_workout_record_id.length = 0x02;
    workout_record_id = Uint16LtoB(workout_record_id);
    item_workout_record_id.value = (uint8_t *)&(workout_record_id);
    TlvItem_AddChild(&item_workout_data_struct,&item_workout_record_id);

    item_workout_data_index.type = 0x03;
    item_workout_data_index.length = 0x02;
    workout_data_index = Uint16LtoB(workout_data_index);
    item_workout_data_index.value = (uint8_t *)&(workout_data_index);
    TlvItem_AddChild(&item_workout_data_struct,&item_workout_data_index);

    item_workout_data_header.type = 0x04;
    item_workout_data_header.length = sizeof(workout_data_header);
    item_workout_data_header.value = workout_data_header;/**<must be modify */
    TlvItem_AddChild(&item_workout_data_struct,&item_workout_data_header);

    item_workout_data.type = 0x05;
    item_workout_data.length = sizeof(workout_data);
    item_workout_data.value = workout_data;/**<must be modify */
    TlvItem_AddChild(&item_workout_data_struct,&item_workout_data);

    SendFramePack(&item_workout_data_struct,0x17,0x0A);
    //SendFramePackEncrypt(&item_workout_data_struct,0x17,0x0A);
}
#else
static void UploadSingalmotionDetailedInfo(uint16_t workout_record_id,uint16_t workout_data_index)
{
//    TlvItem_t item_workout_data_struct;
//    TlvItem_t item_workout_record_id;
//    TlvItem_t item_workout_data_index;
//    TlvItem_t item_workout_data_header;
//    TlvItem_t item_workout_data;
//    single_motion_frame_t *frame = (single_motion_frame_t*)g_tMotionSleepDb.tlv_buffer;

//    memset(&item_workout_data_struct,0,sizeof(TlvItem_t));
//    memset(&item_workout_record_id,0,sizeof(TlvItem_t));
//    memset(&item_workout_data_index,0,sizeof(TlvItem_t));
//    memset(&item_workout_data_header,0,sizeof(TlvItem_t));
//    memset(&item_workout_data,0,sizeof(TlvItem_t));

//    item_workout_data_struct.type = 0x01 | CHILD_MASK;
//    item_workout_data_struct.length = 0;

//    item_workout_record_id.type = 0x02;
//    item_workout_record_id.length = 0x02;
//    workout_record_id = Uint16LtoB(workout_record_id);
//    item_workout_record_id.value = (uint8_t *)&(workout_record_id);
//    TlvItem_AddChild(&item_workout_data_struct,&item_workout_record_id);

//    item_workout_data_index.type = 0x03;
//    item_workout_data_index.length = 0x02;
//    workout_data_index = Uint16LtoB(workout_data_index);
//    item_workout_data_index.value = (uint8_t *)&(workout_data_index);
//    TlvItem_AddChild(&item_workout_data_struct,&item_workout_data_index);

//    item_workout_data_header.type = 0x04;
//    item_workout_data_header.length = sizeof(single_motion_frame_header_t);
//    item_workout_data_header.value = &frame->frame_header;/**<must be modify */
//    TlvItem_AddChild(&item_workout_data_struct,&item_workout_data_header);

//    item_workout_data.type = 0x05;
//    item_workout_data.length = g_tMotionSleepDb.tlv_len - sizeof(single_motion_frame_header_t);
//    item_workout_data.value = frame->frame_data;/**<must be modify */
//    TlvItem_AddChild(&item_workout_data_struct,&item_workout_data);

//    SendFramePack(&item_workout_data_struct,0x17,0x0A);
//    //SendFramePackEncrypt(&item_workout_data_struct,0x17,0x0A);
}
#endif
/******************************************************************************/
/** 2.9.8 Get Singla Motion Record Detailed Information
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/26
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void WorGetSingalmotionDetailedInfo(TlvItem_t *item)
{
//    uint16_t temp_value = 0;
//    uint16_t workout_record_id = 0;
//    uint16_t workout_data_index = 0;

//    TlvItem_t *item_record_id = NULL;
//    TlvItem_t *item_data_index = NULL;

//    if(NULL == item)
//    {
//        GeneralErrorACK(GeneralError[5],0x17,0x0A);
//        return;
//    }

//    item_record_id = TlvItem_ItemWithType(item,2);

//    if(NULL != item_record_id)
//    {
//        TlvItem_ItemInt16Value(item_record_id,&workout_record_id,1);
//    }

//    item_data_index = TlvItem_ItemWithType(item,3);

//    if(NULL != item_data_index)
//    {
//        TlvItem_ItemInt16Value(item_data_index,&workout_data_index,1);
//    }

//    //UploadSingalmotionDetailedInfo(workout_record_id,workout_data_index);
//    g_tMotionSleepDb.page_timestamp = workout_record_id;
//    g_tMotionSleepDb.page_index_request = workout_data_index;

//    if(sensor_data_reading_task_handle != NULL)
//    {
//        OS_TASK_NOTIFY(sensor_data_reading_task_handle,
//                       SINGLE_MOTION_DETAIL_GET_NOTIFY,
//                       OS_NOTIFY_SET_BITS);
//    }
}
/******************************************************************************/
/** Workout Service Type 2.9
  *
  * @param[in] item_t
  * @param[in] command_id
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/19
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void WorkoutServiceType(TlvItem_t *item_t,uint8_t command_iden)
{
    WOR_COMMAND_ID_E command_id;
    command_id = (WOR_COMMAND_ID_E)command_iden;

    switch(command_id)
    {
        case WOR_CMD_INFORM_EQUIPMENT_STATUS:
            WorInformEquipmentStatus(item_t);
            break;

        case WOR_CMD_APPEAR_EQUIPMENT_STATUS:

            break;

        case WOR_CMD_QUERY_EQUIPMENT_STATUS:
            WorQueryEquipmentStatus(item_t);
            break;

        case WOR_CMD_MOTION_REMIND:
            WorMotionRemind(item_t);
            break;

        case WOR_CMD_GET_SINGAL_MOTION_RECORD:
            WorGetSingalMotionRecord(item_t);
            break;

        case WOR_CMD_GET_SINGAL_MOTION_RECORD_DATA:
            WorGetSingalMotionRecordData(item_t);
            break;

        case WOR_CMD_APPEAR_SINGAL_MOTION_RECORD_DATA:
            //WorAppearSingalMotionData();
            break;

        case WOR_CMD_GET_SINGAL_MOTION_RECORD_MINUTE:
            WorGetSingalmotionDetailedInfo(item_t);
            break;

        default:
            break;
    }
}
/******************************************************************************/
/** 2.10.1 Set Device Open Or Close Realtime Heartrate
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/21
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void HarOpenOrCloseRealHeart(TlvItem_t *item)
{
//    uint32_t temp_length;

//    if(NULL == item)
//    {
//        return;
//    }

//    if(NULL == TlvItem_ItemWithType(item,1))
//    {
//        GeneralErrorACK(GeneralError[6],0x19,0x01);
//        return;
//    }

//    g_reming_switch.hrm_operator_type = *TlvItem_ItemValue(item,&temp_length);

//    /*Start real-time heart*/
//    //if((1 != g_tSingleMotionMgr[E_SINGLE_MOTION_SRC_UI].flag)&&(2 != g_tSingleMotionMgr[E_SINGLE_MOTION_SRC_UI].flag))
//    //{
//    if(1 == g_reming_switch.hrm_operator_type)
//    {
//        OS_TASK_NOTIFY(g_hrsensor_handle,HR_OPEN_MASK, OS_NOTIFY_SET_BITS);
//        gUIContext.single_motion_mask_flag = 1;
//    }

//    /*Stop real-time heart*/
//    else if(2 == g_reming_switch.hrm_operator_type)
//    {
//        /*Close HR when app request and Band HR test quit*/
//        if((0 == g_reming_switch.hrm_measure_flag) && (gUIContext.sport_heart_working_flag == FALSE))
//        {
//            OS_TASK_NOTIFY(g_hrsensor_handle,HR_CLOSE_MASK, OS_NOTIFY_SET_BITS);
//            gUIContext.sport_heart_continue_flag = FALSE; /*HR close, reset sport_heart_continue_flag*/
//        }

//        gUIContext.single_motion_mask_flag = 0;
//    }

//    else
//    {

//    }

//    //}

//    GeneralErrorACK(GeneralError[0],0x19,0x01);
}
/******************************************************************************/
/** 2.10.2 Upload Realtime Heartrate Data
  *
  * @param[in] heartreat_start_time
  * @param[in] heartreat_end_time
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/21
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void UploadRealtimeHeartrateData(uint32_t heartreat_start_time,uint32_t heartreat_end_time)
{
    uint8_t heartrate_frame_num = 0;
    uint8_t heartrate_target_index = 0;
    uint8_t temp_hr_value;
    uint32_t temp_hr_time;
    uint32_t hr_time;

    TlvItem_t item_heartrate_list;
    TlvItem_t item_heartrate_struct;
    TlvItem_t item_heartrate_info;
    TlvItem_t item_time_info;

    //heartrate_frame_num = GetHeartrateFrameNum(heartreat_start_time,heartreat_end_time);
    memset(&item_heartrate_list,0,sizeof(TlvItem_t));
    item_heartrate_list.type = 0x03 | CHILD_MASK;
    item_heartrate_list.length = 0;

    memset(&item_heartrate_struct,0,sizeof(TlvItem_t));
    memset(&item_heartrate_info,0,sizeof(TlvItem_t));
    memset(&item_time_info,0,sizeof(TlvItem_t));

    for(heartrate_target_index = 0; heartrate_target_index < heartrate_frame_num; heartrate_target_index++)
    {
        item_heartrate_struct.type = 0x04 | CHILD_MASK;
        item_heartrate_struct.length = 0;

        item_heartrate_info.type = 0x05;
        item_heartrate_info.length = 0x1;
        temp_hr_value = 83;
        item_heartrate_info.value = &temp_hr_value;
        TlvItem_AddChild(&item_heartrate_struct,&item_heartrate_info);

        item_time_info.type = 0x06;
        item_time_info.length = 0x4;
        temp_hr_time = 100;
        hr_time = Uint32LtoB(temp_hr_time);
        item_time_info.value = (uint8_t*)&hr_time;
        TlvItem_AddChild(&item_heartrate_struct,&item_time_info);

        TlvItem_AddChild(&item_heartrate_list,&item_heartrate_struct);
    }

    SendFramePack(&item_heartrate_list,0x19,0x02);
}
/******************************************************************************/
/** 2.10.2 Get Device Realtime Heartrate Information
  *
  * @param[in] item
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/21
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static void HarQueryRealHeartrate(TlvItem_t *item)
{
    uint32_t temp_value = 0;
    uint32_t heartreat_start_time = 0;
    uint32_t heartreat_end_time = 0;
    TlvItem_t *brotheritem = NULL;

    if(NULL == item)
    {
        GeneralErrorACK(GeneralError[5],0x19,0x02);
        return;
    }

    /**Note:This may be a child node */
    brotheritem = TlvItem_Next(item); //point to start_time

    if(NULL == brotheritem)
    {
        GeneralErrorACK(GeneralError[5],0x19,0x02);
        return;
    }

    else
    {
        TlvItem_ItemInt32Value(brotheritem,&temp_value,1);
        heartreat_start_time = temp_value;
    }

    brotheritem = TlvItem_Next(brotheritem); //point to end_time

    if(NULL == brotheritem)
    {
        GeneralErrorACK(GeneralError[5],0x19,0x02);
        return;
    }

    else
    {
        TlvItem_ItemInt32Value(brotheritem,&temp_value,1);
        heartreat_end_time = temp_value;
    }

    UploadRealtimeHeartrateData(heartreat_start_time,heartreat_end_time);
}
/******************************************************************************/
/** Reporter Real Heart rate 2.10.3
  *
  * @param[in]
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/1/19
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
void ReporterRealHeartrate(void)
{
//    TlvItem_t item_hr_struct;
//    TlvItem_t item_hr_info;
//    TlvItem_t item_time_info;

//    uint8_t  temp_hr_info = 0;
//    uint32_t temp_time_info = 0;

//    memset(&Tlvs_value_resp,0,sizeof(TlvItem_t));
//    Tlvs_value_resp.type = 0x01 | CHILD_MASK;
//    Tlvs_value_resp.length = 0x00;

//    memset(&item_hr_struct,0,sizeof(TlvItem_t));
//    item_hr_struct.type = 0x02 | CHILD_MASK;
//    item_hr_struct.length = 0x00;

//    memset(&item_hr_info,0,sizeof(TlvItem_t));
//    item_hr_info.type = 0x03;
//    item_hr_info.length = 0x01;
//    temp_hr_info = GetHRMValue();
//    item_hr_info.value = &temp_hr_info;
//    TlvItem_AddChild(&item_hr_struct,&item_hr_info);

//    memset(&item_time_info,0,sizeof(TlvItem_t));
//    item_time_info.type = 0x04;
//    item_time_info.length = 0x04;
//    temp_time_info = RTCGetCurrentSeconds();
//    temp_time_info = Uint32LtoB(temp_time_info);
//    item_time_info.value = (uint8_t*)&temp_time_info;
//    TlvItem_AddChild(&item_hr_struct,&item_time_info);

//    TlvItem_AddChild(&Tlvs_value_resp,&item_hr_struct);

//    SendFramePack(&Tlvs_value_resp,0x19,0x03);
}
/******************************************************************************/
/** Heart Rate Service Type 2.10
  *
  * @param[in] item_t
  * @param[in] command_id
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/19
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void HeartRateServiceType(TlvItem_t *item_t,uint8_t command_iden)
{
    HAR_COMMAND_ID_E command_id;
    command_id = (HAR_COMMAND_ID_E)command_iden;

    switch(command_id)
    {
        case HAR_CMD_CLOSE_REAL_HEART:
            HarOpenOrCloseRealHeart(item_t);
            break;

        case HAR_CMD_QUERY_REAL_HEART:
            HarQueryRealHeartrate(item_t);
            break;

        case HAR_CMD_APPEAR_REAL_HEART:
            //ReporterRealHeartrate();
            break;

        default:
            break;
    }
}
/******************************************************************************/
/** ErrorLogExport
  *
  * @param[in] log_buf
  * @param[in] payload
  * @param[in] payload_length
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/3/5
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
uint16_t logcheck(uint8_t *log_buf,uint8_t *payload,uint16_t payload_length,uint8_t service_id,uint8_t command_id)
{
    uint16_t frame_length = 0;
    uint16_t temp_crc16 = 0;
    uint16_t temp_index = 0;

    if((log_buf == NULL)||(payload_length == 0))
    {
        return 0;
    }

    frame_length = payload_length+3;

    log_buf[temp_index++] = LOGIC_LINKER_FRAME_SOF;
    log_buf[temp_index++] = (uint8_t)((frame_length&0xFF00)>>8);
    log_buf[temp_index++] = (uint8_t)(frame_length&0x00FF);
    log_buf[temp_index++] = 0;
    log_buf[temp_index++] = service_id;
    log_buf[temp_index++] = command_id;

    memcpy(&log_buf[temp_index],payload,payload_length);
    temp_index += payload_length;

    temp_crc16 = CheckSumCRC16(&log_buf[0],frame_length+3);

    log_buf[temp_index++] = (uint8_t)((temp_crc16 & 0xff00) >> 8);
    log_buf[temp_index] = (uint8_t)(temp_crc16 & 0x00ff);

    return (frame_length+5);
}
/******************************************************************************/
/** FlashLogServiceType
  *
  * MASTER: 5a 00 13 00 66 01 01 02 00 05 02 04 00 00 00 00 03 04 00 00 01 00 40 86
  *
  * SLAVE:
  *
  *
  *
  *
  *//*
  * History        :
  * 1.Date         : 2017/3/28
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
#define FLASH_LOG_MAX  256

static void UploadFlashLog(uint16_t part,uint32_t offset,uint32_t len)
{
//    uint16_t length = 0;
//    uint16_t index  = 0;
//    uint8_t  flash_log_buf[FLASH_LOG_MAX+1];

//    nvms_t LOG_PART_INFO;
//    index = ((len % FLASH_LOG_MAX) > 0) ? (len / FLASH_LOG_MAX + 1) : (len / FLASH_LOG_MAX);

//    for(uint16_t i = 0; i<index; i++)
//    {
//        LOG_PART_INFO = ad_nvms_open(part);
//        ad_nvms_read(LOG_PART_INFO, offset + i*FLASH_LOG_MAX, &flash_log_buf[1],FLASH_LOG_MAX);
//        flash_log_buf[0] = i;
//        length = logcheck(g_receive_packet_buffer,flash_log_buf, sizeof(flash_log_buf),0x66,0x01);
//        LogicLinkerSendFrame(g_receive_packet_buffer,length,1024);
//    }
}

static void FlashLogExport(TlvItem_t *item)
{
    TlvItem_t *tlv_nvms_log_part = NULL;
    TlvItem_t *tlv_flash_log_offset = NULL;
    TlvItem_t *tlv_flash_log_len = NULL;

    uint16_t nvms_log_part = 0;
    uint32_t flash_log_offset = 0;
    uint32_t flash_log_len = 0;

    if(NULL == item)
    {
        return;
    }

    tlv_nvms_log_part = TlvItem_ItemWithType(item,1);

    if(NULL != tlv_nvms_log_part)
    {
        TlvItem_ItemInt16Value(tlv_nvms_log_part,&nvms_log_part,1);
    }

    tlv_flash_log_offset = TlvItem_ItemWithType(item,2);

    if(NULL != tlv_flash_log_offset)
    {
        TlvItem_ItemInt32Value(tlv_flash_log_offset,&flash_log_offset,1);
    }

    tlv_flash_log_len = TlvItem_ItemWithType(item,3);

    if(NULL != tlv_flash_log_len)
    {
        TlvItem_ItemInt32Value(tlv_flash_log_len,&flash_log_len,1);
    }

    UploadFlashLog(nvms_log_part,flash_log_offset,flash_log_len);
}


static void FlashLogServiceType(TlvItem_t *item_t,uint8_t command_iden)
{
    switch(command_iden)
    {
        case 0x01:

            FlashLogExport(item_t);
            break;
    }
}
/******************************************************************************/
/** ErrorLogServiceType
  *
  * @param[in]
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/3/28
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
//extern ERROR_LOG_T   g_error_log;
static void ErrorLogExport(TlvItem_t *item)
{
//    uint16_t length = 0;

//#if 1
//    nvms_t LOG_PART_INFO;
//    LOG_PART_INFO = ad_nvms_open(NVMS_LOG_PART);

//    ad_nvms_read(LOG_PART_INFO, ERROR_LOG_OFFSET, (uint8_t *)&(g_error_log.dead_time),sizeof(ERROR_LOG_T));
//#endif
//    length = logcheck(g_receive_packet_buffer,(uint8_t *)&(g_error_log.dead_time), sizeof(ERROR_LOG_T),0x77,0x01);

//    LogicLinkerSendFrame(g_receive_packet_buffer,length,1024);
}

static void ErrorLogServiceType(TlvItem_t *item_t,uint8_t command_iden)
{
    switch(command_iden)
    {
        case 0x01:

            ErrorLogExport(item_t);
            break;
    }
}
/******************************************************************************/
/** Handle App Item Parse
  *
  * @param[in] item
  * @param[in] service_id
  * @param[in] command_id
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/19
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
static void HandleAppItem(TlvItem_t *item,uint8_t service_iden,uint8_t command_id)
{
    SERVICE_ID_E service_id;
    service_id = (SERVICE_ID_E)service_iden;

    switch(service_id)
    {
        case DEVICE_MANAGER_SERVICE:
            /**2.1  Device Management Service */
            DeviceManagerServiceType(item,command_id);
            break;

        case NOTIFICATION_SERVICE:
            /**2.2  Notification Service */
            NotificationServiceType(item,command_id);
            break;

        case HANDSFREE_CALL_SERVICE:
            /**2.3  Hands-free Call Service */
            HandsFreeCallServiceType(item,command_id);
            break;

        case FITNESS_SERVICE:
            /**2.4  Fitness Service */
            FitnessServiceType(item,command_id);
            break;

        case ALARM_SERVICE:
            /**2.5  Alarm Service */
            AlarmServiceType(item,command_id);
            break;

        case OTA_SERVICE:
            /**2.6  OTA Service */
            OTAServiceType(item,command_id);
            break;

        case MAINTENANCE_SERVICE:
            /**2.7  Maintenance Service */
            MaintenanceServiceType(item,command_id);
            break;

        case PHONE_LOST_REMIND:

            PhoneLostRemind(item,command_id);
            break;

        case FONT_SERVICE:
            /**2.8  Font Service */
            FontServiceType(item,command_id);
            break;

        case PHONE_BATTERY_SERVICE:
            break;

        case EXERCISE_SERVICE:
            ExerciseServiceType(item,command_id);
            break;

        case WORKOUT_SERVICE:
            /**2.9  Workout Service */
            WorkoutServiceType(item,command_id);
            break;

        case HEART_RATE_SERVICE:
            /**2.10 Heart Rate Service */
            HeartRateServiceType(item,command_id);
            break;

        case READ_FLASH_DATA:

            FlashLogServiceType(item,command_id);

            break;


#if LOG_ERROR_LOG_ENABLE

        case ERROR_LOG_SERVICE:
            ErrorLogServiceType(item,command_id);
            break;
#endif

        default:
            break;
    }
}

//#define  OTA_TRANS_LENGTH  128+1
/**OTA Receive Module Content */

uint8_t Ota_Data_Buffer[128];
uint8_t Ota_Read_Data_Buffer[128];
uint8_t OTAReceiveModuleContent(uint8_t *p_buf,uint16_t length)
{
    uint8_t   psn = 0;
    uint8_t   otabufferindex=0;
    uint8_t   timeout_count=0;
    psn = p_buf[0];


    gOTAData.ota_package_index ++;

    if(gOTAData.ota_package_index % 10 == 0)  //??10???????timer
    {
        gOTAData.timercnt= 0;
    }

    gOTAData.ota_req_timeout = 0;
    //LogOta("ota_package_index=%d\r\n",gOTAData.ota_package_index);

    gOTAData.ota_flash_address = (gOTAData.ota_package_index - 1) * 128;

    //printf("ota_flash_address=0x%.8x\r\n", gOTAData.ota_flash_address);

    gOTAData.ota_file_offset = (gOTAData.ota_package_index) * 128;

    //printf("ota_file_offset=0x%.8x\r\n", gOTAData.ota_file_offset);

    gOTAData.package_received_size= (gOTAData.ota_package_index * 128);

    //OTAProgrammFlash(gOTAData.ota_flash_address,Ota_Data_Buffer,128);

    if((gOTAData.ota_package_index-1) == 0x00) //the first array should send package size to app
    {
        timeout_count = 0;
        memcpy(Ota_Data_Buffer,&p_buf[1],128);
        gOTAData.package_valid_size = ((Ota_Data_Buffer[8]<< 24)|(Ota_Data_Buffer[9]<< 16)|(Ota_Data_Buffer[10]<<8)|(Ota_Data_Buffer[11]));

//        LogOta("buffer[12]=0x%x 0x%x,0x%x,0x%x\r\n",Ota_Data_Buffer[12],Ota_Data_Buffer[13],Ota_Data_Buffer[14],Ota_Data_Buffer[15]);

        if((Ota_Data_Buffer[12] == 0xaa) && (Ota_Data_Buffer[13] == 0xbb) && (Ota_Data_Buffer[14] == 0xcc) && (Ota_Data_Buffer[15] == 0xdd))
        {
            gOTAData.ota_hrm_update_flag = 0x01;
            memcpy(gOTAData.hrmsoftwareversion,&Ota_Data_Buffer[43],5);
        }

        if((gOTAData.package_valid_size % 128) !=0)
        {
            gOTAData.ota_package_total  =(gOTAData.package_valid_size / 128) +1;
        }

        else
        {
            gOTAData.ota_package_total =(gOTAData.package_valid_size /128) ;
        }

//        LogOta("package_totla=%d\r\n",gOTAData.ota_package_total);
        do
        {
//            LogOta("timeout_count=%d\r\n",timeout_count);
            OTAProgrammFlash(gOTAData.ota_flash_address,Ota_Data_Buffer,128);
            OTAReadFlash(gOTAData.ota_flash_address,Ota_Read_Data_Buffer,128);
            if(++timeout_count > 10)
            {
//               LogOta("Write update pack timeout exit\r\n");
               timeout_count = 0;
               break;
            }
        }while(memcmp(Ota_Data_Buffer,Ota_Read_Data_Buffer,128));

        for(otabufferindex = OTA_MCU_START_OFFSET; otabufferindex < OTA_RECEIVE_BUFFER_SIZE; otabufferindex++)
        {
            gOTAData.ota_check_sum = (gOTAData.ota_check_sum<<8) ^ crc16tab[((gOTAData.ota_check_sum>>8) ^ Ota_Read_Data_Buffer[otabufferindex])&0x00FF];
        }

        //printf("package_valid_size=0x%.8x,received_size=0x%.8x\r\n",gOTAData.package_valid_size,gOTAData.package_received_size);
        OTAPackageValidReceivedSize(gOTAData.package_valid_size,gOTAData.package_received_size);
    }

    else if(gOTAData.ota_package_index < gOTAData.ota_package_total)
    {
        timeout_count = 0;
        memcpy(Ota_Data_Buffer,&p_buf[1],OTA_RECEIVE_BUFFER_SIZE);
        do
        {
//            LogOta("timeout_count=%d\r\n",timeout_count);
            OTAProgrammFlash(gOTAData.ota_flash_address,Ota_Data_Buffer,OTA_RECEIVE_BUFFER_SIZE);
            OTAReadFlash(gOTAData.ota_flash_address,Ota_Read_Data_Buffer,OTA_RECEIVE_BUFFER_SIZE);
            if(++timeout_count > 10)
            {
//               LogOta("Write update pack timeout exit\r\n");
               timeout_count = 0;
               break;
            }
        }while(memcmp(Ota_Data_Buffer,Ota_Read_Data_Buffer,128));

        for(otabufferindex = 0; otabufferindex < OTA_RECEIVE_BUFFER_SIZE; otabufferindex++)
        {
            gOTAData.ota_check_sum = (gOTAData.ota_check_sum<<8) ^ crc16tab[((gOTAData.ota_check_sum>>8) ^ Ota_Read_Data_Buffer[otabufferindex])&0x00FF];
        }

        //printf("WriteFlashota_package_index=%d\r\n",gOTAData.ota_package_index);
    }

    else if(gOTAData.ota_package_index == gOTAData.ota_package_total)//1781?????????
    {
//        LogOta("gOTAData.ota_package_index == gOTAData.ota_package_total\r\n");
        timeout_count = 0;
        memcpy(Ota_Data_Buffer,&p_buf[1],gOTAData.ota_last_bytes);
        do
        {
//             LogOta("timeout_count=%d\r\n",timeout_count);
            OTAProgrammFlash(gOTAData.ota_flash_address,Ota_Data_Buffer,gOTAData.ota_last_bytes);
            OTAReadFlash(gOTAData.ota_flash_address,Ota_Read_Data_Buffer,gOTAData.ota_last_bytes);
            if(++timeout_count > 10)
            {
//               LogOta("Write update pack timeout exit\r\n");
               timeout_count = 0;
               break;
            }
        }while(memcmp(Ota_Data_Buffer,Ota_Read_Data_Buffer,gOTAData.ota_last_bytes));
        for(otabufferindex = 0; otabufferindex < gOTAData.ota_last_bytes; otabufferindex++)
        {
            gOTAData.ota_check_sum = (gOTAData.ota_check_sum<<8) ^ crc16tab[((gOTAData.ota_check_sum>>8) ^ Ota_Read_Data_Buffer[otabufferindex])&0x00FF];
        }
        /*should add package verify in the future*/
        //OS_TIMER_STOP(gOTAData.ota_timer,OS_TIMER_FOREVER);
        gOTAData.timercnt = 0;
        gOTAData.ota_transfer_finished = 0x01;
//        LogOta("11gOTAData.timercnt =%d\r\n",gOTAData.timercnt);
        OTAPackageValidity();
    }

    if(gOTAData.ota_package_index < (gOTAData.ota_package_total-1)) //??????,??128,?????????
    {
        OTAApplyUpgradePackageData(gOTAData.ota_file_offset,128,gOTAData.ota_file_bitmap);
    }

    else if(gOTAData.ota_package_index == (gOTAData.ota_package_total-1))
    {
        //printf("RequstLast ota_package_index=%d\r\n",gOTAData.ota_package_index);
        gOTAData.ota_last_bytes = (gOTAData.package_valid_size - gOTAData.ota_package_index * 128) ;
//        LogOta("gOTAData.ota_last_bytes=%d\r\n",gOTAData.ota_last_bytes);
        OTAApplyUpgradePackageData(gOTAData.ota_file_offset,gOTAData.ota_last_bytes,gOTAData.ota_file_bitmap);
    }
}

/******************************************************************************/
/** parse data frame from source app
  *
  * @param[in] payload_total_length
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/18
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
void ParseDataFrameSourceApp(uint16_t payload_total_length)
{
    SERVICE_ID_E service_id;
    uint8_t command_id;

    service_id = (SERVICE_ID_E)g_receive_packet_buffer[0];
    command_id = g_receive_packet_buffer[1];

    if((0x09 == service_id)&&(0x02 == command_id))
    {
        OTAParameterNegotiate();
    }

    else if((0x09 == service_id)&&(0x04 == command_id))
    {
        OTAReceiveModuleContent(&g_receive_packet_buffer[2],payload_total_length-2);
    }

    else if((0x09 == service_id) && (0x06 == command_id))
    {
        OTANotifyDeviceReboot();
    }

    else if((0x0a == service_id) && (0x01 == command_id))
    {

        GeneralErrorACK(GeneralError[7],0x0a,0x01);
    }

    else
    {

        ReceiveFramePackEncrypt(payload_total_length,service_id,command_id);
#if 0
        Tlvs_value = TlvItem_MallocTree(payload_total_length-2,&g_receive_packet_buffer[2]);

        if(NULL == Tlvs_value)
            return;

        HandleAppItem(Tlvs_value,service_id,command_id);

        if(NULL != Tlvs_value)
        {
            TlvItem_FreeTree(Tlvs_value);
            Tlvs_value = NULL;
        }

#endif
    }
}
void ProtocolVariateInit(void)
{
    BtBindRequest.bt_bind_status = 0x02;
    BtBindRequest.bt_bind_status_info = 0x04;
//    g_reming_switch.ble_reminder_enable = 1;
//    g_reming_switch.wrist_light_switch = 1;
//    g_reming_switch.bnotification_enable = 0;
//    g_reming_switch.bincoming_call_enable = 1;
//    g_reming_switch.hrm_operator_type = 2;
//    g_reming_switch.hrm_clash_count = 0;
//    g_reming_switch.hrm_clash_enable = 0;
//    g_reming_switch.hrm_measure_flag = 0;
//    g_reming_switch.miss_call_flag = 0;
//     g_reming_switch.ios_call_reject_flag = 0;
    memset(BtBindRequest.bt_bind_id,0,6);
    fifo_init(&g_diaply_msg.message_fifo);
//    g_threshold_step = 0;
}
/******************************************************************************/
/** RealTimeHeartTimerCallback
  *
  * @param[in] timer
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/2/15
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
//static void RealTimeHeartTimerCallback(OS_TIMER timer)
//{
//    if(1 == g_reming_switch.hrm_operator_type)
//    {
//        ReporterRealHeartrate();
//    }

//    if(0x01 == g_reming_switch.hrm_clash_enable)
//    {
//        g_reming_switch.hrm_clash_count++;

//        if(g_reming_switch.hrm_clash_count >= 60)
//        {
//            g_reming_switch.hrm_clash_count = 0;
//            g_reming_switch.hrm_clash_enable = 0;

//            if(1 == g_reming_switch.hrm_operator_type)
//            {
//                if(SINGLE_MOTION_STOP_FLAG == g_tSingleMotionMgr[E_SINGLE_MOTION_SRC_UI].flag)
//                {
//                    gUIContext.single_motion_mask_flag = 0;

//                    if(0x01 == g_reming_switch.hrm_measure_flag)
//                    {

//                    }
//                    else
//                    {
//                        OS_TASK_NOTIFY(g_hrsensor_handle,HR_CLOSE_MASK, OS_NOTIFY_SET_BITS);
//                        gUIContext.sport_heart_continue_flag = FALSE; /*HR close, reset sport_heart_continue_flag*/
//                    }
//                }

//                else
//                {
//                    gUIContext.single_motion_mask_flag = 1;

//                    if(SINGLE_MOTION_START_FLAG == g_tSingleMotionMgr[E_SINGLE_MOTION_SRC_UI].flag)
//                    {
//                        g_reming_switch.hrm_measure_flag = 0x01;
//                    }
//                }

//                g_reming_switch.hrm_operator_type = 2;
//            }
//        }
//    }

//    /**/
//    else
//    {
//        g_reming_switch.hrm_clash_count = 0;
//    }

//    //LogProtocol("hrm_clash_enable : %d\r\n",g_reming_switch.hrm_clash_enable);
//    //LogProtocol("hrm_clash_count : %d\r\n",g_reming_switch.hrm_clash_count);
//    //LogProtocol("hrm_operator_type : %d\r\n",g_reming_switch.hrm_operator_type);
//    //LogProtocol("flag : %d\r\n",g_tSingleMotionMgr[E_SINGLE_MOTION_SRC_UI].flag);
//    //LogProtocol("hrm_measure_flag : %d\r\n",g_reming_switch.hrm_measure_flag);
//    //LogProtocol("heart : %d\r\n",GetHRMValue());
//}
//OS_TIMER real_time_heart_timer;
void RealTimeHeartTimerDelete(void)
{
//    if(real_time_heart_timer != NULL)
//    {
//        OS_TIMER_DELETE(real_time_heart_timer, OS_TIMER_FOREVER);
//        real_time_heart_timer = NULL;
//    }
}
void RealTimeHeartTimerStop(void)
{
//    OS_TIMER_STOP(real_time_heart_timer, OS_TIMER_FOREVER);
}
void RealTimeHeartTimerStart(void)
{
//    OS_TIMER_START(real_time_heart_timer,OS_TIMER_FOREVER);
}
void RealTimeHeartTimerInit(void)
{
//    real_time_heart_timer = OS_TIMER_CREATE("real time heart",
//                                            OS_MS_2_TICKS(REAL_TIME_HEART_SENCONDS),
//                                            OS_TIMER_SUCCESS, /*repeated*/
//                                            (void *) OS_GET_CURRENT_TASK(),
//                                            RealTimeHeartTimerCallback);
//    OS_TIMER_START(real_time_heart_timer,OS_TIMER_FOREVER);
}
/******************************************************************************/
/** Protocol Task
  *
  * @param[in]
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/22
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
//OS_TASK g_protocol_handle = NULL;
//PRIVILEGED_DATA TaskHandle_t g_protocol_task;

//TaskHandle_t GetProtocolTask(void)
//{
//    return g_protocol_task;
//}

//static void ProtocolTask(void *params)
//{
//    BaseType_t ret;
//    uint32_t notif;
//    msg m;

//    GetFirmwareVersion();
//    ProtocolVariateInit();
//    //vSemaphoreCreateBinary(semaphoreBleTx);
//    msg_queue_create(&ble_normal_datapath_trans_queue, BLE_PATH_TRANS_QUEUE_SIZE, DEFAULT_OS_ALLOCATOR);
//    //OS_ASSERT(NULL != ble_normal_datapath_trans_queue.queue);

//    g_protocol_task = OS_GET_CURRENT_TASK();
//    RealTimeHeartTimerInit();

//    for(;;)
//    {
//        ret = OS_TASK_NOTIFY_WAIT(0, (uint32_t)-1, &notif, portMAX_DELAY);
//        configASSERT(ret == pdTRUE);

//        if(notif & BLE_REC_DATA_MASK)
//        {
//            if(isBleReceivingPending())
//            {
//#if 0
//                bool ret = ProtocolPopFromBleQueue(&m);
//                OS_ASSERT(true == ret);
//                LogicLinkerReceiveFrame(m.data,m.size);
//                msg_release(&m);
//#else
//                bool ret = ProtocolPopFromBleQueue(&m);
//                OS_ASSERT(true == ret);
//                LogicLinkerReceiveFrame(m.data,m.size);
//                msg_release(&m);

//                if(isBleReceivingPending())
//                {
//                    OS_TASK_NOTIFY(GetProtocolTask(),BLE_REC_DATA_MASK, OS_NOTIFY_SET_BITS);
//                }

//#endif
//            }
//        }

//        /*2.4.3: Get motion sleep statistic information*/
//        if(notif & PRO_GET_MOTION_SLEEP_STATISTIC_INFO)
//        {
//            FitGetTotalMotionData();
//        }

//        /*2.4.4: Get Motion data page numbers*/
//        if(notif & PRO_GET_MOTION_DATA_FRAME)
//        {
//            FitUploadMotionSampleFrameNum();
//        }

//        /*2.4.5: Get Motion data of a page*/
//        if(notif & PRO_GET_MOTION_DATA)
//        {
//            FitUploadSampleData(g_tMotionSleepDb.page_index_request);
//        }

//        /*2.4.6: Get Motion status page nums*/
//        if(notif & PRO_GET_MOTION_STATE_FRAME)
//        {
//            FitUploadStateDataFrame();
//        }

//        /*2.4.7: Get Motion status of a page*/
//        if(notif & PRO_GET_MOTION_STATE)
//        {
//            FitUploadStateDataPort(g_tMotionSleepDb.page_index_request);
//        }

//        if(notif & PRO_GET_SINGLE_MOTION_INFO_OK_NOTIFY)
//        {
//            UploadSingalMotionRecord(g_tMotionSleepDb.start_timestamp_request,g_tMotionSleepDb.end_timestamp_request);
//        }

//        if(notif & PRO_CLOSE_SINGLE_MOTION)
//        {
//            //WorAppearSingalMotionData(0);
//            //UploadSingalmotionDetailedInfo(0,0);
//        }

//        if(notif & PRO_GET_SINGLE_MOTION_DETAIL_INFO_OK_NOTIFY)
//        {
//            UploadSingalmotionDetailedInfo(g_tMotionSleepDb.page_timestamp,g_tMotionSleepDb.page_index_request);
//        }

//        if(notif & DELETE_PROTOCOL_TASK_SELF)
//        {
//            RealTimeHeartTimerDelete();
//            OS_TASK_DELETE(OS_GET_CURRENT_TASK());
//        }
//    }
//}
/******************************************************************************/
/** Protocol Task Init
  *
  * @return
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/22
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
void ProtocolTaskInit(void)
{
//#if CFG_APP_PROTOCOL
//    /* Start the protocol application task. */
//    OS_TASK_CREATE("protocol_task",                  /* The text name assigned to the task, for
//                                                           debug only; not used by the kernel. */
//                   ProtocolTask,               /* The function that implements the task. */
//                   NULL,                            /* The parameter passed to the task. */
//                   1024*5,                             /* The number of bytes to allocate to the
//                                                           stack of the task. */
//                   PROTOCOL_TASK_PRIORITY,  /* 1  The priority assigned to the task. */
//                   g_protocol_handle);                         /* The task handle. */

//    OS_ASSERT(g_protocol_handle);
//#endif
}


/** @}*/
