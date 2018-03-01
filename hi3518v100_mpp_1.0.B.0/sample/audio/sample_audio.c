/******************************************************************************
  A simple program of Hisilicon HI3518 audio input/output/encoder/decoder implementation.
  Copyright (C), 2010-2012, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2012-7-3 Created
******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#include "sample_comm.h"
#include "acodec.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define SAMPLE_AUDIO_PTNUMPERFRM   320

static PAYLOAD_TYPE_E gs_enPayloadType = PT_ADPCMA;

static HI_BOOL gs_bMicIn = HI_FALSE;

static HI_BOOL gs_bAiAnr = HI_FALSE;
static HI_BOOL gs_bAioReSample = HI_FALSE;
static HI_BOOL gs_bUserGetMode = HI_FALSE;
static AUDIO_RESAMPLE_ATTR_S *gs_pstAiReSmpAttr = NULL;
static AUDIO_RESAMPLE_ATTR_S *gs_pstAoReSmpAttr = NULL;

/*0: AiVqe closed; 1: stAiVqeConfig work; 2: stAiVqe3Config work*/
static HI_U32 gs_u32AiVqeType = 0; 
static AI_VQE_CONFIG_S *gs_pstAiVqeConfig;
static AI_VQE3_CONFIG_S *gs_pstAiVqe3Config;

static HI_BOOL gs_bAoVqe3 = HI_FALSE;
static AO_VQE3_CONFIG_S *gs_pstAoVqe3Config;


#define SAMPLE_DBG(s32Ret)\
do{\
    printf("s32Ret=%#x,fuc:%s,line:%d\n", s32Ret, __FUNCTION__, __LINE__);\
}while(0)

/******************************************************************************
* function : PT Number to String
******************************************************************************/
static char* SAMPLE_AUDIO_Pt2Str(PAYLOAD_TYPE_E enType)
{
    if (PT_G711A == enType)  return "g711a";
    else if (PT_G711U == enType)  return "g711u";
    else if (PT_ADPCMA == enType)  return "adpcm";
    else if (PT_G726 == enType)  return "g726";
    else if (PT_LPCM == enType)  return "pcm";
    else return "data";
}

/******************************************************************************
* function : Open Aenc File
******************************************************************************/
static FILE * SAMPLE_AUDIO_OpenAencFile(AENC_CHN AeChn, PAYLOAD_TYPE_E enType)
{
    FILE *pfd;
    HI_CHAR aszFileName[128];

    /* create file for save stream*/
    sprintf(aszFileName, "audio_chn%d.%s", AeChn, SAMPLE_AUDIO_Pt2Str(enType));
    pfd = fopen(aszFileName, "w+");
    if (NULL == pfd)
    {
        printf("%s: open file %s failed\n", __FUNCTION__, aszFileName);
        return NULL;
    }
    printf("open stream file:\"%s\" for aenc ok\n", aszFileName);
    return pfd;
}

/******************************************************************************
* function : Open Adec File
******************************************************************************/
static FILE *SAMPLE_AUDIO_OpenAdecFile(ADEC_CHN AdChn, PAYLOAD_TYPE_E enType)
{
    FILE *pfd;
    HI_CHAR aszFileName[128];

    /* create file for save stream*/
    sprintf(aszFileName, "audio_chn%d.%s", AdChn, SAMPLE_AUDIO_Pt2Str(enType));
    pfd = fopen(aszFileName, "rb");
    if (NULL == pfd)
    {
        printf("%s: open file %s failed\n", __FUNCTION__, aszFileName);
        return NULL;
    }
    printf("open stream file:\"%s\" for adec ok\n", aszFileName);
    return pfd;
}


