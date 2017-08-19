/*******************************************************************************
            Copyright (C), 2017, GoerTek Technology Co., Ltd.
*******************************************************************************/
/** 
  *  @defgroup	main module name
  *  @brief 	main 
  *  @file 		main
  *  @author 	freddie.zhang
  *  @version 	1.0
  *  @date    	2017/8/19
  *  @{
  *//*
  *  FileName      : main
  *  Description   : main function
  *  FunctionList  :
  * History        :
  * 1.Date         : 2017/8/19
  *    Author      : freddie.zhang
  *    Modification: Created file
*******************************************************************************/

/*----------------------------------------------*
 * Include File                                 *
 *----------------------------------------------*/
 #include "private.h"

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
/** main function
  * 
  * @param[in] argc
  * @param[in] argv[]
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
int main( int argc, char *argv[] )
{
    float num1 = 8;
    float num2 = 5;
    float num3 = 0;
    float float_result;
    int int_result;
    /// the following code to be used for add first_num + second_num functin call
    int_result = Add( num1, num2 ); 
    /// the following code to be used for subtract first_num - second_num functin call
    int_result = Subtract( num1, num2 ); 
    /// the following code to be used for multiply first_num*second_num functin call
    float_result = Multi( num1, num2 );  
    /// the following code to be used for divide first_num/second_num functin call  
    float_result = Divide( num1, num2 );
    float_result = Divide( num1, num3 );
    /// the following code to be used for remainder first_num%second_num functin call  
    int_result = Remainder( num1, num2 );
    /// the following code to be used for odd or even first_num(second_num) is odd or even functin call 
    int_result = OddEven(num1);
    int_result = OddEven(num2);
    system("PAUSE");	
    return 0;
}


/** @}*/


