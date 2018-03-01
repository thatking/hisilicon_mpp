#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
    
#include "mpi_hdmi.h"
#include "hi_comm_hdmi.h"

#define VO_HDMI_REG 0x201f0000l
#define VO_HDMI_CSC_REG_SIZE 0x10000l


typedef struct 
{
    HI_S32 csc_coef00;
    HI_S32 csc_coef01;
    HI_S32 csc_coef02;

    HI_S32 csc_coef10;
    HI_S32 csc_coef11;
    HI_S32 csc_coef12;

    HI_S32 csc_coef20;
    HI_S32 csc_coef21;
    HI_S32 csc_coef22;

    HI_S32 csc_in_dc0;
    HI_S32 csc_in_dc1;
    HI_S32 csc_in_dc2;

    HI_S32 csc_out_dc0;
    HI_S32 csc_out_dc1;
    HI_S32 csc_out_dc2;
} HDMI_CSC_COEF_S;

/* RGB->YUV601 常量系数矩阵 */
const HDMI_CSC_COEF_S g_stCSC_RGB2YUV601_pc
#if 0
    = {    
     /*csc系数*/
     299, 587, 114, -172, -339, 511, 511, -428, -83,
     /*csc输入直流(IDC)*/
     0, 0, 0,
     /*csc输出直流(ODC)*/
     0, 128, 128,
    };
#else
    = {
     /*csc系数*/
     257, 504, 98, -148, -291, 439, 439, -368, -71,
     /*csc输入直流(IDC)*/
     0, 0, 0,
     /*csc输出直流(ODC)*/
     16, 128, 128, 
    };
#endif
/* RGB->YUV709 常量系数矩阵 */
const HDMI_CSC_COEF_S g_stCSC_RGB2YUV709_pc
#if 0
    = {
     /*csc系数*/
     213, 715, 72, -117, -394, 511, 511, -464, -47,
     /*csc输入直流(IDC)*/
     0, 0, 0,
     /*csc输出直流(ODC)*/
     0, 128, 128,
    };
#else
    = {
     /*csc系数*/
     183, 614, 62, -101, -338, 439, 439, -399, -40,
     /*csc输入直流(IDC)*/
     0, 0, 0,
     /*csc输出直流(ODC)*/
     16, 128, 128,
    };
#endif

const int SIN_TABLE[61] = {
  -500,  -485,  -469,  -454,  -438,  -422,  -407,  -391,  -374,  -358,
  -342,  -325,  -309,  -292,  -276,  -259,  -242,  -225,  -208,  -191,
  -174,  -156,  -139,  -122,  -104,   -87,   -70,   -52,   -35,   -17,
     0,    17,    35,    52,    70,    87,   104,   122,   139,   156,
   174,   191,   208,   225,   242,   259,   276,   292,   309,   325,
   342,   358,   374,   391,   407,   422,   438,   454,   469,   485,
   500};

const int COS_TABLE[61] = {
   866,   875,   883,   891,   899,   906,   914,   921,   927,   934,
   940,   946,   951,   956,   961,   966,   970,   974,   978,   982,
   985,   988,   990,   993,   995,   996,   998,   999,   999,  1000,
  1000,  1000,   999,   999,   998,   996,   995,   993,   990,   988,
   985,   982,   978,   974,   970,   966,   961,   956,   951,   946,
   940,   934,   927,   921,   914,   906,   899,   891,   883,   875,
   866};
   
static HI_S32 s_s32MemDev = 0;

HI_S32 memopen( void )
{
   if (s_s32MemDev <= 0)
   {
       s_s32MemDev = open ("/dev/mem", O_CREAT|O_RDWR|O_SYNC);
       if (s_s32MemDev <= 0)
       {
           return -1;
       }
   }
   return 0;
}

HI_VOID memclose()
{
   close(s_s32MemDev);
}

