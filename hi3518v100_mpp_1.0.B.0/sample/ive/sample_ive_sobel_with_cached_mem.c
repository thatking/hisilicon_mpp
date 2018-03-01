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

#define WRITE_DATA_TO_FILE 0

typedef struct ST_IMAGE
{
    HI_S32  s32Width;                  
    HI_S32  s32Height;                
    HI_S32  s32WidthStep;             
    HI_U8*  puImageData;          
    HI_U32  u32PhyDataAddr;         
    HI_U32  u32Reserved;	
}ST_IMAGE;


HI_S32 SAMPLE_IVE_WriteTestData(HI_U8 *pucWinName, HI_U8 *pucSrcData, HI_U32 uiDataLength, HI_U8 ucFileAttribute, HI_U8 ucPrintAttribute)
{
    FILE *fpFile = NULL;
        
    fpFile  = fopen((const char *)pucWinName, "ab+");
    if ( !fpFile )
    {
        printf( "Couldn't open %s\n", pucWinName);
        return HI_FAILURE;
    }
    fwrite((const void *)pucSrcData, uiDataLength, 1, fpFile);
    fclose(fpFile);

    return HI_SUCCESS;
}

void SAMPLE_IVE_WriteData2File(HI_CHAR* fileName, HI_VOID* pData, HI_VOID* pData2, HI_U32 u32Width, HI_U32 u32Height, HI_U32 u32Stride)
{
    int i,j;
        
    FILE *fp = fopen(fileName, "wb+");
  
    for(i=0; i<u32Height; i++)
    {
        HI_U8 *pH   = (HI_U8*)pData + i * u32Stride;
        for(j=0; j<u32Width; j++)
        {
            fprintf(fp, "%d, ", pH[j]);

        }
        fprintf(fp, "\n");
    }
    if(pData2)
    {
        for(i=0; i<u32Height; i++)
        {
            HI_U8 *pH   = (HI_U8*)pData + i * u32Stride;
            for(j=0; j<u32Width; j++)
            {
                fprintf(fp, "%d, ", pH[j]);

            }
            fprintf(fp, "\n");
        }
    }

    fclose(fp);
}


/******************************************************************************
* function : show usage
******************************************************************************/
HI_VOID SAMPLE_IVE_Sobel_Usage(char *sPrgNm)
{
    printf("This example demonstrates the usage of Sobel detector, also the usage of Cached DDR momery!\n");
    printf("If we don't flush the memory properly, when we run this sample twice, the second sobel's output 'H & V'\n");
    printf("could not be the same with the first running time!\n");
    printf("Usage : %s \n", sPrgNm);
    
    return;
}


