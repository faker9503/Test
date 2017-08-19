//this file is used for complete the add function
/*******************************************************************************
            Copyright (C), 2017, GoerTek Technology Co., Ltd.
*******************************************************************************/
/**
  *  @defgroup	add module name
  *  @brief 	add
  *  @file 		add
  *  @author 	Landy.zhao
  *  @version 	1.0
  *  @date    	2017/8/19
  *  @{
  *//*
  *  FileName      : add
  *  Description   : show comments information
  *  FunctionList  :
              		Add
  * History        :
  * 1.Date         : 2017/8/19
  *    Author      : Landy.zhao
  *    Modification: Created file
*******************************************************************************/

/*----------------------------------------------*
 * Include File                                 *
 *----------------------------------------------*/
#include "stdio.h"
#include "math.h"
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
/** Realize the two numbers add function
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
  *   Author       : Landy.zhao
  *   Modification : Created function

*******************************************************************************/
float Add( float num1, float num2 )
{
/**calculate the result  */
    float result = 0;
    result = num1 + num2;

/*print the calculate result */

    printf("%s%f\r\n","the add result is",result);
/**return the result */
    return result;

}

/** @}*/