void * memmap( HI_U32 u32PhyAddr, HI_U32 u32Size )
{
   HI_U32 u32Diff;
   HI_U32 u32PagePhy;
   HI_U32 u32PageSize;
   HI_U8 * pPageAddr;

   u32PagePhy = u32PhyAddr & 0xfffff000;
   u32Diff = u32PhyAddr - u32PagePhy;

   /* size in page_size */
   u32PageSize = ((u32Size + u32Diff - 1) & 0xfffff000) + 0x1000;
   pPageAddr = mmap ((void *)0, u32PageSize, PROT_READ|PROT_WRITE,
                                   MAP_SHARED, s_s32MemDev, u32PagePhy);
   if (MAP_FAILED == pPageAddr )
   {
           return NULL;
   }
   return (void *) (pPageAddr + u32Diff);
}

HI_S32 memunmap(HI_VOID* pVirAddr, HI_U32 u32Size )
{
   HI_U32 u32PageAddr;
   HI_U32 u32PageSize;
   HI_U32 u32Diff;

   u32PageAddr = (((HI_U32)pVirAddr) & 0xfffff000);
   /* size in page_size */
   u32Diff     = (HI_U32)pVirAddr - u32PageAddr;
   u32PageSize = ((u32Size + u32Diff - 1) & 0xfffff000) + 0x1000;

   return munmap((HI_VOID*)u32PageAddr, u32PageSize);
}

HI_VOID usage(HI_VOID)
{    
    printf("\nNOTICE: This tool only can be used for TESTING !!!\n");      
    printf("NOTICE: Luma/Contrast/Hue/Satuature [0,100], enCscMatrix [5,6]\n");
    printf("usage: ./vo_hdmi_config [enCscMatrix] [Luma] [Contrast] [Hue] [Satuature]. sample: ./vo_hdmi_config 5 50 50 50 50\n\n");
}
HI_U32 inline vo_hdmi_ConverCscCoef(HI_S32 s32Value)
{
    HI_S32 s32Result = ((s32Value << 14) / 1000);
    
    if (s32Result < 0)
    {
        s32Result = (-1)*s32Result;
    }
    
    s32Result = s32Result & 0xffff;
    
    return s32Result;
}
HI_U32 inline vo_hdmi_ConverCscOffset(HI_S32 s32Value)
{
    HI_S32 s32Result = (s32Value << 6);
    #if 1
    if (s32Result < 0)
    {
        s32Result = (-1)*s32Result;
    }
    #else
    if (s32Result < 0)
    {
        s32Result = (~((-1)*s32Result) + 1);
    }        
    #endif
    s32Result = s32Result & 0xffff;
    
    return s32Result;
}

