/******************************************************************************
  A simple program of Hisilicon HI3516CV300 osd implementation.
  the flow as follows:
    1) init mpp system.
    2) start vi ( internal isp, ViDev 0, 2 vichn)
    3) start venc
    4) osd process, you can see video from some H264 streams files. the video will show as follows step:
        4.1) create some cover/osd regions
        4.2) display  cover/osd regions ( One Region -- Multi-VencGroup )
        4.3) change all vencGroups Regions' Layer
        4.4) change all vencGroups Regions' position
        4.5) change all vencGroups Regions' color
        4.6) change all vencGroups Regions' alpha (front and backgroud)
        4.7) load bmp form bmp-file to Region-0
        4.8) change BmpRegion-0
    6) stop venc
    7) stop vi and system.
  Copyright (C), 2010-2011, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2011-2 Created
******************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "loadbmp.h"

#include "sample_comm.h"
#include "mpi_vgs.h"

HI_BOOL bExit   = HI_FALSE;
HI_BOOL bTravel = HI_FALSE;
HI_BOOL bShape1 = HI_FALSE;
HI_BOOL bShape2 = HI_FALSE;
HI_BOOL bArea   = HI_FALSE;

pthread_t g_stRgnThread = 0;
pthread_t g_stVencThread = 0;
pthread_t g_stVencRgnThread = 0;
pthread_t g_stVpssRgnThread = 0;
pthread_t g_stViRgnThread = 0;

pthread_t g_stVpssOsdReverseThread = 0;
pthread_t g_stVencOsdReverseThread = 0;

#define OSD_REVERSE_RGN_MAXCNT 16
#define MAX_VENC_WORK_CHN_NUM  4

VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_PAL;
VO_INTF_TYPE_E g_enVoIntfType = VO_INTF_CVBS;
PIC_SIZE_E g_enPicSize = PIC_HD1080;


SAMPLE_VI_CONFIG_S g_stViChnConfig =
{
    PANASONIC_MN34220_SUBLVDS_1080P_30FPS,
    VIDEO_ENCODING_MODE_AUTO,

    ROTATE_NONE,
    VI_CHN_SET_NORMAL,
    WDR_MODE_NONE
};

/* used in thread */
#define SAMPLE_RGN_NOT_PASS(err)\
    do {\
        SAMPLE_PRT("\033[0;31mtest case <%s>not pass at line:%d err:%x\033[0;39m\n",\
               __FUNCTION__,__LINE__,err);\
        return NULL;\
    }while(0)

pthread_mutex_t Rgnmutex_Tmp = PTHREAD_MUTEX_INITIALIZER;

typedef struct hiRGN_OSD_REVERSE_INFO_S
{
    RGN_HANDLE Handle;
    HI_U8 u8PerPixelLumaThrd;

    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_REGION_INFO_S stLumaRgnInfo;

} RGN_OSD_REVERSE_INFO_S;

typedef struct PTHREAD_VENC
{
    VENC_CHN VeChnId;
    HI_S32 s32FrmCnt;

    FILE*  pstream;
} VENC_PTHREAD_INFO_S;

typedef struct hiRGN_SIZE_S
{
    HI_U32 u32RgnNum;
    SIZE_S stSIZE;
} RGN_SIZE_S;

typedef struct hiRGN_ATTR_INFO_S
{
    RGN_HANDLE Handle;
    HI_U32 u32RgnNum;
} RGN_ATTR_INFO_S;

