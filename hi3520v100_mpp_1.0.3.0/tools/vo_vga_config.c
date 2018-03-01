/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : vga_csc.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2012/03/15
  Description   : 
  History       :
  1.Date        : 2012/03/15
    Author      : n00168968
    Modification: Created file

******************************************************************************/
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

#define VO_VGA_CSC_REG 0X205CCE00
#define VO_VGA_CSC_REG_SIZE 0X10000

#define VGACSCIDC   0X205CCE24
#define VGACSCODC   0X205CCE28
#define VGACSCP0    0X205CCE2C
#define VGACSCP1    0X205CCE30
#define VGACSCP2    0X205CCE34
#define VGACSCP3    0X205CCE38
#define VGACSCP4    0X205CCE3C

#define DACCTRL3_5  0x205CCEE4

typedef enum hiHAL_CSC_MODE_E
{
    HAL_CSC_MODE_NONE = 0,

    HAL_CSC_MODE_BT601_TO_BT601,
    HAL_CSC_MODE_BT709_TO_BT709,
    HAL_CSC_MODE_RGB_TO_RGB,
    
    HAL_CSC_MODE_BT601_TO_BT709,
    HAL_CSC_MODE_BT709_TO_BT601,
    
    HAL_CSC_MODE_BT601_TO_RGB_PC,   
    HAL_CSC_MODE_BT709_TO_RGB_PC,
    HAL_CSC_MODE_RGB_TO_BT601_PC,
    HAL_CSC_MODE_RGB_TO_BT709_PC,
    
    HAL_CSC_MODE_BT601_TO_RGB_TV, 
    HAL_CSC_MODE_BT709_TO_RGB_TV,
    HAL_CSC_MODE_RGB_TO_BT601_TV,
    HAL_CSC_MODE_RGB_TO_BT709_TV,
    
    HAL_CSC_MODE_BUTT
} HAL_CSC_MODE_E;

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
} CscCoef_S;

/* RGB->YUV601 常量系数矩阵 */
const CscCoef_S g_stCSC_RGB2YUV601_tv        
    = {
     /*csc系数*/
     257, 504, 98, -148, -291, 439, 439, -368, -71,
     /*csc输入直流(IDC)*/
     0, 0, 0,
     /*csc输出直流(ODC)*/
     16, 128, 128, 
    };
/* RGB->YUV601 常量系数矩阵 */
const CscCoef_S g_stCSC_RGB2YUV601_pc        
    = {    
     /*csc系数*/
     299, 587, 114, -172, -339, 511, 511, -428, -83,
     /*csc输入直流(IDC)*/
     0, 0, 0,
     /*csc输出直流(ODC)*/
     0, 128, 128,
    };
/* RGB->YUV709 常量系数矩阵 */
const CscCoef_S g_stCSC_RGB2YUV709_tv
    = {
     /*csc系数*/
     183, 614, 62, -101, -338, 439, 439, -399, -40,
     /*csc输入直流(IDC)*/
     0, 0, 0,
     /*csc输出直流(ODC)*/
     16, 128, 128,
    };

/* RGB->YUV709 常量系数矩阵 */
const CscCoef_S g_stCSC_RGB2YUV709_pc
    = {
     /*csc系数*/
     213, 715, 72, -117, -394, 511, 511, -464, -47,
     /*csc输入直流(IDC)*/
     0, 0, 0,
     /*csc输出直流(ODC)*/
     0, 128, 128,
    };
#if 0
/* YUV601->RGB 常量系数矩阵 */
const CscCoef_S g_stCSC_YUV6012RGB_pc
    = {
     /*csc系数*/
     1000, 0, 1371, 1000, -698, -336, 1000, 1732, 0,
     /*csc输入直流(IDC)*/
     -16, -128, -128,
     /*csc输出直流(ODC)*/
     0, 0, 0,
    };
/* YUV709->RGB 常量系数矩阵 */
const CscCoef_S g_stCSC_YUV7092RGB_pc
    = {
     /*csc系数*/
     1000, 0, 1540, 1000, -183, -459, 1000, 1816, 0,
     /*csc输入直流(IDC)*/
     -16, -128, -128,
     /*csc输出直流(ODC)*/
     0, 0, 0,
    };
