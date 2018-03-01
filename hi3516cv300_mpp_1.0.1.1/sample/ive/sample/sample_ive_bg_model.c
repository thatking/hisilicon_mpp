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

#define VPSS_CHN_NUM 2

typedef struct hiSAMPLE_IVE_BG_MODEL_S
{
    IVE_SRC_IMAGE_S stCurImg;
    IVE_IMAGE_S stFgFlag;
    IVE_IMAGE_S  stImg1;
    IVE_IMAGE_S  stImg2;
    IVE_DST_IMAGE_S stBgDiffFg;
    IVE_DST_IMAGE_S stFrmDiffFg;
    IVE_DST_IMAGE_S stBgImg;
    IVE_DST_IMAGE_S stChgStaImg;
    IVE_DST_IMAGE_S stChgStaFg;
    IVE_DST_IMAGE_S stChgStaLife;
    IVE_DST_MEM_INFO_S  stStatData;
    IVE_DST_MEM_INFO_S stBlob;
    IVE_DATA_S stBgModel;
    IVE_CCL_CTRL_S  stCclCtrl;
    IVE_FILTER_CTRL_S stFltCtrl;
    IVE_DILATE_CTRL_S stDilateCtrl;
    IVE_ERODE_CTRL_S stErodeCtrl;
    IVE_MATCH_BG_MODEL_CTRL_S stMatchBgModelCtrl;
    IVE_UPDATE_BG_MODEL_CTRL_S stUpdateBgModelCtrl;
    SAMPLE_RECT_ARRAY_S stRegion;
    HI_U32 u32FrameCnt;
    HI_U32 u32UpdCnt;
    HI_U32 u32PreUpdTime;
    HI_U32 u32PreChkTime;

} SAMPLE_IVE_BG_MODEL_S;

typedef struct hiSAMPLE_IVE_BG_MODEL_INFO_S
{
    SAMPLE_IVE_BG_MODEL_S stBgModel;
    HI_BOOL bEncode;
    HI_BOOL bVo;
} SAMPLE_IVE_BG_MODEL_INFO_S;

static HI_BOOL s_bStopSignal = HI_FALSE;
static HI_BOOL s_bViVpssOnline = HI_TRUE;
static pthread_t s_hIveThread = 0;
static 	VENC_CHN s_VeH264Chn = 0;
static SAMPLE_IVE_BG_MODEL_INFO_S s_stBgModelInfo;
static SAMPLE_VI_CONFIG_S s_stViConfig;

static HI_VOID SAMPLE_IVE_BgModel_Uninit(SAMPLE_IVE_BG_MODEL_S* pstBgModel)
{
    IVE_MMZ_FREE(pstBgModel->stCurImg.u32PhyAddr[0], pstBgModel->stCurImg.pu8VirAddr[0]);
    IVE_MMZ_FREE(pstBgModel->stFgFlag.u32PhyAddr[0], pstBgModel->stFgFlag.pu8VirAddr[0]);
    IVE_MMZ_FREE(pstBgModel->stImg1.u32PhyAddr[0], pstBgModel->stImg1.pu8VirAddr[0]);
    IVE_MMZ_FREE(pstBgModel->stImg2.u32PhyAddr[0], pstBgModel->stImg2.pu8VirAddr[0]);
    IVE_MMZ_FREE(pstBgModel->stBgDiffFg.u32PhyAddr[0], pstBgModel->stBgDiffFg.pu8VirAddr[0]);
    IVE_MMZ_FREE(pstBgModel->stFrmDiffFg.u32PhyAddr[0], pstBgModel->stFrmDiffFg.pu8VirAddr[0]);
    IVE_MMZ_FREE(pstBgModel->stBgImg.u32PhyAddr[0], pstBgModel->stBgImg.pu8VirAddr[0]);
    IVE_MMZ_FREE(pstBgModel->stChgStaImg.u32PhyAddr[0], pstBgModel->stChgStaImg.pu8VirAddr[0]);
    IVE_MMZ_FREE(pstBgModel->stChgStaFg.u32PhyAddr[0], pstBgModel->stChgStaFg.pu8VirAddr[0]);
    IVE_MMZ_FREE(pstBgModel->stChgStaLife.u32PhyAddr[0], pstBgModel->stChgStaLife.pu8VirAddr[0]);
    IVE_MMZ_FREE(pstBgModel->stBgModel.u32PhyAddr, pstBgModel->stBgModel.pu8VirAddr);
    IVE_MMZ_FREE(pstBgModel->stStatData.u32PhyAddr, pstBgModel->stStatData.pu8VirAddr);
    IVE_MMZ_FREE(pstBgModel->stBlob.u32PhyAddr, pstBgModel->stBlob.pu8VirAddr);
}


