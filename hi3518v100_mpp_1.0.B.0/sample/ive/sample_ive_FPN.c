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
HI_U32 gs_u32ViFrmRate = 30; 

SAMPLE_VI_CONFIG_S g_stViChnConfig = 
{
    APTINA_AR0130_DC_720P_30FPS,
    VIDEO_ENCODING_MODE_AUTO,    
    ROTATE_NONE,
    VI_CHN_SET_NORMAL
};

HI_BOOL g_bStopSignal;
HI_BOOL g_bFpnPrintSerial = HI_FALSE;

typedef struct hiIVE_FPN_DATA_S
{
    HI_BOOL bFpnProcess;
    HI_BOOL bUpdateFpnData;

    float afFpnAGain[3];
    float afFpnDGain[3];
    float afFpnISPDGainMax[3];

    HI_BOOL abFpnFrameValid[3];
    VIDEO_FRAME_INFO_S astFpnFrame[3];
    VB_BLK aFpnVbBlk[3];
}IVE_FPN_DATA_S;

HI_S32 SAMPLE_IVE_GetFpnProcessRange(float fFpnGainMult, float *pfDoFpnMin, float *pfDoFpnMax)
{
    if ((NULL == pfDoFpnMin) || (NULL == pfDoFpnMax))
    {
        SAMPLE_PRT("NULL ptr\n");
        return HI_FAILURE;
    }    
    /* you can change the range */
    if (48 == fFpnGainMult)
    {
        *pfDoFpnMin = 36;
        *pfDoFpnMax = 48;

    }
    else if (64 == fFpnGainMult)
    {
        *pfDoFpnMin = 56;
        *pfDoFpnMax = 128;
    }
    else if (256 == fFpnGainMult)
    {
        *pfDoFpnMin = 128;
        *pfDoFpnMax = 256;
    }
    else
    {
        SAMPLE_PRT("invale input %.2f\n", fFpnGainMult);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_VOID * SAMPLE_IVE_FpnProcess(HI_VOID *pArgs)
{
    HI_S32 i, index = 0, s32Ret = HI_SUCCESS;
    VI_CHN ViChn = 0;
    HI_U32 u32Depth = 7, u32Count = 0;
    IVE_FPN_DATA_S *pstFpnData = NULL;
    VIDEO_FRAME_INFO_S stViFrame;
    HI_U32 u32MilliSec = 5000;//5second

    IVE_HANDLE IveHandle;
    IVE_SRC_INFO_S stIveSrc1, astIveFpn[3], *pstIveSrc2 = NULL;
    IVE_MEM_INFO_S stIveDst;
    HI_BOOL bInstant = HI_TRUE, bFinished = HI_FALSE, bDoIveFpn = HI_FALSE;

    VO_DEV VoDev = SAMPLE_VO_DEV_DSD0;
    VO_CHN VoChn = 0;
    ISP_INNER_STATE_INFO_EX_S stInnerStateInfoEx;
    float fCurAGain, fCurDGain, fCurISPDGain;
    float afDoFpnMin[3] = {0}, afDoFpnMax[3] = {0};
    float afFpnGainMult[3] = {0}, fCurGainMult = 0;
    char acString[128] = "";
	
	pstFpnData = (IVE_FPN_DATA_S * )pArgs;

    s32Ret = HI_MPI_VI_SetFrameDepth(ViChn, u32Depth);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VI_SetFrameDepth fail, err code 0x%x\n",s32Ret);
        //return NULL;
    }

	while(!g_bStopSignal)
	{
        if (HI_TRUE == pstFpnData->bUpdateFpnData)
        {
            for (i = 0; i < 3; i++)
            {
                afFpnGainMult[i] = pstFpnData->afFpnAGain[i] * pstFpnData->afFpnDGain[i] * pstFpnData->afFpnISPDGainMax[i];
                if (HI_FALSE == pstFpnData->abFpnFrameValid[i])
                {
                    astIveFpn[i].stSrcMem.u32PhyAddr = 0;
                    astIveFpn[i].stSrcMem.u32Stride = 0;
                    astIveFpn[i].u32Width = 0;
                    astIveFpn[i].u32Height = 0;
                    afDoFpnMin[i] = 0;
                    afDoFpnMax[i] = 0;
                }
                else
                {        
                    astIveFpn[i].enSrcFmt = IVE_SRC_FMT_SINGLE;
                    astIveFpn[i].stSrcMem.u32PhyAddr = pstFpnData->astFpnFrame[i].stVFrame.u32PhyAddr[0];
                    astIveFpn[i].stSrcMem.u32Stride = pstFpnData->astFpnFrame[i].stVFrame.u32Stride[0];
                    astIveFpn[i].u32Width = pstFpnData->astFpnFrame[i].stVFrame.u32Width;
                    astIveFpn[i].u32Height = pstFpnData->astFpnFrame[i].stVFrame.u32Height;
                    s32Ret = SAMPLE_IVE_GetFpnProcessRange(afFpnGainMult[i], &afDoFpnMin[i], &afDoFpnMax[i]);
                    if (s32Ret != HI_SUCCESS)
                    {
                        SAMPLE_PRT("SAMPLE_IVE_GetFpnProcessRange fail, err code 0x%x\n",s32Ret);
                    }
                }
            }
            pstFpnData->bUpdateFpnData = HI_FALSE;
            printf("FPN data update ok\n");
        }
        
        s32Ret = HI_MPI_VI_GetFrameTimeOut(ViChn, &stViFrame, u32MilliSec);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VI_GetFrameTimeOut fail, err code 0x%x\n",s32Ret);
            continue;
        }  
        
        s32Ret = HI_MPI_ISP_QueryInnerStateInfoEx(&stInnerStateInfoEx);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_ISP_GetAEAttr fail, err code 0x%x\n", s32Ret);
            continue;
        }
        sprintf(acString, "%d.%d", stInnerStateInfoEx.u32AnalogGain>>10, stInnerStateInfoEx.u32AnalogGain&0x3FF);
        fCurAGain = atof(acString);
        sprintf(acString, "%d.%d", stInnerStateInfoEx.u32DigitalGain>>10, stInnerStateInfoEx.u32DigitalGain&0x3FF);
        fCurDGain = atof(acString);
        sprintf(acString, "%d.%d", stInnerStateInfoEx.u32ISPDigitalGain>>10, stInnerStateInfoEx.u32ISPDigitalGain&0x3FF);
        fCurISPDGain = atof(acString);
        fCurGainMult = fCurAGain * fCurDGain * fCurISPDGain;

        if (HI_TRUE == pstFpnData->bFpnProcess)
        {
            /* only the multiply value of the three type gains matchs with the FPN frame, we do IVE sub processing */
            for (i = 0; i < 3; i++)
            {
                index = i;
                if ((fCurGainMult > afDoFpnMin[i]) && (fCurGainMult <= afDoFpnMax[i]))
                {
                    pstIveSrc2 = &astIveFpn[i];
                    bDoIveFpn = HI_TRUE;
                    break;
                }
            }
            if (i >= 3) bDoIveFpn = HI_FALSE;
            
            if (HI_TRUE == bDoIveFpn)
            {
                stIveDst.u32PhyAddr = stViFrame.stVFrame.u32PhyAddr[0];
                stIveDst.u32Stride = stViFrame.stVFrame.u32Stride[0];

                stIveSrc1.enSrcFmt = IVE_SRC_FMT_SINGLE;
                stIveSrc1.stSrcMem.u32PhyAddr = stViFrame.stVFrame.u32PhyAddr[0];
                stIveSrc1.stSrcMem.u32Stride = stViFrame.stVFrame.u32Stride[0];
                stIveSrc1.u32Width = stViFrame.stVFrame.u32Width;
                stIveSrc1.u32Height = stViFrame.stVFrame.u32Height;            

                s32Ret = HI_MPI_IVE_SUB(&IveHandle, &stIveSrc1, pstIveSrc2, &stIveDst, IVE_SUB_OUT_FMT_ABS, bInstant);
                if (s32Ret != HI_SUCCESS)
                {
                    SAMPLE_PRT("HI_MPI_IVE_SUB fail, err code 0x%x\n", s32Ret);
                    HI_MPI_VI_ReleaseFrame(ViChn, &stViFrame);
                    continue;
                }

                /* query ive: whether the sub process was finished */
                HI_MPI_IVE_Query(IveHandle, &bFinished, HI_TRUE);                
                if (bFinished != HI_TRUE)
                {
                    SAMPLE_PRT("ive sub process doesn't finish.\n");
                }            

                if ((HI_TRUE == g_bFpnPrintSerial) && (0 == u32Count%60))
                {
                    printf("ISP current GainMult=%.2f,AGain=%.2f,DGain=%.2f,ISPDGain=%.2f; Do FPN range[%.2f,%.2f] frame AGain=%.2f, DGain=%.2f, ISPDG=%.2f.\n",
                        fCurGainMult, fCurAGain, fCurDGain, fCurISPDGain, afDoFpnMin[index], afDoFpnMax[index], 
                        pstFpnData->afFpnAGain[index], pstFpnData->afFpnDGain[index], pstFpnData->afFpnISPDGainMax[index]);
                    printf("VI frame was processed by IVE.\n");
                }
            }
            else
            {               
                if ((HI_TRUE == g_bFpnPrintSerial) && (0 == u32Count%60))
                {
                    printf("ISP current GainMult=%.2f,AGain=%.2f,DGain=%.2f,ISPDGain=%.2f.\n",
                        fCurGainMult, fCurAGain, fCurDGain, fCurISPDGain);
                    printf("VI frame wasn't processed by IVE.\n");
                }
            }
        }
        else
        {
            if ((HI_TRUE == g_bFpnPrintSerial) && (0 == u32Count%60))
            {
                printf("ISP current GainMult=%.2f,AGain=%.2f,DGain=%.2f,ISPDGain=%.2f.\n",
                    fCurGainMult, fCurAGain, fCurDGain, fCurISPDGain);
                printf("VI frame wasn't processed by IVE.\n");
            }
        }

        s32Ret = HI_MPI_VO_SendFrameTimeOut(VoDev, VoChn, &stViFrame, u32MilliSec);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VO_SendFrameTimeOut fail, err code 0x%x\n", s32Ret);
        }
            
        s32Ret = HI_MPI_VI_ReleaseFrame(ViChn, &stViFrame);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VI_ReleaseFrame fail, err code 0x%x\n",s32Ret);
        }
                
        usleep(10000);
        u32Count++;
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

