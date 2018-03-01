/******************************************************************************
  A simple program of Hisilicon Hi35xx video input and output implementation.
  Copyright (C), 2010-2011, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2011-8 Created
******************************************************************************/

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

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

#include "sample_comm.h"


VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_PAL;
VO_INTF_TYPE_E g_enVoIntfType = VO_INTF_CVBS;
PIC_SIZE_E g_enPicSize = PIC_HD1080;


SAMPLE_VIDEO_LOSS_S gs_stVideoLoss;
HI_U32 gs_u32ViFrmRate = 0;
static SAMPLE_BIND_E g_enVioBind = SAMPLE_BIND_VI_VPSS_VO;

SAMPLE_VI_CONFIG_S g_stViChnConfig =
{
    .enViMode = PANASONIC_MN34220_SUBLVDS_1080P_30FPS,
    .enNorm   = VIDEO_ENCODING_MODE_AUTO,

    .enRotate = ROTATE_NONE,
    .enViChnSet = VI_CHN_SET_NORMAL,
    .enWDRMode  = WDR_MODE_NONE,
    .enFrmRate  = SAMPLE_FRAMERATE_DEFAULT,
};

SAMPLE_VO_CONFIG_S g_stVoConfig = 
{
    .u32DisBufLen = 3,
};

static SAMPLE_VPSS_ATTR_S g_stVpssAttr = 
{
    .VpssGrp = 0,
    .VpssChn = 1,
    .stVpssGrpAttr = 
    {
        .bDciEn     = HI_FALSE,
        .bHistEn    = HI_FALSE,
        .bIeEn      = HI_FALSE,
        .bNrEn      = HI_TRUE,
        .bSharpenEn = HI_FALSE,
        .enDieMode  = VPSS_DIE_MODE_NODIE,
        .enPixFmt   = PIXEL_FORMAT_YUV_SEMIPLANAR_420,
        .u32MaxW    = 1920,
        .u32MaxH    = 1080
    },
    
    .stVpssChnAttr =
    {
        .bBorderEn       = HI_FALSE,
        .bFlip           = HI_FALSE,
        .bMirror         = HI_FALSE,
        .bSpEn           = HI_FALSE,
        .s32DstFrameRate = -1,
        .s32SrcFrameRate = -1,
    },
    
    .stVpssChnMode =
    {
        .bDouble         = HI_FALSE,
        .enChnMode       = VPSS_CHN_MODE_USER,
        .enCompressMode  = COMPRESS_MODE_SEG,
        .enPixelFormat   = PIXEL_FORMAT_YUV_SEMIPLANAR_420,
        .u32Width        = 1920,
        .u32Height       = 1080
    },
    
    .stVpssExtChnAttr = 
    {
    }
};

static HI_S32 SAMPLE_VIO_StartVPSS(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, 
                            SAMPLE_VPSS_ATTR_S* pstVpssAttr, ROTATE_E enRotate);
static HI_S32 SAMPLE_VIO_StopVPSS(VPSS_GRP VpssGrp, VPSS_CHN VpssChn);


/******************************************************************************
* function : show usage
******************************************************************************/
HI_VOID SAMPLE_VIO_VoIntf_Usage(HI_VOID)
{
    printf("INTF:\n");
    printf("\t 0) vo cvbs output, default.\n");
    printf("\t 1) vo BT1120 output.\n");
}

HI_VOID SAMPLE_VIO_Case_Usage(HI_CHAR* sPrgNm)
{
    #ifdef HI_FPGA
    printf("Usage : %s <INDEX> [INTF]\n", sPrgNm);
    #else
    printf("Usage : %s <INDEX> \n", sPrgNm);
    #endif
    printf("INDEX:\n");
    printf("\t 0)online/offline VIO mode VI/VPSS - VO.      Embeded isp, phychn channel preview.\n");
    printf("\t 1)online/offline WDR switch mode VI(WDR)/VPSS - VO. Embeded isp, phychn channel preview.\n");
    printf("\t 2)online/offline FPN mode VI(FPN)/VPSS - VO. Embeded isp, frame mode FPN, phychn channel preview.\n");
    printf("\t 3)online/offline ROTATE mode VI/VPSS; - VO.  Embeded isp, phychn channel preview.\n");
    printf("\t 4)offline LDC mode VI/VPSS - VO.      Embeded isp, phychn channel preview.\n");
    printf("\t 5)offline DIS mode VI(DIS) - VO.       Embeded isp, phychn channel preview.\n");
    printf("\t 6)offline BT1120 mode VI(BT1120) - VO. Isp bypass, phychn channel preview.\n");
    printf("\t 7)online/offline Resolution switch mode VI/VPSS - VO. Embeded isp, resolution switch.\n");
}

HI_VOID SAMPLE_VIO_Usage(HI_CHAR* sPrgNm)
{
    SAMPLE_VIO_Case_Usage(sPrgNm);
    #ifdef HI_FPGA
    SAMPLE_VIO_VoIntf_Usage();
    #endif
}

/******************************************************************************
* function : to process abnormal case
******************************************************************************/
#ifndef __HuaweiLite__
void SAMPLE_VIO_HandleSig(HI_S32 signo)
{ 
    signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
    
    if (SIGINT == signo || SIGTERM == signo)
    {
        SAMPLE_COMM_ISP_Stop();
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}
#endif

static HI_U32 SAMPLE_VIO_SwitchWDRMode( SAMPLE_VI_CONFIG_S* pstViConfig)
{
    ISP_DEV IspDev = 0;
    VI_DEV ViDev = 0;
    VI_CHN ViChn = 0;
    ISP_WDR_MODE_S stWDRMode;
    ISP_INNER_STATE_INFO_S stInnerStateInfo = {0};
    HI_S32 s32Tries = 25;
    HI_S32 s32Ret;
#if 0
    s32Ret = HI_MPI_ISP_SetFMWState(IspDev, ISP_FMW_STATE_FREEZE);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return s32Ret;
    }
#endif
    // 1. Disable Chn and Dev
    s32Ret = HI_MPI_VI_DisableChn(ViChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    s32Ret = HI_MPI_VI_DisableDev(ViDev);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    // 2. configure MIPI Attr
    s32Ret = SAMPLE_COMM_VI_StartMIPI(pstViConfig);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return s32Ret;
    }
#if 0
    s32Ret = HI_MPI_ISP_SetFMWState(IspDev, ISP_FMW_STATE_RUN);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return s32Ret;
    }
