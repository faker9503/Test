//this file is used for complete the divide function
/******************************************************************************/
/** 相除，如果除数为0返回-9999999
  *
  * @param[in] a
  * @param[in] b
  *
  * @return
  *
  * @note
  *
  *//*
  * History        :
  * 1.Date         : 2017/8/19
  *   Author       : faker
  *   Modification : Created function

*******************************************************************************/
int Divide(int a , int b)
{
    if( b == 0)
    {
        return -9999999;
    }
	else
	{
		return a/b;
	}
   
}
