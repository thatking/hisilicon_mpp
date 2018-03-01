#ifndef __HI_SCENEAUTO_COMM_H__
#define __HI_SCENEAUTO_COMM_H__

#include "hi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define CHECK_NULL_PTR(ptr)\
    do{\
        if(NULL == ptr)\
        {\
            printf("func:%s,line:%d, NULL pointer\n",__FUNCTION__,__LINE__);\
            return HI_FAILURE;\
        }\
    }while(0);

#define EXPOSURE_LEVEL	16
#define AE_WEIGHT_ROW   15
#define AE_WEIGHT_COLUMN 17

typedef enum hiSCENEAUTO_OP_TYPE_E
{
    TYPE_AUTO    = 0,
    TYPE_MANUAL  = 1,
    TYPE_BUTT
} SCENEAUTO_OP_TYPE_E;




typedef struct hiSCENEAUTO_INIPARAM_NRS_S
{
    HI_BOOL BoolLNTH;
	HI_S32 s32IES0;
	HI_S32 s32SBS0;
	HI_S32 s32SBS1;
	HI_S32 s32SBS2;
	HI_S32 s32SBS3;
	HI_S32 s32SDS0;
	HI_S32 s32SDS1;
	HI_S32 s32SDS2;
	HI_S32 s32SDS3;
	HI_S32 s32STH0;
	HI_S32 s32STH1;
	HI_S32 s32STH2;
	HI_S32 s32STH3;
	HI_S32 s32MDP;
	HI_S32 s32MATH1;
	HI_S32 s32MATH2;
	HI_S32 s32Pro3;
	HI_S32 s32MDDZ1;
	HI_S32 s32MDDZ2;
	HI_S32 s32TFS1;
	HI_S32 s32TFS2;
	HI_S32 s32SFC;
	HI_S32 s32TFC;
	HI_S32 s32TPC;
	HI_S32 s32TRC;

}SCENEAUTO_INIPARAM_NRS_S,ADPT_SCENEAUTO_3DNR_ATTR_S,SCENEAUTO_INIPARAM_3DNRCFG_S;

typedef struct hiSCENEAUTO_INIPARAM_3DNR_S
{
    HI_BOOL BoolRefMGValue;
    HI_BOOL BoolLNTH;
    HI_U32 u323DnrIsoCount;
    HI_U32 *pu323DnrIsoThresh;
    SCENEAUTO_INIPARAM_NRS_S *pst3dnrParam;
} SCENEAUTO_INIPARAM_3DNR_S;


typedef struct hiSCENEAUTO_INIPARAM_HLC_S
{
    HI_U8 u8ExpCompensation;
    HI_U8 u8Saturation[16];
    HI_U16 u16BlackDelayFrame;
    HI_U16 u16WhiteDelayFrame;
    HI_U8 u8Speed;
    HI_U16 u16HistRatioSlope;
    HI_U8 u8MaxHistOffset;
    HI_U8 u8Tolerance;
    HI_BOOL bDCIEnable;
    HI_U32 u32DCIBlackGain;
    HI_U32 u32DCIContrastGain;
    HI_U32 u32DCILightGain;
    HI_BOOL bDRCEnable;
    HI_BOOL bDRCManulEnable;
    HI_U8  u8DRCStrengthTarget;
    HI_U16 u16GammaTable[257];
	SCENEAUTO_INIPARAM_3DNR_S stHLC3dnr;
}SCENEAUTO_INIPARAM_HLC_S;