static HI_S32 SAMPLE_RGN_SYS_Init(HI_VOID)
{
    HI_S32 s32Ret;
    VB_CONF_S struVbConf;
    MPP_SYS_CONF_S struSysConf;
    SIZE_S stSize;
    HI_U32 u32BlkSize;

    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, g_enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret) 
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }

    /* comm video buffer */
    memset(&struVbConf, 0, sizeof(VB_CONF_S));

    /* sensor actual pic size */
    struVbConf.u32MaxPoolCnt = 128;
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSize, 
                    SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    struVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    struVbConf.astCommPool[0].u32BlkCnt  = 10;

    stSize.u32Width  = 1920;
    stSize.u32Height = 1080;
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSize, 
                    SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    struVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
    struVbConf.astCommPool[1].u32BlkCnt  = 20;

    stSize.u32Width  = 768;
    stSize.u32Height = 576;
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize2(&stSize, 
                    SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    struVbConf.astCommPool[2].u32BlkSize = u32BlkSize;
    struVbConf.astCommPool[2].u32BlkCnt  = 10; 

    s32Ret = HI_MPI_VB_SetConf(&struVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VB_SetConf fail! s32Ret:0x%x\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_VB_Init();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VB_Init fail! s32Ret:0x%x\n", s32Ret);
        return s32Ret;
    }

    struSysConf.u32AlignWidth = 64;
    s32Ret = HI_MPI_SYS_SetConf(&struSysConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_SetConf fail! s32Ret:0x%x\n", s32Ret);
        (HI_VOID)HI_MPI_VB_Exit();
        return s32Ret;
    }

    s32Ret = HI_MPI_SYS_Init();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_Init fail! s32Ret:0x%x\n", s32Ret);
        (HI_VOID)HI_MPI_VB_Exit();
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_RGN_SYS_Exit(HI_VOID)
{
    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();

    return HI_SUCCESS;
}

HI_VOID SAMPLE_RGN_Intf_Usage(HI_CHAR* sPrgNm)
{
    printf("Usage : %s <intf> <index>\n", sPrgNm);
    printf("intf:\n");
    printf("\t 0) vo cvbs output, default.\n");
    return;
}

HI_VOID SAMPLE_RGN_Case_Usage(HI_VOID)
{
    printf("index:\n");
    printf("\t 0) online/offline VI->VPSS(COVER+MOSAIC)->VO \n");
    printf("\t 1) online/offline VI->VPSS(COVEREX)->VO \n");
    printf("\t 2) online/offline VI->VPSS(OVERLAYEX)->VO \n");
    printf("\t 3) online/offline VI->VPSS(OVERLAYEX)->VO/VENC(OVERLAY)->file \n");
	printf("\t 4) offline VI(COVEREX+OVERLAYEX)->VPSS->VO \n");
    return;
}


/******************************************************************************
* function : to process abnormal case
******************************************************************************/
#ifndef __HuaweiLite__
HI_VOID SAMPLE_RGN_HandleSig(HI_S32 signo)
{
    static int sig_handled = 0;    
    signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
    if (!sig_handled &&(SIGINT == signo || SIGTERM == signo))
    {
        sig_handled = 1;
        bExit = HI_TRUE;
        if (g_stVencOsdReverseThread)
        {
            pthread_join(g_stVencOsdReverseThread, 0);
            g_stVencOsdReverseThread = 0;
        }
		
        if (g_stVpssOsdReverseThread)
        {
            pthread_join(g_stVpssOsdReverseThread, 0);
            g_stVpssOsdReverseThread = 0;
        }
		
        if (g_stVencRgnThread)
        {
            pthread_join(g_stVencRgnThread, 0);
            g_stVencRgnThread = 0;
        }
		
        if (g_stVpssRgnThread)
        {
            pthread_join(g_stVpssRgnThread, 0);
            g_stVpssRgnThread = 0;
        }    
		
        if (g_stVencThread)
        {
            pthread_join(g_stVencThread, 0);
            g_stVencThread = 0;
        }
		
        if (g_stRgnThread)
        {
            pthread_join(g_stRgnThread, 0);
            g_stRgnThread = 0;
        }
		
		if (g_stViRgnThread)
        {
            pthread_join(g_stViRgnThread, 0);
            g_stViRgnThread = 0;
        }
        
        //HI_MPI_RGN_Destroy(gs_s32RgnCntCur);
        SAMPLE_COMM_ISP_Stop();
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
		sig_handled = 0;
    }
    
    exit(0);
}
#endif

/******************************************************************************
* funciton : osd region change color
******************************************************************************/
HI_S32 SAMPLE_RGN_ChgColor(RGN_HANDLE RgnHandle, HI_U32 u32Color)
{
    HI_S32 s32Ret;
    RGN_ATTR_S stRgnAttr;

    s32Ret = HI_MPI_RGN_GetAttr(RgnHandle, &stRgnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_RGN_GetAttr (%d)) failed with %#x!\n", RgnHandle, s32Ret);
        return HI_FAILURE;
    }

    stRgnAttr.unAttr.stOverlay.u32BgColor = u32Color;

    s32Ret = HI_MPI_RGN_SetAttr(RgnHandle, &stRgnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_RGN_SetAttr (%d)) failed with %#x!\n", RgnHandle, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/******************************************************************************
* funciton : load bmp from file
******************************************************************************/
HI_S32 SAMPLE_RGN_LoadBmp(const char* filename, BITMAP_S* pstBitmap, HI_BOOL bFil, HI_U32 u16FilColor)
{
    OSD_SURFACE_S Surface;
    OSD_BITMAPFILEHEADER bmpFileHeader;
    OSD_BITMAPINFO bmpInfo;
    HI_U32 u32BytePerPix = 0;

    if (GetBmpInfo(filename, &bmpFileHeader, &bmpInfo) < 0)
    {
        SAMPLE_PRT("GetBmpInfo err!\n");
        return HI_FAILURE;
    }

    Surface.enColorFmt = OSD_COLOR_FMT_RGB1555;
    u32BytePerPix      = 2;

    pstBitmap->pData = malloc(u32BytePerPix * (bmpInfo.bmiHeader.biWidth) * (bmpInfo.bmiHeader.biHeight));

    if (NULL == pstBitmap->pData)
    {
        SAMPLE_PRT("malloc osd memroy err!\n");
        return HI_FAILURE;
    }
    
    if (0 != CreateSurfaceByBitMap(filename, &Surface, (HI_U8*)(pstBitmap->pData)))
    {
    	free(pstBitmap->pData);
		pstBitmap->pData = HI_NULL;
		return HI_FAILURE;
	}
	
    pstBitmap->u32Width      = Surface.u16Width;
    pstBitmap->u32Height     = Surface.u16Height;
    pstBitmap->enPixelFormat = PIXEL_FORMAT_RGB_1555;


    int i, j;
    HI_U16* pu16Temp;
    pu16Temp = (HI_U16*)pstBitmap->pData;

    if (bFil)
    {
        for (i = 0; i < pstBitmap->u32Height; i++)
        {
            for (j = 0; j < pstBitmap->u32Width; j++)
            {
                if (u16FilColor == *pu16Temp)
                {
                    *pu16Temp &= 0x7FFF;
                }

                pu16Temp++;
            }
        }

    }

    return HI_SUCCESS;
}


HI_S32 SAMPLE_RGN_UpdateCanvas(const char* filename, BITMAP_S* pstBitmap, HI_BOOL bFil,
                               HI_U32 u16FilColor, SIZE_S* pstSize, HI_U32 u32Stride, PIXEL_FORMAT_E enPixelFmt)
{
    OSD_SURFACE_S Surface;
    OSD_BITMAPFILEHEADER bmpFileHeader;
    OSD_BITMAPINFO bmpInfo;

    if (GetBmpInfo(filename, &bmpFileHeader, &bmpInfo) < 0)
    {
        SAMPLE_PRT("GetBmpInfo err!\n");
        return HI_FAILURE;
    }

    if (PIXEL_FORMAT_RGB_1555 == enPixelFmt)
    {
        Surface.enColorFmt = OSD_COLOR_FMT_RGB1555;
    }
    else if (PIXEL_FORMAT_RGB_4444 == enPixelFmt)
    {
        Surface.enColorFmt = OSD_COLOR_FMT_RGB4444;
    }
    else if (PIXEL_FORMAT_RGB_8888 == enPixelFmt)
    {
        Surface.enColorFmt = OSD_COLOR_FMT_RGB8888;
    }
    else
    {
        SAMPLE_PRT("Pixel format is not support!\n");
        return HI_FAILURE;
    }

    if (NULL == pstBitmap->pData)
    {
        SAMPLE_PRT("malloc osd memroy err!\n");
        return HI_FAILURE;
    }
    CreateSurfaceByCanvas(filename, &Surface, (HI_U8*)(pstBitmap->pData), pstSize->u32Width, pstSize->u32Height, u32Stride);

    pstBitmap->u32Width  = Surface.u16Width;
    pstBitmap->u32Height = Surface.u16Height;

    if (PIXEL_FORMAT_RGB_1555 == enPixelFmt)
    {
        pstBitmap->enPixelFormat = PIXEL_FORMAT_RGB_1555;
    }
    else if (PIXEL_FORMAT_RGB_4444 == enPixelFmt)
    {
        pstBitmap->enPixelFormat = PIXEL_FORMAT_RGB_4444;
    }
    else if (PIXEL_FORMAT_RGB_8888 == enPixelFmt)
    {
        pstBitmap->enPixelFormat = PIXEL_FORMAT_RGB_8888;
    }

    int i, j;
    HI_U16* pu16Temp;
    pu16Temp = (HI_U16*)pstBitmap->pData;

    if (bFil)
    {
        for (i = 0; i < pstBitmap->u32Height; i++)
        {
            for (j = 0; j < pstBitmap->u32Width; j++)
            {
                if (u16FilColor == *pu16Temp)
                {
                    *pu16Temp &= 0x7FFF;
                }

                pu16Temp++;
            }
        }

    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_RGN_ReverseOsdColor(RGN_CANVAS_INFO_S *pstCanvasInfoSrc, RGN_CANVAS_INFO_S *pstCanvasInfoDest,
                                  const VPSS_REGION_INFO_S* pstRgnInfo)
{
    HI_S32 i;
    HI_S32 s32Ret;
    VGS_HANDLE handle = -1;
    VGS_TASK_ATTR_S stTaskAttr;
	VGS_ADD_QUICK_COPY_S stAddQuickCpy;
	VGS_ADD_REVERT_S stAddRevert;

    HI_ASSERT(NULL != pstCanvasInfoSrc);
    HI_ASSERT(NULL != pstCanvasInfoDest);
    HI_ASSERT(NULL != pstRgnInfo);

    s32Ret = HI_MPI_VGS_BeginJob(&handle);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VGS_BeginJob fail! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    stTaskAttr.stImgIn.stVFrame.enPixelFormat = pstCanvasInfoSrc->enPixelFmt;
	stTaskAttr.stImgIn.stVFrame.u32Height = pstCanvasInfoSrc->stSize.u32Height;
	stTaskAttr.stImgIn.stVFrame.u32Width = pstCanvasInfoSrc->stSize.u32Width;
	stTaskAttr.stImgIn.stVFrame.pVirAddr[0] = (HI_VOID *)pstCanvasInfoSrc->u32VirtAddr;
	stTaskAttr.stImgIn.stVFrame.u32PhyAddr[0] = pstCanvasInfoSrc->u32PhyAddr;
	stTaskAttr.stImgIn.stVFrame.u32Stride[0] = pstCanvasInfoSrc->u32Stride;

	stTaskAttr.stImgOut.stVFrame.enPixelFormat = pstCanvasInfoDest->enPixelFmt;
	stTaskAttr.stImgOut.stVFrame.u32Height = pstCanvasInfoDest->stSize.u32Height;
	stTaskAttr.stImgOut.stVFrame.u32Width = pstCanvasInfoDest->stSize.u32Width;
	stTaskAttr.stImgOut.stVFrame.pVirAddr[0] = (HI_VOID *)pstCanvasInfoDest->u32VirtAddr;
	stTaskAttr.stImgOut.stVFrame.u32PhyAddr[0] = pstCanvasInfoDest->u32PhyAddr;
	stTaskAttr.stImgOut.stVFrame.u32Stride[0] = pstCanvasInfoDest->u32Stride;

	stAddQuickCpy.stSrcRect.s32X = 0;
	stAddQuickCpy.stSrcRect.s32Y = 0;
	stAddQuickCpy.stSrcRect.u32Width = stTaskAttr.stImgIn.stVFrame.u32Width;
	stAddQuickCpy.stSrcRect.u32Height = stTaskAttr.stImgIn.stVFrame.u32Height;

	stAddQuickCpy.stDestRect.s32X = 0;
	stAddQuickCpy.stDestRect.s32Y = 0;
	stAddQuickCpy.stDestRect.u32Width = stTaskAttr.stImgOut.stVFrame.u32Width;
	stAddQuickCpy.stDestRect.u32Height = stTaskAttr.stImgOut.stVFrame.u32Height;
    s32Ret = HI_MPI_VGS_AddQuickCopyTask(handle, &stTaskAttr, &stAddQuickCpy);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VGS_AddQuickCopyTask fail! s32Ret: 0x%x.\n", s32Ret);
        HI_MPI_VGS_CancelJob(handle);
        return s32Ret;
    }

    for (i = 0; i < pstRgnInfo->u32RegionNum; ++i)
    {
        stAddRevert.stSrcRect.s32X = pstRgnInfo->pstRegion[i].s32X;
        stAddRevert.stSrcRect.s32Y = pstRgnInfo->pstRegion[i].s32Y;
        stAddRevert.stSrcRect.u32Width  = pstRgnInfo->pstRegion[i].u32Width;
        stAddRevert.stSrcRect.u32Height = pstRgnInfo->pstRegion[i].u32Height;

		stAddRevert.stDestRect.s32X = pstRgnInfo->pstRegion[i].s32X;
        stAddRevert.stDestRect.s32Y = pstRgnInfo->pstRegion[i].s32Y;
        stAddRevert.stDestRect.u32Width  = pstRgnInfo->pstRegion[i].u32Width;
        stAddRevert.stDestRect.u32Height = pstRgnInfo->pstRegion[i].u32Height;
		stAddRevert.enRevertMode = VGS_COLOR_REVERT_RGB;

        s32Ret = HI_MPI_VGS_AddRevertTask(handle, &stTaskAttr, &stAddRevert);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VGS_AddRevertTask fail! s32Ret: 0x%x.\n", s32Ret);
            HI_MPI_VGS_CancelJob(handle);
            return s32Ret;
        }
    }

    s32Ret = HI_MPI_VGS_EndJob(handle);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VGS_EndJob fail! s32Ret: 0x%x.\n", s32Ret);
        HI_MPI_VGS_CancelJob(handle);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_RGN_GetVoDisplayNum(HI_U32 u32VoChnNum)
{
    HI_S32 s32DispNum;

    if (1 == u32VoChnNum)
    {
        s32DispNum = 1;
    }
    else if (4 == u32VoChnNum)
    {
        s32DispNum = 2;
    }
    else if (9 == u32VoChnNum)
    {
        s32DispNum = 3;
    }
    else if (16 == u32VoChnNum)
    {
        s32DispNum = 4;
    }
    else
    {
        return -1;
    }

    return s32DispNum;
}


HI_S32 SAMPLE_RGN_GetVoLayer(VO_DEV VoDev)
{
    HI_S32 s32LayerNum;

    if (0 == VoDev)
    {
        s32LayerNum = 0;
    }
    else
    {
        return -1;
    }

    return s32LayerNum;
}

HI_S32 SAMPLE_RGN_CreateMosaic(RGN_HANDLE Handle, HI_U32 u32Num)
{
    HI_S32 i;
    HI_S32 s32Ret;
    MPP_CHN_S stChn;
    RGN_ATTR_S stRgnAttr;
    RGN_CHN_ATTR_S stChnAttr;

    /* Add cover to vpss group */
    stChn.enModId  = HI_ID_VPSS;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;

    /* Create cover and attach to vpss group */
    for (i = Handle; i < (Handle + u32Num); i++)
    {
        stRgnAttr.enType = MOSAIC_RGN;

        s32Ret = HI_MPI_RGN_Create(i, &stRgnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_RGN_Create fail! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }

        stChnAttr.bShow  = HI_TRUE;
        stChnAttr.enType = MOSAIC_RGN;
        
        stChnAttr.unChnAttr.stMosaicChn.enBlkSize = MOSAIC_BLK_SIZE_8 + (i - Handle) % MOSAIC_BLK_SIZE_64;
        stChnAttr.unChnAttr.stMosaicChn.stRect.s32X      = 32 * (i - Handle);
        stChnAttr.unChnAttr.stMosaicChn.stRect.s32Y      = 32 * (i - Handle);
        stChnAttr.unChnAttr.stMosaicChn.stRect.u32Height = 64 * (i - Handle + 1);
        stChnAttr.unChnAttr.stMosaicChn.stRect.u32Width  = 64 * (i - Handle + 1);
       
        stChnAttr.unChnAttr.stMosaicChn.u32Layer         = i - Handle;

        s32Ret = HI_MPI_RGN_AttachToChn(i, &stChn, &stChnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_RGN_AttachToChn fail! s32Ret: 0x%x. w %d, H %d\n", s32Ret, \
                   stChnAttr.unChnAttr.stMosaicChn.stRect.u32Height, stChnAttr.unChnAttr.stMosaicChn.stRect.u32Width);
            return s32Ret;
        }
    }

    return HI_SUCCESS;

}


HI_S32 SAMPLE_RGN_CreateCover(RGN_HANDLE Handle, HI_U32 u32Num)
{
    HI_S32 i;
    HI_S32 s32Ret;
    MPP_CHN_S stChn;
    RGN_ATTR_S stRgnAttr;
    RGN_CHN_ATTR_S stChnAttr;

    /* Add cover to vpss group */
    stChn.enModId  = HI_ID_VPSS;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;

    /* Create cover and attach to vpss group */
    for (i = Handle; i < (Handle + u32Num); i++)
    {
        stRgnAttr.enType = COVER_RGN;

        s32Ret = HI_MPI_RGN_Create(i, &stRgnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_RGN_Create fail! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }

        stChnAttr.bShow  = HI_TRUE;
        stChnAttr.enType = COVER_RGN;
        if (i < 4)
        {
            stChnAttr.unChnAttr.stCoverChn.enCoverType = AREA_RECT;
            stChnAttr.unChnAttr.stCoverChn.stRect.s32X      = 10 * (i - Handle);
            stChnAttr.unChnAttr.stCoverChn.stRect.s32Y      = 10 * (i - Handle);
            stChnAttr.unChnAttr.stCoverChn.stRect.u32Height = 64 * (i - Handle + 1);
            stChnAttr.unChnAttr.stCoverChn.stRect.u32Width  = 64 * (i - Handle + 1);
        }
        else
        {
            stChnAttr.unChnAttr.stCoverChn.enCoverType = AREA_QUAD_RANGLE;
            stChnAttr.unChnAttr.stCoverChn.stQuadRangle.bSolid = HI_TRUE;
            stChnAttr.unChnAttr.stCoverChn.stQuadRangle.u32Thick = 2;
            stChnAttr.unChnAttr.stCoverChn.stQuadRangle.stPoint[0].s32X = 50 * i;
            stChnAttr.unChnAttr.stCoverChn.stQuadRangle.stPoint[0].s32Y = 0;
            stChnAttr.unChnAttr.stCoverChn.stQuadRangle.stPoint[1].s32X = 50 + 50 * i;
            stChnAttr.unChnAttr.stCoverChn.stQuadRangle.stPoint[1].s32Y = 50;
            stChnAttr.unChnAttr.stCoverChn.stQuadRangle.stPoint[2].s32X = 50 + 50 * i;
            stChnAttr.unChnAttr.stCoverChn.stQuadRangle.stPoint[2].s32Y = 300;
            stChnAttr.unChnAttr.stCoverChn.stQuadRangle.stPoint[3].s32X = 50 * i;
            stChnAttr.unChnAttr.stCoverChn.stQuadRangle.stPoint[3].s32Y = 200;
        }
        stChnAttr.unChnAttr.stCoverChn.u32Color         = 0x000000ff;
        if (1 == i % COVER_MAX_NUM_VPSS)
        {
            stChnAttr.unChnAttr.stCoverChn.u32Color     = 0x0000ff00;
        }
        else if (2 == i % COVER_MAX_NUM_VPSS)
        {
            stChnAttr.unChnAttr.stCoverChn.u32Color     = 0x00ff0000;
        }
        else if (3 == i % COVER_MAX_NUM_VPSS)
        {
            stChnAttr.unChnAttr.stCoverChn.u32Color     = 0x00808080;
        }
        stChnAttr.unChnAttr.stCoverChn.u32Layer         = i - Handle;

        s32Ret = HI_MPI_RGN_AttachToChn(i, &stChn, &stChnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_RGN_AttachToChn fail! s32Ret: 0x%x. w %d, H %d\n", s32Ret, \
                   stChnAttr.unChnAttr.stCoverChn.stRect.u32Height, stChnAttr.unChnAttr.stCoverChn.stRect.u32Width);
            return s32Ret;
        }
    }

    return HI_SUCCESS;

}

HI_S32 SAMPLE_RGN_CreateCoverEx(RGN_HANDLE Handle, HI_U32 u32Num, MPP_CHN_S *pstChn)
{
    HI_S32 i;
    HI_S32 s32Ret;
    MPP_CHN_S stChn;
    RGN_ATTR_S stRgnAttr;
    RGN_CHN_ATTR_S stChnAttr;

	if (pstChn == HI_NULL)
    {
        SAMPLE_PRT("Input pstChn cannot be NULL! \n");
        return HI_FAILURE;
    }

    /* Add cover to vpss group */
    stChn.enModId  = pstChn->enModId;
    stChn.s32DevId = pstChn->s32DevId;
    stChn.s32ChnId = pstChn->s32ChnId;

    /* Create cover and attach to vpss group */
    for (i = Handle; i < (Handle + u32Num); i++)
    {
        stRgnAttr.enType = COVEREX_RGN;

        s32Ret = HI_MPI_RGN_Create(i, &stRgnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_RGN_Create fail! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }

        stChnAttr.bShow  = HI_TRUE;
        stChnAttr.enType = COVEREX_RGN;
        if (i < 4)
        {
            stChnAttr.unChnAttr.stCoverExChn.enCoverType = AREA_RECT;
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32X      = 10 * (i - Handle);
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y      = 10 * (i - Handle);
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = 64 * (i - Handle + 1);
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width  = 64 * (i - Handle + 1);
        }
        else
        {
            stChnAttr.unChnAttr.stCoverExChn.enCoverType = AREA_QUAD_RANGLE;
            stChnAttr.unChnAttr.stCoverExChn.stQuadRangle.bSolid = HI_TRUE;
            stChnAttr.unChnAttr.stCoverExChn.stQuadRangle.u32Thick = 2;
            stChnAttr.unChnAttr.stCoverExChn.stQuadRangle.stPoint[0].s32X = 50 * i;
            stChnAttr.unChnAttr.stCoverExChn.stQuadRangle.stPoint[0].s32Y = 0;
            stChnAttr.unChnAttr.stCoverExChn.stQuadRangle.stPoint[1].s32X = 50 + 50 * i;
            stChnAttr.unChnAttr.stCoverExChn.stQuadRangle.stPoint[1].s32Y = 50;
            stChnAttr.unChnAttr.stCoverExChn.stQuadRangle.stPoint[2].s32X = 50 + 50 * i;
            stChnAttr.unChnAttr.stCoverExChn.stQuadRangle.stPoint[2].s32Y = 300;
            stChnAttr.unChnAttr.stCoverExChn.stQuadRangle.stPoint[3].s32X = 50 * i;
            stChnAttr.unChnAttr.stCoverExChn.stQuadRangle.stPoint[3].s32Y = 200;
        }
        stChnAttr.unChnAttr.stCoverExChn.u32Color         = 0x000000ff;
        if (1 == i % u32Num)
        {
            stChnAttr.unChnAttr.stCoverExChn.u32Color     = 0x0000ff00;
        }
        else if (2 == i % u32Num)
        {
            stChnAttr.unChnAttr.stCoverExChn.u32Color     = 0x00ff0000;
        }
        else if (3 == i % u32Num)
        {
            stChnAttr.unChnAttr.stCoverExChn.u32Color     = 0x00808080;
        }
        stChnAttr.unChnAttr.stCoverExChn.u32Layer         = i - Handle;

        s32Ret = HI_MPI_RGN_AttachToChn(i, &stChn, &stChnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_RGN_AttachToChn fail! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;

}

HI_S32 SAMPLE_RGN_CreateOverlayEx(RGN_HANDLE Handle, HI_U32 u32Num, MPP_CHN_S *pstChn)
{
    HI_S32 i;
    HI_S32 s32Ret;
    MPP_CHN_S stChn;
    HI_U32 u32layer = 0;
    RGN_ATTR_S stRgnAttrSet;
    RGN_CHN_ATTR_S stChnAttr;

	if (pstChn == HI_NULL)
    {
        SAMPLE_PRT("Input pstChn cannot be NULL! \n");
        return HI_FAILURE;
    }

    /*attach the OSD to the vpss*/
    stChn.enModId  = pstChn->enModId;
    stChn.s32DevId = pstChn->s32DevId;
    stChn.s32ChnId = pstChn->s32ChnId;

    for (i = Handle; i < (Handle + u32Num); i++)
    {
        stRgnAttrSet.enType = OVERLAYEX_RGN;
        stRgnAttrSet.unAttr.stOverlayEx.enPixelFmt       = PIXEL_FORMAT_RGB_1555;
        stRgnAttrSet.unAttr.stOverlayEx.stSize.u32Width  = 300;
        stRgnAttrSet.unAttr.stOverlayEx.stSize.u32Height = 300;
        stRgnAttrSet.unAttr.stOverlayEx.u32BgColor       = 0x000003e0;

        if (1 == i % u32Num)
        {
            stRgnAttrSet.unAttr.stOverlayEx.stSize.u32Width  = 180;
            stRgnAttrSet.unAttr.stOverlayEx.stSize.u32Height = 120;
            stRgnAttrSet.unAttr.stOverlayEx.u32BgColor       = 0x0000001f;
        }
        else if (2 == i % u32Num)
        {
            stRgnAttrSet.unAttr.stOverlayEx.stSize.u32Width  = 160;
            stRgnAttrSet.unAttr.stOverlayEx.stSize.u32Height = 120;
            stRgnAttrSet.unAttr.stOverlayEx.u32BgColor       = 0x00007c00;
        }
        else if (3 == i % u32Num)
        {
            stRgnAttrSet.unAttr.stOverlayEx.stSize.u32Width  = 100;
            stRgnAttrSet.unAttr.stOverlayEx.stSize.u32Height = 150;
            stRgnAttrSet.unAttr.stOverlayEx.u32BgColor       = 0x000007ff;
        }

        s32Ret = HI_MPI_RGN_Create(i, &stRgnAttrSet);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_RGN_Create failed! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }

        stChnAttr.bShow  = HI_TRUE;
        stChnAttr.enType = OVERLAYEX_RGN;
        stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 48;
        stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 48;
        stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha   = 255;
        stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha   = 255;
        stChnAttr.unChnAttr.stOverlayExChn.u32Layer     = u32layer;
        u32layer++;
        if (1 == i % 4)
        {
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 130;
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 330;
        }
        else if (2 == i % 4)
        {
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 270;
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 300;
        }
        else if (3 == i % 4)
        {
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 180;
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 400;
        }

        s32Ret = HI_MPI_RGN_AttachToChn(i, &stChn, &stChnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_RGN_AttachToChn failed! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;

}


HI_S32 SAMPLE_RGN_CreateOverlayForVenc(RGN_HANDLE Handle, HI_U32 u32Num)
{
    HI_S32 i;
    HI_S32 s32Ret;
    MPP_CHN_S stChn;
    RGN_ATTR_S stRgnAttr;
    RGN_CHN_ATTR_S stChnAttr;

    /* Add cover to vpss group */
    stChn.enModId  = HI_ID_VENC;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;

    for (i = Handle; i < (Handle + u32Num); i++)
    {
        stRgnAttr.enType = OVERLAY_RGN;
        stRgnAttr.unAttr.stOverlay.enPixelFmt       = PIXEL_FORMAT_RGB_1555;
        stRgnAttr.unAttr.stOverlay.stSize.u32Width  = 300;
        stRgnAttr.unAttr.stOverlay.stSize.u32Height = 300;
        stRgnAttr.unAttr.stOverlay.u32BgColor       = 0x000003e0;
		//stRgnAttr.unAttr.stOverlay.u32CanvasNum = 6;

        if (1 == i % u32Num)
        {
            stRgnAttr.unAttr.stOverlay.stSize.u32Width  = 180;
            stRgnAttr.unAttr.stOverlay.stSize.u32Height = 120;
            stRgnAttr.unAttr.stOverlay.u32BgColor       = 0x0000001f;
        }
        else if (2 == i % u32Num)
        {
            stRgnAttr.unAttr.stOverlay.stSize.u32Width  = 160;
            stRgnAttr.unAttr.stOverlay.stSize.u32Height = 120;
            stRgnAttr.unAttr.stOverlay.u32BgColor       = 0x00007c00;
        }
        else if (3 == i % u32Num)
        {
            stRgnAttr.unAttr.stOverlay.stSize.u32Width  = 100;
            stRgnAttr.unAttr.stOverlay.stSize.u32Height = 160;
            stRgnAttr.unAttr.stOverlay.u32BgColor       = 0x000007ff;
        }

        s32Ret = HI_MPI_RGN_Create(i, &stRgnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_RGN_Create failed! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }

        stChnAttr.bShow  = HI_TRUE;
        stChnAttr.enType = OVERLAY_RGN;
        stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 48;
        stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 48;
        stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha   = 128;
        stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha   = 128;
        stChnAttr.unChnAttr.stOverlayChn.u32Layer     = i;

        stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bAbsQp = HI_FALSE;
        stChnAttr.unChnAttr.stOverlayChn.stQpInfo.s32Qp  = 0;
		stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bQpDisable = HI_FALSE;
        stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = HI_FALSE;
        if (1 == i % u32Num)
        {
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 128;
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 320;
        }
        else if (2 == i % u32Num)
        {
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 280;
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 240;
        }
        else if (3 == i % u32Num)
        {
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 180;
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 400;
        }
        s32Ret = HI_MPI_RGN_AttachToChn(i, &stChn, &stChnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_RGN_AttachToChn failed! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;

}


HI_S32 SAMPLE_RGN_GetVoAttr(VO_DEV VoDev, VO_INTF_SYNC_E enIntfSync, VO_PUB_ATTR_S* pstPubAttr,
                            VO_VIDEO_LAYER_ATTR_S* pstLayerAttr, HI_S32 s32SquareSort, VO_CHN_ATTR_S* astChnAttr)
{
    VO_INTF_TYPE_E enIntfType;
    HI_U32 u32Frmt, u32Width, u32Height, j;

    enIntfType = VO_INTF_BT1120;
    if (enIntfSync == VO_OUTPUT_960H_PAL || enIntfSync == VO_OUTPUT_960H_NTSC
        || enIntfSync == VO_OUTPUT_PAL || enIntfSync == VO_OUTPUT_NTSC)
    {
        enIntfType = VO_INTF_CVBS;
    }

    switch (enIntfSync)
    {
        case VO_OUTPUT_PAL      :
            u32Width = 720;
            u32Height = 576;
            u32Frmt = 25;
            break;
        case VO_OUTPUT_NTSC     :
            u32Width = 720;
            u32Height = 480;
            u32Frmt = 30;
            break;
        case VO_OUTPUT_1080P24  :
            u32Width = 1920;
            u32Height = 1080;
            u32Frmt = 24;
            break;
        case VO_OUTPUT_1080P25  :
            u32Width = 1920;
            u32Height = 1080;
            u32Frmt = 25;
            break;
        case VO_OUTPUT_1080P30  :
            u32Width = 1920;
            u32Height = 1080;
            u32Frmt = 30;
            break;
        case VO_OUTPUT_720P50   :
            u32Width = 1280;
            u32Height = 720;
            u32Frmt = 50;
            break;
        case VO_OUTPUT_720P60   :
            u32Width = 1280;
            u32Height = 720;
            u32Frmt = 60;
            break;
        case VO_OUTPUT_1080I50  :
            u32Width = 1920;
            u32Height = 1080;
            u32Frmt = 50;
            break;
        case VO_OUTPUT_1080I60  :
            u32Width = 1920;
            u32Height = 1080;
            u32Frmt = 60;
            break;
        case VO_OUTPUT_1080P50  :
            u32Width = 1920;
            u32Height = 1080;
            u32Frmt = 50;
            break;
        case VO_OUTPUT_1080P60  :
            u32Width = 1920;
            u32Height = 1080;
            u32Frmt = 60;
            break;
        case VO_OUTPUT_576P50   :
            u32Width = 720;
            u32Height = 576;
            u32Frmt = 50;
            break;
        case VO_OUTPUT_480P60   :
            u32Width = 720;
            u32Height = 480;
            u32Frmt = 60;
            break;
        case VO_OUTPUT_800x600_60:
            u32Width = 800;
            u32Height = 600;
            u32Frmt = 60;
            break;
        case VO_OUTPUT_1024x768_60:
            u32Width = 1024;
            u32Height = 768;
            u32Frmt = 60;
            break;
        case VO_OUTPUT_1280x1024_60:
            u32Width = 1280;
            u32Height = 1024;
            u32Frmt = 60;
            break;
        case VO_OUTPUT_1366x768_60:
            u32Width = 1366;
            u32Height = 768;
            u32Frmt = 60;
            break;
        case VO_OUTPUT_1440x900_60:
            u32Width = 1440;
            u32Height = 900;
            u32Frmt = 60;
            break;
        case VO_OUTPUT_1280x800_60:
            u32Width = 1280;
            u32Height = 800;
            u32Frmt = 60;
            break;

        default:
            return HI_FAILURE;
    }

    if (NULL != pstPubAttr)
    {
        pstPubAttr->enIntfSync = enIntfSync;
        pstPubAttr->u32BgColor = 0;
        pstPubAttr->enIntfType = enIntfType;
    }

    if (NULL != pstLayerAttr)
    {
        pstLayerAttr->stDispRect.s32X       = 0;
        pstLayerAttr->stDispRect.s32Y       = 0;
        pstLayerAttr->stDispRect.u32Width   = u32Width;
        pstLayerAttr->stDispRect.u32Height  = u32Height;
        pstLayerAttr->stImageSize.u32Width  = u32Width;
        pstLayerAttr->stImageSize.u32Height = u32Height;
        pstLayerAttr->bDoubleFrame          = HI_FALSE;
        pstLayerAttr->bClusterMode          = HI_FALSE;
        pstLayerAttr->u32DispFrmRt          = u32Frmt;
        pstLayerAttr->enPixFormat           = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    }

    if (NULL != astChnAttr)
    {
        for (j = 0; j < (s32SquareSort * s32SquareSort); j++)
        {
            astChnAttr[j].stRect.s32X       = ALIGN_BACK((u32Width / s32SquareSort) * (j % s32SquareSort), 4);
            astChnAttr[j].stRect.s32Y       = ALIGN_BACK((u32Height / s32SquareSort) * (j / s32SquareSort), 4);
            astChnAttr[j].stRect.u32Width   = ALIGN_BACK(u32Width / s32SquareSort, 4);
            astChnAttr[j].stRect.u32Height  = ALIGN_BACK(u32Height / s32SquareSort, 4);
            astChnAttr[j].u32Priority       = 0;
            astChnAttr[j].bDeflicker        = HI_FALSE;
        }
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_RGN_StartVpss(HI_S32 s32VpssGrpNum, HI_U32 u32VpssChnNum)
{
    HI_S32 i = 0;
    HI_S32 s32Ret;
    VPSS_CHN_MODE_S stVpssChnMode;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stGrpAttr = {0};
    VPSS_CHN_ATTR_S stVpssChnAttr;
    PIC_SIZE_E enPicSize = g_enPicSize;
    SIZE_S stSize;

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enPicSize, &stSize);
	if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("get pic size fail, s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;

    }
    memset(&stVpssChnAttr, 0, sizeof(VPSS_CHN_ATTR_S));

    stGrpAttr.u32MaxW   = stSize.u32Width;
    stGrpAttr.u32MaxH   = stSize.u32Height;
    stGrpAttr.enPixFmt  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
    stGrpAttr.bIeEn     = HI_FALSE;
    stGrpAttr.bNrEn     = HI_FALSE;
    stGrpAttr.bHistEn   = HI_FALSE;
    stGrpAttr.bDciEn    = HI_FALSE;

    for (i = 0; i < s32VpssGrpNum; i++)
    {
        s32Ret = HI_MPI_VPSS_CreateGrp(i, &stGrpAttr);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("create vpss grp%d fail, s32Ret: 0x%x.\n", i, s32Ret);
            return s32Ret;

        }

        stVpssChnMode.bDouble 	     = HI_FALSE;
        stVpssChnMode.enChnMode 	 = VPSS_CHN_MODE_USER;
        stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        stVpssChnMode.u32Width 	     = 720;
        stVpssChnMode.u32Height 	 = 576;
        stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;

        stVpssChnAttr.bBorderEn = HI_FALSE;
        stVpssChnAttr.bSpEn = HI_FALSE;
        stVpssChnAttr.bFlip = HI_FALSE;
        stVpssChnAttr.bMirror = HI_FALSE;
        stVpssChnAttr.s32SrcFrameRate = -1;
        stVpssChnAttr.s32DstFrameRate = -1;

        for (VpssChn = 0; VpssChn < u32VpssChnNum; VpssChn++)
        {
            if (0 == VpssChn)
            {
                stVpssChnMode.u32Width 	     = stSize.u32Width;
                stVpssChnMode.u32Height 	 = stSize.u32Height;
            }
			else
			{
				stVpssChnMode.u32Width 	     = 720;
				stVpssChnMode.u32Height 	 = 576;
			}
            
            s32Ret = HI_MPI_VPSS_SetChnMode(i, VpssChn, &stVpssChnMode);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("set vpss grp%d chn%d mode fail, s32Ret: 0x%x.\n", i, VpssChn, s32Ret);
                return s32Ret;
            }

            s32Ret = HI_MPI_VPSS_SetChnAttr(i, VpssChn, &stVpssChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("set vpss grp%d chn%d Attr fail, s32Ret: 0x%x.\n", i, VpssChn, s32Ret);
                return s32Ret;
            }

            s32Ret = HI_MPI_VPSS_EnableChn(i, VpssChn);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("create vpss grp%d chnl%d fail, s32Ret: 0x%x.\n", i, VpssChn, s32Ret);
                return s32Ret;
            }
        }

        s32Ret = HI_MPI_VPSS_StartGrp(i);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("start vpss grp%d fail, s32Ret: 0x%x.\n", i, s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_RGN_StartVpssHD(HI_S32 s32VpssGrpNum, HI_U32 u32VpssChnNum)
{
    HI_S32 i = 0;
    HI_S32 s32Ret;
    VPSS_CHN_MODE_S stVpssChnMode;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stGrpAttr = {0};
    VPSS_CHN_ATTR_S stVpssChnAttr;
    PIC_SIZE_E enPicSize = g_enPicSize;
    SIZE_S stSize;

    if (SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enPicSize, &stSize))
    {
		SAMPLE_PRT("get pic size fail!\n");
        return HI_FAILURE;
	}
	

    memset(&stVpssChnAttr, 0, sizeof(VPSS_CHN_ATTR_S));

    stGrpAttr.u32MaxW      = stSize.u32Width;
    stGrpAttr.u32MaxH      = stSize.u32Height;
    stGrpAttr.enPixFmt     = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stGrpAttr.enDieMode    = VPSS_DIE_MODE_NODIE;
    stGrpAttr.bIeEn        = HI_FALSE;
    stGrpAttr.bNrEn        = HI_FALSE;
    stGrpAttr.bHistEn      = HI_FALSE;
    stGrpAttr.bDciEn       = HI_FALSE;

    for (i = 0; i < s32VpssGrpNum; i++)
    {
        s32Ret = HI_MPI_VPSS_CreateGrp(i, &stGrpAttr);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("creat vpss grp%d fail! s32Ret: 0x%x.\n", i, s32Ret);
            return s32Ret;

        }

        stVpssChnMode.bDouble 	     = HI_FALSE;
        stVpssChnMode.enChnMode 	 = VPSS_CHN_MODE_USER;
        stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        stVpssChnMode.u32Width 	     = stSize.u32Width;
        stVpssChnMode.u32Height 	 = stSize.u32Height;
        stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;

        stVpssChnAttr.bBorderEn = HI_FALSE;
        stVpssChnAttr.bSpEn     = HI_FALSE;
        stVpssChnAttr.bFlip     = HI_FALSE;
        stVpssChnAttr.bMirror   = HI_FALSE;
        stVpssChnAttr.s32SrcFrameRate = -1;
        stVpssChnAttr.s32DstFrameRate = -1;

        for (VpssChn = 0; VpssChn < u32VpssChnNum; VpssChn++)
        {
			if (VPSS_CHN1 == VpssChn)
            {
                stVpssChnMode.u32Width 	     = 720;
        		stVpssChnMode.u32Height 	 = 576;
            }
            s32Ret = HI_MPI_VPSS_SetChnMode(i, VpssChn, &stVpssChnMode);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("set vpss grp%d chn%d mode fail, s32Ret: 0x%x.\n", i, VpssChn, s32Ret);
                return s32Ret;
            }

            s32Ret = HI_MPI_VPSS_SetChnAttr(i, VpssChn, &stVpssChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("set vpss grp%d chn%d Attr fail, s32Ret: 0x%x.\n", i, VpssChn, s32Ret);
                return s32Ret;
            }

            s32Ret = HI_MPI_VPSS_EnableChn(i, VpssChn);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("create vpss grp%d chnl%d fail, s32Ret: 0x%x.\n", i, VpssChn, s32Ret);
                return s32Ret;
            }
        }

        s32Ret = HI_MPI_VPSS_StartGrp(i);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("start vpss grp%d fail! s32Ret: 0x%x.\n", i, s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;
}


HI_S32 SAMPLE_RGN_StopVpss(HI_S32 s32VpssGrpNum, HI_U32 u32VpssChnNum)
{
    HI_S32 i = 0;
    HI_S32 s32Ret;
    VPSS_CHN VpssChn;

    for (i = 0; i < s32VpssGrpNum; i++)
    {
        for (VpssChn = 0; VpssChn < u32VpssChnNum; VpssChn++)
        {
            s32Ret = HI_MPI_VPSS_DisableChn(i, VpssChn);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("disable vpss grp%d chnl%d fail, s32Ret: 0x%x.\n", i, VpssChn, s32Ret);
                return s32Ret;
            }
        }
        
        s32Ret =  HI_MPI_VPSS_StopGrp(i);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("stop vpss grp%d fail! s32Ret: 0x%x.\n", i, s32Ret);
            return s32Ret;
        }

        s32Ret =  HI_MPI_VPSS_DestroyGrp(i);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("destroy vpss grp%d fail! s32Ret: 0x%x.\n", i, s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_RGN_StartVo(VO_DEV VoDev, HI_U32 u32VoChnNum, VO_INTF_SYNC_E enIntfSync)
{
    HI_S32 i;
    HI_S32 s32Ret;
    HI_S32 s32DispNum;
    VO_LAYER VoLayer;
    VO_PUB_ATTR_S stPubAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    VO_CHN_ATTR_S astChnAttr[16];

    s32DispNum = SAMPLE_RGN_GetVoDisplayNum(u32VoChnNum);
    if (s32DispNum < 0)
    {
        SAMPLE_PRT("SAMPLE_RGN_GetVoDisplayNum failed! u32VoChnNum: %d.\n", u32VoChnNum);
        return HI_FAILURE;
    }

    s32Ret = SAMPLE_RGN_GetVoAttr(VoDev, enIntfSync, &stPubAttr, &stLayerAttr, s32DispNum, astChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_GetVoAttr failed!\n");
        return HI_FAILURE;
    }

    VoLayer = SAMPLE_RGN_GetVoLayer(VoDev);
    if (VoLayer < 0)
    {
        SAMPLE_PRT("SAMPLE_RGN_GetVoLayer failed! VoDev: %d.\n", VoDev);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VO_Disable(VoDev);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VO_Disable failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_VO_SetPubAttr(VoDev, &stPubAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VO_SetPubAttr failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_VO_Enable(VoDev);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VO_Enable failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_VO_SetVideoLayerAttr(VoLayer, &stLayerAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VO_SetVideoLayerAttr failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_VO_EnableVideoLayer(VoLayer);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VO_EnableVideoLayer failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    for (i = 0; i < u32VoChnNum; i++)
    {
        s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, i, &astChnAttr[i]);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VO_SetChnAttr failed! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }

        s32Ret = HI_MPI_VO_EnableChn(VoLayer, i);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VO_EnableChn failed! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_RGN_StopVoChn(VO_DEV VoDev, HI_U32 u32VoChnNum)
{
    HI_S32 i;
    HI_S32 s32Ret;
    VO_LAYER VoLayer;

    VoLayer = SAMPLE_RGN_GetVoLayer(VoDev);
    if (VoLayer < 0)
    {
        SAMPLE_PRT("SAMPLE_RGN_GetVoLayer failed! VoDev: %d.\n", VoDev);
        return HI_FAILURE;
    }

    for (i = 0; i < u32VoChnNum; i++)
    {
        s32Ret = HI_MPI_VO_DisableChn(VoLayer, i);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VO_DisableChn failed! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;
}


HI_S32 SAMPLE_RGN_StopVoDev(VO_DEV VoDev)
{
    HI_S32 s32Ret;
    VO_LAYER VoLayer;

    VoLayer = SAMPLE_RGN_GetVoLayer(VoDev);
    if (VoLayer < 0)
    {
        SAMPLE_PRT("SAMPLE_RGN_GetVoLayer failed! VoDev: %d.\n", VoDev);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VO_DisableVideoLayer(VoLayer);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VO_DisableVideoLayer failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_VO_Disable(VoDev);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VO_Disable failed! s32Ret:0x%x.\n", s32Ret);
        return s32Ret;
    }

    return 0;
}

/******************************************************************************
 * function : change coverex
 ******************************************************************************/
void* SAMPLE_RGN_ViCoverExDynamicDisplay(void* p)
{
    HI_S32 s32Ret;
    HI_U32 u32RgnNum;
    RGN_HANDLE Handle;
    RGN_HANDLE startHandle;
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    RGN_ATTR_INFO_S* pstRgnAttrInfo = NULL;

    stChn.enModId  = HI_ID_VIU;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;

    pstRgnAttrInfo = (RGN_ATTR_INFO_S*)p;
    startHandle    = pstRgnAttrInfo->Handle;
    u32RgnNum      = pstRgnAttrInfo->u32RgnNum;

    if (u32RgnNum > COVEREX_MAX_NUM_VI)
    {
        SAMPLE_PRT("coverex num(%d) is bigger than COVEREX_MAX_NUM_VI(%d)..\n", u32RgnNum, COVEREX_MAX_NUM_VI);
        return NULL;
    }

    while (HI_FALSE == bExit)
    {
        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = COVEREX_RGN;
            stChnAttr.unChnAttr.stCoverExChn.enCoverType = AREA_RECT;
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32X      = 32 * (Handle - startHandle);
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y      = 32 * (Handle - startHandle);
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = 64;
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width  = 64;
            stChnAttr.unChnAttr.stCoverExChn.u32Color         = 0x0000ffff;
            if (Handle % 2)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color = 0x00ff0000;
            }
            stChnAttr.unChnAttr.stCoverExChn.u32Layer     = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32X      = 32 * ((Handle - startHandle) + 1);
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y      = 32;
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = 64;
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width  = 64;
            stChnAttr.unChnAttr.stCoverExChn.u32Color         = 0x0000ffff;
            if (Handle % 2)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color = 0x00ff8000;
            }
            stChnAttr.unChnAttr.stCoverExChn.u32Layer     = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32X      = 60 * (Handle - startHandle);
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y      = 60 * (Handle - startHandle);
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = 64 * ((Handle - startHandle) + 1);
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width  = 64 * ((Handle - startHandle) + 1);
            stChnAttr.unChnAttr.stCoverExChn.u32Color         = 0x000000ff;
            if (1 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0x0000ff00;
            }
            else if (2 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0x00ff0000;
            }
            else if (3 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0x0080800;
            }
            stChnAttr.unChnAttr.stCoverExChn.u32Layer      = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32X      = 60 * ((Handle - startHandle) + 1);
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y      = 60;
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = 64 * ((Handle - startHandle) + 1);
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width  = 64 * ((Handle - startHandle) + 1);
            stChnAttr.unChnAttr.stCoverExChn.u32Color         = 0x000000ff;
            if (1 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0x0000ff00;
            }
            else if (2 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0x00ff0000;
            }
            else if (3 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0x0080800;
            }
            stChnAttr.unChnAttr.stCoverExChn.u32Layer      = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }

        sleep(2);
    }

    return (HI_VOID*)HI_SUCCESS;
}

/******************************************************************************
 * function : change OSD
 ******************************************************************************/
void* SAMPLE_RGN_ViOSdExDynamicDisplay(void* p)
{
    HI_S32 s32Ret;
    HI_U32 u32RgnNum;
    RGN_HANDLE Handle;
    RGN_HANDLE startHandle;
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    RGN_ATTR_INFO_S* pstRgnAttrInfo = NULL;

    stChn.enModId  = HI_ID_VIU;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;

    pstRgnAttrInfo = (RGN_ATTR_INFO_S*)p;
    startHandle    = pstRgnAttrInfo->Handle;
    u32RgnNum      = pstRgnAttrInfo->u32RgnNum;

    if (u32RgnNum > OVERLAYEX_MAX_NUM_VI)
    {
        SAMPLE_PRT("cover num(%d) is bigger than OVERLAY_MAX_NUM_VPSS(%d)..\n", u32RgnNum, OVERLAY_MAX_NUM_VI);
        return NULL;
    }

    while (HI_FALSE == bExit)
    {
        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAYEX_RGN;
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 80 * (Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 80 * (Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha   = (30 * (OVERLAYEX_MAX_NUM_VI - (Handle - startHandle))) % 256;
            stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha   = (30 * (OVERLAYEX_MAX_NUM_VI - (Handle - startHandle))) % 256;
            stChnAttr.unChnAttr.stOverlayExChn.u32Layer     = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAYEX_RGN;
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 50 * (Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 50 * (Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha   = (30 * (OVERLAYEX_MAX_NUM_VI - (Handle - startHandle))) % 256;
            stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha   = (30 * (OVERLAYEX_MAX_NUM_VI - (Handle - startHandle))) % 256;
            stChnAttr.unChnAttr.stOverlayExChn.u32Layer     = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAYEX_RGN;
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 60 * (Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 60 * (Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha   = (30 * (OVERLAYEX_MAX_NUM_VI - (Handle - startHandle))) % 256;
            stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha   = (30 * (OVERLAYEX_MAX_NUM_VI - (Handle - startHandle))) % 256;
            stChnAttr.unChnAttr.stOverlayExChn.u32Layer     = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAYEX_RGN;
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 100 * (Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 100 * (Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha   = (30 * (OVERLAYEX_MAX_NUM_VI - (Handle - startHandle))) % 256;
            stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha   = (30 * (OVERLAYEX_MAX_NUM_VI - (Handle - startHandle))) % 256;
            stChnAttr.unChnAttr.stOverlayExChn.u32Layer     = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }

        sleep(2);
    }

    return (HI_VOID*)HI_SUCCESS;
}

/******************************************************************************
 * function : change Mosaic
 ******************************************************************************/
void* SAMPLE_RGN_VpssMosaicDynamicDisplay(void* p)
{
    HI_S32 s32Ret;
    HI_U32 u32RgnNum;
    RGN_HANDLE Handle;
    RGN_HANDLE startHandle;
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    RGN_ATTR_INFO_S* pstRgnAttrInfo = NULL;
	HI_U32 u32Offset = 60;

    stChn.enModId  = HI_ID_VPSS;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;

    pstRgnAttrInfo = (RGN_ATTR_INFO_S*)p;
    startHandle    = pstRgnAttrInfo->Handle;
    u32RgnNum      = pstRgnAttrInfo->u32RgnNum;

    if (u32RgnNum > MOSAIC_MAX_NUM_VPSS)
    {
        SAMPLE_PRT("cover num(%d) is bigger than MOSAIC_MAX_NUM_VPSS(%d)..\n", u32RgnNum, MOSAIC_MAX_NUM_VPSS);
        return NULL;
    }

    while (HI_FALSE == bExit)
    {
        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = MOSAIC_RGN;
            stChnAttr.unChnAttr.stMosaicChn.enBlkSize = MOSAIC_BLK_SIZE_8 + (Handle - startHandle) % MOSAIC_BLK_SIZE_64;
            stChnAttr.unChnAttr.stMosaicChn.stRect.s32X      = 32 * (Handle - startHandle) + u32Offset;
            stChnAttr.unChnAttr.stMosaicChn.stRect.s32Y      = 32 * (Handle - startHandle) + u32Offset;
            stChnAttr.unChnAttr.stMosaicChn.stRect.u32Height = 64;
            stChnAttr.unChnAttr.stMosaicChn.stRect.u32Width  = 64;
            
            stChnAttr.unChnAttr.stMosaicChn.u32Layer     = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.unChnAttr.stMosaicChn.stRect.s32X      = 32 * ((Handle - startHandle) + 1) + u32Offset;
            stChnAttr.unChnAttr.stMosaicChn.stRect.s32Y      = 32 + u32Offset;
            stChnAttr.unChnAttr.stMosaicChn.stRect.u32Height = 64;
            stChnAttr.unChnAttr.stMosaicChn.stRect.u32Width  = 64;
            stChnAttr.unChnAttr.stMosaicChn.enBlkSize = MOSAIC_BLK_SIZE_32 - (Handle - startHandle) % MOSAIC_BLK_SIZE_64;
            
            stChnAttr.unChnAttr.stMosaicChn.u32Layer     = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.unChnAttr.stMosaicChn.stRect.s32X      = 60 * (Handle - startHandle) + u32Offset;
            stChnAttr.unChnAttr.stMosaicChn.stRect.s32Y      = 60 * (Handle - startHandle) + u32Offset;
            stChnAttr.unChnAttr.stMosaicChn.stRect.u32Height = 64 * ((Handle - startHandle) + 1);
            stChnAttr.unChnAttr.stMosaicChn.stRect.u32Width  = 64 * ((Handle - startHandle) + 1);
            stChnAttr.unChnAttr.stMosaicChn.enBlkSize = MOSAIC_BLK_SIZE_8 + (Handle - startHandle) % MOSAIC_BLK_SIZE_64;
            
            stChnAttr.unChnAttr.stMosaicChn.u32Layer      = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.unChnAttr.stMosaicChn.stRect.s32X      = 60 * ((Handle - startHandle) + 1) + u32Offset;
            stChnAttr.unChnAttr.stMosaicChn.stRect.s32Y      = 60 + u32Offset;
            stChnAttr.unChnAttr.stMosaicChn.stRect.u32Height = 64 * ((Handle - startHandle) + 1);
            stChnAttr.unChnAttr.stMosaicChn.stRect.u32Width  = 64 * ((Handle - startHandle) + 1);
            stChnAttr.unChnAttr.stMosaicChn.enBlkSize = MOSAIC_BLK_SIZE_32 - (Handle - startHandle) % MOSAIC_BLK_SIZE_64;
            
            stChnAttr.unChnAttr.stMosaicChn.u32Layer      = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }

        sleep(2);
    }

    return (HI_VOID*)HI_SUCCESS;
}

/******************************************************************************
 * function : change cover
 ******************************************************************************/
void* SAMPLE_RGN_VpssCoverDynamicDisplay(void* p)
{
    HI_S32 s32Ret;
    HI_U32 u32RgnNum;
    RGN_HANDLE Handle;
    RGN_HANDLE startHandle;
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    RGN_ATTR_INFO_S* pstRgnAttrInfo = NULL;

    stChn.enModId  = HI_ID_VPSS;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;

    pstRgnAttrInfo = (RGN_ATTR_INFO_S*)p;
    startHandle    = pstRgnAttrInfo->Handle;
    u32RgnNum      = pstRgnAttrInfo->u32RgnNum;

    if (u32RgnNum > COVER_MAX_NUM_VPSS)
    {
        SAMPLE_PRT("cover num(%d) is bigger than COVER_MAX_NUM_VPSS(%d)..\n", u32RgnNum, COVER_MAX_NUM_VPSS);
        return NULL;
    }

    while (HI_FALSE == bExit)
    {
        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = COVER_RGN;
            stChnAttr.unChnAttr.stCoverChn.enCoverType = AREA_RECT;
            stChnAttr.unChnAttr.stCoverChn.stRect.s32X      = 32 * (Handle - startHandle);
            stChnAttr.unChnAttr.stCoverChn.stRect.s32Y      = 32 * (Handle - startHandle);
            stChnAttr.unChnAttr.stCoverChn.stRect.u32Height = 64;
            stChnAttr.unChnAttr.stCoverChn.stRect.u32Width  = 64;
            stChnAttr.unChnAttr.stCoverChn.u32Color         = 0x0000ffff;
            if (Handle % 2)
            {
                stChnAttr.unChnAttr.stCoverChn.u32Color = 0x00ff0000;
            }
            stChnAttr.unChnAttr.stCoverChn.u32Layer     = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.unChnAttr.stCoverChn.stRect.s32X      = 32 * ((Handle - startHandle) + 1);
            stChnAttr.unChnAttr.stCoverChn.stRect.s32Y      = 32;
            stChnAttr.unChnAttr.stCoverChn.stRect.u32Height = 64;
            stChnAttr.unChnAttr.stCoverChn.stRect.u32Width  = 64;
            stChnAttr.unChnAttr.stCoverChn.u32Color         = 0x0000ffff;
            if (Handle % 2)
            {
                stChnAttr.unChnAttr.stCoverChn.u32Color = 0x00ff0000;
            }
            stChnAttr.unChnAttr.stCoverChn.u32Layer     = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.unChnAttr.stCoverChn.stRect.s32X      = 60 * (Handle - startHandle);
            stChnAttr.unChnAttr.stCoverChn.stRect.s32Y      = 60 * (Handle - startHandle);
            stChnAttr.unChnAttr.stCoverChn.stRect.u32Height = 64 * ((Handle - startHandle) + 1);
            stChnAttr.unChnAttr.stCoverChn.stRect.u32Width  = 64 * ((Handle - startHandle) + 1);
            stChnAttr.unChnAttr.stCoverChn.u32Color         = 0x000000ff;
            if (1 == Handle)
            {
                stChnAttr.unChnAttr.stCoverChn.u32Color  = 0x0000ff00;
            }
            else if (2 == Handle)
            {
                stChnAttr.unChnAttr.stCoverChn.u32Color  = 0x00ff0000;
            }
            else if (3 == Handle)
            {
                stChnAttr.unChnAttr.stCoverChn.u32Color  = 0x00808000;
            }
            stChnAttr.unChnAttr.stCoverChn.u32Layer      = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.unChnAttr.stCoverChn.stRect.s32X      = 60 * ((Handle - startHandle) + 1);
            stChnAttr.unChnAttr.stCoverChn.stRect.s32Y      = 60;
            stChnAttr.unChnAttr.stCoverChn.stRect.u32Height = 64 * ((Handle - startHandle) + 1);
            stChnAttr.unChnAttr.stCoverChn.stRect.u32Width  = 64 * ((Handle - startHandle) + 1);
            stChnAttr.unChnAttr.stCoverChn.u32Color         = 0x000000ff;
            if (1 == Handle)
            {
                stChnAttr.unChnAttr.stCoverChn.u32Color  = 0x0000ff00;
            }
            else if (2 == Handle)
            {
                stChnAttr.unChnAttr.stCoverChn.u32Color  = 0x00ff0000;
            }
            else if (3 == Handle)
            {
                stChnAttr.unChnAttr.stCoverChn.u32Color  = 0x00800080;
            }
            stChnAttr.unChnAttr.stCoverChn.u32Layer      = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }

        sleep(2);
    }

    return (HI_VOID*)HI_SUCCESS;
}

/******************************************************************************
 * function : change coverex
 ******************************************************************************/
void* SAMPLE_RGN_VpssCoverExDynamicDisplay(void* p)
{
    HI_S32 s32Ret;
    HI_U32 u32RgnNum;
    RGN_HANDLE Handle;
    RGN_HANDLE startHandle;
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    RGN_ATTR_INFO_S* pstRgnAttrInfo = NULL;

    stChn.enModId  = HI_ID_VPSS;
    stChn.s32DevId = 0;
    stChn.s32ChnId = VPSS_CHN0;

    pstRgnAttrInfo = (RGN_ATTR_INFO_S*)p;
    startHandle    = pstRgnAttrInfo->Handle;
    u32RgnNum      = pstRgnAttrInfo->u32RgnNum;

    if (u32RgnNum > COVEREX_MAX_NUM_VPSS)
    {
        SAMPLE_PRT("coverex num(%d) is bigger than COVEREX_MAX_NUM_VPSS(%d)..\n", u32RgnNum, COVEREX_MAX_NUM_VPSS);
        return NULL;
    }

    while (HI_FALSE == bExit)
    {
        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = COVEREX_RGN;
            stChnAttr.unChnAttr.stCoverExChn.enCoverType = AREA_RECT;
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32X      = 32 * (Handle - startHandle);
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y      = 32 * (Handle - startHandle);
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = 64;
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width  = 64;
            stChnAttr.unChnAttr.stCoverExChn.u32Color         = 0x0000ffff;
            if (Handle % 2)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color = 0x00ff0000;
            }
            stChnAttr.unChnAttr.stCoverExChn.u32Layer     = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32X      = 32 * ((Handle - startHandle) + 1);
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y      = 32;
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = 64;
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width  = 64;
            stChnAttr.unChnAttr.stCoverExChn.u32Color         = 0x0000ffff;
            if (Handle % 2)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color = 0x00ff8000;
            }
            stChnAttr.unChnAttr.stCoverExChn.u32Layer     = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32X      = 60 * (Handle - startHandle);
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y      = 60 * (Handle - startHandle);
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = 64 * ((Handle - startHandle) + 1);
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width  = 64 * ((Handle - startHandle) + 1);
            stChnAttr.unChnAttr.stCoverExChn.u32Color         = 0x000000ff;
            if (1 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0x0000ff00;
            }
            else if (2 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0x00ff0000;
            }
            else if (3 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0x0080800;
            }
            stChnAttr.unChnAttr.stCoverExChn.u32Layer      = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32X      = 60 * ((Handle - startHandle) + 1);
            stChnAttr.unChnAttr.stCoverExChn.stRect.s32Y      = 60;
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = 64 * ((Handle - startHandle) + 1);
            stChnAttr.unChnAttr.stCoverExChn.stRect.u32Width  = 64 * ((Handle - startHandle) + 1);
            stChnAttr.unChnAttr.stCoverExChn.u32Color         = 0x000000ff;
            if (1 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0x0000ff00;
            }
            else if (2 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0x00ff0000;
            }
            else if (3 == Handle)
            {
                stChnAttr.unChnAttr.stCoverExChn.u32Color  = 0x0080800;
            }
            stChnAttr.unChnAttr.stCoverExChn.u32Layer      = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }

        sleep(2);
    }

    return (HI_VOID*)HI_SUCCESS;
}


/******************************************************************************
 * function : change OSD
 ******************************************************************************/
void* SAMPLE_RGN_VpssOSdDynamicDisplay(void* p)
{
    HI_S32 s32Ret;
    HI_U32 u32RgnNum;
    RGN_HANDLE Handle;
    RGN_HANDLE startHandle;
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    RGN_ATTR_INFO_S* pstRgnAttrInfo = NULL;

    stChn.enModId  = HI_ID_VPSS;
    stChn.s32DevId = 0;
    stChn.s32ChnId = VPSS_CHN0;

    pstRgnAttrInfo = (RGN_ATTR_INFO_S*)p;
    startHandle    = pstRgnAttrInfo->Handle;
    u32RgnNum      = pstRgnAttrInfo->u32RgnNum;

    if (u32RgnNum > OVERLAYEX_MAX_NUM_VPSS)
    {
        SAMPLE_PRT("cover num(%d) is bigger than OVERLAYEX_MAX_NUM_VPSS(%d)..\n", u32RgnNum, OVERLAYEX_MAX_NUM_VPSS);
        return NULL;
    }

    while (HI_FALSE == bExit)
    {
        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAYEX_RGN;
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 80 * (Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 80 * (Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha   = 30 * (OVERLAYEX_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha   = 30 * (OVERLAYEX_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32Layer     = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAYEX_RGN;
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 50 * (Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 50 * (Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha   = 30 * (OVERLAYEX_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha   = 30 * (OVERLAYEX_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32Layer     = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAYEX_RGN;
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 60 * (Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 60 * (Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha   = 30 * (OVERLAYEX_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha   = 30 * (OVERLAYEX_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32Layer     = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);

        for (Handle = startHandle; Handle < (startHandle + u32RgnNum); Handle++)
        {
            stChnAttr.bShow  = HI_TRUE;
            stChnAttr.enType = OVERLAYEX_RGN;
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 100 * (Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 100 * (Handle - startHandle);
            stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha   = 30 * (OVERLAYEX_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha   = 30 * (OVERLAYEX_MAX_NUM_VPSS - (Handle - startHandle));
            stChnAttr.unChnAttr.stOverlayExChn.u32Layer     = Handle - startHandle;

            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }

        sleep(2);
    }

    return (HI_VOID*)HI_SUCCESS;
}


/******************************************************************************
 * function : change OSD
 ******************************************************************************/
void* SAMPLE_RGN_VencOSdDynamicDisplay(void* p)
{
    HI_S32 s32Ret;
    RGN_HANDLE Handle;
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr;
    RGN_ATTR_S stRegion;

    stChn.enModId  = HI_ID_VENC;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;


    while (HI_FALSE == bExit)
    {
        /* switch background color and move dynamic */
        for (Handle = 0; Handle < 4; Handle++)
        {
            s32Ret = HI_MPI_RGN_GetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }

            s32Ret = HI_MPI_RGN_GetAttr(Handle, &stRegion);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
            
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 720 - stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X;
            if ((stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X + stRegion.unAttr.stOverlay.stSize.u32Width) > 720)
                stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 720 - stRegion.unAttr.stOverlay.stSize.u32Width;
            
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 576 - stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y;
            if ((stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y + stRegion.unAttr.stOverlay.stSize.u32Height) > 576)
                stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 576 - stRegion.unAttr.stOverlay.stSize.u32Height;
            
            s32Ret = HI_MPI_RGN_SetDisplayAttr(Handle, &stChn, &stChnAttr);
            if (HI_SUCCESS != s32Ret)
            {
                SAMPLE_RGN_NOT_PASS(s32Ret);
            }
        }
        sleep(3);
    }

    return (HI_VOID*)HI_SUCCESS;
}


HI_VOID SAMPLE_RGN_WriteStreamToFile( FILE* fp, VENC_PACK_S* pstNalu )
{
    HI_U8* p;
    HI_U32 u32Len;

    p = (HI_U8*) pstNalu->pu8Addr + pstNalu->u32Offset;

    u32Len = pstNalu->u32Len - pstNalu->u32Offset;

    fwrite(p, u32Len, sizeof(HI_U8), fp);

    fflush(fp);
}

HI_S32 SAMPLE_RGN_DestroyRegion(RGN_HANDLE Handle, HI_U32 u32Num)
{
    HI_S32 i;
    HI_S32 s32Ret;

    for (i = Handle; i < (Handle + u32Num); i++)
    {
        s32Ret = HI_MPI_RGN_Destroy(i);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_RGN_Destroy failed! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;

}

HI_VOID* SAMPLE_RGN_VpssOsdReverse(void* pData)
{
    HI_S32 i = 0, j = 0;
    RGN_HANDLE RgnHandle;
    //TDE2_SURFACE_S stRgnOrignSurface = {0};
    //TDE2_SURFACE_S stRgnSurface = {0};
    RGN_CANVAS_INFO_S stCanvasInfoOri;
    RGN_CANVAS_INFO_S stCanvasInfo;
    VGS_HANDLE VgsHandle = -1;
    //TDE2_RECT_S stRect = {0};
    VPSS_REGION_INFO_S stReverseRgnInfo;
    HI_U32 au32LumaData[OSD_REVERSE_RGN_MAXCNT];
    RECT_S astOsdRevRect[OSD_REVERSE_RGN_MAXCNT];
    RGN_OSD_REVERSE_INFO_S* pstOsdReverseInfo;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32* pu32VirAddr = NULL;
    MPP_CHN_S stMppChn =  {0};
    RGN_CHN_ATTR_S stOsdChnAttr = {0};
	VGS_TASK_ATTR_S stVgsTask;
	VGS_ADD_QUICK_COPY_S stAddQuickCopy;

    HI_ASSERT(NULL != pData);
    pstOsdReverseInfo = (RGN_OSD_REVERSE_INFO_S*)pData;
    RgnHandle = pstOsdReverseInfo->Handle;
    HI_ASSERT(OSD_REVERSE_RGN_MAXCNT >= pstOsdReverseInfo->stLumaRgnInfo.u32RegionNum);

    srand(time(NULL));

    /* 1.get current osd info */
    s32Ret = HI_MPI_RGN_GetCanvasInfo(RgnHandle, &stCanvasInfo);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_RGN_GetCanvasInfo fail! s32Ret: 0x%x.\n", s32Ret);
        return NULL;
    }

    s32Ret = HI_MPI_RGN_UpdateCanvas(RgnHandle);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_RGN_UpdateCanvas fail! s32Ret: 0x%x.\n", s32Ret);
        return NULL;
    }

    /* 2.make a backup of current osd */
	#if 0
    s32Ret = SAMPLE_RGN_ConvOsdCavasToTdeSurface(&stRgnSurface, &stCanvasInfo);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Func: %s, line: %d! s32Ret: 0x%x.\n", __FUNCTION__, __LINE__, s32Ret);
        return NULL;
    }
	#endif

    memcpy(&stCanvasInfoOri, &stCanvasInfo, sizeof(RGN_CANVAS_INFO_S));

    s32Ret = HI_MPI_SYS_MmzAlloc(&stCanvasInfoOri.u32PhyAddr, (void**)(&pu32VirAddr),
                                 NULL, NULL, stCanvasInfoOri.u32Stride * stCanvasInfoOri.stSize.u32Height);
    if (HI_SUCCESS != s32Ret)
    {
        HI_MPI_SYS_MmzFree(stCanvasInfoOri.u32PhyAddr, pu32VirAddr);
        return NULL;
    }

    s32Ret = HI_MPI_VGS_BeginJob(&VgsHandle);
    if (HI_SUCCESS != s32Ret)
    {
        HI_MPI_SYS_MmzFree(stCanvasInfoOri.u32PhyAddr, pu32VirAddr);
        return NULL;
    }

    stVgsTask.stImgIn.stVFrame.u32Width = stCanvasInfo.stSize.u32Width;
	stVgsTask.stImgIn.stVFrame.u32Height= stCanvasInfo.stSize.u32Height;
	stVgsTask.stImgIn.stVFrame.u32Stride[0] = stCanvasInfo.u32Stride;
	stVgsTask.stImgIn.stVFrame.u32PhyAddr[0] = stCanvasInfo.u32PhyAddr;
	stVgsTask.stImgIn.stVFrame.pVirAddr[0] = (HI_VOID *)stCanvasInfo.u32VirtAddr;
	stVgsTask.stImgIn.stVFrame.enPixelFormat = stCanvasInfo.enPixelFmt;

	stVgsTask.stImgOut.stVFrame.u32Width = stCanvasInfoOri.stSize.u32Width;
	stVgsTask.stImgOut.stVFrame.u32Height= stCanvasInfoOri.stSize.u32Height;
	stVgsTask.stImgOut.stVFrame.u32Stride[0] = stCanvasInfoOri.u32Stride;
	stVgsTask.stImgOut.stVFrame.u32PhyAddr[0] = stCanvasInfoOri.u32PhyAddr;
	stVgsTask.stImgOut.stVFrame.pVirAddr[0] = (HI_VOID *)stCanvasInfoOri.u32VirtAddr;
	stVgsTask.stImgOut.stVFrame.enPixelFormat = stCanvasInfoOri.enPixelFmt;

	stAddQuickCopy.stSrcRect.s32X = 0;
	stAddQuickCopy.stSrcRect.s32Y = 0;
	stAddQuickCopy.stSrcRect.u32Width = stVgsTask.stImgIn.stVFrame.u32Width;
	stAddQuickCopy.stSrcRect.u32Height = stVgsTask.stImgIn.stVFrame.u32Height;

	stAddQuickCopy.stDestRect.s32X = 0;
	stAddQuickCopy.stDestRect.s32Y = 0;
	stAddQuickCopy.stDestRect.u32Width = stVgsTask.stImgOut.stVFrame.u32Width;
	stAddQuickCopy.stDestRect.u32Height = stVgsTask.stImgOut.stVFrame.u32Height;
    s32Ret = HI_MPI_VGS_AddQuickCopyTask(VgsHandle, &stVgsTask, &stAddQuickCopy);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("[Func]:%s [Line]:%d [Info]:HI_MPI_VGS_AddQuickCopyTask failed\n", __FUNCTION__, __LINE__);
        HI_MPI_VGS_CancelJob(VgsHandle);
        HI_MPI_SYS_MmzFree(stCanvasInfoOri.u32PhyAddr, pu32VirAddr);
        return NULL;
    }

    s32Ret = HI_MPI_VGS_EndJob(VgsHandle);
    if (HI_SUCCESS != s32Ret)
    {
        HI_MPI_VGS_CancelJob(VgsHandle);
        HI_MPI_SYS_MmzFree(stCanvasInfoOri.u32PhyAddr, pu32VirAddr);
        return NULL;
    }

    /* 3.get the  display attribute of OSD attached to vpss*/
    stMppChn.enModId  = HI_ID_VPSS;
    stMppChn.s32DevId = pstOsdReverseInfo->VpssGrp;
    stMppChn.s32ChnId = pstOsdReverseInfo->VpssChn;
    s32Ret = HI_MPI_RGN_GetDisplayAttr(RgnHandle, &stMppChn, &stOsdChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        HI_MPI_SYS_MmzFree(stCanvasInfoOri.u32PhyAddr, pu32VirAddr);
        return NULL;
    }

    stReverseRgnInfo.pstRegion = (RECT_S*)astOsdRevRect;
    while (HI_FALSE == bExit)
    {
        /* 4.get the sum of luma of a region specified by user*/
        s32Ret = HI_MPI_VPSS_GetRegionLuma(pstOsdReverseInfo->VpssGrp, pstOsdReverseInfo->VpssChn, &(pstOsdReverseInfo->stLumaRgnInfo), au32LumaData, -1);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("[Func]:%s [Line]:%d [Info]:HI_MPI_VPSS_GetRegionLuma VpssGrp=%d failed, s32Ret: 0x%x.\n",
                   __FUNCTION__, __LINE__, pstOsdReverseInfo->VpssGrp, s32Ret);
            continue ;
        }

        /* 5.decide which region to be reverse color according to the sum of the region*/
        for (i = 0, j = 0; i < pstOsdReverseInfo->stLumaRgnInfo.u32RegionNum; ++i)
        {
            pstOsdReverseInfo->u8PerPixelLumaThrd = rand() % 256;

            if (au32LumaData[i] > (pstOsdReverseInfo->u8PerPixelLumaThrd *
                                   pstOsdReverseInfo->stLumaRgnInfo.pstRegion[i].u32Width *
                                   pstOsdReverseInfo->stLumaRgnInfo.pstRegion[i].u32Height))
            {
                /* 6.get the regions to be reverse color */
                stReverseRgnInfo.pstRegion[j].s32X = pstOsdReverseInfo->stLumaRgnInfo.pstRegion[i].s32X
                                                     - stOsdChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X;
                stReverseRgnInfo.pstRegion[j].s32Y = pstOsdReverseInfo->stLumaRgnInfo.pstRegion[i].s32Y
                                                     - stOsdChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y;
                stReverseRgnInfo.pstRegion[j].u32Width = pstOsdReverseInfo->stLumaRgnInfo.pstRegion[i].u32Width;
                stReverseRgnInfo.pstRegion[j].u32Height = pstOsdReverseInfo->stLumaRgnInfo.pstRegion[i].u32Height;
                ++j;
            }
        }

        stReverseRgnInfo.u32RegionNum = j;

        /* 7.the the canvas to be update */
        if (HI_SUCCESS != HI_MPI_RGN_GetCanvasInfo(RgnHandle, &stCanvasInfo))
        {
            SAMPLE_PRT("[Info]:HI_MPI_RGN_GetCanvasInfo failed\n");
            HI_MPI_SYS_MmzFree(stCanvasInfoOri.u32PhyAddr, pu32VirAddr);
            return NULL;
        }
		#if 0
        if (HI_SUCCESS != SAMPLE_RGN_ConvOsdCavasToTdeSurface(&stRgnSurface, &stCanvasInfo))
        {
            SAMPLE_PRT("[Func]:%s [Line]:%d [Info]:SAMPLE_RGN_ConvOsdCavasToTdeSurface failed\n", __FUNCTION__, __LINE__);
            HI_MPI_SYS_MmzFree(stRgnOrignSurface.u32PhyAddr, pu32VirAddr);
            return NULL;
        }
		#endif
        /* 8.reverse color */
        if (HI_SUCCESS != SAMPLE_RGN_ReverseOsdColor(&stCanvasInfoOri, &stCanvasInfo, &stReverseRgnInfo))
        {
            SAMPLE_PRT("[Info]:SAMPLE_RGN_ReverseOsdColor failed\n");
            if (HI_SUCCESS != HI_MPI_RGN_UpdateCanvas(RgnHandle))
	        {
	            SAMPLE_PRT("[Info]:HI_MPI_RGN_UpdateCanvas failed\n");
	        }
			
            HI_MPI_SYS_MmzFree(stCanvasInfoOri.u32PhyAddr, pu32VirAddr);
            return NULL;
        }

        /* 9.update OSD */
        if (HI_SUCCESS != HI_MPI_RGN_UpdateCanvas(RgnHandle))
        {
            SAMPLE_PRT("[Info]:HI_MPI_RGN_UpdateCanvas failed\n");
            HI_MPI_SYS_MmzFree(stCanvasInfoOri.u32PhyAddr, pu32VirAddr);
            return NULL;
        }

        sleep(2);
    }

    HI_MPI_SYS_MmzFree(stCanvasInfoOri.u32PhyAddr, pu32VirAddr);
    return HI_NULL;

}

HI_VOID* SAMPLE_RGN_VencOsdReverse(void* pData)
{
    HI_S32 i = 0, j = 0;
    RGN_HANDLE RgnHandle;
    //TDE2_SURFACE_S stRgnOrignSurface = {0};
    //TDE2_SURFACE_S stRgnSurface = {0};
    RGN_CANVAS_INFO_S stCanvasInfo;
	RGN_CANVAS_INFO_S stCanvasInfoOri;
    VGS_HANDLE VgsHandle = -1;
    //TDE2_RECT_S stRect = {0};
    VPSS_REGION_INFO_S stReverseRgnInfo;
    HI_U32 au32LumaData[OSD_REVERSE_RGN_MAXCNT];
    RECT_S astOsdRevRect[OSD_REVERSE_RGN_MAXCNT];
    RGN_OSD_REVERSE_INFO_S* pstOsdReverseInfo;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32* pu32VirAddr = NULL;
    MPP_CHN_S stMppChn =  {0};
    RGN_CHN_ATTR_S stOsdChnAttr = {0};

	VGS_TASK_ATTR_S stVgsTask;
	VGS_ADD_QUICK_COPY_S stAddQuickCopy;

    HI_ASSERT(NULL != pData);
    pstOsdReverseInfo = (RGN_OSD_REVERSE_INFO_S*)pData;
    RgnHandle = pstOsdReverseInfo->Handle;
    HI_ASSERT(OSD_REVERSE_RGN_MAXCNT >= pstOsdReverseInfo->stLumaRgnInfo.u32RegionNum);

    srand(time(NULL));

    /* 1.get current osd info */
    s32Ret = HI_MPI_RGN_GetCanvasInfo(RgnHandle, &stCanvasInfo);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_RGN_GetCanvasInfo fail! s32Ret: 0x%x.\n", s32Ret);
        return NULL;
    }

    s32Ret = HI_MPI_RGN_UpdateCanvas(RgnHandle);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_RGN_UpdateCanvas fail! s32Ret: 0x%x.\n", s32Ret);
        return NULL;
    }

    /* 2.make a backup of current osd */
	#if 0
    s32Ret = SAMPLE_RGN_ConvOsdCavasToTdeSurface(&stRgnSurface, &stCanvasInfo);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Func: %s, line: %d! s32Ret: 0x%x.\n", __FUNCTION__, __LINE__, s32Ret);
        return NULL;
    }
	#endif

    memcpy(&stCanvasInfoOri, &stCanvasInfo, sizeof(stCanvasInfo));

    s32Ret = HI_MPI_SYS_MmzAlloc(&stCanvasInfoOri.u32PhyAddr, (void**)(&pu32VirAddr),
                                 NULL, NULL, stCanvasInfoOri.u32Stride * stCanvasInfoOri.stSize.u32Height);
    if (HI_SUCCESS != s32Ret)
    {
        HI_MPI_SYS_MmzFree(stCanvasInfoOri.u32PhyAddr, pu32VirAddr);
        return NULL;
    }

    s32Ret = HI_MPI_VGS_BeginJob(&VgsHandle);
    if (HI_SUCCESS != s32Ret)
    {
        HI_MPI_SYS_MmzFree(stCanvasInfoOri.u32PhyAddr, pu32VirAddr);
        return NULL;
    }

    stVgsTask.stImgIn.stVFrame.u32Width = stCanvasInfo.stSize.u32Width;
	stVgsTask.stImgIn.stVFrame.u32Height= stCanvasInfo.stSize.u32Height;
	stVgsTask.stImgIn.stVFrame.u32Stride[0] = stCanvasInfo.u32Stride;
	stVgsTask.stImgIn.stVFrame.u32PhyAddr[0] = stCanvasInfo.u32PhyAddr;
	stVgsTask.stImgIn.stVFrame.pVirAddr[0] = (HI_VOID *)stCanvasInfo.u32VirtAddr;
	stVgsTask.stImgIn.stVFrame.enPixelFormat = stCanvasInfo.enPixelFmt;

	stVgsTask.stImgOut.stVFrame.u32Width = stCanvasInfoOri.stSize.u32Width;
	stVgsTask.stImgOut.stVFrame.u32Height= stCanvasInfoOri.stSize.u32Height;
	stVgsTask.stImgOut.stVFrame.u32Stride[0] = stCanvasInfoOri.u32Stride;
	stVgsTask.stImgOut.stVFrame.u32PhyAddr[0] = stCanvasInfoOri.u32PhyAddr;
	stVgsTask.stImgOut.stVFrame.pVirAddr[0] = (HI_VOID *)stCanvasInfoOri.u32VirtAddr;
	stVgsTask.stImgOut.stVFrame.enPixelFormat = stCanvasInfoOri.enPixelFmt;

	stAddQuickCopy.stSrcRect.s32X = 0;
	stAddQuickCopy.stSrcRect.s32Y = 0;
	stAddQuickCopy.stSrcRect.u32Width = stVgsTask.stImgIn.stVFrame.u32Width;
	stAddQuickCopy.stSrcRect.u32Height = stVgsTask.stImgIn.stVFrame.u32Height;

	stAddQuickCopy.stDestRect.s32X = 0;
	stAddQuickCopy.stDestRect.s32Y = 0;
	stAddQuickCopy.stDestRect.u32Width = stVgsTask.stImgOut.stVFrame.u32Width;
	stAddQuickCopy.stDestRect.u32Height = stVgsTask.stImgOut.stVFrame.u32Height;
    s32Ret = HI_MPI_VGS_AddQuickCopyTask(VgsHandle, &stVgsTask, &stAddQuickCopy);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("[Info]:HI_MPI_VGS_AddQuickCopyTask failed\n");
        HI_MPI_VGS_CancelJob(VgsHandle);
        HI_MPI_SYS_MmzFree(stCanvasInfoOri.u32PhyAddr, pu32VirAddr);
        return NULL;
    }

    s32Ret = HI_MPI_VGS_EndJob(VgsHandle);
    if (HI_SUCCESS != s32Ret)
    {
        HI_MPI_VGS_CancelJob(VgsHandle);
        HI_MPI_SYS_MmzFree(stCanvasInfoOri.u32PhyAddr, pu32VirAddr);
        return NULL;
    }

    /* 3.get the  display attribute of OSD attached to venc*/
    stMppChn.enModId  = HI_ID_VENC;
    stMppChn.s32DevId = 0;
    stMppChn.s32ChnId = 0;
    s32Ret = HI_MPI_RGN_GetDisplayAttr(RgnHandle, &stMppChn, &stOsdChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        HI_MPI_SYS_MmzFree(stCanvasInfoOri.u32PhyAddr, pu32VirAddr);
        return NULL;
    }

    stReverseRgnInfo.pstRegion = (RECT_S*)astOsdRevRect;
    while (HI_FALSE == bExit)
    {
        /* 4.get the sum of luma of a region specified by user*/
        s32Ret = HI_MPI_VPSS_GetRegionLuma(pstOsdReverseInfo->VpssGrp, pstOsdReverseInfo->VpssChn, &(pstOsdReverseInfo->stLumaRgnInfo), au32LumaData, -1);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("[Info]:HI_MPI_VPSS_GetRegionLuma VpssGrp=%d failed, s32Ret: 0x%x.\n",
                   pstOsdReverseInfo->VpssGrp, s32Ret);
            continue ;
        }

        /* 5.decide which region to be reverse color according to the sum of the region*/
        for (i = 0, j = 0; i < pstOsdReverseInfo->stLumaRgnInfo.u32RegionNum; ++i)
        {
            pstOsdReverseInfo->u8PerPixelLumaThrd = rand() % 256;

            if (au32LumaData[i] > (pstOsdReverseInfo->u8PerPixelLumaThrd *
                                   pstOsdReverseInfo->stLumaRgnInfo.pstRegion[i].u32Width *
                                   pstOsdReverseInfo->stLumaRgnInfo.pstRegion[i].u32Height))
            {
                /* 6.get the regions to be reverse color */
                stReverseRgnInfo.pstRegion[j].s32X = pstOsdReverseInfo->stLumaRgnInfo.pstRegion[i].s32X
                                                     - stOsdChnAttr.unChnAttr.stOverlayChn.stPoint.s32X;
                stReverseRgnInfo.pstRegion[j].s32Y = pstOsdReverseInfo->stLumaRgnInfo.pstRegion[i].s32Y
                                                     - stOsdChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y;
                stReverseRgnInfo.pstRegion[j].u32Width = pstOsdReverseInfo->stLumaRgnInfo.pstRegion[i].u32Width;
                stReverseRgnInfo.pstRegion[j].u32Height = pstOsdReverseInfo->stLumaRgnInfo.pstRegion[i].u32Height;
                ++j;
            }
        }

        stReverseRgnInfo.u32RegionNum = j;

        /* 7.the the canvas to be update */
        if (HI_SUCCESS != HI_MPI_RGN_GetCanvasInfo(RgnHandle, &stCanvasInfo))
        {
            SAMPLE_PRT("[Info]:HI_MPI_RGN_GetCanvasInfo failed\n");
            HI_MPI_SYS_MmzFree(stCanvasInfoOri.u32PhyAddr, pu32VirAddr);
            return NULL;
        }
		#if 0
        if (HI_SUCCESS != SAMPLE_RGN_ConvOsdCavasToTdeSurface(&stCanvasInfoOri, &stCanvasInfo))
        {
            printf("[Func]:%s [Line]:%d [Info]:SAMPLE_RGN_ConvOsdCavasToTdeSurface failed\n", __FUNCTION__, __LINE__);
            HI_MPI_SYS_MmzFree(stCanvasInfoOri.u32PhyAddr, pu32VirAddr);
            return NULL;
        }
		#endif
        /* 8.reverse color */
        if (HI_SUCCESS != SAMPLE_RGN_ReverseOsdColor(&stCanvasInfoOri, &stCanvasInfo, &stReverseRgnInfo))
        {
            SAMPLE_PRT("[Info]:SAMPLE_RGN_ReverseOsdColor failed\n");
            s32Ret = HI_MPI_RGN_UpdateCanvas(RgnHandle);
			if (HI_SUCCESS != s32Ret)
	        {
	            SAMPLE_PRT("HI_MPI_RGN_UpdateCanvas failed! s32Ret: 0x%x.\n", s32Ret);
	        }
            HI_MPI_SYS_MmzFree(stCanvasInfoOri.u32PhyAddr, pu32VirAddr);
            return NULL;
        }

        /* 9.update OSD */
        if (HI_SUCCESS != HI_MPI_RGN_UpdateCanvas(RgnHandle))
        {
            SAMPLE_PRT("[Info]:HI_MPI_RGN_UpdateCanvas failed\n");
            HI_MPI_SYS_MmzFree(stCanvasInfoOri.u32PhyAddr, pu32VirAddr);
            return NULL;
        }

        sleep(2);
    }

    HI_MPI_SYS_MmzFree(stCanvasInfoOri.u32PhyAddr, pu32VirAddr);
    return HI_NULL;

}


/******************************************************************************************
    function :  Vpss cover mosaic
    process steps:
    0) create 8 cover and 4 mosaic
    1) attach them to vpss
    2) change Regions' Layer
    3) change Regions' position
    4) change Regions' alpha (front and backgroud)
******************************************************************************************/
HI_S32 SAMPLE_RGN_AddCoverAndMosaicToVpss(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    RGN_HANDLE Handle;
    HI_U32 u32RgnNumCover;
    HI_U32 u32RgnNumMosaic;
    MPP_CHN_S stSrcChn, stDesChn;
    HI_S32 s32VpssGrpNum;
    HI_U32 u32VpssChnNum = 1;//VPSS_MAX_PHY_CHN_NUM;
    VPSS_CHN VpssChn = VPSS_CHN0;
    VO_DEV VoDev;
    VO_LAYER VoLayer;
    HI_U32 s32VoChnNum;
    VO_INTF_SYNC_E enIntfSync = VO_OUTPUT_1080P30;
    RGN_ATTR_INFO_S stRgnAttrInfoCover;
	RGN_ATTR_INFO_S stRgnAttrInfoMosaic;

    s32Ret = SAMPLE_RGN_SYS_Init();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_SYS_Init failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit0;
    }

    /*************************************************
      step 1: create region and attach to vpss group
     *************************************************/
    Handle    = 0;
    u32RgnNumCover = 8;
    s32Ret = SAMPLE_RGN_CreateCover(Handle, u32RgnNumCover);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_CreateCover failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit1;
    }
    u32RgnNumMosaic = 4;
    s32Ret = SAMPLE_RGN_CreateMosaic(Handle + u32RgnNumCover, u32RgnNumMosaic);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_CreateMosaic failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit1;
    }

    /*************************************************
      step 2: start vi chn
     *************************************************/
    s32Ret = SAMPLE_COMM_VI_StartVi(&g_stViChnConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_StartVi failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit2;
    }
    /*************************************************
      step 3: start vpss group and chn
     *************************************************/
    s32VpssGrpNum = 1;
    if (VO_INTF_BT1120 == g_enVoIntfType)
    {
        s32Ret = SAMPLE_RGN_StartVpssHD(s32VpssGrpNum, u32VpssChnNum);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_RGN_StartVpssHD failed! s32Ret: 0x%x.\n", s32Ret);
            goto exit3;
        }
    }
    else if (VO_INTF_CVBS == g_enVoIntfType)
    {
        s32Ret = SAMPLE_RGN_StartVpss(s32VpssGrpNum, u32VpssChnNum);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_RGN_StartVpss failed! s32Ret: 0x%x.\n", s32Ret);
            goto exit3;
        }
    }

    /*************************************************
      step 4: start vo dev and chn
     *************************************************/
    VoDev       = 0;
    s32VoChnNum = 1;
    if (VO_INTF_BT1120 == g_enVoIntfType)
    {
        enIntfSync  = VO_OUTPUT_1080P30;
    }
    else if (VO_INTF_CVBS == g_enVoIntfType)
    {
        enIntfSync  = VO_OUTPUT_PAL;
    }
    s32Ret = SAMPLE_RGN_StartVo(VoDev, s32VoChnNum, enIntfSync);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_StartVo failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit4;
    }

    /*************************************************
      step 5: bind vi and vpss
     *************************************************/
    stSrcChn.enModId  = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDesChn.enModId  = HI_ID_VPSS;
    stDesChn.s32DevId = 0;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDesChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_Bind failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit5;
    }

    /*************************************************
      step 6: bind vpss and vo
     *************************************************/
    VoLayer = SAMPLE_RGN_GetVoLayer(VoDev);
    if (VoLayer < 0)
    {
        SAMPLE_PRT("SAMPLE_RGN_GetVoLayer failed! VoDev: %d.\n", VoDev);
        goto exit5;
    }

    stSrcChn.enModId  = HI_ID_VPSS;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VpssChn;

    stDesChn.enModId  = HI_ID_VOU;
    stDesChn.s32DevId = VoLayer;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDesChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_Bind failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit5;
    }

    /*************************************************
      step 7: create a thread to change region's
      layer, position, alpha and other display attribute
     *************************************************/
    stRgnAttrInfoCover.Handle    = Handle;
    stRgnAttrInfoCover.u32RgnNum = u32RgnNumCover;
    pthread_create(&g_stRgnThread, NULL, SAMPLE_RGN_VpssCoverDynamicDisplay, (HI_VOID*)&stRgnAttrInfoCover);

	stRgnAttrInfoMosaic.Handle    = Handle + u32RgnNumCover;
    stRgnAttrInfoMosaic.u32RgnNum = u32RgnNumMosaic;
    pthread_create(&g_stVpssRgnThread, NULL, SAMPLE_RGN_VpssMosaicDynamicDisplay, (HI_VOID*)&stRgnAttrInfoMosaic);

    printf("\n#############Sample start ok! Press Enter to switch!#############\n");


    /*************************************************
      step 8: stop thread and release all the resource
     *************************************************/
    getchar();
    bExit = HI_TRUE;

    if (g_stRgnThread)
    {
        pthread_join(g_stRgnThread, 0);
        g_stRgnThread = 0;
    }

	if (g_stVpssRgnThread)
    {
        pthread_join(g_stVpssRgnThread, 0);
        g_stVpssRgnThread = 0;
    }
    
    bExit = HI_FALSE;
    

    /*************************************************
      step 9: unbind vpss and vo
     *************************************************/
exit5:
    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_UnBind failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 10: unbind vi and vpss
     *************************************************/
    stSrcChn.enModId  = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDesChn.enModId  = HI_ID_VPSS;
    stDesChn.s32DevId = 0;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_UnBind failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 11: stop vo dev and chn
     *************************************************/
exit4:
    s32Ret = SAMPLE_RGN_StopVoChn(VoDev, s32VoChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_StopVoChn failed! s32Ret: 0x%x.\n", s32Ret);
    }

    s32Ret = SAMPLE_RGN_StopVoDev(VoDev);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_StopVoDev failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 12: stop vpss group and chn
     *************************************************/
exit3:
    s32Ret = SAMPLE_RGN_StopVpss(s32VpssGrpNum, u32VpssChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_StopVpss failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 13: stop vi chn
     *************************************************/
exit2:
    s32Ret = SAMPLE_COMM_VI_StopVi(&g_stViChnConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_StopVi failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 14: destroy region
     *************************************************/
exit1:
    s32Ret = SAMPLE_RGN_DestroyRegion(Handle, u32RgnNumCover+u32RgnNumMosaic);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_DestroyRegion failed! s32Ret: 0x%x.\n", s32Ret);
    }
    
exit0:
    s32Ret = SAMPLE_RGN_SYS_Exit();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_SYS_Exit failed! s32Ret: 0x%x.\n", s32Ret);
    }

    return s32Ret;
}

/******************************************************************************************
    function :  Vpss coverex
    process steps:
    0) create some coverex regions
    1) attach them to vpss
    2) change Regions' Layer
    3) change Regions' position
    4) change Regions' alpha (front and backgroud)
******************************************************************************************/

HI_S32 SAMPLE_RGN_AddCoverExToVpss(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    RGN_HANDLE Handle;
    HI_U32 u32RgnNum;
    MPP_CHN_S stSrcChn, stDesChn;
    HI_S32 s32VpssGrpNum;
    HI_U32 u32VpssChnNum = 1;//VPSS_MAX_PHY_CHN_NUM;
    VPSS_CHN VpssChn = VPSS_CHN0;
    VO_DEV VoDev;
    VO_LAYER VoLayer;
    HI_U32 s32VoChnNum;
    VO_INTF_SYNC_E enIntfSync = VO_OUTPUT_1080P30;
    RGN_ATTR_INFO_S stRgnAttrInfo;

    s32Ret = SAMPLE_RGN_SYS_Init();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_SYS_Init failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit0;
    }

    /*************************************************
      step 1: create region and attach to vpss group
     *************************************************/
    Handle    = 0;
    u32RgnNum = 8;
	stSrcChn.enModId = HI_ID_VPSS;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = VPSS_CHN0;
    s32Ret = SAMPLE_RGN_CreateCoverEx(Handle, u32RgnNum, &stSrcChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_CreateCoverEx failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit1;
    }

    /*************************************************
      step 2: start vi chn
     *************************************************/
    s32Ret = SAMPLE_COMM_VI_StartVi(&g_stViChnConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_StartVi failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit2;
    }

    /*************************************************
      step 3: start vpss group and chn
     *************************************************/
    s32VpssGrpNum = 1;
    if (VO_INTF_BT1120 == g_enVoIntfType)
    {
        s32Ret = SAMPLE_RGN_StartVpssHD(s32VpssGrpNum, u32VpssChnNum);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_RGN_StartVpssHD failed! s32Ret: 0x%x.\n", s32Ret);
            goto exit3;
        }
    }
    else if (VO_INTF_CVBS == g_enVoIntfType)
    {
        s32Ret = SAMPLE_RGN_StartVpss(s32VpssGrpNum, u32VpssChnNum);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_RGN_StartVpss failed! s32Ret: 0x%x.\n", s32Ret);
            goto exit3;
        }
    }

    /*************************************************
      step 4: start vo dev and chn
     *************************************************/
    VoDev       = 0;
    s32VoChnNum = 1;
    if (VO_INTF_BT1120 == g_enVoIntfType)
    {
        enIntfSync  = VO_OUTPUT_1080P30;
    }
    else if (VO_INTF_CVBS == g_enVoIntfType)
    {
        enIntfSync  = VO_OUTPUT_PAL;
    }
    s32Ret = SAMPLE_RGN_StartVo(VoDev, s32VoChnNum, enIntfSync);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_StartVo failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit4;
    }

    /*************************************************
      step 5: bind vi and vpss
     *************************************************/
    stSrcChn.enModId  = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDesChn.enModId  = HI_ID_VPSS;
    stDesChn.s32DevId = 0;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDesChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_Bind failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit5;
    }

    /*************************************************
      step 6: bind vpss and vo
     *************************************************/
    VoLayer = SAMPLE_RGN_GetVoLayer(VoDev);
    if (VoLayer < 0)
    {
        SAMPLE_PRT("SAMPLE_RGN_GetVoLayer failed! VoDev: %d.\n", VoDev);
        goto exit5;
    }

    stSrcChn.enModId  = HI_ID_VPSS;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VpssChn;

    stDesChn.enModId  = HI_ID_VOU;
    stDesChn.s32DevId = VoLayer;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDesChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_Bind failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit5;
    }

    /*************************************************
      step 7: create a thread to change region's
      layer, position, alpha and other display attribute
     *************************************************/
    stRgnAttrInfo.Handle    = Handle;
    stRgnAttrInfo.u32RgnNum = u32RgnNum;
    pthread_create(&g_stRgnThread, NULL, SAMPLE_RGN_VpssCoverExDynamicDisplay, (HI_VOID*)&stRgnAttrInfo);

    printf("\n#############Sample start ok! Press Enter to switch!#############\n");


    /*************************************************
      step 8: stop thread and release all the resource
     *************************************************/
    getchar();
    bExit = HI_TRUE;

    if (g_stRgnThread)
    {
        pthread_join(g_stRgnThread, 0);
        g_stRgnThread = 0;
    }
    
    bExit = HI_FALSE;

    /*************************************************
      step 9: unbind vpss and vo
     *************************************************/
