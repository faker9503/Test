/*******************************************************************************
            Copyright (C), 2017, GoerTek Technology Co., Ltd.
*******************************************************************************/
/** 
  *  @defgroup	divide module name
  *  @brief 	divide 
  *  @file 		divide
  *  @author 	freddie.zhang
  *  @version 	1.0
  *  @date    	2017/8/19
  *  @{
  *//*
  *  FileName      : divide
  *  Description   : get the result of division
  *  FunctionList  :
  * History        :
  * 1.Date         : 2017/8/19
  *    Author      : freddie.zhang
  *    Modification: Created file
*******************************************************************************/

/*----------------------------------------------*
 * Include File                                 *
 *----------------------------------------------*/
 #include"math.h"
 #include"stdio.h"

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
/** get the result of num1 divide num2
  * 
  * @param[in] num1
  * @param[in] num2
  * 
  * @return 
  * 
  * @note 
  * 
  *//* 
  * History        :
  * 1.Date         : 2017/8/19
  *   Author       : freddie.zhang
  *   Modification : Created function
 
*******************************************************************************/
float Divide( float num1, float num2 )
{
    /**store the result of calculation */
    float result = 0;

    /*judge if the divider is 0 */
    if (num2 == 0)
    {
        /**print the error */
        printf("%s","Error: the divider can not be 0!\r\n");
    }
    else
    {
        /**calculation */
        result = num1/num2;
        printf("%s%f\r\n","the result is:",result);
    }
    return result;
}
/** @}*/




