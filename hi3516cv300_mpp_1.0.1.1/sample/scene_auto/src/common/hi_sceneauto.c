#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <stdlib.h>


//#include "iniparser.h"
#include "hi_sceneauto_comm.h"
#include "hi_sceneauto_define_ext.h"
#include "hi_sceneauto_ext.h"
#include "hi_common.h"
#include "hi_comm_isp.h"
#include "hi_comm_vpss.h"
#include "mpi_vpss.h"
#include "hi_comm_venc.h"
#include "mpi_vi.h"
#include "mpi_ae.h"
#include "mpi_awb.h"
#include "mpi_isp.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

HI_BOOL g_bSceneautoInit = HI_FALSE;
HI_BOOL g_bSceneautoStart = HI_FALSE;
HI_BOOL g_bNormalThreadFlag = HI_FALSE;
HI_BOOL g_bSpecialThreadFlag = HI_FALSE;
HI_BOOL g_b3DnrThreadFlag = HI_FALSE;
HI_BOOL g_bSceneautoFirstStart = HI_TRUE;
SCENEAUTO_INIPARA_S g_stINIPara;
SCENEAUTO_PREVIOUSPARA_S g_stPreviousPara;
SCENEAUTO_SEPCIAL_SCENE_E g_eSpecialScene;	
SCENEAUTO_SEPCIAL_SCENE_E g_eLastSpecialScene = SCENEAUTO_SPECIAL_SCENE_BUTT;
ISP_WDR_MODE_S g_stIspWdrMode;

pthread_mutex_t g_stSceneautoLock = PTHREAD_MUTEX_INITIALIZER;
pthread_t g_pthSceneAutoNormal;
pthread_t g_pthSceneAutoSpecial;
pthread_t g_pthSceneAuto3Dnr;

#ifdef _MSC_VER
#define MUTEX_INIT_LOCK(mutex) InitializeCriticalSection(&mutex)
#define MUTEX_LOCK(mutex) EnterCriticalSection(&mutex)
#define MUTEX_UNLOCK(mutex) LeaveCriticalSection(&mutex)
#define MUTEX_DESTROY(mutex) DeleteCriticalSection(&mutex)
#else
#define MUTEX_INIT_LOCK(mutex) \
do \
{ \
(void)pthread_mutex_init(&mutex, NULL); \
}while(0)
#define MUTEX_LOCK(mutex) \
do \
{ \
(void)pthread_mutex_lock(&mutex); \
}while(0)
#define MUTEX_UNLOCK(mutex) \
do \
{ \
(void)pthread_mutex_unlock(&mutex); \
}while(0)
#define MUTEX_DESTROY(mutex) \
do \
{ \
(void)pthread_mutex_destroy(&mutex); \
}while(0)
#endif

#define CHECK_SCENEAUTO_INIT()\
    do{\
        MUTEX_LOCK(g_stSceneautoLock);\
        if (HI_FALSE == g_bSceneautoInit)\
        {\
            printf("func:%s,line:%d, please init sceneauto first!\n",__FUNCTION__,__LINE__);\
            MUTEX_UNLOCK(g_stSceneautoLock);\
            return HI_FAILURE;\
        }\
        MUTEX_UNLOCK(g_stSceneautoLock);\
    }while(0);

//extern HI_S32 HI_MPI_VPSS_SetNRV3Param(VPSS_GRP VpssGrp, VPSS_GRP_VPPNRBEX_S *pstVpssNrParam);
//extern HI_S32 HI_MPI_VPSS_GetNRV3Param(VPSS_GRP VpssGrp, VPSS_GRP_VPPNRBEX_S *pstVpssNrParam);


static __inline int iClip2(int x, int b) {{ if (x < 0) x = 0; };{ if (x > b) x = b; }; return x; }
static __inline int iMin2(int a, int b) {{ if (a > b) a = b; }; return a; }
static __inline int iMax2(int a, int b) {{ if (a > b) b = a; }; return b; }  

static __inline HI_U32 MapISO(HI_U32 iso)
{
  HI_U32   j,  i = (iso >= 200);
  
  if (iso < 72) return iMax2(iso, -3); 
  
  i += ( (iso >= (200 << 1)) + (iso >= (400 << 1)) + (iso >= (400 << 2)) + (iso >= (400 << 3)) + (iso >= (400 << 4)) );
  i += ( (iso >= (400 << 5)) + (iso >= (400 << 6)) + (iso >= (400 << 7)) + (iso >= (400 << 8)) + (iso >= (400 << 9)) );
  i += ( (iso >= (400 << 10))+ (iso >= (400 << 11))+ (iso >= (400 << 12))+ (iso >= (400 << 13))+ (iso >= (400 << 14))) ;
  j  = ( (iso >  (112 << i)) + (iso >  (125 << i)) + (iso >  (141 << i)) + (iso >  (158 << i)) + (iso >  (178 << i)) );
  
  return (i * 6 + j + (iso >= 80) + (iso >= 90) + (iso >= 100) - 3);  
}

HI_U8 Interpulate(HI_U32 u32Mid,HI_U32 u32Left, HI_U8 u8LValue, HI_U32 u32Right, HI_U8 u8RValue)
{
    HI_U8 u8Value;
    HI_U32 k;

    if (u32Mid <= u32Left)
    {
        u8Value = u8LValue;
        return u8Value;
    }
    if (u32Mid >= u32Right)
    {
        u8Value = u8RValue;
        return u8Value;
    }
    
    k = (u32Right - u32Left); 
    u8Value = (((u32Right - u32Mid) * u8LValue + (u32Mid - u32Left) * u8RValue + (k >> 1))/ k);

    return u8Value;
}


HI_U64 GetMG()
{
	//ISP_STATISTICS_S stStat;
	ISP_STATISTICS_S *pstStat = NULL;

	pstStat = (ISP_STATISTICS_S *)malloc(sizeof(ISP_STATISTICS_S));
	if(NULL ==pstStat)
	{
       return 0;
	}
	
	int i,j,res,resmax;
	ISP_DEV IspDev = 0;
	resmax  = 0;
	HI_MPI_ISP_GetStatistics(IspDev, pstStat);
	for(i=0;i<15;i++)
	{
		for(j = 0;j<17;j++)
		{
			res = ((pstStat->stMGStat.au16ZoneAvg[i][j][1]<<16)+(pstStat->stMGStat.au16ZoneAvg[i][j][2]<<16))/DIV_0_TO_1((pstStat->stAEStat.au16ZoneAvg[i][j][1]+pstStat->stAEStat.au16ZoneAvg[i][j][2]));
			if(res > resmax)
			{
				resmax = res;
			}
		}
	}
	
	free(pstStat);
	
	return (HI_U64)resmax*resmax;
	
}
HI_VOID Interpolate(SCENEAUTO_INIPARAM_NRS_S *pst3dnrcfg, HI_U32 u32Mid, 
                                const SCENEAUTO_INIPARAM_NRS_S *pstL3dnrcfg, HI_U32 u32Left,  
                                const SCENEAUTO_INIPARAM_NRS_S *pstR3dnrcfg, HI_U32 u32Right)
{
  int   k, left, right, i = ((u32Mid > 3) ? MapISO(u32Mid) : iMin2(95,-u32Mid));  
  left  = ((u32Left  > 3) ? MapISO(u32Left) : iMin2(95,-u32Left) ); if (i <= left)  { *pst3dnrcfg = *pstL3dnrcfg; return; }
  right = ((u32Right > 3) ? MapISO(u32Right): iMin2(95,-u32Right)); if (i >= right) { *pst3dnrcfg = *pstR3dnrcfg; return; }
  k = (right - left); *pst3dnrcfg = *(( (i+((k * 3) >> 2)) < right ) ? pstL3dnrcfg : pstR3dnrcfg);
  pst3dnrcfg->s32SFC = ( ((right - i) * pstL3dnrcfg->s32SFC + (i - left) * pstR3dnrcfg->s32SFC + (k >> 1)) / k ); 
  pst3dnrcfg->s32TFC = ( ((right - i) * pstL3dnrcfg->s32TFC + (i - left) * pstR3dnrcfg->s32TFC + (k >> 1)) / k ); 
  pst3dnrcfg->s32TPC = ( ((right - i) * pstL3dnrcfg->s32TPC + (i - left) * pstR3dnrcfg->s32TPC + (k >> 1)) / k ); 
  pst3dnrcfg->s32TRC = ( ((right - i) * pstL3dnrcfg->s32TRC + (i - left) * pstR3dnrcfg->s32TRC + (k >> 1)) / k ); 
  pst3dnrcfg->s32IES0 = ( ((right - i) * pstL3dnrcfg->s32IES0 + (i - left) * pstR3dnrcfg->s32IES0 + (k >> 1)) / k ); 
  pst3dnrcfg->s32SBS0 = ( ((right - i) * pstL3dnrcfg->s32SBS0 + (i - left) * pstR3dnrcfg->s32SBS0 + (k >> 1)) / k ); 
  pst3dnrcfg->s32SBS1 = ( ((right - i) * pstL3dnrcfg->s32SBS1 + (i - left) * pstR3dnrcfg->s32SBS1 + (k >> 1)) / k ); 
  pst3dnrcfg->s32SBS2 = ( ((right - i) * pstL3dnrcfg->s32SBS2 + (i - left) * pstR3dnrcfg->s32SBS2 + (k >> 1)) / k ); 
  pst3dnrcfg->s32SBS3 = ( ((right - i) * pstL3dnrcfg->s32SBS3 + (i - left) * pstR3dnrcfg->s32SBS3 + (k >> 1)) / k ); 
  pst3dnrcfg->s32SDS0 = ( ((right - i) * pstL3dnrcfg->s32SDS0 + (i - left) * pstR3dnrcfg->s32SDS0 + (k >> 1)) / k ); 
  pst3dnrcfg->s32SDS1 = ( ((right - i) * pstL3dnrcfg->s32SDS1 + (i - left) * pstR3dnrcfg->s32SDS1 + (k >> 1)) / k ); 
  pst3dnrcfg->s32SDS2 = ( ((right - i) * pstL3dnrcfg->s32SDS2 + (i - left) * pstR3dnrcfg->s32SDS2 + (k >> 1)) / k ); 
  pst3dnrcfg->s32SDS3 = ( ((right - i) * pstL3dnrcfg->s32SDS3 + (i - left) * pstR3dnrcfg->s32SDS3 + (k >> 1)) / k ); 
  pst3dnrcfg->s32STH0 = ( ((right - i) * pstL3dnrcfg->s32STH0 + (i - left) * pstR3dnrcfg->s32STH0 + (k >> 1)) / k ); 
  pst3dnrcfg->s32STH1 = ( ((right - i) * pstL3dnrcfg->s32STH1 + (i - left) * pstR3dnrcfg->s32STH1 + (k >> 1)) / k ); 
  pst3dnrcfg->s32STH2 = ( ((right - i) * pstL3dnrcfg->s32STH2 + (i - left) * pstR3dnrcfg->s32STH2 + (k >> 1)) / k ); 
  pst3dnrcfg->s32STH3 = ( ((right - i) * pstL3dnrcfg->s32STH3 + (i - left) * pstR3dnrcfg->s32STH3 + (k >> 1)) / k ); 

  pst3dnrcfg->s32MATH1 = ( ((right - i) * pstL3dnrcfg->s32MATH1 + (i - left) * pstR3dnrcfg->s32MATH1 + (k >> 1)) / k ); 
  pst3dnrcfg->s32MATH2 = ( ((right - i) * pstL3dnrcfg->s32MATH2 + (i - left) * pstR3dnrcfg->s32MATH2 + (k >> 1)) / k ); 
  pst3dnrcfg->s32MDP = ( ((right - i) * pstL3dnrcfg->s32MDP + (i - left) * pstR3dnrcfg->s32MDP + (k >> 1)) / k ); 
  pst3dnrcfg->s32TFS1 = ( ((right - i) * pstL3dnrcfg->s32TFS1 + (i - left) * pstR3dnrcfg->s32TFS1 + (k >> 1)) / k ); 
  pst3dnrcfg->s32TFS2 = ( ((right - i) * pstL3dnrcfg->s32TFS2 + (i - left) * pstR3dnrcfg->s32TFS2 + (k >> 1)) / k ); 
  
  pst3dnrcfg->s32MDDZ1 = ( ((right - i) * pstL3dnrcfg->s32MDDZ1 + (i - left) * pstR3dnrcfg->s32MDDZ1 + (k >> 1)) / k ); 
  pst3dnrcfg->s32MDDZ2 = ( ((right - i) * pstL3dnrcfg->s32MDDZ2 + (i - left) * pstR3dnrcfg->s32MDDZ2 + (k >> 1)) / k );
  pst3dnrcfg->s32Pro3 = ( ((right - i) * pstL3dnrcfg->s32Pro3 + (i - left) * pstR3dnrcfg->s32Pro3 + (k >> 1)) / k ); 

 
} 


static __inline HI_U32 GetLevelLtoH(HI_U32 u32Value, HI_U32 u32Level, HI_U32 u32Count, HI_U32 *pu32Thresh)
{
    for (u32Level = 0; u32Level < u32Count; u32Level++)
    {
        if (u32Value <= pu32Thresh[u32Level])
        {
            break;
        }
    }
    if (u32Level == u32Count)
    {
        u32Level = u32Count - 1;
    }

    return u32Level;
}

static __inline HI_U32 GetLevelHtoL(HI_U32 u32Value, HI_U32 u32Level, HI_U32 u32Count, HI_U32 *pu32Thresh)
{
    for (u32Level = u32Count; u32Level > 0; u32Level--)
    {
        if (u32Value > pu32Thresh[u32Level - 1])
        {
            break;
        }

    }
    if (u32Level > 0)
    {
        u32Level = u32Level - 1;
    }

    return u32Level;    
}


HI_S32 Sceneauto_SetAERealatedBit(HI_S32 s32IspDev, HI_U32 u32Bitrate, HI_U32 u32LastBitrate)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_DEV IspDev = s32IspDev;
    HI_U32 u32BitrateLevel = 0;
    HI_U32 u32LastBitrateLevel = 0;
    HI_U32 u32Count = g_stINIPara.stIniAE.u32BitrateCount;
    HI_U32 *pu32Thresh = g_stINIPara.stIniAE.pu32BitrateThresh;
    ISP_EXPOSURE_ATTR_S stIspExposureAttr;

    u32BitrateLevel = GetLevelLtoH(u32Bitrate, u32BitrateLevel, u32Count, pu32Thresh);
    u32LastBitrateLevel = GetLevelLtoH(u32LastBitrate, u32LastBitrateLevel, u32Count, pu32Thresh);

    if (u32BitrateLevel != u32LastBitrateLevel)
    {
        s32Ret = HI_MPI_ISP_GetExposureAttr(IspDev, &stIspExposureAttr);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_ISP_GetExposureAttr failed\n");
            return HI_FAILURE;
        }
        stIspExposureAttr.stAuto.u8Speed = g_stINIPara.stIniAE.pstAERelatedBit[u32BitrateLevel].u8Speed;
        stIspExposureAttr.stAuto.u8Tolerance = g_stINIPara.stIniAE.pstAERelatedBit[u32BitrateLevel].u8Tolerance;
        stIspExposureAttr.stAuto.stAEDelayAttr.u16BlackDelayFrame = g_stINIPara.stIniAE.pstAERelatedBit[u32BitrateLevel].u16BlackDelayFrame;
        stIspExposureAttr.stAuto.stAEDelayAttr.u16WhiteDelayFrame = g_stINIPara.stIniAE.pstAERelatedBit[u32BitrateLevel].u16WhiteDelayFrame;
        stIspExposureAttr.stAuto.stSysGainRange.u32Max = g_stINIPara.stIniAE.pstAERelatedBit[u32BitrateLevel].u32SysGainMax;
        s32Ret = HI_MPI_ISP_SetExposureAttr(IspDev, &stIspExposureAttr);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_ISP_SetExposureAttr failed\n");
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_S32 Sceneauto_SetSharpen(HI_S32 s32IspDev, HI_U32 u32Bitrate, HI_U32 u32LastBitrate, HI_U32 u32Exposure, HI_U32 u32LastExposure)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    ISP_DEV IspDev = s32IspDev;
    HI_U32 u32BitrateLevel = 0;
    HI_U32 u32LastBitrateLevel = 0;
    HI_U32 u32BitrateCount = g_stINIPara.stIniSharpen.u32BitrateCount;
    HI_U32 *pu32BitrateThresh = g_stINIPara.stIniSharpen.pu32BitrateThresh;
    HI_U32 u32ExpLevel = 0;
    HI_U32 u32LastExpLevel = 0;
    HI_U32 u32ExpCount = g_stINIPara.stIniSharpen.u32ExpCount;
    HI_U32 *pu32ExpThresh = g_stINIPara.stIniSharpen.pu32ExpThresh;
    ISP_SHARPEN_ATTR_S stIspBayerSharpenAttr;
   

    u32BitrateLevel = GetLevelLtoH(u32Bitrate, u32BitrateLevel, u32BitrateCount, pu32BitrateThresh);
    u32LastBitrateLevel = GetLevelLtoH(u32LastBitrate, u32LastBitrateLevel, u32BitrateCount, pu32BitrateThresh);

    u32ExpLevel = GetLevelLtoH(u32Exposure, u32ExpLevel, u32ExpCount, pu32ExpThresh);
    u32LastExpLevel = GetLevelLtoH(u32LastExposure, u32LastExpLevel, u32ExpCount, pu32ExpThresh);
    
    if ((SCENEAUTO_SPECIAL_SCENE_IR != g_eSpecialScene) &&(SCENEAUTO_SPECIAL_SCENE_TRAFFIC != g_eSpecialScene) && ((u32BitrateLevel != u32LastBitrateLevel) || (u32ExpLevel != u32LastExpLevel)))
    {
        s32Ret = HI_MPI_ISP_GetSharpenAttr(IspDev, &stIspBayerSharpenAttr);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_ISP_GetBayerSharpenAttr failed\n");
            return HI_FAILURE;
        }
       
        for (i = 0; i < EXPOSURE_LEVEL; i++)
        {
        /*
			stIspBayerSharpenAttr.stAuto.au16SharpenUd[i] = g_stINIPara.stIniSharpen.pstSharpen->au16SharpenUd[i];
			stIspBayerSharpenAttr.stAuto.au8SharpenD[i]  = g_stINIPara.stIniSharpen.pstSharpen->au8SharpenD[i];
			stIspBayerSharpenAttr.stAuto.au8TextureThr[i] = g_stINIPara.stIniSharpen.pstSharpen->au8TextureThr[i];
			stIspBayerSharpenAttr.stAuto.au8SharpenEdge[i] = g_stINIPara.stIniSharpen.pstSharpen->au8SharpenEdge[i];
			stIspBayerSharpenAttr.stAuto.au8OverShoot[i] =	 g_stINIPara.stIniSharpen.pstSharpen->au8OverShoot[i] ;
			stIspBayerSharpenAttr.stAuto.au8UnderShoot[i]=	 g_stINIPara.stIniSharpen.pstSharpen->au8UnderShoot[i];
			stIspBayerSharpenAttr.stAuto.au8shootSupStr[i]=  g_stINIPara.stIniSharpen.pstSharpen->au8shootSupStr[i]; 	
			stIspBayerSharpenAttr.stAuto.au8DetailCtrl[i] =  g_stINIPara.stIniSharpen.pstSharpen->au8DetailCtrl[i];
		*/
			stIspBayerSharpenAttr.stAuto.au16SharpenUd[i] = g_stINIPara.stIniSharpen.pstSharpen[u32ExpLevel].au16SharpenUd[i];
			stIspBayerSharpenAttr.stAuto.au8SharpenD[i]  = g_stINIPara.stIniSharpen.pstSharpen[u32ExpLevel].au8SharpenD[i];
			stIspBayerSharpenAttr.stAuto.au8TextureThr[i] = g_stINIPara.stIniSharpen.pstSharpen[u32ExpLevel].au8TextureThr[i];
			stIspBayerSharpenAttr.stAuto.au8SharpenEdge[i] = g_stINIPara.stIniSharpen.pstSharpen[u32ExpLevel].au8SharpenEdge[i];
			stIspBayerSharpenAttr.stAuto.au8EdgeThr[i] = g_stINIPara.stIniSharpen.pstSharpen[u32ExpLevel].au8EdgeThr[i];
			stIspBayerSharpenAttr.stAuto.au8OverShoot[i] =	 g_stINIPara.stIniSharpen.pstSharpen[u32ExpLevel].au8OverShoot[i] ;
			stIspBayerSharpenAttr.stAuto.au8UnderShoot[i]=	 g_stINIPara.stIniSharpen.pstSharpen[u32ExpLevel].au8UnderShoot[i];
			stIspBayerSharpenAttr.stAuto.au8shootSupStr[i]=  g_stINIPara.stIniSharpen.pstSharpen[u32ExpLevel].au8shootSupStr[i]; 	
			stIspBayerSharpenAttr.stAuto.au8DetailCtrl[i] =  g_stINIPara.stIniSharpen.pstSharpen[u32ExpLevel].au8DetailCtrl[i];		
        }
        s32Ret = HI_MPI_ISP_SetSharpenAttr(IspDev, &stIspBayerSharpenAttr);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_ISP_SetBayerSharpenAttr failed\n");
            return HI_FAILURE;
        }
 
    }    

    return HI_SUCCESS;
}


