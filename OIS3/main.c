/*******************************************************************************
            Copyright (C), 2017, GoerTek Technology Co., Ltd.
*******************************************************************************/
/**
  *  @defgroup	main module name
  *  @brief 	main
  *  @file 		main
  *  @author 	wilson
  *  @version 	1.0
  *  @date    	2017/8/14
  *  @{
  *//*
  *  FileName      : main
  *  Description   : do the function including add, substract, multiple,
                     divide,odd or even,
  *  FunctionList  :
  * History        :
  * 1.Date         : 2017/8/14
  *    Author      : wilson
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
#define MAX_NUM 12
#define MIN_NUM 9
/*----------------------------------------------*
 * Function Declaration                         *
 *----------------------------------------------*/
int main(int argc, char *argv[])
{

  int first_num  = MAX_NUM;
  int second_num = MIN_NUM;
  /// the following code to be used for add first_num + second_num functin call
  printf("first_num + second_num = %d\n",Add(first_num,second_num));
  /// the following code to be used for subtract first_num - second_num functin call
  printf("first_num - second_num = %d\n",Subtract(first_num,second_num));
  /// the following code to be used for multiply first_num*second_num functin call
  printf("first_num * second_num = %d\n",Multiply(first_num,second_num));
  /// the following code to be used for divide first_num/second_num functin call
  printf("first_num / second_num = %d\n",Divide(first_num,second_num));
  /// the following code to be used for remainder first_num%second_num functin call
  printf("first_num mod second_num = %d\n",GetRemainder(first_num,second_num));
  /// the following code to be used for odd or even first_num(second_num) is odd or even functin call
  if(IsEven(first_num))
  {
    puts("the num is even");
  }
  else
  {
    puts("the num is odd");
  }
  system("PAUSE");
  return 0;
}


/** @}*/