#endif
    // 3. Set WDR Mode to ISP
    stWDRMode.enWDRMode = pstViConfig->enWDRMode;
    s32Ret = HI_MPI_ISP_SetWDRMode(IspDev, &stWDRMode);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    // 4. query WDR Switch Status
    while (s32Tries > 0)
    {
        HI_MPI_ISP_QueryInnerStateInfo(IspDev, &stInnerStateInfo);
        if (HI_TRUE == stInnerStateInfo.bWDRSwitchFinish)
        {
            SAMPLE_PRT("wdr switch finish!\n");
            break;
        }
        //printf("query ISP inner state ... ...\n");
        usleep(20000);
        s32Tries--;
    }

    // 5. Start Dev and Chn again
    s32Ret = SAMPLE_COMM_VI_StartDev(ViDev, pstViConfig->enViMode);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    usleep(100000);
    s32Ret = HI_MPI_VI_EnableChn(ViChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_S32 SAMPLE_VIO_SwitchResolution(SAMPLE_VI_CONFIG_S *pstViConfig, SAMPLE_VPSS_ATTR_S *pstVpssAttr, PIC_SIZE_E a_enPicSize[])
{
    VI_CHN  ViChn  = 0;
    VI_DEV  ViDev  = 0;
    VPSS_GRP VpssGrp = 0;
    VPSS_CHN VpssChn = 1;
    ISP_DEV IspDev = 0;
    ISP_PUB_ATTR_S stPubAttr;
    SIZE_S stTargetSize;
    RECT_S stCapRect;
    ISP_INNER_STATE_INFO_S stInnerStateInfo;
    HI_S32 s32Ret;

    s32Ret = HI_MPI_ISP_GetPubAttr(IspDev, &stPubAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    SAMPLE_COMM_VI_SwitchResParam(pstViConfig, &stPubAttr, &stCapRect);
    stTargetSize.u32Width = stCapRect.u32Width;
    stTargetSize.u32Height = stCapRect.u32Height;

    s32Ret = HI_MPI_VI_DisableChn(ViChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    s32Ret = HI_MPI_VI_DisableDev(ViDev);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    
    s32Ret = SAMPLE_COMM_VI_StartMIPI(pstViConfig);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_ISP_SetPubAttr(IspDev, &stPubAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    while (1)
    {
        HI_MPI_ISP_QueryInnerStateInfo(IspDev, &stInnerStateInfo);
        if (HI_TRUE == stInnerStateInfo.bResSwitchFinish)
        {
            SAMPLE_PRT("Res switch finish!\n");
            break;
        }
        usleep(1000);
    }

    s32Ret = SAMPLE_COMM_VI_StartDev(ViDev, pstViConfig->enViMode);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_VI_StartChn(ViChn, &stCapRect, &stTargetSize, pstViConfig);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    SAMPLE_VIO_StopVPSS(VpssGrp, VpssChn);
    pstVpssAttr->stVpssGrpAttr.u32MaxW = stTargetSize.u32Width;
    pstVpssAttr->stVpssGrpAttr.u32MaxH = stTargetSize.u32Height;
    pstVpssAttr->stVpssChnMode.u32Width = stTargetSize.u32Width;
    pstVpssAttr->stVpssChnMode.u32Height= stTargetSize.u32Height;
    SAMPLE_VIO_StartVPSS(VpssGrp, VpssChn, pstVpssAttr, pstViConfig->enRotate);

    return HI_SUCCESS;
}

static HI_S32 SAMPLE_VIO_StartVO(SAMPLE_VO_CONFIG_S *pstVoConfig)
{
    VO_DEV VoDev = SAMPLE_VO_DEV_DSD0;
    //VO_CHN VoChn = 0;
    VO_LAYER VoLayer = 0;
    SAMPLE_VO_MODE_E enVoMode = VO_MODE_1MUX;
    VO_PUB_ATTR_S stVoPubAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    HI_S32 s32Ret = HI_SUCCESS;

    stVoPubAttr.enIntfType = g_enVoIntfType;
    if (VO_INTF_BT1120 == g_enVoIntfType)
    {
        stVoPubAttr.enIntfSync = VO_OUTPUT_1080P30;
        gs_u32ViFrmRate = 50;
    }
    else
    {
        stVoPubAttr.enIntfSync = VO_OUTPUT_PAL;
    }
    stVoPubAttr.u32BgColor = 0x000000ff;

    /* In HD, this item should be set to HI_FALSE */
    s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartDev failed!\n");
        return s32Ret;
    }

    stLayerAttr.bClusterMode = HI_FALSE;
    stLayerAttr.bDoubleFrame = HI_FALSE;
    stLayerAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;

    stLayerAttr.stDispRect.s32X = 0;
    stLayerAttr.stDispRect.s32Y = 0;

    s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync,
                                  &stLayerAttr.stDispRect.u32Width, &stLayerAttr.stDispRect.u32Height,
                                  &stLayerAttr.u32DispFrmRt);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_GetWH failed!\n");
        SAMPLE_COMM_VO_StopDev(VoDev);
        return s32Ret;
    }

    stLayerAttr.stImageSize.u32Width  = stLayerAttr.stDispRect.u32Width;
    stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;

    if (pstVoConfig->u32DisBufLen)
    {
        s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr, HI_FALSE);
    }
    else
    {
        s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr, HI_TRUE);
    }
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartLayer failed!\n");
        SAMPLE_COMM_VO_StopDev(VoDev);
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_VO_StartChn(VoDev, enVoMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartChn failed!\n");
        SAMPLE_COMM_VO_StopLayer(VoLayer);
        SAMPLE_COMM_VO_StopDev(VoDev);
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_S32 SAMPLE_VIO_StopVO(void)
{
    VO_DEV VoDev = SAMPLE_VO_DEV_DSD0;
    //VO_CHN VoChn = 0;
    VO_LAYER VoLayer = 0;
    SAMPLE_VO_MODE_E enVoMode = VO_MODE_1MUX;
    
    SAMPLE_COMM_VO_StopChn(VoDev, enVoMode);
    SAMPLE_COMM_VO_StopLayer(VoLayer);
    SAMPLE_COMM_VO_StopDev(VoDev);

    return HI_SUCCESS;
}

static HI_S32 SAMPLE_VIO_StartVPSS(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, 
                            SAMPLE_VPSS_ATTR_S* pstVpssAttr, ROTATE_E enRotate)
{
    VPSS_GRP_ATTR_S *pstVpssGrpAttr = NULL;
    VPSS_CHN_ATTR_S *pstVpssChnAttr = NULL;
    VPSS_CHN_MODE_S *pstVpssChnMode = NULL;
    VPSS_EXT_CHN_ATTR_S *pstVpssExtChnAttr = NULL;
    HI_S32 s32Ret = HI_SUCCESS;

    if (NULL != pstVpssAttr)
    {
        pstVpssGrpAttr = &pstVpssAttr->stVpssGrpAttr;
        pstVpssChnAttr = &pstVpssAttr->stVpssChnAttr;
        pstVpssChnMode = &pstVpssAttr->stVpssChnMode;
        pstVpssExtChnAttr = &pstVpssAttr->stVpssExtChnAttr;
    }
    else
    {
        pstVpssGrpAttr = &g_stVpssAttr.stVpssGrpAttr;
        pstVpssChnAttr = &g_stVpssAttr.stVpssChnAttr;
        pstVpssChnMode = &g_stVpssAttr.stVpssChnMode;
        pstVpssExtChnAttr = &g_stVpssAttr.stVpssExtChnAttr;
    }
    
    s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp, pstVpssGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start VPSS GROUP failed!\n");
        return s32Ret;
    }

    if (enRotate != ROTATE_NONE && SAMPLE_COMM_IsViVpssOnline())
    {
        s32Ret =  HI_MPI_VPSS_SetRotate(VpssGrp, VpssChn, enRotate);
        if (HI_SUCCESS != s32Ret)
        {
           SAMPLE_PRT("set VPSS rotate failed\n");
           SAMPLE_COMM_VPSS_StopGroup(VpssGrp);
           return s32Ret;
        }
    }

    if ((enRotate == ROTATE_90 || enRotate == ROTATE_270) && (!SAMPLE_COMM_IsViVpssOnline()))
    {
        HI_U32 u32Temp;
        u32Temp = pstVpssChnMode->u32Height;
        pstVpssChnMode->u32Height = pstVpssChnMode->u32Width;
        pstVpssChnMode->u32Width = u32Temp;
    }

    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, pstVpssChnAttr, pstVpssChnMode, pstVpssExtChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start VPSS CHN failed!\n");
        SAMPLE_COMM_VPSS_StopGroup(VpssGrp);
        return s32Ret;
    }

    return s32Ret;
}

static HI_S32 SAMPLE_VIO_StopVPSS(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
    SAMPLE_COMM_VPSS_StopGroup(VpssGrp);

    return HI_SUCCESS;
}

static HI_S32 SAMPLE_VIO_StartVENC(VENC_CHN VencChn, SAMPLE_VPSS_ATTR_S* pstVpssAttr, ROTATE_E enRotate)
{
#ifdef HI_FPGA
    return HI_SUCCESS;
#else
    SAMPLE_RC_E enRcMode= SAMPLE_RC_CBR;
    PAYLOAD_TYPE_E enPayLoad = PT_H264;
    PIC_SIZE_E enSize= g_enPicSize;
	HI_U32 u32Profile = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_GRP VpssGrp = pstVpssAttr->VpssGrp;
    VPSS_CHN VpssChn = pstVpssAttr->VpssChn;

    s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad,\
	                                   gs_enNorm, enSize, enRcMode,u32Profile, enRotate);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto exit;
    }

    if (SAMPLE_BIND_VI_VO == g_enVioBind)
    {
        VI_CHN ViChn = 0;
        s32Ret = SAMPLE_COMM_VENC_BindVi(VencChn, ViChn);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Bind Vi failed!\n");
            goto exit1;
        }
    }
    else
    {
        s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Bind Vpss failed!\n");
            goto exit1;
        }
    }

    s32Ret = SAMPLE_COMM_VENC_StartGetStream(1);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start venc stream failed!\n");
        goto exit2;
    }
    
    return HI_SUCCESS;
    
    exit2:
        if (SAMPLE_BIND_VI_VO == g_enVioBind)
        {
            VI_CHN ViChn = 0;
            SAMPLE_COMM_VENC_UnBindVi(VencChn, ViChn);
        }
        else
        {
            SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
        }
    exit1:
	    SAMPLE_COMM_VENC_Stop(VencChn);
    exit:
        return s32Ret;
#endif
}

static HI_S32 SAMPLE_VIO_StopVENC(VENC_CHN VencChn, SAMPLE_VPSS_ATTR_S* pstVpssAttr)
{
#ifdef HI_FPGA
        return HI_SUCCESS;
#else
    SAMPLE_COMM_VENC_StopGetStream();
    
    if (SAMPLE_BIND_VI_VO == g_enVioBind)
    {
        VI_CHN ViChn = 0;
        SAMPLE_COMM_VENC_UnBindVi(VencChn, ViChn);
    }
    else
    {
        VPSS_GRP VpssGrp = pstVpssAttr->VpssGrp;
        VPSS_CHN VpssChn = pstVpssAttr->VpssChn;
        SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn);
    }
    
	SAMPLE_COMM_VENC_Stop(VencChn);

    return HI_SUCCESS;
