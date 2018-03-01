/******************************************************************************
  A simple program of Hisilicon mpp implementation.
  Copyright (C), 2012-2020, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2013-7 Created
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "sample_comm_ive.h"

#include "./sample/sample_ive_main.h"


/******************************************************************************
* function : to process abnormal case                                         
******************************************************************************/
HI_VOID SAMPLE_IVE_HandleSig(HI_S32 s32Signo)
{
    if (SIGINT == s32Signo || SIGTSTP == s32Signo)
    {
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

/******************************************************************************
* function : show usage
******************************************************************************/
HI_VOID SAMPLE_IVE_Usage(HI_CHAR *pchPrgName)
{
    printf("Usage : %s <index> [complete] [vi/file]\n", pchPrgName);
    printf("index:\n");
    printf("\t 0)Canny,<complete>:0, complete canny;1,part canny.\n");
    printf("\t 1)Occlusion detected.\n");	
    printf("\t 2)Motion detected,<vi/file>:0,vi;1,file.\n");
    printf("\t 3)MemoryTest.\n");
    printf("\t 4)Sobel.\n");   
	
}


/******************************************************************************
* function : ive sample
******************************************************************************/
int main(int argc, char *argv[])
{
    HI_S32 s32Ret = HI_SUCCESS;

    signal(SIGINT, SAMPLE_IVE_HandleSig);
    signal(SIGTERM, SAMPLE_IVE_HandleSig);

    if (argc < 2)
    {
        SAMPLE_IVE_Usage(argv[0]);
        return HI_FAILURE;
    }
        
    switch (*argv[1])
    {
    case '0':
    	{
			if ((argc < 3) || (('0' != *argv[2]) && ('1' != *argv[2])))
			{
				SAMPLE_IVE_Usage(argv[0]);
				return HI_FAILURE;
			}
        	SAMPLE_IVE_Canny(*argv[2]);        
    	}
		break;   
    case '1':
    	{
        	SAMPLE_IVE_Od();       
    	}
	 	break;
	case '2':
		{
			if ((argc < 3) || (('0' != *argv[2]) && ('1' != *argv[2])))
			{
				SAMPLE_IVE_Usage(argv[0]);
				return HI_FAILURE;
			}
			SAMPLE_IVE_Md(*argv[2]);	   
		}
		break;		
    case '3':
    	{
        	SAMPLE_IVE_TestMemory();       
    	}
	 	break;
    case '4':
    	{
        	SAMPLE_IVE_Sobel();        
    	}
		break;    
    default :
    	{
        	SAMPLE_IVE_Usage(argv[0]);        
    	}
		break;
    }
        
       

    return s32Ret;
}



