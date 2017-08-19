/*******************************************************************************
            Copyright (C), 2017, GoerTek Technology Co., Ltd.
*******************************************************************************/
/**
  *  @defgroup	add module name
  *  @brief 	add
  *  @file 		add
  *  @author 	buck.yin
  *  @version 	1.0
  *  @date    	2017/8/19
  *  @{
  *//*
  *  FileName      : add
  *  Description   : this file is used for complete the add function
  *  FunctionList  : Add
 
  * History        :
  * 1.Date         : 2017/8/19
  *    Author      : buck.yin
  *    Modification: Created file
*******************************************************************************/

/*----------------------------------------------*
 * Include File                                 *
 *----------------------------------------------*/

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


//this file is used for complete the add function
#include"private.h"
/******************************************************************************/
/** complete add two numbers
  *
  * @param[in] num_1
  * @param[in] num_2
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/8/19
  *   Author       : buck.yin
  *   Modification : Created function

*******************************************************************************/

int Add(int num_1, int num_2)
    {
        int sum;
        
        sum = num_1 + num_2;
        
        printf("%d\n", sum);//输出两数相加的结果sum 
        
        return 0;
    }

/** @}*/