#endif
}

/******************************************************************************
 * Function:    SAMPLE_VIO_PreView
 * Description: online mode / offline mode. Embeded isp, phychn preview
******************************************************************************/
static HI_S32 SAMPLE_VIO_StartViVo(SAMPLE_VI_CONFIG_S* pstViConfig, SAMPLE_VPSS_ATTR_S* pstVpssAttr, SIZE_S *pstSize, SAMPLE_VO_CONFIG_S* pstVoConfig)
{
    VI_CHN   ViChn   = 0;
    VO_DEV   VoDev   = SAMPLE_VO_DEV_DSD0;
    VO_CHN   VoChn   = 0;
    VPSS_GRP VpssGrp = pstVpssAttr->VpssGrp;
    VPSS_CHN VpssChn = pstVpssAttr->VpssChn;
    HI_BOOL  bViVpssOnline = HI_FALSE;
    HI_S32   s32Ret = HI_SUCCESS;

    HI_ASSERT(NULL != pstViConfig);
    HI_ASSERT(NULL != pstSize);
    HI_ASSERT(NULL != pstVoConfig);
    /******************************************
     step 1: start vi dev & chn to capture
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_StartVi(pstViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        return s32Ret;
    }

    /******************************************
    step 2: start VO SD0
    ******************************************/
    s32Ret = SAMPLE_VIO_StartVO(pstVoConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO start VO failed with %#x!\n", s32Ret);
        goto exit1;
    }

    /******************************************
    step 3: start VPSS
    ******************************************/
    bViVpssOnline = SAMPLE_COMM_IsViVpssOnline();
    if (bViVpssOnline || 
        (SAMPLE_BIND_VPSS_VO == g_enVioBind) ||
        (SAMPLE_BIND_VI_VPSS_VO == g_enVioBind) )
    {
        s32Ret = SAMPLE_VIO_StartVPSS(VpssGrp, VpssChn, pstVpssAttr, pstViConfig->enRotate);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_VIO_StartVPSS failed with %#x!\n", s32Ret);
            goto exit2;
        }
    }

    /******************************************
    step 4: sys bind
    ******************************************/
    if (bViVpssOnline)       // vi-vpss online
    {
        SAMPLE_COMM_VO_BindVpss(VoDev, VoChn, VpssGrp, VpssChn);
    }
    else
    {
        if (SAMPLE_BIND_VI_VPSS_VO == g_enVioBind)
        {
            SAMPLE_COMM_VI_BindVpss(pstViConfig->enViMode);   // VI --> VPSS
            SAMPLE_COMM_VO_BindVpss(VoDev, VoChn, VpssGrp, VpssChn);   // VPSS --> VO
        }
        else
        {
            s32Ret = SAMPLE_COMM_VO_BindVi(VoDev, VoChn, ViChn);    // VI --> VO
            if (HI_SUCCESS != s32Ret)
	 	    {
	 	        SAMPLE_PRT("SAMPLE_COMM_VO_BindVi(vo:%d)-(vichn:%d) failed with %#x!\n", VoDev, VoChn, s32Ret);
	 	    }
        }
    }

    return s32Ret;
    
exit2:
    SAMPLE_VIO_StopVO();
exit1:
    SAMPLE_COMM_VI_StopVi(pstViConfig);
    return s32Ret;
}

HI_S32 SAMPLE_VIO_StopViVo(SAMPLE_VI_CONFIG_S* pstViConfig, SAMPLE_VPSS_ATTR_S* pstVpssAttr)
{
    VI_CHN   ViChn   = 0;
    VO_DEV   VoDev   = SAMPLE_VO_DEV_DSD0;
    VO_CHN   VoChn   = 0;
    VPSS_GRP VpssGrp = pstVpssAttr->VpssGrp;
    VPSS_CHN VpssChn = pstVpssAttr->VpssChn;
    HI_BOOL  bViVpssOnline;

    bViVpssOnline = SAMPLE_COMM_IsViVpssOnline();
   
    SAMPLE_COMM_VO_UnBindVpss(VoDev, VoChn, VpssGrp, VpssChn);
    SAMPLE_COMM_VI_UnBindVpss(pstViConfig->enViMode);
    SAMPLE_COMM_VO_UnBindVi(VoDev, ViChn);

    if (bViVpssOnline || 
        (SAMPLE_BIND_VPSS_VO == g_enVioBind) ||
        (SAMPLE_BIND_VI_VPSS_VO == g_enVioBind) )
    {
        SAMPLE_VIO_StopVPSS(VpssGrp, VpssChn);
    }

    SAMPLE_VIO_StopVO();
    SAMPLE_COMM_VI_StopVi(pstViConfig);

    return HI_SUCCESS;
}

/******************************************************************************
* function : vi/vpss: online/offline mode VI-VO. Embeded isp, phychn channel preview.
******************************************************************************/
HI_S32 SAMPLE_VIO_PreView(SAMPLE_VI_CONFIG_S* pstViConfig)
{
    HI_U32 u32ViChnCnt = 2;
    SIZE_S stSize;
    VB_CONF_S stVbConf;
    PIC_SIZE_E enPicSize = g_enPicSize;
    HI_U32 u32BlkSize;
    SAMPLE_VPSS_ATTR_S stVpssAttr;
    VENC_CHN VencChn = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret) 
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }
    
    /******************************************
     step  1: init global  variable
    ******************************************/
    gs_u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL == gs_enNorm) ? 25 : 30;
    g_enVioBind = SAMPLE_BIND_VI_VPSS_VO;

    /******************************************
     step  2: mpp system init
    ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    stVbConf.u32MaxPoolCnt = 128;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSize, 
                    SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);

    /* comm video buffer */
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt  = u32ViChnCnt * 8;

    if (pstViConfig)
    {
        // video buffer for the rotate video buffer
        ROTATE_E enRotate = pstViConfig->enRotate;
        SIZE_S stSizeTmp;
        if (ROTATE_90 == enRotate || ROTATE_270 == enRotate)
        {
            stSizeTmp.u32Width  = stSize.u32Height;
            stSizeTmp.u32Height = stSize.u32Width;
            u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSizeTmp, 
                            SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
            stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
            stVbConf.astCommPool[1].u32BlkCnt = 8;
        }
    }

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    /******************************************
     step  3: start VI VO
    ******************************************/
    memcpy(&stVpssAttr, &g_stVpssAttr, sizeof(SAMPLE_VPSS_ATTR_S));
    stVpssAttr.stVpssGrpAttr.u32MaxW = stSize.u32Width;
    stVpssAttr.stVpssGrpAttr.u32MaxH = stSize.u32Height;
    if (stSize.u32Width * stSize.u32Height < 1920 * 1080)
    {
        stVpssAttr.stVpssChnMode.u32Width = stSize.u32Width;
        stVpssAttr.stVpssChnMode.u32Height= stSize.u32Height;
    }
    s32Ret = SAMPLE_VIO_StartViVo(pstViConfig, &stVpssAttr, &stSize, &g_stVoConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO_StartViVo failed witfh %d\n", s32Ret);
        goto exit;   
    }

    /******************************************
     step  4: start VENC, bind vpss
    ******************************************/
    s32Ret = SAMPLE_VIO_StartVENC(VencChn, &stVpssAttr, pstViConfig->enRotate);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO_StartVENC failed witfh %d\n", s32Ret);
        goto exit;   
    }

    VI_PAUSE();
    
    SAMPLE_VIO_StopVENC(VencChn, &stVpssAttr);
    SAMPLE_VIO_StopViVo(pstViConfig, &stVpssAttr);

exit:
    SAMPLE_COMM_SYS_Exit();
    return s32Ret;
}

