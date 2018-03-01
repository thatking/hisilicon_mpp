#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>


#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "hi_comm_ive.h"
#include "hi_comm_vgs.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"

#include "mpi_vb.h"
#include "mpi_sys.h"
#include "mpi_ive.h"
#include "mpi_vgs.h"
#include "mpi_vi.h"
#include "mpi_vo.h"


#include "sample_comm_ive.h"
#include "sample_comm.h"
//#include "sample_comm_venc.h"

#define VPSS_CHN_NUM 2
typedef struct hiSAMPLE_IVE_GMM_S
{
    IVE_SRC_IMAGE_S stSrc;
    IVE_DST_IMAGE_S stFg;
    IVE_DST_IMAGE_S stBg;
    IVE_MEM_INFO_S  stModel;
    IVE_IMAGE_S	stImg1;
    IVE_IMAGE_S	stImg2;
    IVE_DST_MEM_INFO_S stBlob;
    IVE_GMM_CTRL_S  stGmmCtrl;
    IVE_CCL_CTRL_S  stCclCtrl;
    IVE_FILTER_CTRL_S stFltCtrl;
    IVE_DILATE_CTRL_S stDilateCtrl;
    IVE_ERODE_CTRL_S stErodeCtrl;
    SAMPLE_RECT_ARRAY_S stRegion;
} SAMPLE_IVE_GMM_S;

typedef struct hiSAMPLE_IVE_GMM_INFO_S
{
    SAMPLE_IVE_GMM_S stGmm;
    HI_BOOL bEncode;
    HI_BOOL bVo;
} SAMPLE_IVE_GMM_INFO_S;

static HI_BOOL s_bStopSignal = HI_FALSE;
static HI_BOOL s_bViVpssOnline = HI_TRUE;
static pthread_t s_hIveThread = 0;
static VENC_CHN s_VeH264Chn = 0;
static SAMPLE_IVE_GMM_INFO_S s_stGmmInfo;
static SAMPLE_VI_CONFIG_S s_stViConfig;

static HI_VOID SAMPLE_IVE_Gmm_Uninit(SAMPLE_IVE_GMM_S* pstGmm)
{
    IVE_MMZ_FREE(pstGmm->stSrc.u32PhyAddr[0], pstGmm->stSrc.pu8VirAddr[0]);
    IVE_MMZ_FREE(pstGmm->stFg.u32PhyAddr[0], pstGmm->stFg.pu8VirAddr[0]);
    IVE_MMZ_FREE(pstGmm->stBg.u32PhyAddr[0], pstGmm->stBg.pu8VirAddr[0]);
    IVE_MMZ_FREE(pstGmm->stModel.u32PhyAddr, pstGmm->stModel.pu8VirAddr);
    IVE_MMZ_FREE(pstGmm->stImg1.u32PhyAddr[0], pstGmm->stImg1.pu8VirAddr[0]);
    IVE_MMZ_FREE(pstGmm->stImg2.u32PhyAddr[0], pstGmm->stImg2.pu8VirAddr[0]);
    IVE_MMZ_FREE(pstGmm->stBlob.u32PhyAddr, pstGmm->stBlob.pu8VirAddr);
}