void SAMPLE_IVE_YUV_Dump(VIDEO_FRAME_S * pVBuf, FILE *pfd)
{
    unsigned int w, h;
    char * pVBufVirt_Y;
    char * pVBufVirt_C;
    char * pMemContent;
    unsigned char TmpBuff[2000];                //如果这个值太小，图像很大的话存不了
    HI_U32 phy_addr,size;
	HI_CHAR *pUserPageAddr[2];
    PIXEL_FORMAT_E  enPixelFormat = pVBuf->enPixelFormat;
    HI_U32 u32UvHeight;/* 存为planar 格式时的UV分量的高度 */
    
    if (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == enPixelFormat)
    {
        size = (pVBuf->u32Stride[0])*(pVBuf->u32Height)*3/2;    
        u32UvHeight = pVBuf->u32Height/2;
    }
    else
    {
        size = (pVBuf->u32Stride[0])*(pVBuf->u32Height)*2;   
        u32UvHeight = pVBuf->u32Height;
    }

    phy_addr = pVBuf->u32PhyAddr[0];

    //printf("phy_addr:%x, size:%d\n", phy_addr, size);
    pUserPageAddr[0] = (HI_CHAR *) HI_MPI_SYS_Mmap(phy_addr, size);	
    if (NULL == pUserPageAddr[0])
    {
        return;
    }
    //printf("stride: %d,%d\n",pVBuf->u32Stride[0],pVBuf->u32Stride[1] );
    
	pVBufVirt_Y = pUserPageAddr[0]; 
	pVBufVirt_C = pVBufVirt_Y + (pVBuf->u32Stride[0])*(pVBuf->u32Height);

    /* save Y ----------------------------------------------------------------*/
    fprintf(stderr, "saving......Y......");
    fflush(stderr);
    for(h=0; h<pVBuf->u32Height; h++)
    {
        pMemContent = pVBufVirt_Y + h*pVBuf->u32Stride[0];
        fwrite(pMemContent, pVBuf->u32Width, 1, pfd);
    }
    fflush(pfd);
    

    /* save U ----------------------------------------------------------------*/
    fprintf(stderr, "U......");
    fflush(stderr);
    for(h=0; h<u32UvHeight; h++)
    {
        pMemContent = pVBufVirt_C + h*pVBuf->u32Stride[1];

        pMemContent += 1;

        for(w=0; w<pVBuf->u32Width/2; w++)
        {
            TmpBuff[w] = *pMemContent;
            pMemContent += 2;
        }
        fwrite(TmpBuff, pVBuf->u32Width/2, 1, pfd);
    }
    fflush(pfd);

    /* save V ----------------------------------------------------------------*/
    fprintf(stderr, "V......");
    fflush(stderr);
    for(h=0; h<u32UvHeight; h++)    
    {
        pMemContent = pVBufVirt_C + h*pVBuf->u32Stride[1];

        for(w=0; w<pVBuf->u32Width/2; w++)
        {
            TmpBuff[w] = *pMemContent;
            pMemContent += 2;
        }
        fwrite(TmpBuff, pVBuf->u32Width/2, 1, pfd);
    }
    fflush(pfd);
    
    fprintf(stderr, "done %d!\n", pVBuf->u32TimeRef);
    fflush(stderr);
    
    HI_MPI_SYS_Munmap(pUserPageAddr[0], size);    
}