/******************************************************************************
* function : vi/vpss: online/offline ROTATE mode VI-VO. Embeded isp, phychn channel preview.
******************************************************************************/
HI_S32 SAMPLE_VIO_ROTATE_PreView(SAMPLE_VI_CONFIG_S* pstViConfig)
{
    HI_U32 u32ViChnCnt = 2;
    SIZE_S stSize;
    VB_CONF_S stVbConf;
    PIC_SIZE_E enPicSize = g_enPicSize;
    HI_U32 u32BlkSize;
    SAMPLE_VPSS_ATTR_S stVpssAttr;
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_VO_CONFIG_S stVoConfig;
    VENC_CHN VencChn = 0;

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret) 
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }
    
    /******************************************
     step  1: init global  variable
    ******************************************/
    gs_u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL == gs_enNorm) ? 25 : 30;

    /******************************************
     step  2: mpp system init
    ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    stVbConf.u32MaxPoolCnt = 128;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSize, 
                    SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);

    /* comm video buffer */
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt  = u32ViChnCnt * 8;

    if (pstViConfig)
    {
        // video buffer for the rotate video buffer
        ROTATE_E enRotate = pstViConfig->enRotate;
        SIZE_S stSizeTmp;
        if (ROTATE_90 == enRotate || ROTATE_270 == enRotate)
        {
            stSizeTmp.u32Width  = stSize.u32Height;
            stSizeTmp.u32Height = stSize.u32Width;
            u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSizeTmp, 
                            SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
            stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
            stVbConf.astCommPool[1].u32BlkCnt = 8;
        }
    }

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    /******************************************
     step  3: start VI VO
    ******************************************/
    memcpy(&stVpssAttr, &g_stVpssAttr, sizeof(SAMPLE_VPSS_ATTR_S));
    stVpssAttr.stVpssGrpAttr.u32MaxW = stSize.u32Width;
    stVpssAttr.stVpssGrpAttr.u32MaxH = stSize.u32Height;
    stVpssAttr.stVpssChnMode.u32Width = stSize.u32Width;
    stVpssAttr.stVpssChnMode.u32Height= stSize.u32Height;
    // online rotate not support VPSS chn Compress
    stVpssAttr.stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
    memcpy(&stVoConfig, &g_stVoConfig, sizeof(SAMPLE_VO_CONFIG_S));
    stVoConfig.u32DisBufLen = 0;
    s32Ret = SAMPLE_VIO_StartViVo(pstViConfig, &stVpssAttr, &stSize, &stVoConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO_StartViVo failed witfh %d\n", s32Ret);
        goto exit;   
    }

    /******************************************
     step  4: start VENC, bind vi
    ******************************************/
    if (SAMPLE_COMM_IsViVpssOnline())
    {
        g_enVioBind = SAMPLE_BIND_VI_VPSS_VO;
    }
    else
    {
        g_enVioBind = SAMPLE_BIND_VI_VO;
    }
    s32Ret = SAMPLE_VIO_StartVENC(VencChn, &stVpssAttr, pstViConfig->enRotate);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO_StartVENC failed witfh %d\n", s32Ret);
        goto exit;
    }

    VI_PAUSE();

    SAMPLE_VIO_StopVENC(VencChn, &stVpssAttr);
    SAMPLE_VIO_StopViVo(pstViConfig, &stVpssAttr);

exit:
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}

/******************************************************************************
* function : vi/vpss: online/offline WDR Switch mode VI(WDR). Embeded isp, phychn channel preview.
******************************************************************************/
HI_S32 SAMPLE_VIO_WDRSwitch_PreView(SAMPLE_VI_CONFIG_S* pstViConfig)
{
    HI_U32 u32ViChnCnt = 2;
    SIZE_S stSize;
    VB_CONF_S stVbConf;
    PIC_SIZE_E enPicSize = g_enPicSize;
    HI_U32 u32BlkSize;
    SAMPLE_VPSS_ATTR_S stVpssAttr;
    WDR_MODE_E  enOldWDRMode;
    HI_U32 u32Times = 2;
    VENC_CHN VencChn = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret) 
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }
    
    /******************************************
     step  1: init global  variable
    ******************************************/
    gs_u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL == gs_enNorm) ? 25 : 30;
    g_enVioBind = SAMPLE_BIND_VI_VPSS_VO;

    /******************************************
     step  2: mpp system init
    ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    stVbConf.u32MaxPoolCnt = 128;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSize, 
                    SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);

    /* comm video buffer */
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt  = u32ViChnCnt * 8;

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    /******************************************
     step  3: start VI VO
    ******************************************/
    memcpy(&stVpssAttr, &g_stVpssAttr, sizeof(SAMPLE_VPSS_ATTR_S));
    stVpssAttr.stVpssGrpAttr.u32MaxW = stSize.u32Width;
    stVpssAttr.stVpssGrpAttr.u32MaxH = stSize.u32Height;
    stVpssAttr.stVpssChnMode.u32Width = stSize.u32Width;
    stVpssAttr.stVpssChnMode.u32Height= stSize.u32Height;
    s32Ret = SAMPLE_VIO_StartViVo(pstViConfig, &stVpssAttr, &stSize, &g_stVoConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO_StartViVo failed witfh %d\n", s32Ret);
        goto exit;   
    }

    /******************************************
     step  3: start VENC, bind vpss
    ******************************************/
    s32Ret = SAMPLE_VIO_StartVENC(VencChn, &stVpssAttr, pstViConfig->enRotate);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO_StartVENC failed witfh %d\n", s32Ret);
        goto exit1;
    }

    /******************************************
     step  4: switch WDR Mode
    ******************************************/
    enOldWDRMode = pstViConfig->enWDRMode;
    while (u32Times > 0)
    {
		u32Times--;
        VI_PAUSE();

        /* switch to linear mode */
        pstViConfig->enWDRMode = WDR_MODE_NONE;
        s32Ret = SAMPLE_VIO_SwitchWDRMode(pstViConfig);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("switch WDR Mode failed\n");
            goto exit2;
        }
        VI_PAUSE();

        /* switch to 2to1 line WDR mode */
        pstViConfig->enWDRMode = enOldWDRMode;
        s32Ret = SAMPLE_VIO_SwitchWDRMode(pstViConfig);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("switch WDR Mode failed\n");
            goto exit2;
        }

    }

    VI_PAUSE();
    
exit2:
    SAMPLE_VIO_StopVENC(VencChn, &stVpssAttr);

exit1:
    SAMPLE_VIO_StopViVo(pstViConfig, &stVpssAttr);

exit:
    SAMPLE_COMM_SYS_Exit();
    return s32Ret;
}

/******************************************************************************
* function : vi/vpss: online mode VI(FPN): DC(1080P); VO: SD0(CVBS). Embeded isp, phychn channel preview.
******************************************************************************/
HI_S32 SAMPLE_VIO_FRAME_FPN_PreView(SAMPLE_VI_CONFIG_S* pstViConfig)
{
    HI_U32 u32ViChnCnt = 2;
    VI_CHN ViChn = 0;
    SIZE_S stSize;
    VB_CONF_S stVbConf;
    PIC_SIZE_E enPicSize = g_enPicSize;
    HI_U32 u32BlkSize;
    SAMPLE_VPSS_ATTR_S stVpssAttr;
    VI_CHN_ATTR_S stTempChnAttr;
    VENC_CHN VencChn = 0;

    const char* fpn_file = "sample";   /* fpn file name */
    char fileName[FILE_NAME_LEN];
    ISP_FPN_TYPE_E  enFpnType      = ISP_FPN_TYPE_FRAME; /* line/frame */
    PIXEL_FORMAT_E  enPixelFormat  = PIXEL_FORMAT_RGB_BAYER;
    COMPRESS_MODE_E enCompressMode = COMPRESS_MODE_NONE;
    HI_U32 u32FrmNum    = 16;
    HI_U32 u32Threshold = 4095;
    ISP_OP_TYPE_E  enOpType = OP_TYPE_AUTO; /* auto/manual */
    HI_U32 u32Strength = 0X100;             /* strength */
    
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret) 
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }

    /******************************************
      step  1: init global  variable
     ******************************************/
    gs_u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL == gs_enNorm) ? 25 : 30;

    /******************************************
      step  2: mpp system init
     ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    stVbConf.u32MaxPoolCnt = 128;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSize, 
            PIXEL_FORMAT_YUV_SEMIPLANAR_422, SAMPLE_SYS_ALIGN_WIDTH);

    /* comm video buffer */
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt  = u32ViChnCnt * 8;

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    /******************************************
      step  3: start VI VO
     ******************************************/
    memcpy(&stVpssAttr, &g_stVpssAttr, sizeof(SAMPLE_VPSS_ATTR_S));
    stVpssAttr.stVpssGrpAttr.u32MaxW = stSize.u32Width;
    stVpssAttr.stVpssGrpAttr.u32MaxH = stSize.u32Height;
    stVpssAttr.stVpssChnMode.u32Width = stSize.u32Width;
    stVpssAttr.stVpssChnMode.u32Height= stSize.u32Height;
    s32Ret = SAMPLE_VIO_StartViVo(pstViConfig, &stVpssAttr, &stSize, &g_stVoConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO_StartViVo failed with %d\n", s32Ret);
        goto exit;   
    }

    /******************************************
      step  4: fpn calibrate, save dark frame file
     ******************************************/
    s32Ret = SAMPLE_COMM_VI_FPN_CALIBRATE_CONFIG(fpn_file, enFpnType,
                    enPixelFormat, enCompressMode, u32FrmNum, u32Threshold);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("FPN calibrate failed %#x!\n", s32Ret);
        goto exit1;
    }

    /******************************************
      step  5: after fpn calibrate, start VENC, bind vpss
     ******************************************/
    s32Ret = SAMPLE_VIO_StartVENC(VencChn, &stVpssAttr, pstViConfig->enRotate);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO_StartVENC failed witfh %d\n", s32Ret);
        goto exit1;   
    }

    /******************************************
      step  6: fpn correction, read dark frame file
     ******************************************/
    s32Ret =  HI_MPI_VI_GetChnAttr(ViChn, &stTempChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("get vi chn attr failed!");
        goto exit2;
    }
    snprintf(fileName, FILE_NAME_LEN, "./%s_%d_%d_%dbit.raw", fpn_file, stTempChnAttr.stDestSize.u32Width,
            stTempChnAttr.stDestSize.u32Height , 16);
    s32Ret = SAMPLE_COMM_VI_CORRECTION_CONFIG(fileName, enFpnType, enOpType,
            u32Strength, enPixelFormat);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("FPN correction failed %#x!\n", s32Ret);
        goto exit2;
    }

