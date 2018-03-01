/******************************************************************************
  A simple program of Hisilicon mpp implementation.
  Copyright (C), 2012-2020, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2013-7 Created
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "sample_comm.h"
#include "mpi_vgs.h"
#include "loadbmp.h"

typedef struct hisample_MEMBUF_S
{
    VB_BLK  hBlock;
    VB_POOL hPool;
    HI_U32  u32PoolId;
    
    HI_U32  u32PhyAddr;
    HI_U8   *pVirAddr;
    HI_S32  s32Mdev;
} SAMPLE_MEMBUF_S;

SAMPLE_MEMBUF_S g_stMem;
VIDEO_FRAME_INFO_S g_stFrameInfo;
HI_BOOL g_bUserViBuf = HI_FALSE;
HI_BOOL g_bUserCommVb = HI_FALSE;

VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_PAL;
PIC_SIZE_E g_enPicSize = PIC_HD1080;

SAMPLE_VI_CONFIG_S g_stViChnConfig =
{
    .enViMode = PANASONIC_MN34220_SUBLVDS_1080P_30FPS,
    .enNorm   = VIDEO_ENCODING_MODE_AUTO,

    .enRotate = ROTATE_NONE,
    .enViChnSet = VI_CHN_SET_NORMAL,
    .enWDRMode  = WDR_MODE_NONE,
    .enFrmRate  = SAMPLE_FRAMERATE_DEFAULT,
};


FILE *g_fpYuv = NULL;
VB_POOL g_hPool  = VB_INVALID_POOLID;
HI_U32 g_SaveCnt = 0;



/******************************************************************************
* function : to process abnormal case                                         
******************************************************************************/
#ifndef __HuaweiLite__
void SAMPLE_VGS_HandleSig(HI_S32 signo)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
    if (SIGINT == signo || SIGTERM == signo)
    {
        if (g_bUserViBuf)
        {
            HI_MPI_VI_ReleaseFrame(0, &g_stFrameInfo);
    		g_bUserViBuf = HI_FALSE;
        }
		if (g_bUserCommVb)
		{
		    HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
		    g_bUserCommVb = HI_FALSE;
		}
		
        SAMPLE_COMM_ISP_Stop();
        SAMPLE_COMM_SYS_Exit();
		if (g_fpYuv != NULL)
		{
			fclose(g_fpYuv);
		}
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}
#endif

HI_VOID SAMPLE_VGS_Usage(HI_CHAR* sPrgNm)
{
    printf("Usage : %s <cnt>\n", sPrgNm);
    printf("cnt: frame count.\n");
	printf("case: VI->VGS(SCALE+OSD+COVER+LINE)->FILE.\n");
    return;
}


HI_VOID SAMPLE_VGS_SaveSP42XToPlanar(FILE *pfile, VIDEO_FRAME_S *pVBuf)
{
    unsigned int w, h;
    char * pVBufVirt_Y;
    char * pVBufVirt_C;
    char * pMemContent;
    unsigned char *TmpBuff;
    HI_U32 size;
    PIXEL_FORMAT_E  enPixelFormat = pVBuf->enPixelFormat;
    HI_U32 u32UvHeight;
	
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

    pVBufVirt_Y = pVBuf->pVirAddr[0]; 
    pVBufVirt_C = pVBufVirt_Y + (pVBuf->u32Stride[0])*(pVBuf->u32Height);

    TmpBuff = (unsigned char *)malloc(size);
    if(NULL == TmpBuff)
    {
        printf("Func:%s line:%d -- unable alloc %dB memory for tmp buffer\n", 
            __FUNCTION__, __LINE__, size);
        return;
    }

    /* save Y ----------------------------------------------------------------*/

    for(h=0; h<pVBuf->u32Height; h++)
    {
        pMemContent = pVBufVirt_Y + h*pVBuf->u32Stride[0];
        fwrite(pMemContent, 1,pVBuf->u32Width,pfile);
    }

    /* save U ----------------------------------------------------------------*/
    for(h=0; h<u32UvHeight; h++)
    {
        pMemContent = pVBufVirt_C + h*pVBuf->u32Stride[1];

        pMemContent += 1;

        for(w=0; w<pVBuf->u32Width/2; w++)
        {
            TmpBuff[w] = *pMemContent;
            pMemContent += 2;
        }
        fwrite(TmpBuff, 1,pVBuf->u32Width/2,pfile);
    }

    /* save V ----------------------------------------------------------------*/
    for(h=0; h<u32UvHeight; h++)    
    {
        pMemContent = pVBufVirt_C + h*pVBuf->u32Stride[1];

        for(w=0; w<pVBuf->u32Width/2; w++)
        {
            TmpBuff[w] = *pMemContent;
            pMemContent += 2;
        }
        fwrite(TmpBuff, 1,pVBuf->u32Width/2,pfile);
    }
    
    free(TmpBuff);

    return;
}


