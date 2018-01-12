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
//#include "AEEStdlib.h"
#include "am_msg_task.h"
#include "am_tasks.h"

#define  TYPESIZE (1)

#define  BUFSIZE (100)


#define  UINT16LEN (2)
#define  UINT32LEN (4)

static void updateParentLen(TlvItem_t *item);
void TlvItem_ItemToBuf(TlvItem_t *item,uint8_t *buf,uint32_t length);
static int needConverse(int dump);



//????????,????????,????.??????????????
static int needConverse(int dump)
{
    static int need = -1;
    if(-1 == need)
    {
        uint32_t test = 1;
        int8_t *p = (int8_t *)&test;
        if (1 == *p)
        {
            //????????,????
            need = 1;
        }
        else
        {
            need = 0;
        }
    }
    else
    {

    }
	return need;
}


/*??lengh?????,????,????????length,
  ??child??parent?length??1???,?parent?parent?length?????
  ??child???,???parent????????
  ????:
  ??length?4???,length ?4??,????????,??length?4-7,????????
  ??item,item t+l+v ??4????
  item parent length ??4,??,??item parent?x?????length,??item?,??x+1???,???
  item parent ? parent length ?? 4??? + 1???
  */
static void updateParentLen(TlvItem_t *item)
{
    TlvItem_t *p = item;
    uint32_t inCreaseLen = 0;
    uint32_t itemLen = 0;
    if(NULL == p)
    {
        return;
    }
    itemLen = item->length + TYPESIZE + TlvItem_GetLenSize(item->length);
    while (NULL != p->parentNode)
    {
        p->parentNode->length += itemLen + inCreaseLen;
        inCreaseLen = TlvItem_GetLenSize(itemLen + inCreaseLen);
        p = p->parentNode;
    }
}

//????
uint32_t TlvItem_GetLenSize(uint32_t len)
{
    uint32_t size = 0;

    do
    {
        size++;
        len = len / BIGLEN_MASK;
    }while(0 != len);

    return size;
}


TlvItem_t *TlvItem_New(uint8_t type,uint32_t len,uint8_t *value)
{
    TlvItem_t *node = (TlvItem_t *)pvPortMalloc(sizeof(TlvItem_t));
    if(NULL == node)
    {
        //DBGPRINTF_HIGH("TlvItem Malloc Fail\n");
        //DBG_APP_MSG(DBG_ZONE_INTERFACE,("TlvItem Malloc Fail\r\n"));
        return NULL;
    }
    memset(node, 0, sizeof(TlvItem_t));
    node->type = type;
    node->length = len;
    if(len > 0 && NULL != value)
    {
        node->value = (uint8_t *)pvPortMalloc(len);
        if(NULL == node->value)
        {
            //DBGPRINTF_HIGH("TlvItem value Malloc Fail\n");
            //DBG_APP_MSG(DBG_ZONE_INTERFACE,("TlvItem value Malloc Fail\r\n"));
            vPortFree(node);
            return NULL;
        }
        memcpy(node->value, value, len);
        node->length = len;
    }
    return node;
}


TlvItem_t *TlvItem_Int32ValueNew(uint8_t type,uint32_t value)
{
    uint8_t buf[UINT32LEN] = {0};
    uint8_t i = 0;
    uint8_t *q = (uint8_t *) &value;
    memset(buf, 0 ,UINT32LEN);
    if (needConverse(0))
    {
        for (i = 0; i < UINT32LEN; i++)
        {
            buf[i] = *(q + UINT32LEN - i - 1);
        }
        return TlvItem_New(type, UINT32LEN, buf);
    }
    else
    {
        return TlvItem_New(type, UINT32LEN, q);
    }
}


TlvItem_t *TlvItem_Int16ValueNew(uint8_t type,uint16_t value)
{
    uint8_t buf[UINT16LEN] = {0};
    uint8_t i = 0;
    uint8_t *q = (uint8_t *) &value;
    memset(buf, 0 ,UINT16LEN);
    if (needConverse(0))
    {
        for (i = 0; i < UINT16LEN; i++)
        {
            buf[i] = *(q + UINT16LEN - i - 1);
        }
        return TlvItem_New(type, UINT16LEN, buf);
    }
    else
    {
        return TlvItem_New(type, UINT16LEN, q);
    }
}

void TlvItem_Free(TlvItem_t *item)
{
    if(NULL == item)
    {
        return;
    }
    if(NULL != item->value)
    {
        vPortFree(item->value);
        item->value = NULL;
    }
    vPortFree(item);
    item = NULL;
    return;
}