HI_S32 Sceneauto_SetDP(HI_S32 s32IspDev, HI_U32 u32Exposure, HI_U32 u32LastExposure)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_DEV IspDev = s32IspDev;
    HI_U32 u32ExpLevel = 0;
    HI_U32 u32LastExpLevel = 0;
    HI_U32 u32Count = g_stINIPara.stIniDP.u32ExpCount;
    HI_U32 *pu32Thresh = g_stINIPara.stIniDP.pu32ExpThresh;
    ISP_DP_DYNAMIC_ATTR_S stDPAttr;

    u32ExpLevel = GetLevelLtoH(u32Exposure, u32ExpLevel, u32Count, pu32Thresh);
    u32LastExpLevel = GetLevelLtoH(u32LastExposure, u32LastExpLevel, u32Count, pu32Thresh);

    if ((u32ExpLevel != u32LastExpLevel))
    {
		if(SCENEAUTO_SPECIAL_SCENE_NONE == g_eSpecialScene)
		{
			s32Ret = HI_MPI_ISP_GetDPDynamicAttr(IspDev, &stDPAttr);
        	if (HI_SUCCESS != s32Ret)
        	{
           		printf("HI_MPI_ISP_GetDPAttr failed\n");
            	return HI_FAILURE;
        	}
		
        	stDPAttr.bSupTwinkleEn= g_stINIPara.stIniDP.pstDPAttr[u32ExpLevel].bSupTwinkleEn;
			stDPAttr.s8SoftThr = g_stINIPara.stIniDP.pstDPAttr[u32ExpLevel].SoftThr;
			stDPAttr.u8SoftSlope = g_stINIPara.stIniDP.pstDPAttr[u32ExpLevel].SoftSlope;
        	s32Ret = HI_MPI_ISP_SetDPDynamicAttr(IspDev, &stDPAttr);
        	if (HI_SUCCESS != s32Ret)
        	{
            	printf("HI_MPI_ISP_SetDPAttr failed\n");
            	return HI_FAILURE;
        	}
		
    	}

    }
    return HI_SUCCESS;
}

HI_S32 Sceneauto_SetShading(HI_S32 s32IspDev)
{
	HI_S32 s32Ret = HI_FAILURE;
	ISP_SHADING_ATTR_S pstShadingAttr;
	VI_DEV ViDev;
	ViDev = g_stINIPara.stMpInfo.s32ViDev;
	s32Ret = HI_MPI_ISP_GetMeshShadingAttr(ViDev, &pstShadingAttr);
	if (HI_SUCCESS != s32Ret)
      {
     		printf("HI_MPI_ISP_GetMeshShadingAttr failed\n");
            	return HI_FAILURE;
      }
      pstShadingAttr.bEnable = g_stINIPara.stIniShading.bEnable;
      pstShadingAttr.enOpType =  (ISP_OP_TYPE_E)g_stINIPara.stIniShading.enOpType;
      s32Ret = HI_MPI_ISP_SetMeshShadingAttr(ViDev, &pstShadingAttr);
      if (HI_SUCCESS != s32Ret)
      {
        	printf("HI_MPI_ISP_SetMeshShadingAttr failed\n");
        	return HI_FAILURE;
    	}
	return HI_SUCCESS;
}

HI_S32 Sceneauto_SetDci(HI_S32 s32IspDev)
{
    HI_S32 s32Ret = HI_SUCCESS;
	VI_DCI_PARAM_S stViDciParam;
	VI_DEV ViDev;
	ViDev = g_stINIPara.stMpInfo.s32ViDev;
	if(SCENEAUTO_SPECIAL_SCENE_NONE == g_eSpecialScene)
	{
		//DCI
		s32Ret = HI_MPI_VI_GetDCIParam(ViDev, &stViDciParam);
		if (HI_SUCCESS != s32Ret)
		{
			printf("HI_MPI_VI_GetDCIParam failed\n");
			return HI_FAILURE;
		}
		stViDciParam.bEnable = g_stINIPara.stIniDci.bDCIEnable;
		stViDciParam.u32BlackGain = g_stINIPara.stIniDci.u32DCIBlackGain;
		stViDciParam.u32ContrastGain = g_stINIPara.stIniDci.u32DCIContrastGain;
		stViDciParam.u32LightGain = g_stINIPara.stIniDci.u32DCILightGain;
	    s32Ret = HI_MPI_VI_SetDCIParam(ViDev, &stViDciParam);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_VI_SetDCIParam failed\n");
	        return HI_FAILURE;
	    }
    }
    return HI_SUCCESS;
}
HI_S32 Sceneauto_Set2DNR(HI_S32 s32IspDev)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_NR_ATTR_S pstNRAttr;
    ISP_DEV IspDev = s32IspDev;
    
    s32Ret = HI_MPI_ISP_GetNRAttr(IspDev, &pstNRAttr);
    if (HI_SUCCESS != s32Ret)
    {
         printf("HI_MPI_ISP_GetNRAttr failed\n");
         return HI_FAILURE;
    }
   // printf("error=%d\n",pstNRAttr.stAuto.au8ChromaStr[0][4]);
    pstNRAttr.stWDR.au8WDRCoarseStr[0] = g_stINIPara.stIni2dnr.au8WDRCoarseStr[0];
    pstNRAttr.stWDR.au8WDRCoarseStr[1] = g_stINIPara.stIni2dnr.au8WDRCoarseStr[1];
    
    s32Ret = HI_MPI_ISP_SetNRAttr(IspDev, &pstNRAttr);
    if (HI_SUCCESS != s32Ret)
    {
         printf("HI_MPI_ISP_SetNRAttr failed\n");
         return HI_FAILURE;
    }


    return HI_SUCCESS;
}
HI_S32 Sceneauto_SetDefog(HI_S32 s32IspDev)
{
	HI_S32 s32Ret = HI_SUCCESS;
    ISP_DEV IspDev = s32IspDev;
	ISP_DEFOG_ATTR_S stDefogAttr;
	s32Ret = HI_MPI_ISP_GetDeFogAttr(IspDev, &stDefogAttr);
    if (HI_SUCCESS != s32Ret)
    {
         printf("HI_MPI_ISP_GetDeFogAttr failed\n");
         return HI_FAILURE;
    }
    stDefogAttr.bEnable= g_stINIPara.stIniDefog.bEnable;
	stDefogAttr.enOpType = g_stINIPara.stIniDefog.enOpType;
	stDefogAttr.stManual.u8strength = g_stINIPara.stIniDefog.ManualStrength;
    s32Ret = HI_MPI_ISP_SetDeFogAttr(s32IspDev, &stDefogAttr);
    if (HI_SUCCESS != s32Ret)
    {
         printf("HI_MPI_ISP_SetDeFogAttr failed\n");
         return HI_FAILURE;
    }
	return HI_SUCCESS;
}
HI_S32 Sceneauto_SetDrc(HI_S32 s32IspDev,HI_U32 u32Exposure, HI_U32 u32LastExposure)
{
	HI_S32 s32Ret = HI_SUCCESS;
    ISP_DEV IspDev = s32IspDev;
	ISP_DRC_ATTR_S stDrcAttr;
	HI_U32 u32ExpLevel = 0;
	HI_U32 *pu32Thresh = NULL;
	HI_U32 u32Count = g_stINIPara.stIniDrc.u32ExpCount;
	HI_U32 u32Temp;
    static HI_U32 g_u32LastExpLevel = 0;
	s32Ret = HI_MPI_ISP_GetDRCAttr(IspDev, &stDrcAttr);
    if (HI_SUCCESS != s32Ret)
    {
         printf("HI_MPI_ISP_GetDRCAttr failed\n");
         return HI_FAILURE;
    }
    stDrcAttr.enOpType= g_stINIPara.stIniDrc.enOpType;
    //printf("DRC enable = %d\n",stDrcAttr.bEnable);
	s32Ret = HI_MPI_ISP_SetDRCAttr(s32IspDev, &stDrcAttr);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_ISP_SetDrcAttr failed\n");
		return HI_FAILURE;
	}
	if (((u32Exposure != u32LastExposure))&&(SCENEAUTO_SPECIAL_SCENE_NONE == g_eSpecialScene))
    {
    	if (u32Exposure > u32LastExposure)
		{
		    pu32Thresh = g_stINIPara.stIniDrc.pu32ExpThreshLtoD;
		    u32ExpLevel = GetLevelLtoH(u32Exposure, u32ExpLevel, u32Count, pu32Thresh);
		}
		else 
		{
		    pu32Thresh = g_stINIPara.stIniDrc.pu32ExpThreshDtoL;
		    u32ExpLevel = GetLevelHtoL(u32Exposure, u32ExpLevel, u32Count, pu32Thresh);
		}
		if (u32ExpLevel != g_u32LastExpLevel)
		{
			HI_S32 j = 0;
			for (j = 0; j < g_stINIPara.stIniDrc.s32Interval; j++)
            {
            	s32Ret = HI_MPI_ISP_GetDRCAttr(IspDev, &stDrcAttr);
			    if (HI_SUCCESS != s32Ret)
			    {
			         printf("HI_MPI_ISP_GetDRCAttr failed\n");
			         return HI_FAILURE;
			    }
	            if (g_stINIPara.stIniDrc.pstDrcAttr[g_u32LastExpLevel].stManulalStr > g_stINIPara.stIniDrc.pstDrcAttr[u32ExpLevel].stManulalStr)
	             {
	                 u32Temp = ((HI_U32)(g_stINIPara.stIniDrc.pstDrcAttr[g_u32LastExpLevel].stManulalStr - g_stINIPara.stIniDrc.pstDrcAttr[u32ExpLevel].stManulalStr)) << 8;
	                 u32Temp = (u32Temp * (j + 1)) / g_stINIPara.stIniDrc.s32Interval;
	                 u32Temp = u32Temp >> 8;
	                 stDrcAttr.stManual.u8Strength = g_stINIPara.stIniDrc.pstDrcAttr[g_u32LastExpLevel].stManulalStr - (HI_U32)u32Temp;
	            }
	            else
	            {
	                u32Temp = ((HI_U32)(g_stINIPara.stIniDrc.pstDrcAttr[u32ExpLevel].stManulalStr - g_stINIPara.stIniDrc.pstDrcAttr[g_u32LastExpLevel].stManulalStr)) << 8;
	                u32Temp = (u32Temp * (j + 1)) / g_stINIPara.stIniDrc.s32Interval;
	                u32Temp = u32Temp >> 8;
	                stDrcAttr.stManual.u8Strength = g_stINIPara.stIniDrc.pstDrcAttr[g_u32LastExpLevel].stManulalStr + (HI_U32)u32Temp;
	            }
			    s32Ret = HI_MPI_ISP_SetDRCAttr(s32IspDev, &stDrcAttr);
			    if (HI_SUCCESS != s32Ret)
			    {
			         printf("HI_MPI_ISP_SetDrcAttr failed\n");
			         return HI_FAILURE;
			    }
				hi_usleep(80000);
			}
            g_u32LastExpLevel = u32ExpLevel;
		}
	}
	return HI_SUCCESS;
}