exit2:
    SAMPLE_VIO_StopVENC(VencChn, &stVpssAttr);

exit1:
    SAMPLE_VIO_StopViVo(pstViConfig, &stVpssAttr);

exit:
    SAMPLE_COMM_SYS_Exit();
    return s32Ret;
}

/******************************************************************************
* function : vi/vpss: offline/online LDC mode VI-VO. Embeded isp, phychn channel preview.
******************************************************************************/
HI_S32 SAMPLE_VIO_LDC_PreView(SAMPLE_VI_CONFIG_S* pstViConfig)
{
    HI_U32 u32ViChnCnt = 2;
    VI_CHN ViChn = 0;
    VPSS_GRP VpssGrp = 0;
    VPSS_CHN VpssChn = 1;
    SIZE_S stSize;
    HI_U32 u32BlkSize;
    VB_CONF_S stVbConf;
    PIC_SIZE_E enPicSize = g_enPicSize;
    VI_LDC_ATTR_S stLDCAttr;
    VPSS_LDC_ATTR_S stOnlineLDCAttr;
    HI_BOOL bViVpssOnline = HI_FALSE;
    SAMPLE_VPSS_ATTR_S stVpssAttr;
    VENC_CHN VencChn = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret) 
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }

    /******************************************
      step  1: init global  variable
     ******************************************/
    gs_u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL == gs_enNorm) ? 25 : 30;

    /******************************************
      step  2: mpp system init
     ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    stVbConf.u32MaxPoolCnt = 128;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSize, 
            SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);

    /* comm video buffer */
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt  = u32ViChnCnt * 8;

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    /******************************************
      step  3: start VI VO
     ******************************************/
    memcpy(&stVpssAttr, &g_stVpssAttr, sizeof(SAMPLE_VPSS_ATTR_S));
    stVpssAttr.stVpssGrpAttr.u32MaxW = stSize.u32Width;
    stVpssAttr.stVpssGrpAttr.u32MaxH = stSize.u32Height;
    stVpssAttr.stVpssChnMode.u32Width = stSize.u32Width;
    stVpssAttr.stVpssChnMode.u32Height= stSize.u32Height;
    // online LDC, VPSS chn not support compress mode
    // online LDC, as the fisheye's low performance VPSS need to do frame rate control
    stVpssAttr.stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
    stVpssAttr.stVpssChnAttr.s32SrcFrameRate = 30;
    stVpssAttr.stVpssChnAttr.s32DstFrameRate = 30;
    s32Ret = SAMPLE_VIO_StartViVo(pstViConfig, &stVpssAttr, &stSize, &g_stVoConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO_StartViVo failed witfh %d\n", s32Ret);
        goto exit;   
    }

    /******************************************
      step  4: start VNEC
     ******************************************/
    s32Ret = SAMPLE_VIO_StartVENC(VencChn, &stVpssAttr, pstViConfig->enRotate);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO_StartVENC failed witfh %d\n", s32Ret);
        goto exit1;   
    }
    
    /******************************************
      step  5: set LDC Attr
     ******************************************/
    bViVpssOnline = SAMPLE_COMM_IsViVpssOnline();
    
    if (bViVpssOnline)
    {
        stOnlineLDCAttr.bEnable = HI_TRUE;
        stOnlineLDCAttr.stAttr.enViewType = LDC_VIEW_TYPE_ALL;
        stOnlineLDCAttr.stAttr.s32CenterXOffset = 0;
        stOnlineLDCAttr.stAttr.s32CenterYOffset = 0;
        stOnlineLDCAttr.stAttr.s32Ratio = 100;
        s32Ret =  HI_MPI_VPSS_SetLDCAttr(VpssGrp, VpssChn, &stOnlineLDCAttr);
    }
    else
    {
        stLDCAttr.bEnable = HI_TRUE;
        stLDCAttr.stAttr.enViewType = LDC_VIEW_TYPE_ALL;
        stLDCAttr.stAttr.s32CenterXOffset = 0;
        stLDCAttr.stAttr.s32CenterYOffset = 0;
        stLDCAttr.stAttr.s32Ratio = -180;
        stLDCAttr.stAttr.s32MinRatio = 0;
        s32Ret =  HI_MPI_VI_SetLDCAttr(ViChn, &stLDCAttr);
    }
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set LDC failed with %#x!\n", s32Ret);
        goto exit2;   
    }

    printf("\nplease hit any key, disable LDC\n\n");
    VI_PAUSE();

    if (bViVpssOnline)
    {
        stOnlineLDCAttr.bEnable = HI_FALSE;
        s32Ret =  HI_MPI_VPSS_SetLDCAttr(VpssGrp, VpssChn,  &stOnlineLDCAttr);
    }
    else
    {
        stLDCAttr.bEnable = HI_FALSE;
        s32Ret =  HI_MPI_VI_SetLDCAttr(ViChn, &stLDCAttr);
    }
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set LDC failed with %#x!\n", s32Ret);
        goto exit2;   
    }

    printf("\nplease hit any key, enable LDC\n");
    VI_PAUSE();

    if (bViVpssOnline)
    {
        stOnlineLDCAttr.bEnable = HI_TRUE;
        s32Ret =  HI_MPI_VPSS_SetLDCAttr(VpssGrp, VpssChn,  &stOnlineLDCAttr);
    }
    else
    {
        stLDCAttr.bEnable = HI_TRUE;
        s32Ret =  HI_MPI_VI_SetLDCAttr(ViChn, &stLDCAttr);
    }
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set LDC failed with %#x!\n", s32Ret);
        goto exit2;   
    }

    VI_PAUSE();
    
exit2:
    SAMPLE_VIO_StopVENC(VencChn, &stVpssAttr);
    
exit1:
    SAMPLE_VIO_StopViVo(pstViConfig, &stVpssAttr);

exit:
    SAMPLE_COMM_SYS_Exit();
    return s32Ret;

}

/******************************************************************************
* function : vi/vpss: offline mode VI: DC(1080P),DIS; VPSS:3DNR; VO: SD0(CVBS). Embeded isp, phychn channel preview.
******************************************************************************/

