//this file is used for complete the remainder function
/*******************************************************************************
            Copyright (C), 2017, GoerTek Technology Co., Ltd.
*******************************************************************************/
/**
  *  @defgroup	remainder module name
  *  @brief 	remainder
  *  @file 		remainder
  *  @author 	Sego.Zhang
  *  @version 	1.0
  *  @date    	2017/8/19
  *  @{
  *//*
  *  FileName      : remainder
  *  Description   : ??
  *  FunctionList  :
              		Remainder
  * History        :
  * 1.Date         : 2017/8/19
  *    Author      : Sego.Zhang
  *    Modification: Created file
*******************************************************************************/

/*----------------------------------------------*
 * Include File                                 *
 *----------------------------------------------*/
#include <stdio.h>
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
/** ??
  *
  * @param[in] x1
  * @param[in] x2
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/8/19
  *   Author       : Sego.Zhang
  *   Modification : Created function

*******************************************************************************/
int Remainder( int x1, int x2 )
{
    int remainder_value;
    if(x2 == 0)
    {
        printf("Please make sure the second param is not zero\n");
        return -1;
    }
    else
    {
        remainder_value = x1 % x2;
        printf("remainder_value is  %d\n", remainder_value);
        return remainder_value;
    }
}

/** @}*/