HI_S32 Sceneauto_SetAeRelatedExp(HI_S32 s32IspDev, HI_U32 u32Exposure, HI_U32 u32LastExposure)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_DEV IspDev = s32IspDev;
    HI_U32 u32ExpLevel = 0;
    HI_U32 u32LastExpLevel = 0;
    HI_U32 u32Count = 0;
    HI_U32 *pu32Thresh = NULL;
    ISP_EXPOSURE_ATTR_S stIspExposureAttr;
	ISP_WDR_EXPOSURE_ATTR_S stIspWDRExposureAttr;

    s32Ret = HI_MPI_ISP_GetExposureAttr(IspDev, &stIspExposureAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetExposureAttr failed\n");
        return HI_FAILURE;
    }

	s32Ret = HI_MPI_ISP_GetWDRExposureAttr(IspDev, &stIspWDRExposureAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetWDRExposureAttr failed\n");
        return HI_FAILURE;
    }

    if (SCENEAUTO_SPECIAL_SCENE_IR == g_eSpecialScene)
    {
        u32Count = g_stINIPara.stIniIr.u32ExpCount;

        if (u32Exposure > u32LastExposure)
        {
            pu32Thresh = g_stINIPara.stIniIr.pu32ExpThreshLtoH;
            u32ExpLevel = GetLevelLtoH(u32Exposure, u32ExpLevel, u32Count, pu32Thresh);
            u32LastExpLevel = GetLevelLtoH(u32LastExposure, u32LastExpLevel, u32Count, pu32Thresh);
        }
        else if (u32Exposure < u32LastExposure)
        {
            pu32Thresh = g_stINIPara.stIniIr.pu32ExpThreshHtoL;
            u32ExpLevel = GetLevelHtoL(u32Exposure, u32ExpLevel, u32Count, pu32Thresh);
            u32LastExpLevel = GetLevelHtoL(u32LastExposure, u32LastExpLevel, u32Count, pu32Thresh);
        }

        if (u32ExpLevel != u32LastExpLevel)
        {
            stIspExposureAttr.stAuto.u8Compensation = g_stINIPara.stIniIr.pu8ExpCompensation[u32ExpLevel];
            stIspExposureAttr.stAuto.u8MaxHistOffset = g_stINIPara.stIniIr.pu8MaxHistOffset[u32ExpLevel];
        }

		stIspWDRExposureAttr.u32ExpRatioMax = g_stINIPara.stIniAE.u32ExpRatioMax;
        stIspWDRExposureAttr.u32ExpRatioMin = g_stINIPara.stIniAE.u32ExpRatioMin;
        stIspWDRExposureAttr.u16RatioBias = g_stINIPara.stIniAE.u16RatioBias;
    }
	else if(SCENEAUTO_SPECIAL_SCENE_TRAFFIC == g_eSpecialScene)
    {
        hi_usleep(200000);
        s32Ret = HI_MPI_ISP_GetExposureAttr(IspDev, &stIspExposureAttr);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_ISP_GetExposureAttr failed\n");
            return HI_FAILURE;
        }

        stIspExposureAttr.stAuto.u8Compensation = g_stINIPara.stIniTraffic.u8ExpCompensation;
        stIspExposureAttr.stAuto.u8MaxHistOffset = g_stINIPara.stIniTraffic.u8MaxHistoffset;
    }
    else if (SCENEAUTO_SPECIAL_SCENE_NONE == g_eSpecialScene)
    {
        u32Count = g_stINIPara.stIniAE.u32ExpCount;

        if (u32Exposure > u32LastExposure)
        {
            pu32Thresh = g_stINIPara.stIniAE.pu32AEExpLtoHThresh;
            u32ExpLevel = GetLevelLtoH(u32Exposure, u32ExpLevel, u32Count, pu32Thresh);
            u32LastExpLevel = GetLevelLtoH(u32LastExposure, u32LastExpLevel, u32Count, pu32Thresh);
        }
        else if (u32Exposure < u32LastExposure)
        {
            pu32Thresh = g_stINIPara.stIniAE.pu32AEExpHtoLThresh;
            u32ExpLevel = GetLevelHtoL(u32Exposure, u32ExpLevel, u32Count, pu32Thresh);
            u32LastExpLevel = GetLevelHtoL(u32LastExposure, u32LastExpLevel, u32Count, pu32Thresh);
        }

        if (u32ExpLevel != u32LastExpLevel)
        {
            stIspExposureAttr.stAuto.u8Compensation = g_stINIPara.stIniAE.pstAERelatedExp[u32ExpLevel].u8AECompesation;
            stIspExposureAttr.stAuto.u8MaxHistOffset = g_stINIPara.stIniAE.pstAERelatedExp[u32ExpLevel].u8AEHistOffset;
        }
		
		stIspWDRExposureAttr.u32ExpRatioMax  = g_stINIPara.stIniAE.u32ExpRatioMax;
        stIspWDRExposureAttr.u32ExpRatioMin  = g_stINIPara.stIniAE.u32ExpRatioMin;
        stIspWDRExposureAttr.u16RatioBias    = g_stINIPara.stIniAE.u16RatioBias;
        stIspWDRExposureAttr.enExpRatioType  = (ISP_OP_TYPE_E)g_stINIPara.stIniAE.u8ExpRatioType;
        stIspWDRExposureAttr.u32ExpRatio     = g_stINIPara.stIniAE.u32ExpRatio;
    }

    s32Ret = HI_MPI_ISP_SetExposureAttr(IspDev, &stIspExposureAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetExposureAttr failed\n");
        return HI_FAILURE;
    }

	s32Ret = HI_MPI_ISP_SetWDRExposureAttr(IspDev, &stIspWDRExposureAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetExposureAttr failed\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 Sceneauto_SetAWB(HI_S32 s32IspDev, HI_U32 u32ISO)
{
	HI_S32 s32Ret = HI_SUCCESS;
	ISP_DEV  IspDev = s32IspDev;
	ISP_STATISTICS_CFG_S stIspStatisticsCfg;
	ISP_WB_ATTR_S	stIspWBAttr;
	HI_S32 i;
	HI_U32 ISO;
	
	s32Ret = HI_MPI_ISP_GetStatisticsConfig(IspDev, &stIspStatisticsCfg);
	if (HI_SUCCESS != s32Ret)
    	{
        	printf("HI_MPI_ISP_GetStatisticsConfig failed\n");
        	return HI_FAILURE;
    	}
	ISO = g_stINIPara.stIniAWB.u32ISO;
	if(u32ISO >= ISO)
	{
		stIspStatisticsCfg.stWBCfg.stBayerCfg.u16BlackLevel = g_stINIPara.stIniAWB.stAwbStatisticsPara->u16BlackLevel;
	}
	else
	{
		stIspStatisticsCfg.stWBCfg.stBayerCfg.u16BlackLevel = 0;
	}
	s32Ret = HI_MPI_ISP_SetStatisticsConfig(IspDev, &stIspStatisticsCfg);
	if (HI_SUCCESS != s32Ret)
    	{
        	printf("HI_MPI_ISP_SetStatisticsConfig failed\n");
        	return HI_FAILURE;
    	}
	
	s32Ret = HI_MPI_ISP_GetWBAttr(IspDev, &stIspWBAttr);
	if (HI_SUCCESS != s32Ret)
    	{
        	printf("HI_MPI_ISP_GetWBAttr failed\n");
        	return HI_FAILURE;
    	}
	for(i = 0; i < 16; i++)
	{
		stIspWBAttr.stAuto.stCbCrTrack.au16CbMax[i] = g_stINIPara.stIniAWB.stAwbCrCbTrack->au16CbMax[i];
		stIspWBAttr.stAuto.stCbCrTrack.au16CbMin[i] = g_stINIPara.stIniAWB.stAwbCrCbTrack->au16CbMin[i];
		stIspWBAttr.stAuto.stCbCrTrack.au16CrMax[i] = g_stINIPara.stIniAWB.stAwbCrCbTrack->au16CrMax[i];
		stIspWBAttr.stAuto.stCbCrTrack.au16CrMin[i] = g_stINIPara.stIniAWB.stAwbCrCbTrack->au16CrMin[i];
	}
	s32Ret = HI_MPI_ISP_SetWBAttr(IspDev, &stIspWBAttr);
	if (HI_SUCCESS != s32Ret)
    	{
        	printf("HI_MPI_ISP_SetWBAttr failed\n");
        	return HI_FAILURE;
    	}
	return HI_SUCCESS;
}

HI_S32 Sceneauto_SetBlackLevel(HI_S32 s32IspDev, HI_U32 u32ISO)
{
	HI_S32 s32Ret = HI_SUCCESS;
	ISP_DEV  IspDev = s32IspDev;
	ISP_BLACK_LEVEL_S stBlackLevel;
	HI_S32 i,j;
	HI_U32 ISO[16];
	HI_FLOAT Increment;

	s32Ret = HI_MPI_ISP_GetBlackLevelAttr(IspDev, &stBlackLevel);
	if (HI_SUCCESS != s32Ret)
    	{
        	printf("HI_MPI_ISP_GetBlackLevelAttr failed\n");
        	return HI_FAILURE;
    	}
	
	for(i = 0; i<16; i++)
	{
		if(i == 0)
		{
			ISO[i] = 100;
		}
		else
		{
			ISO[i] = ISO[i-1] * 2;
		}
	}
	for(i = 0, j = 0; i<16 && u32ISO >= ISO[i] ; i++)
	{
		j = i;
	}
	if(i == 16)
	{
		u32ISO = ISO[j];
		--i;
		--j;
	}
	//printf("u32ISO = %d   i = %d\n", u32ISO, i);
	if((g_stINIPara.stIniBlackLevel.BlackLevel->au16B[i] != g_stINIPara.stIniBlackLevel.BlackLevel->au16B[j]) || (g_stINIPara.stIniBlackLevel.BlackLevel->au16B[i] != stBlackLevel.au16BlackLevel[0]))
	{
		Increment = (u32ISO-ISO[j])/((HI_FLOAT)(ISO[i] - ISO[j]));
		stBlackLevel.au16BlackLevel[0] = g_stINIPara.stIniBlackLevel.BlackLevel->au16R[j] + ((g_stINIPara.stIniBlackLevel.BlackLevel->au16R[i] - g_stINIPara.stIniBlackLevel.BlackLevel->au16R[j]) * Increment);
		stBlackLevel.au16BlackLevel[1] = g_stINIPara.stIniBlackLevel.BlackLevel->au16Gr[j] + ((g_stINIPara.stIniBlackLevel.BlackLevel->au16Gr[i] - g_stINIPara.stIniBlackLevel.BlackLevel->au16Gr[j]) * Increment);
		stBlackLevel.au16BlackLevel[2] = g_stINIPara.stIniBlackLevel.BlackLevel->au16Gb[j] + ((g_stINIPara.stIniBlackLevel.BlackLevel->au16Gb[i] - g_stINIPara.stIniBlackLevel.BlackLevel->au16Gb[j]) * Increment);
		stBlackLevel.au16BlackLevel[3] = g_stINIPara.stIniBlackLevel.BlackLevel->au16B[j] + ((g_stINIPara.stIniBlackLevel.BlackLevel->au16B[i] - g_stINIPara.stIniBlackLevel.BlackLevel->au16B[j]) * Increment);
	}
	s32Ret = HI_MPI_ISP_SetBlackLevelAttr(IspDev, &stBlackLevel);
	if (HI_SUCCESS != s32Ret)
    	{
        	printf("HI_MPI_ISP_SetBlackLevelAttr failed\n");
        	return HI_FAILURE;
    	}

	return HI_SUCCESS;
	
}

HI_S32 Sceneauto_SetGamma(HI_S32 s32IspDev, HI_U32 u32Exposure,HI_U32 u32LastExposure1)
{
    static HI_U32 u32GammaCount = 0;
    static HI_U32 u32LastExpLevel = 0;
    static HI_U32 u32LastExposure = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Temp;
    HI_S32 i, j;
    ISP_DEV IspDev = s32IspDev;
    HI_U32 u32ExpLevel = 0;
    HI_U32 u32Count = g_stINIPara.stIniGamma.u32ExpCount;
    HI_U32 *pu32Thresh = NULL;
    ISP_GAMMA_ATTR_S stIspGammaAttr;

    #if 0
    if (0 == u32LastExposure)
    {
        //set param in first time        
        pu32Thresh = g_stINIPara.stIniGamma.pu32ExpThreshLtoH;
        u32ExpLevel = GetLevelLtoH(u32Exposure, u32ExpLevel, u32Count, pu32Thresh);
        u32LastExposure = u32Exposure;
        u32LastExpLevel = u32ExpLevel;
        return HI_SUCCESS;
    }
    #endif
    u32LastExposure = u32LastExposure1;
	if(u32LastExposure == 0)//first init 
    {
	    u32LastExpLevel = 0;
	}
    if ((u32Exposure != u32LastExposure) 
        && (SCENEAUTO_SPECIAL_SCENE_IR != g_eSpecialScene) 
        && (SCENEAUTO_SPECIAL_SCENE_HLC != g_eSpecialScene)
        && (SCENEAUTO_SPECIAL_SCENE_TRAFFIC != g_eSpecialScene))
    {
    	if (u32Exposure > u32LastExposure)
    	{
        pu32Thresh = g_stINIPara.stIniGamma.pu32ExpThreshLtoH;
        u32ExpLevel = GetLevelLtoH(u32Exposure, u32ExpLevel, u32Count, pu32Thresh);
    	}
    	else if (u32Exposure < u32LastExposure)
    	{
        pu32Thresh = g_stINIPara.stIniGamma.pu32ExpThreshHtoL;
        u32ExpLevel = GetLevelHtoL(u32Exposure, u32ExpLevel, u32Count, pu32Thresh);
    	}

       
        if (u32ExpLevel != u32LastExpLevel)
    	  {
        	u32GammaCount++;
        	if (u32GammaCount > g_stINIPara.stIniGamma.u32DelayCount)
        	{
                 //set gamma
            		for (j = 0; j < g_stINIPara.stIniGamma.s32Interval; j++)
            		{
                		s32Ret = HI_MPI_ISP_GetGammaAttr(IspDev, &stIspGammaAttr);
               	 	if (HI_SUCCESS != s32Ret)
                		{
                    		printf("HI_MPI_ISP_GetGammaAttr failed\n");
                    		return HI_FAILURE;
                		}
                		for (i = 0; i < 257; i++)
                		{
                   	 		if (g_stINIPara.stIniGamma.pstGamma[u32LastExpLevel].u16Table[i] > g_stINIPara.stIniGamma.pstGamma[u32ExpLevel].u16Table[i])
                    		{
                        			u32Temp = ((HI_U32)(g_stINIPara.stIniGamma.pstGamma[u32LastExpLevel].u16Table[i] - g_stINIPara.stIniGamma.pstGamma[u32ExpLevel].u16Table[i])) << 8;
                        			u32Temp = (u32Temp * (j + 1)) / g_stINIPara.stIniGamma.s32Interval;
                        			u32Temp = u32Temp >> 8;
                        			stIspGammaAttr.u16Table[i] = g_stINIPara.stIniGamma.pstGamma[u32LastExpLevel].u16Table[i] - (HI_U32)u32Temp;
                    		}
                    		else
                    		{
                        			u32Temp = ((HI_U32)(g_stINIPara.stIniGamma.pstGamma[u32ExpLevel].u16Table[i] - g_stINIPara.stIniGamma.pstGamma[u32LastExpLevel].u16Table[i])) << 8;
                        			u32Temp = (u32Temp * (j + 1)) / g_stINIPara.stIniGamma.s32Interval;
                        			u32Temp = u32Temp >> 8;
                        			stIspGammaAttr.u16Table[i] = g_stINIPara.stIniGamma.pstGamma[u32LastExpLevel].u16Table[i] + (HI_U32)u32Temp;
                    		}
                		}
                    	stIspGammaAttr.enCurveType = ISP_GAMMA_CURVE_USER_DEFINE;
                		s32Ret = HI_MPI_ISP_SetGammaAttr(IspDev, &stIspGammaAttr);
                		if (HI_SUCCESS != s32Ret)
                		{
                    		printf("HI_MPI_ISP_SetGammaAttr failed\n");
                    		return HI_FAILURE;
                		}
                		hi_usleep(80000);
            		}
                 	u32LastExposure = u32Exposure;
                 	u32LastExpLevel = u32ExpLevel;
                 	u32GammaCount = 0;
        	}
    	}
    	else
    	{
        	u32GammaCount = 0;
    	}
    }
             
    return HI_SUCCESS;
}

HI_S32 CommSceneautoSet3DNRAttr(HI_S32 s32VpssGrp, ADPT_SCENEAUTO_3DNR_ATTR_S *pstAdpt3dnrAttr)
{

 
	
	VPSS_GRP_NRS_PARAM_S stNRSParam;
    stNRSParam.enNRVer = VPSS_NR_V2;
  
    stNRSParam.stNRSParam_V2.LNTH = pstAdpt3dnrAttr ->BoolLNTH;
    stNRSParam.stNRSParam_V2.IES0    = pstAdpt3dnrAttr->s32IES0;

    stNRSParam.stNRSParam_V2.SBS0   = pstAdpt3dnrAttr->s32SBS0;  
	stNRSParam.stNRSParam_V2.SBS1   = pstAdpt3dnrAttr->s32SBS1;
    stNRSParam.stNRSParam_V2.SBS2   = pstAdpt3dnrAttr->s32SBS2;  
	stNRSParam.stNRSParam_V2.SBS3   = pstAdpt3dnrAttr->s32SBS3;

	stNRSParam.stNRSParam_V2.SDS0   = pstAdpt3dnrAttr->s32SDS0;  
	stNRSParam.stNRSParam_V2.SDS1   = pstAdpt3dnrAttr->s32SDS1;  
	stNRSParam.stNRSParam_V2.SDS2   = pstAdpt3dnrAttr->s32SDS2;  
	stNRSParam.stNRSParam_V2.SDS3   = pstAdpt3dnrAttr->s32SDS3;

	stNRSParam.stNRSParam_V2.STH0   = pstAdpt3dnrAttr->s32STH0; 
	stNRSParam.stNRSParam_V2.STH1   = pstAdpt3dnrAttr->s32STH1; 
	stNRSParam.stNRSParam_V2.STH2   = pstAdpt3dnrAttr->s32STH2; 
	stNRSParam.stNRSParam_V2.STH3   = pstAdpt3dnrAttr->s32STH3; 
	
    stNRSParam.stNRSParam_V2.MDP    = pstAdpt3dnrAttr->s32MDP;
    stNRSParam.stNRSParam_V2.MATH1    = pstAdpt3dnrAttr->s32MATH1;
    stNRSParam.stNRSParam_V2.MATH2   = pstAdpt3dnrAttr->s32MATH2;
    stNRSParam.stNRSParam_V2.Pro3      = pstAdpt3dnrAttr->s32Pro3;
	
    stNRSParam.stNRSParam_V2.MDDZ1    = pstAdpt3dnrAttr->s32MDDZ1;
    stNRSParam.stNRSParam_V2.MDDZ2   = pstAdpt3dnrAttr->s32MDDZ2;
        
    stNRSParam.stNRSParam_V2.TFS1    = pstAdpt3dnrAttr->s32TFS1;
	stNRSParam.stNRSParam_V2.TFS2    = pstAdpt3dnrAttr->s32TFS2;

    stNRSParam.stNRSParam_V2.SFC      = pstAdpt3dnrAttr->s32SFC;     
    stNRSParam.stNRSParam_V2.TFC      = pstAdpt3dnrAttr->s32TFC;
    stNRSParam.stNRSParam_V2.TPC      = pstAdpt3dnrAttr->s32TPC;
    stNRSParam.stNRSParam_V2.TRC    = pstAdpt3dnrAttr->s32TRC;

    HI_MPI_VPSS_SetGrpNRSParam(s32VpssGrp,&stNRSParam);


    return HI_SUCCESS;
}


HI_S32 Sceneauto_Set3DNR(HI_S32 s32VpssGrp, HI_U32 u32Iso,const SCENEAUTO_INIPARAM_3DNR_S st3dnrparam)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i;
 
    HI_S32 s32IsoLevel,s32IsoLevel1;
	SCENEAUTO_INIPARAM_NRS_S stSceneauto3dnr;


    if ((u32Iso >= 1300) && (u32Iso <= 2000))
    {
        u32Iso = 1300;
    }
    if ((u32Iso >= 2000) && (u32Iso <= 2700))
    {
        u32Iso = 2700;
    }

    

	   s32IsoLevel = st3dnrparam.u323DnrIsoCount - 1;
	   for (i = 0; i < st3dnrparam.u323DnrIsoCount; i++)
	   {
		   if (u32Iso <= st3dnrparam.pu323DnrIsoThresh[i])
		   {
			   s32IsoLevel = i;
			   break;
		   }
	   }
	   
	   s32IsoLevel1 = s32IsoLevel - 1;
	   
	   if (s32IsoLevel1 < 0)
	   {
		   s32IsoLevel1 = 0;
	   }
	   
	   Interpolate(&stSceneauto3dnr, u32Iso, 
			   &st3dnrparam.pst3dnrParam[s32IsoLevel1], st3dnrparam.pu323DnrIsoThresh[s32IsoLevel1],
			   &st3dnrparam.pst3dnrParam[s32IsoLevel], st3dnrparam.pu323DnrIsoThresh[s32IsoLevel]);



	   ADPT_SCENEAUTO_3DNR_ATTR_S stAdptSceneauto3dnr;
        stAdptSceneauto3dnr.BoolLNTH   = st3dnrparam.BoolLNTH;//not related with ISO

		stAdptSceneauto3dnr.s32IES0	   = stSceneauto3dnr.s32IES0;

		stAdptSceneauto3dnr.s32SBS0	  = stSceneauto3dnr.s32SBS0;  
		stAdptSceneauto3dnr.s32SBS1	  = stSceneauto3dnr.s32SBS1;
		stAdptSceneauto3dnr.s32SBS2	  = stSceneauto3dnr.s32SBS2;  
		stAdptSceneauto3dnr.s32SBS3	  = stSceneauto3dnr.s32SBS3;

		stAdptSceneauto3dnr.s32SDS0	  = stSceneauto3dnr.s32SDS0;  
		stAdptSceneauto3dnr.s32SDS1	  = stSceneauto3dnr.s32SDS1;  
		stAdptSceneauto3dnr.s32SDS2	  = stSceneauto3dnr.s32SDS2;  
		stAdptSceneauto3dnr.s32SDS3	  = stSceneauto3dnr.s32SDS3;

		stAdptSceneauto3dnr.s32STH0	  = stSceneauto3dnr.s32STH0; 
		stAdptSceneauto3dnr.s32STH1	  = stSceneauto3dnr.s32STH1; 
		stAdptSceneauto3dnr.s32STH2	  = stSceneauto3dnr.s32STH2; 
		stAdptSceneauto3dnr.s32STH3	  = stSceneauto3dnr.s32STH3; 

		stAdptSceneauto3dnr.s32MDP    = stSceneauto3dnr.s32MDP;
		stAdptSceneauto3dnr.s32MATH1	 = stSceneauto3dnr.s32MATH1;
		stAdptSceneauto3dnr.s32MATH2	= stSceneauto3dnr.s32MATH2;
		stAdptSceneauto3dnr.s32Pro3	 = stSceneauto3dnr.s32Pro3;	  
		
		stAdptSceneauto3dnr.s32MDDZ1	 = stSceneauto3dnr.s32MDDZ1;
		stAdptSceneauto3dnr.s32MDDZ2	= stSceneauto3dnr.s32MDDZ2;

		stAdptSceneauto3dnr.s32TFS1    = stSceneauto3dnr.s32TFS1;
		stAdptSceneauto3dnr.s32TFS2    = stSceneauto3dnr.s32TFS2;
		
		stAdptSceneauto3dnr.s32SFC	   = stSceneauto3dnr.s32SFC;	 
		stAdptSceneauto3dnr.s32TFC	   = stSceneauto3dnr.s32TFC;
		stAdptSceneauto3dnr.s32TPC	   = stSceneauto3dnr.s32TPC;
		stAdptSceneauto3dnr.s32TRC	   = stSceneauto3dnr.s32TRC;
	   
	   s32Ret = CommSceneautoSet3DNRAttr(s32VpssGrp, &stAdptSceneauto3dnr);
	   if (HI_SUCCESS != s32Ret)
	   {
		   printf("CommSceneautoSet3DNR failed\n");
		   return HI_FAILURE;
	   }
	   return HI_SUCCESS;


}



