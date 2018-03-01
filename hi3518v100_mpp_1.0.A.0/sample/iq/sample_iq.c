/******************************************************************************
  A simple program of Hisilicon HI3518 image quality adjusting implementation.
  Copyright (C), 2010-2011, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2013-5 Created
******************************************************************************/

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

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

#define SAMPLE_IQ_VPSS_GRP (0)
#define EXPOSURE_GAIN_SHIFT 10

/*****************************************************************************
* function : calculate the real value from agc table
*****************************************************************************/
HI_U32 SAMPLE_IQ_AgcTableCalculate(const HI_U8 *au8Array, HI_U8 u8Index, HI_U32 u32ISO)
{
    HI_U32 u32Data1, u32Data2;
    HI_U32 u32Range, u32ISO1;

    u32Data1 = au8Array[u8Index];
    u32Data2 = (7 == u8Index)? au8Array[u8Index]: au8Array[u8Index + 1];

    /************************************
    range = iso2 - iso1 = iso1
    ************************************/
    u32ISO1 = 100 << u8Index;
    u32Range = u32ISO1;

    if(u32Data1 > u32Data2)
    {
        return u32Data1 - ((u32Data1 - u32Data2) * (u32ISO - u32ISO1) + (u32Range >> 1)) / u32Range;
    }
    else
    {
        return u32Data1 + ((u32Data2 - u32Data1) * (u32ISO - u32ISO1) + (u32Range >> 1)) / u32Range;
    }
}


