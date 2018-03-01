#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "hi_comm_vo.h"
#include "hi_comm_vi.h"
#include "hi_comm_vpss.h"

#include "mpi_vb.h"
#include "mpi_sys.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_vpss.h"

#define USAGE_HELP(void)\
{\
    printf("\n\tusage : %s para value group [chn] \n", argv[0]);    \
    printf("\n\t para: \n");    \
    printf("\t\tenIE   [0, disable; 1,enable]\n");   \
    printf("\t\tenNR   [0, disable; 1,enable]\n");    \
    printf("\t\tenDEI  [0, auto; 1,die; 1, nodie]\n");   \
    printf("\t\tenHIST  [0, disable; 1,enable]\n");   \
    printf("\t\tie     [IE强度，value:0~255, default:32]\n");   \
    printf("\t\tiesp   [IE锐度，value:0~7,  default:7]\n");   \
    printf("\t\tlum   [亮度，value:0~48, default:32]\n");   \
    printf("\t\tcon   [对比度，value:0~48, default:8]\n");   \
    printf("\t\tde    [暗区增强，value:0~48, default:16]\n");   \
    printf("\t\tbe    [亮区增强，value:0~48, default:16]\n");   \
    printf("\t\tdei   [de-interlace强度，value:0~7, default:0]\n");   \
    printf("\t\tsf    [空域去噪强度，value:0~7, default:3]\n");   \
    printf("\t\ttf    [时域去噪强度，value:0~15, default:1]\n");   \
    printf("\t\tmt    [运动判断阈值，value:0~7, default:1]\n");   \
    printf("\t\tenSP   [0, disable; 1,enable]\n");   \
    printf("\t\tchnsp [sp strength of chn，value:0~255, default:40]\n");  \
}

#define CHECK_RET(express,name)\
    do{\
        if (HI_SUCCESS != express)\
        {\
            printf("%s failed at %s: LINE: %d ! errno:%d \n", \
                name, __FUNCTION__, __LINE__, express);\
            return HI_FAILURE;\
        }\
    }while(0)


HI_S32 main(int argc, char *argv[])
{
    HI_S32 s32Ret;
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr;
    VPSS_GRP_PARAM_S stVpssGrpParam;
    VPSS_CHN_SP_PARAM_S stChnSpParam;
    VPSS_CHN_NR_PARAM_S stChnNrParam;
    
    HI_U8 para[16];
    HI_U32 value = 0;
    VPSS_GRP VpssGrp = 0;
    VPSS_CHN VpssChn = 0;
    
    if (argc < 4)
    {
        USAGE_HELP();
        return -1;
    }
    
    strcpy((char *)para,argv[1]);  
    value = atoi(argv[2]);
    VpssGrp = atoi(argv[3]);
    if (5 == argc)
    {
        VpssChn = atoi(argv[4]);
    }

    s32Ret = HI_MPI_VPSS_GetGrpAttr(VpssGrp, &stVpssGrpAttr);
    CHECK_RET(s32Ret, "HI_MPI_VPSS_GetGrpAttr");

    s32Ret = HI_MPI_VPSS_GetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
    CHECK_RET(s32Ret, "HI_MPI_VPSS_GetChnAttr");

    s32Ret = HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssGrpParam);
    CHECK_RET(s32Ret, "HI_MPI_VPSS_GetParam");

    s32Ret = HI_MPI_VPSS_GetChnSpParam(VpssGrp, VpssChn, &stChnSpParam);
    //CHECK_RET(s32Ret, "HI_MPI_VPSS_GetChnSpParam");

    s32Ret = HI_MPI_VPSS_GetChnNrParam(VpssGrp, VpssChn, &stChnNrParam);
    //CHECK_RET(s32Ret, "HI_MPI_VPSS_GetChnNrParam");

    if (0 == strcmp((const char *)para, "enIE"))
    {
       stVpssGrpAttr.bIeEn = value;
    }
    else if (0 == strcmp((const char *)para, "enNR"))
    {
        stVpssGrpAttr.bNrEn = value;
    }
    else if (0 == strcmp((const char *)para, "enDEI"))
    {
		if (value > 2) 
		{
			printf("invalid enDEI param %d\n", value);
			return -1;
		}
		
        stVpssGrpAttr.enDieMode = (VPSS_DIE_MODE_E)value;
	}
    else if (0 == strcmp((const char *)para, "enHIST"))
    {
        stVpssGrpAttr.bHistEn = value;
	}
    else if (0 == strcmp((const char *)para, "ie"))
    {
        stVpssGrpParam.u32IeStrength = value;
        
    }
    else if (0 == strcmp((const char *)para, "iesp"))
    {
        stVpssGrpParam.u32IeSharp = value;
        
    }
    else if (0 == strcmp((const char *)para, "lum"))
    {
        stVpssGrpParam.u32Luminance = value;
        
    }
    else if (0 == strcmp((const char *)para, "con"))
    {
        stVpssGrpParam.u32Contrast = value;
        
    }
    else if (0 == strcmp((const char *)para, "de"))
    {
        stVpssGrpParam.u32DarkEnhance = value;
        
    } 
    else if (0 == strcmp((const char *)para, "be"))
    {
        stVpssGrpParam.u32BrightEnhance = value;
        
    }   
    else if (0 == strcmp((const char *)para, "dei"))
    {
        stVpssGrpParam.u32DiStrength = value;
        
    }     
	