static HI_S32 SAMPLE_IVE_Gmm_Init(SAMPLE_IVE_GMM_S* pstGmm, HI_U16 u16Width, HI_U16 u16Height)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Size = 0;
    HI_S8 as8Mask[25] = {1, 2, 3, 2, 1,
                         2, 5, 6, 5, 2,
                         3, 6, 8, 6, 3,
                         2, 5, 6, 5, 2,
                         1, 2, 3, 2, 1
                        };

    memset(pstGmm, 0, sizeof(SAMPLE_IVE_GMM_S));

    s32Ret = SAMPLE_COMM_IVE_CreateImage(&(pstGmm->stSrc), IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
        goto GMM_INIT_FAIL;
    }
    s32Ret = SAMPLE_COMM_IVE_CreateImage(&(pstGmm->stFg), IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
        goto GMM_INIT_FAIL;
    }
    s32Ret = SAMPLE_COMM_IVE_CreateImage(&(pstGmm->stBg), IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
        goto GMM_INIT_FAIL;
    }

    pstGmm->stGmmCtrl.u0q16InitWeight = 3276; //0.05
    pstGmm->stGmmCtrl.u0q16BgRatio = 52428;   //0.8
    pstGmm->stGmmCtrl.u22q10MaxVar = (2000 << 10);
    pstGmm->stGmmCtrl.u22q10MinVar = (200 << 10);
    pstGmm->stGmmCtrl.u22q10NoiseVar = (225 << 10);
    pstGmm->stGmmCtrl.u8q8VarThr = 1600;
    pstGmm->stGmmCtrl.u8ModelNum = 3;
    pstGmm->stGmmCtrl.u0q16LearnRate = 327;

    u32Size = pstGmm->stSrc.u16Width * pstGmm->stSrc.u16Height * pstGmm->stGmmCtrl.u8ModelNum * 7;
    s32Ret = SAMPLE_COMM_IVE_CreateMemInfo(&pstGmm->stModel, u32Size);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateMemInfo fail\n");
        goto GMM_INIT_FAIL;
    }
    memset(pstGmm->stModel.pu8VirAddr, 0, pstGmm->stModel.u32Size);

    s32Ret = SAMPLE_COMM_IVE_CreateImage(&(pstGmm->stImg1), IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
        goto GMM_INIT_FAIL;
    }
    s32Ret = SAMPLE_COMM_IVE_CreateImage(&(pstGmm->stImg2), IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
        goto GMM_INIT_FAIL;
    }

    u32Size = sizeof(IVE_CCBLOB_S);
    s32Ret = SAMPLE_COMM_IVE_CreateMemInfo(&pstGmm->stBlob, u32Size);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateMemInfo fail\n");
        goto GMM_INIT_FAIL;
    }

    memcpy(pstGmm->stFltCtrl.as8Mask, as8Mask, 25);
    pstGmm->stFltCtrl.u8Norm = 7;
    memset(pstGmm->stDilateCtrl.au8Mask, 255, 25);
    memset(pstGmm->stErodeCtrl.au8Mask, 255, 25);
    pstGmm->stCclCtrl.u16InitAreaThr 	= 16;
    pstGmm->stCclCtrl.u16Step 			= 4;
	pstGmm->stCclCtrl.enMode 			= IVE_CCL_MODE_8C;
GMM_INIT_FAIL:

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_IVE_Gmm_Uninit(pstGmm);
    }
    return s32Ret;
}