HI_S32 SAMPLE_VI_DIS_VPSS_VO_1080P_PreView(SAMPLE_VI_CONFIG_S* pstViConfig)
{
    HI_U32 u32ViChnCnt = 2;
    VB_CONF_S stVbConf;
    VO_DEV VoDev = SAMPLE_VO_DEV_DSD0;
    VO_CHN VoChn = 0;
    VI_CHN ViChn = 0;
    VO_PUB_ATTR_S stVoPubAttr;
    SAMPLE_VO_MODE_E enVoMode = VO_MODE_2MUX;
    PIC_SIZE_E enPicSize = g_enPicSize;
    VO_LAYER VoLayer = 0;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    VPSS_GRP VpssGrp = 0;
    VPSS_CHN VpssChn = 0;
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr;
    VPSS_CHN_MODE_S stVpssChnMode;
    VI_CHN_ATTR_S stChnAttr;
    VPSS_EXT_CHN_ATTR_S stVpssExtChnAttr;
    VENC_CHN VencChn = 0;
    SAMPLE_VPSS_ATTR_S stVpssAttr;
    ISP_DEV IspDev = 0;
    ISP_PUB_ATTR_S stPubAttr;
    ISP_INNER_STATE_INFO_S stInnerStateInfo;
    HI_S32 s32Tries;

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    SIZE_S stSize;
    ROTATE_E enRotate = ROTATE_NONE;

    /******************************************
     step  1: init global  variable
    ******************************************/
    gs_u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL == gs_enNorm) ? 25 : 30;
    memset(&stVbConf, 0, sizeof(VB_CONF_S));

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, enPicSize,
                 SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.u32MaxPoolCnt = 128;

    /*ddr0 video buffer*/
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 15;

    /******************************************
     step 2: start vpss and vi bind vpss (subchn needn't bind vpss in this mode)
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_1080P_0;
    }

    if (pstViConfig)
    {
        enRotate = pstViConfig->enRotate;
        if (ROTATE_90 == enRotate || ROTATE_270 == enRotate)
        {
            u32BlkSize = (CEILING_2_POWER(stSize.u32Width, SAMPLE_SYS_ALIGN_WIDTH) * \
                          CEILING_2_POWER(stSize.u32Height, SAMPLE_SYS_ALIGN_WIDTH) * \
                          ((PIXEL_FORMAT_YUV_SEMIPLANAR_422 == SAMPLE_PIXEL_FORMAT) ? 2 : 1.5));
            stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
            stVbConf.astCommPool[1].u32BlkCnt = 8;
        }
    }

    /******************************************
     step 3: mpp system init.
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_1080P_0;
    }

    /******************************************
     step 4: start vi dev & chn to capture
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_StartVi(pstViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_1080P_0;
    }

    /* crop vi chn  */
    s32Ret =  HI_MPI_VI_DisableChn(ViChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("disable  vi chn failed!\n");
        goto END_1080P_0;
    }
    stChnAttr.stCapRect.s32X = 0;
    stChnAttr.stCapRect.s32Y = 0;
    stChnAttr.stCapRect.u32Width = 1280 + 100;
    stChnAttr.stCapRect.u32Height = 720 + 100;
    stChnAttr.bFlip = HI_FALSE;
    stChnAttr.bMirror = HI_FALSE;
    stChnAttr.enCapSel = VI_CAPSEL_BOTH;
    stChnAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stChnAttr.enCompressMode = COMPRESS_MODE_NONE;
    stChnAttr.s32SrcFrameRate = 30;
    stChnAttr.s32DstFrameRate = 30;

    stChnAttr.stDestSize.u32Width = stChnAttr.stCapRect.u32Width;
    stChnAttr.stDestSize.u32Height = stChnAttr.stCapRect.u32Height;
    s32Ret =  HI_MPI_VI_SetChnAttr(ViChn, &stChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set vi chn failed!\n");
        goto END_1080P_0;
    }

    HI_MPI_ISP_GetPubAttr(IspDev, &stPubAttr);
    stPubAttr.stWndRect.u32Height = stChnAttr.stCapRect.u32Height;
    stPubAttr.stWndRect.u32Width= stChnAttr.stCapRect.u32Width;
    s32Ret = HI_MPI_ISP_SetPubAttr(IspDev, &stPubAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set isp PubAttr failed!\n");
        goto END_1080P_0;
    }
    s32Tries = 400;
    while (s32Tries--)
    {
        HI_MPI_ISP_QueryInnerStateInfo(IspDev, &stInnerStateInfo);
        if (HI_TRUE == stInnerStateInfo.bResSwitchFinish)
        {
            printf("res mode switch finish, cost time: %d ms!\n", (400 - s32Tries) * 20);
            break;
        }
        usleep(20000);
    }

    s32Ret =  HI_MPI_VI_EnableChn(ViChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("enable vi chn failed!\n");
        goto END_1080P_0;
    }

    stVpssGrpAttr.bDciEn = HI_FALSE;
    stVpssGrpAttr.bHistEn = HI_FALSE;
    stVpssGrpAttr.bIeEn = HI_FALSE;
    stVpssGrpAttr.bNrEn = HI_TRUE;
    stVpssGrpAttr.bSharpenEn = HI_TRUE;
    stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
    stVpssGrpAttr.enPixFmt = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stVpssGrpAttr.u32MaxH = 720 + 100;
    stVpssGrpAttr.u32MaxW = 1280 + 100;


    /******************************************
     step 5: start vpss group
    ******************************************/
    s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp, &stVpssGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start VPSS GROUP failed!\n");
        goto END_1080P_1;
    }

    stVpssChnAttr.bBorderEn = 0;
    stVpssChnAttr.bFlip = 0;
    stVpssChnAttr.bMirror = 0;
    stVpssChnAttr.bSpEn = 0 ;
    stVpssChnAttr.s32DstFrameRate = -1;
    stVpssChnAttr.s32SrcFrameRate = -1;
    stVpssChnMode.bDouble = HI_FALSE;
    stVpssChnMode.enChnMode = VPSS_CHN_MODE_USER;
    stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
    stVpssChnMode.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stVpssChnMode.u32Width  = 1280;
    stVpssChnMode.u32Height = 720;


    VPSS_CROP_INFO_S stVpssCropInfo;

    stVpssCropInfo.bEnable = HI_TRUE;
    stVpssCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
    stVpssCropInfo.stCropRect.s32X = 50;
    stVpssCropInfo.stCropRect.s32Y = 50;
    stVpssCropInfo.stCropRect.u32Width  = 1280;
    stVpssCropInfo.stCropRect.u32Height = 720;
    s32Ret = HI_MPI_VPSS_SetGrpCrop(VpssGrp, &stVpssCropInfo);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set VPSS group crop failed!\n");
        goto END_1080P_1;
    }

    /******************************************
    step 6: start vpss chn
    ******************************************/
    s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, &stVpssExtChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start VPSS CHN failed!\n");
        goto END_1080P_1;
    }

    /******************************************
      step  7: start VNEC, bind vpss
     ******************************************/
    g_enPicSize = PIC_HD720;
    stVpssAttr.VpssGrp = VpssGrp;
    stVpssAttr.VpssChn = VpssChn;
    s32Ret = SAMPLE_VIO_StartVENC(VencChn, &stVpssAttr, pstViConfig->enRotate);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO_StartVENC failed witfh %d\n", s32Ret);
        goto END_1080P_2;   
    }

    /******************************************
    step 8: start VO SD0 (bind * vi )
    ******************************************/
    stVoPubAttr.enIntfType = g_enVoIntfType;
    if (VO_INTF_BT1120 == g_enVoIntfType)
    {
        stVoPubAttr.enIntfSync = VO_OUTPUT_1080P30;
        gs_u32ViFrmRate = 50;
    }
    else
    {
        stVoPubAttr.enIntfSync = VO_OUTPUT_PAL;
    }
    stVoPubAttr.u32BgColor = 0x000000ff;

    /* In HD, this item should be set to HI_FALSE */
    s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartDev failed!\n");
        goto END_1080P_3;
    }

    stLayerAttr.bClusterMode = HI_FALSE;
    stLayerAttr.bDoubleFrame = HI_FALSE;
    stLayerAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;

    stLayerAttr.stDispRect.s32X = 0;
    stLayerAttr.stDispRect.s32Y = 0;

    s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync,
                                  &stLayerAttr.stDispRect.u32Width, &stLayerAttr.stDispRect.u32Height,
                                  &stLayerAttr.u32DispFrmRt);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_GetWH failed!\n");
        goto END_1080P_3;
    }
    stLayerAttr.stImageSize.u32Width  = stLayerAttr.stDispRect.u32Width;
    stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;

    s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr, HI_FALSE);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartLayer failed!\n");
        goto END_1080P_4;
    }

    s32Ret = SAMPLE_COMM_VO_StartChn(VoDev, enVoMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartChn failed!\n");
        goto END_1080P_5;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(pstViConfig->enViMode);
	if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_BindVpss failed with %#x!\n", s32Ret);
        goto END_1080P_6;
    }

    s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev, VoChn + 1, VpssGrp, VpssChn);
	if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_BindVpss(VoChn:%d)-(VpssChn:%d) failed with %#x!\n", VoChn + 1, VpssChn, s32Ret);
        goto END_1080P_6;
    }
	
    s32Ret = SAMPLE_COMM_VO_BindVi(VoDev, VoChn, ViChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_BindVi(VoChn:%d)-(ViChn:%d) failed with %#x!\n", VoChn, ViChn, s32Ret);
        goto END_1080P_6;
    }


    ISP_DIS_ATTR_S stDisAttr;
    stDisAttr.bEnable = HI_TRUE;

    s32Ret =  HI_MPI_ISP_SetDISAttr(IspDev, &stDisAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set DIS atrr failed!\n");
        goto END_1080P_6;
    }

    printf("\n\npress key to disable dis ------\n");
    VI_PAUSE();

    s32Ret =  HI_MPI_ISP_GetDISAttr(IspDev, &stDisAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("get DIS atrr failed!\n");
        goto END_1080P_6;
    }
    stDisAttr.bEnable = HI_FALSE;
    s32Ret =  HI_MPI_ISP_SetDISAttr(IspDev, &stDisAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set DIS atrr failed!\n");
        goto END_1080P_6;
    }
    printf("\n\npress key to enable dis ------\n");
    VI_PAUSE();

    s32Ret =  HI_MPI_ISP_GetDISAttr(IspDev, &stDisAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("get DIS atrr failed!\n");
        goto END_1080P_6;
    }

    stDisAttr.bEnable = HI_TRUE;
    s32Ret =  HI_MPI_ISP_SetDISAttr(IspDev, &stDisAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set DIS atrr failed!\n");
        goto END_1080P_6;
    }
    VI_PAUSE();

    stDisAttr.bEnable = HI_FALSE;
    s32Ret =  HI_MPI_ISP_SetDISAttr(IspDev, &stDisAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set DIS atrr failed!\n");
        goto END_1080P_6;
    }

    /******************************************
     step 8: exit process
    ******************************************/