HI_S32 Sceneauto_IVEStop()
{
    return HI_SUCCESS;
}

HI_S32 Sceneauto_IVEStart()
{
    return HI_SUCCESS;
}

HI_S32 Sceneauto_SetPreviousPara()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i,j;
    ISP_DEV IspDev;
    VI_DEV ViDev;

    //ISP_DEMOSAIC_ATTR_S stIspDemosaicAttr;
    ISP_SHARPEN_ATTR_S stIspBayerSharpenAttr;
   
    ISP_DP_DYNAMIC_ATTR_S stIspDpAttr;
    ISP_GAMMA_ATTR_S stIspGammaAttr;
    ISP_EXPOSURE_ATTR_S stIspExposureAttr;
    ISP_AE_ROUTE_S stIspAeRoute;

    ISP_STATISTICS_CFG_S stIspStatisticsCfg;
	
    ISP_SATURATION_ATTR_S stIspSaturationAttr;
	ISP_WB_ATTR_S stIspAwbAttr;
    VI_DCI_PARAM_S stViDciParam;
    ISP_DRC_ATTR_S stIspDrcAttr;
    ISP_SHADING_ATTR_S pstShadingAttr;
	ISP_DEFOG_ATTR_S stDefogAttr;
    ISP_NR_ATTR_S pstNRAttr;

    IspDev = g_stINIPara.stMpInfo.s32IspDev;
    ViDev = g_stINIPara.stMpInfo.s32ViDev;
    
	
    //BayerSharpen
    s32Ret = HI_MPI_ISP_GetSharpenAttr(IspDev, &stIspBayerSharpenAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetBayerSharpenAttr failed\n");
        return HI_FAILURE;
    }
	
	for (i = 0; i < EXPOSURE_LEVEL; i++)
    {
        stIspBayerSharpenAttr.stAuto.au16SharpenUd[i] = g_stINIPara.stIniSharpen.pstSharpen->au16SharpenUd[i];
        stIspBayerSharpenAttr.stAuto.au8SharpenD[i]  = g_stINIPara.stIniSharpen.pstSharpen->au8SharpenD[i];
        stIspBayerSharpenAttr.stAuto.au8TextureThr[i] = g_stINIPara.stIniSharpen.pstSharpen->au8TextureThr[i];
		stIspBayerSharpenAttr.stAuto.au8SharpenEdge[i] = g_stINIPara.stIniSharpen.pstSharpen->au8SharpenEdge[i];
		stIspBayerSharpenAttr.stAuto.au8OverShoot[i] =   g_stINIPara.stIniSharpen.pstSharpen->au8OverShoot[i] ;
		stIspBayerSharpenAttr.stAuto.au8UnderShoot[i]=   g_stINIPara.stIniSharpen.pstSharpen->au8UnderShoot[i];
		stIspBayerSharpenAttr.stAuto.au8shootSupStr[i]=  g_stINIPara.stIniSharpen.pstSharpen->au8shootSupStr[i];		
		stIspBayerSharpenAttr.stAuto.au8DetailCtrl[i] =  g_stINIPara.stIniSharpen.pstSharpen->au8DetailCtrl[i];
    }

	 
    s32Ret = HI_MPI_ISP_SetSharpenAttr(IspDev, &stIspBayerSharpenAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetBayerSharpenAttr failed\n");
        return HI_FAILURE;
    }


    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetYuvSharpenAttr failed\n");
        return HI_FAILURE;
    }

    //DP
    if(SCENEAUTO_SPECIAL_SCENE_NONE != g_eSpecialScene)
    {
    s32Ret = HI_MPI_ISP_GetDPDynamicAttr(IspDev, &stIspDpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetDPAttr failed\n");
        return HI_FAILURE;
    }
    stIspDpAttr.bSupTwinkleEn= g_stPreviousPara.stDp.bSupTwinkleEn;
	stIspDpAttr.s8SoftThr= g_stPreviousPara.stDp.SoftThr;
	stIspDpAttr.u8SoftSlope= g_stPreviousPara.stDp.SoftSlope;
    s32Ret = HI_MPI_ISP_SetDPDynamicAttr(IspDev, &stIspDpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetDPAttr failed\n");
        return HI_FAILURE;
    }
    }

	//Defog
    s32Ret = HI_MPI_ISP_GetDeFogAttr(IspDev, &stDefogAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetDeFogAttr failed\n");
        return HI_FAILURE;
    }
	stDefogAttr.bEnable = g_stPreviousPara.stDefog.bEnable;
	stDefogAttr.enOpType = g_stPreviousPara.stDefog.enOpType;
	stDefogAttr.stManual.u8strength = g_stPreviousPara.stDefog.ManualStrength;
	s32Ret = HI_MPI_ISP_SetDeFogAttr(IspDev, &stDefogAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetDeFogAttr failed\n");
        return HI_FAILURE;
    }

    //2dnr
    s32Ret = HI_MPI_ISP_GetNRAttr(IspDev, &pstNRAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetNRAttr failed\n");
        return HI_FAILURE;
    }
	pstNRAttr.stWDR.au8WDRCoarseStr[0] = g_stPreviousPara.st2DNR.au8WDRCoarseStr[0];
    pstNRAttr.stWDR.au8WDRCoarseStr[1] = g_stPreviousPara.st2DNR.au8WDRCoarseStr[1];
    s32Ret = HI_MPI_ISP_SetNRAttr(IspDev, &pstNRAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetNRAttr failed\n");
        return HI_FAILURE;
    }
    //gamma
    s32Ret = HI_MPI_ISP_GetGammaAttr(IspDev, &stIspGammaAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetGammaAttr failed\n");
        return HI_FAILURE;
    }
    //printf("g_stPreviousPara.stGamma.u16Table[i] = %u\n", g_stPreviousPara.stGamma.u16Table[1]);
    stIspGammaAttr.enCurveType = ISP_GAMMA_CURVE_USER_DEFINE;
    for (i = 0; i < 257; i++)
    {
        stIspGammaAttr.u16Table[i] = g_stPreviousPara.stGamma.u16Table[i];
    }
    s32Ret = HI_MPI_ISP_SetGammaAttr(IspDev, &stIspGammaAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetGammaAttr failed\n");
        return HI_FAILURE;
    }

    //ae
    s32Ret = HI_MPI_ISP_GetExposureAttr(IspDev, &stIspExposureAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetExposureAttr failed\n");
        return HI_FAILURE;
    }
    stIspExposureAttr.u8AERunInterval = g_stPreviousPara.stExposure.u8AERunInterval;
    stIspExposureAttr.stAuto.u8Speed = g_stPreviousPara.stExposure.u8Speed;
    stIspExposureAttr.stAuto.u8Tolerance = g_stPreviousPara.stExposure.u8Tolerance;
    stIspExposureAttr.stAuto.stAEDelayAttr.u16BlackDelayFrame = g_stPreviousPara.stExposure.u16BlackDelayFrame;
    stIspExposureAttr.stAuto.stAEDelayAttr.u16WhiteDelayFrame = g_stPreviousPara.stExposure.u16WhiteDelayFrame;
    stIspExposureAttr.stAuto.stSysGainRange.u32Max = g_stPreviousPara.stExposure.u32SysGainMax;
    stIspExposureAttr.stAuto.u8Compensation = g_stPreviousPara.stExposure.u8AECompesation;
    stIspExposureAttr.stAuto.u8MaxHistOffset = g_stPreviousPara.stExposure.u8AEHistOffset;
    stIspExposureAttr.stAuto.u16HistRatioSlope = g_stPreviousPara.stExposure.u16HistRatioSlope;
    for (i = 0; i < AE_WEIGHT_ROW; i++)
    {
        for (j = 0; j < AE_WEIGHT_COLUMN; j++)
        {
            stIspExposureAttr.stAuto.au8Weight[i][j] = g_stPreviousPara.stExposure.au8Weight[i][j];
        }
    }
    s32Ret = HI_MPI_ISP_SetExposureAttr(IspDev, &stIspExposureAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetExposureAttr failed\n");
        return HI_FAILURE;
    }
    s32Ret = HI_MPI_ISP_GetAERouteAttr(IspDev, &stIspAeRoute);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetAERouteAttr failed\n");
        return HI_FAILURE;
    }
    stIspAeRoute.u32TotalNum = g_stPreviousPara.stExposure.s32TotalNum;
    for (i = 0; i < g_stPreviousPara.stExposure.s32TotalNum; i++)
    {
        stIspAeRoute.astRouteNode[i].u32IntTime = g_stPreviousPara.stExposure.astRouteNode[i].u32IntTime;
        stIspAeRoute.astRouteNode[i].u32SysGain = g_stPreviousPara.stExposure.astRouteNode[i].u32SysGain;
    }
    s32Ret = HI_MPI_ISP_SetAERouteAttr(IspDev, &stIspAeRoute);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetAERouteAttr failed\n");
        return HI_FAILURE;
    }

	/****************AWB**************/
	s32Ret = HI_MPI_ISP_GetStatisticsConfig(IspDev, &stIspStatisticsCfg);
	if (HI_SUCCESS != s32Ret)
    	{
        	printf("HI_MPI_ISP_GetStatisticsConfig failed\n");
        	return HI_FAILURE;
    	}
	stIspStatisticsCfg.stWBCfg.stBayerCfg.u16BlackLevel = 0;
	s32Ret = HI_MPI_ISP_SetStatisticsConfig(IspDev, &stIspStatisticsCfg);
	if (HI_SUCCESS != s32Ret)
    	{
        	printf("HI_MPI_ISP_SetStatisticsConfig failed\n");
        	return HI_FAILURE;
    	}
	s32Ret = HI_MPI_ISP_GetWBAttr(IspDev, &stIspAwbAttr);
	if (HI_SUCCESS != s32Ret)
    	{
        	printf("HI_MPI_ISP_GetWBAttr failed\n");
        	return HI_FAILURE;
    	}
	stIspAwbAttr.enOpType = (ISP_OP_TYPE_E)g_stPreviousPara.stAwb.u8OpType;
	for(i = 0; i < 16; i++)
	{
		stIspAwbAttr.stAuto.stCbCrTrack.au16CbMax[i] = g_stPreviousPara.stAwb.au16CbMax[i];
		stIspAwbAttr.stAuto.stCbCrTrack.au16CbMin[i] = g_stPreviousPara.stAwb.au16CbMin[i];
		stIspAwbAttr.stAuto.stCbCrTrack.au16CrMax[i] = g_stPreviousPara.stAwb.au16CrMax[i];
		stIspAwbAttr.stAuto.stCbCrTrack.au16CrMin[i] = g_stPreviousPara.stAwb.au16CrMin[i];
	}
	s32Ret = HI_MPI_ISP_SetWBAttr(IspDev, &stIspAwbAttr);
	if (HI_SUCCESS != s32Ret)
    	{
        	printf("HI_MPI_ISP_SetWBAttr failed\n");
        	return HI_FAILURE;
    	}

    //saturation
    s32Ret = HI_MPI_ISP_GetSaturationAttr(IspDev, &stIspSaturationAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetSaturationAttr failed\n");
        return HI_FAILURE;
    }
    stIspSaturationAttr.enOpType = (ISP_OP_TYPE_E)g_stPreviousPara.stSaturation.u8OpType;
    stIspSaturationAttr.stManual.u8Saturation = g_stPreviousPara.stSaturation.u8ManuSaturation;
    for (i = 0; i < 16; i++)
    {
        stIspSaturationAttr.stAuto.au8Sat[i] = g_stPreviousPara.stSaturation.u8Saturation[i];
    }
    s32Ret = HI_MPI_ISP_SetSaturationAttr(IspDev, &stIspSaturationAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetSaturationAttr failed\n");
        return HI_FAILURE;
    }
	//AWB
	s32Ret = HI_MPI_ISP_GetWBAttr(IspDev, &stIspAwbAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetWBAttr failed\n");
        return HI_FAILURE;
    }
    stIspAwbAttr.enOpType = (ISP_OP_TYPE_E)g_stPreviousPara.stAwb.u8OpType;
    s32Ret = HI_MPI_ISP_SetWBAttr(IspDev, &stIspAwbAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetWBAttr failed\n");
        return HI_FAILURE;
    }

    //DCI
    s32Ret = HI_MPI_VI_GetDCIParam(ViDev, &stViDciParam);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VI_GetDCIParam failed\n");
        return HI_FAILURE;
    }
    stViDciParam.bEnable= g_stPreviousPara.stDci.bDCIEnable;
    stViDciParam.u32BlackGain = g_stPreviousPara.stDci.u32DCIBlackGain;
    stViDciParam.u32ContrastGain = g_stPreviousPara.stDci.u32DCIContrastGain;
    stViDciParam.u32LightGain = g_stPreviousPara.stDci.u32DCILightGain;
    s32Ret = HI_MPI_VI_SetDCIParam(ViDev, &stViDciParam);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VI_SetDCIParam failed\n");
        return HI_FAILURE;
    }

    //DRC
    s32Ret = HI_MPI_ISP_GetDRCAttr(IspDev, &stIspDrcAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetDRCAttr failed\n");
        return HI_FAILURE;
    }
    stIspDrcAttr.bEnable = g_stPreviousPara.stDrc.bEnable;
    stIspDrcAttr.enOpType = (ISP_OP_TYPE_E)g_stPreviousPara.stDrc.u8OpType;
    stIspDrcAttr.stManual.u8Strength = g_stPreviousPara.stDrc.u8Strength;
    stIspDrcAttr.u8SpatialVar = g_stPreviousPara.stDrc.u8SpatialVar;
    stIspDrcAttr.u8RangeVar = g_stPreviousPara.stDrc.u8RangeVar;
    stIspDrcAttr.u8Asymmetry = g_stPreviousPara.stDrc.u8Asymmetry;
    stIspDrcAttr.u8SecondPole = g_stPreviousPara.stDrc.u8SecondPole;
    stIspDrcAttr.u8Stretch = g_stPreviousPara.stDrc.u8Stretch;
    stIspDrcAttr.u8Compress = g_stPreviousPara.stDrc.u8Compress;
    stIspDrcAttr.u8PDStrength = g_stPreviousPara.stDrc.u8PDStrength;
    stIspDrcAttr.u8LocalMixingBrigtht = g_stPreviousPara.stDrc.u8LocalMixingBrigtht;
    stIspDrcAttr.u8LocalMixingDark = g_stPreviousPara.stDrc.u8LocalMixingDark;    
    s32Ret = HI_MPI_ISP_SetDRCAttr(IspDev, &stIspDrcAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetDRCAttr failed\n");
        return HI_FAILURE;
    }

    //Shading
    s32Ret = HI_MPI_ISP_GetMeshShadingAttr(IspDev, &pstShadingAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetMeshShadingAttr failed\n");
        return HI_FAILURE;
    }
    pstShadingAttr.bEnable = g_stPreviousPara.stShading.bEnable;
    pstShadingAttr.enOpType = g_stPreviousPara.stShading.enOpType;
    s32Ret = HI_MPI_ISP_SetMeshShadingAttr(IspDev, &pstShadingAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetMeshShadingAttr failed\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 Sceneauto_GetPreviousPara()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i,j;
    ISP_DEV IspDev;
    VI_DEV ViDev;

    ISP_SHARPEN_ATTR_S stIspBayerSharpenAttr;
   
    ISP_DP_DYNAMIC_ATTR_S stIspDpAttr;
    ISP_GAMMA_ATTR_S stIspGammaAttr;
    ISP_EXPOSURE_ATTR_S stIspExposureAttr;
    ISP_AE_ROUTE_S stIspAeRoute;
    ISP_STATISTICS_CFG_S stIspStatisticsCfg;
    ISP_SATURATION_ATTR_S stIspSaturationAttr;
	ISP_WB_ATTR_S pstAWBAttr;
    VI_DCI_PARAM_S stViDciParam;
    ISP_DRC_ATTR_S stIspDrcAttr;
    ISP_DEFOG_ATTR_S stDefogAttr;
    ISP_SHADING_ATTR_S pstShadingAttr;
    ISP_BLACK_LEVEL_S stBlackLevel;
    ISP_NR_ATTR_S pstNRAttr;

    IspDev = g_stINIPara.stMpInfo.s32IspDev;
    ViDev = g_stINIPara.stMpInfo.s32ViDev;
    
    /****************BayerSharpen****************/
    s32Ret = HI_MPI_ISP_GetSharpenAttr(IspDev, &stIspBayerSharpenAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetBayerSharpenAttr failed\n");
        return HI_FAILURE;
    }
    for (i = 0; i < EXPOSURE_LEVEL; i++)
    {
        g_stPreviousPara.stSharpen.au16SharpenUd[i] = stIspBayerSharpenAttr.stAuto.au16SharpenUd[i];
        g_stPreviousPara.stSharpen.au8SharpenD[i] = stIspBayerSharpenAttr.stAuto.au8SharpenD[i];
        g_stPreviousPara.stSharpen.au8TextureThr[i] = stIspBayerSharpenAttr.stAuto.au8TextureThr[i];
		g_stPreviousPara.stSharpen.au8SharpenEdge[i] =stIspBayerSharpenAttr.stAuto.au8SharpenEdge[i];
		g_stPreviousPara.stSharpen.au8OverShoot[i] =  stIspBayerSharpenAttr.stAuto.au8OverShoot[i]; 
		g_stPreviousPara.stSharpen.au8UnderShoot[i] = stIspBayerSharpenAttr.stAuto.au8UnderShoot[i];
		g_stPreviousPara.stSharpen.au8shootSupStr[i] =stIspBayerSharpenAttr.stAuto.au8shootSupStr[i];
		g_stPreviousPara.stSharpen.au8DetailCtrl[i] = stIspBayerSharpenAttr.stAuto.au8DetailCtrl[i];

    }

    /****************DP****************/
    s32Ret = HI_MPI_ISP_GetDPDynamicAttr(IspDev, &stIspDpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetDPAttr failed\n");
        return HI_FAILURE;
    }
    g_stPreviousPara.stDp.bSupTwinkleEn = stIspDpAttr.bSupTwinkleEn;
	g_stPreviousPara.stDp.SoftSlope = stIspDpAttr.u8SoftSlope;
    g_stPreviousPara.stDp.SoftThr = stIspDpAttr.s8SoftThr;
	//Defog
    s32Ret = HI_MPI_ISP_GetDeFogAttr(IspDev, &stDefogAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetDeFogAttr failed\n");
        return HI_FAILURE;
    }
	g_stPreviousPara.stDefog.bEnable = stDefogAttr.bEnable;
	g_stPreviousPara.stDefog.enOpType  = stDefogAttr.enOpType;
	g_stPreviousPara.stDefog.ManualStrength = stDefogAttr.stManual.u8strength;
    //2dnr
    s32Ret = HI_MPI_ISP_GetNRAttr(IspDev, &pstNRAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetNRAttr failed\n");
        return HI_FAILURE;
    }
	g_stPreviousPara.st2DNR.au8WDRCoarseStr[0]= pstNRAttr.stWDR.au8WDRCoarseStr[0];
    g_stPreviousPara.st2DNR.au8WDRCoarseStr[1]= pstNRAttr.stWDR.au8WDRCoarseStr[1];

    /****************gamma****************/
    s32Ret = HI_MPI_ISP_GetGammaAttr(IspDev, &stIspGammaAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetGammaAttr failed\n");
        return HI_FAILURE;
    }
    g_stPreviousPara.stGamma.u8CurveType = stIspGammaAttr.enCurveType;
    for (i = 0; i < 257; i++)
    {
        g_stPreviousPara.stGamma.u16Table[i] = stIspGammaAttr.u16Table[i];
    }

    /**************ae***********/
    s32Ret = HI_MPI_ISP_GetExposureAttr(IspDev, &stIspExposureAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetExposureAttr failed\n");
        return HI_FAILURE;
    }
    g_stPreviousPara.stExposure.u8AERunInterval = stIspExposureAttr.u8AERunInterval;
    g_stPreviousPara.stExposure.u8Speed = stIspExposureAttr.stAuto.u8Speed;
    g_stPreviousPara.stExposure.u8Tolerance = stIspExposureAttr.stAuto.u8Tolerance;
    g_stPreviousPara.stExposure.u16BlackDelayFrame = stIspExposureAttr.stAuto.stAEDelayAttr.u16BlackDelayFrame;
    g_stPreviousPara.stExposure.u16WhiteDelayFrame = stIspExposureAttr.stAuto.stAEDelayAttr.u16WhiteDelayFrame;
    g_stPreviousPara.stExposure.u32SysGainMax = stIspExposureAttr.stAuto.stSysGainRange.u32Max;
    g_stPreviousPara.stExposure.u8AECompesation = stIspExposureAttr.stAuto.u8Compensation;
    g_stPreviousPara.stExposure.u8AEHistOffset = stIspExposureAttr.stAuto.u8MaxHistOffset;
    g_stPreviousPara.stExposure.u16HistRatioSlope = stIspExposureAttr.stAuto.u16HistRatioSlope;
    for (i = 0; i < AE_WEIGHT_ROW; i++)
    {
        for (j = 0; j < AE_WEIGHT_COLUMN; j++)
        {
            g_stPreviousPara.stExposure.au8Weight[i][j] = stIspExposureAttr.stAuto.au8Weight[i][j];
        }
    }
    s32Ret = HI_MPI_ISP_GetAERouteAttr(IspDev, &stIspAeRoute);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetAERouteAttr failed\n");
        return HI_FAILURE;
    }
    g_stPreviousPara.stExposure.s32TotalNum = stIspAeRoute.u32TotalNum;
    for (i = 0; i < stIspAeRoute.u32TotalNum; i++)
    {
        g_stPreviousPara.stExposure.astRouteNode[i].u32IntTime = stIspAeRoute.astRouteNode[i].u32IntTime;
        g_stPreviousPara.stExposure.astRouteNode[i].u32SysGain = stIspAeRoute.astRouteNode[i].u32SysGain;
    }

	/**************AWB***********/
	s32Ret = HI_MPI_ISP_GetStatisticsConfig(IspDev, &stIspStatisticsCfg);
	if (HI_SUCCESS != s32Ret)
    	{
        	printf("HI_MPI_ISP_GetStatisticsConfig failed\n");
        	return HI_FAILURE;
    	}
	g_stPreviousPara.stAwb.u16BlackLevel = stIspStatisticsCfg.stWBCfg.stBayerCfg.u16BlackLevel;
	s32Ret = HI_MPI_ISP_GetWBAttr(IspDev, &pstAWBAttr);
	if (HI_SUCCESS != s32Ret)
    	{
        	printf("HI_MPI_ISP_GetWBAttr failed\n");
        	return HI_FAILURE;
    	}
	g_stPreviousPara.stAwb.bEnable = pstAWBAttr.stAuto.stCbCrTrack.bEnable;
	g_stPreviousPara.stAwb.u8OpType = pstAWBAttr.enOpType;
	for(i = 0; i <16; i++)
	{
	
		g_stPreviousPara.stAwb.au16CbMax[i] = pstAWBAttr.stAuto.stCbCrTrack.au16CbMax[i];
	
		g_stPreviousPara.stAwb.au16CbMin[i]  = pstAWBAttr.stAuto.stCbCrTrack.au16CbMin[i];
	
		g_stPreviousPara.stAwb.au16CrMax[i]  =  pstAWBAttr.stAuto.stCbCrTrack.au16CrMax[i];
	
		g_stPreviousPara.stAwb.au16CrMin[i]   =  pstAWBAttr.stAuto.stCbCrTrack.au16CrMin[i];
	
	}

	/**************Black_Level***********/
	s32Ret = HI_MPI_ISP_GetBlackLevelAttr(IspDev, &stBlackLevel);
	if (HI_SUCCESS != s32Ret)
    	{
        	printf("HI_MPI_ISP_GetBlackLevelAttr failed\n");
        	return HI_FAILURE;
    	}
	for(i = 0; i < 4; i++)
	{
		g_stPreviousPara.stBlackLevel.au16BlackLevel[i] = stBlackLevel.au16BlackLevel[i];
	}

	/**************Shading**************/
	s32Ret = HI_MPI_ISP_GetMeshShadingAttr(IspDev, &pstShadingAttr);
	if (HI_SUCCESS != s32Ret)
    	{
        	printf("HI_MPI_ISP_GetMeshShadingAttr failed\n");
        	return HI_FAILURE;
    	}
	g_stPreviousPara.stShading.bEnable = pstShadingAttr.bEnable;
	g_stPreviousPara.stShading.enOpType = pstShadingAttr.enOpType;

    /****************saturation****************/
    s32Ret = HI_MPI_ISP_GetSaturationAttr(IspDev, &stIspSaturationAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetSaturationAttr failed\n");
        return HI_FAILURE;
    }
    g_stPreviousPara.stSaturation.u8OpType = stIspSaturationAttr.enOpType;
    g_stPreviousPara.stSaturation.u8ManuSaturation = stIspSaturationAttr.stManual.u8Saturation;
    for (i = 0; i < 16; i++)
    {
        g_stPreviousPara.stSaturation.u8Saturation[i] = stIspSaturationAttr.stAuto.au8Sat[i];
    }
	//AWB
	s32Ret = HI_MPI_ISP_GetWBAttr(IspDev,&pstAWBAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetWBAttr failed\n");
        return HI_FAILURE;
    }
	g_stPreviousPara.stAwb.u8OpType = pstAWBAttr.enOpType;
    s32Ret = HI_MPI_VI_GetDCIParam(ViDev, &stViDciParam);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VI_GetDCIParam failed\n");
        return HI_FAILURE;
    }
    g_stPreviousPara.stDci.bDCIEnable = stViDciParam.bEnable;
    g_stPreviousPara.stDci.u32DCIBlackGain = stViDciParam.u32BlackGain;
    g_stPreviousPara.stDci.u32DCIContrastGain = stViDciParam.u32ContrastGain;
    g_stPreviousPara.stDci.u32DCILightGain = stViDciParam.u32LightGain;

    /****************DRC****************/
    s32Ret = HI_MPI_ISP_GetDRCAttr(IspDev, &stIspDrcAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetDRCAttr failed\n");
        return HI_FAILURE;
    }
    g_stPreviousPara.stDrc.bEnable = stIspDrcAttr.bEnable;
    g_stPreviousPara.stDrc.u8OpType = stIspDrcAttr.enOpType;
    g_stPreviousPara.stDrc.u8Strength = stIspDrcAttr.stManual.u8Strength;
    g_stPreviousPara.stDrc.u8SpatialVar = stIspDrcAttr.u8SpatialVar;
    g_stPreviousPara.stDrc.u8RangeVar = stIspDrcAttr.u8RangeVar;
    g_stPreviousPara.stDrc.u8Asymmetry = stIspDrcAttr.u8Asymmetry;
    g_stPreviousPara.stDrc.u8SecondPole = stIspDrcAttr.u8SecondPole;
    g_stPreviousPara.stDrc.u8Stretch = stIspDrcAttr.u8Stretch;
    g_stPreviousPara.stDrc.u8Compress = stIspDrcAttr.u8Compress;
    g_stPreviousPara.stDrc.u8PDStrength = stIspDrcAttr.u8PDStrength;
    g_stPreviousPara.stDrc.u8LocalMixingBrigtht = stIspDrcAttr.u8LocalMixingBrigtht;
    g_stPreviousPara.stDrc.u8LocalMixingDark = stIspDrcAttr.u8LocalMixingDark;
 
    return HI_SUCCESS;
}