typedef struct hiSCENEAUTO_INIPARAM_IR_S
{
    HI_U32 u32ExpCount;
    HI_U32* pu32ExpThreshLtoH;
    HI_U32* pu32ExpThreshHtoL;
    HI_U8* pu8ExpCompensation;
    HI_U8* pu8MaxHistOffset;

    HI_U16 u16HistRatioSlope;
    HI_U16 u16BlackDelayFrame;
    HI_U16 u16WhiteDelayFrame;
    HI_U8 u8Speed;
    HI_U8 u8Tolerance;
    HI_BOOL bDCIEnable;
    HI_U32 u32DCIBlackGain;
    HI_U32 u32DCIContrastGain;
    HI_U32 u32DCILightGain;
	
	HI_BOOL bSupTwinkleEn;
	HI_S8 SoftThr;
	HI_U8 SoftSlope; 

	HI_U16 au16SharpenUd[16];
	HI_U8 au8SharpenD[16];	
	HI_U8 au8TextureThr[16];	
	HI_U8 au8SharpenEdge[16];
	HI_U8 au8EdgeThr[16];	
	HI_U8 au8OverShoot[16];	
	HI_U8 au8UnderShoot[16];	
	HI_U8 au8shootSupStr[16];
	HI_U8 au8DetailCtrl[16]; 
    HI_U16 u16GammaTable[257];
    HI_U8 au8Weight[AE_WEIGHT_ROW][AE_WEIGHT_COLUMN];
	SCENEAUTO_INIPARAM_3DNR_S stIni3dnr;

} SCENEAUTO_INIPARAM_IR_S;


typedef struct hiSCENEAUTO_H265VENC_RCPARAM_S
{
    HI_U32 u32ThrdI[12];
    HI_U32 u32ThrdP[12];
    HI_U32 u32DeltaQP;
    HI_S32 s32IPQPDelta;
} SCENEAUTO_H265VENC_RCPARAM_S;

typedef struct hiSCENEAUTO_INIPARAM_H265VENC_RCPARAM_S
{
    HI_U32 u32BitrateCount;
    HI_U32* pu32BitrateThresh;
    SCENEAUTO_H265VENC_RCPARAM_S* pstH265VencRcParam;
} SCENEAUTO_INIPARAM_H265VENC_RCPARAM_S;

typedef struct hiSCENEAUTO_INIPARAM_H265VENC_S
{
    SCENEAUTO_INIPARAM_H265VENC_RCPARAM_S stIniH265VencRcParam;
    //SCENEAUTO_INIPARAM_H265VENC_FACECFG_S stIniH265VencFaceCfg;
} SCENEAUTO_INIPARAM_H265VENC_S;


typedef struct hiSCENEAUTO_H264DBLK_S
{
    HI_U32 disable_deblocking_filter_idc;
    HI_S32 slice_alpha_c0_offset_div2;
    HI_S32 slice_beta_offset_div2;
} SCENEAUTO_H264DBLK_S;

typedef struct hiSCENEAUTO_H264VENC_S
{
    HI_U32 u32ThrdI[16];
    HI_U32 u32ThrdP[16];
    HI_U32 u32DeltaQP;
    HI_S32 s32IPQPDelta;
    HI_S32 s32chroma_qp_index_offset;
    SCENEAUTO_H264DBLK_S stH264Dblk;
} SCENEAUTO_H264VENC_S;

typedef struct hiSCENEAUTO_INIPARAM_H264VENC_S
{
    HI_U32 u32BitrateCount;
    HI_U32* pu32BitrateThresh;
    SCENEAUTO_H264VENC_S* pstH264Venc;
} SCENEAUTO_INIPARAM_H264VENC_S;

typedef struct hiSCENEAUTO_FALSECOLOR_S
{
    HI_U8 u8Strength;
}SCENEAUTO_FALSECOLOR_S;

typedef struct hiSCENEAUTO_INIPARAM_FALSECOLOR_S
{
    HI_U32 u32ExpCount;
    HI_U32* pu32ExpThresh;
    SCENEAUTO_FALSECOLOR_S* pstFalseColor;
}SCENEAUTO_INIPARAM_FALSECOLOR_S;

typedef struct hiSCENEAUTO_DEPATTR_S
{
	HI_BOOL bSupTwinkleEn;
	HI_S8 SoftThr;
	HI_U8 SoftSlope; 
	
} SCENEAUTO_DEPATTR_S;

typedef struct hiSCENEAUTO_INIPARAM_DP_S
{
    HI_U32 u32ExpCount;
    HI_U32* pu32ExpThresh;
    SCENEAUTO_DEPATTR_S* pstDPAttr;
} SCENEAUTO_INIPARAM_DP_S;

