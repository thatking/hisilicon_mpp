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

static char **s_ppChCmdArgv = NULL;
/******************************************************************************
* function : to process abnormal case
******************************************************************************/
#ifndef __HuaweiLite__
HI_VOID SAMPLE_IVE_HandleSig(HI_S32 s32Signo)
{
    signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
    
    if (SIGINT == s32Signo || SIGTERM == s32Signo)
    {
		switch (*s_ppChCmdArgv[1])
	    {
	        case '0':
		        {
		           SAMPLE_IVE_Canny_HandleSig();
		        }
		        break;
	        case '1':
		        {
					SAMPLE_IVE_BgModel_HandleSig();
		        }
		        break;
	        case '2':
		        {				
					SAMPLE_IVE_Gmm_HandleSig();
		        }
		        break;
			case '3':
				{
					SAMPLE_IVE_Gmm2_HandleSig();
				}
				break;			
	        case '4':
		        {				
					SAMPLE_IVE_Od_HandleSig();
		        }
		        break;		
			case '5':
				{
					SAMPLE_IVE_Md_HandleSig();
				}
				break;
	        case '6':
		        {				
					SAMPLE_IVE_TestMemory_HandleSig();
		        }
		        break;
	        case '7':
		        {				
					SAMPLE_IVE_Sobel_HandleSig();
		        }
		        break;
	        default :
		        {
		        }
		        break;
	    }
		
        //SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}
#endif
/******************************************************************************
* function : show usage
******************************************************************************/
HI_VOID SAMPLE_IVE_Usage(HI_CHAR* pchPrgName)
{
    printf("Usage : %s <index> [complete] [encode] [vo]\n", pchPrgName);
    printf("index:\n");
    printf("\t 0)Canny,<complete>:0, part canny;1,complete canny.(FILE->IVE->FILE).\n");
    printf("\t 1)BgModel,<encode>:0, not encode;1,encode.<vo>:0,not call vo;1,call vo.(VI->VPSS->IVE->VGS->[VENC_H264]->[VO]).\n");
    printf("\t 2)Gmm,<encode>:0, not encode;1,encode.<vo>:0,not call vo;1,call vo.(VI->VPSS->IVE->VGS->[VENC_H264]->[VO]).\n");
    printf("\t 3)Gmm2.(FILE->IVE->FILE).\n");
	printf("\t 4)Occlusion detected.(VI->VPSS->IVE->VO).\n");
	printf("\t 5)Motion detected.(VI->VPSS->IVE->VGS->VO).\n");
	printf("\t 6)MemoryTest.(FILE->IVE->FILE).\n");
    printf("\t 7)Sobel.(FILE->IVE->FILE).\n");

}


/******************************************************************************
* function : ive sample
******************************************************************************/
#ifdef __HuaweiLite__
int app_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    HI_S32 s32Ret = HI_SUCCESS;
	
    if ((argc < 2) || (1 != strlen(argv[1])))
    {
        SAMPLE_IVE_Usage(argv[0]);
        return HI_FAILURE;
    }

#ifndef __HuaweiLite__
    signal(SIGINT, SAMPLE_IVE_HandleSig);
    signal(SIGTERM, SAMPLE_IVE_HandleSig);
#endif

	s_ppChCmdArgv = argv;
    switch (*argv[1])
    {
        case '0':
	        {
	            if ((argc < 3) || (1 != strlen(argv[2])) || (('0' != *argv[2]) && ('1' != *argv[2])))
	            {
	                SAMPLE_IVE_Usage(argv[0]);
	                return HI_FAILURE;
	            }
	            SAMPLE_IVE_Canny(*argv[2]);
	        }
	        break;
        case '1':
	        {
	            if ((argc < 4) || (1 != strlen(argv[2])) || (1 != strlen(argv[3])) \
					|| (('0' != *argv[2]) && ('1' != *argv[2]))\
					|| (('0' != *argv[3]) && ('1' != *argv[3])))
	            {
	                SAMPLE_IVE_Usage(argv[0]);
	                return HI_FAILURE;
	            }
	            SAMPLE_IVE_BgModel(*argv[2], *argv[3]);
	        }
	        break;
        case '2':
	        {
	            if ((argc < 4) || (1 != strlen(argv[2])) || (1 != strlen(argv[3]))\
					|| (('0' != *argv[2]) && ('1' != *argv[2]))\
					|| (('0' != *argv[3]) && ('1' != *argv[3])))
	            {
	                SAMPLE_IVE_Usage(argv[0]);
	                return HI_FAILURE;
	            }
	            SAMPLE_IVE_Gmm(*argv[2], *argv[3]);
	        }
	        break;
		case '3':
			{
				SAMPLE_IVE_Gmm2();
			}
			break;			
        case '4':
	        {
	            SAMPLE_IVE_Od();
	        }
	        break;		
		case '5':
			{
				SAMPLE_IVE_Md();	   
			}
			break;
        case '6':
	        {
	            SAMPLE_IVE_TestMemory();
	        }
	        break;
        case '7':
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