HI_S32 Sceneauto_SetTrafficMode()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    ISP_DEV IspDev;
    ISP_DRC_ATTR_S stIspDrcAttr;
    ISP_GAMMA_ATTR_S stIspGammaAttr;
    ISP_WDR_EXPOSURE_ATTR_S stIspWdrExposureAttr;
    ISP_SHARPEN_ATTR_S stIspYuvSharpenAttr;
	VI_DCI_PARAM_S stViDciParam;

    IspDev = g_stINIPara.stMpInfo.s32IspDev;


    if(g_eLastSpecialScene != g_eSpecialScene)
    {
		//In this part ,parameters will set once ,they won't be changed related with ISO or exposure.
        s32Ret = Sceneauto_SetPreviousPara();
        if (HI_SUCCESS != s32Ret)
	    {
	        printf("Sceneauto_SetPreviousPara failed\n");
	    }
		/*****set DCI param*****/
	    s32Ret = HI_MPI_VI_GetDCIParam(IspDev, &stViDciParam);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_VI_GetDCIParam failed\n");
	        return HI_FAILURE;
	    }
	    g_stPreviousPara.stDci.bDCIEnable = stViDciParam.bEnable;
	    g_stPreviousPara.stDci.u32DCIBlackGain = stViDciParam.u32BlackGain;
	    g_stPreviousPara.stDci.u32DCIContrastGain = stViDciParam.u32ContrastGain;
	    g_stPreviousPara.stDci.u32DCILightGain = stViDciParam.u32LightGain;
	    stViDciParam.bEnable = g_stINIPara.stIniTraffic.bDCIEnable;
	    stViDciParam.u32BlackGain = g_stINIPara.stIniTraffic.u32DCIBlackGain;
	    stViDciParam.u32ContrastGain = g_stINIPara.stIniTraffic.u32DCIContrastGain;
	    stViDciParam.u32LightGain = g_stINIPara.stIniTraffic.u32DCILightGain;
	    s32Ret = HI_MPI_VI_SetDCIParam(IspDev, &stViDciParam);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_VI_SetDCIParam failed\n");
	        return HI_FAILURE;
	    }

	    /*****set DRC param*****/
	    s32Ret = HI_MPI_ISP_GetDRCAttr(IspDev, &stIspDrcAttr);    
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_GetDRCAttr failed\n");
	        return HI_FAILURE;
	    }
        g_stPreviousPara.stDrc.u8SpatialVar = stIspDrcAttr.u8SpatialVar;
        g_stPreviousPara.stDrc.u8RangeVar = stIspDrcAttr.u8RangeVar;
        g_stPreviousPara.stDrc.u8Asymmetry = stIspDrcAttr.u8Asymmetry;
        g_stPreviousPara.stDrc.u8SecondPole = stIspDrcAttr.u8SecondPole;
        g_stPreviousPara.stDrc.u8Stretch = stIspDrcAttr.u8Stretch;
        g_stPreviousPara.stDrc.u8Compress = stIspDrcAttr.u8Compress;
        g_stPreviousPara.stDrc.u8PDStrength = stIspDrcAttr.u8PDStrength;
        g_stPreviousPara.stDrc.u8LocalMixingBrigtht = stIspDrcAttr.u8LocalMixingBrigtht;
        g_stPreviousPara.stDrc.u8LocalMixingDark = stIspDrcAttr.u8LocalMixingDark;

	    stIspDrcAttr.u8SpatialVar = g_stINIPara.stIniTraffic.u8SpatialVar;
	    stIspDrcAttr.u8RangeVar = g_stINIPara.stIniTraffic.u8RangeVar;
	    stIspDrcAttr.u8Asymmetry = g_stINIPara.stIniTraffic.u8Asymmetry;
	    stIspDrcAttr.u8SecondPole = g_stINIPara.stIniTraffic.u8SecondPole;
	    stIspDrcAttr.u8Stretch = g_stINIPara.stIniTraffic.u8Stretch;
	    stIspDrcAttr.u8Compress = g_stINIPara.stIniTraffic.u8Compress;
	    stIspDrcAttr.u8PDStrength = g_stINIPara.stIniTraffic.u8PDStrength;
	    stIspDrcAttr.u8LocalMixingBrigtht = g_stINIPara.stIniTraffic.u8LocalMixingBrigtht;
	    stIspDrcAttr.u8LocalMixingDark = g_stINIPara.stIniTraffic.u8LocalMixingDark;
	    
	    s32Ret = HI_MPI_ISP_SetDRCAttr(IspDev, &stIspDrcAttr);    
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_SetDRCAttr failed\n");
	        return HI_FAILURE;
	    }


	    /*****set WDR Exposure param*****/
	    s32Ret = HI_MPI_ISP_GetWDRExposureAttr(IspDev, &stIspWdrExposureAttr);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_GetWDRExposureAttr failed\n");
	        return HI_FAILURE;
	    }
	    stIspWdrExposureAttr.enExpRatioType = (ISP_OP_TYPE_E)g_stINIPara.stIniTraffic.u8ExpRatioType;
	  
	    stIspWdrExposureAttr.u32ExpRatio = g_stINIPara.stIniTraffic.u32ExpRatio;
	 
	    stIspWdrExposureAttr.u32ExpRatioMax = g_stINIPara.stIniTraffic.u32ExpRatioMax;
	    stIspWdrExposureAttr.u32ExpRatioMin = g_stINIPara.stIniTraffic.u32ExpRatioMin;
	    s32Ret = HI_MPI_ISP_SetWDRExposureAttr(IspDev, &stIspWdrExposureAttr);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_SetWDRExposureAttr failed\n");
	        return HI_FAILURE;
	    }


	    /*****set Gamma param*****/
	    s32Ret = HI_MPI_ISP_GetGammaAttr(IspDev, &stIspGammaAttr);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_GetGammaAttr failed\n");
	        return HI_FAILURE;
	    }

	    //save ths last gamma
	    g_stPreviousPara.stGamma.u8CurveType = stIspGammaAttr.enCurveType;
	    for (i = 0; i < 257; i++)
	    {
	        g_stPreviousPara.stGamma.u16Table[i] = stIspGammaAttr.u16Table[i];
	    }

	    stIspGammaAttr.enCurveType = ISP_GAMMA_CURVE_USER_DEFINE;
	    for (i = 0; i < 257 ; i++)
	    {
	        stIspGammaAttr.u16Table[i] = g_stINIPara.stIniTraffic.u16GammaTable[i];
	    }
	    s32Ret = HI_MPI_ISP_SetGammaAttr(IspDev, &stIspGammaAttr);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_SetGammaAttr failed\n");
	        return HI_FAILURE;
	    }

	    /*****set YuvSharpen param*****/
	    s32Ret = HI_MPI_ISP_GetSharpenAttr(IspDev, &stIspYuvSharpenAttr);    
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_GetYuvSharpenAttr failed\n");
	        return HI_FAILURE;
	    }
	    for (i = 0; i < EXPOSURE_LEVEL; i++)
	    {
	        stIspYuvSharpenAttr.stAuto.au16SharpenUd[i] = g_stINIPara.stIniTraffic.au16SharpenUd[i];
	        stIspYuvSharpenAttr.stAuto.au8SharpenD[i] = g_stINIPara.stIniTraffic.au8SharpenD[i];
	        stIspYuvSharpenAttr.stAuto.au8TextureThr[i] = g_stINIPara.stIniTraffic.au8TextureThr[i];
	        stIspYuvSharpenAttr.stAuto.au8SharpenEdge[i] = g_stINIPara.stIniTraffic.au8SharpenEdge[i];
	        stIspYuvSharpenAttr.stAuto.au8EdgeThr[i] = g_stINIPara.stIniTraffic.au8EdgeThr[i];
	        stIspYuvSharpenAttr.stAuto.au8OverShoot[i] = g_stINIPara.stIniTraffic.au8OverShoot[i];
	        stIspYuvSharpenAttr.stAuto.au8UnderShoot[i] = g_stINIPara.stIniTraffic.au8UnderShoot[i];
			stIspYuvSharpenAttr.stAuto.au8shootSupStr[i] = g_stINIPara.stIniTraffic.au8shootSupStr[i];
			stIspYuvSharpenAttr.stAuto.au8DetailCtrl[i] = g_stINIPara.stIniTraffic.au8DetailCtrl[i];
	    }
	    s32Ret = HI_MPI_ISP_SetSharpenAttr(IspDev, &stIspYuvSharpenAttr);    
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_SetYuvSharpenAttr failed\n");
	        return HI_FAILURE;
	    }
	}

    return HI_SUCCESS;
}