/******************************************************************************
* function : file -> ADec -> Ao
******************************************************************************/
HI_S32 SAMPLE_AUDIO_AdecAo(AIO_ATTR_S *pstAioAttr)
{
    HI_S32      s32Ret;
    AUDIO_DEV   AoDev = 0;
    AO_CHN      AoChn = 0;
    ADEC_CHN    AdChn = 0;
    FILE        *pfd = NULL;

    if (NULL == pstAioAttr)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "NULL pointer");
        return HI_FAILURE;
    }

    s32Ret = SAMPLE_COMM_AUDIO_CfgAcodec(pstAioAttr, gs_bMicIn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    s32Ret = SAMPLE_COMM_AUDIO_StartAdec(AdChn, gs_enPayloadType);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, AoChn, pstAioAttr, gs_pstAoReSmpAttr, NULL);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    s32Ret = SAMPLE_COMM_AUDIO_AoBindAdec(AoDev, AoChn, AdChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    pfd = SAMPLE_AUDIO_OpenAdecFile(AdChn, gs_enPayloadType);
    if (!pfd)
    {
        SAMPLE_DBG(HI_FAILURE);
        return HI_FAILURE;
    }
    s32Ret = SAMPLE_COMM_AUDIO_CreatTrdFileAdec(AdChn, pfd);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    printf("bind adec:%d to ao(%d,%d) ok \n", AdChn, AoDev, AoChn);

    printf("\nplease press twice ENTER to exit this sample\n");
    getchar();
    getchar();

    SAMPLE_COMM_AUDIO_DestoryTrdFileAdec(AdChn);
    SAMPLE_COMM_AUDIO_StopAo(AoDev, AoChn, gs_bAioReSample);
    SAMPLE_COMM_AUDIO_StopAdec(AdChn);
    SAMPLE_COMM_AUDIO_AoUnbindAdec(AoDev, AoChn, AdChn);

    return HI_SUCCESS;
}

/******************************************************************************
* function : Ai -> Aenc -> file
*                                -> Adec -> Ao
******************************************************************************/
HI_S32 SAMPLE_AUDIO_AiAenc(AIO_ATTR_S *pstAioAttr)
{
    HI_S32 i, s32Ret;
    AUDIO_DEV   AiDev = 0;
    AI_CHN      AiChn;
    AUDIO_DEV   AoDev = 0;
    AO_CHN      AoChn = 0;
    ADEC_CHN    AdChn = 0;
    HI_S32      s32AiChnCnt;
    HI_S32      s32AencChnCnt;
    AENC_CHN    AeChn;
    HI_BOOL     bSendAdec = HI_TRUE;
    FILE        *pfd = NULL;

    /* config ai aenc dev attr */
    if (NULL == pstAioAttr)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "NULL pointer");
        return HI_FAILURE;
    }

    /********************************************
      step 1: config audio codec
    ********************************************/
    s32Ret = SAMPLE_COMM_AUDIO_CfgAcodec(pstAioAttr, gs_bMicIn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    /********************************************
      step 2: start Ai
    ********************************************/
    s32AiChnCnt = pstAioAttr->u32ChnCnt;
    s32AencChnCnt = 1;//s32AiChnCnt;
    s32Ret = SAMPLE_COMM_AUDIO_StartAi(AiDev, s32AiChnCnt, pstAioAttr, gs_bAiAnr, gs_pstAiReSmpAttr, NULL, NULL);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    /********************************************
      step 3: start Aenc
    ********************************************/
    s32Ret = SAMPLE_COMM_AUDIO_StartAenc(s32AencChnCnt, gs_enPayloadType);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    /********************************************
      step 4: Aenc bind Ai Chn
    ********************************************/
    for (i=0; i<s32AencChnCnt; i++)
    {
        AeChn = i;
        AiChn = 0;

        if (HI_TRUE == gs_bUserGetMode)
        {
            s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAiAenc(AiDev, AiChn, AeChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_DBG(s32Ret);
                return HI_FAILURE;
            }
        }
        else
        {
            s32Ret = SAMPLE_COMM_AUDIO_AencBindAi(AiDev, AiChn, AeChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_DBG(s32Ret);
                return s32Ret;
            }
        }
        printf("Ai(%d,%d) bind to AencChn:%d ok!\n",AiDev , AiChn, AeChn);
    }

    /********************************************
      step 5: start Adec & Ao. ( if you want )
    ********************************************/
    if (HI_TRUE == bSendAdec)
    {
        s32Ret = SAMPLE_COMM_AUDIO_StartAdec(AdChn, gs_enPayloadType);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
        }

        s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, AoChn, pstAioAttr, gs_pstAoReSmpAttr, NULL);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
        }

        pfd = SAMPLE_AUDIO_OpenAencFile(AdChn, gs_enPayloadType);
        if (!pfd)
        {
            SAMPLE_DBG(HI_FAILURE);
            return HI_FAILURE;
        }
        s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAencAdec(AeChn, AdChn, pfd);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
        }

        s32Ret = SAMPLE_COMM_AUDIO_AoBindAdec(AoDev, AoChn, AdChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
        }

        printf("bind adec:%d to ao(%d,%d) ok \n", AdChn, AoDev, AoChn);
    }


    printf("\nplease press twice ENTER to exit this sample\n");
    getchar();
    getchar();

    /********************************************
      step 6: exit the process
    ********************************************/
    if (HI_TRUE == bSendAdec)
    {
        SAMPLE_COMM_AUDIO_DestoryTrdAencAdec(AdChn);
        SAMPLE_COMM_AUDIO_StopAo(AoDev, AoChn, gs_bAioReSample);
        SAMPLE_COMM_AUDIO_StopAdec(AdChn);
        SAMPLE_COMM_AUDIO_AoUnbindAdec(AoDev, AoChn, AdChn);
    }

    for (i=0; i<s32AencChnCnt; i++)
    {
        AeChn = i;
        AiChn = i;

        if (HI_TRUE == gs_bUserGetMode)
        {
            SAMPLE_COMM_AUDIO_DestoryTrdAi(AiDev, AiChn);
        }
        else
        {
            SAMPLE_COMM_AUDIO_AencUnbindAi(AiDev, AiChn, AeChn);
        }
    }

    SAMPLE_COMM_AUDIO_StopAenc(s32AencChnCnt);
    SAMPLE_COMM_AUDIO_StopAi(AiDev, s32AiChnCnt, gs_bAiAnr, gs_bAioReSample);

    return HI_SUCCESS;
}

