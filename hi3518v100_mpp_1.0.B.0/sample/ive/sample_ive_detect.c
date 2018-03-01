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

SAMPLE_VI_CONFIG_S g_stViChnConfig = 
{
    APTINA_AR0130_DC_720P_30FPS,
    VIDEO_ENCODING_MODE_AUTO,    
    ROTATE_NONE,
    VI_CHN_SET_NORMAL
};

HI_S32 SAMPLE_IVE_VI(HI_VOID)
{
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
                PIC_HD720, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
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
    s32Ret = SAMPLE_COMM_VI_StartVi(&g_stViChnConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END;
    }

    
END: 
    return s32Ret;

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
			   || ( (pstChar[i].s32X<pstLinearPoint[j].s32X) && (pstNextLinearPoint[j].s32Y==pstLinearPoint[j].s32Y) )
			   || ((pstChar[i].s32X==pstLinearPoint[j].s32X) && (pstNextLinearPoint[j].s32Y<pstLinearPoint[j].s32Y)))	
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

HI_S32 SAMPLE_IVE_YUVSP2P (VIDEO_FRAME_S * pInVBuf, HI_U8 *pOutBuf)
{
    unsigned int w, h, c;
    char * pVBufVirt_Y;
    char * pVBufVirt_C;
    char * pMemContent;
    HI_U32 phy_addr,Ysize,Csize;
    PIXEL_FORMAT_E  enPixelFormat = pInVBuf->enPixelFormat;
    HI_U32 u32UvHeight;/* 存为planar 格式时的UV分量的高度 */

    Ysize = (pInVBuf->u32Stride[0])*(pInVBuf->u32Height);
    if (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == enPixelFormat)
    {
        Csize = (pInVBuf->u32Stride[1])*(pInVBuf->u32Height)/2;    
        u32UvHeight = pInVBuf->u32Height/2;
    }
    else
    {
        Csize = (pInVBuf->u32Stride[1])*(pInVBuf->u32Height);   
        u32UvHeight = pInVBuf->u32Height;
    }

    phy_addr = pInVBuf->u32PhyAddr[0];

    //printf("phy_addr:%x, size:%d\n", phy_addr, size);
    pVBufVirt_Y = (HI_CHAR *) HI_MPI_SYS_Mmap(phy_addr, Ysize);	
    if (NULL == pVBufVirt_Y)
    {
        return HI_FAILURE;
    }

    pVBufVirt_C = (HI_CHAR *) HI_MPI_SYS_Mmap(pInVBuf->u32PhyAddr[1], Csize);
    if (NULL == pVBufVirt_C)
    {
        HI_MPI_SYS_Munmap(pVBufVirt_Y, Ysize); 
        return HI_FAILURE;
    }

    /*Copy Y*/
    memcpy(pOutBuf, pVBufVirt_Y, Ysize);

    c = Ysize;
    /* Copy U ----------------------------------------------------------------*/
    for( h=0; h<u32UvHeight; h++)
    {
        pMemContent = pVBufVirt_C + h*pInVBuf->u32Stride[1];

        pMemContent += 1;

        for(w=0; w<pInVBuf->u32Width/2; w++)
        {
            pOutBuf[c] = *pMemContent;
            c ++;
            pMemContent += 2;
        }
    }

    /* Copy V ----------------------------------------------------------------*/
    for(h=0; h<u32UvHeight; h++)    
    {
        pMemContent = pVBufVirt_C + h*pInVBuf->u32Stride[1];

        for(w=0; w<pInVBuf->u32Width/2; w++)
        {
            pOutBuf[c] = *pMemContent;
            c ++;
            pMemContent += 2;
        }
    }
        
    HI_MPI_SYS_Munmap(pVBufVirt_Y, Ysize);  
    HI_MPI_SYS_Munmap(pVBufVirt_C, Csize);

    return HI_SUCCESS;
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
        FILE *fp;
        HI_VOID *pYUVPBuf;
        int size;
            
        s32Ret = HI_MPI_VI_SetFrameDepth(0, u32Depth);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("set max depth err:0x%x\n", s32Ret);
            return HI_NULL;
        }
            
	    s32Ret = HI_MPI_VI_GetFrameTimeOut(0, &stFrameInfo, 0);
            
		if(s32Ret!=HI_SUCCESS)
		{
			SAMPLE_PRT("can't get vi frame for %x\n",s32Ret);
		    return HI_NULL;
		}                    

        stSrc.u32Width = stFrameInfo.stVFrame.u32Width;
	    stSrc.u32Height = stFrameInfo.stVFrame.u32Height;
		stSrc.stSrcMem.u32PhyAddr = stFrameInfo.stVFrame.u32PhyAddr[0];
		stSrc.stSrcMem.u32Stride = stFrameInfo.stVFrame.u32Stride[0];
		stSrc.enSrcFmt = IVE_SRC_FMT_SINGLE;

        size = stSrc.u32Width * stSrc.u32Height;
        size *= (SAMPLE_PIXEL_FORMAT == PIXEL_FORMAT_YUV_SEMIPLANAR_420)? 3:4;
        size /= 2;
            
        //printf("frame size = (widthy)%d x %d(heitht) = %d\n", stSrc.u32Width, stSrc.u32Height, size);

        pYUVPBuf = malloc(size);
        SAMPLE_IVE_YUVSP2P(&stFrameInfo.stVFrame, (HI_U8 *)pYUVPBuf);
            
        fp = fopen("frame1280x720.yuv","wb");
        if(fwrite(pYUVPBuf,1 , size, fp) != size)
        {
            SAMPLE_PRT("fwrite %d bytes error!\n", size);
            return HI_NULL;
        }
        fclose(fp);
        free(pYUVPBuf);
            
	    if(stDst.u32PhyAddr==0)
		{
			s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&stDst.u32PhyAddr, (HI_VOID *)&pu64VirData, 
				    "User", HI_NULL, stSrc.u32Height * stSrc.u32Width*8);
			if(s32Ret!=HI_SUCCESS)
			{
				SAMPLE_PRT("can't alloc intergal memory for %x\n",s32Ret);
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
				SAMPLE_PRT("can't alloc intergal memory for %x\n",s32Ret);
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
			SAMPLE_PRT(" ive integal function can't submmit for %x\n",s32Ret);
			return HI_NULL;
		}
            
		for(i=0;i<IVECHARCALW;i++)
		{
			for(j=0;j<IVECHARCALH;j++)
			{
				HI_U64 u64BlockSum,u64BlockSq;
				u64BlockSum = (pu64VirData[i*w+j*h]&0xfffffffLL)+(pu64VirData[(i+1)*w+(j+1)*h]&0xfffffffLL)
						    - (pu64VirData[(i+1)*w+j*h]&0xfffffffLL)-(pu64VirData[i*w+(j+1)*h]&0xfffffffLL);//sum; low 28bits is sum
				u64BlockSq  = (pu64VirData[i*w+j*h]>>28)+(pu64VirData[(i+1)*w+(j+1)*h]>>28)
							- (pu64VirData[(i+1)*w+j*h]>>28)-(pu64VirData[i*w+(j+1)*h]>>28); //square		hig 36bits is square sum					
				stChar[j*IVECHARCALW+i].s32X = u64BlockSum/(w*h);  // mean
				stChar[j*IVECHARCALW+i].s32Y = sqrt(u64BlockSum*u64BlockSum/(w*h)-2*u64BlockSum*stChar[j*IVECHARCALW+i].s32X
                                             + stChar[j*IVECHARCALW+i].s32X*stChar[j*IVECHARCALW+i].s32X); // sigma=sqrt(sum((x(i,j)-mean)*(x(i,j)-mena))								
			}	
		}
		s32Ret = SAMPLE_IVE_Linear2DClassifer(&stChar[0],IVECHARNUM,pstLinearPoint,s32LinearNum);

        printf("s32Ret = %d, s32ThreshNum = %d\n", s32Ret, s32ThreshNum);
            
        if(s32Ret>s32ThreshNum)
		{
			printf("detect block\n");
		}		
		if(g_bStopSignal == HI_TRUE)
		{
			HI_MPI_SYS_MmzFree(stDst.u32PhyAddr, pu64VirData);
            HI_MPI_VI_ReleaseFrame(0, &stFrameInfo);
            printf(".....StopSigal.....\n");
			break;
		}

        s32Ret=HI_MPI_SYS_MmzFlushCache(stDst.u32PhyAddr , pu64VirData , stSrc.u32Height * stSrc.u32Width*8);
		if(s32Ret!=HI_SUCCESS)
		{
		    HI_MPI_SYS_MmzFree(stDst.u32PhyAddr, pu64VirData);
            HI_MPI_VI_ReleaseFrame(0, &stFrameInfo);
			SAMPLE_PRT(" ive integal function can't flush cache for %x\n",s32Ret);
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
    HI_S32 s32Ret = HI_SUCCESS;
	pthread_t hIveThread;
    POINT_S astPoints[10] = {{0,0}};
	IVE_LINEAR_DATA_S stIveLinerData;
	
	signal(SIGINT, SAMPLE_IVE_HandleSig);
    signal(SIGTERM, SAMPLE_IVE_HandleSig);
    
	//SAMPLE_IVE_MST_INIT_MPI();    
	g_bStopSignal = HI_FALSE;
    
	stIveLinerData.pstLinearPoint = &astPoints[0];
	stIveLinerData.s32LinearNum = 2;
	stIveLinerData.s32ThreshNum = IVECHARNUM/2;
	stIveLinerData.pstLinearPoint[0].s32X = 40;
	stIveLinerData.pstLinearPoint[0].s32Y = 0;
	stIveLinerData.pstLinearPoint[1].s32X = 40;
	stIveLinerData.pstLinearPoint[1].s32Y = 256;
    
    s32Ret = SAMPLE_IVE_VI();
    if (s32Ret != HI_SUCCESS)
    {
        goto END_0;
    }
    
	pthread_create(&hIveThread, 0, SAMPLE_IVE_BlockDetect, (HI_VOID *)&stIveLinerData);
    
	printf("press 'q' to exit!\n"); 
	while(1)
	{
		char c;
		c=getchar();

		if(c=='q')            
		{
            break;
		}
        else
        {
            printf("press 'q' to exit!\n"); 
        }
	}
        
    g_bStopSignal = HI_TRUE;
	pthread_join(hIveThread,HI_NULL);

    SAMPLE_COMM_VI_StopVi(&g_stViChnConfig);
END_0:
    SAMPLE_COMM_SYS_Exit();
    if (HI_SUCCESS == s32Ret)
        printf("program exit normally!\n");
    else
        printf("program exit abnormally!\n");

	exit(s32Ret);
}