TlvItem_t *TlvItem_MallocTree(uint32_t len,uint8_t *buf)
{
    uint8_t *p = buf;
    uint8_t temp = 0;
    uint8_t msg_type = 0;
    uint32_t msg_lenth = 0;
    uint8_t offset = 0;
    TlvItem_t *item = NULL;
    if(NULL == p || 0 == len)
    {
        return NULL;
    }
    if(len > offset)
    {
        msg_type = (uint8_t)*p;
        offset++;
    }

    if(len > offset)
    {
        temp = (uint8_t)*(p + offset);
        msg_lenth = (~BIGLEN_MASK) & temp;
        offset++;
        if((BIGLEN_MASK & temp) && (offset < len))
        {
            temp = (uint8_t)*(p+offset);
            msg_lenth = msg_lenth * BIGLEN_MASK + ((~BIGLEN_MASK) & temp);
            offset++;
        }
    }

    if(msg_lenth > len - offset)
    {
        return NULL;
    }

    if (msg_type & CHILD_MASK)
    {
        item = TlvItem_New(msg_type&~CHILD_MASK,msg_lenth,NULL);
        item->childNode = TlvItem_MallocTree(msg_lenth,p + offset);
        item->brotherNode = TlvItem_MallocTree(len - msg_lenth - offset,p + offset + msg_lenth);
    }
    else
    {
        item = TlvItem_New(msg_type&~CHILD_MASK,msg_lenth,p + offset);
        item->childNode = NULL;
        item->brotherNode = TlvItem_MallocTree(len - msg_lenth - offset,p + offset + msg_lenth);
    }

    if(item->childNode != NULL)
    {
        item->childNode->parentNode = item;
    }
    if(item->brotherNode != NULL)
    {
        item->brotherNode->parentNode = item->parentNode;
    }
    return item;
}

TlvItem_t *TlvItem_MallocTree_long_msg(uint32_t len,uint8_t *buf)
{
    uint8_t *p = buf;
    uint8_t temp = 0;
    uint8_t msg_type = 0;
    uint32_t msg_lenth = 0;
    uint8_t offset = 0;
    TlvItem_t *item = NULL;
    if(NULL == p || 0 == len)
    {
        return NULL;
    }
    if(len > offset)
    {
        msg_type = (uint8_t)*p;
        offset++;
    }

    if(len > offset)
    {
        temp = (uint8_t)*(p + offset);
        msg_lenth = (~BIGLEN_MASK) & temp;
        offset++;
        if((BIGLEN_MASK & temp) && (offset < len))
        {
            temp = (uint8_t)*(p+offset);
            msg_lenth = msg_lenth * BIGLEN_MASK + ((~BIGLEN_MASK) & temp);
            offset++;
        }
    }

    if(msg_lenth > len - offset)
    {
        msg_lenth = len - offset;
    }

    if (msg_type & CHILD_MASK)
    {
        item = TlvItem_New(msg_type&~CHILD_MASK,msg_lenth,NULL);
        item->childNode = TlvItem_MallocTree_long_msg(msg_lenth,p + offset);
        item->brotherNode = TlvItem_MallocTree_long_msg(len - msg_lenth - offset,p + offset + msg_lenth);
    }
    else
    {
        item = TlvItem_New(msg_type&~CHILD_MASK,msg_lenth,p + offset);
        item->childNode = NULL;
        item->brotherNode = TlvItem_MallocTree_long_msg(len - msg_lenth - offset,p + offset + msg_lenth);
    }

    if(item->childNode != NULL)
    {
        item->childNode->parentNode = item;
    }
    if(item->brotherNode != NULL)
    {
        item->brotherNode->parentNode = item->parentNode;
    }
    return item;
}


void TlvItem_FreeTree(TlvItem_t *tree)
{
    if(NULL == tree)
    {
        return;
    }

    TlvItem_FreeTree(tree->childNode);
    TlvItem_FreeTree(tree->brotherNode);
    TlvItem_Free(tree);
}