exit5:
    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_UnBind failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 10: unbind vi and vpss
     *************************************************/
    stSrcChn.enModId  = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDesChn.enModId  = HI_ID_VPSS;
    stDesChn.s32DevId = 0;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_UnBind failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 11: stop vo dev and chn
     *************************************************/
exit4:
    s32Ret = SAMPLE_RGN_StopVoChn(VoDev, s32VoChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_StopVoChn failed! s32Ret: 0x%x.\n", s32Ret);
    }

    s32Ret = SAMPLE_RGN_StopVoDev(VoDev);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_StopVoDev failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 12: stop vpss group and chn
     *************************************************/
exit3:
    s32Ret = SAMPLE_RGN_StopVpss(s32VpssGrpNum, u32VpssChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_StopVpss failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 13: stop vi chn
     *************************************************/
exit2:
    s32Ret = SAMPLE_COMM_VI_StopVi(&g_stViChnConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_StopVi failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 14: destroy region
     *************************************************/
exit1:
    s32Ret = SAMPLE_RGN_DestroyRegion(Handle, u32RgnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_DestroyRegion failed! s32Ret: 0x%x.\n", s32Ret);
    }

exit0:
    s32Ret = SAMPLE_RGN_SYS_Exit();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_SYS_Exit failed! s32Ret: 0x%x.\n", s32Ret);
    }

    return s32Ret;
}

