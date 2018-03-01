/******************************************************************************
  A simple program of Hisilicon HI3521 video input and output implementation.
  Copyright (C), 2012-2020, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2012-7 Created
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

#include "sample_comm.h"

typedef struct hiSAMPLE_SPOT_THRD_ARG_S 
{
    VO_DEV VoDev;
    HI_S32 s32ViChnCnt;
    HI_BOOL bToExit;
}SAMPLE_SPOT_THRD_ARG_S;

/******************************************************************************
* function : show usage
******************************************************************************/
typedef struct hiSAMPLE_VICHN_D1_STATS_S
{
    /* 0 means this chn is not capturing D1, else means it is capturing D1.
       No matter who requests, it should add 1. */
    HI_U32 u32ReqCnt;
    pthread_mutex_t stThrdLock;
}SAMPLE_VICHN_D1_STATS_S;


static VIDEO_NORM_E s_enNorm = VIDEO_ENCODING_MODE_PAL;
static HI_U32 s_u32D1Height = 0; 
static HI_U32 s_u32ViFrmRate = 0; 
static pthread_t s_stSdSpotThrd;
static SAMPLE_SPOT_THRD_ARG_S s_stSpotThrdArg;
static SAMPLE_VICHN_D1_STATS_S s_astViCapD1Status[VIU_MAX_CHN_NUM];
static SIZE_S s_stD1Size;
static SIZE_S s_st2CifSize;
static SIZE_S s_stCifSize;

/******************************************************************************
* function : show usage
******************************************************************************/
void SAMPLE_IT_Usage(char *sPrgNm)
{
    printf("Usage : %s <index>\n", sPrgNm);
    printf("index:\n");
    printf("\t 0) VI:16*D1/2Cif MixCap + Venc:16*D1@6fps&16*CIF@25fps + VO:HD0(HDMI + VGA), SD1(CVBS) video preview.\n");
    printf("\t 1) VI:16*D1/2Cif MixCap + Venc:16*D1@6fps&16*CIF@6fps + VO:HD0(HDMI + VGA), SD1(CVBS) video preview.\n");
    printf("\t 2) VI:16*2Cif + Venc:16*CIF@25fps&16*QCIF@25fps + VO:HD0(HDMI + VGA).\n");	

    return;
}

void SAMPLE_IT_GlobalVarInit()
{   
    HI_S32 i;
    
    s_u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL == s_enNorm) ? 25 : 30;
    s_u32D1Height = (VIDEO_ENCODING_MODE_PAL == s_enNorm) ? 576 : 480;
    s_stD1Size.u32Width = D1_WIDTH;
    s_stD1Size.u32Height = s_u32D1Height;
    s_st2CifSize.u32Width = D1_WIDTH / 2;
    s_st2CifSize.u32Height = s_u32D1Height;
    s_stCifSize.u32Width = D1_WIDTH / 2;
    s_stCifSize.u32Height = s_u32D1Height / 2;
    for (i = 0; i < VIU_MAX_CHN_NUM; ++i)
    {
        s_astViCapD1Status[i].u32ReqCnt = 0;
        pthread_mutex_init(&s_astViCapD1Status[i].stThrdLock, NULL);
    }
}

/******************************************************************************
* function : to process abnormal case                                         
******************************************************************************/
void SAMPLE_IT_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo)
    {
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

HI_S32 SAMPLE_IT_StartHD(VO_DEV VoDev, const VO_PUB_ATTR_S *pstVoPubAttr, 
    SAMPLE_VO_MODE_E enVoMode, VIDEO_NORM_E enVideoNorm, HI_U32 u32SrcFrmRate)
{
    HI_S32 s32Ret;
    
	s32Ret = SAMPLE_COMM_VO_StartDevLayer(VoDev, pstVoPubAttr, u32SrcFrmRate);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDevLayer failed!\n");
		goto END_StartHD_0;
	}

	s32Ret = SAMPLE_COMM_VO_StartChn(VoDev, pstVoPubAttr, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_StartHD_1;
	}

	/* if it's displayed on HDMI, we should start HDMI */
	if (pstVoPubAttr->enIntfType & VO_INTF_HDMI)
	{
        s32Ret = SAMPLE_COMM_VO_HdmiStart(pstVoPubAttr->enIntfSync);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
			goto END_StartHD_2;
		}
	}
    return HI_SUCCESS;
    
END_StartHD_2:
    SAMPLE_COMM_VO_StopChn(VoDev, enVoMode);
END_StartHD_1:
    SAMPLE_COMM_VO_StopDevLayer(VoDev);
END_StartHD_0:
    return s32Ret;
}

HI_S32 SAMPLE_IT_StopHD(VO_DEV VoDev, const VO_PUB_ATTR_S *pstVoPubAttr, SAMPLE_VO_MODE_E enVoMode)
{   
    if (pstVoPubAttr->enIntfType & VO_INTF_HDMI)
    {
        SAMPLE_COMM_VO_HdmiStop();
    }
    SAMPLE_COMM_VO_StopChn(VoDev, enVoMode);
    SAMPLE_COMM_VO_StopDevLayer(VoDev);
    
    return HI_SUCCESS;
}

/******************************************************************************
* function :  vodev sd spot process. 
******************************************************************************/
void *SAMPLE_IT_SdSpotProc(void *pData)
{
    SAMPLE_SPOT_THRD_ARG_S *pstThrdArg;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    HI_S32 i = 0;
    HI_S32 s32Ret;
    VI_CHN_ATTR_S stViChnOldAttr;
    VI_CHN_ATTR_S stViChnD1Attr;   
    
    pstThrdArg = (SAMPLE_SPOT_THRD_ARG_S *)pData;

    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    
    stDestChn.enModId = HI_ID_VOU;
    stDestChn.s32DevId = pstThrdArg->VoDev;
    stDestChn.s32ChnId = 0;

    while (!pstThrdArg->bToExit)
    {
        stSrcChn.s32ChnId = i;

        pthread_mutex_lock(&s_astViCapD1Status[i].stThrdLock);
        if (0 == s_astViCapD1Status[i].u32ReqCnt)
        {
            /* If this chn is not capturing D1 now, change it to capture D1 */
            s32Ret = HI_MPI_VI_GetChnAttr(i, &stViChnOldAttr);
            memcpy(&stViChnD1Attr, &stViChnOldAttr, sizeof(stViChnD1Attr));
            stViChnD1Attr.stDestSize.u32Width = D1_WIDTH;
            stViChnD1Attr.s32SrcFrameRate = s_u32ViFrmRate;
            stViChnD1Attr.s32FrameRate = s_u32ViFrmRate;
            s32Ret = HI_MPI_VI_SetChnAttr(i, &stViChnD1Attr);
        }
        ++s_astViCapD1Status[i].u32ReqCnt;
        pthread_mutex_unlock(&s_astViCapD1Status[i].stThrdLock);

        s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return NULL;
        }

        sleep(2);

        pthread_mutex_lock(&s_astViCapD1Status[i].stThrdLock);
        if (1 == s_astViCapD1Status[i].u32ReqCnt)
        {
            s32Ret = HI_MPI_VI_SetChnAttr(i, &stViChnOldAttr);
        }
        --s_astViCapD1Status[i].u32ReqCnt;
        pthread_mutex_unlock(&s_astViCapD1Status[i].stThrdLock);

        s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return NULL;
        }
        i = (i + 1) % pstThrdArg->s32ViChnCnt;
    }
}