/*****************************************************************************
* function : show info
*****************************************************************************/
HI_S32 SAMPLE_IQ_Info(void)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_INNER_STATE_INFO_EX_S stInnerStateInfoEx;
    ISP_SHARPEN_ATTR_S stSharpenAttr;
    ISP_DENOISE_ATTR_S stDenoiseAttr;
    ISP_SATURATION_ATTR_S stSatAttr;
    HI_U32 u32ISO = 100;
    HI_U32 u32ISOTmp;
    HI_U8 u8Sharpen_d, u8Sharpen_ud, u8Denoise;
    HI_U32 u32Sat;
    HI_U8 u8Index;
    VPSS_GRP VpssGrp = SAMPLE_IQ_VPSS_GRP;
    VPSS_GRP_PARAM_S stVpssParam;

    /******************************************
     get current state info
    ******************************************/
    s32Ret = HI_MPI_ISP_QueryInnerStateInfoEx(&stInnerStateInfoEx);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_ISP_QueryInnerStateInfoEx failed!\n");
        return s32Ret;
    }

    s32Ret = HI_MPI_ISP_GetSharpenAttr(&stSharpenAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_ISP_GetSharpenAttr failed!\n");
        return s32Ret;
    }

    s32Ret = HI_MPI_ISP_GetDenoiseAttr(&stDenoiseAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_ISP_GetDenoiseAttr failed!\n");
        return s32Ret;
    }

    s32Ret = HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssParam);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VPSS_GetGrpParam failed!\n");
        return s32Ret;
    }

    s32Ret = HI_MPI_ISP_GetSaturationAttr(&stSatAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_ISP_GetSaturationAttr failed!\n");
        return s32Ret;
    }

    /******************************************
     ISO may be different for different sensor,
     refer to associated cmos.c
    ******************************************/
    u32ISO = ((HI_U64)stInnerStateInfoEx.u32AnalogGain * stInnerStateInfoEx.u32DigitalGain
             * stInnerStateInfoEx.u32ISPDigitalGain * 100) >> (EXPOSURE_GAIN_SHIFT * 3);

    /******************************************
     calculate the lower array index for sharpen,
     denoise and saturation auto control table
     ISO            index
     [100, 200)         0
     [200, 400)         1
     [400, 800)         2
     [800, 1600)        3
     [1600, 3200)       4
     [3200, 6400)       5
     [6400, 12800)      6
     [12800, infinity)  7
    ******************************************/
    u32ISOTmp = u32ISO / 100;
    for(u8Index = 0; u8Index < 7;u8Index++)
    {
        if(1 == u32ISOTmp)
        {
            break;
        }

        u32ISOTmp >>= 1;
    }

    /******************************************
    calculate real sharpen,denoise and
    saturation strength
    ******************************************/
    if(stSharpenAttr.bManualEnable)
    {
        u8Sharpen_d = stSharpenAttr.u8StrengthTarget;
        u8Sharpen_ud = stSharpenAttr.u8StrengthUdTarget;
    }
    else
    {
        u8Sharpen_d = SAMPLE_IQ_AgcTableCalculate(stSharpenAttr.u8SharpenAltD, u8Index, u32ISO);
        u8Sharpen_ud = SAMPLE_IQ_AgcTableCalculate(stSharpenAttr.u8SharpenAltUd, u8Index, u32ISO);
    }

    if(stDenoiseAttr.bManualEnable)
    {
        u8Denoise = stDenoiseAttr.u8ThreshTarget;
    }
    else
    {
        u8Denoise = SAMPLE_IQ_AgcTableCalculate(stDenoiseAttr.u8SnrThresh, u8Index, u32ISO);
    }

    if(stSatAttr.bSatManualEnable)
    {
        u32Sat = stSatAttr.u8SatTarget;
    }
    else
    {
        u32Sat = (SAMPLE_IQ_AgcTableCalculate(stSatAttr.au8Sat, u8Index, u32ISO) 
                 * stSatAttr.u8SatTarget + (0x80 >> 1)) / 0x80;
        u32Sat = (u32Sat > 0xFF)? 0xFF: u32Sat; 
    }
    
    printf("-------------------------------------------------------------------------------------------------\n");
    printf("   ISO   again   dgain   isp_dgain   Sharpen_d   Sharpen_ud   Denoise   SF   TF   CS   Saturation\n");
    printf("%6d      %2d      %2d          %2d        0x%2x         0x%2x      0x%2x  %3d  %3d  %3d          %3d\n"
           , u32ISO, stInnerStateInfoEx.u32AnalogGain >> EXPOSURE_GAIN_SHIFT
           , stInnerStateInfoEx.u32DigitalGain >> EXPOSURE_GAIN_SHIFT
           , stInnerStateInfoEx.u32ISPDigitalGain >> EXPOSURE_GAIN_SHIFT
           , u8Sharpen_d, u8Sharpen_ud, u8Denoise, stVpssParam.u32SfStrength, stVpssParam.u32TfStrength
           , stVpssParam.u32ChromaRange, u32Sat);

    return HI_SUCCESS;
}