/******************************************************************************************
    function :  Vpss Osd
    process steps:
    0) create some osd regions
    1) attach them to vpss
    2) start a thread to handle color reverse
    3) change Regions' Layer
    4) change Regions' position
    5) change Regions' alpha (front and backgroud)
******************************************************************************************/

HI_S32 SAMPLE_RGN_AddOsdToVpss(HI_VOID)
{
    RGN_HANDLE Handle;
    HI_S32 u32RgnNum;
    HI_S32 s32Ret = HI_SUCCESS;
    RGN_ATTR_S stRgnAttrSet;
    RGN_CANVAS_INFO_S stCanvasInfo;
    RGN_CHN_ATTR_S stChnAttr;
    MPP_CHN_S stChn;
    HI_S32 i;
    BITMAP_S stBitmap;
    HI_S32 s32VpssGrpNum;
    HI_S32 u32VpssChnNum = 1;//VPSS_MAX_PHY_CHN_NUM;
    VPSS_CHN VpssChn = VPSS_CHN0;
    VO_DEV VoDev;
    VO_LAYER VoLayer;
    HI_U32 s32VoChnNum;
    VO_INTF_SYNC_E enIntfSync = VO_OUTPUT_1080P30;
    MPP_CHN_S stSrcChn, stDesChn;
    RGN_OSD_REVERSE_INFO_S stOsdReverseInfo;
    RGN_ATTR_INFO_S stRgnAttrInfo;
    RECT_S astOsdLumaRect[3];
    HI_U32 u32OsdRectCnt;
    SIZE_S stSize;

    s32Ret = SAMPLE_RGN_SYS_Init();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_SYS_Init failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit0;
    }

    /*************************************************
      step 1: create region and attach to vpss group
     *************************************************/
    Handle    = 0;
    u32RgnNum = 4;
	stSrcChn.enModId = HI_ID_VPSS;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = 0;
    s32Ret = SAMPLE_RGN_CreateOverlayEx(Handle, u32RgnNum, &stSrcChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_CreateOverlayExForVpss failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit1;
    }

    /*************************************************
      step 2: start vi chn
     *************************************************/
    s32Ret = SAMPLE_COMM_VI_StartVi(&g_stViChnConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_StartVi failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit2;
    }

    /*************************************************
      step 3: start vpss group and chn
     *************************************************/
    s32VpssGrpNum = 1;
    if (VO_INTF_BT1120 == g_enVoIntfType)
    {
        s32Ret = SAMPLE_RGN_StartVpssHD(s32VpssGrpNum, u32VpssChnNum);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_RGN_StartVpssHD failed! s32Ret: 0x%x.\n", s32Ret);
            goto exit3;
        }
    }
    else if (VO_INTF_CVBS == g_enVoIntfType)
    {
        s32Ret = SAMPLE_RGN_StartVpss(s32VpssGrpNum, u32VpssChnNum);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_RGN_StartVpss failed! s32Ret: 0x%x.\n", s32Ret);
            goto exit3;
        }
    }

    /*************************************************
      step 4: start vo dev and chn
     *************************************************/
    VoDev       = 0;
    s32VoChnNum = 1;
    if (VO_INTF_BT1120 == g_enVoIntfType)
    {
        enIntfSync  = VO_OUTPUT_1080P30;
    }
    else if (VO_INTF_CVBS == g_enVoIntfType)
    {
        enIntfSync  = VO_OUTPUT_PAL;
    }
    s32Ret = SAMPLE_RGN_StartVo(VoDev, s32VoChnNum, enIntfSync);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_StartVo failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit4;
    }

    /*************************************************
      step 5: bind vi and vpss
     *************************************************/
    stSrcChn.enModId  = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDesChn.enModId  = HI_ID_VPSS;
    stDesChn.s32DevId = 0;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDesChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_Bind failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit5;
    }

    /*************************************************
      step 6: bind vpss and vo
     *************************************************/
    VoLayer = SAMPLE_RGN_GetVoLayer(VoDev);
    if (VoLayer < 0)
    {
        SAMPLE_PRT("SAMPLE_RGN_GetVoLayer failed! VoDev: %d.\n", VoDev);
        goto exit5;
    }

    stSrcChn.enModId  = HI_ID_VPSS;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VpssChn;

    stDesChn.enModId  = HI_ID_VOU;
    stDesChn.s32DevId = VoLayer;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDesChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_Bind failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit5;
    }

    /*************************************************
      step 7: load bitmap to region
     *************************************************/
    s32Ret = HI_MPI_RGN_GetAttr(Handle, &stRgnAttrSet);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_RGN_GetAttr failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit5;
    }

    s32Ret = HI_MPI_RGN_GetCanvasInfo(Handle, &stCanvasInfo);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_RGN_GetCanvasInfo failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit5;
    }

    stBitmap.pData   = (HI_VOID*)stCanvasInfo.u32VirtAddr;
    stSize.u32Width  = stCanvasInfo.stSize.u32Width;
    stSize.u32Height = stCanvasInfo.stSize.u32Height;
    s32Ret = SAMPLE_RGN_UpdateCanvas("mm.bmp", &stBitmap, HI_FALSE, 0, &stSize, stCanvasInfo.u32Stride,
                                     stRgnAttrSet.unAttr.stOverlayEx.enPixelFmt);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_UpdateCanvas failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit5;
    }

    s32Ret = HI_MPI_RGN_UpdateCanvas(Handle);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_RGN_UpdateCanvas failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit5;
    }

    /*************************************************
      step 8: create a thread to change region's
      layer, position, alpha and other display attribute
     *************************************************/
    stRgnAttrInfo.Handle    = Handle;
    stRgnAttrInfo.u32RgnNum = u32RgnNum; 
    pthread_create(&g_stRgnThread, NULL, SAMPLE_RGN_VpssOSdDynamicDisplay, (HI_VOID*)&stRgnAttrInfo);
    sleep(3);
    /*************************************************
      step 10: start a thread to handle osd color reverse
     *************************************************/
    u32OsdRectCnt                            = 3;
    stOsdReverseInfo.Handle                  = Handle;
    stOsdReverseInfo.VpssGrp                 = 0;
    stOsdReverseInfo.VpssChn                 = VpssChn;
    stOsdReverseInfo.u8PerPixelLumaThrd      = 128;
    stOsdReverseInfo.stLumaRgnInfo.u32RegionNum   = u32OsdRectCnt;
    stOsdReverseInfo.stLumaRgnInfo.pstRegion = astOsdLumaRect;

    stSize.u32Width  = stRgnAttrSet.unAttr.stOverlayEx.stSize.u32Width;
    stSize.u32Height = stRgnAttrSet.unAttr.stOverlayEx.stSize.u32Height;

    stChn.enModId  = HI_ID_VPSS;
    stChn.s32DevId = 0;
    stChn.s32ChnId = VpssChn;
    s32Ret = HI_MPI_RGN_GetDisplayAttr(Handle, &stChn, &stChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_RGN_GetDisplayAttr failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit6;
    }

    for (i = 0; i < u32OsdRectCnt; i++)
    {
        astOsdLumaRect[i].s32X = ((stSize.u32Width / u32OsdRectCnt) * i) + stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X;
        astOsdLumaRect[i].s32Y = stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y;
        astOsdLumaRect[i].u32Width  = (stSize.u32Width / u32OsdRectCnt);
        astOsdLumaRect[i].u32Height = stSize.u32Height;
    }
    pthread_create(&g_stVpssOsdReverseThread, NULL, SAMPLE_RGN_VpssOsdReverse, (HI_VOID*)&stOsdReverseInfo);

    printf("\n#############Sample start ok! Press Enter to switch!#############\n");



    /*************************************************
      step 11: stop thread and release all the resource
     *************************************************/
    getchar();