static HI_S32 SAMPLE_IVE_GmmProc(SAMPLE_IVE_GMM_INFO_S* pstGmmInfo, VIDEO_FRAME_INFO_S* pstExtFrmInfo,HI_U16 u16BaseWidth,HI_U16 u16BaseHeight)
{
    HI_S32 s32Ret;
    SAMPLE_IVE_GMM_S* pstGmm;
    IVE_HANDLE IveHandle;
    HI_BOOL bFinish = HI_FALSE;
    HI_BOOL bBlock = HI_TRUE;
    HI_BOOL bInstant = HI_TRUE;
    IVE_CCBLOB_S* pstBlob;

    pstGmm = &pstGmmInfo->stGmm;
    pstBlob = (IVE_CCBLOB_S*)pstGmm->stBlob.pu8VirAddr;

    //1.Get Y    
    bInstant = HI_FALSE;
	s32Ret = SAMPLE_COMM_IVE_DmaImage(pstExtFrmInfo,&pstGmm->stSrc,bInstant);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_DmaImage fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }
    //2.Filter
    s32Ret = HI_MPI_IVE_Filter(&IveHandle, &pstGmm->stSrc, &pstGmm->stImg1, &pstGmm->stFltCtrl, bInstant);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_IVE_Filter fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }
    //3.Gmm
    s32Ret = HI_MPI_IVE_GMM(&IveHandle, &pstGmm->stImg1, &pstGmm->stFg, &pstGmm->stBg, &pstGmm->stModel, &pstGmm->stGmmCtrl, bInstant);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_IVE_GMM fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }
    //4.Dilate
    s32Ret = HI_MPI_IVE_Dilate(&IveHandle, &pstGmm->stFg, &pstGmm->stImg1, &pstGmm->stDilateCtrl, bInstant);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_IVE_Dilate fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }
    //5.Erode
    s32Ret = HI_MPI_IVE_Erode(&IveHandle, &pstGmm->stImg1, &pstGmm->stImg2, &pstGmm->stErodeCtrl, bInstant);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_IVE_Erode fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }
    //6.CCL
    bInstant = HI_TRUE;
    s32Ret = HI_MPI_IVE_CCL(&IveHandle, &pstGmm->stImg2, &pstGmm->stBlob, &pstGmm->stCclCtrl, bInstant);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_IVE_CCL fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }
    //Wait task finish
    s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, bBlock);
    while (HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
    {
        usleep(100);
        s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, bBlock);
    }
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_IVE_Query fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }

    SAMPLE_COMM_IVE_BlobToRect(pstBlob, &(pstGmm->stRegion), 50, 8,  
		pstExtFrmInfo->stVFrame.u32Width, pstExtFrmInfo->stVFrame.u32Height,
		u16BaseWidth,u16BaseHeight);

    return HI_SUCCESS;
}


static HI_VOID* SAMPLE_IVE_ViToVo(HI_VOID* pArgs)
{
    HI_S32 s32Ret;
    SAMPLE_IVE_GMM_INFO_S *pstGmmInfo;
    SAMPLE_IVE_GMM_S *pstGmm;
	VIDEO_FRAME_INFO_S stBaseFrmInfo;
	VIDEO_FRAME_INFO_S stExtFrmInfo;
    HI_S32 s32MilliSec = 20000;
    VO_LAYER voLayer = 0;
    VO_CHN voChn = 0;
    VENC_CHN vencChn = 0;
    HI_BOOL bEncode;
    HI_BOOL bVo;
	VPSS_GRP s32VpssGrp = 0;
	VPSS_CHN as32VpssChn[] = {VPSS_CHN0, VPSS_CHN1};

    pstGmmInfo = (SAMPLE_IVE_GMM_INFO_S*)pArgs;
    pstGmm = &(pstGmmInfo->stGmm);
    bEncode = pstGmmInfo->bEncode;
    bVo = pstGmmInfo->bVo;

    while (HI_FALSE == s_bStopSignal)
    {

		s32Ret = HI_MPI_VPSS_GetChnFrame(s32VpssGrp, as32VpssChn[1], &stExtFrmInfo, s32MilliSec);
		if(HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("HI_MPI_VPSS_GetChnFrame failed, VPSS_GRP(%d), VPSS_CHN(%d), Error(%#x)!\n",
				s32VpssGrp, as32VpssChn[1], s32Ret);
			continue;
		}
		
		s32Ret = HI_MPI_VPSS_GetChnFrame(s32VpssGrp, as32VpssChn[0], &stBaseFrmInfo, s32MilliSec);
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!=s32Ret, EXT_RELEASE,
			"HI_MPI_VPSS_GetChnFrame failed, VPSS_GRP(%d), VPSS_CHN(%d), Error(%#x)!\n",
			s32VpssGrp, as32VpssChn[0], s32Ret);

        s32Ret = SAMPLE_IVE_GmmProc(pstGmmInfo, &stExtFrmInfo,
			(HI_U16)stBaseFrmInfo.stVFrame.u32Width,
			(HI_U16)stBaseFrmInfo.stVFrame.u32Height);		
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!=s32Ret, BASE_RELEASE,
			"SAMPLE_IVE_GmmProc failed, Error(%#x)!\n", s32Ret);      
        
        //Draw rect
        s32Ret = SAMPLE_COMM_VGS_FillRect(&stBaseFrmInfo, &pstGmm->stRegion, 0x0000FF00);
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!=s32Ret, BASE_RELEASE,
			"SAMPLE_COMM_VGS_FillRect failed, Error(%#x)!\n", s32Ret);
		
        //Venc
        if (HI_TRUE == bEncode)
        {
            s32Ret = HI_MPI_VENC_SendFrame(vencChn, &stBaseFrmInfo, s32MilliSec);
			SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!=s32Ret, BASE_RELEASE,
				"HI_MPI_VENC_SendFrame failed, Error(%#x)!\n", s32Ret);
        }

        //Vo
        if (HI_TRUE == bVo)
        {
            s32Ret = HI_MPI_VO_SendFrame(voLayer, voChn, &stBaseFrmInfo, s32MilliSec);			
			SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!=s32Ret, BASE_RELEASE,
				"HI_MPI_VO_SendFrame failed, Error(%#x)!\n", s32Ret);
        }
		BASE_RELEASE:
			s32Ret = HI_MPI_VPSS_ReleaseChnFrame(s32VpssGrp,as32VpssChn[0], &stBaseFrmInfo);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("HI_MPI_VPSS_ReleaseChnFrame fail,Grp(%d) chn(%d),Error(%#x)\n",
					s32VpssGrp,as32VpssChn[0],s32Ret);	
			}

		EXT_RELEASE:
			s32Ret = HI_MPI_VPSS_ReleaseChnFrame(s32VpssGrp,as32VpssChn[1], &stExtFrmInfo);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("HI_MPI_VPSS_ReleaseChnFrame fail,Grp(%d) chn(%d),Error(%#x)\n",
					s32VpssGrp,as32VpssChn[1],s32Ret);	
			}
        
    }
	
    return HI_NULL;
}