END_1080P_6:
    SAMPLE_COMM_VO_UnBindVi(VoDev, VoChn);
    SAMPLE_COMM_VO_StopChn(VoDev, enVoMode);
END_1080P_5:
    SAMPLE_COMM_VO_StopLayer(VoLayer);
END_1080P_4:
    SAMPLE_COMM_VO_StopDev(VoDev);
END_1080P_3:
    SAMPLE_VIO_StopVENC(VencChn, &stVpssAttr);
END_1080P_2:
    SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
    SAMPLE_COMM_VPSS_StopGroup(VpssGrp);
END_1080P_1:
    SAMPLE_COMM_VI_StopVi(pstViConfig);
END_1080P_0:
    SAMPLE_COMM_SYS_Exit();
    
    g_enPicSize = PIC_HD1080;
    return s32Ret;
}



/******************************************************************************
* function :  VI: BT1120(1080I); VO: SD0(CVBS). bypass isp, phychn channel preview.
******************************************************************************/
HI_S32 SAMPLE_VIO_BT1120_1080P_PreView(SAMPLE_VI_CONFIG_S* pstViConfig)
{
    HI_U32 u32ViChnCnt = 1;
    VB_CONF_S stVbConf;
    VO_DEV VoDev = SAMPLE_VO_DEV_DSD0;
    VO_CHN VoChn = 0;
    VI_CHN ViChn = 0;
    VO_PUB_ATTR_S stVoPubAttr;
    SAMPLE_VO_MODE_E enVoMode = VO_MODE_1MUX;
    PIC_SIZE_E enPicSize = g_enPicSize;

    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    SIZE_S stSize;
    VO_LAYER VoLayer = 0;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    VENC_CHN VencChn = 0;
    SAMPLE_VPSS_ATTR_S stVpssAttr;

	memset(&stVpssAttr, 0, sizeof(SAMPLE_VPSS_ATTR_S));


    /******************************************
     step  1: init global  variable
    ******************************************/
    gs_u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL == gs_enNorm) ? 25 : 30;
    memset(&stVbConf, 0, sizeof(VB_CONF_S));

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, PIC_HD1080,
                 SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.u32MaxPoolCnt = 128;

    /*ddr0 video buffer*/
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 8;

    /******************************************
     step 2: mpp system init.
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_1080P_0;
    }

    /******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_StartVi(pstViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_1080P_0;
    }

    /******************************************
     step 4: start vpss and vi bind vpss (subchn needn't bind vpss in this mode)
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_1080P_1;
    }

    /******************************************
    step 5: start VENC, bind vi
    ******************************************/
    g_enVioBind = SAMPLE_BIND_VI_VO;
    s32Ret = SAMPLE_VIO_StartVENC(VencChn, &stVpssAttr, pstViConfig->enRotate);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO_StartVENC failed witfh %d\n", s32Ret);
        goto END_1080P_1;   
    }

    /******************************************
    step 6: start VO SD0 (bind * vi )
    ******************************************/
    stVoPubAttr.enIntfType = g_enVoIntfType;
    if (VO_INTF_BT1120 == g_enVoIntfType)
    {
        stVoPubAttr.enIntfSync = VO_OUTPUT_1080P30;
    }
    else
    {
        stVoPubAttr.enIntfSync = VO_OUTPUT_PAL;
    }
    stVoPubAttr.u32BgColor = 0x000000ff;
    /* In HD, this item should be set to HI_FALSE */
    s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartDevLayer failed!\n");
        goto END_1080P_2;
    }

    stLayerAttr.bClusterMode = HI_FALSE;
    stLayerAttr.bDoubleFrame = HI_FALSE;
    stLayerAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;

    stLayerAttr.stDispRect.s32X = 0;
    stLayerAttr.stDispRect.s32Y = 0;
    s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync,
                                  &stLayerAttr.stDispRect.u32Width, &stLayerAttr.stDispRect.u32Height,
                                  &stLayerAttr.u32DispFrmRt);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_GetWH failed!\n");
        goto END_1080P_2;
    }
    stLayerAttr.stImageSize.u32Width  = stLayerAttr.stDispRect.u32Width;
    stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;

    s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr, HI_FALSE);
	if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartLayer failed!\n");
        goto END_1080P_3;
    }

    s32Ret = SAMPLE_COMM_VO_StartChn(VoDev, VO_MODE_1MUX);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartChn failed!\n");
        goto END_1080P_4;
    }

    s32Ret = SAMPLE_COMM_VO_BindVi(VoDev, VoChn, ViChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_BindVi(vo:%d)-(vichn:%d) failed with %#x!\n", VoDev, VoChn, s32Ret);
        goto END_1080P_4;
    }

    VI_PAUSE();
    /******************************************
     step 7: exit process
    ******************************************/
END_1080P_4:
    SAMPLE_COMM_VO_UnBindVi(VoDev, VoChn);
    SAMPLE_COMM_VO_StopChn(VoDev, enVoMode);
    SAMPLE_COMM_VO_StopLayer(VoLayer);
END_1080P_3:
    SAMPLE_COMM_VO_StopDev(VoDev);
END_1080P_2:
    SAMPLE_VIO_StopVENC(VencChn, &stVpssAttr);
END_1080P_1:
    SAMPLE_COMM_VI_StopVi(pstViConfig);
END_1080P_0:
    SAMPLE_COMM_SYS_Exit();
    g_enVioBind = SAMPLE_BIND_VI_VPSS_VO;
    return s32Ret;
}

/******************************************************************************************************
* function :  VI: DC(720P); VO: SD0(CVBS). Embeded isp,  linear/wdr change, phychn channel preview.
******************************************************************************************************/
HI_S32 SAMPLE_VIO_ResolutionSwitch_Preview(SAMPLE_VI_CONFIG_S* pstViConfig)
{
    HI_U32 u32ViChnCnt = 2;
    SIZE_S stSize;
    VB_CONF_S stVbConf;
    PIC_SIZE_E a_enPicSize[3];
    HI_U32 u32BlkSize;
    HI_S32 s32Cnt = (sizeof(a_enPicSize) / sizeof(a_enPicSize[0])) - 1;
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_VPSS_ATTR_S stVpssAttr;
    SAMPLE_VO_CONFIG_S stVoConfig;
#ifndef HI_FPGA
    VENC_CHN VencChn = 0;
#endif

    memset(&a_enPicSize, PIC_BUTT, sizeof(a_enPicSize));
#if 0
    if ( PANASONIC_MN34220_SUBLVDS_1080P_30FPS == pstViConfig->enViMode
         || PANASONIC_MN34220_SUBLVDS_720P_120FPS == pstViConfig->enViMode
         || PANASONIC_MN34220_MIPI_1080P_30FPS == pstViConfig->enViMode
         || PANASONIC_MN34220_MIPI_720P_120FPS == pstViConfig->enViMode
         || SONY_IMX117_LVDS_1080P_120FPS == pstViConfig->enViMode 
         || SONY_IMX117_LVDS_720P_240FPS == pstViConfig->enViMode )
#else
    if ( PANASONIC_MN34220_SUBLVDS_1080P_30FPS == pstViConfig->enViMode
     || PANASONIC_MN34220_SUBLVDS_720P_120FPS == pstViConfig->enViMode
     || SONY_IMX117_LVDS_1080P_120FPS == pstViConfig->enViMode 
     || SONY_IMX117_LVDS_720P_240FPS == pstViConfig->enViMode)
#endif
    {
        a_enPicSize[0] = PIC_HD1080;
        a_enPicSize[1] = PIC_HD720;
    }
    else
    {
        SAMPLE_PRT("Not support this sensor type!\n");
        return HI_FAILURE;
    }

    // use the big size to init mpp
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, a_enPicSize[0], &stSize);
    if (HI_SUCCESS != s32Ret) 
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }

    /******************************************
      step  1: init global  variable
     ******************************************/
    gs_u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL == gs_enNorm) ? 25 : 30;
    g_enVioBind = SAMPLE_BIND_VI_VPSS_VO;

    /******************************************
      step  2: mpp system init
     ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    stVbConf.u32MaxPoolCnt = 128;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSize, 
            SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);

    /* comm video buffer */
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt  = u32ViChnCnt * 8;

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        SAMPLE_COMM_SYS_Exit();
        return s32Ret;
    }

    /******************************************
      step  3: start VI VO
     ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, g_enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret) 
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }
    memcpy(&stVpssAttr, &g_stVpssAttr, sizeof(SAMPLE_VPSS_ATTR_S));
    stVpssAttr.stVpssGrpAttr.u32MaxW = stSize.u32Width;
    stVpssAttr.stVpssGrpAttr.u32MaxH = stSize.u32Height;
    stVpssAttr.stVpssChnMode.u32Width = stSize.u32Width;
    stVpssAttr.stVpssChnMode.u32Height= stSize.u32Height;
    stVpssAttr.stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
    memcpy(&stVoConfig, &g_stVoConfig, sizeof(SAMPLE_VO_CONFIG_S));
    stVoConfig.u32DisBufLen = 0;
    s32Ret = SAMPLE_VIO_StartViVo(pstViConfig, &stVpssAttr, &stSize, &stVoConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO_StartViVo failed witfh %d\n", s32Ret);
        goto exit;   
    }

    /******************************************
      step  3: start VENC, bind vpss
     ******************************************/
