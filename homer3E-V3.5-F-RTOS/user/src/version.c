

/***************************************
**	FileName:Version.c
**	Time:
**	����汾����
**	����汾��Ϣ
****************************************/

#include "version.h"


/***********************
** ���ذ汾��
************************/

unsigned int read_fir_ver(void)
{
	unsigned int rv;
	
	rv = Major_Number * 10 + Minor_Number;
	
	return rv;
}



/****************File End*******************/