/*****************************************************************************
* function : set gamma table
*****************************************************************************/
HI_S32 SAMPLE_IQ_SetGammaTable(HI_U8 u8GammaTabNum)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_GAMMA_TABLE_S stGammaTable;
    /******************************************
    table 0 is the default gamma,with lowest 
    contrast and best detail in dark area.
    ******************************************/
    static const HI_U16 au16GammaTable0[GAMMA_NODE_NUMBER] = 
        { 0 ,120 ,220 ,310 ,390 ,470 ,540 ,610 ,670 ,730 ,786 ,842 ,894 ,944 ,994 ,1050,    
        1096,1138,1178,1218,1254,1280,1314,1346,1378,1408,1438,1467,1493,1519,1543,1568,    
        1592,1615,1638,1661,1683,1705,1726,1748,1769,1789,1810,1830,1849,1869,1888,1907,    
        1926,1945,1963,1981,1999,2017,2034,2052,2069,2086,2102,2119,2136,2152,2168,2184,    
        2200,2216,2231,2247,2262,2277,2292,2307,2322,2337,2351,2366,2380,2394,2408,2422,    
        2436,2450,2464,2477,2491,2504,2518,2531,2544,2557,2570,2583,2596,2609,2621,2634,    
        2646,2659,2671,2683,2696,2708,2720,2732,2744,2756,2767,2779,2791,2802,2814,2825,    
        2837,2848,2859,2871,2882,2893,2904,2915,2926,2937,2948,2959,2969,2980,2991,3001,    
        3012,3023,3033,3043,3054,3064,3074,3085,3095,3105,3115,3125,3135,3145,3155,3165,    
        3175,3185,3194,3204,3214,3224,3233,3243,3252,3262,3271,3281,3290,3300,3309,3318,    
        3327,3337,3346,3355,3364,3373,3382,3391,3400,3409,3418,3427,3436,3445,3454,3463,    
        3471,3480,3489,3498,3506,3515,3523,3532,3540,3549,3557,3566,3574,3583,3591,3600,    
        3608,3616,3624,3633,3641,3649,3657,3665,3674,3682,3690,3698,3706,3714,3722,3730,    
        3738,3746,3754,3762,3769,3777,3785,3793,3801,3808,3816,3824,3832,3839,3847,3855,    
        3862,3870,3877,3885,3892,3900,3907,3915,3922,3930,3937,3945,3952,3959,3967,3974,    
        3981,3989,3996,4003,4010,4018,4025,4032,4039,4046,4054,4061,4068,4075,4082,4089,4095};

    /******************************************
    table 1 has higher contrast than table 0
    ******************************************/
    static const HI_U16 au16GammaTable1[GAMMA_NODE_NUMBER] = 
        {0,54,106,158,209,259, 308, 356, 403, 450, 495, 540, 584, 628, 670, 713, 754, 795,
        835, 874, 913, 951, 989,1026,1062,1098,1133,1168,1203,1236,1270,1303, 1335,1367,
        1398,1429,1460,1490,1520,1549,1578,1607,1635,1663,1690,1717,1744,1770,1796,1822,
        1848,1873,1897,1922,1946,1970,1993,2017,2040,2062,2085,2107,2129,2150, 2172,2193,
        2214,2235,2255,2275,2295,2315,2335,2354,2373,2392,2411,2429,2447,2465, 2483,2501,
        2519,2536,2553,2570,2587,2603,2620,2636,2652,2668,2684,2700,2715,2731, 2746,2761,
        2776,2790,2805,2819,2834,2848,2862,2876,2890,2903,2917,2930,2944,2957, 2970,2983,
        2996,3008,3021,3033,3046,3058,3070,3082,3094,3106,3118,3129,3141,3152, 3164,3175,
        3186,3197,3208,3219,3230,3240,3251,3262,3272,3282,3293,3303,3313,3323, 3333,3343,
        3352,3362,3372,3381,3391,3400,3410,3419,3428,3437,3446,3455,3464,3473, 3482,3490,
        3499,3508,3516,3525,3533,3541,3550,3558,3566,3574,3582,3590,3598,3606, 3614,3621,
        3629,3637,3644,3652,3660,3667,3674,3682,3689,3696,3703,3711,3718,3725, 3732,3739,
        3746,3752,3759,3766,3773,3779,3786,3793,3799,3806,3812,3819,3825,3831, 3838,3844,
        3850,3856,3863,3869,3875,3881,3887,3893,3899,3905,3910,3916,3922,3928, 3933,3939,
        3945,3950,3956,3962,3967,3973,3978,3983,3989,3994,3999,4005,4010,4015, 4020,4026,
        4031,4036,4041,4046,4051,4056,4061,4066,4071,4076,4081,4085,4090,4095, 4095};

    /******************************************
    table 2 has very high contrast
    ******************************************/
    static const HI_U16 au16GammaTable2[GAMMA_NODE_NUMBER] = 
        {  0,  27,  60, 100, 140, 178,  216, 242, 276, 312, 346,  380, 412, 444, 476, 508,
         540, 572, 604, 636, 667, 698, 729,  760, 791, 822, 853, 884, 915, 945, 975, 1005,
        1035,1065,1095,1125,1155,1185,1215,1245,1275,1305,1335,1365,1395,1425,1455,1485,
        1515,1544,1573,1602,1631,1660,1689,1718,1746,1774,1802,1830,1858,1886,1914,1942,
        1970,1998,2026,2054,2082,2110,2136,2162,2186,2220,2244,2268,2292,2316,2340,2362,
        2384,2406,2428,2448,2468,2488,2508,2528,2548,2568,2588,2608,2628,2648,2668,2688,
        2708,2728,2748,2768,2788,2808,2828,2846,2862,2876,2890,2903,2917,2930,2944,2957,
        2970,2983,2996,3008,3021,3033,3046,3058,3070,3082,3094,3106,3118,3129,3141,3152,
        3164,3175,3186,3197,3208,3219,3230,3240,3251,3262,3272,3282,3293,3303,3313,3323,
        3333,3343,3352,3362,3372,3381,3391,3400,3410,3419,3428,3437,3446,3455,3464,3473,
        3482,3490,3499,3508,3516,3525,3533,3541,3550,3558,3566,3574,3582,3590,3598,3606,
        3614,3621,3629,3637,3644,3652,3660,3667,3674,3682,3689,3696,3703,3711,3718,3725,
        3732,3739,3746,3752,3759,3766,3773,3779,3786,3793,3799,3806,3812,3819,3825,3831,
        3838,3844,3850,3856,3863,3869,3875,3881,3887,3893,3899,3905,3910,3916,3922,3928,
        3933,3939,3945,3950,3956,3962,3967,3973,3978,3983,3989,3994,3999,4005,4010,4015,
        4020,4026,4031,4036,4041,4046,4051,4056,4061,4066,4071,4076,4081,4085,4090,4095,4095};


    stGammaTable.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
    switch(u8GammaTabNum)
    {
        case 0:
            memcpy(stGammaTable.u16Gamma, au16GammaTable0, sizeof(au16GammaTable0));
            break;
        case 1:
            memcpy(stGammaTable.u16Gamma, au16GammaTable1, sizeof(au16GammaTable1));
            break;
        case 2:
            memcpy(stGammaTable.u16Gamma, au16GammaTable2, sizeof(au16GammaTable2));
            break;
        default:
            return HI_SUCCESS;
    }


    HI_MPI_ISP_SetGammaTable(&stGammaTable);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_ISP_SetGammaTable failed!\n");
        return s32Ret;
    }

    printf("use gamma%d\n", u8GammaTabNum);
    return HI_SUCCESS;
}


