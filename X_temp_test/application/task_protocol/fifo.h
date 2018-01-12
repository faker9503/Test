
#ifndef _FIFO_H_
#define _FIFO_H_

#include "stdint.h"
//#include "App_common.h"
#include "protocol_config.h"

#define FIFO_MAX_ELEMENT_COUNT              (5)
#define FIFO_GET_SEQUENCE                   (1)    /*0=get old element first, 1=get new element first*/

#if FIFO_GET_SEQUENCE
#define FIFO_GET_NEW_FIRST
#else
#define FIFO_GET_OLD_FIRST
#endif

typedef struct
{
    uint8_t fifo_count; /*current fifo size*/
    uint8_t fifo_size;  /*max element count*/
    uint8_t start_index;    /*point to first element of FIFO*/
    MSG_NODE_T * element[5];
}FIFO_T;
typedef struct
{
    FIFO_T message_fifo;
}DISPLY_STATE_T;

extern void fifo_init(FIFO_T *pFifo);
extern void fifo_add(FIFO_T *pFifo,  MSG_NODE_T * pMsg);
extern MSG_NODE_T * fifo_get_head(FIFO_T *pFifo);
extern uint16_t fifo_delete_head(FIFO_T *pFifo);
extern uint16_t fifo_get_count(FIFO_T *pFifo);
extern void fifo_delete_all(FIFO_T *pFifo);

#endif