HI_S32 vo_hdmi_CalcCscMatrix(HI_U32 u32Luma, HI_U32 u32Contrast,
    HI_U32 u32Hue, HI_U32 u32Satuature, const HDMI_CSC_COEF_S *pstCoefInput,
    HDMI_CSC_COEF_S *pstCoefOutput)
{
    HI_S32 s32Luma     = 0;
    HI_S32 s32Contrast = 0;
    HI_S32 s32Hue      = 0;
    HI_S32 s32Satu     = 0;
    
    s32Luma     = (HI_S32)u32Luma * 64 / 100 - 32;
    s32Contrast = ((HI_S32)u32Contrast - 50) * 2 + 100;
    s32Hue      = (HI_S32)u32Hue * 60 / 100;
    s32Satu     = ((HI_S32)u32Satuature - 50) * 2 + 100;
   
    pstCoefOutput->csc_in_dc0 = pstCoefInput->csc_in_dc0;
    pstCoefOutput->csc_in_dc1 = pstCoefInput->csc_in_dc1;
    pstCoefOutput->csc_in_dc2 = pstCoefInput->csc_in_dc2;
    pstCoefOutput->csc_out_dc0 = pstCoefInput->csc_out_dc0;
    pstCoefOutput->csc_out_dc1 = pstCoefInput->csc_out_dc1;
    pstCoefOutput->csc_out_dc2 = pstCoefInput->csc_out_dc2;

    /* C_ratio的调节范围一般是0～1.99, C_ratio=s32Contrast/100
    *  S的调节范围一般为0~1.99,S=s32Satu/100
    *  色调调节参数的范围一般为-30°~30°,通过查表法求得COS和SIN值并/1000
    */
    /* 此公式仅用于RGB->YUV转换，YUV->RGB转换不可用此公式，
    *  YUV->YUV仅调节图像效果可用此公式，因为常量矩阵为单位矩阵 */
    pstCoefOutput->csc_coef00 = (s32Contrast * pstCoefInput->csc_coef00) / 100;
    pstCoefOutput->csc_coef01 = (s32Contrast * pstCoefInput->csc_coef01) / 100;
    pstCoefOutput->csc_coef02 = (s32Contrast * pstCoefInput->csc_coef02) / 100;
    pstCoefOutput->csc_coef10 = (s32Contrast * s32Satu * ((pstCoefInput->csc_coef10*COS_TABLE[s32Hue]
        + pstCoefInput->csc_coef20*SIN_TABLE[s32Hue]) / 1000)) / 10000;
    pstCoefOutput->csc_coef11 = (s32Contrast * s32Satu * ((pstCoefInput->csc_coef11*COS_TABLE[s32Hue]
        + pstCoefInput->csc_coef21*SIN_TABLE[s32Hue]) / 1000)) / 10000;
    pstCoefOutput->csc_coef12 = (s32Contrast * s32Satu * ((pstCoefInput->csc_coef12*COS_TABLE[s32Hue]
        + pstCoefInput->csc_coef22*SIN_TABLE[s32Hue]) / 1000)) / 10000;
    pstCoefOutput->csc_coef20 = (s32Contrast * s32Satu * ((pstCoefInput->csc_coef20*COS_TABLE[s32Hue]
        - pstCoefInput->csc_coef10*SIN_TABLE[s32Hue]) / 1000)) / 10000;
    pstCoefOutput->csc_coef21 = (s32Contrast * s32Satu * ((pstCoefInput->csc_coef21*COS_TABLE[s32Hue]
        - pstCoefInput->csc_coef11*SIN_TABLE[s32Hue]) / 1000)) / 10000;
    pstCoefOutput->csc_coef22 = (s32Contrast * s32Satu * ((pstCoefInput->csc_coef22*COS_TABLE[s32Hue]
        - pstCoefInput->csc_coef12*SIN_TABLE[s32Hue]) / 1000)) / 10000;
    pstCoefOutput->csc_out_dc0 += s32Luma;
    
    return HI_SUCCESS;
}

HI_U32 ReadByteHDMITXP0(HI_U8* SlaveAddr, HI_U8 RegAddr)
{
    HI_U8 Data = 0;
    volatile HI_U32 *pu32VirAddr = HI_NULL;
    pu32VirAddr = (volatile HI_U32 *)(SlaveAddr + (0x000+RegAddr)*4);
    Data = (*pu32VirAddr) & 0xff;
    return Data;
}

HI_VOID WriteByteHDMITXP0(HI_U8* SlaveAddr, HI_U8 RegAddr, HI_U32 Data)
{
	volatile HI_U32 *pu32VirAddr = HI_NULL;
    
    pu32VirAddr = (volatile HI_U32 *)(SlaveAddr + (0x00+RegAddr)*4);
    pu32VirAddr[0] = Data&0xff;
    
    return;
}

