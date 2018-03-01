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
#include "ivs_md.h"

#include "sample_comm_ive.h"

#define SAMPLE_IVE_MD_IMAGE_NUM 2
#define VPSS_CHN_NUM 2
typedef struct hiSAMPLE_IVE_MD_S
{
	IVE_SRC_IMAGE_S astImg[SAMPLE_IVE_MD_IMAGE_NUM];	
	IVE_DST_MEM_INFO_S stBlob;
	MD_ATTR_S stMdAttr;		
	SAMPLE_RECT_ARRAY_S stRegion;

}SAMPLE_IVE_MD_S;

static HI_BOOL s_bStopSignal = HI_FALSE;
static HI_BOOL s_bViVpssOnline = HI_TRUE; 
static pthread_t s_hMdThread = 0;
static SAMPLE_IVE_MD_S s_stMd; 	
static SAMPLE_VI_CONFIG_S s_stViConfig;

static HI_VOID SAMPLE_IVE_Md_Uninit(SAMPLE_IVE_MD_S *pstMd)
{
	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	
	for (i = 0; i < SAMPLE_IVE_MD_IMAGE_NUM; i++)
	{	
    	IVE_MMZ_FREE(pstMd->astImg[i].u32PhyAddr[0],pstMd->astImg[i].pu8VirAddr[0]);
	}
	
    IVE_MMZ_FREE(pstMd->stBlob.u32PhyAddr,pstMd->stBlob.pu8VirAddr);
		
	s32Ret = HI_IVS_MD_Exit();
	if(s32Ret != HI_SUCCESS)
	{
	   SAMPLE_PRT("HI_IVS_MD_Exit fail,Error(%#x)\n",s32Ret);
	   return ;
	}
	
}

static HI_S32 SAMPLE_IVE_Md_Init(SAMPLE_IVE_MD_S *pstMd,HI_U16 u16ExtWidth,HI_U16 u16ExtHeight,
			HI_U16 u16BaseWidth,HI_U16 u16BaseHeight)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i ;
	HI_U32 u32Size;
	HI_U8 u8WndSz;	
	
   	memset(pstMd,0,sizeof(SAMPLE_IVE_MD_S));
	for (i = 0;i < SAMPLE_IVE_MD_IMAGE_NUM;i++)
	{
		s32Ret = SAMPLE_COMM_IVE_CreateImage(&pstMd->astImg[i],IVE_IMAGE_TYPE_U8C1,u16ExtWidth,u16ExtHeight);
		if(s32Ret != HI_SUCCESS)
		{
		   SAMPLE_PRT("SAMPLE_COMM_IVE_CreateImage fail\n");
		   goto MD_INIT_FAIL;
		}
	}
	u32Size = sizeof(IVE_CCBLOB_S);
	s32Ret = SAMPLE_COMM_IVE_CreateMemInfo(&pstMd->stBlob,u32Size);
    if(s32Ret != HI_SUCCESS)
	{
	   SAMPLE_PRT("SAMPLE_COMM_IVE_CreateMemInfo fail\n");
	   goto MD_INIT_FAIL;
	}
	
	//Set attr info
	pstMd->stMdAttr.enAlgMode = MD_ALG_MODE_BG;
	pstMd->stMdAttr.enSadMode = IVE_SAD_MODE_MB_4X4;
	pstMd->stMdAttr.enSadOutCtrl = IVE_SAD_OUT_CTRL_THRESH;
	pstMd->stMdAttr.u16SadThr = 100 * (1 << 1);//100 * (1 << 2);
	pstMd->stMdAttr.u16Width = u16ExtWidth;
	pstMd->stMdAttr.u16Height = u16ExtHeight;
	pstMd->stMdAttr.stAddCtrl.u0q16X = 32768;
	pstMd->stMdAttr.stAddCtrl.u0q16Y = 32768;
	pstMd->stMdAttr.stCclCtrl.enMode = IVE_CCL_MODE_4C;
	u8WndSz = ( 1 << (2 + pstMd->stMdAttr.enSadMode));
	pstMd->stMdAttr.stCclCtrl.u16InitAreaThr = u8WndSz * u8WndSz;
	pstMd->stMdAttr.stCclCtrl.u16Step = u8WndSz;

	s32Ret = HI_IVS_MD_Init();
	if(s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("HI_IVS_MD_Init fail,Error(%#x)\n",s32Ret);
		goto MD_INIT_FAIL;
	}