/******************************************************************************
* function : Ai -> Ao
******************************************************************************/
HI_S32 SAMPLE_AUDIO_AiAo(AIO_ATTR_S *pstAioAttr)
{
    HI_S32 s32Ret, s32AiChnCnt;
    AUDIO_DEV   AiDev = 0;
    AI_CHN      AiChn = 0;
    AUDIO_DEV   AoDev = 0;
    AO_CHN      AoChn = 0;

    /* config aio dev attr */
    if (NULL == pstAioAttr)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "NULL pointer");
        return HI_FAILURE;
    }

    /* config audio codec */
    s32Ret = SAMPLE_COMM_AUDIO_CfgAcodec(pstAioAttr, gs_bMicIn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    /* enable AI channle */
    s32AiChnCnt = pstAioAttr->u32ChnCnt;
    s32Ret = SAMPLE_COMM_AUDIO_StartAi(AiDev, s32AiChnCnt, pstAioAttr, gs_bAiAnr, gs_pstAiReSmpAttr, NULL, NULL);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    /* enable AO channle */
    pstAioAttr->u32ChnCnt = 2;
    s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, AoChn, pstAioAttr, gs_pstAoReSmpAttr, NULL);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }
    /* bind AI to AO channle */
    if (HI_TRUE == gs_bUserGetMode)
    {
        s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAiAo(AiDev, AiChn, AoDev, AoChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
        }
    }
    else
    {
        s32Ret = SAMPLE_COMM_AUDIO_AoBindAi(AiDev, AiChn, AoDev, AoChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
        }
    }
    printf("ai(%d,%d) bind to ao(%d,%d) ok\n", AiDev, AiChn, AoDev, AoChn);

    printf("\nplease press twice ENTER to exit this sample\n");
    getchar();
    getchar();

    if (HI_TRUE == gs_bUserGetMode)
    {
        SAMPLE_COMM_AUDIO_DestoryTrdAi(AiDev, AiChn);
    }
    else
    {
        SAMPLE_COMM_AUDIO_AoUnbindAi(AiDev, AiChn, AoDev, AoChn);
    }
    SAMPLE_COMM_AUDIO_StopAi(AiDev, s32AiChnCnt, gs_bAiAnr, gs_bAioReSample);
    SAMPLE_COMM_AUDIO_StopAo(AoDev, AoChn, gs_bAioReSample);
    return HI_SUCCESS;
}

