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
#include <string.h>
#include <stdlib.h>
//#include "FreeRTOS.h"
//#include "task.h"
//#include "queue.h"
//#include "semphr.h"

#include "ble_tlv.h"
//#include "logging.h"
//#include "app_factorytest.h"
#include "am_util_delay.h"

/*----------------------------------------------*
 * Include File                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * External Declaration                         *
 *----------------------------------------------*/
uint8_t g_receive_frame_buffer[FRAME_BUFFER_SIZE];
uint8_t g_send_frame_buffer[FRAME_BUFFER_SIZE];

uint8_t g_receive_packet_buffer[RECEIVE_BUFFER_SIZE];

void ParseDataFrameSourceApp(uint16_t payload_total_length);
extern void amdtpsSendData(uint8_t *buf, uint16_t len);
/*----------------------------------------------*
 * Extern Prototype                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * Internal Declaration                         *
 *----------------------------------------------*/

/*----------------------------------------------*
 * Globle Variables                             *
 *----------------------------------------------*/

/*The max logic linker packet size*/
static uint16_t g_logic_linker_frame_max_length = LINKER_FRAME_MAX_LENGTH;

/*mtu max length*/
static uint16_t g_mtu_max_length = MTU_MAX_LENGTH;

/**receive index */
static uint16_t g_rx_in_index;
/**fsn index count */
static uint8_t g_fsn_index;

static uint8_t g_header_found = 0;
static uint16_t g_frame_length = 0;
static uint16_t g_received_length = 0;

/*----------------------------------------------*
 * Module Variables                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * Const Defination                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * Micro Defination                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * Function Declaration                         *
 *----------------------------------------------*/