MD_INIT_FAIL:
    
    if(HI_SUCCESS != s32Ret)
	{
        SAMPLE_IVE_Md_Uninit(pstMd);
	}
	return s32Ret;    
    
}
static HI_VOID * SAMPLE_IVE_MdProc(HI_VOID * pArgs)
{
    HI_S32 s32Ret;
	SAMPLE_IVE_MD_S *pstMd;
	MD_ATTR_S *pstMdAttr;
	VIDEO_FRAME_INFO_S stBaseFrmInfo;
	VIDEO_FRAME_INFO_S stExtFrmInfo;
	VPSS_GRP s32VpssGrp = 0;
	VPSS_CHN as32VpssChn[] = {VPSS_CHN0, VPSS_CHN1};
    HI_S32 s32MilliSec = 20000;
  	MD_CHN MdChn = 0;
  
    HI_BOOL bInstant = HI_TRUE;
    VO_LAYER voLayer = 0;
	VO_CHN voChn = 0;
 	HI_S32 s32CurIdx = 0;
	HI_BOOL bFirstFrm = HI_TRUE;
	IVE_DST_IMAGE_S stSad;

	memset(&stSad, 0, sizeof(IVE_DST_IMAGE_S));
	
	pstMd = (SAMPLE_IVE_MD_S *)(pArgs);   
	pstMdAttr = &(pstMd->stMdAttr);
	//Create chn
	s32Ret = HI_IVS_MD_CreateChn(MdChn,&(pstMd->stMdAttr));	
	if (HI_SUCCESS != s32Ret)
	{
        SAMPLE_PRT("HI_IVS_MD_CreateChn fail,Error(%#x)\n",s32Ret);
        return NULL;
    }
		
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

		if (HI_TRUE != bFirstFrm)
		{		
			s32Ret = SAMPLE_COMM_IVE_DmaImage(&stExtFrmInfo,&pstMd->astImg[s32CurIdx],bInstant);
			SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, BASE_RELEASE,
							"SAMPLE_COMM_IVE_DmaImage fail,Error(%#x)\n",s32Ret);			
		}
		else
		{		
			s32Ret = SAMPLE_COMM_IVE_DmaImage(&stExtFrmInfo,&pstMd->astImg[1 - s32CurIdx],bInstant);			
			SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, BASE_RELEASE,
							"SAMPLE_COMM_IVE_DmaImage fail,Error(%#x)\n",s32Ret);	
			
			bFirstFrm = HI_FALSE;			
			goto CHANGE_IDX;//first frame just init reference frame

		}
		
		s32Ret = HI_IVS_MD_Process(MdChn,&pstMd->astImg[s32CurIdx],&pstMd->astImg[1 - s32CurIdx],&stSad,&pstMd->stBlob);		
        SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, BASE_RELEASE,
							"HI_IVS_MD_Process fail,Error(%#x)\n",s32Ret);		   
		 
		SAMPLE_COMM_IVE_BlobToRect((IVE_CCBLOB_S *)pstMd->stBlob.pu8VirAddr,&(pstMd->stRegion),50,8,
			pstMdAttr->u16Width,pstMdAttr->u16Height,(HI_U16)stBaseFrmInfo.stVFrame.u32Width,(HI_U16)stBaseFrmInfo.stVFrame.u32Height);
		
	    //Draw rect
		s32Ret = SAMPLE_COMM_VGS_FillRect(&stBaseFrmInfo, &pstMd->stRegion, 0x0000FF00);
		 SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, BASE_RELEASE,
							"SAMPLE_COMM_VGS_FillRect fail,Error(%#x)\n",s32Ret);	
		 
		 s32Ret = HI_MPI_VO_SendFrame(voLayer,voChn,&stBaseFrmInfo,s32MilliSec);
		 SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, BASE_RELEASE,
									 "HI_MPI_VO_SendFrame fail,Error(%#x)\n",s32Ret);	 
		CHANGE_IDX:
			//Change reference and current frame index
			s32CurIdx =	1 - s32CurIdx;

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

	 //destroy 
	 s32Ret = HI_IVS_MD_DestroyChn(MdChn);	 
	 if (HI_SUCCESS != s32Ret)
	 {
		 SAMPLE_PRT("HI_IVS_MD_DestroyChn fail,Error(%#x)\n",s32Ret);  
	 }

     return HI_NULL;
}