exit6:
    bExit = HI_TRUE;

    if (g_stVpssOsdReverseThread)
    {
        pthread_join(g_stVpssOsdReverseThread, 0);
        g_stVpssOsdReverseThread = 0;
    }

    if (g_stRgnThread)
    {
        pthread_join(g_stRgnThread, 0);
        g_stRgnThread = -1;
    }
    
    bExit = HI_FALSE;

    /*************************************************
      step 12: unbind vpss and vo
     *************************************************/
exit5:
    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_UnBind failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 13: unbind vdec and vpss
     *************************************************/
    stSrcChn.enModId  = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDesChn.enModId  = HI_ID_VPSS;
    stDesChn.s32DevId = 0;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_UnBind failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 14: stop vo dev and chn
     *************************************************/
exit4:
    s32Ret = SAMPLE_RGN_StopVoChn(VoDev, s32VoChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_StopVoChn failed! s32Ret: 0x%x.\n", s32Ret);
    }

    s32Ret = SAMPLE_RGN_StopVoDev(VoDev);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_StopVoDev failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 15: stop vpss group and chn
     *************************************************/
exit3:
    s32Ret = SAMPLE_RGN_StopVpss(s32VpssGrpNum, u32VpssChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_StopVpss failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 16: stop vi chn
     *************************************************/
exit2:
    s32Ret = SAMPLE_COMM_VI_StopVi(&g_stViChnConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_StopVi failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 17: destroy region
     *************************************************/
exit1:
    s32Ret = SAMPLE_RGN_DestroyRegion(Handle, u32RgnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_DestroyRegion failed! s32Ret: 0x%x.\n", s32Ret);
    }

exit0:
    s32Ret = SAMPLE_RGN_SYS_Exit();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_SYS_Exit failed! s32Ret: 0x%x.\n", s32Ret);
    }

    return s32Ret;
}