void TlvItem_Printf(uint32_t spaceNumber, TlvItem_t *item)
{
    const char* format1 = "t:%d,l:%d";
    const char* format2 = ",v:";
    const char* format3 = "%x,";
    uint32_t len = 0;
    uint32_t i = 0;
    char* buf = NULL;
    char* value = NULL;
    char temp[BUFSIZE];

    if(NULL == item)
    {
//      DBG_APP_MSG(DBG_ZONE_INTERFACE,("TlvItem_Printf item is NULL\r\n"));
        return;
    }
    value = (char *)item->value;

    len = strlen(format1) + strlen(format2) + strlen(format3) * item->length + spaceNumber + 1;
    buf = (char *)pvPortMalloc(len);
    if(NULL == buf)
    {
//	DBG_APP_MSG(DBG_ZONE_INTERFACE,("TlvItem_Printf Malloc buf faild\r\n"));
        return;
    }
    memset(buf, 0, len);
    memset(temp, 0 ,BUFSIZE);
    for (i = 0; i < spaceNumber; i++)
    {
        temp[i] = ' ';
    }
    strlcat(buf, temp, spaceNumber);

    memset(temp, 0 ,BUFSIZE);
    snprintf(temp, BUFSIZE, format1,item->type,item->length);
    strlcat(buf, temp, len);

    /*lint -e539*/
    for(i = 0; i < item->length && NULL != value; i++)
    {
        if(0 == i)
        {
             memset(temp, 0 ,BUFSIZE);
             snprintf(temp, BUFSIZE, format2);
             strlcat(buf, temp, len);
        }
    	else
    	{

    	}
        memset(temp, 0 ,BUFSIZE);
        snprintf(temp, BUFSIZE, format3,value[i]);
        strlcat(buf, temp, len);
    }
    buf[len - 1] = '\0';
    //	DBG_APP_MSG(DBG_ZONE_INTERFACE,("%s\r\n",buf));
    vPortFree(buf);
    buf = NULL;
    return;
}

void TlvItem_DUMP(uint32_t n,TlvItem_t *tree)
{
    if(tree == NULL)
    {
        return;
    }
    TlvItem_Printf(n, tree);
    TlvItem_DUMP(n + 4,tree->childNode);
    TlvItem_DUMP(n, tree->brotherNode);
}

void TlvItem_AddChild(TlvItem_t *parent, TlvItem_t *child)
{
    TlvItem_t *p = parent;
    if(NULL == p || NULL == child)
    {
        //DBG_APP_MSG(DBG_ZONE_INTERFACE,("TlvItem_AddChild NULL PARAM\r\n"));
        return;
    }
    if(NULL != p->value)
    {
        //DBGPRINTF_HIGH("Parent Value is Not NULL,is Leaf");
        //DBG_APP_MSG(DBG_ZONE_INTERFACE,("Parent Value is Not NULL,is Leaf\r\n"));
        return;
    }

    //??????,????????
    child->parentNode = p;

    if (NULL == p->childNode)
    {
        p->childNode = child;
        updateParentLen(child);
    }
    else
    {
        TlvItem_AddBrother(p->childNode, child);
    }
}


void TlvItem_AddBrother(TlvItem_t *brother, TlvItem_t *me)
{
    TlvItem_t *b = brother;
    if(NULL == b || NULL == me)
    {
        //DBGPRINTF_HIGH("TlvItem_AddBrother NULL PARAM");
        //DBG_APP_MSG(DBG_ZONE_INTERFACE,("TlvItem_AddBrother NULL PARAM\r\n"));
        return;
    }
    while(b->brotherNode != NULL)
    {
        b = b->brotherNode;
    }
    b->brotherNode = me;
    me->parentNode = b->parentNode;
    updateParentLen(me);
}

/*????*/
TlvItem_t *TlvItem_ItemWithType(TlvItem_t *root, uint8_t type)
{
    TlvItem_t *node = NULL;
    if (NULL == root)
    {
        return NULL;
    }
    if(type == root->type)
    {
        return root;
    }
    node = TlvItem_ItemWithType(root->childNode, type);
    if(NULL == node)
    {
        node = TlvItem_ItemWithType(root->brotherNode, type);
    }
    return node;
}

uint8_t *TlvItem_ItemValueWithType(TlvItem_t *root, uint8_t type, uint32_t *len)
{
    TlvItem_t *node = NULL;
    if(NULL == root)
    {
        *len = 0;
        return NULL;
    }
    node = TlvItem_ItemWithType(root, type);
    if(NULL != node && NULL != node->value)
    {
        *len = node->length;
        return node->value;
    }
    else
    {
        *len = 0;
        return NULL;
    }
}


TlvItem_t* TlvItem_Next(TlvItem_t *root)
{
    if(NULL == root)
    {
        return NULL;
    }
    return root->brotherNode;
}


TlvItem_t* TlvItem_Child(TlvItem_t *root)
{
    if(NULL == root)
    {
        return NULL;
    }
    return root->childNode;
}

uint8_t TlvItem_ItemType(TlvItem_t *item)
{
    if(NULL == item)
    {
        return 0;
    }
    return item->type;
}


uint8_t* TlvItem_ItemValue(TlvItem_t *item,uint32_t *length)
{
    if(NULL == item)
    {
        *length = 0;
        return NULL;
    }
    if(NULL == item->value)
    {
        *length = 0;
    }
    else
    {
        *length = item->length;
    }
    return item->value;
}