HI_VOID SAMPLE_IVE_Md(HI_VOID)
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
	
    memset(&s_stMd,0,sizeof(s_stMd));
	memset(&s_stViConfig,0,sizeof(s_stViConfig));
	
    s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(&aenSize[0]);	
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_MD_0, 
		"SAMPLE_COMM_VI_GetSizeBySensor failed,Error(%#x)!\n",s32Ret);	
	aenSize[1] = PIC_D1;
    /******************************************
     step  1: Init vb
    ******************************************/ 
    s32Ret = SAMPLE_COMM_IVE_VbInit(aenSize,astSize,VPSS_CHN_NUM);	
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_MD_0, 
		"SAMPLE_IVS_VbInit failed,Error(%#x)!\n",s32Ret);	
    /******************************************
     step 2: Start vi
    ******************************************/
    s_stViConfig.enViMode   = SENSOR_TYPE;
	s_stViConfig.enRotate   = ROTATE_NONE;
	s_stViConfig.enNorm	  = VIDEO_ENCODING_MODE_AUTO;
	s_stViConfig.enViChnSet = VI_CHN_SET_NORMAL;
	s_stViConfig.enWDRMode  = WDR_MODE_NONE;
	s32Ret = SAMPLE_COMM_VI_StartVi(&s_stViConfig);	
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_MD_1, 
		"SAMPLE_COMM_VI_StartVi failed,Error(%#x)!\n",s32Ret);		
    /******************************************
     step 3: Start vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_IVE_StartVpss(astSize,VPSS_CHN_NUM);	
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_MD_2, 
		"SAMPLE_IVS_StartVpss failed,Error(%#x)!\n",s32Ret);	
	/******************************************
	  step 4: Bind vpss to vi
	 ******************************************/
 	s_bViVpssOnline = SAMPLE_COMM_IsViVpssOnline();
	if (HI_FALSE == s_bViVpssOnline) //if it is not online,we bind vpss to vi ,so,we can get data from vpss
	{
		s32Ret = SAMPLE_COMM_VI_BindVpss(s_stViConfig.enViMode); // VI --> VPSS
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_MD_3, 
			"SAMPLE_COMM_VI_BindVpss failed,Error(%#x)!\n",s32Ret);	
		//Set vi frame
		s32Ret = HI_MPI_VI_GetChnAttr(s32ViChn,&stViChnAttr);		
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_MD_4, 
			"HI_MPI_VI_GetChnAttr failed,Error(%#x)!\n",s32Ret);	
		stViChnAttr.s32SrcFrameRate = 3;
		stViChnAttr.s32DstFrameRate = 1;		
		s32Ret = HI_MPI_VI_SetChnAttr(s32ViChn,&stViChnAttr);		
		SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_MD_4, 
			"HI_MPI_VI_SetChnAttr failed,Error(%#x)!\n",s32Ret);	
	}	
	else
	{
		//Set Vpss frame
		for (i = 0; i < VPSS_CHN_NUM; i++)
		{
			s32Ret = HI_MPI_VPSS_GetChnAttr(s32VpssGrp,as32VpssChn[i],&stVpssChnAttr);		
			SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_MD_3, 
				"HI_MPI_VPSS_GetChnAttr failed,Error(%#x)!\n",s32Ret);	
			stVpssChnAttr.s32SrcFrameRate = 3;
			stVpssChnAttr.s32DstFrameRate = 1;
			s32Ret = HI_MPI_VPSS_SetChnAttr(s32VpssGrp,as32VpssChn[i],&stVpssChnAttr);		
			SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_MD_3, 
				"HI_MPI_VPSS_SetChnAttr failed,Error(%#x)!\n",s32Ret);	

		}

	}
	/******************************************
	  step 5: Start Vo
	 ******************************************/
	s32Ret = SAMPLE_COMM_IVE_StartVo();
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_MD_4, 
		"SAMPLE_IVS_StartVo failed, Error(%#x)!\n", s32Ret);	
	/******************************************
	  step 6: Init Md
	 ******************************************/	
	s32Ret = SAMPLE_IVE_Md_Init(&s_stMd,astSize[1].u32Width,astSize[1].u32Height,
	astSize[0].u32Width,astSize[0].u32Height);
	SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_MD_5, 
		"SAMPLE_IVE_Md_Init failed, Error(%#x)!\n", s32Ret);
	s_bStopSignal = HI_FALSE;
	pthread_create(&s_hMdThread, 0, SAMPLE_IVE_MdProc, (HI_VOID *)&s_stMd);
	
    SAMPLE_VI_PAUSE();
	s_bStopSignal = HI_TRUE;
	pthread_join(s_hMdThread, HI_NULL);
	s_hMdThread = 0;

    SAMPLE_IVE_Md_Uninit(&(s_stMd));	
END_MD_5:
	SAMPLE_COMM_IVE_StopVo();
END_MD_4:
	if (HI_FALSE == s_bViVpssOnline )
	{
	   SAMPLE_COMM_VI_UnBindVpss(s_stViConfig.enViMode);
	}
END_MD_3:	
	SAMPLE_COMM_IVE_StopVpss(VPSS_CHN_NUM);
END_MD_2:	
	SAMPLE_COMM_VI_StopVi(&s_stViConfig);
END_MD_1:	//system exit
    SAMPLE_COMM_SYS_Exit();	
    memset(&s_stMd,0,sizeof(s_stMd));
	memset(&s_stViConfig,0,sizeof(s_stViConfig));	
	s_bViVpssOnline = HI_TRUE;
END_MD_0:	
	return ;    

}

/******************************************************************************
* function : Md sample signal handle
******************************************************************************/
HI_VOID SAMPLE_IVE_Md_HandleSig(HI_VOID)
{
	s_bStopSignal = HI_TRUE;
	if (0 != s_hMdThread)
	{
		pthread_join(s_hMdThread, HI_NULL);	
		s_hMdThread = 0;
	}
    SAMPLE_IVE_Md_Uninit(&(s_stMd));	
	SAMPLE_COMM_IVE_StopVo();	
	if (HI_FALSE == s_bViVpssOnline )
	{
	   SAMPLE_COMM_VI_UnBindVpss(s_stViConfig.enViMode);
	}
	SAMPLE_COMM_IVE_StopVpss(VPSS_CHN_NUM);
	SAMPLE_COMM_VI_StopVi(&s_stViConfig);	
	SAMPLE_COMM_SYS_Exit();	

    memset(&s_stMd,0,sizeof(s_stMd));
	memset(&s_stViConfig,0,sizeof(s_stViConfig));
	s_bViVpssOnline = HI_TRUE;
}



