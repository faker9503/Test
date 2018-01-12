#include "fifo.h"
#include "FreeRTOS.h"

/*****************************************************************************
@Brief: Init fifo
@Note:
*****************************************************************************/
void fifo_init(FIFO_T *pFifo)
{
    pFifo->fifo_count = 0;
    pFifo->fifo_size = FIFO_MAX_ELEMENT_COUNT;
    pFifo->start_index = 0;
}
/*****************************************************************************
@Brief: Add an element to FIFO
@Return: If old element is deleted, return pointer to the element, else return NULL
@Note:
*****************************************************************************/
void fifo_add(FIFO_T *pFifo,  MSG_NODE_T * pMsg)
{
    /*free old element*/
    if (pFifo->fifo_count == pFifo->fifo_size)
    {
        vPortFree(pFifo->element[pFifo->start_index]);
        pFifo->element[pFifo->start_index] = NULL;
    }

    /*add new element*/
    pFifo->element[(pFifo->start_index+pFifo->fifo_count)%pFifo->fifo_size] = pMsg;


    /*move start_index*/
    if ( pFifo->fifo_count == pFifo->fifo_size)
    {
        pFifo->start_index = (pFifo->start_index+1)%pFifo->fifo_size;
    }
    else
    {
        pFifo->fifo_count ++;
    }

}
/*****************************************************************************
@Brief: Get head element from FIFO
@Note:
*****************************************************************************/
MSG_NODE_T * fifo_get_head(FIFO_T *pFifo)
{

    if (pFifo->fifo_count > 0)
    {
#ifdef FIFO_GET_NEW_FIRST
        return pFifo->element[(pFifo->start_index+pFifo->fifo_count-1)%pFifo->fifo_size];
#else
        return pFifo->element[pFifo->start_index];
#endif
    }
    else
    {
        return NULL;
    }

}
/*****************************************************************************
@Brief: Delete head element from FIFO
@Note:
*****************************************************************************/
uint16_t fifo_delete_head(FIFO_T *pFifo)
{
    uint16_t count = 0;

    if (pFifo->fifo_count > 0)
    {
        pFifo->fifo_count --;
#ifdef FIFO_GET_NEW_FIRST
        vPortFree(pFifo->element[(pFifo->start_index+pFifo->fifo_count)%pFifo->fifo_size]);
        pFifo->element[(pFifo->start_index+pFifo->fifo_count)%pFifo->fifo_size] = NULL;
#else
        vPortFree(pFifo->element[pFifo->start_index]);
        pFifo->element[pFifo->start_index] = NULL;
        pFifo->start_index = (pFifo->start_index+1)%pFifo->fifo_size;
#endif
        count = pFifo->fifo_count;
    }

    return count;
}
/*****************************************************************************
@Brief: Get element count
@Note:
*****************************************************************************/
uint16_t fifo_get_count(FIFO_T *pFifo)
{
    return pFifo->fifo_count;
}

/*****************************************************************************
@Brief: Delete all messages
@Note:
*****************************************************************************/
void fifo_delete_all(FIFO_T *pFifo)
{
    uint16_t count;

    do {
        count = fifo_delete_head(pFifo);
    }while(count != 0);
}

/*****************************************************************************
@Brief: Print FIFO
@Note:
******************************************************************************/
/*
void fifo_dump(fifo_t *pFifo)
{
    uint8_t i;
    uint8_t index;

    for (i=0; i<pFifo->fifo_count; i++)
    {
        index = (pFifo->start_index + i)%pFifo->fifo_size;
    }

}
*/