typedef struct hiSCENEAUTO_INIPARAM_DRC_S
{
	HI_U8 stManulalStr ;
} SCENEAUTO_INIPARAM_DRC_S;
typedef struct hiSCENEAUTO_INIPARAM_Drc_S
{
    HI_U32 u32ExpCount;
	HI_U32 s32Interval;
    HI_U32* pu32ExpThreshLtoD;
	HI_U32* pu32ExpThreshDtoL;
	SCENEAUTO_OP_TYPE_E enOpType ;
    SCENEAUTO_INIPARAM_DRC_S* pstDrcAttr;
} SCENEAUTO_INIPARAM_Drc_S;
typedef struct hiSCENEAUTO_INIPARAM_Defog_S
{
	HI_BOOL bEnable;
	SCENEAUTO_OP_TYPE_E enOpType;
	HI_U8 ManualStrength;
}SCENEAUTO_INIPARAM_Ddefog_S;
typedef struct hiSCENEAUTO_INIPARAM_2DNR_S
{
	HI_U8   au8WDRCoarseStr[2];  //[0, 80] //Denoise strength of long frame and short frame in wdr mode
}SCENEAUTO_INIPARAM_2DNR_S;
typedef struct hiSCENEAUTO_SHARPEN_S
{
	HI_U16 au16SharpenUd[16];
	HI_U8 au8SharpenD[16];	
	HI_U8 au8TextureThr[16];	
	HI_U8 au8SharpenEdge[16];
	HI_U8 au8EdgeThr[16];	
	HI_U8 au8OverShoot[16];	
	HI_U8 au8UnderShoot[16];	
	HI_U8 au8shootSupStr[16]; 
	HI_U8 au8DetailCtrl[16];  

} SCENEAUTO_SHARPEN_S;

typedef struct hiSCENEAUTO_INIPARAM_SHARPEN_S
{
    HI_U32 u32BitrateCount;
    HI_U32* pu32BitrateThresh;
    HI_U32 u32ExpCount;
    HI_U32* pu32ExpThresh;
    SCENEAUTO_SHARPEN_S* pstSharpen;
} SCENEAUTO_INIPARAM_SHARPEN_S;

typedef struct hiSCENEAUTO_AERELATEDEXP_S
{
    HI_U8 u8AECompesation;
    HI_U8 u8AEHistOffset;
} SCENEAUTO_AERELATEDEXP_S;

typedef struct hiSCENEAUTO_AERELATEDBIT_S
{
    HI_U8  u8Speed;
    HI_U8  u8Tolerance;
    HI_U16 u16BlackDelayFrame;
    HI_U16 u16WhiteDelayFrame;
    HI_U32 u32SysGainMax;
} SCENEAUTO_AERELATEDBIT_S;

typedef struct hiSCENEAUTO_INIPARAM_AE_S
{
    HI_U8 u8AERunInterval;
    HI_U32 u32BitrateCount;
    HI_U32* pu32BitrateThresh;
    SCENEAUTO_AERELATEDBIT_S* pstAERelatedBit;
    HI_U32 u32ExpCount;
    HI_U32* pu32AEExpHtoLThresh;
    HI_U32* pu32AEExpLtoHThresh;
    SCENEAUTO_AERELATEDEXP_S* pstAERelatedExp;

	HI_U8  u8ExpRatioType;
	HI_U32 au32ExpRatio[3];
    HI_U32 u32ExpRatioMax; 
    HI_U32 u32ExpRatioMin;
    HI_U16 u16RatioBias;
    HI_U32 u32ExpRatio;
} SCENEAUTO_INIPARAM_AE_S;

typedef struct hiSCENEAUTO_GAMMA_S
{
    HI_U8 u8CurveType;
    HI_U16 u16Table[257];
} SCENEAUTO_GAMMA_S;