/******************************************************************************
* function : to process abnormal case
******************************************************************************/
HI_VOID SAMPLE_IVE_Sobel_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo)
    {
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

/******************************************************************************
* function : sobel
******************************************************************************/
HI_S32 SAMPLE_IVE_Sobel()
{     
    HI_S32 i = 0, j= 0;
    HI_S32 s32Ret;
    ST_IMAGE stSobelH;
    ST_IMAGE stSobelV;
    ST_IMAGE stSrc1;
    ST_IMAGE stSrc2;
    IVE_SRC_INFO_S stSrc;
    IVE_MEM_INFO_S stDstH;
    IVE_MEM_INFO_S stDstV;
    HI_S8 as8MaskH[9] = {-1,0,1,-2,0,2,-1,0,1};//mask
    HI_U8* pucVirtualAddr;
    HI_U32 uiPhyAddr;
    HI_S32 iImgSize = 64*64;
    HI_S32 iIveHandle;
    IVE_SOBEL_CTRL_S stSobelCtrl;
    HI_BOOL bFinish;
    HI_U8  *psum;
    HI_S16 *pH;
    HI_S16 *pV;

    VB_CONF_S stVbConf;

    memset(&stVbConf,0,sizeof(VB_CONF_S));     
   
    stVbConf.u32MaxPoolCnt = 128;
     
    /*ddr0 video buffer*/
    stVbConf.astCommPool[0].u32BlkSize = 720 * 576;
    stVbConf.astCommPool[0].u32BlkCnt =  8;
     
    stVbConf.astCommPool[1].u32BlkSize = 1280 * 720;
    stVbConf.astCommPool[1].u32BlkCnt = 8;

    SAMPLE_COMM_SYS_Init(&stVbConf);
    
    printf("\nSys init ok!\n");

    s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&uiPhyAddr, (void **)&pucVirtualAddr, 
        "User", HI_NULL, 64* 64 *2);
    stSobelH.s32Width = 64;
    stSobelH.s32Height = 64;
    stSobelH.puImageData = pucVirtualAddr;
    stSobelH.u32PhyDataAddr = uiPhyAddr;

    s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&uiPhyAddr, (void **)&pucVirtualAddr, 
        "User", HI_NULL, 64* 64 *2);
    stSobelV.s32Width = 64;
    stSobelV.s32Height = 64;
    stSobelV.puImageData = pucVirtualAddr;
    stSobelV.u32PhyDataAddr = uiPhyAddr;

    s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&uiPhyAddr, (void **)&pucVirtualAddr, 
        "User", HI_NULL, 64* 64);
    stSrc1.s32Width = 64;
    stSrc1.s32Height = 64;
    stSrc1.puImageData = pucVirtualAddr;
    stSrc1.u32PhyDataAddr = uiPhyAddr;

    s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&uiPhyAddr, (void **)&pucVirtualAddr, 
        "User", HI_NULL, 64* 64);
    stSrc2.s32Width = 64;
    stSrc2.s32Height = 64;
    stSrc2.puImageData = pucVirtualAddr;
    stSrc2.u32PhyDataAddr = uiPhyAddr;

    printf("stSobelW Phy = 0x%x, stSobelW Virtual = 0x%x\n", stSobelH.u32PhyDataAddr, stSobelH.puImageData);
    printf("stSobelH Phy = 0x%x, stSobelH Virtual = 0x%x\n", stSobelV.u32PhyDataAddr, stSobelV.puImageData);
    printf("stSrc1 Phy = 0x%x, stSrc1 Virtual = 0x%x\n", stSrc1.u32PhyDataAddr, stSrc1.puImageData);
    printf("stSrc2 Phy = 0x%x, stSrc2 Virtual = 0x%x\n", stSrc2.u32PhyDataAddr, stSrc2.puImageData);

    memset(stSrc1.puImageData, 212, iImgSize);
    memset(stSrc2.puImageData, 10, iImgSize);

    /* read the input image */
    {
        HI_U8 *Databuf1, *Databuf2;
        FILE *fpFile;
        Databuf1 = (HI_U8 *)malloc(64*64);
        fpFile  = fopen("./input/BG.yuv", "r");
        fread(Databuf1, 1, 64*64, fpFile);
        memcpy(stSrc1.puImageData, Databuf1, 64*64);
        Databuf2 = (HI_U8 *)malloc(64*64);
        fpFile  = fopen("./input/FG.yuv", "r");
        fread(Databuf2, 1, 64*64, fpFile);
        memcpy(stSrc2.puImageData, Databuf2, 64*64);
        free(Databuf1);
        free(Databuf2);
    }

    
#if WRITE_DATA_TO_FILE
    SAMPLE_IVE_WriteTestData("./output/stSrc1.txt",stSrc1.puImageData, 64*64,0,1);
    SAMPLE_IVE_WriteTestData("./outpu/stSrc2.txt",stSrc2.puImageData, 64*64,0,1);
#else
    SAMPLE_IVE_WriteData2File("./output/stSrc1.txt", stSrc1.puImageData, HI_NULL, 64, 64, 64);
    SAMPLE_IVE_WriteData2File("./output/stSrc2.txt", stSrc2.puImageData, HI_NULL, 64, 64, 64);
#endif 

    s32Ret = HI_MPI_SYS_MmzFlushCache(stSrc1.u32PhyDataAddr, stSrc1.puImageData, iImgSize);
    if (s32Ret)
    {
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_SYS_MmzFlushCache(stSrc2.u32PhyDataAddr, stSrc2.puImageData, iImgSize);
    if (s32Ret)
    {
        return HI_FAILURE;
    }

    /* The fisrt Sobel */
    stSrc.enSrcFmt = IVE_SRC_FMT_SINGLE;
    stSrc.stSrcMem.u32Stride = 64;
    stSrc.stSrcMem.u32PhyAddr = (HI_U32)stSrc1.u32PhyDataAddr;
    stSrc.u32Height = (HI_U32)64;
    stSrc.u32Width = (HI_U32)64;

    stDstH.u32Stride = 64;
    stDstH.u32PhyAddr = (HI_U32)stSobelH.u32PhyDataAddr;

    stDstV.u32Stride = 64;
    stDstV.u32PhyAddr = (HI_U32)stSobelV.u32PhyDataAddr;

    for(i = 0; i < 9; i++)
    {
        stSobelCtrl.as8Mask[i] = as8MaskH[i];
    }

    s32Ret =  HI_MPI_IVE_SOBEL(&iIveHandle, &stSrc, &stDstH, &stDstV, &stSobelCtrl, HI_TRUE);
    if (s32Ret)
    {
        return HI_FAILURE;
    }

    bFinish = HI_FALSE;
    s32Ret = HI_MPI_IVE_Query((IVE_HANDLE)(iIveHandle), &bFinish, HI_TRUE);
    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }
    
    /* The second Sobel */
