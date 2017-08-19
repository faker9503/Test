/*******************************************************************************
            Copyright (C), 2017, GoerTek Technology Co., Ltd.
*******************************************************************************/
/**
  *  @defgroup	remainder module name
  *  @brief 	remainder
  *  @file 		remainder
  *  @author 	foamy.zang
  *  @version 	1.0
  *  @date    	2017/8/19
  *  @{
  *//*
  *  FileName      : remainder
  *  Description   : remainder
  *  FunctionList  :
  * History        :
  * 1.Date         : 2017/8/19
  *    Author      : foamy.zang
  *    Modification: Created file
*******************************************************************************/

#include "Private.h"

/*****************************************************************************/
/** num1 and num2 remainder
  *
  * @param[in] num2
  * @param[in] num1
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/8/19
  *   Author       : foamy.zang
  *   Modification : Created function

******************************************************************************/
int Remainder( int num1 ,int num2 )
{
    int rem; //rem is the remainder
    rem = num1 % num2;//calculate the remainder of two numbers
    return rem;
}

