#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "mpi_sys.h"
#include "hi_comm_vb.h"
#include "mpi_vb.h"
#include "hi_comm_vo.h"
#include "mpi_vo.h"

#define MAX_FRM_CNT 256

/* sp420 转存为 p420 ; sp422 转存为 p422  */
void sample_yuv_dump(VIDEO_FRAME_S* pVBuf, FILE* pfd)
{
    unsigned int w, h;
    char* pVBufVirt_Y;
    char* pVBufVirt_C;
    char* pMemContent;
    unsigned char TmpBuff[4096];                //如果这个值太小，图像很大的话存不了
    HI_U32 phy_addr, Ysize, Csize;
    PIXEL_FORMAT_E  enPixelFormat = pVBuf->enPixelFormat;
    HI_U32 u32UvHeight;/* 存为planar 格式时的UV分量的高度 */

    Ysize = (pVBuf->u32Stride[0]) * (pVBuf->u32Height);
    if (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == enPixelFormat)
    {
        Csize = (pVBuf->u32Stride[1]) * (pVBuf->u32Height) / 2;
        u32UvHeight = pVBuf->u32Height / 2;
    }
    else if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 == enPixelFormat)
    {
        Csize = (pVBuf->u32Stride[1]) * (pVBuf->u32Height);
        u32UvHeight = pVBuf->u32Height;
    }
    else
    {
        Csize = 0;
        u32UvHeight = 0;
    }

    phy_addr = pVBuf->u32PhyAddr[0];

    pVBufVirt_Y = (HI_CHAR*) HI_MPI_SYS_Mmap(phy_addr, Ysize);
    if (NULL == pVBufVirt_Y)
    {
        return;
    }

    /* save Y ----------------------------------------------------------------*/
    fprintf(stderr, "saving......Y......");
    fflush(stderr);
    for (h = 0; h < pVBuf->u32Height; h++)
    {
        pMemContent = pVBufVirt_Y + h * pVBuf->u32Stride[0];
        fwrite(pMemContent, pVBuf->u32Width, 1, pfd);
    }
    fflush(pfd);

    if (PIXEL_FORMAT_SINGLE != enPixelFormat)
    {
        pVBufVirt_C = (HI_CHAR*) HI_MPI_SYS_Mmap(pVBuf->u32PhyAddr[1], Csize);
        if (NULL == pVBufVirt_C)
        {
            HI_MPI_SYS_Munmap(pVBufVirt_Y, Ysize);
            return;
        }

        /* save U ----------------------------------------------------------------*/
        fprintf(stderr, "U......");
        fflush(stderr);
        for (h = 0; h < u32UvHeight; h++)
        {
            pMemContent = pVBufVirt_C + h * pVBuf->u32Stride[1];

            pMemContent += 1;

            for (w = 0; w < pVBuf->u32Width / 2; w++)
            {
                TmpBuff[w] = *pMemContent;
                pMemContent += 2;
            }
            fwrite(TmpBuff, pVBuf->u32Width / 2, 1, pfd);
        }
        fflush(pfd);

        /* save V ----------------------------------------------------------------*/
        fprintf(stderr, "V......");
        fflush(stderr);
        for (h = 0; h < u32UvHeight; h++)
        {
            pMemContent = pVBufVirt_C + h * pVBuf->u32Stride[1];

            for (w = 0; w < pVBuf->u32Width / 2; w++)
            {
                TmpBuff[w] = *pMemContent;
                pMemContent += 2;
            }
            fwrite(TmpBuff, pVBuf->u32Width / 2, 1, pfd);
        }
        fflush(pfd);
        HI_MPI_SYS_Munmap(pVBufVirt_C, Csize);
    }
    fprintf(stderr, "done %d!\n", pVBuf->u32TimeRef);
    fflush(stderr);

    HI_MPI_SYS_Munmap(pVBufVirt_Y, Ysize);

}

HI_S32 SAMPLE_MISC_VoDump(VO_LAYER VoLayer, HI_U32 u32Cnt)
{
    HI_S32 i, s32Ret;
    VIDEO_FRAME_INFO_S stFrame;
    //VIDEO_FRAME_INFO_S astFrame[256];
    HI_CHAR szYuvName[128];
    HI_CHAR szPixFrm[10];
    FILE* pfd;

    printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
    printf("usage: ./vou_screen_dump [VoLayer] [frmcnt]. sample: ./vou_screen_dump 0 1\n\n");

    /* Get Frame to make file name*/
    s32Ret = HI_MPI_VO_GetScreenFrame(VoLayer, &stFrame, 0);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VO(%d)_GetScreenFrame errno %#x\n", VoLayer, s32Ret);
        return -1;
    }

    /* make file name */
    if (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == stFrame.stVFrame.enPixelFormat)
    {
        strcpy(szPixFrm, "p420");
    }
    else if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 == stFrame.stVFrame.enPixelFormat)
    {
        strcpy(szPixFrm, "p422");
    }
    else if (PIXEL_FORMAT_SINGLE == stFrame.stVFrame.enPixelFormat)
    {
        strcpy(szPixFrm, "single");
    }
    else
    {
        return -1;
    }

    sprintf(szYuvName, "./vo(%d)_%d_%d_%s_%d.yuv", VoLayer,
            stFrame.stVFrame.u32Width, stFrame.stVFrame.u32Height, szPixFrm, u32Cnt);
    printf("Dump YUV frame of vo(%d) to file: \"%s\"\n", VoLayer, szYuvName);

    HI_MPI_VO_ReleaseScreenFrame(VoLayer, &stFrame);

    /* open file */
    pfd = fopen(szYuvName, "wb");

    if (NULL == pfd)
    {
        return -1;
    }

    /* get VO frame  */
    for (i = 0; i < u32Cnt; i++)
    {
        s32Ret = HI_MPI_VO_GetScreenFrame(VoLayer, &stFrame, 0);
        if (HI_SUCCESS != s32Ret)
        {
            printf("get vo(%d) frame err\n", VoLayer);
            printf("only get %d frame\n", i);
            break;
        }
        /* save VO frame to file */
        sample_yuv_dump(&stFrame.stVFrame, pfd);

        /* release frame after using */
        s32Ret = HI_MPI_VO_ReleaseScreenFrame(VoLayer, &stFrame);
        if (HI_SUCCESS != s32Ret)
        {
            printf("Release vo(%d) frame err\n", VoLayer);
            printf("only get %d frame\n", i);
            break;
        }
    }

    fclose(pfd);

    return 0;
}

HI_S32 main(int argc, char* argv[])
{
    VO_LAYER VoLayer = 0;
    HI_U32 u32FrmCnt = 1;

    /* VO视频层ID*/
    if (argc > 1)
    {
        VoLayer = atoi(argv[1]);
    }
    /* 需要采集的帧数目*/
    if (argc > 2)
    {
        u32FrmCnt = atoi(argv[2]);
    }

    SAMPLE_MISC_VoDump(VoLayer, u32FrmCnt);

    return HI_SUCCESS;
}