HI_S32 vo_hdmi_setCSC(HI_U32 u32Luma, HI_U32 u32Contrast,
    HI_U32 u32Hue, HI_U32 u32Satuature, const HDMI_CSC_COEF_S *pstCoefInput,
    HI_U32 u32MatrixSel)
{
    HI_U32 reg, u32Tmp;
    HDMI_CSC_COEF_S stCoefOutput;
    HI_U8  *pAddr = NULL;
    
    if (0 == u32MatrixSel)
    {
        
        memopen();
        pAddr = (HI_U8 *)memmap(VO_HDMI_REG, VO_HDMI_CSC_REG_SIZE);
        /* 用配置系数替代逻辑中的系数 */
        reg = ReadByteHDMITXP0(pAddr,0x50);
        reg |= 0x5;
        WriteByteHDMITXP0(pAddr,0x50, (HI_U8)reg);

        vo_hdmi_CalcCscMatrix(u32Luma, u32Contrast, u32Hue, u32Satuature,
            pstCoefInput, &stCoefOutput);
        
        u32Tmp = vo_hdmi_ConverCscCoef(stCoefOutput.csc_coef00);
        WriteByteHDMITXP0(pAddr,0x51, (HI_U8)(u32Tmp & 0xff));    // R2Y
        WriteByteHDMITXP0(pAddr,0x52, (HI_U8)((u32Tmp & 0xff00) >> 8));
        u32Tmp = vo_hdmi_ConverCscCoef(stCoefOutput.csc_coef01);
        WriteByteHDMITXP0(pAddr,0x53, (HI_U8)(u32Tmp & 0xff));    // G2Y
        WriteByteHDMITXP0(pAddr,0x54, (HI_U8)((u32Tmp & 0xff00) >> 8));
        u32Tmp = vo_hdmi_ConverCscCoef(stCoefOutput.csc_coef02);
        WriteByteHDMITXP0(pAddr,0x55, (HI_U8)(u32Tmp & 0xff));    // B2Y
        WriteByteHDMITXP0(pAddr,0x56, (HI_U8)((u32Tmp & 0xff00) >> 8));
        u32Tmp = vo_hdmi_ConverCscCoef(stCoefOutput.csc_coef10);
        WriteByteHDMITXP0(pAddr,0x57, (HI_U8)(u32Tmp & 0xff));    // R2Cb
        WriteByteHDMITXP0(pAddr,0x58, (HI_U8)((u32Tmp & 0xff00) >> 8));
        u32Tmp = vo_hdmi_ConverCscCoef(stCoefOutput.csc_coef11);
        WriteByteHDMITXP0(pAddr,0x59, (HI_U8)(u32Tmp & 0xff));    // G2Cb
        WriteByteHDMITXP0(pAddr,0x5a, (HI_U8)((u32Tmp & 0xff00) >> 8));
        u32Tmp = vo_hdmi_ConverCscCoef(stCoefOutput.csc_coef12);
        WriteByteHDMITXP0(pAddr,0x5b, (HI_U8)(u32Tmp & 0xff));    // B2Cb
        WriteByteHDMITXP0(pAddr,0x5c, (HI_U8)((u32Tmp & 0xff00) >> 8));
        u32Tmp = vo_hdmi_ConverCscCoef(stCoefOutput.csc_coef20);
        WriteByteHDMITXP0(pAddr,0x5d, (HI_U8)(u32Tmp & 0xff));    // R2Cr
        WriteByteHDMITXP0(pAddr,0x5e, (HI_U8)((u32Tmp & 0xff00) >> 8));
        u32Tmp = vo_hdmi_ConverCscCoef(stCoefOutput.csc_coef21);
        WriteByteHDMITXP0(pAddr,0x5f, (HI_U8)(u32Tmp & 0xff));    // G2Cr
        WriteByteHDMITXP0(pAddr,0x60, (HI_U8)((u32Tmp & 0xff00) >> 8));
        u32Tmp = vo_hdmi_ConverCscCoef(stCoefOutput.csc_coef22);
        WriteByteHDMITXP0(pAddr,0x61, (HI_U8)(u32Tmp & 0xff));    // B2Cr
        WriteByteHDMITXP0(pAddr,0x62, (HI_U8)((u32Tmp & 0xff00) >> 8));
        u32Tmp = vo_hdmi_ConverCscOffset(stCoefOutput.csc_in_dc0);
        WriteByteHDMITXP0(pAddr,0x63, (HI_U8)(u32Tmp & 0xff));    // RGB_OFFSET
        WriteByteHDMITXP0(pAddr,0x64, (HI_U8)((u32Tmp & 0xff00) >> 8));
        u32Tmp = vo_hdmi_ConverCscOffset(stCoefOutput.csc_out_dc0);
        WriteByteHDMITXP0(pAddr,0x65, (HI_U8)(u32Tmp & 0xff));    // Y_OFFSET
        WriteByteHDMITXP0(pAddr,0x66, (HI_U8)((u32Tmp & 0xff00) >> 8)); 
        u32Tmp = vo_hdmi_ConverCscOffset(stCoefOutput.csc_out_dc1);
        WriteByteHDMITXP0(pAddr,0x67, (HI_U8)(u32Tmp & 0xff));    // CbCr_OFFSET
        WriteByteHDMITXP0(pAddr,0x68, (HI_U8)((u32Tmp & 0xff00) >> 8));

        memunmap(pAddr, VO_HDMI_CSC_REG_SIZE);
        memclose();
    }
    else
    {
    }
    
    return HI_SUCCESS;
}

