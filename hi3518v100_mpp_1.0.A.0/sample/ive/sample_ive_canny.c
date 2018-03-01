#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include "sample_comm.h"
#include "mpi_ive.h"
#include "hi_tde_type.h"
#include "hi_tde_api.h"

VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_PAL;
VO_INTF_TYPE_E  g_enVoIntfType = VO_INTF_CVBS;
HI_U32 gs_u32ViFrmRate = 0;

HI_BOOL g_bStopSignal = HI_FALSE;


SAMPLE_VI_CONFIG_S g_stViChnConfig =
{
    APTINA_AR0130_DC_720P_30FPS,
    VIDEO_ENCODING_MODE_AUTO,

    ROTATE_180,
    VI_CHN_SET_NORMAL
};


/******************************************************************************
* function : show usage
******************************************************************************/
HI_VOID SAMPLE_IVE_Canny_Usage(char *sPrgNm)
{
    printf("This example demonstrates the usage of Canny detector, also with the DMA and TDE zoom !\n");
    printf("Usage : %s \n", sPrgNm);
    
    return;
}


/******************************************************************************
* function : to process abnormal case
******************************************************************************/
HI_VOID SAMPLE_IVE_Canny_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo)
    {
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

HI_S32 SAMPLE_IVE_UnBindViVo(VO_DEV VoDev, VO_CHN VoChn)
{
    MPP_CHN_S stDestChn;

    stDestChn.enModId   = HI_ID_VOU;
    stDestChn.s32DevId  = VoDev;
    stDestChn.s32ChnId  = VoChn;

    return HI_MPI_SYS_UnBind(NULL, &stDestChn);
}

HI_S32 SAMPLE_IVE_BindViVo(VI_CHN ViChn, VO_DEV VoDev, VO_CHN VoChn)
{
    MPP_CHN_S stSrcChn, stDestChn;

    stSrcChn.enModId    = HI_ID_VIU;
    stSrcChn.s32DevId   = 0;
    stSrcChn.s32ChnId   = ViChn;

    stDestChn.enModId   = HI_ID_VOU;
    stDestChn.s32ChnId  = VoChn;
    stDestChn.s32DevId  = VoDev;

    return HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
}


/******************************************************************************
* function : edge detection using Canny detector
******************************************************************************/
HI_S32  SAMPLE_IVE_Canny()
{
    HI_U32 u32ViChnCnt = 2;
    VB_CONF_S stVbConf;
    VO_DEV VoDev = SAMPLE_VO_DEV_DSD0;;
    VO_CHN VoChn = 0;
    VI_CHN ViChn = 0;
    VO_PUB_ATTR_S stVoPubAttr;
    HI_U32 i,j;

    HI_S32 s32Out, s32Ret = HI_SUCCESS;
    SIZE_S stSize;
    VI_EXT_CHN_ATTR_S stExtChnAttr;
    VI_CHN ExtChn = 1;

    VIDEO_FRAME_INFO_S stViFrameInfo, stCannyFrameInfo, stTDEFrameInfo;
    HI_U32 u32Width, u32Height, u32Stride;

    HI_U32 u32BlkSize;
    VB_BLK vbBlkHandle,vbBlkHandle2;
    VB_POOL vbPoolHandle = VB_INVALID_POOLID;  
    
    HI_U32 u32Depth = 1;

    IVE_HANDLE iveFilterHdl, iveCannyHdl, iveDMAHdl;
    IVE_SRC_INFO_S stFilterSrc, stCannySrc, stDMASrc;
    IVE_MEM_INFO_S stFilterDst, stCannyDstMag, stDMADst;
    HI_VOID *pVirFilterDst, *pVirCannyDstMag, *pVirDMASrc;
    IVE_FILTER_CTRL_S stFilterCtrl;
    IVE_CANNY_CTRL_S stCannyCtrl;

    HI_U16 *pCannyDstMag;
    HI_U8 *pDMASrc;
    HI_BOOL bInstant = HI_FALSE;
    HI_BOOL bFinish;
    HI_BOOL bBlock = HI_TRUE;

    /* for TDE */
    TDE_HANDLE handle;
    TDE2_MB_S stMB = {0};
    TDE2_MB_S stMBOut = {0};
    TDE2_RECT_S stMBRect;
    TDE2_RECT_S stMBRect2;
    HI_BOOL bSync = HI_FALSE;
    HI_U32 u32TimeOut = 10;
    TDE2_MBOPT_S stMbOpt= {0};


    /******************************************
      step  1: init global  variable
    ******************************************/
    gs_u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL == gs_enNorm)? 25: 30;
    memset(&stVbConf,0,sizeof(VB_CONF_S));
     
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, PIC_HD720,
                             SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.u32MaxPoolCnt = 128;
     
    /*ddr0 video buffer*/
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 8;
     
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, PIC_D1,
                             SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt = 8;
     
     
    /******************************************
     step 2: mpp system init.
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_0;
    }
     
     /******************************************
      step 3: start vi dev & chn to capture
     ******************************************/
     s32Ret = SAMPLE_COMM_VI_StartVi(&g_stViChnConfig);
     if (HI_SUCCESS != s32Ret)
     {
         SAMPLE_PRT("start vi failed!\n");
         goto END_0;
     }
     
     stExtChnAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
     stExtChnAttr.s32BindChn = ViChn;
     stExtChnAttr.stDestSize.u32Width = 720;
     stExtChnAttr.stDestSize.u32Height = 576;
     stExtChnAttr.s32FrameRate = -1;
     stExtChnAttr.s32SrcFrameRate = -1;
     
     s32Ret = HI_MPI_VI_SetExtChnAttr(ExtChn, &stExtChnAttr);
     if (HI_SUCCESS != s32Ret)
     {
         SAMPLE_PRT("HI_MPI_VI_SetExtChnAttr failed!\n");
         goto END_1;
     }
     s32Ret = HI_MPI_VI_EnableChn(ViChn);
     s32Ret = HI_MPI_VI_EnableChn(ExtChn);
     if (HI_SUCCESS != s32Ret)
     {
         SAMPLE_PRT("HI_MPI_VI_EnableChn failed!\n");
         goto END_2;
     }
     
     /******************************************
      step 4: start vpss and vi bind vpss (subchn needn't bind vpss in this mode)
     ******************************************/
     s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, PIC_HD720, &stSize);
     if (HI_SUCCESS != s32Ret)
     {
         SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
         goto END_2;
     }
     
     /******************************************
     step 5: start VO SD0 (bind * vi )
     ******************************************/
      stVoPubAttr.enIntfType = g_enVoIntfType;
     if(VO_INTF_BT1120 == g_enVoIntfType)
     {
         stVoPubAttr.enIntfSync = VO_OUTPUT_720P50;
         //gs_u32ViFrmRate = 50;
     }
     else
     {
         stVoPubAttr.enIntfSync = VO_OUTPUT_PAL;
     }
     stVoPubAttr.u32BgColor = 0x000000ff;
     /* In HD, this item should be set to HI_FALSE */
     stVoPubAttr.bDoubleFrame = HI_FALSE;
     s32Ret = SAMPLE_COMM_VO_StartDevLayer(VoDev, &stVoPubAttr, gs_u32ViFrmRate);
     if (HI_SUCCESS != s32Ret)
     {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartDevLayer failed!\n");
        goto END_2;
     }
     
    u32Width = 720;
    u32Height = 576;
    for(i=0; i < 4; i++)
    {
        VO_CHN_ATTR_S stVoChnAttr = {0};        

        stVoChnAttr.stRect.s32X = i % 2 * u32Width / 2;   
        stVoChnAttr.stRect.s32Y = i / 2 * u32Height / 2;

        stVoChnAttr.stRect.u32Width = u32Width / 2;
        stVoChnAttr.stRect.u32Height = u32Height /2 ;
        HI_MPI_VO_SetChnAttr(VoDev, VoChn + i, &stVoChnAttr);
        HI_MPI_VO_EnableChn(VoDev, VoChn + i);

        //SAMPLE_IVE_UnBindViVo(VoDev, VoChn + i);
        //SAMPLE_IVE_BindViVo(ExtChn, VoDev, VoChn + i);    
    }
    SAMPLE_IVE_UnBindViVo(VoDev, VoChn);
    SAMPLE_IVE_BindViVo(ExtChn, VoDev, VoChn); 

    vbPoolHandle = HI_MPI_VB_CreatePool(u32Width * u32Height * 2, 8, NULL);
    if ( VB_INVALID_POOLID == vbPoolHandle ) 
    { 
       SAMPLE_PRT("HI_MPI_VB_CreatePool err\n"); 
       goto END_3; 
    } 
    
    /***********************************************
    step 6: get frame and detect edge, and TDE zoom
    ***********************************************/
    
    /* open TDE */
    s32Ret = HI_TDE2_Open();
    if(HI_SUCCESS != s32Ret)
    {
       SAMPLE_PRT("HI_TDE2_Open return 0x%x !\n", s32Ret); 
       goto END_3; 
    }

    s32Ret = HI_MPI_VI_SetFrameDepth(ExtChn, u32Depth);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("set Frame depth err:0x%x\n", s32Ret);
        goto END_TDE;
    }

    while (!g_bStopSignal)
    {
        s32Ret = HI_MPI_VI_GetFrameTimeOut(ExtChn, &stViFrameInfo, 0);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VI_GetFrame failed!\n");
            goto END_TDE;
        }

        /* First step of Canny: Gaussian filter */
        u32Width = stViFrameInfo.stVFrame.u32Width;
        u32Height = stViFrameInfo.stVFrame.u32Height;
        u32Stride = stViFrameInfo.stVFrame.u32Stride[0];
        stFilterSrc.u32Width = u32Width;
        stFilterSrc.u32Height = u32Height;
        stFilterSrc.enSrcFmt = IVE_SRC_FMT_SINGLE;
        stFilterSrc.stSrcMem.u32Stride = u32Stride;
        stFilterSrc.stSrcMem.u32PhyAddr = stViFrameInfo.stVFrame.u32PhyAddr[0];

        stFilterCtrl.as8Mask[0]          = 3;
        stFilterCtrl.as8Mask[1]          = 9;
        stFilterCtrl.as8Mask[2]          = 3;
        stFilterCtrl.as8Mask[3]          = 8;
        stFilterCtrl.as8Mask[4]          = 18;
        stFilterCtrl.as8Mask[5]          = 8;
        stFilterCtrl.as8Mask[6]          = 3;
        stFilterCtrl.as8Mask[7]          = 9;
        stFilterCtrl.as8Mask[8]          = 3;    
        stFilterCtrl.u8Norm              = 6;

        stFilterDst.u32Stride = stFilterSrc.stSrcMem.u32Stride;
        s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&stFilterDst.u32PhyAddr, &pVirFilterDst, "user", HI_NULL,
                                        stFilterDst.u32Stride * stFilterSrc.u32Height);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_SYS_MmzAlloc_Cached return 0x%x!\n", s32Ret);
            goto END_4;
        }

        s32Ret = HI_MPI_IVE_FILTER(&iveFilterHdl, &stFilterSrc, &stFilterDst, &stFilterCtrl, bInstant);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_IVE_FILTER return 0x%x!\n", s32Ret);
            goto END_5;
        }

        /* Second step of Canny: gradient magnitude and angle */
        /* This example didn't calculate angle */
        stCannySrc.u32Width = stFilterSrc.u32Width;
        stCannySrc.u32Height = stFilterSrc.u32Height;
        stCannySrc.enSrcFmt = IVE_SRC_FMT_SINGLE;
        stCannySrc.stSrcMem.u32Stride = stFilterDst.u32Stride;
        stCannySrc.stSrcMem.u32PhyAddr = stFilterDst.u32PhyAddr;
    
        stCannyCtrl.enOutFmt            = IVE_CANNY_OUT_FMT_ONLY_MAG;
        stCannyCtrl.as8Mask[0]          = -1;
        stCannyCtrl.as8Mask[1]          = 0;
        stCannyCtrl.as8Mask[2]          = 1;
        stCannyCtrl.as8Mask[3]          = -2;
        stCannyCtrl.as8Mask[4]          = 0;
        stCannyCtrl.as8Mask[5]          = 2;
        stCannyCtrl.as8Mask[6]          = -1;
        stCannyCtrl.as8Mask[7]          = 0;
        stCannyCtrl.as8Mask[8]          = 1;

        stCannyDstMag.u32Stride = stCannySrc.stSrcMem.u32Stride;
        s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&stCannyDstMag.u32PhyAddr, &pVirCannyDstMag, "user", HI_NULL,
                               stCannyDstMag.u32Stride * stCannySrc.u32Height * 2);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_SYS_MmzAlloc_Cached return 0x%x!\n", s32Ret);
            goto END_5;
        }

        s32Ret = HI_MPI_IVE_CANNY(&iveCannyHdl, &stCannySrc, &stCannyDstMag, HI_NULL, &stCannyCtrl, bInstant);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_IVE_CANNY return 0x%x!\n", s32Ret);
            goto END_6;
        }
    
        pCannyDstMag = (HI_U16*)HI_MPI_SYS_Mmap(stCannyDstMag.u32PhyAddr, stCannyDstMag.u32Stride * stCannySrc.u32Height * 2);

        stDMASrc.u32Width = stCannySrc.u32Width;
        stDMASrc.u32Height = stCannySrc.u32Height;
        stDMASrc.enSrcFmt = IVE_SRC_FMT_SINGLE;
        stDMASrc.stSrcMem.u32Stride = stCannySrc.stSrcMem.u32Stride;

        s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&stDMASrc.stSrcMem.u32PhyAddr, &pVirDMASrc, "user", HI_NULL,
                                            stDMASrc.stSrcMem.u32Stride * stDMASrc.u32Height);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_SYS_MmzAlloc_Cached return 0x%x!\n", s32Ret);
            goto END_7;
        }
    
        pDMASrc = (HI_U8*)HI_MPI_SYS_Mmap(stDMASrc.stSrcMem.u32PhyAddr, stDMASrc.stSrcMem.u32Stride * stDMASrc.u32Height);

        /* binaryzation */
        for(i=0; i<stDMASrc.u32Height; ++i)
        {
            HI_U16 *ptrCanny = pCannyDstMag + i * stCannyDstMag.u32Stride;
            HI_U8 *ptrDMA = pDMASrc + i * stDMASrc.stSrcMem.u32Stride;
            for(j=0; j<stDMASrc.u32Width; ++j)
            {
                ptrDMA[j] = ptrCanny[j] >100 ? 255:0;
            }
        }

        /* For VO preview */
        stCannyFrameInfo.stVFrame.u32Field = VIDEO_FIELD_FRAME;
        stCannyFrameInfo.stVFrame.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_422;
        stCannyFrameInfo.stVFrame.u32Height = u32Height;
        stCannyFrameInfo.stVFrame.u32Width  = u32Width;
        stCannyFrameInfo.stVFrame.u32Stride[0] = u32Stride;
        stCannyFrameInfo.stVFrame.u32Stride[1] = u32Stride;
        stCannyFrameInfo.stVFrame.u32Stride[2] = 0;
        stCannyFrameInfo.stVFrame.u32TimeRef = 0;
        stCannyFrameInfo.stVFrame.u64pts = 0;
    
        vbBlkHandle = HI_MPI_VB_GetBlock(vbPoolHandle, u32Stride * u32Height * 2, HI_NULL);
        stCannyFrameInfo.u32PoolId = HI_MPI_VB_Handle2PoolId(vbBlkHandle);
        stCannyFrameInfo.stVFrame.u32PhyAddr[0] = HI_MPI_VB_Handle2PhysAddr( vbBlkHandle );
        stCannyFrameInfo.stVFrame.u32PhyAddr[1] = stCannyFrameInfo.stVFrame.u32PhyAddr[0] + u32Width * u32Height;
        stCannyFrameInfo.stVFrame.u32PhyAddr[2] = HI_NULL;
   
        stCannyFrameInfo.stVFrame.pVirAddr[0] =  HI_MPI_SYS_Mmap(stCannyFrameInfo.stVFrame.u32PhyAddr[0], u32Stride * u32Height * 2);
        stCannyFrameInfo.stVFrame.pVirAddr[1] = (HI_VOID *)(stCannyFrameInfo.stVFrame.pVirAddr[0]) + u32Stride * u32Height;
        stCannyFrameInfo.stVFrame.pVirAddr[2] = HI_NULL;

        memset(stCannyFrameInfo.stVFrame.pVirAddr[0], 0, u32Stride * u32Height * 2);

        stDMADst.u32Stride = stCannyFrameInfo.stVFrame.u32Stride[0];
        stDMADst.u32PhyAddr = stCannyFrameInfo.stVFrame.u32PhyAddr[0];

        s32Ret = HI_MPI_SYS_MmzFlushCache(stDMASrc.stSrcMem.u32PhyAddr, pVirDMASrc, stDMASrc.stSrcMem.u32Stride * stDMASrc.u32Height);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_SYS_MmzFlushCache return 0x%x!\n", s32Ret);
            goto END_8;
        }

        /* copy the binary image to Y commponent */
        s32Ret = HI_MPI_IVE_DMA(&iveDMAHdl, &stDMASrc, &stDMADst, HI_TRUE);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_IVE_DMA return 0x%x!\n", s32Ret);
            goto END_8;
        }

        s32Ret = HI_MPI_IVE_Query(iveDMAHdl, &bFinish, bBlock);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_IVE_Query return 0x%x!\n", s32Ret);
            goto END_8;
        }
    
        if(HI_TRUE == bFinish)
        {
            s32Ret = HI_MPI_VO_SendFrame(VoDev, VoChn + 1, &stCannyFrameInfo);
            if(HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("HI_MPI_VO_SendFrame return 0x%x!\n", s32Ret);
                goto END_8;
            }
        }
  

  #if 1
        /* TDE zoom */
        /*source image info*/
        stMB.enMbFmt = TDE2_MB_COLOR_FMT_JPG_YCbCr420MBP; //pixel format
        stMB.u32YPhyAddr = stViFrameInfo.stVFrame.u32PhyAddr[0]; //Y component physical address
        stMB.u32YWidth = stViFrameInfo.stVFrame.u32Width; 
        stMB.u32YHeight = stViFrameInfo.stVFrame.u32Height; 
        stMB.u32YStride = stViFrameInfo.stVFrame.u32Stride[0]; 
        stMB.u32CbCrPhyAddr = stViFrameInfo.stVFrame.u32PhyAddr[1]; //C component physical address
        stMB.u32CbCrStride = stViFrameInfo.stVFrame.u32Stride[0]; 
        
        /*source image's ROI rect*/
        stMBRect.s32Xpos = 0;
        stMBRect.s32Ypos = 0;
        stMBRect.u32Height = stMB.u32YHeight;
        stMBRect.u32Width = stMB.u32YWidth;

        
        stTDEFrameInfo.stVFrame.u32Field = VIDEO_FIELD_FRAME;
        stTDEFrameInfo.stVFrame.enPixelFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        stTDEFrameInfo.stVFrame.u32Height = u32Height/4;
        stTDEFrameInfo.stVFrame.u32Width  = u32Width/4;
        stTDEFrameInfo.stVFrame.u32Stride[0] = u32Stride/4;
        stTDEFrameInfo.stVFrame.u32Stride[1] = u32Stride/4;
        stTDEFrameInfo.stVFrame.u32Stride[2] = 0;
        stTDEFrameInfo.stVFrame.u32TimeRef = 0;
        stTDEFrameInfo.stVFrame.u64pts = 0;

        u32Width = stTDEFrameInfo.stVFrame.u32Width;
        u32Height = stTDEFrameInfo.stVFrame.u32Height;
        u32Stride = stTDEFrameInfo.stVFrame.u32Stride[0];
            
        vbBlkHandle2 = HI_MPI_VB_GetBlock(vbPoolHandle, u32Stride * u32Height * 2, HI_NULL);
        stTDEFrameInfo.u32PoolId = HI_MPI_VB_Handle2PoolId(vbBlkHandle2);
        stTDEFrameInfo.stVFrame.u32PhyAddr[0] = HI_MPI_VB_Handle2PhysAddr( vbBlkHandle2 );
        stTDEFrameInfo.stVFrame.u32PhyAddr[1] = stTDEFrameInfo.stVFrame.u32PhyAddr[0] + u32Stride * u32Height;
        stTDEFrameInfo.stVFrame.u32PhyAddr[2] = HI_NULL;
   
        stTDEFrameInfo.stVFrame.pVirAddr[0] =  HI_MPI_SYS_Mmap(stTDEFrameInfo.stVFrame.u32PhyAddr[0], u32Stride * u32Height * 2);
        stTDEFrameInfo.stVFrame.pVirAddr[1] = (HI_VOID *)(stTDEFrameInfo.stVFrame.pVirAddr[0]) + u32Stride * u32Height;
        stTDEFrameInfo.stVFrame.pVirAddr[2] = HI_NULL;

        memset(stCannyFrameInfo.stVFrame.pVirAddr[0], 0, u32Stride * u32Height * 2);
        
        /* output image info */
        stMBOut.enMbFmt = TDE2_MB_COLOR_FMT_JPG_YCbCr420MBP;  //pixel format
        stMBOut.u32YPhyAddr = stTDEFrameInfo.stVFrame.u32PhyAddr[0]; //Y component physical address
        stMBOut.u32YWidth = stTDEFrameInfo.stVFrame.u32Width; 
        stMBOut.u32YHeight = stTDEFrameInfo.stVFrame.u32Height;
        stMBOut.u32YStride = stTDEFrameInfo.stVFrame.u32Stride[0];
        stMBOut.u32CbCrPhyAddr = stTDEFrameInfo.stVFrame.u32PhyAddr[1]; //C component physical address
        stMBOut.u32CbCrStride = stTDEFrameInfo.stVFrame.u32Stride[0];
        
        /*output image's ROI rect */
        stMBRect2.s32Xpos = 0;
        stMBRect2.s32Ypos = 0;
        stMBRect2.u32Height = stMBOut.u32YHeight;
        stMBRect2.u32Width = stMBOut.u32YWidth;

        
        handle = HI_TDE2_BeginJob();
        
        stMbOpt.enResize = TDE2_MBRESIZE_QUALITY_HIGH;
        stMbOpt.bDeflicker = HI_FALSE;

        /* TDE zoom*/
        s32Ret = HI_TDE2_Mb2Mb(handle, &stMB, &stMBRect, &stMBOut, &stMBRect2, &stMbOpt);
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_TDE2_Mb2Mb return 0x%x!\n", s32Ret);
            goto END_9;
        }   

        /* submit job to hardware */
        s32Ret = HI_TDE2_EndJob(handle, bSync, bBlock, u32TimeOut); 
        if(HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_TDE2_EndJob return 0x%x!\n", s32Ret);
        }
        else
        {
            s32Ret = HI_MPI_VO_SendFrame(VoDev, VoChn + 2, &stTDEFrameInfo);
            if(HI_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("HI_MPI_VO_SendFrame return 0x%x!\n", s32Ret);
            }            
        }

    /******************************************
     exit process
    ******************************************/
    END_9:
        s32Out = HI_MPI_SYS_Munmap(stCannyFrameInfo.stVFrame.pVirAddr[0], u32Width * u32Height * 2);
        if(HI_SUCCESS != s32Out)
        {
            SAMPLE_PRT("HI_MPI_SYS_Munmap return 0x%x!\n", s32Out);
        }
        s32Out = HI_MPI_VB_ReleaseBlock(vbBlkHandle2);
        if(HI_SUCCESS != s32Out)
        {
            SAMPLE_PRT("HI_MPI_VB_ReleaseBlock return 0x%x!\n", s32Out);
        }        