#if ((HICHIP == HI3521_V100) || (HICHIP == HI3520A_V100))
    else if (0 == strcmp((const char *)para, "sf"))
    {
        stChnNrParam.u32SfStrength = value;
        
    }    
    else if (0 == strcmp((const char *)para, "tf"))
    {
        stChnNrParam.u32TfStrength = value;
        
    } 
	
    else if (0 == strcmp((const char *)para, "mt"))
    {
        stChnNrParam.u32MotionThresh = value;
        
    }  
	
#elif ((HICHIP == HI3531_V100) || (HICHIP == HI3532_V100) || (HICHIP == HI3520D_V100))
    else if (0 == strcmp((const char *)para, "sf"))
    {
        stVpssGrpParam.u32SfStrength = value;
        
    }    
    else if (0 == strcmp((const char *)para, "tf"))
    {
        stVpssGrpParam.u32TfStrength = value;
        
    } 
	
    else if (0 == strcmp((const char *)para, "mt"))
    {
        stVpssGrpParam.u32MotionThresh = value;
        
    }  
#endif

    else if (0 == strcmp((const char *)para, "enSP"))
    {
        stVpssChnAttr.bSpEn = value;
    }
    else if (0 == strcmp((const char *)para, "chnsp"))    
    {
        stChnSpParam.u32LumaGain = value;
    }
    else
    {
        printf("err para\n");
        USAGE_HELP();
    }

    s32Ret = HI_MPI_VPSS_SetGrpAttr(VpssGrp, &stVpssGrpAttr);
    CHECK_RET(s32Ret, "HI_MPI_VPSS_SetGrpAttr");

    s32Ret = HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
    CHECK_RET(s32Ret, "HI_MPI_VPSS_SetChnAttr");

    s32Ret = HI_MPI_VPSS_SetGrpParam(VpssGrp, &stVpssGrpParam);
    CHECK_RET(s32Ret, "HI_MPI_VPSS_SetParam");

    s32Ret = HI_MPI_VPSS_SetChnSpParam(VpssGrp, VpssChn, &stChnSpParam);
    //CHECK_RET(s32Ret, "HI_MPI_VPSS_SetChnSpParam");

    s32Ret = HI_MPI_VPSS_SetChnNrParam(VpssGrp, VpssChn, &stChnNrParam);
    //CHECK_RET(s32Ret, "HI_MPI_VPSS_SetChnNrParam");

    printf("\t\tenIE   %d\n", stVpssGrpAttr.bIeEn);
    printf("\t\tenNR   %d\n", stVpssGrpAttr.bNrEn);
    printf("\t\tenDEI  %d\n", (stVpssGrpAttr.enDieMode == VPSS_DIE_MODE_NODIE)? 0:1);
    printf("\t\tenHIST  %d\n", stVpssGrpAttr.bHistEn);
    printf("\t\tie     %d\n", stVpssGrpParam.u32IeStrength);
    printf("\t\tiesp   %d\n", stVpssGrpParam.u32IeSharp);
    printf("\t\tlum    %d\n", stVpssGrpParam.u32Luminance);
    printf("\t\tcon    %d\n", stVpssGrpParam.u32Contrast);
    printf("\t\tde     %d\n", stVpssGrpParam.u32DarkEnhance);
    printf("\t\tbe     %d\n", stVpssGrpParam.u32BrightEnhance);
    printf("\t\tdei    %d\n", stVpssGrpParam.u32DiStrength);
#if ((HICHIP == HI3521_V100) || (HICHIP == HI3520A_V100))	
    printf("\t\tsf     %d\n", stChnNrParam.u32SfStrength);
    printf("\t\ttf     %d\n", stChnNrParam.u32TfStrength);
    printf("\t\tmt     %d\n", stChnNrParam.u32MotionThresh);
#elif ((HICHIP == HI3531_V100) || (HICHIP == HI3532_V100)|| (HICHIP == HI3520D_V100))
    printf("\t\tsf     %d\n", stVpssGrpParam.u32SfStrength);
    printf("\t\ttf     %d\n", stVpssGrpParam.u32TfStrength);
    printf("\t\tmt     %d\n", stVpssGrpParam.u32MotionThresh);
#endif
    printf("\t\tenSP   %d\n", stVpssChnAttr.bSpEn);
    printf("\t\tchnsp  %d\n", stChnSpParam.u32LumaGain);

    return 0;
}