HI_S32 vo_hdmi_config(HI_U32 enCscMatrix , HI_U32 u32Luma , HI_U32 u32Contrast , HI_U32 u32Hue , HI_U32 u32Satuature)
{
    
    HI_HDMI_CSC_S stCSC;
    const HDMI_CSC_COEF_S *pstCoefInput;
    
    //usage();
    
    if (u32Contrast < 0 || u32Contrast > 100)
    {
        usage();
        return -1;
    }
    if (u32Hue < 0 || u32Hue > 100)
    {
        usage();
        return -1;
    }
    if (u32Luma < 0 || u32Luma > 100)
    {
        usage();
        return -1;
    }
    if (u32Satuature < 0 || u32Satuature > 100)
    {
        usage();
        return -1;
    }
    if (enCscMatrix < 5 || enCscMatrix > 6)
    {
        usage();
        return -1;
    }

    stCSC.enCscMatrix = enCscMatrix;
    stCSC.u32Contrast = u32Contrast;
    stCSC.u32Satuature = u32Satuature;
    stCSC.u32Luma = u32Luma;
    stCSC.u32Hue = u32Hue;
    
    if (HI_HDMI_CSC_MATRIX_RGB_TO_BT601_PC == stCSC.enCscMatrix)
    {
        pstCoefInput = &g_stCSC_RGB2YUV601_pc;
    }
    else if(HI_HDMI_CSC_MATRIX_RGB_TO_BT709_PC == stCSC.enCscMatrix)
       
    {
        pstCoefInput = &g_stCSC_RGB2YUV709_pc;
    }
    else
    {
        printf("Csc matrix should be rgb_to_601\rgb_to_709\rgb_to_rgb when rgb444 input!\n");
        return -1;
    }
    
    vo_hdmi_setCSC(stCSC.u32Luma,
                stCSC.u32Contrast,
                stCSC.u32Hue,
                stCSC.u32Satuature,
                pstCoefInput, 0);    
    return 0;
}

HI_S32 main(int argc, char *argv[])
{
    HI_U32 enCscMatrix; 
    HI_U32 u32Luma;
    HI_U32 u32Contrast;
    HI_U32 u32Hue;
    HI_U32 u32Satuature;       

	if (argc > 1)
    {
        enCscMatrix = atoi(argv[1]);
    }

	if (argc > 2)
    {
        u32Luma = atoi(argv[2]);
    }

	if (argc > 3)
    {
        u32Contrast = atoi(argv[3]);
    }

    if (argc > 4)
    {
        u32Hue = atoi(argv[4]);
    }

    if (argc > 5)
    {
        u32Satuature = atoi(argv[5]);
    }

    vo_hdmi_config(enCscMatrix, u32Luma, u32Contrast, u32Hue, u32Satuature);

	return HI_SUCCESS;
}