HI_S32 Sceneauto_SetSpecialNone(HI_U32 u32ISO,HI_U32 u32Exposure,HI_U32 u32Bitrate)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_DEV IspDev;
    IspDev = g_stINIPara.stMpInfo.s32IspDev;
    static HI_U32 u32LastBitrate = 0;
    static HI_U32 u32LastExposure = 0;
    static HI_U32 u32LastISO = 0;
    //printf("g_eLastSpecialScene=%d,g_eSpecialScene=%d\n",g_eLastSpecialScene,g_eSpecialScene);
    //if((g_eLastSpecialScene != g_eSpecialScene))
    if((g_eLastSpecialScene != g_eSpecialScene)||(g_bSceneautoFirstStart== HI_TRUE))
    {
		//In this part ,parameters will set once ,they won't be changed related with ISO or exposure.
    	//printf("here");
    	u32LastBitrate = 0;
        u32LastExposure = 0;
        u32LastISO = 0;
        s32Ret = Sceneauto_SetPreviousPara();
        if (HI_SUCCESS != s32Ret)
	    {
	        printf("Sceneauto_SetPreviousPara failed\n");
			return HI_FAILURE;
	    }
    	s32Ret = Sceneauto_SetDefog(IspDev);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("Sceneauto_SetDefog failed\n");
			return HI_FAILURE;
	    }
		s32Ret = Sceneauto_SetShading(IspDev);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("Sceneauto_SetShading failed\n");
			return HI_FAILURE;
	    }
		s32Ret = Sceneauto_SetDci(IspDev);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("Sceneauto_SetDci failed\n");
			return HI_FAILURE;
	    }

	    s32Ret = Sceneauto_Set2DNR(IspDev);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("Sceneauto_Set2DNR failed\n");
			return HI_FAILURE;
	    }
    }
    //else
    {
    	//if you need to change the param related with ISO or Exposure,write here
    	//printf("u32ISO:%d,u32LastISO:%d,u32Exposure:%d,u32LastExposure:%d,u32Bitrate:%d,u32LastBitrate:%d\n",
    	//        u32ISO,u32LastISO,u32Exposure,u32LastExposure,u32Bitrate,u32LastBitrate);
    	if(u32ISO != u32LastISO)
		{
			/*********************set AWB releated with ISO*******************/
		   	s32Ret = Sceneauto_SetAWB(IspDev, u32ISO);
		   	if (HI_SUCCESS != s32Ret)
	         {
	            	printf("Sceneauto_SetAWB failed\n");
					return HI_FAILURE;
	         }	 

		 	/*********************set BlackLevel releated with ISO*******************/
		   	s32Ret = Sceneauto_SetBlackLevel(IspDev, u32ISO);
		   	if (HI_SUCCESS != s32Ret)
	       	{
	            	printf("Sceneauto_SetBlackLevel failed\n");
					return HI_FAILURE;
	        }
		}
		/*********************set gamma releated with exposure*********************/
        s32Ret = Sceneauto_SetGamma(IspDev, u32Exposure,u32LastExposure);
        if (HI_SUCCESS != s32Ret)
        {
            printf("Sceneauto_SetGamma failed\n");
			return HI_FAILURE;
        }
        if (u32Exposure != u32LastExposure)
        {
                       
            /*********************set ae param releated with exposure******************/
            s32Ret = Sceneauto_SetAeRelatedExp(IspDev, u32Exposure, u32LastExposure);
           	if (HI_SUCCESS != s32Ret)
            {
                printf("Sceneauto_SetAeRelatedExp failed\n");
				return HI_FAILURE;
            }

            /*********************set dp param releated with exposure******************/
            s32Ret = Sceneauto_SetDP(IspDev, u32Exposure, u32LastExposure);
            if (HI_SUCCESS != s32Ret)
            {
                printf("Sceneauto_SetDP failed\n");
				return HI_FAILURE;
            }
           
        }
        if(g_stIspWdrMode.enWDRMode != WDR_MODE_NONE)
		{
		    s32Ret = Sceneauto_SetDrc(IspDev,u32Exposure, u32LastExposure);
            if (HI_SUCCESS != s32Ret)
            {
                printf("Sceneauto_SetDRC failed\n");
				return HI_FAILURE;
            }
        }
        if ((u32Bitrate != u32LastBitrate) || (u32Exposure != u32LastExposure))
        {	   
            /*********************set sharpen related with bitrate and exposure*********************/
            s32Ret = Sceneauto_SetSharpen(IspDev, u32Bitrate, u32LastBitrate, u32Exposure, u32LastExposure);
            if (HI_SUCCESS != s32Ret)
            {
                printf("Sceneauto_SetSharpen failed\n");
				return HI_FAILURE;
            } 
        }
        u32LastISO = u32ISO;
        u32LastExposure = u32Exposure;
        u32LastBitrate = u32Bitrate;
        
    }

    return HI_SUCCESS;
}

HI_S32 Sceneauto_SetHLC()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    ISP_DEV IspDev;
    VI_DEV ViDev;
    ISP_EXPOSURE_ATTR_S stIspExposureAttr;
    VI_DCI_PARAM_S stViDciAttr;
    ISP_DRC_ATTR_S stIspDrcAttr;
    ISP_SATURATION_ATTR_S stIspSaturationAttr;
    ISP_GAMMA_ATTR_S stIspGammaAttr;

    IspDev = g_stINIPara.stMpInfo.s32IspDev;
    ViDev = g_stINIPara.stMpInfo.s32ViDev;


	if(g_eLastSpecialScene != g_eSpecialScene)
    {
		//In this part ,parameters will set once ,they won't be changed related with ISO or exposure.
    	
	    /*****set AE param*****/
        s32Ret = Sceneauto_SetPreviousPara();
        if (HI_SUCCESS != s32Ret)
	    {
	        printf("Sceneauto_SetPreviousPara failed\n");
	    }
	    s32Ret = HI_MPI_ISP_GetExposureAttr(IspDev, &stIspExposureAttr);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_GetExposureAttr failed\n");
	        return HI_FAILURE;
	    }
	    stIspExposureAttr.stAuto.u8Compensation = g_stINIPara.stIniHlc.u8ExpCompensation;
	    stIspExposureAttr.stAuto.stAEDelayAttr.u16BlackDelayFrame = g_stINIPara.stIniHlc.u16BlackDelayFrame;
	    stIspExposureAttr.stAuto.stAEDelayAttr.u16WhiteDelayFrame = g_stINIPara.stIniHlc.u16WhiteDelayFrame;
	    stIspExposureAttr.stAuto.u8Speed = g_stINIPara.stIniHlc.u8Speed;
	    stIspExposureAttr.stAuto.u8Tolerance = g_stINIPara.stIniHlc.u8Tolerance;
	    stIspExposureAttr.stAuto.u16HistRatioSlope = g_stINIPara.stIniHlc.u16HistRatioSlope;
	    stIspExposureAttr.stAuto.u8MaxHistOffset = g_stINIPara.stIniHlc.u8MaxHistOffset;
	    s32Ret = HI_MPI_ISP_SetExposureAttr(IspDev, &stIspExposureAttr);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_SetExposureAttr failed\n");
	        return HI_FAILURE;
	    }

	    /*****set DCI param*****/
	    s32Ret = HI_MPI_VI_GetDCIParam(ViDev, &stViDciAttr);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_VI_GetDCIParam failed\n");
	        return HI_FAILURE;
	    }
	    stViDciAttr.bEnable = g_stINIPara.stIniHlc.bDCIEnable;
	    stViDciAttr.u32BlackGain = g_stINIPara.stIniHlc.u32DCIBlackGain;
	    stViDciAttr.u32ContrastGain = g_stINIPara.stIniHlc.u32DCIContrastGain;
	    stViDciAttr.u32LightGain = g_stINIPara.stIniHlc.u32DCILightGain;
	    s32Ret = HI_MPI_VI_SetDCIParam(ViDev, &stViDciAttr);
	    if (HI_SUCCESS != s32Ret)
    	{
	        printf("HI_MPI_VI_SetDCIParam failed\n");
	        return HI_FAILURE;
	    }

	    /*****set DRC param*****/
	    s32Ret = HI_MPI_ISP_GetDRCAttr(IspDev, &stIspDrcAttr);    
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_GetDRCAttr failed\n");
	        return HI_FAILURE;
	    }
	    stIspDrcAttr.bEnable = g_stINIPara.stIniHlc.bDRCEnable;
	    if (HI_TRUE == g_stINIPara.stIniHlc.bDRCManulEnable)
	    {
	        stIspDrcAttr.enOpType = OP_TYPE_MANUAL;
	        stIspDrcAttr.stManual.u8Strength = g_stINIPara.stIniHlc.u8DRCStrengthTarget;
	    }
	    else
	    {
	        stIspDrcAttr.enOpType = OP_TYPE_AUTO;
	    }
	    s32Ret = HI_MPI_ISP_SetDRCAttr(IspDev, &stIspDrcAttr);    
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_SetDRCAttr failed\n");
	        return HI_FAILURE;
	    }

	    /*****set Saturation param*****/
	    s32Ret = HI_MPI_ISP_GetSaturationAttr(IspDev, &stIspSaturationAttr);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_GetSaturationAttr failed\n");
	        return HI_FAILURE;
	    }
	    for (i = 0; i < 16; i++)
	    {
	        stIspSaturationAttr.stAuto.au8Sat[i] = g_stINIPara.stIniHlc.u8Saturation[i];
	    }
	    s32Ret = HI_MPI_ISP_SetSaturationAttr(IspDev, &stIspSaturationAttr);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_SetSaturationAttr failed\n");
	        return HI_FAILURE;
	    }

	    /*****set Gamma param*****/
	    s32Ret = HI_MPI_ISP_GetGammaAttr(IspDev, &stIspGammaAttr);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_GetGammaAttr failed\n");
	        return HI_FAILURE;
	    }

	    //save ths last gamma
	    g_stPreviousPara.stGamma.u8CurveType = stIspGammaAttr.enCurveType;
	    for (i = 0; i < 257; i++)
	    {
	        g_stPreviousPara.stGamma.u16Table[i] = stIspGammaAttr.u16Table[i];
	    }

	    stIspGammaAttr.enCurveType = ISP_GAMMA_CURVE_USER_DEFINE;
	    for (i = 0; i < 257 ; i++)
	    {
	        stIspGammaAttr.u16Table[i] = g_stINIPara.stIniHlc.u16GammaTable[i];
	    }
	    s32Ret = HI_MPI_ISP_SetGammaAttr(IspDev, &stIspGammaAttr);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_SetGammaAttr failed\n");
	        return HI_FAILURE;
	    }
	}
	//else
	//{
		
	//}
    
    return HI_SUCCESS;
}

