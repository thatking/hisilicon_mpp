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
#include <math.h>
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

#define VPSS_CHN_NUM 2
typedef struct hiSAMPLE_IVE_OD_S
{
    IVE_SRC_IMAGE_S stSrc;
    IVE_DST_IMAGE_S stInteg;
    IVE_INTEG_CTRL_S stIntegCtrl;
    HI_U32 u32W;
    HI_U32 u32H;
} SAMPLE_IVE_OD_S;

static HI_BOOL s_bStopSignal = HI_FALSE;
static HI_BOOL s_bViVpssOnline = HI_TRUE;
static pthread_t s_hIveThread = 0;
static SAMPLE_IVE_OD_S s_stOd;
static SAMPLE_VI_CONFIG_S s_stViConfig;

static HI_VOID SAMPLE_IVE_Od_Uninit(SAMPLE_IVE_OD_S* pstOd)
{
    IVE_MMZ_FREE(pstOd->stSrc.u32PhyAddr[0], pstOd->stSrc.pu8VirAddr[0]);
    IVE_MMZ_FREE(pstOd->stInteg.u32PhyAddr[0], pstOd->stInteg.pu8VirAddr[0]);

}

static HI_S32 SAMPLE_IVE_Od_Init(SAMPLE_IVE_OD_S* pstOd, HI_U16 u16Width, HI_U16 u16Height)
{
    HI_S32 s32Ret = HI_SUCCESS;

    memset(pstOd, 0, sizeof(SAMPLE_IVE_OD_S));

    s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstOd->stSrc, IVE_IMAGE_TYPE_U8C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
        goto OD_INIT_FAIL;
    }
    s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstOd->stInteg, IVE_IMAGE_TYPE_U64C1, u16Width, u16Height);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
        goto OD_INIT_FAIL;
    }

    pstOd->stIntegCtrl.enOutCtrl = IVE_INTEG_OUT_CTRL_COMBINE;

    pstOd->u32W = u16Width / IVE_CHAR_CALW;
    pstOd->u32H = u16Height / IVE_CHAR_CALH;


OD_INIT_FAIL:

    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_IVE_Od_Uninit(pstOd);
    }
    return s32Ret;

}

static HI_S32 SAMPLE_IVE_Linear2DClassifer(POINT_S* pstChar, HI_S32 s32CharNum,
        POINT_S* pstLinearPoint, HI_S32 s32Linearnum )
{
    HI_S32 s32ResultNum;
    HI_S32 i, j;
    HI_BOOL bTestFlag;
    POINT_S* pstNextLinearPoint;

    s32ResultNum = 0;
    pstNextLinearPoint = &pstLinearPoint[1];
    for (i = 0; i < s32CharNum; i++)
    {
        bTestFlag = HI_TRUE;
        for (j = 0; j < (s32Linearnum - 1); j++)
        {
            if ( ( (pstChar[i].s32Y - pstLinearPoint[j].s32Y) * (pstNextLinearPoint[j].s32X - pstLinearPoint[j].s32X) >
                   (pstChar[i].s32X - pstLinearPoint[j].s32X) * (pstNextLinearPoint[j].s32Y - pstLinearPoint[j].s32Y)
                   && (pstNextLinearPoint[j].s32X != pstLinearPoint[j].s32X))
                 || ( (pstChar[i].s32X > pstLinearPoint[j].s32X) && (pstNextLinearPoint[j].s32X == pstLinearPoint[j].s32X) ))
            {
                bTestFlag = HI_FALSE;
                break;
            }

        }
        if (HI_TRUE == bTestFlag)
        {
            s32ResultNum++;
        }
    }
    return s32ResultNum;
}