/******************************************************************************
* function : Ai -> Ao(VQE process)
******************************************************************************/
HI_S32 SAMPLE_AUDIO_AiAo_VQE(AIO_ATTR_S *pstAioAttr)
{
    HI_S32 s32Ret, s32AiChnCnt;
    AI_VQE_CONFIG_S stAiVqeCfg;
    AI_VQE3_CONFIG_S stAiVqe3Cfg;
    AO_VQE3_CONFIG_S stAoVqe3Cfg;
    AUDIO_DEV   AiDev = 0;
    AI_CHN      AiChn = 0;
    AUDIO_DEV   AoDev = 0;
    AO_CHN      AoChn = 0;

    gs_bUserGetMode = HI_TRUE;

    gs_u32AiVqeType = 2;
    gs_pstAiVqeConfig = NULL;
    gs_pstAiVqe3Config = NULL;
    gs_pstAoVqe3Config = NULL;
    gs_bAoVqe3 = HI_FALSE;

    if (1 == gs_u32AiVqeType)
    {
        gs_pstAiVqeConfig = &stAiVqeCfg;
            
        stAiVqeCfg.bAecOpen = HI_FALSE;
        stAiVqeCfg.bAlcOpen = HI_FALSE;
        stAiVqeCfg.bAnrOpen = HI_TRUE;

        stAiVqeCfg.s32FrameSample = pstAioAttr->u32PtNumPerFrm;
        stAiVqeCfg.s32SampleRate = pstAioAttr->enSamplerate;
        stAiVqeCfg.stAecCfg.enAecMode = AUDIO_AEC_MODE_RECEIVER;
        stAiVqeCfg.stAecCfg.s32Reserved = 0;
        stAiVqeCfg.stAlcCfg.s32MaxLev = -4;
        stAiVqeCfg.stAlcCfg.s32MinLev = -16;
        stAiVqeCfg.stAlcCfg.u32MaxGain = 12;
        stAiVqeCfg.stAnrCfg.s32Reserved = 0;
   
    }
    else if (2 == gs_u32AiVqeType)
    {
        gs_pstAiVqe3Config = &stAiVqe3Cfg;
        
        stAiVqe3Cfg.bAecOpen = HI_TRUE;
        stAiVqe3Cfg.bAgcOpen= HI_FALSE;
        stAiVqe3Cfg.bAnrOpen = HI_FALSE;
        stAiVqe3Cfg.bEqOpen = HI_FALSE;
        stAiVqe3Cfg.bHpfOpen = HI_FALSE;

        stAiVqe3Cfg.enWorkstate = VQE3_WORKSTATE_COMMON;
        stAiVqe3Cfg.s32FrameSample = pstAioAttr->u32PtNumPerFrm;
        stAiVqe3Cfg.s32InSampleRate = pstAioAttr->enSamplerate;
        stAiVqe3Cfg.s32WorkSampleRate = AUDIO_SAMPLE_RATE_8000;

        stAiVqe3Cfg.stAecCfg.bUsrMode = HI_FALSE;
    	stAiVqe3Cfg.stAecCfg.s8CngMode = AUDIO_VQE3_AEC_MODE_CLOSE;
        stAiVqe3Cfg.stAecCfg.s32Reserved= 0;

        stAiVqe3Cfg.stAgcCfg.bUsrMode = HI_FALSE;
    	stAiVqe3Cfg.stAgcCfg.s16NoiseSupSwitch= 0;
    	stAiVqe3Cfg.stAgcCfg.s8AdjustSpeed = 0;
    	stAiVqe3Cfg.stAgcCfg.s8ImproveSNR = 0;
        stAiVqe3Cfg.stAgcCfg.s8MaxGain = 0;
        stAiVqe3Cfg.stAgcCfg.s8NoiseFloor = -50;
        stAiVqe3Cfg.stAgcCfg.s8OutputMode = 0;
        stAiVqe3Cfg.stAgcCfg.s8TargetLevel = -40;
        stAiVqe3Cfg.stAgcCfg.s8UseHighPassFilt = 0;

        stAiVqe3Cfg.stAnrCfg.bUsrMode = HI_FALSE;
        stAiVqe3Cfg.stAnrCfg.s16NrIntensity = 0;
        stAiVqe3Cfg.stAnrCfg.s16NoiseDbThr = 30;
        stAiVqe3Cfg.stAnrCfg.s8SpProSwitch = 0;
    	stAiVqe3Cfg.stAnrCfg.s32Reserved= 0;

        stAiVqe3Cfg.stHpfCfg.bUsrMode = HI_FALSE;
        stAiVqe3Cfg.stHpfCfg.enHpfFreq = AUDIO_HPF_FREQ_80;

        memset(&stAiVqe3Cfg.stEqCfg.s8GaindB, 0, VQE3_EQ_BAND_NUM);
        stAiVqe3Cfg.stEqCfg.s8GaindB[0] = 20;
        stAiVqe3Cfg.stEqCfg.s32Reserved = 0;
        
    }

    if (gs_bAoVqe3)
    {
        gs_pstAoVqe3Config = &stAoVqe3Cfg;
        
        stAoVqe3Cfg.bAgcOpen = HI_TRUE;
    	stAoVqe3Cfg.bAnrOpen = HI_TRUE;
        stAoVqe3Cfg.bHpfOpen = HI_FALSE;
        stAoVqe3Cfg.bEqOpen = HI_FALSE;
        
    	stAoVqe3Cfg.s32FrameSample = pstAioAttr->u32PtNumPerFrm;
        stAoVqe3Cfg.enWorkstate = VQE3_WORKSTATE_COMMON;
    	stAoVqe3Cfg.s32WorkSampleRate = AUDIO_SAMPLE_RATE_8000;
        stAoVqe3Cfg.s32InSampleRate = pstAioAttr->enSamplerate;

        stAoVqe3Cfg.stAgcCfg.bUsrMode = HI_FALSE;
    	stAoVqe3Cfg.stAgcCfg.s16NoiseSupSwitch= 0;
    	stAoVqe3Cfg.stAgcCfg.s8AdjustSpeed = 0;
    	stAoVqe3Cfg.stAgcCfg.s8ImproveSNR = 0;
        stAoVqe3Cfg.stAgcCfg.s8MaxGain = 0;
        stAoVqe3Cfg.stAgcCfg.s8NoiseFloor = -50;
        stAoVqe3Cfg.stAgcCfg.s8OutputMode = 0;
        stAoVqe3Cfg.stAgcCfg.s8TargetLevel = -40;
        stAoVqe3Cfg.stAgcCfg.s8UseHighPassFilt = 0;

        stAoVqe3Cfg.stAnrCfg.bUsrMode = HI_FALSE;
        stAoVqe3Cfg.stAnrCfg.s16NrIntensity = 0;
        stAoVqe3Cfg.stAnrCfg.s16NoiseDbThr = 30;
        stAoVqe3Cfg.stAnrCfg.s8SpProSwitch = 0;
    	stAoVqe3Cfg.stAnrCfg.s32Reserved= 0;

        stAoVqe3Cfg.stHpfCfg.bUsrMode = HI_FALSE;
        stAoVqe3Cfg.stHpfCfg.enHpfFreq = AUDIO_HPF_FREQ_80;

        memset(&stAoVqe3Cfg.stEqCfg.s8GaindB, 0, VQE3_EQ_BAND_NUM);
        stAoVqe3Cfg.stEqCfg.s32Reserved = 0;
    }

    /* config aio dev attr */
    if (NULL == pstAioAttr)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "NULL pointer");
        return HI_FAILURE;
    }

    /* config audio codec */
    s32Ret = SAMPLE_COMM_AUDIO_CfgAcodec(pstAioAttr, gs_bMicIn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    /* enable AI channle */
    s32AiChnCnt = pstAioAttr->u32ChnCnt;
    s32Ret = SAMPLE_COMM_AUDIO_StartAi(AiDev, s32AiChnCnt, pstAioAttr, gs_bAiAnr, gs_pstAiReSmpAttr, gs_pstAiVqeConfig, gs_pstAiVqe3Config);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    /* enable AO channle */
    pstAioAttr->u32ChnCnt = 2;
    s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, AoChn, pstAioAttr, gs_pstAoReSmpAttr, gs_pstAoVqe3Config);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }
    
    /* bind AI to AO channle */
    if (HI_TRUE == gs_bUserGetMode)
    {
        s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAiAo(AiDev, AiChn, AoDev, AoChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
        }
    }
    else
    {
        s32Ret = SAMPLE_COMM_AUDIO_AoBindAi(AiDev, AiChn, AoDev, AoChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
        }
    }
    printf("ai(%d,%d) bind to ao(%d,%d) ok\n", AiDev, AiChn, AoDev, AoChn);

    printf("\nplease press twice ENTER to exit this sample\n");
    getchar();
    getchar();

    if (HI_TRUE == gs_bUserGetMode)
    {
        SAMPLE_COMM_AUDIO_DestoryTrdAi(AiDev, AiChn);
    }
    else
    {
        SAMPLE_COMM_AUDIO_AoUnbindAi(AiDev, AiChn, AoDev, AoChn);
    }
    SAMPLE_COMM_AUDIO_StopAi(AiDev, s32AiChnCnt, gs_bAiAnr, gs_bAioReSample);
    SAMPLE_COMM_AUDIO_StopAo(AoDev, AoChn, gs_bAioReSample);
    return HI_SUCCESS;
}