/******************************************************************************/
/** CRC16
  *
  * @param[in] ptr
  * @param[in] len
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/17
  *   Author       : vigoss.wang
  *   Modification : Created function

*******************************************************************************/
uint16_t CheckSumCRC16(uint8_t * ptr, uint16_t len)
{
    uint16_t counter;
    uint16_t crc = 0;

    for(counter = 0; counter < len; counter++)
        crc = (crc<<8) ^ crc16tab[((crc>>8) ^ (*(char *)ptr++))&0x00FF];

    return crc;
}
/******************************************************************************/
/** Set logic linker layer max packet length and mtu max lenght
  *
  * @param[in]
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/17
  *   Author       : vigoss.wang
  *   Modification : Created function

*******************************************************************************/
void LogicLinkerConnectParameterSet(uint16_t max_logic_linker_frame, uint16_t max_mtu_length)
{
    g_logic_linker_frame_max_length = max_logic_linker_frame;
    g_mtu_max_length                 = max_mtu_length;
}
/******************************************************************************/
/** Get Frame Flag Bit via Byte control
  *
  * @param[in] control
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/17
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static uint8_t FraFromControl(uint8_t control)
{
    uint8_t fra;
    fra = control & 0x03;
    if(0 == fra)
        return 0;
    else if(1 == fra)
        return 1;
    else if(2 == fra)
        return 2;
    else
        return 3;
}

/******************************************************************************/
/** Reset Single Frame Sign
  *
  * @return
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/18
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
void ResetSingleFrameSign(void)
{
    g_header_found = 0;
    g_frame_length = 0;
    g_received_length = 0;
}
/** Phrse Frist Datalink Layer Frame
  *
  * @param[in] p_buf
  * @param[in] offset
  * @param[in] length
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/17
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static uint8_t LogicAPPReceivePacketFrist(uint8_t *p_buf,uint8_t offset,uint16_t length)
{
    uint8_t  fsn;
    uint16_t payload_length;

    if((p_buf == NULL)||(length == 0)||(length > LINKER_FRAME_MAX_LENGTH))
    {
        return LOGIC_LINKER_PAR_ERROR;
    }

    fsn = *(p_buf + offset);
    offset++;

    if (0 == fsn)
    {
        payload_length = length - LOGIC_LINKER_CONTROL_LENGTH - LOGIC_LINKER_FSN_LENGTH;
        if(payload_length > RECEIVE_BUFFER_SIZE - g_rx_in_index)
        {
            payload_length = RECEIVE_BUFFER_SIZE - g_rx_in_index;
        }
        memcpy(&g_receive_packet_buffer[0],(p_buf + offset),payload_length);
        g_rx_in_index = payload_length;
        g_fsn_index++;
        return 0;
    }
    else
    {
        g_rx_in_index = 0;
        g_fsn_index   = 0;
        return 1;
    }
}
/******************************************************************************/
/** Phrase Middle Datalink Layer Frame
  *
  * @param[in] p_buf
  * @param[in] offset
  * @param[in] length
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/17
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static uint8_t LogicAPPReceivePacketMiddle(uint8_t *p_buf,uint8_t offset,uint16_t length)
{
    uint8_t  fsn;
    uint16_t payload_length;

    if((p_buf == NULL)||(length == 0)||(length > LINKER_FRAME_MAX_LENGTH))
    {
        return LOGIC_LINKER_PAR_ERROR;
    }

    fsn = *(p_buf + offset);
    offset++;

    if (fsn == g_fsn_index)
    {
        payload_length = length - LOGIC_LINKER_CONTROL_LENGTH - LOGIC_LINKER_FSN_LENGTH;
        if(payload_length > RECEIVE_BUFFER_SIZE - g_rx_in_index)
        {
            payload_length = RECEIVE_BUFFER_SIZE - g_rx_in_index;
        }
        memcpy(&g_receive_packet_buffer[g_rx_in_index],(p_buf + offset),payload_length);
        g_rx_in_index += payload_length;
        g_fsn_index++;
        return 0;
    }
    else
    {
        g_rx_in_index = 0;
        g_fsn_index   = 0;
        return 1;
    }
}
/******************************************************************************/
/** Phrase Last Datalink Layer Frame
  *
  * @param[in] p_buf
  * @param[in] offset
  * @param[in] length
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/17
  *   Author       : Alan.Wang
  *   Modification : Created function

*******************************************************************************/
static uint8_t LogicAPPReceivePacketLast(uint8_t *p_buf,uint8_t offset,uint16_t length)
{
    uint8_t  fsn;
    uint16_t payload_length;
    uint16_t payload_total_length;

    if((p_buf == NULL)||(length == 0)||(length > LINKER_FRAME_MAX_LENGTH))
    {
        return LOGIC_LINKER_PAR_ERROR;
    }

    fsn = *(p_buf + offset);
    offset++;

    if (fsn == g_fsn_index)
    {
        payload_length = length - LOGIC_LINKER_CONTROL_LENGTH - LOGIC_LINKER_FSN_LENGTH;
        if(payload_length > RECEIVE_BUFFER_SIZE - g_rx_in_index)
        {
            payload_length = RECEIVE_BUFFER_SIZE - g_rx_in_index;
        }
        memcpy(&g_receive_packet_buffer[g_rx_in_index],(p_buf + offset),payload_length);
        payload_total_length = g_rx_in_index + payload_length;
        ParseDataFrameSourceApp(payload_total_length);
        g_rx_in_index = 0;
        g_fsn_index   = 0;
        return 0;
    }
    else
    {
        g_rx_in_index = 0;
        g_fsn_index   = 0;
        return 1;
    }
}