/******************************************************************************************
    function :  Venc And Vpss OSD
    process steps:
      1) create some cover/osd regions
      2) display  cover/osd regions
      3) change  Regions' position
      4) load bmp form bmp-file to Region-0
      5) enable color reverse for Region-0
******************************************************************************************/

HI_S32 SAMPLE_RGN_AddOsdToVencAndVpss(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    RGN_HANDLE Handle;
    RGN_HANDLE VencOsdHandle;
    RGN_HANDLE VpssOsdHandle;
    HI_S32 u32VencRgnNum;
    HI_S32 u32VpssRgnNum;
    RGN_ATTR_S stRgnAttrSet;
    RGN_CANVAS_INFO_S stCanvasInfo;
    BITMAP_S stBitmap;
    MPP_CHN_S stSrcChn, stDesChn;
    RGN_CHN_ATTR_S stChnAttr;
    MPP_CHN_S stChn;
    HI_S32 i;

    VO_DEV VoDev;
    VO_LAYER VoLayer;
    HI_U32 s32VoChnNum;
    VO_INTF_SYNC_E enIntfSync = VO_OUTPUT_1080P30;

    VENC_CHN VencChn = 0;
    HI_S32 s32VpssGrpNum;
    HI_S32 u32VpssChnNum = 2;

    RGN_OSD_REVERSE_INFO_S stVpssOsdReverseInfo;
    RGN_ATTR_INFO_S stRgnAttrInfo;
    RECT_S astVpssOsdLumaRect[3];
    HI_U32 u32VpssOsdRectCnt;
    SIZE_S stSize;

    s32Ret = SAMPLE_RGN_SYS_Init();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_SYS_Init failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit0;
    }

    /*************************************************
      step 1: create region and attach to venc
     *************************************************/
    VencOsdHandle = 0;
    u32VencRgnNum = 4;
    s32Ret = SAMPLE_RGN_CreateOverlayForVenc(VencOsdHandle, u32VencRgnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_CreateOverlayForVenc failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit1;
    }

    /*************************************************
      step 2: create region and attach to vpss group
     *************************************************/
    VpssOsdHandle = u32VencRgnNum;
    u32VpssRgnNum = 4;
	stSrcChn.enModId = HI_ID_VPSS;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = 0;
    s32Ret = SAMPLE_RGN_CreateOverlayEx(VpssOsdHandle, u32VpssRgnNum, &stSrcChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_CreateOverlayExForVpss failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit1;
    }

    /*************************************************
      step 3: start vi chn
     *************************************************/
    s32Ret = SAMPLE_COMM_VI_StartVi(&g_stViChnConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_StartVi failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit2;
    }

    /*************************************************
      step 4: start vpss group and chn
     *************************************************/
    s32VpssGrpNum = 1;
    if (VO_INTF_BT1120 == g_enVoIntfType)
    {
        s32Ret = SAMPLE_RGN_StartVpssHD(s32VpssGrpNum, u32VpssChnNum);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_RGN_StartVpssHD failed! s32Ret: 0x%x.\n", s32Ret);
            goto exit3;
        }
    }
    else if (VO_INTF_CVBS == g_enVoIntfType)
    {
        s32Ret = SAMPLE_RGN_StartVpss(s32VpssGrpNum, u32VpssChnNum);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_RGN_StartVpss failed! s32Ret: 0x%x.\n", s32Ret);
            goto exit3;
        }
    }

    /*************************************************
      step 5: start vo dev and chn
     *************************************************/
    VoDev       = 0;
    s32VoChnNum = 1;
    if (VO_INTF_BT1120 == g_enVoIntfType)
    {
        enIntfSync  = VO_OUTPUT_1080P30;
    }
    else if (VO_INTF_CVBS == g_enVoIntfType)
    {
        enIntfSync  = VO_OUTPUT_PAL;
    }
    s32Ret = SAMPLE_RGN_StartVo(VoDev, s32VoChnNum, enIntfSync);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_StartVo failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit4;
    }

    /*************************************************
      step 6: bind vi and vpss
     *************************************************/
    stSrcChn.enModId  = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDesChn.enModId  = HI_ID_VPSS;
    stDesChn.s32DevId = 0;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDesChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_Bind failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit5;
    }
    
    /*************************************************
      step 7: start venc chn
     *************************************************/
    s32Ret = SAMPLE_COMM_VENC_Start(VencChn, PT_H264, gs_enNorm, PIC_D1, 
        SAMPLE_RC_CBR, 0, ROTATE_NONE);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VENC_Start failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit5;
    }

    /*************************************************
      step 8: bind vpss and vo
     *************************************************/
    VoLayer = SAMPLE_RGN_GetVoLayer(VoDev);
    if (VoLayer < 0)
    {
        SAMPLE_PRT("SAMPLE_RGN_GetVoLayer failed! VoDev: %d.\n", VoDev);
        goto exit6;
    }

    stSrcChn.enModId  = HI_ID_VPSS;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VPSS_CHN0;

    stDesChn.enModId  = HI_ID_VOU;
    stDesChn.s32DevId = VoLayer;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDesChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_Bind failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit6;
    }

    /*************************************************
      step 9: bind vpss and venc
     *************************************************/
    stSrcChn.enModId  = HI_ID_VPSS;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VPSS_CHN1;

    stDesChn.enModId  = HI_ID_VENC;
    stDesChn.s32DevId = 0;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDesChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_Bind failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit6;
    }

    /*************************************************
      step 10: load bitmap to region
     *************************************************/
    for (Handle = VencOsdHandle; Handle <= u32VencRgnNum; Handle += u32VencRgnNum)
    {
        s32Ret = HI_MPI_RGN_GetAttr(Handle, &stRgnAttrSet);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_RGN_GetAttr failed! s32Ret: 0x%x.\n", s32Ret);
            goto exit6;
        }

        s32Ret = HI_MPI_RGN_GetCanvasInfo(Handle, &stCanvasInfo);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_RGN_GetCanvasInfo failed! s32Ret: 0x%x.\n", s32Ret);
            goto exit6;
        }

        stBitmap.pData   = (HI_VOID*)stCanvasInfo.u32VirtAddr;
        stSize.u32Width  = stCanvasInfo.stSize.u32Width;
        stSize.u32Height = stCanvasInfo.stSize.u32Height;
        s32Ret = SAMPLE_RGN_UpdateCanvas("mm.bmp", &stBitmap, HI_FALSE, 0, &stSize, stCanvasInfo.u32Stride,
                                         stRgnAttrSet.unAttr.stOverlayEx.enPixelFmt);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_RGN_UpdateCanvas failed! s32Ret: 0x%x.\n", s32Ret);
            goto exit6;
        }

        s32Ret = HI_MPI_RGN_UpdateCanvas(Handle);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_RGN_UpdateCanvas failed! s32Ret: 0x%x.\n", s32Ret);
            goto exit6;
        }
    }

    /*************************************************
      step 11: create a thread to change region's
      layer, position, alpha and other display attribute
     *************************************************/
    pthread_create(&g_stVencRgnThread, NULL, SAMPLE_RGN_VencOSdDynamicDisplay, NULL);
    stRgnAttrInfo.Handle    = VpssOsdHandle;
    stRgnAttrInfo.u32RgnNum = u32VpssRgnNum;
    pthread_create(&g_stVpssRgnThread, NULL, SAMPLE_RGN_VpssOSdDynamicDisplay, (HI_VOID*)&stRgnAttrInfo);
    sleep(3);
    
    /*************************************************
      step 12: start a thread to handle vpss osd color
      reverse
     *************************************************/
    u32VpssOsdRectCnt                            = 3;
    stVpssOsdReverseInfo.Handle                  = VpssOsdHandle;
    stVpssOsdReverseInfo.VpssGrp                 = 0;
    stVpssOsdReverseInfo.VpssChn                 = VPSS_CHN0;
    stVpssOsdReverseInfo.u8PerPixelLumaThrd      = 128;
    stVpssOsdReverseInfo.stLumaRgnInfo.u32RegionNum   = u32VpssOsdRectCnt;
    stVpssOsdReverseInfo.stLumaRgnInfo.pstRegion = astVpssOsdLumaRect;

    s32Ret = HI_MPI_RGN_GetAttr(VpssOsdHandle, &stRgnAttrSet);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_RGN_GetAttr failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit7;
    }
    stSize.u32Width  = stRgnAttrSet.unAttr.stOverlayEx.stSize.u32Width;
    stSize.u32Height = stRgnAttrSet.unAttr.stOverlayEx.stSize.u32Height;

    stChn.enModId  = HI_ID_VPSS;
    stChn.s32DevId = 0;
    stChn.s32ChnId = VPSS_CHN0;
    s32Ret = HI_MPI_RGN_GetDisplayAttr(VpssOsdHandle, &stChn, &stChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_RGN_GetDisplayAttr failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit7;
    }

    for (i = 0; i < u32VpssOsdRectCnt; i++)
    {
        astVpssOsdLumaRect[i].s32X = ((stSize.u32Width / u32VpssOsdRectCnt) * i) + stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X;
        astVpssOsdLumaRect[i].s32Y = stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y;
        astVpssOsdLumaRect[i].u32Width  = (stSize.u32Width / u32VpssOsdRectCnt);
        astVpssOsdLumaRect[i].u32Height = stSize.u32Height;
    }
    pthread_create(&g_stVpssOsdReverseThread, NULL, SAMPLE_RGN_VpssOsdReverse, (HI_VOID*)&stVpssOsdReverseInfo);

    /*************************************************
      step 13: begin to get stream
     *************************************************/
    SAMPLE_COMM_VENC_StartGetStream(1);

    printf("\n#############Sample start ok! Press Enter to switch!#############\n");

    /*************************************************
      step 14: stop thread and release all the resource
     *************************************************/
    getchar();