HI_VOID SAMPLE_AUDIO_Usage(void)
{
    printf("\n/************************************/\n");
    printf("press sample command as follows!\n");
    printf("1:  send audio frame to AENC channel form AI, save them\n");
    printf("2:  read audio stream from file,decode and send AO\n");
    printf("3:  start AI to AO loop\n");
    printf("4:  start AI to AO (VQE process) loop\n");
    printf("q:  quit whole audio sample\n\n");
    printf("sample command:");
}

/******************************************************************************
* function : to process abnormal case
******************************************************************************/
void SAMPLE_AUDIO_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo)
    {
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    }

    exit(0);
}

/******************************************************************************
* function : main
******************************************************************************/
HI_S32 main(int argc, char *argv[])
{
    char ch;
    HI_S32 s32Ret= HI_SUCCESS;
    VB_CONF_S stVbConf;
    AIO_ATTR_S stAioAttr;

    /* arg 1 is audio payload type */
    if (argc >= 2)
    {
        gs_enPayloadType = atoi(argv[1]);

        if (gs_enPayloadType != PT_G711A && gs_enPayloadType != PT_G711U &&\
            gs_enPayloadType != PT_ADPCMA && gs_enPayloadType != PT_G726 &&\
            gs_enPayloadType != PT_LPCM)
        {
            printf("payload type invalid!\n");
            printf("\nargv[1]:%d is payload type ID, suport such type:\n", gs_enPayloadType);
            printf("%d:g711a, %d:g711u, %d:adpcm, %d:g726, %d:lpcm\n",
            PT_G711A, PT_G711U, PT_ADPCMA, PT_G726, PT_LPCM);
            return HI_FAILURE;
        }
    }

    /* init stAio. all of cases will use it */
    stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_8000;
    stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag = 1;
    stAioAttr.u32FrmNum = 30;
    stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
    stAioAttr.u32ChnCnt = 2;
    stAioAttr.u32ClkSel = 1;

    /* config ao resample attr if needed */
    if (HI_TRUE == gs_bAioReSample)
    {
        AUDIO_RESAMPLE_ATTR_S stAiReSampleAttr;
        AUDIO_RESAMPLE_ATTR_S stAoReSampleAttr;

        stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_32000;
        stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM * 4;

        /* ai 32k -> 8k */
        stAiReSampleAttr.u32InPointNum = SAMPLE_AUDIO_PTNUMPERFRM * 4;
        stAiReSampleAttr.enInSampleRate = AUDIO_SAMPLE_RATE_32000;
        stAiReSampleAttr.enReSampleType = AUDIO_RESAMPLE_4X1;
        gs_pstAiReSmpAttr = &stAiReSampleAttr;

        /* ao 8k -> 32k */
        stAoReSampleAttr.u32InPointNum = SAMPLE_AUDIO_PTNUMPERFRM;
        stAoReSampleAttr.enInSampleRate = AUDIO_SAMPLE_RATE_8000;
        stAoReSampleAttr.enReSampleType = AUDIO_RESAMPLE_1X4;
        gs_pstAoReSmpAttr = &stAoReSampleAttr;
    }
    else
    {
        gs_pstAiReSmpAttr = NULL;
        gs_pstAoReSmpAttr = NULL;
    }

    /* resample and anr should be user get mode */
    gs_bUserGetMode = (HI_TRUE == gs_bAioReSample || HI_TRUE == gs_bAiAnr) ? HI_TRUE : HI_FALSE;

    signal(SIGINT, SAMPLE_AUDIO_HandleSig);
    signal(SIGTERM, SAMPLE_AUDIO_HandleSig);

    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s: system init failed with %d!\n", __FUNCTION__, s32Ret);
        return HI_FAILURE;
    }

    SAMPLE_AUDIO_Usage();

    while ((ch = getchar()) != 'q')
    {
        switch (ch)
        {
            case '1':
            {
                s32Ret = SAMPLE_AUDIO_AiAenc(&stAioAttr);/* send audio frame to AENC channel form AI, save them*/
                break;
            }
            case '2':
            {
                s32Ret = SAMPLE_AUDIO_AdecAo(&stAioAttr);/* read audio stream from file,decode and send AO*/
                break;
            }
            case '3':
            {
                s32Ret = SAMPLE_AUDIO_AiAo(&stAioAttr);/* AI to AO*/
                break;
            }
            case '4':
            {
                s32Ret = SAMPLE_AUDIO_AiAo_VQE(&stAioAttr);/* AI to AO*/
                break;
            }
            default:
            {
                SAMPLE_AUDIO_Usage();
                break;
            }
        }
        if (s32Ret != HI_SUCCESS)
        {
            break;
        }
    }

    SAMPLE_COMM_SYS_Exit();

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
