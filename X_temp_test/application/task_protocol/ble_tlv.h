/*******************************************************************************
            Copyright (C), 2015, GoerTek Technology Co., Ltd.
*******************************************************************************/
/*
  *  FileName      : bletlv
  *  Description   : tlv head
     tlv??????,????,???tlv???,?
     type0 + length0 + value0 + type1 + length1 + value1..

     ?????,?
     type0 + length0
           + type1 + length1 + value1
   		+ type2 + length2
   		        + type3 + length3 + value3

      ?????,??value?,length???????????
      ???value?,length?value????

      Type??1???,
      bit7???????????,0??????,1??????,???????Type????Value,??TL??,Length???????????
      bit6-0??Type??,????0-127,??127????????

      Length????:
      Length??Value???,?Get?Post???,???????????Type?Length??0,??????????????Type?????
      Length????????,???????bit7???????????????,bit7?0?,??length???????
*******************************************************************************/



#ifndef BLELIB_H
#define BLELIB_H
#include <stdint.h>
#include <stdbool.h>
//#include "AEE.h"
#include "protocol_config.h"

#define  CHILD_MASK  0x80
#define  BIGLEN_MASK 0x80

uint32_t TlvItem_GetLenSize(uint32_t len);
void TlvItem_Printf(uint32_t spaceNumber, TlvItem_t *item);
void TlvItem_ItemToBuf(TlvItem_t *item,uint8_t *buf,uint32_t length);

/******************************************************************************/
/** ?????????tree
  *
  * @param[in] len:????
  * @param[in] buf:?????
  *
  * @return ???tree????
  *
  * @note
  *
  *//*
*******************************************************************************/
TlvItem_t *TlvItem_MallocTree(uint32_t len,uint8_t *buf);
TlvItem_t *TlvItem_MallocTree_long_msg(uint32_t len,uint8_t *buf);

/******************************************************************************/
/** ??????tree?????????0
  *
  * @param[in] tree:root??
  *
  * @return
  *
  *//*
*******************************************************************************/
void TlvItem_FreeTree(TlvItem_t *tree);

/******************************************************************************/
/** ????tlv???item???
  * ??????,len?value??0?
  *
  * @param[in] type: tlv??type
  * @param[in] len:tlv??len
  * @param[in] value:tlv??value
  *
  * @return ??Item??,??????
  *
  * @note
  *
  *//*
*******************************************************************************/
TlvItem_t *TlvItem_New(uint8_t type,uint32_t len,uint8_t *value);

/******************************************************************************/
/** ????tlv???item??.???4,value?uint32??
  *
  * @param[in] type: tlv??type
  * @param[in] value:tlv??value
  *
  * @return ??Item??
  *
  * @note
  *
  *//*
*******************************************************************************/
TlvItem_t *TlvItem_Int32ValueNew(uint8_t type,uint32_t value);

/******************************************************************************/
/** ????tlv???item??.???4,value?uint16??
  *
  * @param[in] type: tlv??type
  * @param[in] value:tlv??value
  *
  * @return ??Item??
  *
  * @note
  *
  *//*
*******************************************************************************/
TlvItem_t *TlvItem_Int16ValueNew(uint8_t type,uint16_t value);

/******************************************************************************/
/** ??????item??,????????
  *
  * @param[in] item: ??tlv???item??
  *
  * @return
  *
  *//*
*******************************************************************************/
void TlvItem_Free(TlvItem_t *item);

/******************************************************************************/
/** ??????????????
  *
  * @param[in] n:????0,??????????'-'??
  * @param[in] tree:???tree??????
  *
  * @return
  *
  *//*
*******************************************************************************/
void TlvItem_DUMP(uint32_t n,TlvItem_t *tree);

/******************************************************************************/
/** ?????
  *
  * @param[in] parent:???,????value????,???????
  * @param[in] child:???
  *
  * @return
  *
  *//*
*******************************************************************************/
void TlvItem_AddChild(TlvItem_t *parent, TlvItem_t *child);

/******************************************************************************/
/** ????????
  *
  * @param[in] brother:??
  * @param[in] me:??
  *
  * @return
  *
  *//*
*******************************************************************************/
void TlvItem_AddBrother(TlvItem_t *brother, TlvItem_t *me);

/******************************************************************************/
/** ??root???,????type?item
  *
  * @param[in] root:???
  * @param[in] type:tlv????type
  *
  * @return ????,??????,????NULL
  *
  * @note
  *
  *//*
*******************************************************************************/
TlvItem_t *TlvItem_ItemWithType(TlvItem_t *root, uint8_t type);

/******************************************************************************/
/** ??root???,????type?item,???value
  * ??????????item????,???NULL
  * @param[in] root:???
  * @param[in] type:tlv????type
  * @param[in] len:value???,??????????item????,???0
  *
  * @return value??,????NULL
  *
  * @note
  *
  *//*
*******************************************************************************/
uint8_t *TlvItem_ItemValueWithType(TlvItem_t *root, uint8_t type, uint32_t *len);

/******************************************************************************/
/** root???????
  *
  * @param[in]  root:???
  *
  * @return root???????,?????NULL
  *
  * @note
  *
  *//*
*******************************************************************************/
TlvItem_t* TlvItem_Next(TlvItem_t *root);

/******************************************************************************/
/** root??????????
  *
  * @param[in] root:???
  *
  * @return root??????????,?????NULL
  *
  * @note
  *
  *//*
*******************************************************************************/
TlvItem_t* TlvItem_Child(TlvItem_t *root);

/******************************************************************************/
/** ???????type
  *
  * @param[in] item:??
  *
  * @return item???type
  *
  * @note
  *
  *//*
*******************************************************************************/
uint8_t TlvItem_ItemType(TlvItem_t *item);

/******************************************************************************/
/** ???????value
  *
  * @param[in] item:??
  * @param[in] length:value???,??item????,???0
  *
  * @return item???type
  *
  * @note
  *
  *//*
*******************************************************************************/
uint8_t* TlvItem_ItemValue(TlvItem_t *item,uint32_t *length);

/******************************************************************************/
/** ?item???????,???????,???????
  *
  * @param[in] item:??
  * @param[in] length:????
  *
  * @return ????
  *
  * @note
  *
  *//*
*******************************************************************************/
uint8_t* TlvItem_MallocStrFromItem(TlvItem_t *item,uint32_t *length);

/******************************************************************************/
/** ???????Intvalue,??item???????4
  *
  * @param[in] item:??
  * @param[in] outParam: ???uint16??
  * @param[in] dump
  *
  * @return ??0,???1;
  *
  * @note
  *
  *//*
*******************************************************************************/
uint8_t TlvItem_ItemInt16Value(TlvItem_t *item ,uint16_t *outParam,uint8_t dump);

/******************************************************************************/
/** ???????Intvalue,??item???????4
  *
  * @param[in] item:??
  * @param[in] outParam: ???uint32??
  * @param[in] dump
  *
  * @return ??0,???1;
  *
  * @note
  *
  *//*
*******************************************************************************/
uint8_t TlvItem_ItemInt32Value(TlvItem_t *item ,uint32_t *outParam,uint8_t dump);

//
uint8_t IS_Header(uint8_t value);
void BlePvsSendNotify(uint8_t *send_data,uint16_t data_length);

#endif