/******************************************************************************/
/** Assemble logic application layer packet
  *
  * @param[in]
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/17
  *   Author       : alan.wang
  *   Modification : Created function

*******************************************************************************/
#if 1
uint8_t LogicAPPReceivePacket(uint8_t *buf)
{
    //uint8_t  sof;
    uint16_t length;
    uint8_t  control;
    uint8_t  fra;
    uint8_t  ret_val;
    uint16_t payload_length;
    uint8_t  offset = 0;
    uint8_t  *p_buf = buf;

    if(buf == NULL)
    {
        return LOGIC_LINKER_PAR_ERROR;
    }

    offset++;
    length = p_buf[offset];
    offset++;
    length = (length << 8) + p_buf[offset];
    offset++;
    control = p_buf[offset];
    offset++;

    fra = FraFromControl(control);
    switch(fra)
    {
        case 0:
            payload_length = length - LOGIC_LINKER_CONTROL_LENGTH;
            if(payload_length > RECEIVE_BUFFER_SIZE)
            {
                payload_length = RECEIVE_BUFFER_SIZE;
            }
            memcpy(&g_receive_packet_buffer[0],&p_buf[offset],payload_length);
            ParseDataFrameSourceApp(payload_length);
            ret_val = 0;
            break;
        case 1:
            ret_val = LogicAPPReceivePacketFrist(p_buf,offset,length);
            break;
        case 2:
            ret_val = LogicAPPReceivePacketMiddle(p_buf,offset,length);
            break;
        case 3:
            ret_val = LogicAPPReceivePacketLast(p_buf,offset,length);
            break;
        default: break;
    }
    return ret_val;
}
#else
uint8_t LogicAPPReceivePacket(uint8_t *buf)
{
    uint16_t length;
    uint8_t  control;
    uint8_t  fra;
    uint8_t  ret_val;
    uint16_t payload_length;
    uint8_t  offset = 0;
    uint8_t  *p_buf = buf;

    if(buf == NULL)
    {
        return LOGIC_LINKER_PAR_ERROR;
    }

    offset++;
    length = p_buf[offset];
    offset++;
    length = (length << 8) + p_buf[offset];
    offset++;
    control = p_buf[offset];
    offset++;
    fra = FraFromControl(control);
    if(0x00 == fra)
    {
        payload_length = length - LOGIC_LINKER_CONTROL_LENGTH;
        if(payload_length > RECEIVE_BUFFER_SIZE)
        {
            payload_length = RECEIVE_BUFFER_SIZE;
        }
        memcpy(&g_receive_packet_buffer[0],&p_buf[offset],payload_length);
        ParseDataFrameSourceApp(payload_length);
        ret_val = 0;
    }
    else
    {
        if((0x09 == p_buf[offset+1])&&(0x04 == p_buf[offset+2]))
        {
            offset++;
            payload_length = length - 2;//control
            memcpy(&g_receive_packet_buffer[0],&p_buf[offset],payload_length);
            ParseDataFrameSourceApp(payload_length);
            ret_val = 0;
        }
        else
        {
            switch(fra)
            {
                case 1:
                    ret_val = LogicAPPReceivePacketFrist(p_buf,offset,length);
                    break;
                case 2:
                    ret_val = LogicAPPReceivePacketMiddle(p_buf,offset,length);
                    break;
                case 3:
                    ret_val = LogicAPPReceivePacketLast(p_buf,offset,length);
                    break;
                default: break;
            }
        }
    }
    return ret_val;
}
#endif
/******************************************************************************/
/** assemble one logic linker frame
  *
  * @param[in] dest_pointer: dest buffer pointer
  * @param[in] payload_pointer: payload buffer pointer
  * @param[in] payload_length: payload length
  * @param[in] control_byte: control byte
  * @param[in] fsn_num: FSN byte
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/18
  *   Author       : vigoss.wang
  *   Modification : Created function

*******************************************************************************/
#if 0
uint16_t AssembleOneLogickLinkerFrame(uint8_t *dest_pointer,
                                        uint8_t *payload_pointer,
                                        uint16_t payload_length,
                                        uint8_t control_byte,
                                        uint8_t fsn_num,
                                        uint8_t  service_id,
                                        uint8_t  command_id)
{
    /*length of payload + FSN + control*/
    uint16_t temp_length = 0;
    uint16_t temp_crc16 = 0;
    /*Next buffer index*/
    uint8_t temp_index = 0;

    if((payload_length > LINKER_FRAME_MAX_LENGTH)
            || (dest_pointer == NULL)
            || (payload_pointer == NULL))
    {
        return LOGIC_LINKER_PAR_ERROR;
    }
    /*include FSN*/
    if(control_byte & 0x03)
    {
        temp_length = payload_length + 2;
    }
    /*No FSN*/
    else
    {
        temp_length = payload_length + 1;
    }

    /*Add SOF*/
    dest_pointer[0] = LOGIC_LINKER_FRAME_SOF;
    /*Add Length: control + fsn + payload*/
    dest_pointer[1] = (uint8_t)((temp_length&0xFF00)>>8);
    dest_pointer[2] = (uint8_t)(temp_length&0x00FF);
    /*Add Control byte*/
    dest_pointer[3] = control_byte;
    temp_index += 4;

    /*Add FSN*/
    if(control_byte & 0x03)
    {
        dest_pointer[temp_index++] = fsn_num;
    }

    /*Add payload*/
    if(((control_byte & 0x03) == 0x00)||((control_byte & 0x03) == 0x01))
    {
        dest_pointer[temp_index++] = service_id;
        dest_pointer[temp_index++] = command_id;

        memcpy(&dest_pointer[temp_index],payload_pointer,payload_length-2);
        temp_index += (payload_length-2);
    }
    else
    {
        memcpy(&dest_pointer[temp_index],payload_pointer,payload_length);
        temp_index += payload_length;
    }

    /*Calculate checksum*/
    temp_crc16 = CheckSumCRC16(&dest_pointer[0],temp_length+3);

    /*Add checksum*/
    dest_pointer[temp_index++] = (uint8_t)((temp_crc16 & 0xff00) >> 8);
    dest_pointer[temp_index] = (uint8_t)(temp_crc16 & 0x00ff);

    return 0;
}
#endif
/******************************************************************************/
/** Mcu response APP CRC result
  *
  * @param[in] errno
  *
  * @return
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/18
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
void McuResponseCrcResult(CRC_CHECK_E errno)
{
    uint16_t response_crc;
    uint16_t respnse_crc_temp;
    uint8_t  linker_crc_ack_buf[7] = {0};

    linker_crc_ack_buf[0] = 0x5A;
    linker_crc_ack_buf[1] = 0x00;
    linker_crc_ack_buf[2] = 0x02;
    linker_crc_ack_buf[3] = 0x08;
    linker_crc_ack_buf[4] = (uint8_t)errno;

    respnse_crc_temp = CheckSumCRC16(linker_crc_ack_buf,5);
    response_crc = ((respnse_crc_temp & 0xff)<<8) | ((respnse_crc_temp & 0xff00)>>8);

    memcpy(&linker_crc_ack_buf[5],(uint8_t*)(&response_crc),2);

    BlePvsSendNotify(linker_crc_ack_buf,7);
}

/******************************************************************************/
/** s
  *
  * @return
  *
  *//*
  * History        :
  * 1.Date         : 2017/1/25
  *   Author       : vigoss.wang
  *   Modification : Created function

*******************************************************************************/
void CheckBleCommandCRC(uint8_t *value, uint16_t length)
{
    uint8_t ret = 0;
    uint16_t crc_check = 0;
    uint8_t err_code[4] = {0xF5, 0x02, 0x00, 0xD5};

    if((length > 5)&&(value[length-1] == 0xD5))
    {
        crc_check = CheckSumCRC16(value,length-3);
        if((value[length-3]==(uint8_t)((crc_check & 0xff00)>>8))&&(value[length-2]==(uint8_t)(crc_check & 0x00ff)))
        {
            /* ble cmd format ok */
//            ret = FactoryTestProcess(value, length);
        }
        else
        {
            /* CRC checking Error */
            err_code[2] = 0xFD;
            BlePvsSendNotify(err_code,4);
        }
    }
    else
    {
        /* BLE command format error */
        err_code[2] = 0xFE;
        BlePvsSendNotify(err_code,4);
    }
}
/******************************************************************************/
/** split logic linker frame and send to ble driver
  *
  * @param[in]
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/17
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
uint8_t LogicLinkerSendFrame(uint8_t *data, uint16_t length, uint16_t max_length)
{
    uint16_t  send_num = 0;
    uint16_t  send_totle = 0;
    uint16_t send_length = 0;
    uint8_t  *send_mtu_buffer = data;

    if((data == NULL)||(length == 0)||(length > max_length))
    {
        return LOGIC_LINKER_PAR_ERROR;
    }

    if(length > MTU_MAX_LENGTH)
    {
        send_num = ((length % MTU_MAX_LENGTH) > 0) ? (length / MTU_MAX_LENGTH + 1) : (length / MTU_MAX_LENGTH);

    	for (uint8_t i = 0; i < send_num; i++)
    	{
            send_length = (i < send_num-1) ? MTU_MAX_LENGTH : (length - send_totle);
			amdtpsSendData(&send_mtu_buffer[send_totle], send_length);
            send_totle += send_length;
    	}
    }
    else
    {
		  amdtpsSendData(&send_mtu_buffer[send_totle], length);
    }

    return 0;
}
/******************************************************************************/
/** Assemble logick linker frame and send to logic linker layer
  *
  * @param[in] value
  * @param[in] writeamount
  *
  * @return
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/17
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
uint8_t LogicLinkerReceiveFrame(uint8_t *value, uint16_t length)
{
    uint16_t crc_calculated = 0;
    uint16_t crc_check = 0;
    uint16_t crc_value = 0;
    am_util_stdio_printf("LogicLinkerReceiveFrame...\n");
	
    if((value == NULL)||(length == 0)||(length > LINKER_FRAME_MAX_LENGTH))
    {
        return LOGIC_LINKER_PAR_ERROR;
    }

    if(g_header_found == 0)
    {
        if(0x5A == value[0])
        {
            g_header_found = 1;
        }
        else if(0xB2 == value[0])
        {
            CheckBleCommandCRC(value,length);
        }
        else
        {
            return LOGIC_LINKER_PAR_ERROR;
        }

        g_frame_length = ((uint16_t)value[1] << 8)+ value[2] + 5;

        if(g_frame_length > LINKER_FRAME_MAX_LENGTH)
        {
            ResetSingleFrameSign();
            return LOGIC_LINKER_PAR_ERROR;
        }
        else
        {
            memcpy(&g_receive_frame_buffer[0], value, length);
            g_received_length = length;
        }
    }
    else
    {
        if(length + g_received_length > g_frame_length)
        {
            ResetSingleFrameSign();
            return LOGIC_LINKER_PAR_ERROR;
        }
        else
        {
            memcpy(&g_receive_frame_buffer[g_received_length], value, length);
            g_received_length = g_received_length + length;
        }
    }

    if(g_received_length == g_frame_length)
    {
        memcpy(&crc_value,&g_receive_frame_buffer[g_frame_length - 2],2);
        crc_check = ((crc_value & 0xff00)>>8)|((crc_value & 0xff)<<8);
        crc_calculated = CheckSumCRC16(g_receive_frame_buffer, g_frame_length - 2);
        if(crc_calculated == crc_check)
        {
            if(g_receive_frame_buffer[3] & 0x04)/**<need ack */
            {
                McuResponseCrcResult(CRC_CHECK_OK);
            }
            am_util_stdio_printf("crc_calculation\n");
            LogicAPPReceivePacket(g_receive_frame_buffer);
        }
        else
        {
            memset(g_receive_frame_buffer,0,g_frame_length);
            McuResponseCrcResult(CRC_CHECK_ERROR);
        }
        ResetSingleFrameSign();
    }

    return 0;
}
/******************************************************************************/
/** Logic application layer packet split and send data to logic linker layer
  *
  * @param[in]
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2016/12/17
  *   Author       : vigoss.wang
  *   Modification : Created function

*******************************************************************************/
#if 0
uint8_t LogicAppSendPacket(uint8_t *send_data_buffer,uint8_t service_id,uint8_t command_id,uint16 send_app_data_length)
{
    uint16_t sended_total_length = 0;
    uint16_t sended_frame_length = 0;
    uint16_t sended_packet_length = 0;
    uint8_t  frame_num = 0;
    uint8_t  total_frame = 0;
    uint8_t  temp_num = 0;
    uint8_t  control_byte = 0;

    if((send_data_buffer == NULL)||(send_app_data_length == 0))
    {
        return LOGIC_LINKER_PAR_ERROR;
    }
    memset(g_send_frame_buffer,0,sizeof(g_send_frame_buffer));

    /*Logic application packet need to split into several logic linker packets*/
    if(send_app_data_length > (LINKER_FRAME_MAX_LENGTH - LOGIC_LINKER_FRAME_MIN_HEADER))
    {
        temp_num = (send_app_data_length%(LINKER_FRAME_MAX_LENGTH - LOGIC_LINKER_FRAME_MAX_HEADER));
        /*several full frame*/
        if(temp_num == 0)
        {
            total_frame = (send_app_data_length/(LINKER_FRAME_MAX_LENGTH - LOGIC_LINKER_FRAME_MAX_HEADER));
        }
        /*Have smaller frame (The last frame)*/
        else
        {
            total_frame = (send_app_data_length/(LINKER_FRAME_MAX_LENGTH - LOGIC_LINKER_FRAME_MAX_HEADER)) + 1;
        }

        /*Send frame loop*/
        for(frame_num = 0;frame_num < total_frame;frame_num++)
        {
            /*Clear fsn bits*/
            control_byte &= 0xFC;

            /*Start frame*/
            if(frame_num == 0)
            {
                control_byte |= 0x01;
                sended_frame_length = LINKER_FRAME_MAX_LENGTH - LOGIC_LINKER_FRAME_MAX_HEADER;
            }
            /*end frame*/
            else if(frame_num == total_frame -1)
            {
                control_byte |= 0x03;
                sended_frame_length = send_app_data_length - sended_total_length;
            }
            /*middle frame*/
            else
            {
                control_byte |= 0x02;
                sended_frame_length = LINKER_FRAME_MAX_LENGTH - LOGIC_LINKER_FRAME_MAX_HEADER;
            }
            if(sended_total_length > 2)
            {
                sended_packet_length = sended_total_length - 2;
            }
            /*Assemble a logic linker layer frame*/
            AssembleOneLogickLinkerFrame(&g_send_frame_buffer[0],
                                         &send_data_buffer[sended_packet_length],
                                         sended_frame_length,
                                         control_byte,
                                         frame_num,
                                         service_id,
                                         command_id);
            /*Send one logic linker pakcet to BLE driver*/
            LogicLinkerSendFrame(&g_send_frame_buffer[0],sended_frame_length + LOGIC_LINKER_FRAME_MAX_HEADER);

            sended_total_length += sended_frame_length;

            OS_DELAY_MS(1);
            /*TO DO..... need to delay!!!! vigoss 20161219*/

        }
    }

    /*No need to split, one logic linker packet is enough*/
    else
    {
        /*add SOF, length, control,checksum*/
        /*Assemble a logic linker layer frame*/
        AssembleOneLogickLinkerFrame(&g_send_frame_buffer[0],
                                     send_data_buffer,
                                     send_app_data_length,
                                     control_byte,
                                     frame_num,
                                     service_id,
                                     command_id);
        /*Send one logic linker pakcet to BLE driver*/
        LogicLinkerSendFrame(&g_send_frame_buffer[0],send_app_data_length + LOGIC_LINKER_FRAME_MIN_HEADER);
    }

    return 0;
}
#endif
/******************************************************************************/
/** LogickLinkerSingleFrame
  *
  * @param[in] frame
  * @param[in] structure
  * @param[in] structure_length
  * @param[in] payload
  * @param[in] payload_length
  * @param[in] control_byte
  * @param[in] fsn_num
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/2/23
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
uint16_t LogickLinkerSingleFrame(uint8_t *frame,uint8_t *structure,uint16_t structure_length,
                                 uint8_t *payload,uint16_t payload_length, uint8_t control_byte,
                                 uint8_t fsn_num)
{
    uint16_t frame_length = 0;
    uint16_t temp_crc16 = 0;
    uint16_t temp_index = 0;

    if((frame == NULL)||(structure == NULL)||(payload == NULL)||(payload_length == 0))
    {
        return LOGIC_LINKER_PAR_ERROR;
    }

    if(control_byte == 0x01)
    {
        frame_length = structure_length + payload_length + 2;
    }
    else if( (control_byte == 0x02) || (control_byte == 0x03) )
    {
        frame_length = payload_length + 2;
    }
    else if(control_byte == 0)
    {
        frame_length = structure_length + payload_length + 1;
    }
    else
    {
        return 0;
    }

    frame[0] = LOGIC_LINKER_FRAME_SOF;
    frame[1] = (uint8_t)((frame_length&0xFF00)>>8);
    frame[2] = (uint8_t)(frame_length&0x00FF);
    frame[3] = control_byte;
    temp_index = 4;
    if(control_byte & 0x03)
    {
        frame[temp_index++] = fsn_num;
    }
	if(((control_byte & 0x03) == 0x00)||((control_byte & 0x03) == 0x01))
	{
		memcpy(&frame[temp_index],structure,structure_length);
		temp_index += structure_length;
	}
    memcpy(&frame[temp_index],payload,payload_length);
    temp_index += payload_length;

    temp_crc16 = CheckSumCRC16(&frame[0],frame_length+3);

    frame[temp_index++] = (uint8_t)((temp_crc16 & 0xff00) >> 8);
    frame[temp_index] = (uint8_t)(temp_crc16 & 0x00ff);

    return 0;
}
/******************************************************************************/
/** LogicAppSendPacket
  *
  * @param[in] structure
  * @param[in] structure_length
  * @param[in] payload
  * @param[in] payload_length
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/2/23
  *   Author       : auron.yang
  *   Modification : Created function

*******************************************************************************/
uint8_t LogicAppSendPacket(uint8_t *structure,uint16_t structure_length,
                              uint8_t *payload,uint16_t payload_length)
{
    uint16_t packet_length = 0;
    uint16_t send_length = 0;
    uint16_t send_totle = 0;
    uint16_t temp_crc16 = 0;
    uint8_t  total_frame = 0;
    uint8_t  control_byte = 0;

    if((structure == NULL)||(payload == NULL)||(payload_length == 0))
    {
        return LOGIC_LINKER_PAR_ERROR;
    }

    packet_length = structure_length + payload_length;
    if(packet_length > FRAME_PAYLOAD_MIN)
    {
        total_frame = ((packet_length % FRAME_PAYLOAD_MAX) > 0) ? (packet_length / FRAME_PAYLOAD_MAX + 1) : (packet_length / FRAME_PAYLOAD_MAX);
        for(uint8_t frame_num = 0;frame_num < total_frame;frame_num++)
        {
            //
            control_byte = 0;
            send_length = (frame_num < total_frame -1) ? FRAME_PAYLOAD_MAX : (packet_length - send_totle);
            if(frame_num == 0)
            {
                control_byte |= 0x01;
                LogickLinkerSingleFrame(g_send_frame_buffer,structure,structure_length,&payload[send_totle],
                                          send_length-structure_length,control_byte,frame_num);
            }
            /*end frame*/
            else if(frame_num == total_frame -1)
            {
                control_byte |= 0x03;
                LogickLinkerSingleFrame(g_send_frame_buffer,structure,0,&payload[send_totle-structure_length],
                                          send_length,control_byte,frame_num);
            }
            /*middle frame*/
            else
            {
                control_byte |= 0x02;
                LogickLinkerSingleFrame(g_send_frame_buffer,structure,0,&payload[send_totle-structure_length],
                                          send_length,control_byte,frame_num);
            }
            LogicLinkerSendFrame(&g_send_frame_buffer[0],send_length + LOGIC_LINKER_FRAME_MAX_HEADER,LINKER_FRAME_MAX_LENGTH);
            send_totle += send_length;

            //
            if(send_length >= FRAME_PAYLOAD_MAX)
            {
//                OS_DELAY_MS(100);
            }
        }
    }
    else
    {
        LogickLinkerSingleFrame(g_send_frame_buffer,structure,structure_length,payload,
                                  payload_length,control_byte,0);
        LogicLinkerSendFrame(&g_send_frame_buffer[0],packet_length + LOGIC_LINKER_FRAME_MIN_HEADER,LINKER_FRAME_MAX_LENGTH);
    }

    return 0;
}




/** @}*/