exit7:
    bExit = HI_TRUE;

    SAMPLE_COMM_VENC_StopGetStream();    
    if (g_stVpssRgnThread)
    {
        pthread_join(g_stVpssRgnThread, 0);
        g_stVpssRgnThread = 0;
    }
    if (g_stVencRgnThread)
    {
        pthread_join(g_stVencRgnThread, 0);
        g_stVencRgnThread = 0;
    }
    if (g_stVpssOsdReverseThread)
    {
        pthread_join(g_stVpssOsdReverseThread, 0);
        g_stVpssOsdReverseThread = 0;
    }

    bExit = HI_FALSE;

    /*************************************************
      step 15: unbind vpss and venc
     *************************************************/
exit6:
    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_UnBind failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 16: unbind vpss and vo
     *************************************************/

    stSrcChn.enModId  = HI_ID_VPSS;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VPSS_CHN0;

    stDesChn.enModId  = HI_ID_VOU;
    stDesChn.s32DevId = VoLayer;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_UnBind failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 18: unbind vi and vpss
     *************************************************/
    stSrcChn.enModId  = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDesChn.enModId  = HI_ID_VPSS;
    stDesChn.s32DevId = 0;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_Bind failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 19: stop venc chn
     *************************************************/
exit5:
    s32Ret = SAMPLE_COMM_VENC_Stop(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_StopVenc failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 20: stop vo dev and chn
     *************************************************/
exit4:
    s32Ret = SAMPLE_RGN_StopVoChn(VoDev, s32VoChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_StopVoChn failed! s32Ret: 0x%x.\n", s32Ret);
    }

    s32Ret = SAMPLE_RGN_StopVoDev(VoDev);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_StopVoDev failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 21: stop vpss group and chn
     *************************************************/
exit3:
    s32Ret = SAMPLE_RGN_StopVpss(s32VpssGrpNum, u32VpssChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_StopVpss failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 22: stop vi chn
     *************************************************/
exit2:
    s32Ret = SAMPLE_COMM_VI_StopVi(&g_stViChnConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_StopVi failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 23: destroy region
     *************************************************/
exit1:
    s32Ret = SAMPLE_RGN_DestroyRegion(VencOsdHandle, u32VencRgnNum + u32VpssRgnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_DestroyRegion failed! s32Ret: 0x%x.\n", s32Ret);
    }

exit0:
    s32Ret = SAMPLE_RGN_SYS_Exit();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_SYS_Exit failed! s32Ret: 0x%x.\n", s32Ret);
    }

    return s32Ret;
}

/******************************************************************************************
    function :  Vi coverex overlayex
    process steps:
    0) create 1 coverex and 1 mosaicex
    1) attach them to vpss
    2) change Regions' Layer
    3) change Regions' position
    4) change Regions' alpha (front and backgroud)
******************************************************************************************/

HI_S32 SAMPLE_RGN_AddCoverexAndOverlayexToVi(HI_VOID)
{
    HI_S32 s32Ret = HI_SUCCESS;
    RGN_HANDLE Handle;
    HI_U32 u32RgnNumCoverEx;
    HI_U32 u32RgnNumOverlayEx;
    MPP_CHN_S stSrcChn, stDesChn;
    HI_S32 s32VpssGrpNum;
    HI_U32 u32VpssChnNum = 1;//VPSS_MAX_PHY_CHN_NUM;
    VPSS_CHN VpssChn = VPSS_CHN0;
    VO_DEV VoDev;
    VO_LAYER VoLayer;
    HI_U32 s32VoChnNum;
    VO_INTF_SYNC_E enIntfSync = VO_OUTPUT_1080P30;
    RGN_ATTR_INFO_S stRgnAttrInfoCoverex;
	RGN_ATTR_INFO_S stRgnAttrInfoOverlayex;

    s32Ret = SAMPLE_RGN_SYS_Init();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_SYS_Init failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit0;
    }

    /*************************************************
      step 1: create region and attach to vpss group
     *************************************************/
    Handle    = 0;
    u32RgnNumCoverEx = 8;
    stSrcChn.enModId = HI_ID_VIU;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = 0;
    s32Ret = SAMPLE_RGN_CreateCoverEx(Handle, u32RgnNumCoverEx, &stSrcChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_CreateCoverEx failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit1;
    }
    u32RgnNumOverlayEx = 4;
    s32Ret = SAMPLE_RGN_CreateOverlayEx(Handle + u32RgnNumCoverEx, u32RgnNumOverlayEx, &stSrcChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_CreateMosaic failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit1;
    }

    /*************************************************
      step 2: start vi chn
     *************************************************/
    s32Ret = SAMPLE_COMM_VI_StartVi(&g_stViChnConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_StartVi failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit2;
    }
    /*************************************************
      step 3: start vpss group and chn
     *************************************************/
    s32VpssGrpNum = 1;
    if (VO_INTF_BT1120 == g_enVoIntfType)
    {
        s32Ret = SAMPLE_RGN_StartVpssHD(s32VpssGrpNum, u32VpssChnNum);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_RGN_StartVpssHD failed! s32Ret: 0x%x.\n", s32Ret);
            goto exit3;
        }
    }
    else if (VO_INTF_CVBS == g_enVoIntfType)
    {
        s32Ret = SAMPLE_RGN_StartVpss(s32VpssGrpNum, u32VpssChnNum);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_RGN_StartVpss failed! s32Ret: 0x%x.\n", s32Ret);
            goto exit3;
        }
    }

    /*************************************************
      step 4: start vo dev and chn
     *************************************************/
    VoDev       = 0;
    s32VoChnNum = 1;
    if (VO_INTF_BT1120 == g_enVoIntfType)
    {
        enIntfSync  = VO_OUTPUT_1080P30;
    }
    else if (VO_INTF_CVBS == g_enVoIntfType)
    {
        enIntfSync  = VO_OUTPUT_PAL;
    }
    s32Ret = SAMPLE_RGN_StartVo(VoDev, s32VoChnNum, enIntfSync);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_StartVo failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit4;
    }

    /*************************************************
      step 5: bind vi and vpss
     *************************************************/
    stSrcChn.enModId  = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDesChn.enModId  = HI_ID_VPSS;
    stDesChn.s32DevId = 0;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDesChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_Bind failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit5;
    }

    /*************************************************
      step 6: bind vpss and vo
     *************************************************/
    VoLayer = SAMPLE_RGN_GetVoLayer(VoDev);
    if (VoLayer < 0)
    {
        SAMPLE_PRT("SAMPLE_RGN_GetVoLayer failed! VoDev: %d.\n", VoDev);
        goto exit5;
    }

    stSrcChn.enModId  = HI_ID_VPSS;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VpssChn;

    stDesChn.enModId  = HI_ID_VOU;
    stDesChn.s32DevId = VoLayer;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDesChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_Bind failed! s32Ret: 0x%x.\n", s32Ret);
        goto exit5;
    }

    /*************************************************
      step 7: create a thread to change region's
      layer, position, alpha and other display attribute
     *************************************************/
    stRgnAttrInfoCoverex.Handle    = Handle;
    stRgnAttrInfoCoverex.u32RgnNum = u32RgnNumCoverEx;
    pthread_create(&g_stRgnThread, NULL, SAMPLE_RGN_ViCoverExDynamicDisplay, (HI_VOID*)&stRgnAttrInfoCoverex);

	stRgnAttrInfoOverlayex.Handle    = Handle + u32RgnNumCoverEx;
    stRgnAttrInfoOverlayex.u32RgnNum = u32RgnNumOverlayEx;
    pthread_create(&g_stViRgnThread, NULL, SAMPLE_RGN_ViOSdExDynamicDisplay, (HI_VOID*)&stRgnAttrInfoOverlayex);

    printf("\n#############Sample start ok! Press Enter to switch!#############\n");


    /*************************************************
      step 8: stop thread and release all the resource
     *************************************************/
    getchar();
    bExit = HI_TRUE;

    if (g_stRgnThread)
    {
        pthread_join(g_stRgnThread, 0);
        g_stRgnThread = 0;
    }

	if (g_stViRgnThread)
    {
        pthread_join(g_stViRgnThread, 0);
        g_stViRgnThread = 0;
    }
    
    bExit = HI_FALSE;

    /*************************************************
      step 9: unbind vpss and vo
     *************************************************/