static HI_VOID* SAMPLE_IVE_OdProc(HI_VOID* pArgs)
{
    HI_S32 s32Ret;
    HI_U32 i, j;
    SAMPLE_IVE_OD_S* pstOd;
    VIDEO_FRAME_INFO_S stExtFrmInfo;
	VIDEO_FRAME_INFO_S stBaseFrmInfo;
	VPSS_GRP s32VpssGrp = 0;
	VPSS_CHN as32VpssChn[] = {VPSS_CHN0, VPSS_CHN1};
    HI_S32 s32MilliSec = 20000;
    IVE_DATA_S stSrc;
    IVE_DATA_S stDst;
    IVE_HANDLE IveHandle;
    HI_BOOL bFinish = HI_FALSE;
    HI_BOOL bBlock = HI_TRUE;
    HI_BOOL bInstant = HI_TRUE;
    VO_LAYER voLayer = 0;
    VO_CHN voChn = 0;

    POINT_S stChar[IVE_CHAR_NUM];
    POINT_S astPoints[10] = {{0, 0}};
    IVE_LINEAR_DATA_S stIveLinerData;
    HI_U64* pu64VirData = HI_NULL;
    IVE_DMA_CTRL_S stDmaCtrl = {IVE_DMA_MODE_DIRECT_COPY, 0, 0, 0, 0};

    HI_U64 u64TopLeft, u64TopRight, u64BtmLeft, u64BtmRight;
    HI_U64* pu64TopRow, *pu64BtmRow;
    HI_U64 u64BlockSum, u64BlockSq;
    HI_FLOAT fSqVar;

    pstOd = (SAMPLE_IVE_OD_S*)(pArgs);
    pu64VirData = (HI_U64*)pstOd->stInteg.pu8VirAddr[0];

    stIveLinerData.pstLinearPoint = &astPoints[0];
    stIveLinerData.s32LinearNum = 2;
    stIveLinerData.s32ThreshNum = IVE_CHAR_NUM / 2;
    stIveLinerData.pstLinearPoint[0].s32X = 80;
    stIveLinerData.pstLinearPoint[0].s32Y = 0;
    stIveLinerData.pstLinearPoint[1].s32X = 80;
    stIveLinerData.pstLinearPoint[1].s32Y = 20;

    while (HI_FALSE == s_bStopSignal)
    {

		s32Ret = HI_MPI_VPSS_GetChnFrame(s32VpssGrp, as32VpssChn[1], &stExtFrmInfo, s32MilliSec);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VPSS_GetChnFrame fail,Error(%#x)\n", s32Ret);
            continue;
        }
		
		s32Ret = HI_MPI_VPSS_GetChnFrame(s32VpssGrp, as32VpssChn[0], &stBaseFrmInfo, s32MilliSec);		
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!=s32Ret, EXT_RELEASE,
			"HI_MPI_VPSS_GetChnFrame failed, VPSS_GRP(%d), VPSS_CHN(%d), Error(%#x)!\n",
			s32VpssGrp, as32VpssChn[0], s32Ret);

        stSrc.pu8VirAddr = (HI_U8*)stExtFrmInfo.stVFrame.pVirAddr[0];
        stSrc.u32PhyAddr = stExtFrmInfo.stVFrame.u32PhyAddr[0];
        stSrc.u16Stride = (HI_U16)stExtFrmInfo.stVFrame.u32Stride[0];
        stSrc.u16Width = (HI_U16)stExtFrmInfo.stVFrame.u32Width;
        stSrc.u16Height = stExtFrmInfo.stVFrame.u32Height;

        stDst.pu8VirAddr = pstOd->stSrc.pu8VirAddr[0];
        stDst.u32PhyAddr = pstOd->stSrc.u32PhyAddr[0];
        stDst.u16Stride = (HI_U16)stExtFrmInfo.stVFrame.u32Stride[0];
        stDst.u16Width = (HI_U16)stExtFrmInfo.stVFrame.u32Width;
        stDst.u16Height = stExtFrmInfo.stVFrame.u32Height;

        bInstant = HI_FALSE;
        s32Ret = HI_MPI_IVE_DMA(&IveHandle, &stSrc, &stDst, &stDmaCtrl, bInstant);		
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!=s32Ret, BASE_RELEASE,
			"HI_MPI_IVE_DMA failed,Error(%#x)!\n", s32Ret);

        bInstant = HI_TRUE;
        s32Ret = HI_MPI_IVE_Integ(&IveHandle, &pstOd->stSrc, &pstOd->stInteg, &pstOd->stIntegCtrl, bInstant);
        SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!=s32Ret, BASE_RELEASE,
        	"HI_MPI_IVE_Integ failed,Error(%#x)!\n", s32Ret);	
		
        s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, bBlock);
        while (HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
        {
            usleep(100);
            s32Ret = HI_MPI_IVE_Query(IveHandle, &bFinish, bBlock);
        }
		
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!=s32Ret, BASE_RELEASE,
			"HI_MPI_IVE_Query failed,Error(%#x)!\n", s32Ret);       

        for (j = 0; j < IVE_CHAR_CALH; j++)
        {
            pu64TopRow = (0 == j) ? (pu64VirData) : ( pu64VirData + (j * pstOd->u32H - 1) * pstOd->stInteg.u16Stride[0]);
            pu64BtmRow = pu64VirData + ((j + 1) * pstOd->u32H - 1) * pstOd->stInteg.u16Stride[0];

            for (i = 0; i < IVE_CHAR_CALW; i++)
            {
                u64TopLeft  = (0 == j) ? (0) : ((0 == i) ? (0) : (pu64TopRow[i * pstOd->u32W - 1]));
                u64TopRight = (0 == j) ? (0) : (pu64TopRow[(i + 1) * pstOd->u32W - 1]);
                u64BtmLeft  = (0 == i) ? (0) : (pu64BtmRow[i * pstOd->u32W - 1]);
                u64BtmRight = pu64BtmRow[(i + 1) * pstOd->u32W - 1];

                u64BlockSum = (u64TopLeft & 0xfffffffLL) + (u64BtmRight & 0xfffffffLL)
                              - (u64BtmLeft & 0xfffffffLL) - (u64TopRight & 0xfffffffLL);

                u64BlockSq  = (u64TopLeft >> 28) + (u64BtmRight >> 28)
                              - (u64BtmLeft >> 28) - (u64TopRight >> 28);

                // mean
                stChar[j * IVE_CHAR_CALW + i].s32X = u64BlockSum / (pstOd->u32W * pstOd->u32H);
                // sigma=sqrt(1/(w*h)*sum((x(i,j)-mean)^2)= sqrt(sum(x(i,j)^2)/(w*h)-mean^2)
                fSqVar = u64BlockSq / (pstOd->u32W * pstOd->u32H) - stChar[j * IVE_CHAR_CALW + i].s32X * stChar[j * IVE_CHAR_CALW + i].s32X;
                stChar[j * IVE_CHAR_CALW + i].s32Y = (HI_U32)sqrt(fSqVar);
            }
        }

        s32Ret = SAMPLE_IVE_Linear2DClassifer(&stChar[0], IVE_CHAR_NUM,
                                              stIveLinerData.pstLinearPoint, stIveLinerData.s32LinearNum);
        if (s32Ret > stIveLinerData.s32ThreshNum)
        {
            SAMPLE_PRT("\033[0;31m Occlusion detected!\033[0;39m\n");
        }
        else
        {
            SAMPLE_PRT("Enter any key to quit!\n");
        }

        //Send to vo
        s32Ret = HI_MPI_VO_SendFrame(voLayer, voChn, &stBaseFrmInfo, s32MilliSec);		
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS!=s32Ret, BASE_RELEASE,
			"HI_MPI_VO_SendFrame failed,Error(%#x)!\n", s32Ret);  
		
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