static HI_S32 SAMPLE_IVE_BgModel_Init(SAMPLE_IVE_BG_MODEL_S* pstBgModel, HI_U16 u16Width, HI_U16 u16Height)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32Size = 0;
    HI_S8 as8Mask[25] = {1, 2, 3, 2, 1,
                         2, 5, 6, 5, 2,
                         3, 6, 8, 6, 3,
                         2, 5, 6, 5, 2,
                         1, 2, 3, 2, 1
                        };
    memset(pstBgModel, 0, sizeof(SAMPLE_IVE_BG_MODEL_S));

    s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstBgModel->stCurImg, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
        goto BGMODEL_INIT_FAIL;
    }
    s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstBgModel->stFgFlag, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
        goto BGMODEL_INIT_FAIL;
    }
    memset(pstBgModel->stFgFlag.pu8VirAddr[0], 0, pstBgModel->stFgFlag.u16Stride[0] * pstBgModel->stFgFlag.u16Height);
    s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstBgModel->stBgDiffFg, IVE_IMAGE_TYPE_S8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
        goto BGMODEL_INIT_FAIL;
    }
    memset(pstBgModel->stBgDiffFg.pu8VirAddr[0], 0, pstBgModel->stBgDiffFg.u16Stride[0] * pstBgModel->stBgDiffFg.u16Height);
    s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstBgModel->stFrmDiffFg, IVE_IMAGE_TYPE_S8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
        goto BGMODEL_INIT_FAIL;
    }
    memset(pstBgModel->stFrmDiffFg.pu8VirAddr[0], 0, pstBgModel->stFrmDiffFg.u16Stride[0] * pstBgModel->stFrmDiffFg.u16Height);


    pstBgModel->stBgModel.u16Width = u16Width * sizeof(IVE_BG_MODEL_PIX_S);
    pstBgModel->stBgModel.u16Height = u16Height;
    pstBgModel->stBgModel.u16Stride = SAMPLE_COMM_IVE_CalcStride(pstBgModel->stBgModel.u16Width, IVE_ALIGN);
    u32Size = pstBgModel->stBgModel.u16Stride * pstBgModel->stBgModel.u16Height;
    s32Ret = HI_MPI_SYS_MmzAlloc(&pstBgModel->stBgModel.u32PhyAddr,
                                 (void**)&pstBgModel->stBgModel.pu8VirAddr, NULL, HI_NULL, u32Size);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_SYS_MmzAlloc fail,Error(%#x)\n", s32Ret);
        goto BGMODEL_INIT_FAIL;
    }

    memset(pstBgModel->stBgModel.pu8VirAddr, 0, u32Size);

    s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstBgModel->stBgImg, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
        goto BGMODEL_INIT_FAIL;
    }
    memset(pstBgModel->stBgImg.pu8VirAddr[0], 0, pstBgModel->stBgImg.u16Stride[0] * pstBgModel->stBgImg.u16Height);
    s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstBgModel->stChgStaImg, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
        goto BGMODEL_INIT_FAIL;
    }
    memset(pstBgModel->stChgStaImg.pu8VirAddr[0], 0, pstBgModel->stChgStaImg.u16Stride[0] * pstBgModel->stChgStaImg.u16Height);
    s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstBgModel->stChgStaFg, IVE_IMAGE_TYPE_S8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
        goto BGMODEL_INIT_FAIL;
    }
    memset(pstBgModel->stChgStaFg.pu8VirAddr[0], 0, pstBgModel->stChgStaFg.u16Stride[0] * pstBgModel->stChgStaFg.u16Height);
    s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstBgModel->stChgStaLife, IVE_IMAGE_TYPE_U16C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
        goto BGMODEL_INIT_FAIL;
    }
    memset(pstBgModel->stChgStaLife.pu8VirAddr[0], 0, pstBgModel->stChgStaLife.u16Stride[0] * pstBgModel->stChgStaLife.u16Height * sizeof(HI_U16));

    u32Size = sizeof(IVE_BG_STAT_DATA_S);
    s32Ret = SAMPLE_COMM_IVE_CreateMemInfo(&pstBgModel->stStatData, u32Size);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateMemInfo fail\n");
        goto BGMODEL_INIT_FAIL;
    }
    s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstBgModel->stImg1, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
        goto BGMODEL_INIT_FAIL;
    }
    s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstBgModel->stImg2, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
        goto BGMODEL_INIT_FAIL;
    }
    u32Size = sizeof(IVE_CCBLOB_S);
    s32Ret = SAMPLE_COMM_IVE_CreateMemInfo(&pstBgModel->stBlob, u32Size);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateMemInfo fail\n");
        goto BGMODEL_INIT_FAIL;
    }


    pstBgModel->stMatchBgModelCtrl.u32CurFrmNum = 0;
    pstBgModel->stMatchBgModelCtrl.u32PreFrmNum = 0;
    pstBgModel->stMatchBgModelCtrl.u16TimeThr = 20;
    pstBgModel->stMatchBgModelCtrl.u8DiffThrCrlCoef = 0;
    pstBgModel->stMatchBgModelCtrl.u8DiffMaxThr = 10;
    pstBgModel->stMatchBgModelCtrl.u8DiffMinThr = 10;
    pstBgModel->stMatchBgModelCtrl.u8DiffThrInc = 0;
    pstBgModel->stMatchBgModelCtrl.u8FastLearnRate = 4;
    pstBgModel->stMatchBgModelCtrl.u8DetChgRegion = 0;

    pstBgModel->stUpdateBgModelCtrl.u32CurFrmNum = 0;
    pstBgModel->stUpdateBgModelCtrl.u32PreChkTime = 0;
    pstBgModel->stUpdateBgModelCtrl.u32FrmChkPeriod = 50;
    pstBgModel->stUpdateBgModelCtrl.u32InitMinTime = 25;
    pstBgModel->stUpdateBgModelCtrl.u32StyBgMinBlendTime = 100;
    pstBgModel->stUpdateBgModelCtrl.u32StyBgMaxBlendTime = 1500;
    pstBgModel->stUpdateBgModelCtrl.u32DynBgMinBlendTime = 0;
    pstBgModel->stUpdateBgModelCtrl.u32StaticDetMinTime = 80;
    pstBgModel->stUpdateBgModelCtrl.u16FgMaxFadeTime = 15;
    pstBgModel->stUpdateBgModelCtrl.u16BgMaxFadeTime = 60;
    pstBgModel->stUpdateBgModelCtrl.u8StyBgAccTimeRateThr = 80;
    pstBgModel->stUpdateBgModelCtrl.u8ChgBgAccTimeRateThr = 60;
    pstBgModel->stUpdateBgModelCtrl.u8DynBgAccTimeThr = 0;
    pstBgModel->stUpdateBgModelCtrl.u8DynBgDepth = 3;
    pstBgModel->stUpdateBgModelCtrl.u8BgEffStaRateThr = 90;
    pstBgModel->stUpdateBgModelCtrl.u8AcceBgLearn = 0;
    pstBgModel->stUpdateBgModelCtrl.u8DetChgRegion = 0;

    memcpy(pstBgModel->stFltCtrl.as8Mask, as8Mask, 25);
    pstBgModel->stFltCtrl.u8Norm = 7;
    memset(pstBgModel->stDilateCtrl.au8Mask, 255, 25);
    memset(pstBgModel->stErodeCtrl.au8Mask, 255, 25);
    pstBgModel->stCclCtrl.u16InitAreaThr = 16;
    pstBgModel->stCclCtrl.u16Step = 4;
	pstBgModel->stCclCtrl.enMode  = IVE_CCL_MODE_8C;
    pstBgModel->stRegion.u16Num = 0;