#if WRITE_DATA_TO_FILE
    SAMPLE_IVE_WriteTestData("./output/pstSobleH1.txt",stSobelH.puImageData, 2*64*64,0,1);
    SAMPLE_IVE_WriteTestData("./output/pstSobleV1.txt",stSobelV.puImageData, 2*64*64,0,1);
#else
    SAMPLE_IVE_WriteData2File("./output/pstSobleH1&V1.txt", stSobelH.puImageData, stSobelV.puImageData, 2*64, 64, 2*64);
#endif 

    //回写操作！！！！！！！！！！！！！！！！！！！！！！！！！！！！
    printf("回写操作！！！\n");
    psum = (HI_U8 *)stSrc1.puImageData;
    pH   = (HI_S16*)stSobelH.puImageData;
    pV   = (HI_S16*)stSobelV.puImageData;
    for(i=0; i<64; i++)
    {
        for(j=0; j<64; j++)
        {
            *psum++ =  ((*pH++) + (*pV++))>>1;
        }
    }

#if 1
    s32Ret = HI_MPI_SYS_MmzFlushCache(HI_NULL, HI_NULL, 0);
    if (s32Ret)
    {
        return HI_FAILURE;
    }
#endif

    //第二次Sobel操作 开始！！！！！！！！！！！！！！！！！！！！！！！！！！！！
    stSrc.enSrcFmt = IVE_SRC_FMT_SINGLE;
    stSrc.stSrcMem.u32Stride = 64;
    stSrc.stSrcMem.u32PhyAddr = (HI_U32)stSrc2.u32PhyDataAddr;
    stSrc.u32Height = (HI_U32)64;
    stSrc.u32Width = (HI_U32)64;

    stDstH.u32Stride = 64;
    stDstH.u32PhyAddr = (HI_U32)stSobelH.u32PhyDataAddr;

    stDstV.u32Stride = 64;
    stDstV.u32PhyAddr = (HI_U32)stSobelV.u32PhyDataAddr;

    for(i = 0; i < 9; i++)
    {
        stSobelCtrl.as8Mask[i] = as8MaskH[i];
    }

    s32Ret =  HI_MPI_IVE_SOBEL(&iIveHandle, &stSrc, &stDstH, &stDstV, &stSobelCtrl, HI_TRUE);
    if (s32Ret)
    {
        return HI_FAILURE;
    }

    bFinish = HI_FALSE;
    s32Ret = HI_MPI_IVE_Query((IVE_HANDLE)(iIveHandle), &bFinish, HI_TRUE);
    if (HI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }
    //第二次Sobel操作 结束！！！！！！！！！！！！！！！！！！！！！！！！！！！！
#if WRITE_DATA_TO_FILE
    SAMPLE_IVE_WriteTestData("./output/pstSobleH2.txt",stSobelH.puImageData, 2*64*64,0,1);
    SAMPLE_IVE_WriteTestData("./output/pstSobleV2.txt",stSobelV.puImageData, 2*64*64,0,1);
#else
    SAMPLE_IVE_WriteData2File("./output/pstSobleH2&V2.txt", stSobelH.puImageData, stSobelV.puImageData, 2*64, 64, 2*64);
#endif

    printf("Free!!!!!!!!\n");

    HI_MPI_SYS_MmzFree((HI_U32)(stSobelH.u32PhyDataAddr), (void *)(stSobelH.puImageData));
    HI_MPI_SYS_MmzFree((HI_U32)(stSobelV.u32PhyDataAddr), (void *)(stSobelV.puImageData));
    HI_MPI_SYS_MmzFree((HI_U32)(stSrc1.u32PhyDataAddr), (void *)(stSrc1.puImageData));
    HI_MPI_SYS_MmzFree((HI_U32)(stSrc2.u32PhyDataAddr), (void *)(stSrc2.puImageData));

    printf("The end!!!!!!\n"); 

    return s32Ret;

}

HI_S32 main(HI_S32 argc, HI_CHAR *argv[])
{
    HI_S32 s32Ret = HI_SUCCESS;
    
    SAMPLE_IVE_Sobel_Usage(argv[0]);
    signal(SIGINT, SAMPLE_IVE_Sobel_HandleSig);
    signal(SIGTERM, SAMPLE_IVE_Sobel_HandleSig);

    s32Ret = SAMPLE_IVE_Sobel();

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