HI_S32 VGS_SAMPLE_LoadBmp(const char *filename, BITMAP_S *pstBitmap, HI_BOOL bFil, HI_U32 u16FilColor,PIXEL_FORMAT_E enPixelFormat)
{
    OSD_SURFACE_S Surface;
    OSD_BITMAPFILEHEADER bmpFileHeader;
    OSD_BITMAPINFO bmpInfo;
    HI_S32 s32BytesPerPix = 2;

    if(GetBmpInfo(filename,&bmpFileHeader,&bmpInfo) < 0)
    {
        printf("GetBmpInfo err!\n");
        return HI_FAILURE;
    }

    if(enPixelFormat == PIXEL_FORMAT_RGB_4444)
    {
        Surface.enColorFmt =OSD_COLOR_FMT_RGB4444;
    }
    else if(enPixelFormat == PIXEL_FORMAT_RGB_1555)
    {
        Surface.enColorFmt =OSD_COLOR_FMT_RGB1555;
    }
    else if(enPixelFormat == PIXEL_FORMAT_RGB_8888)
    {
        Surface.enColorFmt =OSD_COLOR_FMT_RGB8888;
        s32BytesPerPix = 4;
    }
    else
    {
        printf("enPixelFormat err %d \n",enPixelFormat);
        return HI_FAILURE;
    }
    
    pstBitmap->pData = malloc(s32BytesPerPix*(bmpInfo.bmiHeader.biWidth)*(bmpInfo.bmiHeader.biHeight));
    
    if(NULL == pstBitmap->pData)
    {
        printf("malloc osd memroy err!\n");        
        return HI_FAILURE;
    }
    //printf("line:%d\n",__LINE__);
    CreateSurfaceByBitMap(filename,&Surface,(HI_U8*)(pstBitmap->pData));
    
    pstBitmap->u32Width = Surface.u16Width;
    pstBitmap->u32Height = Surface.u16Height;
    
    pstBitmap->enPixelFormat = enPixelFormat;
   
    

    int i,j;
    HI_U16 *pu16Temp;
    pu16Temp = (HI_U16*)pstBitmap->pData;
    
    if (bFil)
    {
        for (i=0; i<pstBitmap->u32Height; i++)
        {
            for (j=0; j<pstBitmap->u32Width; j++)
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

HI_S32 SAMPLE_VGS_SaveFile_FromVi(HI_VOID)
{
    VB_CONF_S stVbConf;
    HI_S32 i = 0, s32Ret = HI_SUCCESS;
    HI_U32  u32BlkSize;
    VB_POOL hPool  = VB_INVALID_POOLID;
    HI_U32 u32PicLStride            = 0;
    HI_U32 u32PicCStride            = 0;
    HI_U32 u32LumaSize              = 0;
    HI_U32 u32ChrmSize              = 0;
    HI_U32 u32OutWidth              = 1280;
    HI_U32 u32OutHeight             = 720;
    HI_CHAR OutFilename[100]        = {0};
	VGS_ADD_COVER_S stVgsAddCover[4];
	VGS_ADD_OSD_S 	stVgsAddOsd[3];
	VGS_DRAW_LINE_S  stVgsLine[5];
    RGN_ATTR_S stRgnAttr;
    RGN_HANDLE Handle = 0;
	const char *filename;
    PIXEL_FORMAT_E enPixelFormat = PIXEL_FORMAT_RGB_1555;
    BITMAP_S stBitmap;
	RGN_CANVAS_INFO_S stCanvasInfo;
	
    PIC_SIZE_E enPicSize;
    VI_CHN ViChn = 0;

	g_stViChnConfig.enViMode = SENSOR_TYPE;
	SAMPLE_COMM_VI_GetSizeBySensor(&g_enPicSize);
	enPicSize = g_enPicSize;
	
    u32BlkSize = u32OutWidth*u32OutHeight*3>>1;
    memset(&stBitmap, 0, sizeof(BITMAP_S));

    snprintf(OutFilename, 100, "Sample_VGS_%d_%d_%s.yuv", 
            u32OutWidth, u32OutHeight,  "420");
    g_fpYuv = fopen(OutFilename, "wb");
    if(g_fpYuv == NULL)
    {
        SAMPLE_PRT("SAMPLE_TEST:can't open file %s to save yuv\n","Decompress.yuv");
        return HI_FAILURE;
    }
    
    /************************************************
    step1:  init SYS and common VB 
    *************************************************/
    memset(&stVbConf, 0, sizeof(VB_CONF_S));

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, enPicSize,
                 SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.u32MaxPoolCnt = 128;

    /*ddr0 video buffer*/
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt  = 6;
	
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("init sys fail for %#x!\n", s32Ret);
        goto END1;
    }
	

    /******************************************
    step 2: create private pool on ddr0
    ******************************************/
    hPool   = HI_MPI_VB_CreatePool( u32BlkSize, 10,NULL);
    if (hPool == VB_INVALID_POOLID)
    {
        SAMPLE_PRT("HI_MPI_VB_CreatePool failed! \n");
        goto END1;
    }

   
    /*************************************************
      step 3: start vi chn
    *************************************************/
    s32Ret = SAMPLE_COMM_VI_StartVi(&g_stViChnConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_StartVi failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    u32PicLStride = CEILING_2_POWER(u32OutWidth, SAMPLE_SYS_ALIGN_WIDTH);
    u32PicCStride = CEILING_2_POWER(u32OutWidth, SAMPLE_SYS_ALIGN_WIDTH);
    u32LumaSize = (u32PicLStride * u32OutHeight);
    u32ChrmSize = (u32PicCStride * u32OutHeight) >> 2;

    stRgnAttr.enType = OVERLAYEX_RGN;

    stRgnAttr.unAttr.stOverlayEx.enPixelFmt = enPixelFormat;
    stRgnAttr.unAttr.stOverlayEx.stSize.u32Width  = 320;
    stRgnAttr.unAttr.stOverlayEx.stSize.u32Height = 320;
    stRgnAttr.unAttr.stOverlayEx.u32BgColor = 0x80fc;

    s32Ret = HI_MPI_RGN_Create(Handle, &stRgnAttr);
    if(s32Ret != HI_SUCCESS)
    {
       	SAMPLE_PRT("HI_MPI_RGN_Create failed\n");
        goto END2;
    }
    
    
    filename = "mm.bmp";

    s32Ret = VGS_SAMPLE_LoadBmp(filename, &stBitmap, HI_FALSE, 0x0,enPixelFormat);
	if(s32Ret != HI_SUCCESS)
    {
       	SAMPLE_PRT("VGS_SAMPLE_LoadBmp failed\n");
        goto END2;
    }
		
    stBitmap.enPixelFormat = enPixelFormat;
    s32Ret = HI_MPI_RGN_SetBitMap(Handle, &stBitmap);
	if(s32Ret != HI_SUCCESS)
    {
       	SAMPLE_PRT("HI_MPI_RGN_SetBitMap failed\n");
		free(stBitmap.pData);
        goto END2;
    }

    free(stBitmap.pData);
    stBitmap.pData = HI_NULL;

	s32Ret = HI_MPI_RGN_GetCanvasInfo(Handle, &stCanvasInfo);
	if(s32Ret != HI_SUCCESS)
    {
       	SAMPLE_PRT("HI_MPI_RGN_GetCanvasInfo failed\n");
        goto END2;
    }

	s32Ret = HI_MPI_RGN_UpdateCanvas(Handle);
	if(s32Ret != HI_SUCCESS)
    {
       	SAMPLE_PRT("HI_MPI_RGN_UpdateCanvas failed\n");
        goto END2;
    }
	
	HI_MPI_VI_SetFrameDepth(ViChn, 1);

    for(i =0;i<g_SaveCnt;i++)
    {
        VIDEO_FRAME_INFO_S stFrmInfo;
        VGS_HANDLE hHandle = -1;
        g_stMem.hPool = hPool;
        VGS_TASK_ATTR_S stTask;
		s32Ret = HI_MPI_VI_GetFrame(ViChn, &g_stFrameInfo, -1);
        if(s32Ret != HI_SUCCESS)
        {	
            SAMPLE_PRT("get vi image failed\n");
            goto END2;
        }

		g_bUserViBuf = HI_TRUE;

        while((g_stMem.hBlock = HI_MPI_VB_GetBlock(g_stMem.hPool, u32BlkSize,NULL)) == VB_INVALID_HANDLE)
        {
            ;
        }

		g_bUserCommVb = HI_TRUE;
  
        g_stMem.u32PhyAddr = HI_MPI_VB_Handle2PhysAddr(g_stMem.hBlock);

        g_stMem.pVirAddr = (HI_U8 *) HI_MPI_SYS_Mmap( g_stMem.u32PhyAddr, u32BlkSize );
        if(g_stMem.pVirAddr == NULL)
        {
            SAMPLE_PRT("Mem dev may not open\n");
            HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
			g_bUserCommVb = HI_FALSE;
			HI_MPI_VI_ReleaseFrame(ViChn, &g_stFrameInfo);
			g_bUserViBuf = HI_FALSE;
            goto END2;
        }
   
        memset(&stFrmInfo.stVFrame, 0, sizeof(VIDEO_FRAME_S));
        stFrmInfo.stVFrame.u32PhyAddr[0] = g_stMem.u32PhyAddr;
        stFrmInfo.stVFrame.u32PhyAddr[1] = stFrmInfo.stVFrame.u32PhyAddr[0] + u32LumaSize;
        stFrmInfo.stVFrame.u32PhyAddr[2] = stFrmInfo.stVFrame.u32PhyAddr[1] + u32ChrmSize;


        stFrmInfo.stVFrame.pVirAddr[0] = g_stMem.pVirAddr;
        stFrmInfo.stVFrame.pVirAddr[1] = (HI_U8 *) stFrmInfo.stVFrame.pVirAddr[0] + u32LumaSize;
        stFrmInfo.stVFrame.pVirAddr[2] = (HI_U8 *) stFrmInfo.stVFrame.pVirAddr[1] + u32ChrmSize;
 
        stFrmInfo.stVFrame.u32Width     = u32OutWidth;
        stFrmInfo.stVFrame.u32Height    = u32OutHeight;
        stFrmInfo.stVFrame.u32Stride[0] = u32PicLStride;
        stFrmInfo.stVFrame.u32Stride[1] = u32PicLStride;
        stFrmInfo.stVFrame.u32Stride[2] = u32PicLStride;

        stFrmInfo.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
        stFrmInfo.stVFrame.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        stFrmInfo.stVFrame.enVideoFormat  = VIDEO_FORMAT_LINEAR;

        stFrmInfo.stVFrame.u64pts     = (i * 40);
        stFrmInfo.stVFrame.u32TimeRef = (i * 2);

        stFrmInfo.u32PoolId = hPool;

        s32Ret = HI_MPI_VGS_BeginJob(&hHandle);
        if(s32Ret != HI_SUCCESS)
    	{	
    	    SAMPLE_PRT("HI_MPI_VGS_BeginJob failed %#x\n", s32Ret);
            HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
			g_bUserCommVb = HI_FALSE;
            HI_MPI_VI_ReleaseFrame(ViChn, &g_stFrameInfo);
			g_bUserViBuf = HI_FALSE;
    	    goto END2;
    	}
   
        memcpy(&stTask.stImgIn,&g_stFrameInfo,sizeof(VIDEO_FRAME_INFO_S));
        
        memcpy(&stTask.stImgOut ,&stFrmInfo,sizeof(VIDEO_FRAME_INFO_S));
        s32Ret = HI_MPI_VGS_AddScaleTask(hHandle, &stTask);
        if(s32Ret != HI_SUCCESS)
        {	
            SAMPLE_PRT("HI_MPI_VGS_AddScaleTask failed\n");
            HI_MPI_VGS_CancelJob(hHandle);
            HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
			g_bUserCommVb = HI_FALSE;
            HI_MPI_VI_ReleaseFrame(ViChn, &g_stFrameInfo);
			g_bUserViBuf = HI_FALSE;
            goto END2;
        }

		
		memcpy(&stTask.stImgIn, &stTask.stImgOut, sizeof(VIDEO_FRAME_INFO_S));
		stVgsAddCover[0].enCoverType = COVER_RECT;
		stVgsAddCover[0].stDstRect.s32X = 0;
		stVgsAddCover[0].stDstRect.s32Y = 0;
		stVgsAddCover[0].stDstRect.u32Width  = 100;
		stVgsAddCover[0].stDstRect.u32Height = 100;
		stVgsAddCover[0].u32Color = 0xff;

		stVgsAddCover[1].enCoverType = COVER_RECT;
		stVgsAddCover[1].stDstRect.s32X = 100;
		stVgsAddCover[1].stDstRect.s32Y = 100;
		stVgsAddCover[1].stDstRect.u32Width  = 100;
		stVgsAddCover[1].stDstRect.u32Height = 100;
		stVgsAddCover[1].u32Color = 0xff00;

		stVgsAddCover[2].enCoverType = COVER_RECT;
		stVgsAddCover[2].stDstRect.s32X = 200;
		stVgsAddCover[2].stDstRect.s32Y = 200;
		stVgsAddCover[2].stDstRect.u32Width  = 100;
		stVgsAddCover[2].stDstRect.u32Height = 100;
		stVgsAddCover[2].u32Color = 0xff0000;

		stVgsAddCover[3].enCoverType = COVER_RECT;
		stVgsAddCover[3].stDstRect.s32X = 300;
		stVgsAddCover[3].stDstRect.s32Y = 300;
		stVgsAddCover[3].stDstRect.u32Width  = 400;
		stVgsAddCover[3].stDstRect.u32Height = 400;
		stVgsAddCover[3].u32Color = 0xff0;

		s32Ret = HI_MPI_VGS_AddCoverTaskArray(hHandle, &stTask, stVgsAddCover, 4);
        if(s32Ret != HI_SUCCESS)
        {	
            SAMPLE_PRT("HI_MPI_VGS_AddCoverTaskArray failed\n");
            HI_MPI_VGS_CancelJob(hHandle);
            HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
			g_bUserCommVb = HI_FALSE;
            HI_MPI_VI_ReleaseFrame(ViChn, &g_stFrameInfo);
			g_bUserViBuf = HI_FALSE;
            goto END2;
        }
		
		stVgsAddOsd[0].stRect.s32X = 300;
		stVgsAddOsd[0].stRect.s32Y = 300;
		stVgsAddOsd[0].stRect.u32Width = 200;
		stVgsAddOsd[0].stRect.u32Height = 200;
		stVgsAddOsd[0].u32BgAlpha = 255;
		stVgsAddOsd[0].u32FgAlpha = 255;
		stVgsAddOsd[0].u32BgColor = 0x80fc;
		stVgsAddOsd[0].u32PhyAddr = stCanvasInfo.u32PhyAddr;
		stVgsAddOsd[0].u32Stride  = stCanvasInfo.u32Stride;
		stVgsAddOsd[0].enPixelFmt = enPixelFormat;

		stVgsAddOsd[1].stRect.s32X = 500;
		stVgsAddOsd[1].stRect.s32Y = 500;
		stVgsAddOsd[1].stRect.u32Width = 200;
		stVgsAddOsd[1].stRect.u32Height = 200;
		stVgsAddOsd[1].u32BgAlpha = 255;
		stVgsAddOsd[1].u32FgAlpha = 255;
		stVgsAddOsd[1].u32BgColor = 0x8fc0;
		stVgsAddOsd[1].u32PhyAddr = stCanvasInfo.u32PhyAddr;
		stVgsAddOsd[1].u32Stride  = stCanvasInfo.u32Stride;
		stVgsAddOsd[1].enPixelFmt = enPixelFormat;

		stVgsAddOsd[2].stRect.s32X = 800;
		stVgsAddOsd[2].stRect.s32Y = 400;
		stVgsAddOsd[2].stRect.u32Width = 320;
		stVgsAddOsd[2].stRect.u32Height = 320;
		stVgsAddOsd[2].u32BgAlpha = 255;
		stVgsAddOsd[2].u32FgAlpha = 255;
		stVgsAddOsd[2].u32BgColor = 0x8ffc;
		stVgsAddOsd[2].u32PhyAddr = stCanvasInfo.u32PhyAddr;
		stVgsAddOsd[2].u32Stride  = stCanvasInfo.u32Stride;
		stVgsAddOsd[2].enPixelFmt = enPixelFormat;

		s32Ret = HI_MPI_VGS_AddOsdTaskArray(hHandle, &stTask, stVgsAddOsd, 3);
		if(s32Ret)
	    {
	        SAMPLE_PRT("HI_MPI_VGS_AddOsdTaskArray failed\n");
            HI_MPI_VGS_CancelJob(hHandle);
            HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
			g_bUserCommVb = HI_FALSE;
            HI_MPI_VI_ReleaseFrame(ViChn, &g_stFrameInfo);
			g_bUserViBuf = HI_FALSE;
            goto END2;
	    }

		stVgsLine[0].stStartPoint.s32X = 800;
		stVgsLine[0].stStartPoint.s32Y = 100;
		stVgsLine[0].stEndPoint.s32X = 800;
		stVgsLine[0].stEndPoint.s32Y = 500;
		stVgsLine[0].u32Color = 0xff0000;
		stVgsLine[0].u32Thick = 2;

		stVgsLine[1].stStartPoint.s32X = 900;
		stVgsLine[1].stStartPoint.s32Y = 100;
		stVgsLine[1].stEndPoint.s32X = 900;
		stVgsLine[1].stEndPoint.s32Y = 500;
		stVgsLine[1].u32Color = 0xff0000;
		stVgsLine[1].u32Thick = 2;

		stVgsLine[2].stStartPoint.s32X = 800;
		stVgsLine[2].stStartPoint.s32Y = 300;
		stVgsLine[2].stEndPoint.s32X = 900;
		stVgsLine[2].stEndPoint.s32Y = 300;
		stVgsLine[2].u32Color = 0xff0000;
		stVgsLine[2].u32Thick = 4;

		stVgsLine[3].stStartPoint.s32X = 1000;
		stVgsLine[3].stStartPoint.s32Y = 100;
		stVgsLine[3].stEndPoint.s32X = 1000;
		stVgsLine[3].stEndPoint.s32Y = 120;
		stVgsLine[3].u32Color = 0xff0000;
		stVgsLine[3].u32Thick = 4;

		stVgsLine[4].stStartPoint.s32X = 1000;
		stVgsLine[4].stStartPoint.s32Y = 200;
		stVgsLine[4].stEndPoint.s32X = 1000;
		stVgsLine[4].stEndPoint.s32Y = 500;
		stVgsLine[4].u32Color = 0xff0000;
		stVgsLine[4].u32Thick = 8;
		
		s32Ret = HI_MPI_VGS_AddDrawLineTaskArray(hHandle, &stTask, stVgsLine, 5);
		if(s32Ret)
	    {
	        SAMPLE_PRT("HI_MPI_VGS_AddDrawLineTask failed\n");
            HI_MPI_VGS_CancelJob(hHandle);
            HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
			g_bUserCommVb = HI_FALSE;
            HI_MPI_VI_ReleaseFrame(ViChn, &g_stFrameInfo);
			g_bUserViBuf = HI_FALSE;
            goto END2;
	    }
		

        s32Ret = HI_MPI_VGS_EndJob(hHandle);
        if(s32Ret != HI_SUCCESS)
        {	
            SAMPLE_PRT("HI_MPI_VGS_EndJob failed\n");
            HI_MPI_VGS_CancelJob(hHandle);
            HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
			g_bUserCommVb = HI_FALSE;
            HI_MPI_VI_ReleaseFrame(ViChn, &g_stFrameInfo);
			g_bUserViBuf = HI_FALSE;
            goto END2;
        }

        /*Save the yuv*/
        SAMPLE_VGS_SaveSP42XToPlanar(g_fpYuv, &stFrmInfo.stVFrame);
        fflush(g_fpYuv);
        HI_MPI_SYS_Munmap(g_stMem.pVirAddr,u32BlkSize);
        HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
		g_bUserCommVb = HI_FALSE;
        HI_MPI_VI_ReleaseFrame(ViChn, &g_stFrameInfo);
		g_bUserViBuf = HI_FALSE;
        printf("\rfinish saving picure : %d. ", i+1);
        fflush(stdout);   

    }
    printf("\n");
    
END2:

	s32Ret = HI_MPI_RGN_Destroy(Handle);
    if(s32Ret != HI_SUCCESS)
    {
       	SAMPLE_PRT("HI_MPI_RGN_Destroy failed\n");
        return s32Ret;
    }
	
    /*************************************************
      step 13: stop vi chn
     *************************************************/
    s32Ret = SAMPLE_COMM_VI_StopVi(&g_stViChnConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VI_StopVi failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }	
	
	 s32Ret = HI_MPI_VB_DestroyPool(hPool);
     if (HI_SUCCESS != s32Ret)
     {
	 	SAMPLE_PRT("HI_MPI_VB_DestroyPool failed! s32Ret: 0x%x.\n", s32Ret);
     }
END1:
	
    SAMPLE_COMM_SYS_Exit();	
    fclose(g_fpYuv);
	g_fpYuv = NULL;
    return s32Ret;
}


/******************************************************************************
* function    : main()
* Description : vgs sample
******************************************************************************/
#ifdef __HuaweiLite__
int app_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    HI_S32 s32Ret = HI_SUCCESS;

	if (argc < 2)
    {
        SAMPLE_VGS_Usage(argv[0]);
        return HI_FAILURE;
    }

	g_SaveCnt = atoi(argv[1]);
    
#ifndef __HuaweiLite__
    signal(SIGINT, SAMPLE_VGS_HandleSig);
    signal(SIGTERM, SAMPLE_VGS_HandleSig);
#endif

    s32Ret = SAMPLE_VGS_SaveFile_FromVi();


    return s32Ret;
}