BGMODEL_INIT_FAIL:

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_IVE_BgModel_Uninit(pstBgModel);
    }
    return s32Ret;

}

static HI_S32 SAMPLE_IVE_BgModelProc(SAMPLE_IVE_BG_MODEL_INFO_S* pstBgModelInfo, VIDEO_FRAME_INFO_S* pstExtFrmInfo,HI_U16 u16BaseWidth,HI_U16 u16BaseHeight)
{
    HI_S32 s32Ret;
    SAMPLE_IVE_BG_MODEL_S* pstBgModel;
    IVE_CCBLOB_S* pstBlob;
    IVE_HANDLE IveHandle;
    HI_BOOL bFinish = HI_FALSE;
    HI_BOOL bBlock = HI_TRUE;
    HI_BOOL bInstant = HI_TRUE;
    IVE_IMAGE_S stFg;
    IVE_THRESH_CTRL_S stThrCtrl = {IVE_THRESH_MODE_BINARY, 0, 0, 0, 0, 255};

    HI_U32 u32FrmUpdPeriod = 15;
    HI_U32 u32FrmChkPeriod = 50;

    pstBgModel = &(pstBgModelInfo->stBgModel);

    stFg.enType = IVE_IMAGE_TYPE_U8C1;
    stFg.pu8VirAddr[0] = pstBgModel->stBgDiffFg.pu8VirAddr[0];
    stFg.u32PhyAddr[0] = pstBgModel->stBgDiffFg.u32PhyAddr[0];
    stFg.u16Width = pstBgModel->stBgDiffFg.u16Width;
    stFg.u16Height = pstBgModel->stBgDiffFg.u16Height;
    stFg.u16Stride[0] = pstBgModel->stBgDiffFg.u16Stride[0];
    pstBlob = (IVE_CCBLOB_S*)pstBgModel->stBlob.pu8VirAddr;
	
    pstBgModel->u32FrameCnt++;

    bInstant = HI_FALSE;
	s32Ret = SAMPLE_COMM_IVE_DmaImage(pstExtFrmInfo,&pstBgModel->stCurImg,bInstant);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_DmaImage fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_IVE_Filter(&IveHandle, &pstBgModel->stCurImg, &pstBgModel->stImg1, &pstBgModel->stFltCtrl, bInstant);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_IVE_Filter fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }

    pstBgModel->stMatchBgModelCtrl.u32PreFrmNum = pstBgModel->stMatchBgModelCtrl.u32CurFrmNum;
    pstBgModel->stMatchBgModelCtrl.u32CurFrmNum = pstBgModel->u32FrameCnt;

    s32Ret = HI_MPI_IVE_MatchBgModel(&IveHandle, &pstBgModel->stImg1, &pstBgModel->stBgModel,
                                     &pstBgModel->stFgFlag, &pstBgModel->stBgDiffFg, &pstBgModel->stFrmDiffFg,
                                     &pstBgModel->stStatData, &pstBgModel->stMatchBgModelCtrl, bInstant);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_IVE_MatchBgModel fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }

    if ( (0 == pstBgModel->u32UpdCnt) || (pstBgModel->u32FrameCnt >= pstBgModel->u32PreUpdTime + u32FrmUpdPeriod) )
    {
        pstBgModel->u32UpdCnt++;
        pstBgModel->u32PreUpdTime = pstBgModel->u32FrameCnt;
        pstBgModel->stUpdateBgModelCtrl.u32CurFrmNum = pstBgModel->u32FrameCnt;
        pstBgModel->stUpdateBgModelCtrl.u32PreChkTime = pstBgModel->u32PreChkTime;
        pstBgModel->stUpdateBgModelCtrl.u32FrmChkPeriod = 0;
        if ( pstBgModel->u32FrameCnt >= pstBgModel->u32PreChkTime + u32FrmChkPeriod )
        {
            pstBgModel->stUpdateBgModelCtrl.u32FrmChkPeriod = u32FrmChkPeriod;
            pstBgModel->u32PreChkTime = pstBgModel->u32FrameCnt;
        }

        s32Ret = HI_MPI_IVE_UpdateBgModel(&IveHandle, &pstBgModel->stBgModel, &pstBgModel->stFgFlag,
                                          &pstBgModel->stBgImg, &pstBgModel->stChgStaImg, &pstBgModel->stChgStaFg, &pstBgModel->stChgStaLife,
                                          &pstBgModel->stStatData, &pstBgModel->stUpdateBgModelCtrl, bInstant);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_IVE_UpdateBgModel fail,Error(%#x)\n", s32Ret);
            return s32Ret;
        }
    }


    s32Ret = HI_MPI_IVE_Thresh(&IveHandle, &stFg, &stFg, &stThrCtrl, bInstant);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_IVE_Thresh fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_IVE_Dilate(&IveHandle, &stFg, &pstBgModel->stImg2, &pstBgModel->stDilateCtrl, bInstant);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_IVE_Dilate fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_IVE_Erode(&IveHandle, &pstBgModel->stImg2, &pstBgModel->stImg1, &pstBgModel->stErodeCtrl, bInstant);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_IVE_Dilate fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }

    bInstant = HI_TRUE;
    s32Ret = HI_MPI_IVE_CCL(&IveHandle, &pstBgModel->stImg1, &pstBgModel->stBlob, &pstBgModel->stCclCtrl, bInstant);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_IVE_CCL fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }
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

    SAMPLE_COMM_IVE_BlobToRect(pstBlob, &(pstBgModel->stRegion), 
		50, 8, pstExtFrmInfo->stVFrame.u32Width, pstExtFrmInfo->stVFrame.u32Height,
		u16BaseWidth,u16BaseHeight);

    return HI_SUCCESS;
}


