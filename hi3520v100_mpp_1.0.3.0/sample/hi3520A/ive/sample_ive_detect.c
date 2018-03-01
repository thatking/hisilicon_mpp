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
#include <math.h>
#include <signal.h>

#if 0
#include "hi_common.h"
#include "hi_comm_vi.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"

#include "mpi_vb.h"
#include "mpi_sys.h"
#include "mpi_vi.h"
#endif

#include "mpi_ive.h"
#include "hi_comm_ive.h"

#include "sample_comm.h"


#define IVECHARCALH 8
#define IVECHARCALW 8
#define IVECHARNUM IVECHARCALW*IVECHARCALH

VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_PAL;
HI_U32 gs_u32ViFrmRate = 0; 

HI_S32 SAMPLE_VI_1_D1(HI_VOID)
{
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_1_D1;

    HI_U32 u32ViChnCnt = 1;
    
    VB_CONF_S stVbConf;
    
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;

    /******************************************
     step  1: init variable 
    ******************************************/
    gs_u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL== gs_enNorm)?25:30;
    
    memset(&stVbConf,0,sizeof(VB_CONF_S));

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
                PIC_D1, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.u32MaxPoolCnt = 128;

    /* video buffer*/
    //todo: vb=15
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 8;

    /******************************************
     step 2: mpp system init. 
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END;
    }

    /******************************************
     step 3: start vi dev & chn
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_Start(enViMode, gs_enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END;
    }

    
END: 
    return s32Ret;

}



HI_S32   SAMPLE_IVE_MST_INIT_MPI()
{
    HI_S32 sRet = HI_FAILURE;
    VB_CONF_S       stVbConf;
    MPP_SYS_CONF_S   stSysConf;
	HI_BOOL bMpiInit=HI_FALSE;
    if(HI_TRUE == bMpiInit)
    {
        printf("MPI has been inited \n ");
        return sRet;
    }
    /*初始化之前先确定系统已退出*/
    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();

    /*VB初始化之前先配置VB*/
    memset(&stVbConf,0,sizeof(VB_CONF_S));

    /*1080p*/
    stVbConf.u32MaxPoolCnt = 64;
    stVbConf.astCommPool[0].u32BlkSize   = 1920*1088*2;
    stVbConf.astCommPool[0].u32BlkCnt    = 0;

     /*720p*/

    stVbConf.astCommPool[1].u32BlkSize   = 1280*720*2;
    stVbConf.astCommPool[1].u32BlkCnt    = 0;


     /*D1*/

    stVbConf.astCommPool[2].u32BlkSize   = 768*576*2;
    stVbConf.astCommPool[2].u32BlkCnt    = 0;


     /*Cif*/

    stVbConf.astCommPool[3].u32BlkSize   = 384*288*2;
    stVbConf.astCommPool[3].u32BlkCnt    = 0;


  #if 1
    stVbConf.astCommPool[4].u32BlkSize   = 176*144*2;
    stVbConf.astCommPool[4].u32BlkCnt    = 0;

   #endif
    sRet = HI_MPI_VB_SetConf(&stVbConf);
    if(HI_SUCCESS != sRet)
    {
        printf("Config VB fail!\n");
        return sRet;
    }


    sRet = HI_MPI_VB_Init();
    if(HI_SUCCESS != sRet)
    {
        printf("Init VB fail!\n");
        return sRet;
    }


    memset(&stSysConf,0,sizeof(MPP_SYS_CONF_S));
    stSysConf.u32AlignWidth = 16;

    sRet = HI_MPI_SYS_SetConf(&stSysConf);
    if(HI_SUCCESS != sRet)
    {
        printf("Config sys fail!\n");
        HI_MPI_VB_Exit();
        return sRet;
    }


    sRet = HI_MPI_SYS_Init();
     if(HI_SUCCESS != sRet)
    {
        printf("Init sys fail!\n");
        HI_MPI_VB_Exit();
        return sRet;
    }

    bMpiInit = HI_TRUE;

    return sRet;
}


HI_BOOL g_bStopSignal;