#endif
  
    END_8:
        s32Out = HI_MPI_SYS_Munmap(stCannyFrameInfo.stVFrame.pVirAddr[0], u32Width * u32Height * 2);
        if(HI_SUCCESS != s32Out)
        {
            SAMPLE_PRT("HI_MPI_SYS_Munmap return 0x%x!\n", s32Out);
        }
        s32Out = HI_MPI_VB_ReleaseBlock(vbBlkHandle);
        if(HI_SUCCESS != s32Out)
        {
            SAMPLE_PRT("HI_MPI_VB_ReleaseBlock return 0x%x!\n", s32Out);
        }        
        s32Out = HI_MPI_SYS_Munmap(pDMASrc, stDMASrc.stSrcMem.u32Stride * stDMASrc.u32Height);
        if(HI_SUCCESS != s32Out)
        {
            SAMPLE_PRT("HI_MPI_SYS_Munmap return 0x%x!\n", s32Out);
        }
        s32Out = HI_MPI_SYS_MmzFree(stDMASrc.stSrcMem.u32PhyAddr, pVirDMASrc);
        if(HI_SUCCESS != s32Out)
        {
            SAMPLE_PRT("HI_MPI_SYS_MmzFree return 0x%x!\n", s32Out);
        }
    END_7:
        s32Out = HI_MPI_SYS_Munmap(pCannyDstMag, stCannyDstMag.u32Stride * stCannySrc.u32Height * 2);
        if(HI_SUCCESS != s32Out)
        {
            SAMPLE_PRT("HI_MPI_SYS_Munmap return 0x%x!\n", s32Out);
        }            
    END_6:
        s32Out = HI_MPI_SYS_MmzFree(stCannyDstMag.u32PhyAddr, pVirCannyDstMag);
        if(HI_SUCCESS != s32Out)
        {
            SAMPLE_PRT("HI_MPI_SYS_MmzFree return 0x%x!\n", s32Out);
        }
    END_5:
        s32Out = HI_MPI_SYS_MmzFree(stFilterDst.u32PhyAddr, pVirFilterDst);
        if(HI_SUCCESS != s32Out)
        {
            SAMPLE_PRT("HI_MPI_SYS_MmzFree return 0x%x!\n", s32Out);
        }        
    END_4:    
        s32Out = HI_MPI_VI_ReleaseFrame(ExtChn, &stViFrameInfo);
        if(HI_SUCCESS != s32Out)
        {
            SAMPLE_PRT("HI_MPI_VI_ReleaseFrame return 0x%x\n", s32Out);
        }
    }