static HI_VOID* SAMPLE_IVE_ViToVo(HI_VOID* pArgs)
{
    HI_S32 s32Ret;
    SAMPLE_IVE_BG_MODEL_INFO_S* pstBgModelInfo;
    SAMPLE_IVE_BG_MODEL_S* pstBgModel;
	VIDEO_FRAME_INFO_S stBaseFrmInfo;
	VIDEO_FRAME_INFO_S stExtFrmInfo;
    HI_S32 s32MilliSec = 20000;
    VO_CHN voChn = 0;
    VO_LAYER voLayer = 0;
    VENC_CHN vencChn = 0;
    HI_BOOL bEncode;
    HI_BOOL bVo;
	VPSS_GRP s32VpssGrp = 0;
	VPSS_CHN as32VpssChn[] = {VPSS_CHN0, VPSS_CHN1};

    pstBgModelInfo = (SAMPLE_IVE_BG_MODEL_INFO_S*)pArgs;
    pstBgModel = &(pstBgModelInfo->stBgModel);
    bEncode = pstBgModelInfo->bEncode;
    bVo = pstBgModelInfo->bVo;

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

        s32Ret = SAMPLE_IVE_BgModelProc(pstBgModelInfo, &stExtFrmInfo,(HI_U16)stBaseFrmInfo.stVFrame.u32Width,
			(HI_U16)stBaseFrmInfo.stVFrame.u32Height);		
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!=s32Ret, BASE_RELEASE,
			"SAMPLE_IVE_BgModelProc failed, Error(%#x)!\n", s32Ret);      
        
        //Draw rect
        s32Ret = SAMPLE_COMM_VGS_FillRect(&stBaseFrmInfo, &pstBgModel->stRegion, 0x0000FF00);
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
* function : show BgModel sample
******************************************************************************/
HI_VOID SAMPLE_IVE_BgModel(HI_CHAR chEncode, HI_CHAR chVo)
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
	
	memset(&s_stBgModelInfo,0,sizeof(s_stBgModelInfo));	
	memset(&s_stViConfig,0,sizeof(s_stViConfig));
	
	s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(&aenSize[0]);	
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_BG_MODEL_0, 
		"SAMPLE_COMM_VI_GetSizeBySensor failed,Error(%#x)!\n",s32Ret);	
	aenSize[1] = PIC_CIF;
	/******************************************
	 step  1: Init vb
	******************************************/ 
	s32Ret = SAMPLE_COMM_IVE_VbInit(aenSize,astSize,VPSS_CHN_NUM);	
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_BG_MODEL_0, 
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
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_BG_MODEL_1, 
		"SAMPLE_COMM_VI_StartVi failed,Error(%#x)!\n",s32Ret);		
	/******************************************
	 step 3: Start vpss
	******************************************/
	s32Ret = SAMPLE_COMM_IVE_StartVpss(astSize,VPSS_CHN_NUM);	
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_BG_MODEL_2, 
		"SAMPLE_IVS_StartVpss failed,Error(%#x)!\n",s32Ret);	
	/******************************************
	  step 4: Bind vpss to vi
	 ******************************************/
	s_bViVpssOnline = SAMPLE_COMM_IsViVpssOnline();
	if (HI_FALSE == s_bViVpssOnline) //if it is not online,we bind vpss to vi ,so,we can get data from vpss
	{
		s32Ret = SAMPLE_COMM_VI_BindVpss(s_stViConfig.enViMode); // VI --> VPSS
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_BG_MODEL_3, 
			"SAMPLE_COMM_VI_BindVpss failed,Error(%#x)!\n",s32Ret); 
		//Set vi frame
		s32Ret = HI_MPI_VI_GetChnAttr(s32ViChn,&stViChnAttr);		
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_BG_MODEL_4, 
			"HI_MPI_VI_GetChnAttr failed,Error(%#x)!\n",s32Ret);	
		stViChnAttr.s32SrcFrameRate = 3;
		stViChnAttr.s32DstFrameRate = 1;		
		s32Ret = HI_MPI_VI_SetChnAttr(s32ViChn,&stViChnAttr);		
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_BG_MODEL_4, 
			"HI_MPI_VI_SetChnAttr failed,Error(%#x)!\n",s32Ret);	
	}	
	else
	{
		//Set Vpss frame
		for (i = 0; i < VPSS_CHN_NUM; i++)
		{
			s32Ret = HI_MPI_VPSS_GetChnAttr(s32VpssGrp,as32VpssChn[i],&stVpssChnAttr);		
			SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_BG_MODEL_3, 
				"HI_MPI_VPSS_GetChnAttr failed,Error(%#x)!\n",s32Ret);	
			stVpssChnAttr.s32SrcFrameRate = 3;
			stVpssChnAttr.s32DstFrameRate = 1;
			s32Ret = HI_MPI_VPSS_SetChnAttr(s32VpssGrp,as32VpssChn[i],&stVpssChnAttr);		
			SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_BG_MODEL_3, 
				"HI_MPI_VPSS_SetChnAttr failed,Error(%#x)!\n",s32Ret);	

		}

	}
	/******************************************
	  [step 5]: Start Vo
	 ******************************************/
	 if (HI_TRUE == bVo)
	 {
		 s32Ret = SAMPLE_COMM_IVE_StartVo();
		 SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_BG_MODEL_4, 
			 "SAMPLE_COMM_IVE_StartVo failed, Error(%#x)!\n", s32Ret);

	 }

	 if (HI_TRUE == bEncode)
	 {
		 /******************************************
		   [step 5]: Start Venc
		  ******************************************/	
		  s32Ret = SAMPLE_COMM_VENC_Start(s_VeH264Chn, enStreamType,enVideoNorm,aenSize[0],enRcMode,0,ROTATE_NONE);
		  SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_BG_MODEL_5, 
			  "SAMPLE_COMM_VENC_Start failed,Error(%#x)!\n",s32Ret);  
		  /******************************************
		   [step 5.1]: Venc start get stream
		  ******************************************/	
		  s32Ret = SAMPLE_COMM_VENC_StartGetStream(1);
		  SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_BG_MODEL_6, 
			  "SAMPLE_COMM_VENC_StartGetStream failed,Error(%#x)!\n",s32Ret); 

	 }
	
	/******************************************
	  step 6: Init BgModel
	 ******************************************/		 
    s32Ret = SAMPLE_IVE_BgModel_Init(&(s_stBgModelInfo.stBgModel), astSize[1].u32Width, astSize[1].u32Height);
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_BG_MODEL_7, 
		"SAMPLE_IVE_BgModel_Init failed, Error(%#x)!\n", s32Ret);	

	s_stBgModelInfo.bEncode = bEncode;
	s_stBgModelInfo.bVo = bVo;
	s_bStopSignal = HI_FALSE;
	//Start vi to vo thread
	pthread_create(&s_hIveThread, 0, SAMPLE_IVE_ViToVo, (HI_VOID*)&s_stBgModelInfo);

	SAMPLE_VI_PAUSE();
		
	s_bStopSignal = HI_TRUE;
	pthread_join(s_hIveThread, HI_NULL);
	s_hIveThread = 0;
	SAMPLE_IVE_BgModel_Uninit(&(s_stBgModelInfo.stBgModel));