/******************************************************************************
* function :  start vodev sd to spot. 
******************************************************************************/
HI_S32 SAMPLE_IT_StartSdSpot(VO_DEV VoDev, VIDEO_NORM_E enVideoNorm, HI_S32 s32ViChnCnt)
{
    VO_PUB_ATTR_S stVoPubAttr;
    HI_S32 s32Ret;

    stVoPubAttr.enIntfSync = (VIDEO_ENCODING_MODE_NTSC == enVideoNorm) ? VO_OUTPUT_NTSC : VO_OUTPUT_PAL;
    stVoPubAttr.enIntfType = VO_INTF_CVBS;
    stVoPubAttr.u32BgColor = 0x000000ff;
    stVoPubAttr.bDoubleFrame = HI_FALSE;
    s32Ret = SAMPLE_COMM_VO_StartDevLayer(VoDev, &stVoPubAttr, s_u32ViFrmRate);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartDevLayer failed!\n");
        return s32Ret;
    }

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start VoDev %d failed!\n", VoDev);
        return s32Ret;
    }
    
	s32Ret = SAMPLE_COMM_VO_StartChn(VoDev, &stVoPubAttr, VO_MODE_1MUX);
	if (HI_SUCCESS != s32Ret)
	{
		 SAMPLE_PRT("SAMPLE_COMM_VO_StartChn failed!\n");
         return s32Ret;
	}
    
    s_stSpotThrdArg.VoDev = VoDev;
    s_stSpotThrdArg.s32ViChnCnt = s32ViChnCnt;
    s_stSpotThrdArg.bToExit = HI_FALSE;
    s32Ret = pthread_create(&s_stSdSpotThrd, NULL, SAMPLE_IT_SdSpotProc, (HI_VOID*)&s_stSpotThrdArg);
    if (0 != s32Ret)
    {
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

/******************************************************************************
* funciton : stop get venc stream process.
******************************************************************************/
HI_S32 SAMPLE_IT_StopSdSpot(VO_DEV VoDev)
{
    if (HI_TRUE != s_stSpotThrdArg.bToExit)
    {
        s_stSpotThrdArg.bToExit = HI_TRUE;
        pthread_join(s_stSdSpotThrd, 0);
    }
    
    SAMPLE_COMM_VO_StopChn(VoDev, VO_MODE_1MUX);
    SAMPLE_COMM_VO_StopDevLayer(VoDev);
    
    return HI_SUCCESS;
}



/******************************************************************************
* function :  VI:16*D1(6fps)/2cif(19fps); VO:HD0(HDMI 1080P)video preview. 
******************************************************************************/
HI_S32 SAMPLE_IT_6fpsD1_25fpsCif(HI_VOID)
{
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_16_D1Cif;
    HI_U32 s32ViChnCnt = 16;
    HI_S32 s32VpssGrpCnt = 32;
    VB_CONF_S stVbConf;
    VPSS_GRP VpssGrp;
    VPSS_GRP_ATTR_S stGrpAttr;
	VPSS_CHN VpssChn_VoHD0 = VPSS_PRE0_CHN;
    VPSS_CHN_ATTR_S stVpssChnAttr;
    VO_DEV VoDev;
    VO_CHN VoChn;
    VO_PUB_ATTR_S stVoPubAttr; 
    SAMPLE_VO_MODE_E enVoMode, enPreVoMode;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    HI_S32 i;
    HI_S32 j;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    HI_CHAR ch;
    HI_U32 u32WndNum;
    VENC_GRP VencGrp;
    VENC_CHN VencChn;
    GROUP_FRAME_RATE_S stGrpFrmRate;
    VENC_CHN_ATTR_S stVencChnAttr;

    /******************************************
     step  1: VB and system init.
    ******************************************/
    memset(&stVbConf,0,sizeof(VB_CONF_S));
    stVbConf.u32MaxPoolCnt = 64;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(s_enNorm,\
                PIC_D1, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = 60;
    
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(s_enNorm,\
                PIC_2CIF, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt = 110;
    
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(s_enNorm,\
                PIC_CIF, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[2].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[2].u32BlkCnt = 64;

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_16_MixCap_0;
    }

    /******************************************
     step 2: start vo HD0(HDMI) 
    ******************************************/
	printf("start vo HD0.\n");
	VoDev = SAMPLE_VO_DEV_DHD0;
	u32WndNum = 16;
	enVoMode = VO_MODE_16MUX;
    if(VIDEO_ENCODING_MODE_PAL == s_enNorm)
    {
        stVoPubAttr.enIntfSync = VO_OUTPUT_1080P50;
    }
    else
    {
        stVoPubAttr.enIntfSync = VO_OUTPUT_1080P60;
    }
    stVoPubAttr.enIntfType = VO_INTF_HDMI | VO_INTF_VGA;
    stVoPubAttr.u32BgColor = 0x000000ff;
    stVoPubAttr.bDoubleFrame = HI_TRUE;
    s32Ret = SAMPLE_IT_StartHD(VoDev, &stVoPubAttr, enVoMode, s_enNorm, s_u32ViFrmRate);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start VO failed!\n");
        goto END_16_MixCap_1;
    }
    
    /******************************************
     step 3: start vpss,  bind it to venc and vo
    ******************************************/    
    stGrpAttr.u32MaxW = D1_WIDTH;
    stGrpAttr.u32MaxH = s_u32D1Height;
    stGrpAttr.bDrEn = HI_FALSE;
    stGrpAttr.bDbEn = HI_FALSE;
    stGrpAttr.bIeEn = HI_TRUE;
    stGrpAttr.bNrEn = HI_TRUE;
    stGrpAttr.bHistEn = HI_TRUE;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;
    stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

    stVpssChnAttr.bFrameEn = HI_TRUE;
    stVpssChnAttr.bSpEn = HI_TRUE;
    for (i = 0; i < 4; i++)
    {
        stVpssChnAttr.stFrame.u32Width[i] = 2;
        stVpssChnAttr.stFrame.u32Color[i] = 0xff00;
    }

    stGrpFrmRate.s32ViFrmRate = s_u32ViFrmRate;
    stGrpFrmRate.s32VpssFrmRate = 6;
    for (i = 0; i < s32VpssGrpCnt; i++)
    {
        VpssGrp = i;
        s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stGrpAttr);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VPSS_CreateGrp failed!\n");
            goto END_16_MixCap_2;
        }

        VencGrp = i;
        VencChn = i;
        if (VpssGrp < 16)
        {
            /* 0-15vpss grp for venc 16D1@6fps */
            s32Ret |= HI_MPI_VPSS_SetChnAttr(VpssGrp, VPSS_LSTR_CHN, &stVpssChnAttr);
            s32Ret |= HI_MPI_VPSS_EnableChn(VpssGrp, VPSS_LSTR_CHN);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Start Vpss failed!\n");
                goto END_16_MixCap_2;
            }

            s32Ret |= SAMPLE_COMM_VENC_Start(VencGrp, VencChn, PT_H264,\
                                           s_enNorm, PIC_D1, SAMPLE_RC_VBR);
            s32Ret |= HI_MPI_VENC_SetGrpFrmRate(VencGrp, &stGrpFrmRate);
            s32Ret |= HI_MPI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
            stVencChnAttr.stRcAttr.stAttrH264Vbr.u32ViFrmRate = s_u32ViFrmRate;
            stVencChnAttr.stRcAttr.stAttrH264Vbr.fr32TargetFrmRate = 6;
            s32Ret |= HI_MPI_VENC_SetChnAttr(VencChn, &stVencChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Start Venc failed!\n");
                goto END_16_MixCap_2;
            }
            s32Ret = SAMPLE_COMM_VENC_BindVpss(VencGrp, VpssGrp, VPSS_LSTR_CHN);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Start Venc failed!\n");
                goto END_16_MixCap_2;
            }
        }
        else
        {
            /* 16-31 vpss grp for vo HD and venc 16cif@25fps */            
            s32Ret |= HI_MPI_VPSS_SetChnAttr(VpssGrp, VPSS_LSTR_CHN, &stVpssChnAttr);
            s32Ret |= HI_MPI_VPSS_SetChnAttr(VpssGrp, VPSS_PRE0_CHN, &stVpssChnAttr);
            s32Ret |= HI_MPI_VPSS_EnableChn(VpssGrp, VPSS_LSTR_CHN);
            s32Ret |= HI_MPI_VPSS_EnableChn(VpssGrp, VPSS_PRE0_CHN);
            /* open pre-scale */
            s32Ret |= SAMPLE_COMM_EnableVpssPreScale(i, s_st2CifSize);
            if(HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("HI_MPI_VPSS_SetPreScale failed!\n");
                goto END_16_MixCap_2;
            }
            
            VoChn = VpssGrp - 16;
            s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev, VoChn, VpssGrp, VpssChn_VoHD0);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Start VO failed!\n");
                goto END_16_MixCap_2;
            }
            
            s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, PT_H264,\
                                           s_enNorm, PIC_CIF, SAMPLE_RC_VBR);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Start Venc failed!\n");
                goto END_16_MixCap_2;
            }
            s32Ret = SAMPLE_COMM_VENC_BindVpss(VencGrp, VpssGrp, VPSS_LSTR_CHN);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("SAMPLE_COMM_VENC_BindVpss failed!\n");
                goto END_16_MixCap_2;
            }
        }
        
        s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VPSS_StartGrp failed!\n");
            goto END_16_MixCap_2;
        }
    }

    /******************************************
     step 4: bind vpss to vi
    ******************************************/
    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32ChnId = 0;
    for (j=0; j<s32ViChnCnt; j++)
    {    
        stSrcChn.s32ChnId = j;
        stDestChn.s32DevId = j;
        s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            goto END_16_MixCap_3;
        }
        
        stDestChn.s32DevId = j + 16;
        s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            goto END_16_MixCap_3;
        }
    }

    /******************************************
     step 5: start a thread to get venc stream 
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(32);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_16_MixCap_3;
    }

    /******************************************
     step 6: start vi dev & chn
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_MixCap_Start(enViMode, s_enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_16_MixCap_4;
    }

    /******************************************
     step 7: start vo sd to spot
    ******************************************/
    s32Ret = SAMPLE_IT_StartSdSpot(SAMPLE_VO_DEV_DSD0, s_enNorm, s32ViChnCnt);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start VoDev %d to spot failed!\n", SAMPLE_VO_DEV_DSD0);
        goto END_16_MixCap_4;
    }

    /******************************************
     step 8: HD0 switch mode 
    ******************************************/
    while(1)
    {
  		enPreVoMode = enVoMode;
        printf("please choose preview mode, press 'q' to exit this sample.\n"); 
        printf("\t0) 4 preview\n");
        printf("\t1) 16 preview\n");
        printf("\tq) quit\n");
 
        ch = getchar();
        getchar();
        if ('0' == ch)
        {
            u32WndNum = 4;
            enVoMode = VO_MODE_4MUX;
        }
        else if ('1' == ch)
        {
            u32WndNum = 16;
            enVoMode = VO_MODE_16MUX;
        }
        else if ('q' == ch)
        {
            break;
        }
        else
        {
            SAMPLE_PRT("preview mode invaild! please try again.\n");
            continue;
        }
        
        if (enVoMode == enPreVoMode)
        {
            continue;
        }
        else if (VO_MODE_4MUX == enVoMode)
		{
			for(i = 0; i < 4; i++)
		 	{
                /* change to all capture D1 */
                pthread_mutex_lock(&s_astViCapD1Status[i].stThrdLock);
                if (0 == s_astViCapD1Status[i].u32ReqCnt)
                {
                    s32Ret = SAMPLE_COMM_VI_ChangeMixCap(i, HI_FALSE, s_u32ViFrmRate);
                    if (HI_SUCCESS != s32Ret)
                    {
                        SAMPLE_PRT("SAMPLE_COMM_VI_ChangeCapSize VO failed!\n");
                        goto END_16_MixCap_4;
                    }
                }
                ++s_astViCapD1Status[i].u32ReqCnt;
                pthread_mutex_unlock(&s_astViCapD1Status[i].stThrdLock);

				s32Ret = SAMPLE_COMM_EnableVpssPreScale(i, s_stD1Size);
				if (HI_SUCCESS != s32Ret)
				{
					SAMPLE_PRT("SAMPLE_COMM_DisableVpssPreScale VO failed!\n");
					goto END_16_MixCap_4;
				}
		 	}
		}
        else if (VO_MODE_16MUX== enVoMode)
        {
			for(i = 0; i < 16; i++)
		 	{
                /* try to change from  D1 to D1/2cif */
                pthread_mutex_lock(&s_astViCapD1Status[i].stThrdLock);
                if (1 == s_astViCapD1Status[i].u32ReqCnt)
                {
                    s32Ret = SAMPLE_COMM_VI_ChangeMixCap(i, HI_TRUE, 6);
                    if (HI_SUCCESS != s32Ret)
                    {
                        SAMPLE_PRT("SAMPLE_COMM_VI_ChangeMixCap VO failed!\n");
                        goto END_16_MixCap_4;
                    }
                    --s_astViCapD1Status[i].u32ReqCnt;
                }
                else if (1 < s_astViCapD1Status[i].u32ReqCnt)
                {
                    --s_astViCapD1Status[i].u32ReqCnt;
                }
                else
                {
                    s32Ret = SAMPLE_COMM_VI_ChangeMixCap(i, HI_TRUE, 6);
                    if (HI_SUCCESS != s32Ret)
                    {
                        SAMPLE_PRT("SAMPLE_COMM_VI_ChangeMixCap VO failed!\n");
                        goto END_16_MixCap_4;
                    }
                }
                pthread_mutex_unlock(&s_astViCapD1Status[i].stThrdLock);

				s32Ret = SAMPLE_COMM_EnableVpssPreScale(i + 16, s_st2CifSize);
				if (HI_SUCCESS != s32Ret)
				{
					SAMPLE_PRT("SAMPLE_COMM_DisableVpssPreScale VO failed!\n");
					goto END_16_MixCap_4;
				}
		 	}
        }
		else if (enVoMode== enPreVoMode)
		{
			continue;
		}
		
        SAMPLE_PRT("vo(%d) switch to %d mode\n", VoDev, u32WndNum);

        s32Ret= HI_MPI_VO_SetAttrBegin(VoDev);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start VO failed!\n");
            goto END_16_MixCap_4;
        }
        
        s32Ret = SAMPLE_COMM_VO_StopChn(VoDev, enPreVoMode);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start VO failed!\n");
            goto END_16_MixCap_4;
        }

        s32Ret = SAMPLE_COMM_VO_StartChn(VoDev, &stVoPubAttr, enVoMode);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start VO failed!\n");
            goto END_16_MixCap_4;
        }
        s32Ret= HI_MPI_VO_SetAttrEnd(VoDev);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start VO failed!\n");
            goto END_16_MixCap_4;
        }
    }

    /******************************************
     step 9: exit process
    ******************************************/