HI_VOID SAMPLE_IVE_Od(HI_VOID)
{
	SIZE_S astSize[VPSS_CHN_NUM];	
	PIC_SIZE_E aenSize[VPSS_CHN_NUM];
	
	VI_CHN_ATTR_S stViChnAttr;
	VI_CHN s32ViChn = 0;	
	VPSS_GRP s32VpssGrp = 0;
	VPSS_CHN as32VpssChn[] = {VPSS_CHN0, VPSS_CHN1};
	VPSS_CHN_ATTR_S stVpssChnAttr;

	HI_U32 i;
	
	HI_S32 s32Ret = HI_SUCCESS;
	memset(&s_stOd,0,sizeof(s_stOd));
	memset(&s_stViConfig,0,sizeof(s_stViConfig));
	
	s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(&aenSize[0]);	
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_OD_0, 
		"SAMPLE_COMM_VI_GetSizeBySensor failed,Error(%#x)!\n",s32Ret);	
	aenSize[1] = PIC_HD1080;
	/******************************************
	 step  1: Init vb
	******************************************/ 
	s32Ret = SAMPLE_COMM_IVE_VbInit(aenSize,astSize,VPSS_CHN_NUM);	
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_OD_0, 
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
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_OD_1, 
		"SAMPLE_COMM_VI_StartVi failed,Error(%#x)!\n",s32Ret);	
	
	/******************************************
	 step 3: Start vpss
	******************************************/
	s32Ret = SAMPLE_COMM_IVE_StartVpss(astSize,VPSS_CHN_NUM);	
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_OD_2, 
		"SAMPLE_IVS_StartVpss failed,Error(%#x)!\n",s32Ret);	
	/******************************************
	  step 4: Bind vpss to vi
	 ******************************************/
	s_bViVpssOnline = SAMPLE_COMM_IsViVpssOnline();
	if (HI_FALSE == s_bViVpssOnline) //if it is not online,we bind vpss to vi ,so,we can get data from vpss
	{
		s32Ret = SAMPLE_COMM_VI_BindVpss(s_stViConfig.enViMode); // VI --> VPSS
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_OD_3, 
			"SAMPLE_COMM_VI_BindVpss failed,Error(%#x)!\n",s32Ret); 
		//Set vi frame
		s32Ret = HI_MPI_VI_GetChnAttr(s32ViChn,&stViChnAttr);		
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_OD_4, 
			"HI_MPI_VI_GetChnAttr failed,Error(%#x)!\n",s32Ret);	
		stViChnAttr.s32SrcFrameRate = 3;
		stViChnAttr.s32DstFrameRate = 1;		
		s32Ret = HI_MPI_VI_SetChnAttr(s32ViChn,&stViChnAttr);		
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_OD_4, 
			"HI_MPI_VI_SetChnAttr failed,Error(%#x)!\n",s32Ret);	
	}	
	else
	{
		//Set Vpss frame
		for (i = 0; i < VPSS_CHN_NUM; i++)
		{
			s32Ret = HI_MPI_VPSS_GetChnAttr(s32VpssGrp,as32VpssChn[i],&stVpssChnAttr);		
			SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_OD_3, 
				"HI_MPI_VPSS_GetChnAttr failed,Error(%#x)!\n",s32Ret);	
			stVpssChnAttr.s32SrcFrameRate = 3;
			stVpssChnAttr.s32DstFrameRate = 1;
			s32Ret = HI_MPI_VPSS_SetChnAttr(s32VpssGrp,as32VpssChn[i],&stVpssChnAttr);		
			SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_OD_3, 
				"HI_MPI_VPSS_SetChnAttr failed,Error(%#x)!\n",s32Ret);	

		}

	}
	/******************************************
	  step 5: Start Vo
	 ******************************************/
	 s32Ret = SAMPLE_COMM_IVE_StartVo();
	 SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_OD_4, 
		 "SAMPLE_COMM_IVE_StartVo failed, Error(%#x)!\n", s32Ret);
	/******************************************
	  step 6: Init OD
	 ******************************************/	
	 
    s32Ret = SAMPLE_IVE_Od_Init(&s_stOd, astSize[1].u32Width, astSize[1].u32Height);
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_OD_5, 
		"SAMPLE_IVE_Od_Init failed, Error(%#x)!\n", s32Ret);	

	s_bStopSignal = HI_FALSE;
	//Start vi to vo thread
    pthread_create(&s_hIveThread, 0, SAMPLE_IVE_OdProc, (HI_VOID*)&s_stOd);

    SAMPLE_VI_PAUSE();

    s_bStopSignal = HI_TRUE;
    pthread_join(s_hIveThread, HI_NULL);
	s_hIveThread = 0;
    SAMPLE_IVE_Od_Uninit(&(s_stOd));