END_BG_MODEL_7:
	if (HI_TRUE == bEncode)
	{		
		SAMPLE_COMM_VENC_StopGetStream();
	}
END_BG_MODEL_6:
	if (HI_TRUE == bEncode)
	{		
		SAMPLE_COMM_VENC_Stop(s_VeH264Chn);
	}
END_BG_MODEL_5:
	if (HI_TRUE == bVo)
	{
		SAMPLE_COMM_IVE_StopVo();
	}
END_BG_MODEL_4:
	if (HI_FALSE == s_bViVpssOnline )
	{
	   SAMPLE_COMM_VI_UnBindVpss(s_stViConfig.enViMode);
	}
END_BG_MODEL_3:	
	SAMPLE_COMM_IVE_StopVpss(VPSS_CHN_NUM);
END_BG_MODEL_2:	
	SAMPLE_COMM_VI_StopVi(&s_stViConfig);
END_BG_MODEL_1:	//system exit
	SAMPLE_COMM_SYS_Exit();
	memset(&s_stBgModelInfo,0,sizeof(s_stBgModelInfo));
	memset(&s_stViConfig,0,sizeof(s_stViConfig));	
	s_bViVpssOnline = HI_TRUE;
END_BG_MODEL_0:	
	
	return ;    
}
/******************************************************************************
* function : BgModel sample signal handle
******************************************************************************/
HI_VOID SAMPLE_IVE_BgModel_HandleSig(HI_VOID)
{
	s_bStopSignal = HI_TRUE;
	if (0 != s_hIveThread)
	{
		pthread_join(s_hIveThread, HI_NULL);	
		s_hIveThread = 0;
	}
	SAMPLE_IVE_BgModel_Uninit(&(s_stBgModelInfo.stBgModel));
	if (HI_TRUE == s_stBgModelInfo.bEncode)
	{
		SAMPLE_COMM_VENC_StopGetStream();
		SAMPLE_COMM_VENC_Stop(s_VeH264Chn);
	}
	if (HI_TRUE == s_stBgModelInfo.bVo)
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

	memset(&s_stBgModelInfo,0,sizeof(s_stBgModelInfo));
	memset(&s_stViConfig,0,sizeof(s_stViConfig));
	s_bViVpssOnline = HI_TRUE;
}

