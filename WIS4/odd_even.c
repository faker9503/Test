/******************************************************************************/
/** To get the input patater is odd or evern
  *
  * @param[in] NumInput
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/8/19
  *   Author       : rain.yue
  *   Modification : Created function

*******************************************************************************/
#include "stdio.h"
#include "math.h"

int OddEven( int NumInput )
{
    int Num;

    Num = NumInput%2;
    if( Num == 0 )
    {
        printf("�������%dΪż��\n",NumInput)��
    }
    else
    {
        printf("�������%dΪ����\n",NumInput);
    }
    return 0;
}

//this file is used for complete the remainder function