HI_VOID SAMPLE_IVE_ReadOneFrame( FILE * fp, HI_U8 * pY, HI_U8 * pU, HI_U8 * pV,
                                              HI_U32 width, HI_U32 height, HI_U32 stride, HI_U32 stride2)
{
    HI_U8 * pDst;

    HI_U32 u32Row;

    pDst = pY;
    for ( u32Row = 0; u32Row < height; u32Row++ )
    {
        fread( pDst, width, 1, fp );
        pDst += stride;
    }
    
    pDst = pU;
    for ( u32Row = 0; u32Row < height/2; u32Row++ )
    {
        fread( pDst, width/2, 1, fp );
        pDst += stride2;
    }
    
    pDst = pV;
    for ( u32Row = 0; u32Row < height/2; u32Row++ )
    {
        fread( pDst, width/2, 1, fp );
        pDst += stride2;
    }
}

HI_S32 SAMPLE_IVE_PlanToSemi(HI_U8 *pY, HI_S32 yStride, 
                       HI_U8 *pU, HI_S32 uStride,
                       HI_U8 *pV, HI_S32 vStride, 
                       HI_S32 picWidth, HI_S32 picHeight)
{
    HI_S32 i;
    HI_U8* pTmpU, *ptu;
    HI_U8* pTmpV, *ptv;
    HI_S32 s32HafW = uStride >>1 ;
    HI_S32 s32HafH = picHeight >>1 ;
    HI_S32 s32Size = s32HafW*s32HafH;
        
    pTmpU = malloc( s32Size ); ptu = pTmpU;
    pTmpV = malloc( s32Size ); ptv = pTmpV;
    
    memcpy(pTmpU,pU,s32Size);
    memcpy(pTmpV,pV,s32Size);
    
    for(i = 0;i<s32Size>>1;i++)
    {
        *pU++ = *pTmpV++;
        *pU++ = *pTmpU++;
        
    }
    for(i = 0;i<s32Size>>1;i++)
    {
        *pV++ = *pTmpV++;
        *pV++ = *pTmpU++;        
    }

    free( ptu );
    free( ptv );

    return HI_SUCCESS;
}

