/*******************************************************************************
            Copyright (C), 2017, GoerTek Technology Co., Ltd.
*******************************************************************************/
/**
  *  @defgroup	divide module name
  *  @brief 	divide
  *  @file 		divide
  *  @author 	roger.liu
  *  @version 	1.0
  *  @date    	2017/8/19
  *  @{
  *//*
  *  FileName      : divide
  *  Description   : Calculation the quotient of two numbers
  *  FunctionList  :
              		Divide
  * History        :
  * 1.Date         : 2017/8/19
  *    Author      : roger.liu
  *    Modification: Created file
*******************************************************************************/

/*----------------------------------------------*
 * Include File                                 *
 *----------------------------------------------*/
#include "private.h"

/******************************************************************************/
/** calculate the quotient
  *
  * @param[in] dividend
  * @param[in] divisor
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/8/19
  *   Author       : roger.liu
  *   Modification : Created function

*******************************************************************************/
float Divide(float dividend, float divisor)
{
    float quotient;
    if (divisor == 0)//verify the divisor is not 0
    {
        printf("divisor cannot be 0");
    }
    else
        quotient = dividend / divisor;//calculate the result
    printf("The result is %f",quotient);
    return 0;
    //test
}