#else
/* YUV601->RGB 常量系数矩阵 */
const CscCoef_S g_stCSC_YUV6012RGB_pc
    = {
     /*csc系数*/
     1164, 0, 1596, 1164, -391, -813, 1164, 2018, 0,
     /*csc输入直流(IDC)*/
     -16, -128, -128,
     /*csc输出直流(ODC)*/
     0, 0, 0,
    };
/* YUV709->RGB 常量系数矩阵 */
const CscCoef_S g_stCSC_YUV7092RGB_pc
    = {
     /*csc系数*/
     1164, 0, 1793, 1164, -213, -534, 1164, 2115, 0,
     /*csc输入直流(IDC)*/
     -16, -128, -128,
     /*csc输出直流(ODC)*/
     0, 0, 0,
    };
#endif
/* YUV601->YUV709 常量系数矩阵 */
const CscCoef_S g_stCSC_YUV2YUV_601_709
    = {
     /*csc系数*/
     1000, -116, 208, 0, 1017, 114, 0, 75, 1025,
     /*csc输入直流(IDC)*/
     -16, -128, -128,
     /*csc输出直流(ODC)*/
     16, 128, 128,
    };
/* YUV601->YUV709 常量系数矩阵 */
const CscCoef_S g_stCSC_Init
    = {
     /*csc系数*/
     1000, 0, 0, 0, 1000, 0, 0, 0, 1000,
     /*csc输入直流(IDC)*/
     -16, -128, -128,
     /*csc输出直流(ODC)*/
     16, 128, 128,
    };


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