HI_S32 Sceneauto_SetIR()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i,j;
    ISP_DEV IspDev;
    VI_DEV ViDev;

    ISP_EXPOSURE_ATTR_S stIspExposureAttr;
    VI_DCI_PARAM_S stViDciParam;
	ISP_DP_DYNAMIC_ATTR_S stIspDpAttr;
  
    ISP_SATURATION_ATTR_S stIspSaturationAttr;
	ISP_WB_ATTR_S stIspWBAttr;
    ISP_SHARPEN_ATTR_S stIspBayerSharpenAttr;
   
    ISP_GAMMA_ATTR_S stIspGammaAttr;

    IspDev = g_stINIPara.stMpInfo.s32IspDev;
    ViDev = g_stINIPara.stMpInfo.s32ViDev;
    
	if(g_eLastSpecialScene != g_eSpecialScene)
    {
		//In this part ,parameters will set once ,they won't be changed related with ISO or exposure.
    	
    	/*****set AE param*****/
    	s32Ret = Sceneauto_SetPreviousPara();
        if (HI_SUCCESS != s32Ret)
	    {
	        printf("Sceneauto_SetPreviousPara failed\n");
	    }
    	s32Ret = HI_MPI_ISP_GetExposureAttr(IspDev, &stIspExposureAttr);
    	if (HI_SUCCESS != s32Ret)
    	{
        	printf("HI_MPI_ISP_GetExposureAttr failed\n");
        	return HI_FAILURE;
	    }
	    stIspExposureAttr.stAuto.u16HistRatioSlope = g_stINIPara.stIniIr.u16HistRatioSlope;
	    stIspExposureAttr.stAuto.stAEDelayAttr.u16BlackDelayFrame = g_stINIPara.stIniIr.u16BlackDelayFrame;
	    stIspExposureAttr.stAuto.stAEDelayAttr.u16WhiteDelayFrame = g_stINIPara.stIniIr.u16WhiteDelayFrame;
	    stIspExposureAttr.stAuto.u8Speed = g_stINIPara.stIniIr.u8Speed;
	    stIspExposureAttr.stAuto.u8Tolerance = g_stINIPara.stIniIr.u8Tolerance;
	    for (i = 0; i < AE_WEIGHT_ROW; i++)
	    {
	        for (j = 0; j < AE_WEIGHT_COLUMN; j++)
	        {
	            stIspExposureAttr.stAuto.au8Weight[i][j] = g_stINIPara.stIniIr.au8Weight[i][j];
	        }
	    }
	    s32Ret = HI_MPI_ISP_SetExposureAttr(IspDev, &stIspExposureAttr);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_SetExposureAttr failed\n");
	        return HI_FAILURE;
	    }

	    /*****set DCI param*****/
	    s32Ret = HI_MPI_VI_GetDCIParam(ViDev, &stViDciParam);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_VI_GetDCIParam failed\n");
	        return HI_FAILURE;
	    }
		g_stPreviousPara.stDci.bDCIEnable = stViDciParam.bEnable;
		g_stPreviousPara.stDci.u32DCIBlackGain = stViDciParam.u32BlackGain;
		g_stPreviousPara.stDci.u32DCIContrastGain = stViDciParam.u32ContrastGain;
		g_stPreviousPara.stDci.u32DCILightGain = stViDciParam.u32LightGain;
	    stViDciParam.bEnable = g_stINIPara.stIniIr.bDCIEnable;
	    stViDciParam.u32BlackGain = g_stINIPara.stIniIr.u32DCIBlackGain;
	    stViDciParam.u32ContrastGain = g_stINIPara.stIniIr.u32DCIContrastGain;
	    stViDciParam.u32LightGain = g_stINIPara.stIniIr.u32DCILightGain;
	    s32Ret = HI_MPI_VI_SetDCIParam(ViDev, &stViDciParam);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_VI_SetDCIParam failed\n");
	        return HI_FAILURE;
	    }
	    
	    /*****set DP param*****/
	    s32Ret = HI_MPI_ISP_GetDPDynamicAttr(IspDev, &stIspDpAttr);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_GetDPAttr failed\n");
	        return HI_FAILURE;
	    }
		g_stPreviousPara.stDp.bSupTwinkleEn = stIspDpAttr.bSupTwinkleEn;
		g_stPreviousPara.stDp.SoftThr = stIspDpAttr.s8SoftThr;
		g_stPreviousPara.stDp.SoftSlope = stIspDpAttr.u8SoftSlope;
		stIspDpAttr.bSupTwinkleEn= g_stINIPara.stIniIr.bSupTwinkleEn;
		stIspDpAttr.s8SoftThr= g_stINIPara.stIniIr.SoftThr;
		stIspDpAttr.u8SoftSlope= g_stINIPara.stIniIr.SoftSlope;
	    s32Ret = HI_MPI_ISP_SetDPDynamicAttr(IspDev, &stIspDpAttr);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_SetDPAttr failed\n");
	        return HI_FAILURE;
	    }

	    /*****set Saturation param*****/
	    s32Ret = HI_MPI_ISP_GetSaturationAttr(IspDev, &stIspSaturationAttr);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_GetSaturationAttr failed\n");
	        return HI_FAILURE;
	    }
		g_stPreviousPara.stSaturation.u8OpType = stIspSaturationAttr.enOpType;
		g_stPreviousPara.stSaturation.u8ManuSaturation = stIspSaturationAttr.stManual.u8Saturation;
	    stIspSaturationAttr.enOpType = OP_TYPE_MANUAL;
	    stIspSaturationAttr.stManual.u8Saturation = 0;
	    s32Ret = HI_MPI_ISP_SetSaturationAttr(IspDev, &stIspSaturationAttr);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_SetSaturationAttr failed\n");
	        return HI_FAILURE;
	    }

		/*****set WB param*****/
		s32Ret = HI_MPI_ISP_GetWBAttr(IspDev, &stIspWBAttr);
		if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_GetWBAttr failed\n");
	        return HI_FAILURE;
	    }
		g_stPreviousPara.stAwb.u8OpType = stIspWBAttr.enOpType;
		stIspWBAttr.enOpType = OP_TYPE_MANUAL;
		stIspWBAttr.stManual.u16Bgain = 256;
		stIspWBAttr.stManual.u16Gbgain = 256;
		stIspWBAttr.stManual.u16Grgain = 256;
		stIspWBAttr.stManual.u16Rgain = 256;
		s32Ret = HI_MPI_ISP_SetWBAttr(IspDev, &stIspWBAttr);
		if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_SetWBAttr failed\n");
	        return HI_FAILURE;
	    }
	    /*****set BayerSharpen param*****/
	    s32Ret = HI_MPI_ISP_GetSharpenAttr(IspDev, &stIspBayerSharpenAttr);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_GetBayerSharpenAttr failed\n");
	        return HI_FAILURE;
	    }
	    for (i = 0; i < EXPOSURE_LEVEL; i++)
	    {
	        stIspBayerSharpenAttr.stAuto.au16SharpenUd[i] = g_stINIPara.stIniIr.au16SharpenUd[i];
	        stIspBayerSharpenAttr.stAuto.au8SharpenD[i]  = g_stINIPara.stIniIr.au8SharpenD[i];
	        stIspBayerSharpenAttr.stAuto.au8TextureThr[i] = g_stINIPara.stIniIr.au8TextureThr[i];
			stIspBayerSharpenAttr.stAuto.au8SharpenEdge[i] = g_stINIPara.stIniIr.au8SharpenEdge[i];
			stIspBayerSharpenAttr.stAuto.au8OverShoot[i] =   g_stINIPara.stIniIr.au8OverShoot[i] ;
			stIspBayerSharpenAttr.stAuto.au8UnderShoot[i]=   g_stINIPara.stIniIr.au8UnderShoot[i];
			stIspBayerSharpenAttr.stAuto.au8shootSupStr[i]=  g_stINIPara.stIniIr.au8shootSupStr[i];		
			stIspBayerSharpenAttr.stAuto.au8DetailCtrl[i] =  g_stINIPara.stIniIr.au8DetailCtrl[i];
	    }
	    s32Ret = HI_MPI_ISP_SetSharpenAttr(IspDev, &stIspBayerSharpenAttr);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_SetBayerSharpenAttr failed\n");
	        return HI_FAILURE;
	    }

	    /*****set Gamma param*****/
	    s32Ret = HI_MPI_ISP_GetGammaAttr(IspDev, &stIspGammaAttr);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_GetGammaAttr failed\n");
	        return HI_FAILURE;
	    }
	    //printf("stIspGammaAttr.u16Table[i] = %u\n", stIspGammaAttr.u16Table[1]);
	    g_stPreviousPara.stGamma.u8CurveType = stIspGammaAttr.enCurveType;
	    for (i = 0; i < 257; i++)
	    {
	        g_stPreviousPara.stGamma.u16Table[i] = stIspGammaAttr.u16Table[i];
	    }	    
	    stIspGammaAttr.enCurveType = ISP_GAMMA_CURVE_USER_DEFINE;
	    for (i = 0; i < 257; i++)
	    {
	        stIspGammaAttr.u16Table[i] = g_stINIPara.stIniIr.u16GammaTable[i];
	    }
	    s32Ret = HI_MPI_ISP_SetGammaAttr(IspDev, &stIspGammaAttr);
	    if (HI_SUCCESS != s32Ret)
	    {
	        printf("HI_MPI_ISP_SetGammaAttr failed\n");
	        return HI_FAILURE;
	    }
   	}
   //	else
   	//{
   		//if you need to change the param related with ISO or Exposure,write here
   		
   //}
    return HI_SUCCESS;
}



void *SceneAuto_3DnrThread(void* pVoid)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Iso = 0;
    HI_U32 u32LastIso = 0;
	HI_U32 u32MGValue = 0;
    HI_S32 s32FrameCount = 0;
    ISP_DEV IspDev = 0;
    VPSS_GRP VpssGrp;
    ISP_EXP_INFO_S stIspExpInfo;
    ISP_WDR_MODE_S stIspWdrMode;
	ISP_EXPOSURE_ATTR_S pstExpAttr;
	s32Ret = HI_MPI_ISP_GetWDRMode(IspDev, &stIspWdrMode);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_ISP_GetWDRMode failed\n");		
	} 
	s32Ret = HI_MPI_ISP_GetExposureAttr(IspDev, &pstExpAttr);
	if (HI_SUCCESS != s32Ret)
	{
		printf("HI_MPI_ISP_GetExposureAttr failed\n");	
	} 
    prctl(PR_SET_NAME, (unsigned long)"SceneAuto_3DnrThread", 0, 0, 0);

    IspDev = g_stINIPara.stMpInfo.s32IspDev;
    VpssGrp = g_stINIPara.stMpInfo.s32VpssGrp;

    while (HI_TRUE == g_b3DnrThreadFlag)
    {
        //detect whether the scene mode is changed,if the scene mode is changed , the last iso value should be set 0 
        if(g_eLastSpecialScene != g_eSpecialScene)
        {
            u32LastIso = 0;
        }
        //calculate iso value (iso = AGain * DGain * ISPDGain * 100 >> 30)
        s32Ret = HI_MPI_ISP_QueryExposureInfo(IspDev, &stIspExpInfo);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_ISP_QueryExposureInfo failed\n");
        }
        if((g_stINIPara.stIni3dnr.BoolRefMGValue)&&((WDR_MODE_NONE != stIspWdrMode.enWDRMode))&&(SCENEAUTO_SPECIAL_SCENE_NONE == g_eSpecialScene))
        {
            u32MGValue = ((HI_U64)((HI_U64)GetMG()*stIspExpInfo.u32ISO))>>16;
		    u32Iso = u32MGValue/100;
            s32FrameCount++;
            if(s32FrameCount>=10)
            {
                //printf("u32MGValue = %d\n",u32Iso);
                s32FrameCount = 0;
            }
        }
        else
        {
            u32Iso = (HI_U64)stIspExpInfo.u32AGain * (HI_U64)stIspExpInfo.u32DGain * (HI_U64)stIspExpInfo.u32ISPDGain * 100 
                  >>30;
        }
        //set 3dnr
        if ((u32Iso != u32LastIso))
        {
			if (SCENEAUTO_SPECIAL_SCENE_IR == g_eSpecialScene) 
            {            
              s32Ret = Sceneauto_Set3DNR(VpssGrp, u32Iso, g_stINIPara.stIniIr.stIni3dnr);
            }
			else if(SCENEAUTO_SPECIAL_SCENE_TRAFFIC == g_eSpecialScene)
            {
                //set Traffic 3DNR param
                s32Ret = Sceneauto_Set3DNR(VpssGrp, u32Iso, g_stINIPara.stIniTraffic.stTraffic3dnr);
                if (HI_SUCCESS != s32Ret)
                {
                    printf("Sceneauto_Set3DNR failed\n");
                }
            }
			else
			{
				s32Ret = Sceneauto_Set3DNR(VpssGrp, u32Iso, g_stINIPara.stIni3dnr);
	            if (HI_SUCCESS != s32Ret)
	            {
	                printf("Sceneauto_SetNormal3DNR failed\n");
	            }

			}
      

            u32LastIso = u32Iso;
            
        }
		
		hi_usleep(200000);
    }
    
    return NULL;
}

void * SceneAuto_SpecialThread(void* pVoid)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Exposure = 0;
    HI_U32 u32LastExposure = 0;
    HI_U32 u32DeltaExposure = 0;
    HI_U8 u8AveLum = 0;
    HI_U8 u8LastAveLum = 0;
    HI_U8 u8DelataAveLum = 0;
    HI_U8 u8DisCount = 0;

    ISP_DEV IspDev;
    ISP_EXP_INFO_S stIspExpInfo;
    ISP_DIS_ATTR_S stDisAttr;
    stDisAttr.bEnable = HI_TRUE;

    prctl(PR_SET_NAME, (unsigned long)"SceneautoSpecialThread", 0, 0, 0);

    IspDev = g_stINIPara.stMpInfo.s32IspDev;

    s32Ret = HI_MPI_ISP_QueryExposureInfo(IspDev, &stIspExpInfo);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_QueryExposureInfo failed\n");
    }
    u32Exposure = stIspExpInfo.u32Exposure;
    u32LastExposure = u32Exposure;
    u8AveLum = stIspExpInfo.u8AveLum;
    u8LastAveLum = u8AveLum;

    while (HI_TRUE == g_bSpecialThreadFlag)
    {
        s32Ret = HI_MPI_ISP_QueryExposureInfo(IspDev, &stIspExpInfo);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_ISP_QueryExposureInfo failed\n");
        }
        u32Exposure = stIspExpInfo.u32Exposure;
        u8AveLum = stIspExpInfo.u8AveLum;

        if (u32LastExposure == 0)
        {
            u32LastExposure = u32Exposure;
            continue;
        }

        /*******disable DIC function when luminance and exposure changed instantly*********/
        if ((u32LastExposure != u32Exposure) || (u8LastAveLum != u8AveLum))
        {
            if (u32Exposure > u32LastExposure)
            {
                u32DeltaExposure = (u32Exposure * 256 / u32LastExposure) - 256;
            }
            else
            {
                u32DeltaExposure = (u32LastExposure * 256 / u32Exposure) - 256;
            }

            if (u8AveLum > u8LastAveLum)
            {
                u8DelataAveLum = u8AveLum - u8LastAveLum;
            }
            else
            {
                u8DelataAveLum = u8LastAveLum - u8AveLum;
            }

            if ((u32DeltaExposure > g_stINIPara.stThreshValue.u32DeltaDisExpThreash) || (u8DelataAveLum > g_stINIPara.stThreshValue.u32AveLumThresh))
            {

                if (stDisAttr.bEnable == HI_TRUE)
                {
                    printf("\n\n----------------DIS DISABLE--------------------\n\n");
                    stDisAttr.bEnable = HI_FALSE;
                    s32Ret = HI_MPI_ISP_SetDISAttr(IspDev, &stDisAttr);
                    if (HI_SUCCESS != s32Ret)
                    {
                        printf("HI_MPI_ISP_SetDISAttr failed\n");
                    }
                    sleep(1);
                }
                u8DisCount = 0;
            }
            else
            {
                if (stDisAttr.bEnable == HI_FALSE)
                {
                    u8DisCount++;
                    if (u8DisCount > 7)
                    {
                        printf("\n\n----------------DIS ENABLE--------------------\n\n");
                        stDisAttr.bEnable = HI_TRUE;
                        s32Ret = HI_MPI_ISP_SetDISAttr(IspDev, &stDisAttr);
                        if (HI_SUCCESS != s32Ret)
                        {
                            printf("HI_MPI_ISP_SetDISAttr failed\n");
                        }
                        u8DisCount = 0;
                    }
                }

            }
        }
		
		hi_usleep(200000);
    }

    return NULL;
}

void* SceneAuto_NormalThread(void* pVoid)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Exposure = 0;
    //HI_U32 u32LastExposure = 0;
    HI_U32 u32ISO = 0;
    //static HI_U32 u32LastISO = 0;
    HI_U32 u32Bitrate = 0;
    //HI_U32 u32LastBitrate = 0;
    ISP_DEV IspDev;
    ISP_EXP_INFO_S stIspExpInfo;
    ISP_INNER_STATE_INFO_S stIspInnerStateInfo;
    ISP_DRC_ATTR_S stIspDrcAttr;
    ISP_WDR_MODE_S stIspWdrMode;

    prctl(PR_SET_NAME, (unsigned long)"SceneautoNormalThread", 0, 0, 0);
    IspDev = g_stINIPara.stMpInfo.s32IspDev;

    
    while (HI_TRUE == g_bNormalThreadFlag)
    {
        /*********************calculate exposure value*********************/
        s32Ret = HI_MPI_ISP_QueryExposureInfo(IspDev, &stIspExpInfo);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_ISP_QueryExposureInfo failed\n");
        }

        s32Ret = HI_MPI_ISP_QueryInnerStateInfo(IspDev, &stIspInnerStateInfo);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_ISP_QueryExposureInfo failed\n");
        }
        if (stIspInnerStateInfo.u32DRCStrengthActual < g_stINIPara.stThreshValue.u32DRCStrengthThresh)
        {
            stIspInnerStateInfo.u32DRCStrengthActual = g_stINIPara.stThreshValue.u32DRCStrengthThresh;
        }
        
        s32Ret = HI_MPI_ISP_GetDRCAttr(IspDev, &stIspDrcAttr);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_ISP_GetDRCAttr failed\n");
        }

        s32Ret = HI_MPI_ISP_GetWDRMode(IspDev, &stIspWdrMode);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_ISP_GetWDRMode failed\n");
        }        
        g_stIspWdrMode.enWDRMode = stIspWdrMode.enWDRMode;
        
        if (WDR_MODE_NONE != stIspWdrMode.enWDRMode)
        {
            //in wdr mode, use iso as exposure value            
            u32Exposure = ((HI_U64)stIspExpInfo.u32AGain * (HI_U64)stIspExpInfo.u32DGain * (HI_U64)stIspExpInfo.u32ISPDGain * 100 >> 30)
                          * (HI_U64)stIspInnerStateInfo.u32DRCStrengthActual / (HI_U64)g_stINIPara.stThreshValue.u32DRCStrengthThresh;
	        u32ISO = ((HI_U64)stIspExpInfo.u32AGain * (HI_U64)stIspExpInfo.u32DGain * (HI_U64)stIspExpInfo.u32ISPDGain * 100) >> 30;
            
            if(g_stINIPara.BoolRefExporeTime)
            {
            	u32Exposure = ((HI_U64)stIspExpInfo.u32AGain * (HI_U64)stIspExpInfo.u32DGain * (HI_U64)stIspExpInfo.u32ISPDGain) 
                          * ((HI_U64)stIspExpInfo.u32ExpTime)>> 30;
            }
        }
        else
        {
            u32Exposure = ((HI_U64)stIspExpInfo.u32AGain * (HI_U64)stIspExpInfo.u32DGain * (HI_U64)stIspExpInfo.u32ISPDGain) 
                          * ((HI_U64)stIspExpInfo.u32ExpTime)>> 30;
	        u32ISO = ((HI_U64)stIspExpInfo.u32AGain * (HI_U64)stIspExpInfo.u32DGain * (HI_U64)stIspExpInfo.u32ISPDGain * 100)>> 30;
            if (HI_TRUE == stIspDrcAttr.bEnable)
            {
                u32Exposure = (HI_U64)u32Exposure * (HI_U64)stIspInnerStateInfo.u32DRCStrengthActual / (HI_U64)g_stINIPara.stThreshValue.u32DRCStrengthThresh;
            }
        }
		
		switch(g_eSpecialScene)
		{
			case   SCENEAUTO_SPECIAL_SCENE_IR:
				s32Ret = Sceneauto_SetIR();
	            if (HI_SUCCESS != s32Ret)
	            {
	                printf("Sceneauto_SetIR failed\n");
	                MUTEX_UNLOCK(g_stSceneautoLock);
	                return NULL;
	            }
	            break;
	        case SCENEAUTO_SPECIAL_SCENE_HLC:
	            s32Ret = Sceneauto_SetHLC();
	            if (HI_SUCCESS != s32Ret)
	            {
	                printf("Sceneauto_SetHLC failed\n");
	                MUTEX_UNLOCK(g_stSceneautoLock);
	                return NULL;
	            }
	            break;

			case SCENEAUTO_SPECIAL_SCENE_TRAFFIC:
				
				s32Ret = Sceneauto_SetTrafficMode();
				if (HI_SUCCESS != s32Ret)
				{
					printf("Sceneauto_SetTrafficMode failed\n");
					MUTEX_UNLOCK(g_stSceneautoLock);
					return NULL;
				}
				break;
	        case SCENEAUTO_SPECIAL_SCENE_NONE:
	            s32Ret = Sceneauto_SetSpecialNone(u32ISO,u32Exposure,u32Bitrate);
	            if (HI_SUCCESS != s32Ret)
	            {
	                MUTEX_UNLOCK(g_stSceneautoLock);
	                printf("Sceneauto_SetSpecialNone failed\n");
	                return NULL;
	            }
	            break;
	        default:
	            return NULL;
	            break;
		}

	    //u32LastISO = u32ISO;
        //u32LastExposure = u32Exposure;
        //u32LastBitrate = u32Bitrate;
        g_bSceneautoFirstStart = HI_FALSE;
        g_eLastSpecialScene = g_eSpecialScene;
        hi_usleep(1000000);
    }
    
    return NULL;
}

