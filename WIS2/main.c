/*******************************************************************************
            Copyright (C), 2017, GoerTek Technology Co., Ltd.
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
     add(first_num,second_num);
    /// the following code to be used for subtract first_num - second_num functin call
     Subtract(first_num,second_num);
    /// the following code to be used for multiply first_num*second_num functin call
      Multiply(first_num,second_num);
    /// the following code to be used for divide first_num/second_num functin call  
      divide(first_num,second_num);
    /// the following code to be used for remainder first_num%second_num functin call  
     int AA;
     AA=GetRemainder(first_num,second_num);
     printf("%d\n",AA); 
    /// the following code to be used for odd or even first_num(second_num) is odd or even functin call 
      OddEven(first_num);
  system("PAUSE");	
  return 0;
}


/** @}*/