/******************************************************************************
* function : show Gmm sample
******************************************************************************/
HI_VOID SAMPLE_IVE_Gmm(HI_CHAR chEncode, HI_CHAR chVo)
{

	SIZE_S astSize[VPSS_CHN_NUM];	
	PIC_SIZE_E aenSize[VPSS_CHN_NUM];

	VI_CHN_ATTR_S stViChnAttr;
	VI_CHN s32ViChn = 0;	
	VPSS_GRP s32VpssGrp = 0;
	VPSS_CHN as32VpssChn[] = {VPSS_CHN0, VPSS_CHN1};
	VPSS_CHN_ATTR_S stVpssChnAttr;
	HI_BOOL bEncode = '1' == chEncode ? HI_TRUE : HI_FALSE;
	HI_BOOL bVo = '1' == chVo ? HI_TRUE : HI_FALSE;
    SAMPLE_RC_E enRcMode = SAMPLE_RC_CBR;
    VIDEO_NORM_E enVideoNorm = VIDEO_ENCODING_MODE_PAL;
    PAYLOAD_TYPE_E enStreamType = PT_H264;

	HI_U32 i;
	HI_S32 s32Ret = HI_SUCCESS;

	memset(&s_stGmmInfo,0,sizeof(s_stGmmInfo));
	memset(&s_stViConfig,0,sizeof(s_stViConfig));
	s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(&aenSize[0]);	
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_GMM_0, 
		"SAMPLE_COMM_VI_GetSizeBySensor failed,Error(%#x)!\n",s32Ret);	
	aenSize[1] = PIC_CIF;
	/******************************************
	 step  1: Init vb
	******************************************/ 
	s32Ret = SAMPLE_COMM_IVE_VbInit(aenSize,astSize,VPSS_CHN_NUM);	
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_GMM_0, 
		"SAMPLE_COMM_IVE_VbInit failed,Error(%#x)!\n",s32Ret);	
	/******************************************
	 step 2: Start vi
	******************************************/
	s_stViConfig.enViMode   = SENSOR_TYPE;
	s_stViConfig.enRotate   = ROTATE_NONE;
	s_stViConfig.enNorm	  = VIDEO_ENCODING_MODE_AUTO;
	s_stViConfig.enViChnSet = VI_CHN_SET_NORMAL;
	s_stViConfig.enWDRMode  = WDR_MODE_NONE;
	s32Ret = SAMPLE_COMM_VI_StartVi(&s_stViConfig);	
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_GMM_1, 
		"SAMPLE_COMM_VI_StartVi failed,Error(%#x)!\n",s32Ret);		
	/******************************************
	 step 3: Start vpss
	******************************************/
	s32Ret = SAMPLE_COMM_IVE_StartVpss(astSize,VPSS_CHN_NUM);	
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_GMM_2, 
		"SAMPLE_IVS_StartVpss failed,Error(%#x)!\n",s32Ret);	
	/******************************************
	  step 4: Bind vpss to vi
	 ******************************************/
	s_bViVpssOnline = SAMPLE_COMM_IsViVpssOnline();
	if (HI_FALSE == s_bViVpssOnline) //if it is not online,we bind vpss to vi ,so,we can get data from vpss
	{
		s32Ret = SAMPLE_COMM_VI_BindVpss(s_stViConfig.enViMode); // VI --> VPSS
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_GMM_3, 
			"SAMPLE_COMM_VI_BindVpss failed,Error(%#x)!\n",s32Ret); 
		//Set vi frame
		s32Ret = HI_MPI_VI_GetChnAttr(s32ViChn,&stViChnAttr);		
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_GMM_4, 
			"HI_MPI_VI_GetChnAttr failed,Error(%#x)!\n",s32Ret);	
		stViChnAttr.s32SrcFrameRate = 3;
		stViChnAttr.s32DstFrameRate = 1;		
		s32Ret = HI_MPI_VI_SetChnAttr(s32ViChn,&stViChnAttr);		
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_GMM_4, 
			"HI_MPI_VI_SetChnAttr failed,Error(%#x)!\n",s32Ret);	
	}	
	else
	{
		//Set Vpss frame
		for (i = 0; i < VPSS_CHN_NUM; i++)
		{
			s32Ret = HI_MPI_VPSS_GetChnAttr(s32VpssGrp,as32VpssChn[i],&stVpssChnAttr);		
			SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_GMM_3, 
				"HI_MPI_VPSS_GetChnAttr failed,Error(%#x)!\n",s32Ret);	
			stVpssChnAttr.s32SrcFrameRate = 3;
			stVpssChnAttr.s32DstFrameRate = 1;
			s32Ret = HI_MPI_VPSS_SetChnAttr(s32VpssGrp,as32VpssChn[i],&stVpssChnAttr);		
			SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_GMM_3, 
				"HI_MPI_VPSS_SetChnAttr failed,Error(%#x)!\n",s32Ret);	

		}

	}
	/******************************************
	  [step 5]: Start Vo
	 ******************************************/
	 if (HI_TRUE == bVo)
	 {
		 s32Ret = SAMPLE_COMM_IVE_StartVo();
		 SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_GMM_4, 
			 "SAMPLE_COMM_IVE_StartVo failed, Error(%#x)!\n", s32Ret);

	 }

	 if (HI_TRUE == bEncode)
	 {
		 /******************************************
		   [step 5]: Start Venc
		  ******************************************/	
		  s32Ret = SAMPLE_COMM_VENC_Start(s_VeH264Chn, enStreamType,enVideoNorm,aenSize[0],enRcMode,0,ROTATE_NONE);
		  SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_GMM_5, 
			  "SAMPLE_COMM_VENC_Start failed,Error(%#x)!\n",s32Ret);  
		  /******************************************
		   [step 5.1]: Venc start get stream
		  ******************************************/	
		  s32Ret = SAMPLE_COMM_VENC_StartGetStream(1);
		  SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_GMM_6, 
			  "SAMPLE_COMM_VENC_StartGetStream failed,Error(%#x)!\n",s32Ret); 

	 }
	
	/******************************************
	  step 6: Init Gmm
	 ******************************************/	
	s32Ret = SAMPLE_IVE_Gmm_Init(&s_stGmmInfo.stGmm, astSize[1].u32Width, astSize[1].u32Height);
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_GMM_7, 
		"SAMPLE_IVE_Gmm_Init failed, Error(%#x)!\n", s32Ret);	

	s_stGmmInfo.bEncode = bEncode;
	s_stGmmInfo.bVo = bVo;
	s_bStopSignal = HI_FALSE;
	//Start vi to vo thread
	pthread_create(&s_hIveThread, 0, SAMPLE_IVE_ViToVo, (HI_VOID*)&s_stGmmInfo);

	SAMPLE_VI_PAUSE();
		
	s_bStopSignal = HI_TRUE;
	pthread_join(s_hIveThread, HI_NULL);
	s_hIveThread = 0;
    SAMPLE_IVE_Gmm_Uninit(&s_stGmmInfo.stGmm);
