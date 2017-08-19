/*******************************************************************************
            Copyright (C), 2017, GoerTek Technology Co., Ltd.
*******************************************************************************/
/**
  *  @defgroup	main module name
  *  @brief 	main
  *  @file 		main
  *  @author 	daryl.liu
  *  @version 	1.0
  *  @date    	2017/8/19
  *  @{
  *//*
  *  FileName      : main
  *  Description   : main.c
  *  FunctionList  :
              		main
  * History        :
  * 1.Date         : 2017/8/19
  *    Author      : daryl.liu
  *    Modification: Created file
*******************************************************************************/

/*----------------------------------------------*
 * Include File                                 *
 *----------------------------------------------*/
#include "Private.h"
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

#define MAX_NUM 12
#define MIN_NUM 9
/*----------------------------------------------*
 * Function Declaration                         *
 *----------------------------------------------*/
int main(int argc, char *argv[])
{

    int first_num  = MAX_NUM;
    int second_num = MIN_NUM;

    int add_sum = 0;
    /// the following code to be used for add first_num + second_num functin call
    add_sum=Add(first_num, second_num);
    printf("Add_sum=%d\n", add_sum);
    /// the following code to be used for subtract first_num - second_num functin call

    /// the following code to be used for multiply first_num*second_num functin call

    /// the following code to be used for divide first_num/second_num functin call

    /// the following code to be used for remainder first_num%second_num functin call

    /// the following code to be used for odd or even first_num(second_num) is odd or even functin call

    system("PAUSE");
    return 0;
}


/** @}*/

/** @}*/