END_16_MixCap_4:
    SAMPLE_COMM_VI_Stop(enViMode);
    SAMPLE_IT_StopSdSpot(SAMPLE_VO_DEV_DSD0);
END_16_MixCap_3:
    SAMPLE_COMM_VENC_StopGetStream();
    SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_16_MixCap_2:
    for (i=0; i<32; i++)
    {
        VpssGrp = i;
        VencGrp = i;
        VencChn = i;
        SAMPLE_COMM_VENC_UnBindVpss(VencGrp, VpssGrp, VPSS_LSTR_CHN);
        SAMPLE_COMM_VENC_Stop(VencGrp, VencChn);
    }
    SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_16_MixCap_1:
    VoDev = SAMPLE_VO_DEV_DHD0;
    u32WndNum = 16;
    enVoMode = VO_MODE_16MUX;
    for(i=0;i<u32WndNum;i++)
    {
        VoChn = i;
        VpssGrp = i;
        SAMPLE_COMM_VO_UnBindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);
    }
    SAMPLE_IT_StopHD(VoDev, &stVoPubAttr, enVoMode);
END_16_MixCap_0:
    SAMPLE_COMM_SYS_Exit();
    
    return s32Ret;
}


HI_S32 SAMPLE_IT_6fpsD1_6fpsCif(HI_VOID)
{
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_16_D1Cif;
    HI_U32 s32ViChnCnt = 16;
    HI_S32 s32VpssGrpCnt = 32;
    VB_CONF_S stVbConf;
    VPSS_GRP VpssGrp;
    VPSS_GRP_ATTR_S stGrpAttr;
    VPSS_CHN VpssChn_VoHD0 = VPSS_PRE0_CHN;
    VPSS_CHN_ATTR_S stVpssChnAttr;
    VO_DEV VoDev;
    VO_CHN VoChn;
    VO_PUB_ATTR_S stVoPubAttr; 
    SAMPLE_VO_MODE_E enVoMode, enPreVoMode;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    HI_S32 i;
    HI_S32 j;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    HI_CHAR ch;
    HI_U32 u32WndNum;
    VENC_GRP VencGrp;
    VENC_CHN VencChn;
    GROUP_FRAME_RATE_S stGrpFrmRate;
    VENC_CHN_ATTR_S stVencChnAttr;
    VPSS_SIZER_INFO_S stVpssSizerInfo;
    VPSS_FRAME_RATE_S stVpssFrameRate;

    /******************************************
     step  1: VB and system init. 
    ******************************************/
    memset(&stVbConf,0,sizeof(VB_CONF_S));
    stVbConf.u32MaxPoolCnt = 64;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(s_enNorm,\
                PIC_D1, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = 60;
    
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(s_enNorm,\
                PIC_2CIF, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt = 110;
    
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(s_enNorm,\
                PIC_CIF, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[2].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[2].u32BlkCnt = 64;

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_16_MixCap_0;
    }

    /******************************************
     step 2: start vo HD0(HDMI) 
    ******************************************/
    printf("start vo HD0.\n");
	VoDev = SAMPLE_VO_DEV_DHD0;
	u32WndNum = 16;
	enVoMode = VO_MODE_16MUX;
    if(VIDEO_ENCODING_MODE_PAL == s_enNorm)
    {
        stVoPubAttr.enIntfSync = VO_OUTPUT_1080P50;
    }
    else
    {
        stVoPubAttr.enIntfSync = VO_OUTPUT_1080P60;
    }
    stVoPubAttr.enIntfType = VO_INTF_HDMI | VO_INTF_VGA;
    stVoPubAttr.u32BgColor = 0x000000ff;
    stVoPubAttr.bDoubleFrame = HI_TRUE;
    s32Ret = SAMPLE_IT_StartHD(VoDev, &stVoPubAttr, enVoMode, s_enNorm, s_u32ViFrmRate);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start VO failed!\n");
        goto END_16_MixCap_1;
    }

    /******************************************
     step 3: start vpss, bind it to venc and vo
    ******************************************/    
    stGrpAttr.u32MaxW = D1_WIDTH;
    stGrpAttr.u32MaxH = s_u32D1Height;
    stGrpAttr.bDrEn = HI_FALSE;
    stGrpAttr.bDbEn = HI_FALSE;
    stGrpAttr.bIeEn = HI_TRUE;
    stGrpAttr.bNrEn = HI_TRUE;
    stGrpAttr.bHistEn = HI_TRUE;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;
    stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

    stVpssChnAttr.bFrameEn = HI_TRUE;
    stVpssChnAttr.bSpEn = HI_TRUE;
    for (i=0; i<4; i++)
    {
        stVpssChnAttr.stFrame.u32Width[i] = 2;
        stVpssChnAttr.stFrame.u32Color[i] = 0xff00;
    }

    stGrpFrmRate.s32ViFrmRate = s_u32ViFrmRate;
    stGrpFrmRate.s32VpssFrmRate = 6;
    stVpssSizerInfo.bSizer = HI_TRUE;
    stVpssSizerInfo.stSize.u32Width = D1_WIDTH;
    stVpssSizerInfo.stSize.u32Height = s_u32D1Height;
    stVpssFrameRate.s32SrcFrmRate = s_u32ViFrmRate;
    stVpssFrameRate.s32DstFrmRate = 6;
    for (i = 0; i < s32VpssGrpCnt; i++)
    {
        VpssGrp = i;
        s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stGrpAttr);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VPSS_CreateGrp failed!\n");
            goto END_16_MixCap_2;
        }

        if (VpssGrp < 16)
        {
            /* 0-15vpss grp for venc 16D1@6fps and 16CIF@6fps  */
            /* set vpss sizer to d1 and set the frame rate to only let D1@6fps to pass */
            s32Ret |= HI_MPI_VPSS_SetGrpSizer(VpssGrp, &stVpssSizerInfo);
            s32Ret |= HI_MPI_VPSS_SetGrpFrameRate(VpssGrp, &stVpssFrameRate);
            
            s32Ret |= HI_MPI_VPSS_SetChnAttr(VpssGrp, VPSS_PRE0_CHN, &stVpssChnAttr);
            s32Ret |= HI_MPI_VPSS_SetChnAttr(VpssGrp, VPSS_PRE1_CHN, &stVpssChnAttr);
            s32Ret |= HI_MPI_VPSS_EnableChn(VpssGrp, VPSS_PRE0_CHN);
            s32Ret |= HI_MPI_VPSS_EnableChn(VpssGrp, VPSS_PRE1_CHN);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Start Vpss failed!\n");
                goto END_16_MixCap_2;
            }

            /* start venc chn for D1 and bind it to VPSS_PRE0_CHN */
            VencGrp = i;
            VencChn = i;
            s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, PT_H264,\
                                           s_enNorm, PIC_D1, SAMPLE_RC_VBR);
            s32Ret |= HI_MPI_VENC_SetGrpFrmRate(VencGrp, &stGrpFrmRate);
            s32Ret |= HI_MPI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
            stVencChnAttr.stRcAttr.stAttrH264Vbr.u32ViFrmRate = s_u32ViFrmRate;
            stVencChnAttr.stRcAttr.stAttrH264Vbr.fr32TargetFrmRate = 6;
            s32Ret |= HI_MPI_VENC_SetChnAttr(VencChn, &stVencChnAttr);
            s32Ret |= SAMPLE_COMM_VENC_BindVpss(VencGrp, VpssGrp, VPSS_PRE0_CHN);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Start Venc failed!\n");
                goto END_16_MixCap_2;
            }

            /* start venc chn for CIF and bind it to VPSS_PRE1_CHN */
            VencGrp = i + 16;
            VencChn = i + 16;
            s32Ret |= SAMPLE_COMM_VENC_Start(VencGrp, VencChn, PT_H264,\
                                           s_enNorm, PIC_CIF, SAMPLE_RC_VBR);
            s32Ret |= HI_MPI_VENC_SetGrpFrmRate(VencGrp, &stGrpFrmRate);
            s32Ret |= HI_MPI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
            stVencChnAttr.stRcAttr.stAttrH264Vbr.u32ViFrmRate = s_u32ViFrmRate;
            stVencChnAttr.stRcAttr.stAttrH264Vbr.fr32TargetFrmRate = 6;
            s32Ret |= HI_MPI_VENC_SetChnAttr(VencChn, &stVencChnAttr);
            s32Ret |= SAMPLE_COMM_VENC_BindVpss(VencGrp, VpssGrp, VPSS_PRE1_CHN);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("SAMPLE_COMM_VENC_BindVpss failed!\n");
                goto END_16_MixCap_2;
            }
        }
        else
        {
            /* 16-31 vpss grp for vo HD */            
            s32Ret |= HI_MPI_VPSS_SetChnAttr(VpssGrp, VPSS_PRE0_CHN, &stVpssChnAttr);
            s32Ret |= HI_MPI_VPSS_EnableChn(VpssGrp, VPSS_PRE0_CHN);
            /* open pre-scale */
            s32Ret |= SAMPLE_COMM_EnableVpssPreScale(i, s_st2CifSize);
            if(HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("HI_MPI_VPSS_SetPreScale failed!\n");
                goto END_16_MixCap_2;
            }
            
            VoChn = VpssGrp - 16;
            s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev, VoChn, VpssGrp, VpssChn_VoHD0);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Start VO failed!\n");
                goto END_16_MixCap_2;
            }
        }
        
        s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VPSS_StartGrp failed!\n");
            goto END_16_MixCap_2;
        }
    }

    /******************************************
     step 4: bind vpss to vi
    ******************************************/
    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32ChnId = 0;
    for (j=0; j<s32ViChnCnt; j++)
    {    
        stSrcChn.s32ChnId = j;
        stDestChn.s32DevId = j;
        s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            goto END_16_MixCap_3;
        }
        
        stDestChn.s32DevId = j + 16;
        s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            goto END_16_MixCap_3;
        }
    }

    /******************************************
     step 5: start a thread to get venc stream 
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(32);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_16_MixCap_3;
    }
    
    /******************************************
     step 6: start vi dev & chn
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_MixCap_Start(enViMode, s_enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_16_MixCap_4;
    }

    /******************************************
     step 7: start vo sd to spot 
    ******************************************/
    s32Ret = SAMPLE_IT_StartSdSpot(SAMPLE_VO_DEV_DSD0, s_enNorm, s32ViChnCnt);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start VoDev %d to spot failed!\n", SAMPLE_VO_DEV_DSD0);
        goto END_16_MixCap_4;
    }

    /******************************************
     step 8: HD0 switch mode 
    ******************************************/
    while(1)
    {
        enPreVoMode = enVoMode;
        printf("please choose preview mode, press 'q' to exit this sample.\n"); 
        printf("\t0) 4 preview\n");
        printf("\t1) 16 preview\n");
        printf("\tq) quit\n");
 
        ch = getchar();
        getchar();
        if ('0' == ch)
        {
            u32WndNum = 4;
            enVoMode = VO_MODE_4MUX;
        }
        else if ('1' == ch)
        {
            u32WndNum = 16;
            enVoMode = VO_MODE_16MUX;
        }
        else if ('q' == ch)
        {
            break;
        }
        else
        {
            SAMPLE_PRT("preview mode invaild! please try again.\n");
            continue;
        }
        
        if (enVoMode == enPreVoMode)
        {
            continue;
        }
        else if (VO_MODE_4MUX == enVoMode)
        {
            for(i = 0; i < 4; i++)
            {
                /* change to all capture D1 */
                pthread_mutex_lock(&s_astViCapD1Status[i].stThrdLock);
                if (0 == s_astViCapD1Status[i].u32ReqCnt)
                {
                    s32Ret = SAMPLE_COMM_VI_ChangeMixCap(i, HI_FALSE, s_u32ViFrmRate);
                    if (HI_SUCCESS != s32Ret)
                    {
                        SAMPLE_PRT("SAMPLE_COMM_VI_ChangeCapSize VO failed!\n");
                        goto END_16_MixCap_4;
                    }
                }
                ++s_astViCapD1Status[i].u32ReqCnt;
                pthread_mutex_unlock(&s_astViCapD1Status[i].stThrdLock);

                s32Ret = SAMPLE_COMM_EnableVpssPreScale(i, s_stD1Size);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_PRT("SAMPLE_COMM_DisableVpssPreScale VO failed!\n");
                    goto END_16_MixCap_4;
                }
            }
        }
        else if (VO_MODE_16MUX == enVoMode)
        {
            for(i = 0; i < 16; i++)
            {
                /* try to change from  D1 to D1/2cif */
                pthread_mutex_lock(&s_astViCapD1Status[i].stThrdLock);
                if (1 == s_astViCapD1Status[i].u32ReqCnt)
                {
                    s32Ret = SAMPLE_COMM_VI_ChangeMixCap(i, HI_TRUE, 6);
                    if (HI_SUCCESS != s32Ret)
                    {
                        SAMPLE_PRT("SAMPLE_COMM_VI_ChangeMixCap VO failed!\n");
                        goto END_16_MixCap_4;
                    }
                    --s_astViCapD1Status[i].u32ReqCnt;
                }
                else if (1 < s_astViCapD1Status[i].u32ReqCnt)
                {
                    --s_astViCapD1Status[i].u32ReqCnt;
                }
                else
                {
                    s32Ret = SAMPLE_COMM_VI_ChangeMixCap(i, HI_TRUE, 6);
                    if (HI_SUCCESS != s32Ret)
                    {
                        SAMPLE_PRT("SAMPLE_COMM_VI_ChangeMixCap VO failed!\n");
                        goto END_16_MixCap_4;
                    }
                }
                pthread_mutex_unlock(&s_astViCapD1Status[i].stThrdLock);
                
                s32Ret = SAMPLE_COMM_EnableVpssPreScale(i + 16, s_st2CifSize);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_PRT("SAMPLE_COMM_DisableVpssPreScale VO failed!\n");
                    goto END_16_MixCap_4;
                }
            }
        }
        else if (enVoMode== enPreVoMode)
        {
            continue;
        }
        
        SAMPLE_PRT("vo(%d) switch to %d mode\n", VoDev, u32WndNum);

        s32Ret= HI_MPI_VO_SetAttrBegin(VoDev);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start VO failed!\n");
            goto END_16_MixCap_4;
        }
        
        s32Ret = SAMPLE_COMM_VO_StopChn(VoDev, enPreVoMode);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start VO failed!\n");
            goto END_16_MixCap_4;
        }

        s32Ret = SAMPLE_COMM_VO_StartChn(VoDev, &stVoPubAttr, enVoMode);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start VO failed!\n");
            goto END_16_MixCap_4;
        }
        s32Ret= HI_MPI_VO_SetAttrEnd(VoDev);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start VO failed!\n");
            goto END_16_MixCap_4;
        }
    }

    /******************************************
     step 9: exit process
    ******************************************/