HI_S32 SAMPLE_IVE_GetVFrame_FromYUV(FILE *pYUVFile, 
    HI_U32 u32Width, HI_U32 u32Height,HI_U32 u32Stride, VIDEO_FRAME_INFO_S *pstVFrameInfo, VB_BLK *pVbBlk)
{
    HI_U32             u32LStride;
    HI_U32             u32CStride;
    HI_U32             u32LumaSize;
    HI_U32             u32ChrmSize;
    HI_U32             u32Size;
    VB_BLK VbBlk;
    HI_U32 u32PhyAddr;
    HI_U8 *pVirAddr;

    u32LStride  = u32Stride;
    u32CStride  = u32Stride;
    
    u32LumaSize = (u32LStride * u32Height);    

    pstVFrameInfo->stVFrame.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    u32ChrmSize = (u32CStride * u32Height) >> 2;/* YUV 420 */

    u32Size = u32LumaSize + (u32ChrmSize << 1);

    /* alloc video buffer block ---------------------------------------------------------- */
    VbBlk = HI_MPI_VB_GetBlock(VB_INVALID_POOLID, u32Size, HI_NULL);
    if (VB_INVALID_HANDLE == VbBlk)
    {
        SAMPLE_PRT("HI_MPI_VB_GetBlock fail\n");
        return HI_FAILURE;
    }
    u32PhyAddr = HI_MPI_VB_Handle2PhysAddr(VbBlk);
    if (0 == u32PhyAddr)
    {
        SAMPLE_PRT("HI_MPI_VB_Handle2PhysAddr fail\n");
        return HI_FAILURE;
    }
    pVirAddr = (HI_U8 *) HI_MPI_SYS_Mmap(u32PhyAddr, u32Size);
    if (NULL == pVirAddr)
    {
        SAMPLE_PRT("HI_MPI_SYS_Mmap fail\n");
        return HI_FAILURE;
    }

    pstVFrameInfo->u32PoolId = HI_MPI_VB_Handle2PoolId(VbBlk);
    if (VB_INVALID_POOLID == pstVFrameInfo->u32PoolId)
    {
        SAMPLE_PRT("HI_MPI_VB_Handle2PoolId fail\n");
        return HI_FAILURE;
    }
    
    pstVFrameInfo->stVFrame.u32PhyAddr[0] = u32PhyAddr;
    pstVFrameInfo->stVFrame.u32PhyAddr[1] = pstVFrameInfo->stVFrame.u32PhyAddr[0] + u32LumaSize;
    pstVFrameInfo->stVFrame.u32PhyAddr[2] = pstVFrameInfo->stVFrame.u32PhyAddr[1] + u32ChrmSize;

    pstVFrameInfo->stVFrame.pVirAddr[0] = pVirAddr;
    pstVFrameInfo->stVFrame.pVirAddr[1] = pstVFrameInfo->stVFrame.pVirAddr[0] + u32LumaSize;
    pstVFrameInfo->stVFrame.pVirAddr[2] = pstVFrameInfo->stVFrame.pVirAddr[1] + u32ChrmSize;

    pstVFrameInfo->stVFrame.u32Width  = u32Width;
    pstVFrameInfo->stVFrame.u32Height = u32Height;
    pstVFrameInfo->stVFrame.u32Stride[0] = u32LStride;
    pstVFrameInfo->stVFrame.u32Stride[1] = u32CStride;
    pstVFrameInfo->stVFrame.u32Stride[2] = u32CStride;  
    pstVFrameInfo->stVFrame.u32Field = VIDEO_FIELD_FRAME;  

    /* read Y U V data from file to the addr (注意只支持planar 420) -----------------------*/
    SAMPLE_IVE_ReadOneFrame(pYUVFile, pstVFrameInfo->stVFrame.pVirAddr[0], 
                               pstVFrameInfo->stVFrame.pVirAddr[1], pstVFrameInfo->stVFrame.pVirAddr[2],
                               pstVFrameInfo->stVFrame.u32Width, pstVFrameInfo->stVFrame.u32Height, 
                               pstVFrameInfo->stVFrame.u32Stride[0], pstVFrameInfo->stVFrame.u32Stride[1] >> 1 );

    /* convert planar YUV420 to sem-planar YUV420 -----------------------------------------*/
    SAMPLE_IVE_PlanToSemi(pstVFrameInfo->stVFrame.pVirAddr[0], pstVFrameInfo->stVFrame.u32Stride[0],
                pstVFrameInfo->stVFrame.pVirAddr[1], pstVFrameInfo->stVFrame.u32Stride[1],
                pstVFrameInfo->stVFrame.pVirAddr[2], pstVFrameInfo->stVFrame.u32Stride[1],
                pstVFrameInfo->stVFrame.u32Width, pstVFrameInfo->stVFrame.u32Height);
    
    HI_MPI_SYS_Munmap(pVirAddr, u32Size);

    *pVbBlk = VbBlk;
    return 0;
}