HI_S32 Sceneauto_SetDefaultParam()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    HI_U32 u32Exposure;
    HI_U32 u32ExpLevel = 0;
    VI_DEV ViDev;
    ISP_DEV IspDev;
    VI_DCI_PARAM_S stDciParam;
    //ISP_AE_ROUTE_S stAeRoute;
    ISP_GAMMA_ATTR_S stGammaAttr;
    ISP_EXPOSURE_ATTR_S stExposureAttr;
    ISP_EXP_INFO_S stIspExpInfo;
    
    ViDev = g_stINIPara.stMpInfo.s32ViDev;
    IspDev = g_stINIPara.stMpInfo.s32IspDev;

    /***************enable DCI*************/ 
    s32Ret = HI_MPI_VI_GetDCIParam(ViDev, &stDciParam);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VI_GetDCIParam failed\n");
        return HI_FAILURE;
    } 
    stDciParam.bEnable = g_stINIPara.stIniDci.bDCIEnable;
    stDciParam.u32BlackGain = g_stINIPara.stIniDci.u32DCIBlackGain;
    stDciParam.u32ContrastGain = g_stINIPara.stIniDci.u32DCIContrastGain;
    stDciParam.u32LightGain = g_stINIPara.stIniDci.u32DCILightGain;
    s32Ret = HI_MPI_VI_SetDCIParam(ViDev, &stDciParam);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VI_SetDCIParam failed\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_ISP_QueryExposureInfo(IspDev, &stIspExpInfo);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_QueryExposureInfo failed\n");
    }
    u32Exposure = ((HI_U64)stIspExpInfo.u32AGain * (HI_U64)stIspExpInfo.u32DGain * (HI_U64)stIspExpInfo.u32ISPDGain >> 30) 
                          * ((HI_U64)stIspExpInfo.u32ExpTime);

    /***************set gamma***************/
    if((SCENEAUTO_SPECIAL_SCENE_IR != g_eSpecialScene) && (SCENEAUTO_SPECIAL_SCENE_TRAFFIC != g_eSpecialScene))
    {
    	u32ExpLevel = GetLevelLtoH(u32Exposure, u32ExpLevel, g_stINIPara.stIniGamma.u32ExpCount, g_stINIPara.stIniGamma.pu32ExpThreshLtoH);
    	s32Ret = HI_MPI_ISP_GetGammaAttr(IspDev, &stGammaAttr);  
    	if (HI_SUCCESS != s32Ret)
    	{
        	printf("HI_MPI_ISP_GetGammaAttr failed\n");
        	return HI_FAILURE;
    	}
    	stGammaAttr.enCurveType = ISP_GAMMA_CURVE_USER_DEFINE;
   		for (i = 0; i < 257; i++)
    	{
        	stGammaAttr.u16Table[i] = g_stINIPara.stIniGamma.pstGamma[u32ExpLevel].u16Table[i];
    	}
		//printf("gamma open %d",__LINE__);
	
    
    	s32Ret = HI_MPI_ISP_SetGammaAttr(IspDev, &stGammaAttr);  
    	if (HI_SUCCESS != s32Ret)
    	{
       		 printf("HI_MPI_ISP_SetGammaAttr failed\n");
       		 return HI_FAILURE;
    	}
    }
	
	#if 0
    /***************set AE route***************/
    s32Ret = HI_MPI_ISP_GetAERouteAttr(IspDev, &stAeRoute);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetAERouteAttr failed\n");
        return HI_FAILURE;
    }
    stAeRoute.u32TotalNum = g_stINIPara.stIniNormalAeRoute.s32TotalNum;
    for (i = 0; i < g_stINIPara.stIniNormalAeRoute.s32TotalNum; i++)
    {
        stAeRoute.astRouteNode[i].u32IntTime = g_stINIPara.stIniNormalAeRoute.pstRouteNode[i].u32IntTime;
        stAeRoute.astRouteNode[i].u32SysGain = g_stINIPara.stIniNormalAeRoute.pstRouteNode[i].u32SysGain;
    }
    s32Ret = HI_MPI_ISP_SetAERouteAttr(IspDev, &stAeRoute);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetAERouteAttr failed\n");
        return HI_FAILURE;
    }
    #endif
    /****************set ae runinterval***************/
    s32Ret = HI_MPI_ISP_GetExposureAttr(IspDev, &stExposureAttr);  
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetExposureAttr failed\n");
        return HI_FAILURE;
    }
    stExposureAttr.u8AERunInterval = g_stINIPara.stIniAE.u8AERunInterval;
    s32Ret = HI_MPI_ISP_SetExposureAttr(IspDev, &stExposureAttr);  
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetExposureAttr failed\n");
        return HI_FAILURE;
    }
 
    return HI_SUCCESS;
}

HI_S32 Sceneauto_SetDefaultGamma()
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_GAMMA_ATTR_S stIspGammaAttr;
    ISP_DEV IspDev;

    IspDev = g_stINIPara.stMpInfo.s32IspDev;

    s32Ret = HI_MPI_ISP_GetGammaAttr(IspDev, &stIspGammaAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetGammaAttr failed\n");
        return HI_FAILURE;
    }
    stIspGammaAttr.enCurveType = ISP_GAMMA_CURVE_DEFAULT;
    s32Ret = HI_MPI_ISP_SetGammaAttr(IspDev, &stIspGammaAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetGammaAttr failed\n");
        return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}


HI_S32 HI_SCENEAUTO_SetSpecialMode(const SCENEAUTO_SEPCIAL_SCENE_E* peSpecialScene)
{
    HI_S32 s32Ret = HI_SUCCESS;
    CHECK_SCENEAUTO_INIT();
    CHECK_NULL_PTR(peSpecialScene);
    if(g_bSceneautoStart == HI_FALSE)
    {
         printf("SCENEAUTO Module has been stopped .Not allow to set special mode!\n");
         return HI_FAILURE;
    }

    if ((*peSpecialScene != SCENEAUTO_SPECIAL_SCENE_IR) &&
        (*peSpecialScene != SCENEAUTO_SPECIAL_SCENE_HLC) &&
        (*peSpecialScene != SCENEAUTO_SPECIAL_SCENE_NONE) &&
        (*peSpecialScene != SCENEAUTO_SPECIAL_SCENE_TRAFFIC))
    {
        printf("error input! unknow special scene!\n");
        return HI_FAILURE;
    }


    MUTEX_LOCK(g_stSceneautoLock);
    SCENEAUTO_SEPCIAL_SCENE_E eSpecialScene = *peSpecialScene;

    if (g_eSpecialScene != eSpecialScene)
    {
        if ((HI_TRUE != g_bSceneautoStart) && (SCENEAUTO_SPECIAL_SCENE_NONE == g_eSpecialScene))
        {
            s32Ret = Sceneauto_GetPreviousPara();
            if (HI_SUCCESS != s32Ret)
            {
                printf("Sceneauto_GetPreviousPara failed\n");
                MUTEX_UNLOCK(g_stSceneautoLock);
                return HI_FAILURE;
            }
        }
        #if 0
        switch (eSpecialScene)
        {
            case SCENEAUTO_SPECIAL_SCENE_IR:
                s32Ret = Sceneauto_SetIR();
                if (HI_SUCCESS != s32Ret)
                {
                    printf("Sceneauto_SetIR failed\n");
                    pthread_mutex_unlock(&g_stSceneautoLock);
                    return HI_FAILURE;
                }
                break;
            case SCENEAUTO_SPECIAL_SCENE_HLC:
                s32Ret = Sceneauto_SetHLC();
                if (HI_SUCCESS != s32Ret)
                {
                    printf("Sceneauto_SetHLC failed\n");
                    pthread_mutex_unlock(&g_stSceneautoLock);
                    return HI_FAILURE;
                }
                break;

			case SCENEAUTO_SPECIAL_SCENE_TRAFFIC:
				hi_usleep(1000000);
				s32Ret = Sceneauto_SetTrafficMode();
				if (HI_SUCCESS != s32Ret)
				{
					printf("Sceneauto_SetTrafficMode failed\n");
					pthread_mutex_unlock(&g_stSceneautoLock);
					return HI_FAILURE;
				}
				break;
            case SCENEAUTO_SPECIAL_SCENE_NONE:
                s32Ret = Sceneauto_SetPreviousPara();
                if (HI_SUCCESS != s32Ret)
                {
                    pthread_mutex_unlock(&g_stSceneautoLock);
                    printf("Sceneauto_SetPreviousPara failed\n");
                    return HI_FAILURE;
                }
                break;
            default:
                printf("unkonw choice\n");
                break;
        }
        #endif
        g_eSpecialScene = eSpecialScene;
    }
    MUTEX_UNLOCK(g_stSceneautoLock);
    return HI_SUCCESS;
}

HI_S32 HI_SCENEAUTO_GetSpecialMode(SCENEAUTO_SEPCIAL_SCENE_E* peSpecialScene)
{
    MUTEX_LOCK(g_stSceneautoLock);
    *peSpecialScene = g_eSpecialScene;
    MUTEX_UNLOCK(g_stSceneautoLock);

    return HI_SUCCESS;
}

HI_S32 HI_SCENEAUTO_Stop()
{
    HI_S32 s32Ret = HI_SUCCESS;
    MUTEX_LOCK(g_stSceneautoLock);
    if (HI_FALSE == g_bSceneautoStart)
    {
        printf("SCENEAUTO Module has been stopped already!\n");
        MUTEX_UNLOCK(g_stSceneautoLock);
        return HI_SUCCESS;
    }

    g_bNormalThreadFlag = HI_FALSE;
    g_bSpecialThreadFlag = HI_FALSE;
    g_b3DnrThreadFlag = HI_FALSE;
    g_bSceneautoFirstStart = HI_TRUE;
    (void)pthread_join(g_pthSceneAutoNormal, NULL);
    (void)pthread_join(g_pthSceneAutoSpecial, NULL);
    (void)pthread_join(g_pthSceneAuto3Dnr, NULL);

    if (HI_TRUE == g_stINIPara.stThreshValue.bIVEEnable)
    {
        s32Ret = Sceneauto_IVEStop();
    }
    s32Ret = Sceneauto_SetPreviousPara();
    if (HI_SUCCESS != s32Ret)
    {
        MUTEX_UNLOCK(g_stSceneautoLock);
        printf("Sceneauto_SetPreviousPara failed\n");
        return HI_FAILURE;
    }

    /*s32Ret = Sceneauto_SetDefaultGamma();
    if (HI_SUCCESS != s32Ret)
    {
        pthread_mutex_unlock(&g_stSceneautoLock);
        printf("Sceneauto_SetDefaultGamma failed\n");
        return HI_FAILURE;
    }*/

    g_bSceneautoStart = HI_FALSE;
    MUTEX_UNLOCK(g_stSceneautoLock);
    g_eSpecialScene = SCENEAUTO_SPECIAL_SCENE_NONE;

    printf("SCENEAUTO Module has been stopped successfully!\n");

    return HI_SUCCESS;
}


HI_S32 HI_SCENEAUTO_Start()
{
    HI_S32 s32Ret = HI_SUCCESS;
    CHECK_SCENEAUTO_INIT();
    //printf("g_bSceneautoStart = %d",g_bSceneautoStart);
    MUTEX_LOCK(g_stSceneautoLock);
    if (HI_TRUE == g_bSceneautoStart)
    {
        printf("SCENEAUTO Module is start already!\n");
        MUTEX_UNLOCK(g_stSceneautoLock);
        return HI_SUCCESS;
    }

    if (SCENEAUTO_SPECIAL_SCENE_NONE == g_eSpecialScene)
    {
        s32Ret = Sceneauto_GetPreviousPara();
        if (HI_SUCCESS != s32Ret)
        {
            MUTEX_UNLOCK(g_stSceneautoLock);
            printf("Sceneauto_GetPreviousPara failed\n");
            return HI_FAILURE;
        }
    }

    s32Ret = Sceneauto_SetDefaultParam();
    if (HI_SUCCESS != s32Ret)
    {
        printf("Sceneauto_SetDefaultParam failed\n");
        MUTEX_UNLOCK(g_stSceneautoLock);
        return HI_FAILURE;
    } 
   
    g_bNormalThreadFlag = HI_TRUE;
    g_bSpecialThreadFlag = HI_FALSE;
    g_b3DnrThreadFlag = HI_TRUE;

    s32Ret = pthread_create(&g_pthSceneAutoNormal, NULL, SceneAuto_NormalThread, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        printf("pthread_create SceneAuto_NormalThread failed \n");
        MUTEX_UNLOCK(g_stSceneautoLock);
        return HI_FAILURE;
    }

    s32Ret = pthread_create(&g_pthSceneAutoSpecial, NULL, SceneAuto_SpecialThread, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        printf("pthread_create SceneAuto_SpecialThread failed \n");
        MUTEX_UNLOCK(g_stSceneautoLock);
        return HI_FAILURE;
    }

    s32Ret = pthread_create(&g_pthSceneAuto3Dnr, NULL, SceneAuto_3DnrThread, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        printf("pthread_create SceneAuto_3DnrThread failed \n");
        MUTEX_UNLOCK(g_stSceneautoLock);
        return HI_FAILURE;
    }

    if (HI_TRUE == g_stINIPara.stThreshValue.bIVEEnable)
    {
        s32Ret = Sceneauto_IVEStart();

    }
    g_bSceneautoStart = HI_TRUE;
    MUTEX_UNLOCK(g_stSceneautoLock);

    printf("SCENEAUTO Module has been started successfully!\n");

    return HI_SUCCESS;
}
 
HI_S32 HI_SCENEAUTO_DeInit()
{
    MUTEX_LOCK(g_stSceneautoLock);
    if (HI_FALSE == g_bSceneautoInit)
    {
        printf("SCENEAUTO Module has not been inited !\n");
        MUTEX_UNLOCK(g_stSceneautoLock);
        return HI_FAILURE;
    }

    if (HI_TRUE == g_bNormalThreadFlag)
    {
        g_bNormalThreadFlag = HI_FALSE;
        (void)pthread_join(g_pthSceneAutoNormal, NULL);
    }
    if (HI_TRUE == g_bSpecialThreadFlag)
    {
        g_bSpecialThreadFlag = HI_FALSE;
        (void)pthread_join(g_pthSceneAutoSpecial, NULL);
    }
    if (HI_TRUE == g_b3DnrThreadFlag)
    {
        g_b3DnrThreadFlag = HI_FALSE;
        (void)pthread_join(g_pthSceneAuto3Dnr, NULL);
    }
    Sceneauto_FreeDict();
    Sceneauto_FreeMem();
    g_bSceneautoInit = HI_FALSE;
    MUTEX_UNLOCK(g_stSceneautoLock);

    printf("SCENEAUTO Module has been deinited successfully!\n");

    return HI_SUCCESS;
}

HI_S32 HI_SCENEAUTO_Init(const HI_CHAR* pszFileName)
{
    HI_S32 s32Ret = HI_SUCCESS;

    CHECK_NULL_PTR(pszFileName);

    MUTEX_LOCK(g_stSceneautoLock);
    if (HI_TRUE == g_bSceneautoInit)
    {
        printf("SCENEAUTO Module has been inited already\n");
        MUTEX_UNLOCK(g_stSceneautoLock);
        return HI_SUCCESS;
    }

    s32Ret = Sceneauto_LoadFile(pszFileName);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Sceneauto_LoadFile failed\n");
        Sceneauto_FreeDict();
        MUTEX_UNLOCK(g_stSceneautoLock);
        return HI_FAILURE;
    }

    s32Ret = Sceneauto_LoadINIPara();
    if (HI_SUCCESS != s32Ret)
    {
        printf("Sceneauto_LoadCommonPara failed\n");
        Sceneauto_FreeDict();
        Sceneauto_FreeMem();
        MUTEX_UNLOCK(g_stSceneautoLock);
        return HI_FAILURE;
    }

    g_bSceneautoInit = HI_TRUE;
    g_eSpecialScene = SCENEAUTO_SPECIAL_SCENE_NONE; 
    MUTEX_UNLOCK(g_stSceneautoLock);

    printf("SCENUAUTO Module has been inited successfully!\n");

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