typedef struct hiIVE_LINEAR_DATA_S
{
	HI_S32 s32LinearNum;
	HI_S32 s32ThreshNum;
	POINT_S *pstLinearPoint;
}IVE_LINEAR_DATA_S;


HI_S32 SAMPLE_IVE_Linear2DClassifer(POINT_S *pstChar, HI_S32 s32CharNum, 
                                            POINT_S *pstLinearPoint, HI_S32 s32Linearnum )
{
	HI_S32 s32ResultNum;
	HI_S32 i,j;
	HI_BOOL bTestFlag;
	POINT_S *pstNextLinearPoint;
	
	s32ResultNum=0;
	pstNextLinearPoint=&pstLinearPoint[1];	
	for(i=0;i<s32CharNum;i++)
	{
		bTestFlag=HI_TRUE;
		for(j=0;j<(s32Linearnum-1);j++)
		{

			if(   ( (pstChar[i].s32Y-pstLinearPoint[j].s32Y)*(pstNextLinearPoint[j].s32X-pstLinearPoint[j].s32X)>
				  (pstChar[i].s32X-pstLinearPoint[j].s32X)*(pstNextLinearPoint[j].s32Y-pstLinearPoint[j].s32Y) 
				   && (pstNextLinearPoint[j].s32X!=pstLinearPoint[j].s32X))
			   || ( (pstChar[i].s32X<pstLinearPoint[j].s32X) && (pstNextLinearPoint[j].s32X==pstLinearPoint[j].s32X) ))	
			{
				bTestFlag=HI_FALSE;
				break;
			}
		}
		if(bTestFlag==HI_TRUE)
		{
			s32ResultNum++;
		}
	}
	return s32ResultNum;
}