HI_S32 SAMPLE_IVE_GenerateFpnFrame(HI_VOID)
{
    HI_S32 i, s32Ret = HI_SUCCESS;
    VI_CHN ViChn = 0;
    HI_U32 u32Depth = 7;
    VIDEO_FRAME_INFO_S stFrameInfo;
    HI_U32 u32MilliSec = 5000;//5s
    HI_CHAR szYuvName[128] = "SrcFpnFrame_1280x720.yuv";
    HI_CHAR szFpnYuvName[128] = "./fpn_1280x720.yuv";
    ISP_AE_ATTR_EX_S stAEAttrEx, stAEAttrExOrg;
    char cmd;
    FILE *pfd;

    printf("Step1:Please close aperture(or cover the camera lens). If you have done this, please press any key to execute step2.\n");
    getchar();
    s32Ret = HI_MPI_ISP_GetAEAttrEx(&stAEAttrExOrg);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_ISP_GetAEAttrEx fail, err code 0x%x\n", s32Ret);
        return s32Ret;
    }
    
    memcpy(&stAEAttrEx, &stAEAttrExOrg, sizeof(stAEAttrExOrg));
    
    while(1)
    {
        printf("Please select gains value to set.\n");
        printf("\t 0) AGainMax = 8, DGainMax = 8, ISPDGain = 1.\n");
        printf("\t 1) AGainMax = 8, DGainMax = 8, ISPDGain = 4.\n");
        printf("\t 2) AGainMax = 8, DGainMax = 6, ISPDGain = 1.\n");
        printf("\t r) return to last level.\n");
        cmd = getchar();
        getchar();

        if (cmd == '0')
        {
            stAEAttrEx.u32AGainMax = 8 << 10;
            stAEAttrEx.u32DGainMax = 8 << 10;            
            stAEAttrEx.u32ISPDGainMax = 1 << 10;
            break;
        }
        else if (cmd == '1')
        {
            stAEAttrEx.u32AGainMax = 8 << 10;
            stAEAttrEx.u32DGainMax = 8 << 10;            
            stAEAttrEx.u32ISPDGainMax = 4 << 10;
            break;
        }
        else if (cmd == '2')
        {
            stAEAttrEx.u32AGainMax = 8 << 10;
            stAEAttrEx.u32DGainMax = 6 << 10;            
            stAEAttrEx.u32ISPDGainMax = 1 << 10;
            break;
        }
        else if (cmd == 'r')
        {
            printf("return to last level ok!\n");
            return HI_SUCCESS;
        }
        else
        {
            printf("invalid index\n");
            continue;
        }
    }
    
    s32Ret = HI_MPI_ISP_SetAEAttrEx(&stAEAttrEx);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_ISP_SetAEAttrEx fail, err code 0x%x\n", s32Ret);
        return s32Ret;
    }
    
    printf("Set isp max gain ok. AGain=%d, DGain=%d, ISPDG=%d\n", stAEAttrEx.u32AGainMax>>10, stAEAttrEx.u32DGainMax>>10, stAEAttrEx.u32ISPDGainMax>>10);
    sprintf(szYuvName, "./SrcFpnFrame_1280x720_AG%d_DG%d_ISPDG%d.yuv", stAEAttrEx.u32AGainMax>>10, stAEAttrEx.u32DGainMax>>10, stAEAttrEx.u32ISPDGainMax>>10);
    sprintf(szFpnYuvName, "./fpn_1280x720_AG%d_DG%d_ISPDG%d.yuv", stAEAttrEx.u32AGainMax>>10, stAEAttrEx.u32DGainMax>>10, stAEAttrEx.u32ISPDGainMax>>10);

    /* after set the gains, we should wait some seconds. then dump VI frames */
    #if 0
    sleep(10);
    #else
    printf("\nAfter set the gains, we should wait several seconds to make sure them have taken effect, then press any key to dump VI frames.\n");
    getchar();
    #endif

    /* open file */
    pfd = fopen(szYuvName, "wb");    
    if (NULL == pfd)
    {
        return -1;
    }
    
    s32Ret = HI_MPI_VI_SetFrameDepth(ViChn, u32Depth);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VI_SetFrameDepth fail, err code 0x%x\n", s32Ret);
        return s32Ret;
    }
    
    /* get 128 VI frames, and use them to generate FPN frame  */
    for (i = 0; i < 128; i++)
    {
        s32Ret = HI_MPI_VI_GetFrameTimeOut(ViChn, &stFrameInfo, u32MilliSec);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VI_GetFrameTimeOut fail, err code 0x%x\n", s32Ret);
            continue;
        }                
        
        /* save VI frame to file */
        SAMPLE_IVE_YUV_Dump(&stFrameInfo.stVFrame, pfd);
        
        /* release frame after using */
        s32Ret = HI_MPI_VI_ReleaseFrame(ViChn, &stFrameInfo);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VI_ReleaseFrame fail, err code 0x%x\n",s32Ret);
        }
    }
    fclose(pfd);
    /* after dump vi frames, we should recover isp gain to orignal value */
    s32Ret = HI_MPI_ISP_SetAEAttrEx(&stAEAttrExOrg);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_ISP_SetAEAttrEx fail, err code 0x%x\n", s32Ret);
        return s32Ret;
    }

    printf("\nThe VI frame has been dumped. Step2 has been successfully done.\n");
    /******************************************************************************
      * we should use fpn.exe tool to generate the fixed pattern noise frame(fpn_1280x720.yuv)
      * windows cmd is like "FPN.exe -w 1280 -h 720 -type 420 -i ./FpnFrame_1280x720.yuv -b fpn_1280x720.yuv",
      * In the cmd, the first yuv file is input image frames, the second yuv file is the output FPN frame.
      *****************************************************************************/
    printf("Step3:Please use these frames(name:%s) and FPN.EXE tool to generate FPN frame(name:%s) manually.\n", szYuvName, szFpnYuvName);
    printf("      The windows cmd is like: FPN.exe -w 1280 -h 720 -type 420 -i %s -b %s.\n", szYuvName, szFpnYuvName);
    printf("If you have generated the FPN frame, press any key to execute step4, or generate more FPN frames.\n");
    getchar();

    return HI_SUCCESS;
}