END_16_MixCap_4:
    SAMPLE_COMM_VI_Stop(enViMode);
    SAMPLE_IT_StopSdSpot(SAMPLE_VO_DEV_DSD0);
END_16_MixCap_3:
    SAMPLE_COMM_VENC_StopGetStream();
    SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_16_MixCap_2:
    for (i=0; i<32; i++)
    {
        VpssGrp = i;
        SAMPLE_COMM_VENC_UnBindVpss(i, VpssGrp, VPSS_LSTR_CHN);
        SAMPLE_COMM_VENC_Stop(i, i);
    }
    SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_16_MixCap_1:
    VoDev = SAMPLE_VO_DEV_DHD0;
    u32WndNum = 16;
    enVoMode = VO_MODE_16MUX;
    for(i=0;i<u32WndNum;i++)
    {
        VoChn = i;
        VpssGrp = i;
        SAMPLE_COMM_VO_UnBindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);
    }
    SAMPLE_IT_StopHD(VoDev, &stVoPubAttr, enVoMode);
END_16_MixCap_0:
    SAMPLE_COMM_SYS_Exit();
    
    return s32Ret;
}


/******************************************************************************
* function :  VI:16*2cif; VO:HD0(HDMI  720P50), WBC to SD0(CVBS) video preview. 
******************************************************************************/
HI_S32 SAMPLE_IT_25fpsCif_25fpsQcif(HI_VOID)
{
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_16_2Cif;
    HI_U32 s32ViChnCnt = 16;
    HI_S32 s32VpssGrpCnt = 32;
    VB_CONF_S stVbConf;
    VPSS_GRP VpssGrp;
    VPSS_GRP_ATTR_S stGrpAttr;
    VPSS_CHN VpssChn_VoHD0 = VPSS_PRE0_CHN;
    VPSS_CHN_ATTR_S stVpssChnAttr;
    VO_DEV VoDev;
    VO_CHN VoChn;
    VO_PUB_ATTR_S stVoPubAttr; 
    SAMPLE_VO_MODE_E enVoMode, enPreVoMode;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    HI_S32 i;
    HI_S32 j;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    HI_CHAR ch;
    HI_U32 u32WndNum;
    VENC_GRP VencGrp;
    VENC_CHN VencChn;
    GROUP_FRAME_RATE_S stGrpFrmRate;
    VENC_CHN_ATTR_S stVencChnAttr;
    VPSS_SIZER_INFO_S stVpssSizerInfo;
    VPSS_FRAME_RATE_S stVpssFrameRate;

    /******************************************
     step  1: VB and system init. 
    ******************************************/
    memset(&stVbConf,0,sizeof(VB_CONF_S));
    stVbConf.u32MaxPoolCnt = 64;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(s_enNorm,\
                PIC_D1, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = 60;
    
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(s_enNorm,\
                PIC_2CIF, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt = 110;
    
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(s_enNorm,\
                PIC_CIF, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[2].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[2].u32BlkCnt = 64;

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(s_enNorm,\
                PIC_QCIF, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[3].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[3].u32BlkCnt = 32;

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_16_MixCap_0;
    }

    /******************************************
     step 2: start vo HD0(HDMI) 
    ******************************************/
    printf("start vo HD0.\n");
    VoDev = SAMPLE_VO_DEV_DHD0;
    u32WndNum = 16;
    enVoMode = VO_MODE_16MUX;
    if(VIDEO_ENCODING_MODE_PAL == s_enNorm)
    {
        stVoPubAttr.enIntfSync = VO_OUTPUT_1080P50;
    }
    else
    {
        stVoPubAttr.enIntfSync = VO_OUTPUT_1080P60;
    }
    stVoPubAttr.enIntfType = VO_INTF_HDMI | VO_INTF_VGA;
    stVoPubAttr.u32BgColor = 0x000000ff;
    stVoPubAttr.bDoubleFrame = HI_TRUE;
    s32Ret = SAMPLE_IT_StartHD(VoDev, &stVoPubAttr, enVoMode, s_enNorm, s_u32ViFrmRate);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start VO failed!\n");
        goto END_16_MixCap_1;
    }

    /******************************************
     step 3: start vpss, bind it to venc and vo
    ******************************************/    
    stGrpAttr.u32MaxW = D1_WIDTH;
    stGrpAttr.u32MaxH = s_u32D1Height;
    stGrpAttr.bDrEn = HI_FALSE;
    stGrpAttr.bDbEn = HI_FALSE;
    stGrpAttr.bIeEn = HI_TRUE;
    stGrpAttr.bNrEn = HI_TRUE;
    stGrpAttr.bHistEn = HI_TRUE;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;
    stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

    stVpssChnAttr.bFrameEn = HI_TRUE;
    stVpssChnAttr.bSpEn = HI_TRUE;
    for (i=0; i<4; i++)
    {
        stVpssChnAttr.stFrame.u32Width[i] = 2;
        stVpssChnAttr.stFrame.u32Color[i] = 0xff00;
    }

    for (i = 0; i < s32VpssGrpCnt; i++)
    {
        VpssGrp = i;
        s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stGrpAttr);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VPSS_CreateGrp failed!\n");
            goto END_16_MixCap_2;
        }

        if (VpssGrp < 16)
        {
            /* 0-15vpss grp for venc 16D1@6fps and 16CIF@6fps  */
            /* open pre-scale */
            s32Ret = SAMPLE_COMM_EnableVpssPreScale(i, s_stCifSize);
            if(HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("HI_MPI_VPSS_SetPreScale failed!\n");
                goto END_16_MixCap_2;
            }
            s32Ret |= HI_MPI_VPSS_SetChnAttr(VpssGrp, VPSS_PRE0_CHN, &stVpssChnAttr);
            s32Ret |= HI_MPI_VPSS_SetChnAttr(VpssGrp, VPSS_PRE1_CHN, &stVpssChnAttr);
            s32Ret |= HI_MPI_VPSS_EnableChn(VpssGrp, VPSS_PRE0_CHN);
            s32Ret |= HI_MPI_VPSS_EnableChn(VpssGrp, VPSS_PRE1_CHN);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Start Vpss chn failed!\n");
                goto END_16_MixCap_2;
            }

            /* start venc chn for CIF and bind it to VPSS_PRE0_CHN */
            VencGrp = i;
            VencChn = i;
            s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, PT_H264,\
                                           s_enNorm, PIC_CIF, SAMPLE_RC_VBR);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Start Venc failed!\n");
                goto END_16_MixCap_2;
            }
            s32Ret = SAMPLE_COMM_VENC_BindVpss(VencGrp, VpssGrp, VPSS_PRE0_CHN);

            /* start venc chn for QCIF and bind it to VPSS_PRE1_CHN */
            VencGrp = i + 16;
            VencChn = i + 16;
            s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, PT_H264,\
                                           s_enNorm, PIC_QCIF, SAMPLE_RC_VBR);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Start Venc failed!\n");
                goto END_16_MixCap_2;
            }
            s32Ret = SAMPLE_COMM_VENC_BindVpss(VencGrp, VpssGrp, VPSS_PRE1_CHN);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("SAMPLE_COMM_VENC_BindVpss failed!\n");
                goto END_16_MixCap_2;
            }
        }
        else
        {
            /* 16-31 vpss grp for vo HD */            
            /* open pre-scale */
            s32Ret = SAMPLE_COMM_EnableVpssPreScale(i, s_st2CifSize);
            if(HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("HI_MPI_VPSS_SetPreScale failed!\n");
                goto END_16_MixCap_2;
            }

            s32Ret |= HI_MPI_VPSS_SetChnAttr(VpssGrp, VPSS_PRE0_CHN, &stVpssChnAttr);
            s32Ret |= HI_MPI_VPSS_EnableChn(VpssGrp, VPSS_PRE0_CHN);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Start Vpss chn failed!\n");
                goto END_16_MixCap_2;
            }

            VoChn = VpssGrp - 16;
            s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev, VoChn, VpssGrp, VpssChn_VoHD0);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Start VO failed!\n");
                goto END_16_MixCap_2;
            }
        }
        
        s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VPSS_StartGrp failed!\n");
            goto END_16_MixCap_2;
        }
    }

    /******************************************
     step 4: bind vpss to vi
    ******************************************/
    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32ChnId = 0;
    for (j=0; j<s32ViChnCnt; j++)
    {    
        stSrcChn.s32ChnId = j;
        stDestChn.s32DevId = j;
        s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            goto END_16_MixCap_3;
        }
        
        stDestChn.s32DevId = j + 16;
        s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            goto END_16_MixCap_3;
        }
    }

    /******************************************
     step 5: start a thread to get venc stream 
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(32);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_16_MixCap_3;
    }
    
    /******************************************
     step 6: start vi dev & chn
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_Start(enViMode, s_enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_16_MixCap_4;
    }

    /******************************************
     step 7: start vo sd to spot 
    ******************************************/
    s32Ret = SAMPLE_IT_StartSdSpot(SAMPLE_VO_DEV_DSD0, s_enNorm, s32ViChnCnt);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start VoDev %d to spot failed!\n", SAMPLE_VO_DEV_DSD0);
        goto END_16_MixCap_4;
    }

    /******************************************
     step 8: HD0 switch mode 
    ******************************************/
    while(1)
    {
        enPreVoMode = enVoMode;
        printf("please choose preview mode, press 'q' to exit this sample.\n"); 
        printf("\t0) 4 preview\n");
        printf("\t1) 16 preview\n");
        printf("\tq) quit\n");
 
        ch = getchar();
        getchar();
        if ('0' == ch)
        {
            u32WndNum = 4;
            enVoMode = VO_MODE_4MUX;
        }
        else if ('1' == ch)
        {
            u32WndNum = 16;
            enVoMode = VO_MODE_16MUX;
        }
        else if ('q' == ch)
        {
            break;
        }
        else
        {
            SAMPLE_PRT("preview mode invaild! please try again.\n");
            continue;
        }
        
        if (enVoMode == enPreVoMode)
        {
            continue;
        }
        else if (VO_MODE_4MUX == enVoMode)
        {
            for(i = 0; i < 4; i++)
            {
                /* change to capture D1 */
                pthread_mutex_lock(&s_astViCapD1Status[i].stThrdLock);
                if (0 == s_astViCapD1Status[i].u32ReqCnt)
                {
                    s32Ret = SAMPLE_COMM_VI_ChangeDestSize(i, D1_WIDTH, s_u32D1Height);
                    if (HI_SUCCESS != s32Ret)
                    {
                        SAMPLE_PRT("SAMPLE_COMM_VI_ChangeDestSize VO failed!\n");
                        pthread_mutex_unlock(&s_astViCapD1Status[i].stThrdLock);
                        goto END_16_MixCap_4;
                    }
                }
                ++s_astViCapD1Status[i].u32ReqCnt;
                pthread_mutex_unlock(&s_astViCapD1Status[i].stThrdLock);

                s32Ret = SAMPLE_COMM_EnableVpssPreScale(i, s_stD1Size);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_PRT("SAMPLE_COMM_DisableVpssPreScale VO failed!\n");
                    goto END_16_MixCap_4;
                }
            }
        }
        else if (VO_MODE_16MUX == enVoMode)
        {
            for(i = 0; i < 16; i++)
            {
                /* try to change to capture 2cif */
                pthread_mutex_lock(&s_astViCapD1Status[i].stThrdLock);
                if (1 == s_astViCapD1Status[i].u32ReqCnt)
                {
                    s32Ret = SAMPLE_COMM_VI_ChangeDestSize(i, D1_WIDTH / 2, s_u32D1Height);
                    if (HI_SUCCESS != s32Ret)
                    {
                        SAMPLE_PRT("SAMPLE_COMM_VI_ChangeDestSize failed!\n");
                        pthread_mutex_unlock(&s_astViCapD1Status[i].stThrdLock);
                        goto END_16_MixCap_4;
                    }
                    --s_astViCapD1Status[i].u32ReqCnt;
                }
                else if (1 < s_astViCapD1Status[i].u32ReqCnt)
                {
                    --s_astViCapD1Status[i].u32ReqCnt;
                }
                else
                {
                    s32Ret = SAMPLE_COMM_VI_ChangeDestSize(i, D1_WIDTH / 2, s_u32D1Height);
                    if (HI_SUCCESS != s32Ret)
                    {
                        SAMPLE_PRT("SAMPLE_COMM_VI_ChangeDestSize failed!\n");
                        pthread_mutex_unlock(&s_astViCapD1Status[i].stThrdLock);
                        goto END_16_MixCap_4;
                    }
                }
                pthread_mutex_unlock(&s_astViCapD1Status[i].stThrdLock);

                s32Ret = SAMPLE_COMM_EnableVpssPreScale(i + 16, s_st2CifSize);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_PRT("SAMPLE_COMM_DisableVpssPreScale failed!\n");
                    goto END_16_MixCap_4;
                }
            }
        }
        
        SAMPLE_PRT("vo(%d) switch to %d mode\n", VoDev, u32WndNum);

        s32Ret= HI_MPI_VO_SetAttrBegin(VoDev);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start VO failed!\n");
            goto END_16_MixCap_4;
        }
        
        s32Ret = SAMPLE_COMM_VO_StopChn(VoDev, enPreVoMode);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start VO failed!\n");
            goto END_16_MixCap_4;
        }

        s32Ret = SAMPLE_COMM_VO_StartChn(VoDev, &stVoPubAttr, enVoMode);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start VO failed!\n");
            goto END_16_MixCap_4;
        }
        s32Ret= HI_MPI_VO_SetAttrEnd(VoDev);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start VO failed!\n");
            goto END_16_MixCap_4;
        }
    }

    /******************************************
     step 90: exit process
    ******************************************/