/*****************************************************************************
* function : gamma
*****************************************************************************/
HI_S32 SAMPLE_IQ_Gamma(HI_BOOL bAuto)
{
    HI_S32 s32GetC = '\n';
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8 u8GammaTabNum = 255;

    if(bAuto)
    {
        s32GetC = 'a';
    }
    
    do
    {
        switch(s32GetC)
        {
            case 'd':
                u8GammaTabNum = 0;
                break;
            case 'h':
                u8GammaTabNum = 1;
                break;
            case 'v':
                u8GammaTabNum = 2;
                break;
            case 'a':
                /********************************************************************
                gamma should be different out door and in door, and different for
                different gain, different for different sensor
                Attention: the parameters below are not fine tuned
                ********************************************************************/
                {
                    ISP_INNER_STATE_INFO_EX_S stInnerStateInfoEx;
                    HI_U32 u32ISO;
                    
                    s32Ret = HI_MPI_ISP_QueryInnerStateInfoEx(&stInnerStateInfoEx);
                    if (HI_SUCCESS != s32Ret)
                    {
                        SAMPLE_PRT("HI_MPI_ISP_QueryInnerStateInfoEx failed!\n");
                        return s32Ret;
                    }
                    u32ISO = ((HI_U64)stInnerStateInfoEx.u32AnalogGain * stInnerStateInfoEx.u32DigitalGain
                              * stInnerStateInfoEx.u32ISPDigitalGain * 100) >> (EXPOSURE_GAIN_SHIFT * 3);

                    if(stInnerStateInfoEx.u32ExposureTime < 100)
                    {
                        u8GammaTabNum = 2;
                    }
                    else if(u32ISO < 1000)
                    {
                        u8GammaTabNum = 0;
                    }
                    else if(u32ISO < 3200)
                    {
                        u8GammaTabNum = 1;
                    }
                    else
                    {
                        u8GammaTabNum = 2;
                    }
                }
                break;
            default:
                u8GammaTabNum = 255;
                break;
        }


        SAMPLE_IQ_SetGammaTable(u8GammaTabNum);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_IQ_SetGammaTable failed!\n");
            return s32Ret;
        }   

        if(bAuto)
        {
            return HI_SUCCESS;
        }
        
        printf("\n--back(b)/default(d)/high contrast(h)/very hight contrast(v)/auto correct(a)--\n");
        s32GetC = getchar();
        if(s32GetC != '\n')
        {
            HI_S32 ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
        }
        printf("\n");
    }while('b' != s32GetC);

    return HI_SUCCESS;
}