HI_S32 SAMPLE_IVE_GetFpnFrame(IVE_FPN_DATA_S *pstFpnData)
{
    HI_S32 i, s32Ret = HI_SUCCESS;
    HI_CHAR szFpnYuvName[128] = "./fpn_1280x720.yuv";
    VB_BLK VbBlk;
    FILE *pfd;   
    
    for (i = 0; i < 3; i++)
    {
        if (0 == i)
        {
            sprintf(szFpnYuvName, "./fpn_1280x720_AG8_DG8_ISPDG1.yuv");
            pstFpnData->afFpnAGain[i] = 8;
            pstFpnData->afFpnDGain[i]= 8;
            pstFpnData->afFpnISPDGainMax[i] = 1;
        }
        else if (1 == i)
        {
            sprintf(szFpnYuvName, "./fpn_1280x720_AG8_DG8_ISPDG4.yuv");
            pstFpnData->afFpnAGain[i] = 8;
            pstFpnData->afFpnDGain[i] = 8;
            pstFpnData->afFpnISPDGainMax[i] = 4;
        }
        else if (2 == i)
        {
            sprintf(szFpnYuvName, "./fpn_1280x720_AG8_DG6_ISPDG1.yuv");
            pstFpnData->afFpnAGain[i] = 8;
            pstFpnData->afFpnDGain[i] = 6;
            pstFpnData->afFpnISPDGainMax[i] = 1;
        }

        pfd = fopen(szFpnYuvName, "rb");
        if (!pfd)
        {
            SAMPLE_PRT("open %s fail\n", szFpnYuvName);
            pstFpnData->afFpnAGain[i] = 0;
            pstFpnData->afFpnDGain[i] = 0;
            pstFpnData->afFpnISPDGainMax[i] = 0;
            pstFpnData->aFpnVbBlk[i] = VB_INVALID_HANDLE;  
            pstFpnData->abFpnFrameValid[i] = HI_FALSE;
            continue;
        }
        printf("open %s success.\n", szFpnYuvName);
        
        s32Ret = SAMPLE_IVE_GetVFrame_FromYUV(pfd, 1280, 720, 1280, &pstFpnData->astFpnFrame[i], &VbBlk);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_IVE_GetVFrame_FromYUV fail.\n");
            pstFpnData->afFpnAGain[i] = 0;
            pstFpnData->afFpnDGain[i] = 0;
            pstFpnData->afFpnISPDGainMax[i] = 0;
            pstFpnData->aFpnVbBlk[i] = VB_INVALID_HANDLE;            
            pstFpnData->abFpnFrameValid[i] = HI_FALSE;
        }
        else
        {
            pstFpnData->aFpnVbBlk[i] = VbBlk;            
            pstFpnData->abFpnFrameValid[i] = HI_TRUE;
        }
        
        fclose(pfd);
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_IVE_ReleaseFpnFrame(IVE_FPN_DATA_S *pstFpnData)
{
    HI_S32 i, s32Ret = HI_SUCCESS;
    if (NULL == pstFpnData)
    {
        SAMPLE_PRT("NULL PTR.\n");
        return HI_FAILURE;
    }

    for (i = 0; i < 3; i++)
    {
        if (HI_TRUE == pstFpnData->abFpnFrameValid[i])
        {
            if (pstFpnData->aFpnVbBlk[i] != VB_INVALID_HANDLE)
            {
                s32Ret = HI_MPI_VB_ReleaseBlock(pstFpnData->aFpnVbBlk[i]);
                if (HI_SUCCESS != s32Ret)
                {
                    SAMPLE_PRT("start vi failed!\n");
                    return s32Ret;
                }
            }
        }
    }
    
    return HI_SUCCESS;
}

void SAMPLE_IVE_Usage(char *sPrgNm)
{
    printf("Usage : %s\n", sPrgNm);
    
    printf("\t Notice: If the FPN frame is existing, you can skip the step1~step3. Otherwise, you should execute step1~step3 to generate one FPN frame first.\n");
    printf("\t         This sample, you can generate three FPN frames with different ISP gains.\n\n");
    printf("\t step1: Close aperture or cover the camera lens to generate FPN frame.\n");
    printf("\t step2: Set ISP max gains, and dump several VI frames.\n");
    printf("\t step3: Use the VI dumped frames and FPN.exe tool to generate one FPN frame manually.\n");
    printf("\t step4: According to the ISP gains value, select marching FPN frame data and use ive SUB function to improve image quality.\n");
			
    return;
}

HI_S32 main(int argc, char *argv[])
{   
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32ViChnCnt = 1;
    VB_CONF_S stVbConf;
    VO_DEV VoDev = SAMPLE_VO_DEV_DSD0;;
    VO_PUB_ATTR_S stVoPubAttr;
    SAMPLE_VO_MODE_E enVoMode = VO_MODE_1MUX;
    HI_U32 u32BlkSize;    
	pthread_t hIveThread;
    char cmd;
    IVE_FPN_DATA_S stFpnData;
    HI_BOOL bOutCycl = HI_FALSE;
	
	signal(SIGINT, SAMPLE_IVE_HandleSig);
    signal(SIGTERM, SAMPLE_IVE_HandleSig);
    
	g_bStopSignal = HI_FALSE;
    g_bFpnPrintSerial = HI_FALSE;

    SAMPLE_IVE_Usage(argv[0]);

    /******************************************
     step  1: init variable 
    ******************************************/
    
    memset(&stVbConf,0,sizeof(VB_CONF_S));
    memset(&stFpnData, 0, sizeof(IVE_FPN_DATA_S));

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
                PIC_HD720, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.u32MaxPoolCnt = 128;

    /* video buffer*/
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 30;

    /******************************************
     step 2: mpp system init. 
     ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_FPN_0;
    }

    /******************************************
     step 3: start vi dev & chn
     ******************************************/
    s32Ret = SAMPLE_COMM_VI_StartVi(&g_stViChnConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_FPN_1;
    }

    /******************************************
     step 4: start vo dev & chn
     ******************************************/

    stVoPubAttr.enIntfType = VO_INTF_CVBS;
    stVoPubAttr.enIntfSync = VO_OUTPUT_NTSC;
    stVoPubAttr.u32BgColor = 0xff;
    stVoPubAttr.bDoubleFrame = HI_FALSE;
    s32Ret = SAMPLE_COMM_VO_StartDevLayer(VoDev, &stVoPubAttr, gs_u32ViFrmRate);
    if (HI_SUCCESS != s32Ret)
    {
       SAMPLE_PRT("SAMPLE_COMM_VO_StartDevLayer failed!\n");
       goto END_FPN_1;
    }

    s32Ret = SAMPLE_COMM_VO_StartChn(VoDev, &stVoPubAttr, enVoMode);
    if (HI_SUCCESS != s32Ret)
    {
       SAMPLE_PRT("SAMPLE_COMM_VO_StartChn failed!\n");
       goto END_FPN_2;
    }

    /******************************************
     step 5: create FPN thread, but don't start FPN processing.
     ******************************************/
    memset(&stFpnData, 0, sizeof(stFpnData));
    stFpnData.bFpnProcess = HI_FALSE;
	pthread_create(&hIveThread, 0, SAMPLE_IVE_FpnProcess, (HI_VOID *)&stFpnData);
    
    while(1)
	{
        printf("\nPlease select the index. If the FPN frame is existing, you can press 1 to skip step1~step3.\n");
        printf("\t 0) step1~step3:Dump 128 VI frames, then use them and FPN.EXE tool to generate FPN frame manually.\n");
        printf("\t 1) step4:FPN frame is existing, skip step1~steo3, and start FPN process.\n");
        printf("\t 2) Preview without FPN processing.\n");
        printf("\t 3) Print current frame processing status of FPN.\n");
        printf("\t q) Quit the program.\n");
		cmd = getchar();
        getchar();
        switch(cmd)
        {
            case '0':
                /******************************************
                   step 6: generate FPN frames
                   Please close the aperture or cover the camera lens first, then dump vi frames to generate FPN frame.
                   This sample you can generate three FPN frames that corresponding to different max gain.
                   ******************************************/
                s32Ret = SAMPLE_IVE_GenerateFpnFrame();
                if (HI_SUCCESS != s32Ret)
                {
                   SAMPLE_PRT("SAMPLE_IVE_GenerateFpnFrame failed!\n");
                }
                break;
            case '1':
                /* if last FPN frame exist, release it */
                s32Ret = SAMPLE_IVE_ReleaseFpnFrame(&stFpnData);
                if (HI_SUCCESS != s32Ret)
                {
                   SAMPLE_PRT("SAMPLE_COMM_VO_StartChn failed!\n");
                   bOutCycl = HI_TRUE;
                }
                /******************************************
                   step 7: Start FPN process.
                   Get FPN frames and use ive sub function to optimize the image quality.
                   ******************************************/
                SAMPLE_IVE_GetFpnFrame(&stFpnData);
                stFpnData.bFpnProcess = HI_TRUE;
                stFpnData.bUpdateFpnData = HI_TRUE;
                break;
            case '2':
                /* Stop FPN processing */
                stFpnData.bFpnProcess = HI_FALSE;
                break;
            case '3':
                /* Print current frame processing status of FPN. */
                g_bFpnPrintSerial = HI_TRUE;
                break;
            case 'q':
                   bOutCycl = HI_TRUE;
                break;
            default:
                g_bFpnPrintSerial = HI_FALSE;
                printf("invalid index\n");
                continue;
                
        }
        
		if(HI_TRUE == bOutCycl)
        {
            break;
		}
	}

    /******************************************
     step 8: exit process
     ******************************************/
    g_bFpnPrintSerial = HI_FALSE;
    g_bStopSignal = HI_TRUE;
	pthread_join(hIveThread,HI_NULL);

    SAMPLE_COMM_VO_StopChn(VoDev, enVoMode);
END_FPN_2:
    SAMPLE_COMM_VO_StopDevLayer(VoDev);

END_FPN_1:    
    SAMPLE_COMM_VI_StopVi(&g_stViChnConfig);
END_FPN_0: 
    SAMPLE_COMM_SYS_Exit();

    if (HI_SUCCESS == s32Ret)
        printf("program exit normally!\n");
    else
        printf("program exit abnormally!\n");

	exit(s32Ret);
}