HI_VOID * SAMPLE_IVE_BlockDetect(HI_VOID *pArgs)
{
	VIDEO_FRAME_INFO_S stFrameInfo;
	HI_S32 s32Ret,s32LinearNum;
	HI_S32 s32ThreshNum;
	IVE_MEM_INFO_S stDst;
	IVE_LINEAR_DATA_S *pstIveLinerData;
	POINT_S *pstLinearPoint;
	
	pstIveLinerData=(IVE_LINEAR_DATA_S * )pArgs;
	s32LinearNum=pstIveLinerData->s32LinearNum;
	pstLinearPoint=pstIveLinerData->pstLinearPoint;
	s32ThreshNum=pstIveLinerData->s32ThreshNum;
	
	stDst.u32PhyAddr=0;

		while(1)
		{
			IVE_SRC_INFO_S stSrc;
			IVE_HANDLE hIveHandle;
			HI_U64 *pu64VirData;
			int i,j;
			POINT_S stChar[IVECHARNUM];
			int w,h;
            HI_U32 u32Depth = 2;

            
            s32Ret = HI_MPI_VI_SetFrameDepth(0, u32Depth);
            if (HI_SUCCESS != s32Ret)
            {
                printf("set max depth err:0x%x\n", s32Ret);
                return s32Ret;
            }
            
			s32Ret = HI_MPI_VI_GetFrameTimeOut(0, &stFrameInfo, 0);
            
			if(s32Ret!=HI_SUCCESS)
			{
				 printf("can't get vi frame for %x\n",s32Ret);
				 return HI_NULL;
			}


            stSrc.u32Width = stFrameInfo.stVFrame.u32Width;
			stSrc.u32Height = stFrameInfo.stVFrame.u32Height;
			stSrc.stSrcMem.u32PhyAddr = stFrameInfo.stVFrame.u32PhyAddr[0];
			stSrc.stSrcMem.u32Stride = stFrameInfo.stVFrame.u32Stride[0];
			stSrc.enSrcFmt = IVE_SRC_FMT_SINGLE;
			if(stDst.u32PhyAddr==0)
			{
				s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&stDst.u32PhyAddr, (HI_VOID *)&pu64VirData, 
					"User", HI_NULL, stSrc.u32Height * stSrc.u32Width*8);
				if(s32Ret!=HI_SUCCESS)
				{
					 printf("can't alloc intergal memory for %x\n",s32Ret);
					 return HI_NULL;
				}			
				stDst.u32Stride = stFrameInfo.stVFrame.u32Width;
				w = stFrameInfo.stVFrame.u32Width/IVECHARCALW;
				h = stSrc.u32Height/IVECHARCALH;
			}	
			else if(stDst.u32Stride!=stSrc.u32Width)
			{
				HI_MPI_SYS_MmzFree(stDst.u32PhyAddr, pu64VirData);
				s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&stDst.u32PhyAddr,(HI_VOID *)&pu64VirData, 
					"User", HI_NULL, stSrc.u32Height * stSrc.u32Width*8);
				if(s32Ret!=HI_SUCCESS)
				{
					 printf("can't alloc intergal memory for %x\n",s32Ret);
					 return HI_NULL;
				}			
				stDst.u32Stride = stFrameInfo.stVFrame.u32Width;	
				w = stFrameInfo.stVFrame.u32Width/IVECHARCALW;
				h = stSrc.u32Height/IVECHARCALH;			
			}
			s32Ret = HI_MPI_IVE_INTEG(&hIveHandle, &stSrc, &stDst, HI_TRUE);
			if(s32Ret != HI_SUCCESS)
			{
				HI_MPI_SYS_MmzFree(stDst.u32PhyAddr, pu64VirData);
                HI_MPI_VI_ReleaseFrame(0, &stFrameInfo);
				printf(" ive integal function can't submmit for %x\n",s32Ret);
				return HI_NULL;
			}
            
			for(i=0;i<IVECHARCALW;i++)
			{
				for(j=0;j<IVECHARCALH;j++)
				{
					HI_U64 u64BlockSum,u64BlockSq;
					u64BlockSum=(pu64VirData[i*w+j*h]&0xfffffffLL)+(pu64VirData[(i+1)*w+(j+1)*h]&0xfffffffLL)
												-(pu64VirData[(i+1)*w+j*h]&0xfffffffLL)-(pu64VirData[i*w+(j+1)*h]&0xfffffffLL);//sum; low 28bits is sum
					u64BlockSq=(pu64VirData[i*w+j*h]>>28)+(pu64VirData[(i+1)*w+(j+1)*h]>>28)
												-(pu64VirData[(i+1)*w+j*h]>>28)-(pu64VirData[i*w+(j+1)*h]>>28); //square		hig 36bits is square sum					
					stChar[j*IVECHARCALW+i].s32X=u64BlockSum/(w*h);  // mean
					stChar[j*IVECHARCALW+i].s32Y=sqrt(u64BlockSum*u64BlockSum/(w*h)-2*u64BlockSum*stChar[j*IVECHARCALW+i].s32X+
												stChar[j*IVECHARCALW+i].s32X*stChar[j*IVECHARCALW+i].s32X); // sigma=sqrt(sum((x(i,j)-mean)*(x(i,j)-mena))								
				}	
			}
			s32Ret=SAMPLE_IVE_Linear2DClassifer(&stChar[0],IVECHARNUM,pstLinearPoint,s32LinearNum);

            if(s32Ret>s32ThreshNum)
			{
				printf("detect block\n");
			}		
			if(g_bStopSignal == HI_TRUE)
			{
				HI_MPI_SYS_MmzFree(stDst.u32PhyAddr, pu64VirData);
                HI_MPI_VI_ReleaseFrame(0, &stFrameInfo);
                printf(".........\n");
				break;
			}

            s32Ret=HI_MPI_SYS_MmzFlushCache(stDst.u32PhyAddr , pu64VirData , stSrc.u32Height * stSrc.u32Width*8);
			if(s32Ret!=HI_SUCCESS)
			{
				HI_MPI_SYS_MmzFree(stDst.u32PhyAddr, pu64VirData);
                HI_MPI_VI_ReleaseFrame(0, &stFrameInfo);
				printf(" ive integal function can't flush cache for %x\n",s32Ret);
				return HI_NULL;
			}

            HI_MPI_VI_ReleaseFrame(0, &stFrameInfo);
				
		}
		return HI_NULL;
}