/*****************************************************************************
* function : vpss
*****************************************************************************/
HI_S32 SAMPLE_IQ_Vpss(HI_BOOL bAuto)
{
    static const HI_U8 au8VpssSF[8] = {  20, 20, 20, 20, 32, 48, 64,128};
    static const HI_U8 au8VpssTF[8] = {  8, 8, 8, 8, 12, 16, 16, 32};
    static const HI_U8 au8VpssSF_M[8] = {  5, 6, 8, 16, 32, 48, 64, 128};
    static const HI_U8 au8VpssTF_M[8] = {  5, 6, 8, 8, 8, 12, 16, 32};
    HI_S32 s32GetC = '\n';
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_INNER_STATE_INFO_EX_S stInnerStateInfoEx;
    HI_U32 u32ISO, u32ISOTmp;
    HI_U8 u8Index;
    VPSS_GRP VpssGrp = SAMPLE_IQ_VPSS_GRP;
    VPSS_GRP_PARAM_S stVpssParam;

    if(bAuto)
    {
        s32GetC = 'a';
    }

    do
    {
        s32Ret = HI_MPI_ISP_QueryInnerStateInfoEx(&stInnerStateInfoEx);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_ISP_QueryInnerStateInfoEx failed!\n");
            return s32Ret;
        }
        u32ISO = ((HI_U64)stInnerStateInfoEx.u32AnalogGain * stInnerStateInfoEx.u32DigitalGain
                  * stInnerStateInfoEx.u32ISPDigitalGain * 100) >> (EXPOSURE_GAIN_SHIFT * 3);
        u32ISOTmp = u32ISO / 100;
        for(u8Index = 0; u8Index < 7;u8Index++)
        {
            if(1 == u32ISOTmp)
            {
                break;
            }

            u32ISOTmp >>= 1;
        }

        HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssParam);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VPSS_GetGrpParam failed!\n");
            return s32Ret;
        }

        switch(s32GetC)
        {
            case 'd':
                stVpssParam.u32TfStrength = 8;
                stVpssParam.u32SfStrength = 32;
                stVpssParam.u32ChromaRange = 8;
                break;
            case 'a':
                /********************************************************************************
                 Vpss parameter should be tuned periodically to fit for different gain.
                 It's recommended to set sf = 4 * tf, cs(ChromaRange) = tf or cs = 2 * tf.
                 Attention: the parameters below are not fine tuned, and should be different for
                 differents sensor.
                ********************************************************************************/
                stVpssParam.u32TfStrength = SAMPLE_IQ_AgcTableCalculate(au8VpssTF, u8Index, u32ISO);
                stVpssParam.u32SfStrength = 4 * stVpssParam.u32TfStrength;
                stVpssParam.u32ChromaRange = stVpssParam.u32TfStrength;
                break;
            case 'm':
                /********************************************************************************
                 Vpss parameter should be tuned periodically to fit for different gain.
                 For moving objects, or the camera is not stable, sf ,tf and cs should be set same 
                 to keep details, but there would be more noise.
                 Attention: the parameters below are not fine tuned, and should be different for
                 differents sensor.
                ********************************************************************************/
                stVpssParam.u32TfStrength = SAMPLE_IQ_AgcTableCalculate(au8VpssTF_M, u8Index, u32ISO);
                stVpssParam.u32SfStrength = SAMPLE_IQ_AgcTableCalculate(au8VpssSF_M, u8Index, u32ISO);
                stVpssParam.u32ChromaRange = stVpssParam.u32TfStrength;
                break;
            default:

                break;
        }
        
        HI_MPI_VPSS_SetGrpParam(VpssGrp, &stVpssParam);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VPSS_SetGrpParam failed!\n");
            return s32Ret;
        }    
        
        printf("------------------------------------------------------------------------------------\n");
        printf("   ISO   SF   TF   CS\n");
        printf("%6d  %3d  %3d  %3d\n", u32ISO, stVpssParam.u32SfStrength, stVpssParam.u32TfStrength
               , stVpssParam.u32ChromaRange);
        
        if(bAuto)
        {
            return HI_SUCCESS;
        }
        
        printf("\n---back(b)/default(d)/auto correct(a)/auto correct for motion(m)----\n");
        s32GetC = getchar();
        if(s32GetC != '\n')
        {
            HI_S32 ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
        }
        printf("\n");
    }while('b' != s32GetC);

    return HI_SUCCESS;    
}