typedef struct hiSCENEAUTO_INIPARAM_GAMMA_S
{
    HI_U32 u32ExpCount;
    HI_U32 u32DelayCount;
    HI_S32 s32Interval;
    HI_U32* pu32ExpThreshLtoH;
    HI_U32* pu32ExpThreshHtoL;
    SCENEAUTO_GAMMA_S* pstGamma;
} SCENEAUTO_INIPARAM_GAMMA_S;

typedef struct hiSCENEAUTO_ROUTE_NODE_S
{
    HI_U32  u32IntTime;
    HI_U32  u32SysGain;
}SCENEAUTO_ROUTE_NODE_S;

typedef struct hiSCENEAUTO_INIPARAM_AEROUTE_S
{
    HI_S32 s32TotalNum;
    SCENEAUTO_ROUTE_NODE_S pstRouteNode[16];        
}SCENEAUTO_INIPARAM_AEROUTE_S;

typedef struct hiSCENEAUTO_INIPARAM_THRESHVALUE_S
{
    HI_BOOL bIVEEnable;
    //HI_BOOL bHLCAutoEnable;
    //HI_U32 u32HLCOnThresh;
    //HI_U32 u32HLCOffThresh;
    //HI_U32 u32HLCTolerance;
    //HI_U32 u32HLCExpThresh;
    //HI_U32 u32HLCCount;
    HI_U32 u32AveLumThresh;
    HI_U32 u32DeltaDisExpThreash;
    HI_U32 u32FpnExpThresh;
    HI_U32 u32DRCStrengthThresh;
} SCENEAUTO_INIPARAM_THRESHVALUE_S;

typedef struct hiSCENEAUTO_AWB_CBCR_TRACK_ATTR_S
{
    HI_BOOL bEnable;                
    
    HI_U16  au16CrMax[16];       
    HI_U16  au16CrMin[16];   
    HI_U16  au16CbMax[16];  
    HI_U16  au16CbMin[16];  
} SCENEAUTO_AWB_CBCR_TRACK_ATTR_S;

typedef struct hiSCENEAUTO_WB_STATISTICS_CFG_PARA_S
{
    HI_U16 u16WhiteLevel;       
    HI_U16 u16BlackLevel;       
    HI_U16 u16CbMax;            
    HI_U16 u16CbMin;            
    HI_U16 u16CrMax;            
    HI_U16 u16CrMin;            
    HI_U16 u16CbHigh;           
    HI_U16 u16CbLow;           
    HI_U16 u16CrHigh;          
    HI_U16 u16CrLow;           
} SCENEAUTO_WB_STATISTICS_CFG_PARA_S;

typedef struct hiSCENEAUTO_INIPARAM_AWB_S
{
	HI_U32 u32ISO;
	SCENEAUTO_AWB_CBCR_TRACK_ATTR_S*     stAwbCrCbTrack;
	SCENEAUTO_WB_STATISTICS_CFG_PARA_S*  stAwbStatisticsPara;
}SCENEAUTO_INIPARAM_AWB_S;

typedef struct hiSCENEAUTO_BLACK_LEVEL_S 
{ 
    HI_U16 au16R[16];  
	HI_U16 au16Gr[16];  
	HI_U16 au16Gb[16]; 
	HI_U16 au16B[16]; 
} SCENEAUTO_BLACK_LEVEL_S;

typedef struct hiSCENEAUTO_INIPARAM_BLACK_LEVEL_S
{
	SCENEAUTO_BLACK_LEVEL_S *BlackLevel;
}SCENEAUTO_INIPARAM_BLACK_LEVEL_S;



typedef struct hiSCENEAUTO_INIPARAM_SHADING_S
{
    HI_BOOL bEnable;
    SCENEAUTO_OP_TYPE_E enOpType;
} SCENEAUTO_INIPARAM_SHADING_S;

        
typedef struct hiSCENEAUTO_INIPARAM_MPINFO_S
{
    HI_S32 s32IspDev;
    HI_S32 s32ViDev;
    HI_S32 s32ViChn;
    HI_S32 s32VpssGrp;
    HI_S32 s32VpssChn;
    HI_S32 s32VencGrp;
    HI_S32 s32VencChn;
}SCENEAUTO_INIPARAM_MPINFO_S;