exit5:
    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_UnBind failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 10: unbind vi and vpss
     *************************************************/
    stSrcChn.enModId  = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = 0;

    stDesChn.enModId  = HI_ID_VPSS;
    stDesChn.s32DevId = 0;
    stDesChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDesChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_UnBind failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 11: stop vo dev and chn
     *************************************************/
exit4:
    s32Ret = SAMPLE_RGN_StopVoChn(VoDev, s32VoChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_StopVoChn failed! s32Ret: 0x%x.\n", s32Ret);
    }

    s32Ret = SAMPLE_RGN_StopVoDev(VoDev);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_StopVoDev failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 12: stop vpss group and chn
     *************************************************/
exit3:
    s32Ret = SAMPLE_RGN_StopVpss(s32VpssGrpNum, u32VpssChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_StopVpss failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 13: stop vi chn
     *************************************************/
exit2:
    s32Ret = SAMPLE_COMM_VI_StopVi(&g_stViChnConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_StopVi failed! s32Ret: 0x%x.\n", s32Ret);
    }

    /*************************************************
      step 14: destroy region
     *************************************************/
exit1:
    s32Ret = SAMPLE_RGN_DestroyRegion(Handle, u32RgnNumCoverEx+u32RgnNumOverlayEx);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_DestroyRegion failed! s32Ret: 0x%x.\n", s32Ret);
    }

exit0:
    s32Ret = SAMPLE_RGN_SYS_Exit();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_RGN_SYS_Exit failed! s32Ret: 0x%x.\n", s32Ret);
    }

    return s32Ret;
}


/******************************************************************************
* function    : main()
* Description : region
******************************************************************************/
#ifdef __HuaweiLite__
int app_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    HI_S32 s32Ret = HI_SUCCESS;
    //HI_CHAR ch;

    bExit = HI_FALSE;

    if ( (argc < 3) || (1 != strlen(argv[1])) || (1 != strlen(argv[2])))
    {
        SAMPLE_RGN_Intf_Usage(argv[0]);
		SAMPLE_RGN_Case_Usage();
        return HI_FAILURE;
    }

#ifndef __HuaweiLite__
    signal(SIGINT, SAMPLE_RGN_HandleSig);
    signal(SIGTERM, SAMPLE_RGN_HandleSig);
#endif	

	if ((argc > 1) && ((*argv[1] != '0')))
    {
        SAMPLE_RGN_Intf_Usage("sample_region");
        SAMPLE_RGN_Case_Usage();
        return HI_FAILURE;
    }
	
	g_enVoIntfType = VO_INTF_CVBS;

    g_stViChnConfig.enViMode = SENSOR_TYPE;

    SAMPLE_COMM_VI_GetSizeBySensor(&g_enPicSize);

	

    switch (*argv[2])
    {
        case '0': /* VI->VPSS(COVER+MOSAIC)->VO PAL */
        {
            s32Ret = SAMPLE_RGN_AddCoverAndMosaicToVpss();
            break;
        }
        case '1': /* VI->VPSS(COVEREX)->VO PAL */
        {
            s32Ret = SAMPLE_RGN_AddCoverExToVpss();
            break;
        }
        case '2': /* VI->VPSS(OVERLAYEX)->VO PAL */
        {
            s32Ret = SAMPLE_RGN_AddOsdToVpss();
            break;
        }
        case '3': /* VI->VPSS(OVERLAYEX)->VO/VENC(OVERLAY)->file */
        {
            s32Ret = SAMPLE_RGN_AddOsdToVencAndVpss();
            break;
        }
		case '4': /* VI->VPSS(OVERLAYEX)->VO/VENC(OVERLAY)->file */
        {
            s32Ret = SAMPLE_RGN_AddCoverexAndOverlayexToVi();
            break;
        }
        default :
        {
        	SAMPLE_RGN_Intf_Usage("sample_region");
			SAMPLE_RGN_Case_Usage();
            return HI_FAILURE;
        }
    }

	if (HI_SUCCESS == s32Ret)
    {
        SAMPLE_PRT("program exit normally!\n");
    }
    else
    {
        SAMPLE_PRT("program exit abnormally!\n");
    }
    return s32Ret;

}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