HI_S32 vou_cal_csc_matrix(HI_U32 u32Luma, HI_U32 u32Contrast,
    HI_U32 u32Hue, HI_U32 u32Satuature, HAL_CSC_MODE_E enCscMode, CscCoef_S *pstCstCoef)
{
    HI_S32 s32Luma     = 0;
    HI_S32 s32Contrast = 0;
    HI_S32 s32Hue      = 0;
    HI_S32 s32Satu     = 0;
    const CscCoef_S *pstCscTmp = NULL;
#if 0
    s32Luma     = (HI_S32)u32Luma - 50;
#else
    s32Luma     = (HI_S32)u32Luma * 64 / 100 - 32;
#endif
    s32Contrast = ((HI_S32)u32Contrast - 50) * 2 + 100;
    s32Hue      = (HI_S32)u32Hue * 60 / 100;
    s32Satu     = ((HI_S32)u32Satuature - 50) * 2 + 100;    

    /* 选择色彩空间转换的常系数矩阵 */
    switch (enCscMode)
    {
        case HAL_CSC_MODE_BT601_TO_BT601:
        case HAL_CSC_MODE_BT709_TO_BT709:
        case HAL_CSC_MODE_RGB_TO_RGB:
            pstCscTmp = &g_stCSC_Init;
            break;
        case HAL_CSC_MODE_BT709_TO_BT601:
        case HAL_CSC_MODE_BT601_TO_BT709:
            pstCscTmp = &g_stCSC_YUV2YUV_601_709;
            break;
        case HAL_CSC_MODE_BT601_TO_RGB_PC:
            pstCscTmp = &g_stCSC_YUV6012RGB_pc;
            break;
        case HAL_CSC_MODE_BT709_TO_RGB_PC:
            pstCscTmp = &g_stCSC_YUV7092RGB_pc;
            break;
        case HAL_CSC_MODE_RGB_TO_BT601_PC:
            pstCscTmp = &g_stCSC_RGB2YUV601_pc;
            break;
        case HAL_CSC_MODE_RGB_TO_BT709_PC:
            pstCscTmp = &g_stCSC_RGB2YUV709_pc;
            break;
        default:            
            return HI_FAILURE;
    }

    pstCstCoef->csc_in_dc0 = pstCscTmp->csc_in_dc0;
    pstCstCoef->csc_in_dc1 = pstCscTmp->csc_in_dc1;
    pstCstCoef->csc_in_dc2 = pstCscTmp->csc_in_dc2;
    pstCstCoef->csc_out_dc0 = pstCscTmp->csc_out_dc0;
    pstCstCoef->csc_out_dc1 = pstCscTmp->csc_out_dc1;
    pstCstCoef->csc_out_dc2 = pstCscTmp->csc_out_dc2;

    /* C_ratio的调节范围一般是0～1.99, C_ratio=s32Contrast/100
    *  S的调节范围一般为0~1.99,S=s32Satu/100
    *  色调调节参数的范围一般为-30°~30°,通过查表法求得COS和SIN值并/1000
    */
    if ((HAL_CSC_MODE_BT601_TO_RGB_PC == enCscMode) || (HAL_CSC_MODE_BT709_TO_RGB_PC == enCscMode)
        || (HAL_CSC_MODE_BT601_TO_RGB_TV == enCscMode) || (HAL_CSC_MODE_BT709_TO_RGB_TV == enCscMode))
    {
        /* 此公式仅用于YUV->RGB转换，RGB->YUV转换不可用此公式 */
        pstCstCoef->csc_coef00 = (s32Contrast * pstCscTmp->csc_coef00) / 100;
        pstCstCoef->csc_coef01 = (s32Contrast * s32Satu * ((pstCscTmp->csc_coef01*COS_TABLE[s32Hue] - pstCscTmp->csc_coef02*SIN_TABLE[s32Hue]) /1000)) / 10000;
        pstCstCoef->csc_coef02 = (s32Contrast * s32Satu * ((pstCscTmp->csc_coef01*SIN_TABLE[s32Hue] + pstCscTmp->csc_coef02*COS_TABLE[s32Hue]) /1000)) / 10000;
        pstCstCoef->csc_coef10 = (s32Contrast * pstCscTmp->csc_coef10) / 100;
        pstCstCoef->csc_coef11 = (s32Contrast * s32Satu * ((pstCscTmp->csc_coef11*COS_TABLE[s32Hue] - pstCscTmp->csc_coef12*SIN_TABLE[s32Hue]) /1000)) / 10000;
        pstCstCoef->csc_coef12 = (s32Contrast * s32Satu * ((pstCscTmp->csc_coef11*SIN_TABLE[s32Hue] + pstCscTmp->csc_coef12*COS_TABLE[s32Hue]) /1000)) / 10000;
        pstCstCoef->csc_coef20 = (s32Contrast * pstCscTmp->csc_coef20) / 100;
        pstCstCoef->csc_coef21 = (s32Contrast * s32Satu * ((pstCscTmp->csc_coef21*COS_TABLE[s32Hue] - pstCscTmp->csc_coef22*SIN_TABLE[s32Hue]) /1000)) / 10000;
        pstCstCoef->csc_coef22 = (s32Contrast * s32Satu * ((pstCscTmp->csc_coef21*SIN_TABLE[s32Hue] + pstCscTmp->csc_coef22*COS_TABLE[s32Hue]) /1000)) / 10000;
        pstCstCoef->csc_in_dc0 += (0 != s32Contrast) ? (s32Luma * 100 / s32Contrast) : s32Luma * 100;
    }
    else
    {    
        /* 此公式仅用于RGB->YUV转换，YUV->RGB转换不可用此公式，
        *  YUV->YUV仅调节图像效果可用此公式，因为常量矩阵为单位矩阵 */
        pstCstCoef->csc_coef00 = (s32Contrast * pstCscTmp->csc_coef00) / 100;
        pstCstCoef->csc_coef01 = (s32Contrast * pstCscTmp->csc_coef01) / 100;
        pstCstCoef->csc_coef02 = (s32Contrast * pstCscTmp->csc_coef02) / 100;
        pstCstCoef->csc_coef10 = (s32Contrast * s32Satu * ((pstCscTmp->csc_coef10*COS_TABLE[s32Hue] + pstCscTmp->csc_coef20*SIN_TABLE[s32Hue]) / 1000)) / 10000;
        pstCstCoef->csc_coef11 = (s32Contrast * s32Satu * ((pstCscTmp->csc_coef11*COS_TABLE[s32Hue] + pstCscTmp->csc_coef21*SIN_TABLE[s32Hue]) / 1000)) / 10000;
        pstCstCoef->csc_coef12 = (s32Contrast * s32Satu * ((pstCscTmp->csc_coef12*COS_TABLE[s32Hue] + pstCscTmp->csc_coef22*SIN_TABLE[s32Hue]) / 1000)) / 10000;
        pstCstCoef->csc_coef20 = (s32Contrast * s32Satu * ((pstCscTmp->csc_coef20*COS_TABLE[s32Hue] - pstCscTmp->csc_coef10*SIN_TABLE[s32Hue]) / 1000)) / 10000;
        pstCstCoef->csc_coef21 = (s32Contrast * s32Satu * ((pstCscTmp->csc_coef21*COS_TABLE[s32Hue] - pstCscTmp->csc_coef11*SIN_TABLE[s32Hue]) / 1000)) / 10000;
        pstCstCoef->csc_coef22 = (s32Contrast * s32Satu * ((pstCscTmp->csc_coef22*COS_TABLE[s32Hue] - pstCscTmp->csc_coef12*SIN_TABLE[s32Hue]) / 1000)) / 10000;
        pstCstCoef->csc_out_dc0 += s32Luma;
    }
    
    return HI_SUCCESS;
}