#define SAMPLE_IVE_ExitMpp()\
do{\
    if (HI_MPI_SYS_Exit())\
    {\
        printf("sys exit fail\n");\
        return -1;\
    }\
    if (HI_MPI_VB_Exit())\
    {\
        printf("vb exit fail\n");\
        return -1;\
    }\
    return 0;\
}while(0)

#define SAMPLE_IVE_NOT_PASS(err)\
	do\
	{\
		printf("\033[0;31mtest case <%s>not pass at line:%d.\033[0;39m\n",__FUNCTION__,__LINE__);\
	}while(0)


#define SAMPLE_IVE_CHECK_RET(express,name)\
do{\
    HI_S32 s32Ret;\
    s32Ret = express;\
    if (HI_SUCCESS != s32Ret)\
    {\
        printf("%s failed at %s: LINE: %d with %#x!\n", name, __FUNCTION__, __LINE__, s32Ret);\
		SAMPLE_IVE_NOT_PASS(err);\
	    SAMPLE_IVE_ExitMpp();\
	    return HI_FAILURE;\
    }\
}while(0)


void SAMPLE_IVE_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo)
    {
        printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");

        HI_MPI_SYS_Exit();
	    HI_MPI_VB_Exit();
    }

    exit(0);
}

#if 0	
VI_DEV_ATTR_S DEV_ATTR_BT656D1_1MUX =
{
	/*interface mode*/
	VI_MODE_BT656,
	/*1\2\4 chnl mode*/
	VI_WORK_MODE_1Multiplex,
	/* r_mask	 g_mask    b_mask*/
	{0xFF000000,	0x0},
	/*prog or interlace mode input*/
	VI_SCAN_INTERLACED,
	/*AdChnId*/
	{-1, -1, -1, -1}
};
#endif 

VI_CHN_ATTR_S CHN_ATTR_720x576_422 =
/*classic chnl attr 2:720x576@xxfps format 422*/
{
    /*crop_x crop_y */  
    {0,     0, 720,   576}, 
    /*crop_w  crop_h  */
    {720,   576 },
    /*enCapSel*/
    VI_CAPSEL_BOTH,
    /* chnl format */
    PIXEL_FORMAT_YUV_SEMIPLANAR_422,
    /*bMirr  bFilp   bChromaResample*/
    0,      0,      0,
    /*s32SrcFrameRate   s32FrameRate*/
	-1, -1
};


HI_VOID SAMPLE_IVE_SetViMask(VI_DEV ViDev, VI_DEV_ATTR_S *pstDevAttr)
{
    pstDevAttr->au32CompMask[0] = 0x0;
    pstDevAttr->au32CompMask[1] = 0x0;
	switch (ViDev % 4)
	{
		case 0:
			pstDevAttr->au32CompMask[0] = 0xFF000000;
			if (VI_MODE_BT1120_STANDARD == pstDevAttr->enIntfMode)
			{
				pstDevAttr->au32CompMask[1] = 0x00FF0000;
			}
			else if (VI_MODE_BT1120_INTERLEAVED == pstDevAttr->enIntfMode)
			{
				pstDevAttr->au32CompMask[1] = 0x0;
			}
			break;
		case 1:
			pstDevAttr->au32CompMask[0] = 0xFF0000;
			if (VI_MODE_BT1120_INTERLEAVED == pstDevAttr->enIntfMode)
			{
				pstDevAttr->au32CompMask[1] = 0x0;
			}
			break;
		case 2:
			pstDevAttr->au32CompMask[0] = 0xFF00;
			if (VI_MODE_BT1120_STANDARD == pstDevAttr->enIntfMode)
			{
				pstDevAttr->au32CompMask[1] = 0xFF;
			}
			else if (VI_MODE_BT1120_INTERLEAVED == pstDevAttr->enIntfMode)
			{
				pstDevAttr->au32CompMask[1] = 0x0;
			}
            #if (VI_MST_TEST_CHIP==3531 || VI_MST_TEST_CHIP==3532)
                #ifndef HI_FPGA
                    if ((VI_MODE_BT1120_STANDARD != pstDevAttr->enIntfMode)
                        && (VI_MODE_BT1120_INTERLEAVED != pstDevAttr->enIntfMode)
                        && (VI_WORK_MODE_4Multiplex == pstDevAttr->enWorkMode || VI_WORK_MODE_2Multiplex == pstDevAttr->enWorkMode))
                    {
                        if (HI_FALSE == g_b4x960H)//16x960H还是采用原来的方案，设为自己的mask，与16D1不同
                        {
                            /* 3531的ASIC板是两个BT1120口出16D1，此时dev2/6要设成dev1/5的MASK */
                            pstDevAttr->au32CompMask[0] = 0xFF0000; 
                        }
                    }
                #endif
            #endif
			break;
		case 3:
			pstDevAttr->au32CompMask[0] = 0xFF;
			if (VI_MODE_BT1120_INTERLEAVED == pstDevAttr->enIntfMode)
			{
				pstDevAttr->au32CompMask[1] = 0x0;
			}
			break;
		default:
			HI_ASSERT(0);
	}
}