END_TDE:
    HI_TDE2_Close();
END_3:    
    SAMPLE_IVE_UnBindViVo(VoDev, VoChn + i);
    for(i=0; i < 4; i++)
    {
        s32Out = HI_MPI_VO_DisableChn(VoDev, VoChn +i);
        if(HI_SUCCESS != s32Out)
        {
            SAMPLE_PRT("HI_MPI_VO_DisableChn return 0x%x\n", s32Out);
        }
    }
    
    if (vbPoolHandle != VB_INVALID_POOLID)
    {
        s32Out = HI_MPI_VB_DestroyPool(vbPoolHandle);
        if(HI_SUCCESS != s32Out)
        {
            SAMPLE_PRT("HI_MPI_VB_DestroyPool return 0x%x\n", s32Out);
        }
    }

END_2:
    s32Out = HI_MPI_VI_DisableChn(ExtChn);
    if(HI_SUCCESS != s32Out)
    {
        SAMPLE_PRT("HI_MPI_VI_DisableChn return 0x%x\n", s32Out);
    }
END_1:
    SAMPLE_COMM_VI_StopVi(&g_stViChnConfig);
END_0:
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}

int  main(int argc, char* argv[])
{
    HI_S32 s32Ret = HI_SUCCESS;
    pthread_t iveThread;
    HI_S32 pid;
    HI_CHAR ch;
    
    SAMPLE_IVE_Canny_Usage(argv[0]);
    
    signal(SIGINT, SAMPLE_IVE_Canny_HandleSig);
    signal(SIGTERM, SAMPLE_IVE_Canny_HandleSig);
       
    pid = pthread_create(&iveThread, 0, (HI_VOID*)SAMPLE_IVE_Canny, HI_NULL);

    printf("press 'q' to exit!\n");    
    while(ch != 'q')
    {       
        ch = getchar();
    
        if (ch=='q')
        {        
            g_bStopSignal = HI_TRUE;            
        }
        else
        {
            printf("press 'q' to exit!\n"); 
        }
    }
   
   pthread_join(iveThread, HI_NULL);

    if (HI_SUCCESS == s32Ret)
    {
        SAMPLE_PRT("program exit normally!\n");
    }
    else
    {
        SAMPLE_PRT("program exit abnormally!\n");
    }
    
    exit(s32Ret);
}