END_GMM_7:
	if (HI_TRUE == bEncode)
	{		
		SAMPLE_COMM_VENC_StopGetStream();
	}
END_GMM_6:
	if (HI_TRUE == bEncode)
	{		
		SAMPLE_COMM_VENC_Stop(s_VeH264Chn);
	}
END_GMM_5:
	if (HI_TRUE == bVo)
	{
		SAMPLE_COMM_IVE_StopVo();
	}
END_GMM_4:
	if (HI_FALSE == s_bViVpssOnline )
	{
	   SAMPLE_COMM_VI_UnBindVpss(s_stViConfig.enViMode);
	}
END_GMM_3:	
	SAMPLE_COMM_IVE_StopVpss(VPSS_CHN_NUM);
END_GMM_2:	
	SAMPLE_COMM_VI_StopVi(&s_stViConfig);
END_GMM_1:	//system exit
	SAMPLE_COMM_SYS_Exit();	
	memset(&s_stGmmInfo,0,sizeof(s_stGmmInfo));
	memset(&s_stViConfig,0,sizeof(s_stViConfig));	
	s_bViVpssOnline = HI_TRUE;
END_GMM_0:	
		
	return ;
}

/******************************************************************************
* function : Gmm sample signal handle
******************************************************************************/
HI_VOID SAMPLE_IVE_Gmm_HandleSig(HI_VOID)
{
	s_bStopSignal = HI_TRUE;
	if (0 != s_hIveThread)
	{
		pthread_join(s_hIveThread, HI_NULL);	
		s_hIveThread = 0;
	}
    SAMPLE_IVE_Gmm_Uninit(&s_stGmmInfo.stGmm);
	if (HI_TRUE == s_stGmmInfo.bEncode)
	{
		SAMPLE_COMM_VENC_StopGetStream();
		SAMPLE_COMM_VENC_Stop(s_VeH264Chn);
	}
	if (HI_TRUE == s_stGmmInfo.bVo)
	{
		SAMPLE_COMM_IVE_StopVo();
	}
	if (HI_FALSE == s_bViVpssOnline )
	{
	   SAMPLE_COMM_VI_UnBindVpss(s_stViConfig.enViMode);
	}
	SAMPLE_COMM_IVE_StopVpss(VPSS_CHN_NUM);
	SAMPLE_COMM_VI_StopVi(&s_stViConfig);	
	SAMPLE_COMM_SYS_Exit();	

	memset(&s_stGmmInfo,0,sizeof(s_stGmmInfo));
	memset(&s_stViConfig,0,sizeof(s_stViConfig));
	s_bViVpssOnline = HI_TRUE;
}