HI_S32 SAMPLE_IVE_StartViDevAndWay(HI_S32 u32StartDevNum, HI_U32 u32FistDev, const VI_DEV_ATTR_S *pstDevAttr)
{
    VI_DEV_ATTR_S stDevAttr;
    int i = 0;
 
    for (i = u32FistDev; i < VIU_MAX_DEV_NUM; i+=VIU_MAX_DEV_NUM/u32StartDevNum)
    {
    	memcpy(&stDevAttr, pstDevAttr, sizeof(VI_DEV_ATTR_S));
    	SAMPLE_IVE_SetViMask(i, &stDevAttr);
        SAMPLE_IVE_CHECK_RET(HI_MPI_VI_SetDevAttr(i, &stDevAttr),"SetDevAttr");
        SAMPLE_IVE_CHECK_RET(HI_MPI_VI_EnableDev(i),"Enable Dev");
    }
    
    return HI_SUCCESS;
}

HI_S32 main(int argc, char *argv[])
{
	pthread_t hIveThread;
	IVE_LINEAR_DATA_S stIveLinerData;
	
	signal(SIGINT, SAMPLE_IVE_HandleSig);
    signal(SIGTERM, SAMPLE_IVE_HandleSig);
    
	//SAMPLE_IVE_MST_INIT_MPI();    
	g_bStopSignal = HI_FALSE;
    
	stIveLinerData.pstLinearPoint = malloc(sizeof(POINT_S)*10);
	stIveLinerData.s32LinearNum = 2;
	stIveLinerData.s32ThreshNum = IVECHARNUM/2;
	stIveLinerData.pstLinearPoint[0].s32X = 40;
	stIveLinerData.pstLinearPoint[0].s32Y = 0;
	stIveLinerData.pstLinearPoint[1].s32X = 40;
	stIveLinerData.pstLinearPoint[1].s32Y = 256;

#if 0
    SAMPLE_IVE_CHECK_RET(SAMPLE_IVE_StartViDevAndWay(1,0,&DEV_ATTR_BT656D1_1MUX), "startDev");	
	SAMPLE_IVE_CHECK_RET(HI_MPI_VI_SetChnAttr(0, &CHN_ATTR_720x576_422),"setChnAttr");	
	SAMPLE_IVE_CHECK_RET(HI_MPI_VI_EnableChn(0),"EnableChn");
#endif
    
    SAMPLE_VI_1_D1();
    
	pthread_create(&hIveThread, 0, SAMPLE_IVE_BlockDetect, (HI_VOID *)&stIveLinerData);

    
	printf("press 'e' to exit\n");
	while(1)
	{
		char c;
		c=getchar();

		if(c=='e')            
			break;
	}
        
    g_bStopSignal = HI_TRUE;
	pthread_join(hIveThread,HI_NULL);
	free(stIveLinerData.pstLinearPoint);
    
	HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();
	return 0;
}