/******************************************************************************
* function    : main()
* Description : image quality adjustment
* Note        : To adjust image quality, isp need to be run first. Use 
                sample_vio or other program with isp thread.
******************************************************************************/
int main(int argc, char *argv[])
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 s32GetC = 'i';

    do
    {
        switch(s32GetC)
        {
            case 'i':
                s32Ret = SAMPLE_IQ_Info();
                if(HI_SUCCESS != s32Ret)
                {
                    SAMPLE_PRT("SAMPLE_IQ_Info failed!\n");
                    goto END;
                }
                break;
            case 'g':
                s32Ret = SAMPLE_IQ_Gamma(HI_FALSE);
                if(HI_SUCCESS != s32Ret)
                {
                    SAMPLE_PRT("SAMPLE_IQ_Gamma failed!\n");
                    goto END;
                }
                break;
            case 'v':
                s32Ret = SAMPLE_IQ_Vpss(HI_FALSE);
                if(HI_SUCCESS != s32Ret)
                {
                    SAMPLE_PRT("SAMPLE_IQ_Vpss failed!\n");
                    goto END;
                }
                break;
            case 'a':
                s32Ret = SAMPLE_IQ_Vpss(HI_TRUE);
                if(HI_SUCCESS != s32Ret)
                {
                    SAMPLE_PRT("SAMPLE_IQ_Vpss failed!\n");
                    goto END;
                }
                printf("\n");
                s32Ret = SAMPLE_IQ_Gamma(HI_TRUE);
                if(HI_SUCCESS != s32Ret)
                {
                    SAMPLE_PRT("SAMPLE_IQ_Gamma failed!\n");
                    goto END;
                }
                break;
            default:
                break;
        }
        
        printf("\n------------quit(q)/info(i)/gamma(g)/vpss(v)/auto correct(a)------------\n");
        s32GetC = getchar();
        if(s32GetC != '\n')
        {
            HI_S32 ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
        }

        printf("\n");
    }while('q' != s32GetC);
  
END:
    if (HI_SUCCESS == s32Ret)
        SAMPLE_PRT("program exit normally!\n");
    else
        SAMPLE_PRT("program exit abnormally!\n");
    exit(s32Ret);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

