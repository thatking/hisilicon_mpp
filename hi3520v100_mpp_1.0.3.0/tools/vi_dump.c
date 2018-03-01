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
#include "hi_comm_vi.h"
#include "mpi_vi.h"

/* sp420 转存为 p420 ; sp422 转存为 p422  */
void sample_yuv_dump(VIDEO_FRAME_S * pVBuf, FILE *pfd)
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

HI_S32 SAMPLE_MISC_ViDump(VI_CHN ViChn, HI_U32 u32Cnt)
{	
    HI_S32 i, s32Ret;
    VIDEO_FRAME_INFO_S *pstFrame = NULL;
    HI_CHAR szYuvName[128];
    HI_CHAR szPixFrm[10];
    FILE *pfd;  
    
    printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
    printf("\t: please make sure there is at least %d vb blocks of the chn%d's picture size!\n",
        u32Cnt, ViChn);
    printf("usage: ./vi_dump [vichn] [frmcnt]. sample: ./vi_dump 0 5\n\n");

    if (HI_MPI_VI_SetFrameDepth(ViChn, VIU_MAX_USER_FRAME_DEPTH))
    {        
        printf("HI_MPI_VI_SetFrameDepth err, vi chn %d \n", ViChn);
        return -1;
    }
    
    pstFrame = (VIDEO_FRAME_INFO_S*)malloc(u32Cnt * sizeof(VIDEO_FRAME_INFO_S));
    if (NULL == pstFrame)
    {
        printf("malloc err, vi chn %d \n", ViChn);
        return -1;
    }

    /* get VI frame  */    
    for (i=0; i<u32Cnt; i++)
    {
        s32Ret = HI_MPI_VI_GetFrame(ViChn, &pstFrame[i]);
        if (HI_SUCCESS != s32Ret)
        {        
            printf("get vi chn %d frame err\n", ViChn);
            printf("only get %d frames, the number of vb blocks maybe is not enough\n", i);
            break;
        }
    }
    
    /* make file name */
    strcpy(szPixFrm, 
        (PIXEL_FORMAT_YUV_SEMIPLANAR_420 == pstFrame[0].stVFrame.enPixelFormat)?"p420":"p422");    
    sprintf(szYuvName, "./vi chn %d_%d_%d_%s_%d.yuv", ViChn,
        pstFrame[0].stVFrame.u32Width, pstFrame[0].stVFrame.u32Height,szPixFrm,u32Cnt);        
	printf("Dump YUV frame of vi chn %d  to file: \"%s\"\n", ViChn, szYuvName);
    
    /* open file */
    pfd = fopen(szYuvName, "wb");
    if (NULL == pfd)
    {        
        printf("fopen %s err\n", szYuvName);
        free(pstFrame);
        return -1;
    }

    for (i=0; i<u32Cnt; i++)
    {
        /* save VI frame to file */
		sample_yuv_dump(&pstFrame[i].stVFrame, pfd);
        
        /* release frame after using */
        s32Ret = HI_MPI_VI_ReleaseFrame(ViChn, &pstFrame[i]);
        if (HI_SUCCESS != s32Ret)
        {
            printf("Release vi chn %d frame err\n", ViChn);
            break;
        }
    }

    fclose(pfd);
    free(pstFrame);
	return 0;
}

HI_S32 main(int argc, char *argv[])
{	 
    VI_CHN ViChn = 0;
    HI_U32 u32FrmCnt = 1;

    if (argc > 1)/* VI通道号*/
    {
        ViChn = atoi(argv[1]);
    }
    
    if (argc > 2)
    {
        u32FrmCnt = atoi(argv[2]);/* 需要采集的帧数目*/
    } 
    
    SAMPLE_MISC_ViDump(ViChn, u32FrmCnt);

	return HI_SUCCESS;
}