HI_U32 inline conver_csc_coef(HI_S32 s32Value)
{
    HI_S32 s32Result = ((s32Value << 8) / 1000);

    if (s32Result < 0)
    {
        s32Result = (~((-1)*s32Result) + 1);

        /* 5.8 format */
        s32Result = (s32Result & 0x1fff) | 0x1000;
    }
    else
    {
        s32Result = s32Result & 0x1fff;
    }
    
    return s32Result;
}


HI_U32 inline get_xdc_buma(HI_S32 s32Value)
{
    HI_U32 u32AbsValue = 0;

    if(s32Value >= 0)
    {
        return s32Value;
    }
    /*0~8bit有效，第8bit为符号位*/
    else
    {
        u32AbsValue = (-1)*s32Value;
        //return ( (((~u32AbsValue)+1)& 0xFF) | 0x100 );
        return ( ((~u32AbsValue)+1)& 0x1FF);
    }
}

HI_VOID usage(HI_VOID)
{    
    printf("\nNOTICE: This tool only can be used for TESTING !!!\n");
    printf("\nNOTICE: Luma/Contrast/Hue/Satuature [0,100], Gain [0, 0x3f]\n");
    printf("usage: ./vo_vga_config [Luma] [Contrast] [Hue] [Satuature] [Gain]. sample: ./vo_vga_config 50 50 50 50 48\n\n");
}