void TlvItem_ItemToBuf(TlvItem_t *item,uint8_t *buf,uint32_t length)
{
    uint8_t *p = buf;
    uint32_t lenSize = 0;
    uint32_t itemLen = 0;
    uint32_t temp;
    uint32_t i = 0;
    if(NULL == item || NULL == p)
    {
        return;
    }
    if(0 == length)
    {
        return;
    }
    itemLen = item->length + TlvItem_GetLenSize(item->length) + TYPESIZE;
    if(itemLen > length)
    {
        return;
    }

    if(NULL == item->value && item->length > 0)
    {
        *p = (uint8_t)(item->type | CHILD_MASK);
    }
    else
    {
        *p = (uint8_t)item->type;
    }
    p++;
    lenSize = TlvItem_GetLenSize(item->length);
    temp = item->length;
    for(i = 1; i <= lenSize; i++)
    {
        if(1 == i)
        {
            *(p + lenSize - i) = (uint8_t)(temp % BIGLEN_MASK);
        }
        else
        {
            *(p + lenSize - i) = (uint8_t)((temp % BIGLEN_MASK) | BIGLEN_MASK);
        }
        temp = temp / BIGLEN_MASK;
    }
    p += lenSize;

    if(NULL == item->value)
    {
        TlvItem_ItemToBuf(item->childNode,p,item->length);
    }
    else
    {
        memcpy(p, item->value, item->length);
    }
    p += item->length;
    TlvItem_ItemToBuf(item->brotherNode,p,length - itemLen);
    return;
}

uint8_t* TlvItem_MallocStrFromItem(TlvItem_t *item,uint32_t *length)
{
    uint32_t len = 0;
    TlvItem_t *p = item;
    uint8_t *buf = NULL;
    while(NULL != p)
    {
        len += p->length + TYPESIZE + TlvItem_GetLenSize(p->length);
        p = p->brotherNode;
    }
    if(0 == len)
    {
        //DBGPRINTF_HIGH("item length is 0");
        //DBG_APP_MSG(DBG_ZONE_INTERFACE,("item length is 0\r\n"));

        *length = 0;
        return NULL;
    }
    buf = (uint8_t *)pvPortMalloc(len);
    if(NULL == buf)   //jack.bi add 2015/7/13
    {
        return NULL;
    }
    memset(buf, 0, len);
    *length = len;
    TlvItem_ItemToBuf(item, buf, len);
    return buf;
}

uint8_t TlvItem_ItemInt16Value(TlvItem_t *item, uint16_t *outParam,uint8_t dump)
{
    uint8_t* buf = NULL;
    uint16_t ret = 0;
    uint8_t *r = (uint8_t *) &ret;
    uint8_t i = 0;
    if(NULL == item || NULL == item->value || NULL == outParam || UINT16LEN != item->length)
    {
        //DBGPRINTF_HIGH(" TlvItem_ItemIntValue item is not have int value");
        //DBG_APP_MSG(DBG_ZONE_INTERFACE,("TlvItem_ItemIntValue item is not have int value\r\n"));
        return 1;
    }
    buf = item->value;
    if (needConverse(dump))
    {
        for (i = 0; i < UINT16LEN; i++)
        {
        /*****/
        //*r0 =buf1
        //r1=buf0
        /*****/
        *(r + i) = buf[UINT16LEN - i - 1];
        }
    }
    else
    {
        for (i = 0; i < UINT16LEN; i++)
        {
            *(r + i) = buf[i];
        }
    }
    *outParam = ret;
    return 0;
}


uint8_t TlvItem_ItemInt32Value(TlvItem_t *item, uint32_t *outParam,uint8_t dump)
{
    uint8_t* buf = NULL;
    uint32_t ret = 0;
    uint8_t *r = (uint8_t *)&ret;
    uint8_t i = 0;
    if(NULL == item || NULL == item->value || NULL == outParam || UINT32LEN != item->length)
    {
        //DBGPRINTF_HIGH(" TlvItem_ItemIntValue item is not have int value");
        //DBG_APP_MSG(DBG_ZONE_INTERFACE,("TlvItem_ItemIntValue item is not have int value\r\n"));
        return 1;
    }
    buf = item->value;
    if (needConverse(dump))
    {
        for (i = 0; i < UINT32LEN; i++)
        {
            *(r + i) = buf[UINT32LEN - i - 1]; //buf[3]-->*r0 buf[2]-->*r1  buf[3]-->*r
        }
    }
    else
    {
        for (i = 0; i < UINT32LEN; i++)
        {
            *(r + i) = buf[i];
        }
    }
    *outParam = ret;
    return 0;
}

/*********************yql********************************/

void BlePvsSendNotify(uint8_t *send_data,uint16_t data_length)
{
//    ProtocolPushToBleQeue(send_data,data_length);   
	am_msg_task_send(AM_TASK_PROTOCOL,AM_TASK_BLE,0,send_data);	
}

/********************************************************/
/** @}*/