END_16_MixCap_4:
    SAMPLE_COMM_VI_Stop(enViMode);
    SAMPLE_IT_StopSdSpot(SAMPLE_VO_DEV_DSD0);
END_16_MixCap_3:
    SAMPLE_COMM_VENC_StopGetStream();
    SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_16_MixCap_2:
    for (i=0; i<32; i++)
    {
        VpssGrp = i;
        SAMPLE_COMM_VENC_UnBindVpss(i, VpssGrp, VPSS_LSTR_CHN);
        SAMPLE_COMM_VENC_Stop(i, i);
    }
    SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_16_MixCap_1:
    VoDev = SAMPLE_VO_DEV_DHD0;
    u32WndNum = 16;
    enVoMode = VO_MODE_16MUX;
    for(i=0;i<u32WndNum;i++)
    {
        VoChn = i;
        VpssGrp = i;
        SAMPLE_COMM_VO_UnBindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);
    }
    SAMPLE_IT_StopHD(VoDev, &stVoPubAttr, enVoMode);
END_16_MixCap_0:
    SAMPLE_COMM_SYS_Exit();
    
    return s32Ret;
}



/******************************************************************************
* function    : main()
* Description : video preview sample
******************************************************************************/
int main(int argc, char *argv[])
{
    HI_S32 s32Ret;

#if HICHIP == HI3520A_V100
    printf("Hi3520A chip don't support this sample.\n");
    return HI_SUCCESS;
#endif

    if ( (argc < 2) || (1 != strlen(argv[1])))
    {
        SAMPLE_IT_Usage(argv[0]);
        return HI_FAILURE;
    }

    signal(SIGINT, SAMPLE_IT_HandleSig);
    signal(SIGTERM, SAMPLE_IT_HandleSig);

    SAMPLE_IT_GlobalVarInit();
    
    switch (*argv[1])
    {
        case '0':/*  */
            s32Ret = SAMPLE_IT_6fpsD1_25fpsCif();
            break;
        case '1':/*  */
            s32Ret = SAMPLE_IT_6fpsD1_6fpsCif();
            break;
        case '2':/* VI:16*2cif; VO: HD0(HDMI,1080P ). */
            s32Ret = SAMPLE_IT_25fpsCif_25fpsQcif();
            break;
        default:
            printf("the index is invaild!\n");
            SAMPLE_IT_Usage(argv[0]);
            return HI_FAILURE;
    }

    if (HI_SUCCESS == s32Ret)
        printf("program exit normally!\n");
    else
        printf("program exit abnormally!\n");
    exit(s32Ret);
}