#ifndef HI_FPGA
    s32Ret = SAMPLE_VIO_StartVENC(VencChn, &stVpssAttr, pstViConfig->enRotate);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_VIO_StartVENC failed witfh %d\n", s32Ret);
        goto exit1;   
    }
#endif

    VI_PAUSE();

    /******************************************
      step  4: resolution switch
     ******************************************/
    while(s32Cnt > 0)
    {
        s32Ret = SAMPLE_VIO_SwitchResolution(pstViConfig, &stVpssAttr, a_enPicSize);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SwitchResolution failed with %d\n", s32Ret);
            goto exit2;
        }
        
#ifndef HI_FPGA
        {
            VENC_CHN_ATTR_S  stVencChnAttr;
            VI_CHN ViChn = 0;
            VI_CHN_ATTR_S stViChnAttr;
            
            s32Ret = HI_MPI_VI_GetChnAttr(ViChn, &stViChnAttr);
			if (HI_SUCCESS != s32Ret)
	        {
	            SAMPLE_PRT("Vi GetChnAttr failed with %d\n", s32Ret);
	            goto exit2;
	        }
            s32Ret = HI_MPI_VENC_StopRecvPic(VencChn);
			if (HI_SUCCESS != s32Ret)
	        {
	            SAMPLE_PRT("Venc StopRecvPic failed with %d\n", s32Ret);
	            goto exit2;
	        }
            s32Ret = HI_MPI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
			if (HI_SUCCESS != s32Ret)
	        {
	            SAMPLE_PRT("Venc GetChnAttr failed with %d\n", s32Ret);
	            goto exit2;
	        }
            stVencChnAttr.stVeAttr.stAttrH264e.u32PicHeight = stViChnAttr.stCapRect.u32Height;
            stVencChnAttr.stVeAttr.stAttrH264e.u32PicWidth = stViChnAttr.stCapRect.u32Width;
            HI_MPI_VENC_SetChnAttr(VencChn, &stVencChnAttr);
            s32Ret = HI_MPI_VENC_StartRecvPic(VencChn);
			if (HI_SUCCESS != s32Ret)
	        {
	            SAMPLE_PRT("Venc StartRecvPic failed with %d\n", s32Ret);
	            goto exit2;
	        }
        }
#endif

        VI_PAUSE();
        s32Cnt--;
    }
    
exit2:
#ifndef HI_FPGA
    SAMPLE_VIO_StopVENC(VencChn, &stVpssAttr);
exit1:
#endif
    SAMPLE_VIO_StopViVo(pstViConfig, &stVpssAttr);

exit:
    SAMPLE_COMM_SYS_Exit();
    return s32Ret;

}

/******************************************************************************
* function    : main()
* Description : video preview sample
******************************************************************************/
#ifdef __HuaweiLite__
int app_main(int argc, char *argv[])
#else
int main(int argc, char* argv[])
#endif
{
    HI_S32 s32Ret = HI_FAILURE;
#ifdef HI_FPGA
#ifdef __HuaweiLite__
    HI_S32 VoInf, ch;
#endif
#endif

#ifndef __HuaweiLite__
    if (argc < 2)
    {
        SAMPLE_VIO_Usage(argv[0]);
        return HI_FAILURE;
    }
    
    if (1 != strlen(argv[1]))
    {
        SAMPLE_VIO_Usage(argv[0]);
        return HI_FAILURE;
    }

    signal(SIGINT, SAMPLE_VIO_HandleSig);
    signal(SIGTERM, SAMPLE_VIO_HandleSig);
#endif

#ifdef HI_FPGA
#ifdef __HuaweiLite__
    SAMPLE_VIO_VoIntf_Usage();
    VoInf = getchar();
    while((ch = getchar()) != '\n' && (ch != EOF)); 
    if ('1' == VoInf)
#else
    if ((argc > 2) && *argv[2] == '1')  /* '1': VO_INTF_CVBS, else: BT1120 */
#endif
    {
        g_enVoIntfType = VO_INTF_BT1120;
    }

    if ((argc > 2) && ((*argv[2] != '0') && (*argv[2] != '1')))
    {
        SAMPLE_VIO_Usage("sample_vio");
        return HI_FAILURE;
    }
#endif

    g_stViChnConfig.enViMode = SENSOR_TYPE;
    SAMPLE_COMM_VI_GetSizeBySensor(&g_enPicSize);

    /* choose the case */

    switch(*argv[1])
    {
        /* VI/VPSS - VO. Embeded isp, phychn channel preview. */
    case '0':
        s32Ret = SAMPLE_VIO_PreView(&g_stViChnConfig);
        break;
    case '1':
        if ((SENSOR_TYPE == PANASONIC_MN34220_SUBLVDS_720P_120FPS)
                || (SENSOR_TYPE == PANASONIC_MN34220_MIPI_720P_120FPS)
                || (SENSOR_TYPE == SONY_IMX323_CMOS_1080P_30FPS))
        {
            SAMPLE_PRT("not support!\n");
            break;
        }
        if ((SENSOR_TYPE == PANASONIC_MN34220_SUBLVDS_1080P_30FPS) 
                || (SENSOR_TYPE == PANASONIC_MN34220_MIPI_1080P_30FPS)
                || (SENSOR_TYPE == SONY_IMX290_MIPI_1080P_30FPS)
                || (SENSOR_TYPE == OMNIVISION_OV2718_MIPI_1080P_30FPS)
                || (SENSOR_TYPE == OMNIVISION_OV2718_2A_MIPI_1080P_30FPS)
                || (SENSOR_TYPE == APTINA_AR0237_LVDS_1080P_30FPS))
        {
            g_stViChnConfig.enWDRMode = WDR_MODE_2To1_LINE;
        }
        else
        {
            g_stViChnConfig.enWDRMode = WDR_MODE_2To1_FRAME;
        }
        s32Ret = SAMPLE_VIO_WDRSwitch_PreView(&g_stViChnConfig);
        g_stViChnConfig.enWDRMode = WDR_MODE_NONE;
        break;
        
    case '2':
        s32Ret = SAMPLE_VIO_FRAME_FPN_PreView(&g_stViChnConfig);
        break;

    case '3':
        g_stViChnConfig.enRotate = ROTATE_90;
        s32Ret = SAMPLE_VIO_ROTATE_PreView(&g_stViChnConfig);
		g_stViChnConfig.enRotate = ROTATE_NONE;
        break;

    case '4':
        s32Ret = SAMPLE_VIO_LDC_PreView(&g_stViChnConfig);
        break;

    case '5':
        s32Ret = SAMPLE_VI_DIS_VPSS_VO_1080P_PreView(&g_stViChnConfig);
        break;

        /* VI: BT1120(1080I); VO:HD0(HDMI).  Isp bypass, phychn channel preview. */
    case '6':
        g_stViChnConfig.enViMode = SAMPLE_VI_MODE_BT1120_1080P;
        g_stViChnConfig.enNorm = VIDEO_ENCODING_MODE_PAL;
        s32Ret = SAMPLE_VIO_BT1120_1080P_PreView(&g_stViChnConfig);
        break;
        
    case '7':
        if ((SENSOR_TYPE == SONY_IMX290_MIPI_1080P_30FPS)
                || (SENSOR_TYPE == OMNIVISION_OV2718_MIPI_1080P_30FPS)
                || (SENSOR_TYPE == OMNIVISION_OV2718_2A_MIPI_1080P_30FPS)
                || (SENSOR_TYPE == SONY_IMX323_CMOS_1080P_30FPS)
                || (SENSOR_TYPE == APTINA_AR0237_LVDS_1080P_30FPS))
        {
            SAMPLE_PRT("not support!\n");
            break;
        }
        SAMPLE_VIO_ResolutionSwitch_Preview(&g_stViChnConfig);
        break;
        
    default:
        SAMPLE_VIO_Usage("sample_vio");
        return HI_FAILURE;
    }


    if (HI_SUCCESS == s32Ret)
    {
        SAMPLE_PRT("program exit normally!\n");
    }
    else
    {
        SAMPLE_PRT("program exit abnormally!\n");
    }

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