typedef struct hiSCENEAUTO_INIPARAM_TRAFFIC_S
{
	HI_BOOL bDCIEnable;
    HI_U32 u32DCIBlackGain;
    HI_U32 u32DCIContrastGain;
    HI_U32 u32DCILightGain;
    HI_U8   u8ExpCompensation;
    HI_U8   u8MaxHistoffset;
	
	HI_U8  u8SpatialVar;        
	HI_U8  u8RangeVar;          
	HI_U8  u8Asymmetry;         
	HI_U8  u8SecondPole;        
	HI_U8  u8Stretch;           
	HI_U8  u8Compress;           
	HI_U8  u8PDStrength;         
	HI_U8  u8LocalMixingBrigtht;
	HI_U8  u8LocalMixingDark;   

	HI_U8  u8ExpRatioType;
	HI_U32 u32ExpRatio;
    HI_U32 u32ExpRatioMax; 
    HI_U32 u32ExpRatioMin; 
    HI_U16 u16GammaTable[257];
	HI_U16 au16SharpenUd[16];
	HI_U8 au8SharpenD[16];	
	HI_U8 au8TextureThr[16];	
	HI_U8 au8SharpenEdge[16];
	HI_U8 au8EdgeThr[16];	
	HI_U8 au8OverShoot[16];	
	HI_U8 au8UnderShoot[16];	
	HI_U8 au8shootSupStr[16];
	HI_U8 au8DetailCtrl[16]; 
    SCENEAUTO_INIPARAM_3DNR_S stTraffic3dnr;
}SCENEAUTO_INIPARAM_TRAFFIC_S;




typedef struct hiSCENEAUTO_EXPOSURE_S
{
    HI_U8 u8AERunInterval;
    HI_U8  u8Speed;
    HI_U8  u8Tolerance;
    HI_U16 u16BlackDelayFrame;
    HI_U16 u16WhiteDelayFrame;
    HI_U32 u32SysGainMax;
    HI_U8 u8AECompesation;
    HI_U8 u8AEHistOffset;
    HI_U16 u16HistRatioSlope;
    HI_U8 au8Weight[AE_WEIGHT_ROW][AE_WEIGHT_COLUMN];
    HI_S32 s32TotalNum;
    SCENEAUTO_ROUTE_NODE_S astRouteNode[16];
}SCENEAUTO_EXPOSURE_S;

typedef struct hiSCENEAUTO_PREVIOUSPARAM_SHADING_S
{
    HI_BOOL bEnable;
    SCENEAUTO_OP_TYPE_E enOpType;
}SCENEAUTO_PREVIOUS_SHADING_S;

typedef struct hiSCENEAUTO_DCI_S
{
    HI_BOOL bDCIEnable;
    HI_U32 u32DCIBlackGain;
    HI_U32 u32DCIContrastGain;
    HI_U32 u32DCILightGain;
}SCENEAUTO_INIPARAM_DCI_S;

typedef struct hiSCENEAUTO_SATURATION_S
{
    HI_U8 u8OpType;
    HI_U8 u8ManuSaturation;
    HI_U8 u8Saturation[16];
}SCENEAUTO_SATURATION_S;
typedef struct hiSCENEAUTO_AWB_S
{
    HI_U8 u8OpType;

    HI_U16 u16BlackLevel;       /*RW, Lower limit of valid data for white region, Range: [0x0, u16WhiteLevel]. for Bayer statistics, bitwidth is 12, for RGB statistics, bitwidth is 10*/
    HI_BOOL bEnable;                /*If enabled, statistic parameter cr, cb will change according to luminance*/
    
    HI_U16  au16CrMax[16];       /*only take effect for lowlight & low color temperature*/
    HI_U16  au16CrMin[16];   
    HI_U16  au16CbMax[16];  
    HI_U16  au16CbMin[16];  
}SCENEAUTO_AWB_S;