END_OD_5:
	SAMPLE_COMM_IVE_StopVo();
END_OD_4:
	if (HI_FALSE == s_bViVpssOnline )
	{
	   SAMPLE_COMM_VI_UnBindVpss(s_stViConfig.enViMode);
	}
END_OD_3:	
	SAMPLE_COMM_IVE_StopVpss(VPSS_CHN_NUM);
END_OD_2:	
	SAMPLE_COMM_VI_StopVi(&s_stViConfig);	
	memset(&s_stOd,0,sizeof(s_stOd));
	memset(&s_stViConfig,0,sizeof(s_stViConfig));	
	s_bViVpssOnline = HI_TRUE;
END_OD_1:	//system exit
	SAMPLE_COMM_SYS_Exit();
END_OD_0:	
		
	return ;   
}

/******************************************************************************
* function : Od sample signal handle
******************************************************************************/
HI_VOID SAMPLE_IVE_Od_HandleSig(HI_VOID)
{
	s_bStopSignal = HI_TRUE;
	if (0 != s_hIveThread)
	{
		pthread_join(s_hIveThread, HI_NULL);	
		s_hIveThread = 0;
	}
    SAMPLE_IVE_Od_Uninit(&(s_stOd));
	SAMPLE_COMM_IVE_StopVo();	
	if (HI_FALSE == s_bViVpssOnline )
	{
	   SAMPLE_COMM_VI_UnBindVpss(s_stViConfig.enViMode);
	}
	SAMPLE_COMM_IVE_StopVpss(VPSS_CHN_NUM);
	SAMPLE_COMM_VI_StopVi(&s_stViConfig);	
	SAMPLE_COMM_SYS_Exit();	

	memset(&s_stOd,0,sizeof(s_stOd));
	memset(&s_stViConfig,0,sizeof(s_stViConfig));
	s_bViVpssOnline = HI_TRUE;
}