HI_S32 vo_vga_config(HI_U32 u32Luma, HI_U32 u32Contrast, HI_U32 u32Hue, HI_U32 u32Satuature, HI_U32 u32Gain)
{
    HI_S32      s32Ret;
    CscCoef_S   stCscCoef;
    HI_U8       *pAddr = NULL;
    HI_U32      *pRegAddr = NULL;
    HI_U32      u32VgaCscIdc, u32VgaCscOdc, u32VgaCscP0, u32VgaCscP1, u32VgaCscP2, u32VgaCscP3, u32VgaCscP4;
    HI_U32      u32DacCtrl3_5;    
    
    usage();
    
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
    if (u32Gain < 0x0 || u32Gain > 0x3F)
    {
        usage();
        return -1;
    }

    vou_cal_csc_matrix(u32Luma, u32Contrast, u32Hue, u32Satuature,
        HAL_CSC_MODE_BT601_TO_RGB_PC, &stCscCoef);

    u32VgaCscIdc = ((get_xdc_buma(stCscCoef.csc_in_dc2) & 0x1ff)
            | ((get_xdc_buma(stCscCoef.csc_in_dc1) & 0x1ff) << 9)
            | ((get_xdc_buma(stCscCoef.csc_in_dc0) & 0x1ff) << 18));
    u32VgaCscOdc = ((get_xdc_buma(stCscCoef.csc_out_dc2) & 0x1ff)
            | ((get_xdc_buma(stCscCoef.csc_out_dc1) & 0x1ff) << 9)
            | ((get_xdc_buma(stCscCoef.csc_out_dc0) & 0x1ff) << 18));
    u32VgaCscP0 = ((conver_csc_coef(stCscCoef.csc_coef00) & 0x1fff)
            | ((conver_csc_coef(stCscCoef.csc_coef01) & 0x1fff) << 16));
    u32VgaCscP1 = ((conver_csc_coef(stCscCoef.csc_coef02) & 0x1fff)
            | ((conver_csc_coef(stCscCoef.csc_coef10) & 0x1fff) << 16));
    u32VgaCscP2 = ((conver_csc_coef(stCscCoef.csc_coef11) & 0x1fff)
            | ((conver_csc_coef(stCscCoef.csc_coef12) & 0x1fff) << 16));
    u32VgaCscP3 = ((conver_csc_coef(stCscCoef.csc_coef20) & 0x1fff)
            | ((conver_csc_coef(stCscCoef.csc_coef21) & 0x1fff) << 16));
    u32VgaCscP4 = (conver_csc_coef(stCscCoef.csc_coef22) & 0x1fff);
    #if 0
    pAddr = (HI_U8 *)HI_MPI_SYS_Mmap(VO_VGA_CSC_REG, VO_VGA_CSC_REG_SIZE);
    #else
    memopen();
    pAddr = (HI_U8 *)memmap(VO_VGA_CSC_REG, VO_VGA_CSC_REG_SIZE);
    #endif
    
    if(NULL == pAddr)
    {
        printf("err: memmap failure\n");
        return HI_FAILURE;
    }
    //printf("sys_map 0x%x\n", pAddr);

    pRegAddr = (HI_U32 *)(pAddr + (VGACSCIDC - VO_VGA_CSC_REG));
    u32VgaCscIdc |= (*pRegAddr & 0x08000000);   //csc_en
    //printf("addr 0x%x, old value 0x%x, new value 0x%x\n", pRegAddr, *pRegAddr, u32VgaCscIdc);
    *pRegAddr = u32VgaCscIdc;
    pRegAddr = (HI_U32 *)(pAddr + (VGACSCODC - VO_VGA_CSC_REG));
    //printf("addr 0x%x, old value 0x%x, new value 0x%x\n", pRegAddr, *pRegAddr, u32VgaCscOdc);
    *pRegAddr = u32VgaCscOdc;
    pRegAddr = (HI_U32 *)(pAddr + (VGACSCP0 - VO_VGA_CSC_REG));
    //printf("addr 0x%x, old value 0x%x, new value 0x%x\n", pRegAddr, *pRegAddr, u32VgaCscP0);
    *pRegAddr = u32VgaCscP0;
    pRegAddr = (HI_U32 *)(pAddr + (VGACSCP1 - VO_VGA_CSC_REG));
    //printf("addr 0x%x, old value 0x%x, new value 0x%x\n", pRegAddr, *pRegAddr, u32VgaCscP1);
    *pRegAddr = u32VgaCscP1;
    pRegAddr = (HI_U32 *)(pAddr + (VGACSCP2 - VO_VGA_CSC_REG));
    //printf("addr 0x%x, old value 0x%x, new value 0x%x\n", pRegAddr, *pRegAddr, u32VgaCscP2);
    *pRegAddr = u32VgaCscP2;
    pRegAddr = (HI_U32 *)(pAddr + (VGACSCP3 - VO_VGA_CSC_REG));
    //printf("addr 0x%x, old value 0x%x, new value 0x%x\n", pRegAddr, *pRegAddr, u32VgaCscP3);
    *pRegAddr = u32VgaCscP3;
    pRegAddr = (HI_U32 *)(pAddr + (VGACSCP4 - VO_VGA_CSC_REG));
    //printf("addr 0x%x, old value 0x%x, new value 0x%x\n", pRegAddr, *pRegAddr, u32VgaCscP4);
    *pRegAddr = u32VgaCscP4;

    u32DacCtrl3_5 = ((u32Gain & 0x3f)
            | ((u32Gain & 0x3f) << 6)
            | ((u32Gain & 0x3f) << 12));

    pRegAddr = (HI_U32 *)(pAddr + (DACCTRL3_5 - VO_VGA_CSC_REG));
    u32DacCtrl3_5 |= (*pRegAddr & 0xfffc0000);   //other bits 
    *pRegAddr = u32DacCtrl3_5;
    #if 0
    HI_MPI_SYS_Munmap(VO_VGA_CSC_REG, VO_VGA_CSC_REG_SIZE);
    #else
    memunmap(VO_VGA_CSC_REG, VO_VGA_CSC_REG_SIZE);
    memclose();
    #endif

    return HI_SUCCESS;
}

HI_S32 main(int argc, char *argv[])
{
    HI_U32 u32Luma;
    HI_U32 u32Contrast;
    HI_U32 u32Hue;
    HI_U32 u32Satuature;
    HI_U32 u32Gain;    

	if (argc > 1)
    {
        u32Luma = atoi(argv[1]);
    }

	if (argc > 2)
    {
        u32Contrast = atoi(argv[2]);
    }

	if (argc > 3)
    {
        u32Hue = atoi(argv[3]);
    }

    if (argc > 4)
    {
        u32Satuature = atoi(argv[4]);
    }

    if (argc > 5)
    {
        u32Gain = atoi(argv[5]);
    }

    vo_vga_config(u32Luma, u32Contrast, u32Hue, u32Satuature, u32Gain);

	return HI_SUCCESS;
}