typedef struct hiSCENEAUTO_BlackLevel_S
{
    HI_U16 au16BlackLevel[4]; /* RW, Range: [0x0, 0xFFF]*/ 
}SCENEAUTO_BlackLevel_S;

typedef struct hiSCENEAUTO_DRC_S
{
    HI_BOOL bEnable;
    HI_U8 u8OpType;
    HI_U8  u8Strength;
    HI_U8 u8SpatialVar;
    HI_U8 u8RangeVar;
    HI_U8 u8Asymmetry;
    HI_U8 u8SecondPole;
    HI_U8 u8Stretch;
    HI_U8 u8Compress;
    HI_U8 u8PDStrength;
    HI_U8 u8LocalMixingBrigtht;
    HI_U8 u8LocalMixingDark;
}SCENEAUTO_DRC_S;

typedef struct hiSCENEAUTO_INIPARA_S
{
    HI_U8 au8DciStrengthLut[111];
    HI_BOOL BoolRefExporeTime;
    SCENEAUTO_INIPARAM_MPINFO_S stMpInfo;
    SCENEAUTO_INIPARAM_THRESHVALUE_S stThreshValue;
    SCENEAUTO_INIPARAM_AEROUTE_S stIniNormalAeRoute;
    SCENEAUTO_INIPARAM_AEROUTE_S stIniFastAeRoute;
    SCENEAUTO_INIPARAM_GAMMA_S stIniGamma;
    SCENEAUTO_INIPARAM_AE_S stIniAE;
    SCENEAUTO_INIPARAM_AWB_S stIniAWB;
    SCENEAUTO_INIPARAM_BLACK_LEVEL_S stIniBlackLevel;
    SCENEAUTO_INIPARAM_SHADING_S stIniShading;
    SCENEAUTO_INIPARAM_DCI_S stIniDci;
    SCENEAUTO_INIPARAM_SHARPEN_S stIniSharpen;
    SCENEAUTO_INIPARAM_DP_S stIniDP;
	SCENEAUTO_INIPARAM_Ddefog_S stIniDefog;
    SCENEAUTO_INIPARAM_Drc_S stIniDrc;
    SCENEAUTO_INIPARAM_2DNR_S stIni2dnr;
    SCENEAUTO_INIPARAM_FALSECOLOR_S stIniFalseColor;
    SCENEAUTO_INIPARAM_H264VENC_S stIniH264Venc;
    SCENEAUTO_INIPARAM_H265VENC_S stIniH265Venc;
    SCENEAUTO_INIPARAM_3DNR_S stIni3dnr;
    SCENEAUTO_INIPARAM_IR_S stIniIr;
    SCENEAUTO_INIPARAM_HLC_S stIniHlc;
	SCENEAUTO_INIPARAM_TRAFFIC_S stIniTraffic;
}SCENEAUTO_INIPARA_S;
typedef struct hiSCENEAUTO_PREVIOUSPARA_S
{
    SCENEAUTO_SHARPEN_S stSharpen;
    SCENEAUTO_DEPATTR_S stDp;
    SCENEAUTO_GAMMA_S stGamma;
    SCENEAUTO_EXPOSURE_S stExposure;
    SCENEAUTO_PREVIOUS_SHADING_S stShading;
    SCENEAUTO_INIPARAM_DCI_S stDci;
    SCENEAUTO_INIPARAM_2DNR_S st2DNR;
	SCENEAUTO_INIPARAM_Ddefog_S stDefog;
    SCENEAUTO_SATURATION_S stSaturation;
    SCENEAUTO_DRC_S stDrc;
    SCENEAUTO_AWB_S stAwb;
    SCENEAUTO_BlackLevel_S stBlackLevel;
} SCENEAUTO_PREVIOUSPARA_S;


HI_S32 Sceneauto_LoadFile(const HI_CHAR* pszFILENAME);
HI_S32 Sceneauto_LoadINIPara();
HI_VOID Sceneauto_FreeDict();
HI_VOID Sceneauto_FreeMem();
HI_S32 Sceneauto_SetNrb(const SCENEAUTO_INIPARAM_NRS_S *pst3Nrb);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
