/******************************************************************************

  Copyright (C), 2015-2020, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_dci.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2016/01/26
  Description   : 
  History       :
  1.Date        : 2016/01/26
    Author      : q00214668
    Modification: Created file

******************************************************************************/

#include "isp_alg.h"
#include "isp_sensor.h"
#include "isp_config.h"
#include "isp_proc.h"
#include "isp_ext_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

/****************************************************************************
 * GLOBAL VARIABLES                                                         *
 ****************************************************************************/

static HI_S32 dci_gainpos_cbcr[9] = {0,2,3,4,5,6,7,8,9};


static HI_S32 dci_gainpos_thr[7] = {20,30,40,50,60,70,115};


static HI_S32 dci_gainneg_cbcr[9] = {0,1,2,3,3,4,5,6,6};


static HI_S32 dci_gainneg_thr[7] = {30,50,70,90,130,150,170};


static HI_S32 dci_gainpos_slp[8] = {0};


static HI_S32 dci_gainneg_slp[8] = {0};


static HI_U32 wgt_lut_full_range[3][32] = {
    {17,16,15,14,13,11,9,7,4,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,1,2,3,4,6,8,9,10,11,12,13,14,15,16,16,15,14,13,12,11,10,9,8,7,6,5,4,3},                            
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,2,3,4,6,8,9,10,12,13,14,14,15,15,15}};

           
static HI_U32 wgt_lut_limit_range[3][32] = {
    {17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},                
    {0,0,0,0,1,2,3,4,6,8,9,10,11,12,13,14,15,16,16,15,14,13,12,11,10,9,8,7,6,5,4,3},                            
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,2,3,4,6,8,9,10,12,13,14,14,14,14,14}};

          
static HI_S32 adj[3][33] = {
    {0,0,0,-10,-7,1,8,17,24,41,58,75,92,109,130,141,154,164,171,172,169,162,150,136,119,101,82,62,39,6,0,0,0},                
    {0,0,0,-26,-40,-51,-49,-45,-38,-32,-26,-12,-6,13,28,38,53,62,69,70,72,73,70,66,58,52,44,34,10,0,0,0,0},                
    {0,0,0,-50,-72,-82,-83,-82,-85,-90,-97,-102,-110,-114,-116,-116,-114,-109,-103,-98,-93,-88,-82,-77,-69,-60,-47,-31,-15,-2,0,0,0}};


static HI_U32 dci_div[64]= {1,   4095, 2048, 1365, 1024, 819,  682, 585, 
                     512, 455,  409,  372,  341,  315,  292, 273,
                     256, 240,  227,  215,  204,  195,  186, 178, 
                     170, 163,  157,  151,  146,  141,  136, 132, 
                     128, 124,  120,  117,  113,  109,  107, 105,  
                     102, 99,   97,   95,   93,   91,   89,  87,   
                     85,  83,   81,   79,   80,   78,   75,  74,  
                     73,  71,   70,   69,   68,   67,   66,  65};


#define HI_ISP_DCI_BLACK_GAIN_DEFAULT (32)
#define HI_ISP_DCI_CONTRAST_GAIN_DEFAULT (32)
#define HI_ISP_DCI_LIGHT_GAIN_DEFAULT (32)

typedef struct hiHI_ISP_DCI_ATTR
{
    HI_U8 u8BlackGain;       /*u8BlackGain     : [0, 63]*/
    HI_U8 u8ContrastGain;    /*u8ContrastGain : [0, 63]*/
    HI_U8 u8LightGain;       /*u8LightGain      : [0, 63]*/

}ISP_DCI_ATTR;

ISP_DCI_ATTR g_astDciCtx[ISP_MAX_DEV_NUM];

#define DCI_GET_CTX(dev, pstCtx)   pstCtx = &g_astDciCtx[dev]

HI_VOID ISP_Dci_GainPos_Thr_Set(ISP_DEV IspDev)
{
	HI_U8 i;
   
    for(i = 0; i< 7; i++)
    {	
    	hi_isp_dci_gainpos_thr_write(IspDev, i, dci_gainpos_thr[i]);
    }
}

HI_VOID ISP_Dci_GainPos_Cbcr_Set(ISP_DEV IspDev)
{
	HI_U8 i;
 
    for(i = 0; i< 9; i++)
    {	
    	hi_isp_dci_gainpos_cbcr_write(IspDev, i, dci_gainpos_cbcr[i]);
    }
}

HI_VOID ISP_Dci_GainPos_Slp_Set(ISP_DEV IspDev)
{
	HI_U8 i;
 
    for(i = 0; i< 8; i++)
    {	
    	hi_isp_dci_gainpos_slp_write(IspDev, i, dci_gainpos_slp[i]);
    }
}

HI_VOID ISP_Dci_GainNeg_Thr_Set(ISP_DEV IspDev)
{
	HI_U8 i;
 
    for(i = 0; i< 7; i++)
    {	
    	hi_isp_dci_gainneg_thr_write(IspDev, i, dci_gainneg_thr[i]);
    }
}

HI_VOID ISP_Dci_GainNeg_Cbcr_Set(ISP_DEV IspDev)
{
	HI_U8 i;
 
    for(i = 0; i< 9; i++)
    {	
    	hi_isp_dci_gainneg_cbcr_write(IspDev, i, dci_gainneg_cbcr[i]);
    }
}

HI_VOID ISP_Dci_GainNeg_Slp_Set(ISP_DEV IspDev)
{
	HI_U8 i;

    for(i = 0; i< 8; i++)
    {	
    	hi_isp_dci_gainneg_slp_write(IspDev, i, dci_gainneg_slp[i]);
    }
}

HI_VOID ISP_Dci_FullRangeWgt_Set(ISP_DEV IspDev)
{
	HI_U8 i;
 
    for(i = 0; i< 96; i++)
    {	
    	hi_isp_dci_bin_wgt_write(IspDev, i, wgt_lut_full_range[i/32][i%32]);
    }
}

HI_VOID ISP_Dci_LimitRangeWgt_Set(ISP_DEV IspDev)
{
	HI_U8 i;
 
    for(i = 0; i< 96; i++)
    {	
    	hi_isp_dci_bin_wgt_write(IspDev, i, wgt_lut_limit_range[i/32][i%32]);
    }
}

HI_VOID ISP_Dci_AdjWgt_Set(ISP_DEV IspDev)
{
	HI_U8 i;
 
    for(i = 0; i< 99; i++)
    {	
    	hi_isp_dci_adj_wgt_write(IspDev, i, adj[i/33][i%33]);
    }
}

HI_VOID ISP_Dci_DivWgt_Set(ISP_DEV IspDev)
{
	HI_U8 i;
  
    for(i = 0; i< 64; i++)
    {	
    	hi_isp_dci_div_wgt_write(IspDev, i, dci_div[i]);
    }
}


HI_VOID ISP_Dci_GlobalArray_Init()
{
	HI_U8 i;
	
	for(i=0; i<8; i++)
    {
        if(0 == i)
        {
            dci_gainpos_slp[i] = (dci_gainpos_cbcr[i+1] - dci_gainpos_cbcr[i])*256/(dci_gainpos_thr[i] - 0);
        }
        else if(7 == i)
        {
            dci_gainpos_slp[i] = (dci_gainpos_cbcr[i+1] - dci_gainpos_cbcr[i])*256/(255 - dci_gainpos_thr[i-1]);
        }
        else
        {
            dci_gainpos_slp[i] = (dci_gainpos_cbcr[i+1] - dci_gainpos_cbcr[i])*256/(dci_gainpos_thr[i] - dci_gainpos_thr[i-1]);
        }
        
        if (dci_gainpos_slp[i] >= 511)
        {
            dci_gainpos_slp[i] = 511;
        }
        else if(dci_gainpos_slp[i] <= -512)
        {
            dci_gainpos_slp[i] = -512;
        }
        else
        {
            //nothing to do
        }
    }

    for(i=0; i<8; i++)
    {
        if(0 == i)
        {
            dci_gainneg_slp[i] = (dci_gainneg_cbcr[i+1] - dci_gainneg_cbcr[i])*256/(dci_gainneg_thr[i] - 0);
        }
        else if(7 == i)
        {
            dci_gainneg_slp[i] = (dci_gainneg_cbcr[i+1] - dci_gainneg_cbcr[i])*256/(255 - dci_gainneg_thr[i-1]);
        }
        else
        {
            dci_gainneg_slp[i] = (dci_gainneg_cbcr[i+1] - dci_gainneg_cbcr[i])*256/(dci_gainneg_thr[i] - dci_gainneg_thr[i-1]);
        }
        
        if (dci_gainneg_slp[i] >= 511)
        {
            dci_gainneg_slp[i] = 511;
        }
        else if(dci_gainneg_slp[i] <= -512)
        {
            dci_gainneg_slp[i] = -512;
        }
        else
        {
            //nothing to do
        }
    }   
}

HI_VOID DciRegsDefault(ISP_DEV IspDev)
{
    int i;
	
	hi_isp_dci_shiftctrl_write(IspDev, 2);
	hi_isp_dci_dbg_en_write(IspDev, 0);
	hi_isp_dci_cbcrcmp_en_write(IspDev, 1);
	hi_isp_dci_cbcrsta_en_write(IspDev, 0);
	hi_isp_dci_in_range_write(IspDev, 1);
	hi_isp_dci_out_range_write(IspDev, 1);

	hi_isp_dci_cbcrsta_write(IspDev,0,0,64);

	hi_isp_dci_cbcrsta_oft_write(IspDev,0);

	hi_isp_dci_histcor_thr_write(IspDev,3,3,3);
	
	hi_isp_dci_org_abld_write(IspDev,18);
	hi_isp_dci_hist_abld_write(IspDev,16);
	hi_isp_dci_metrc_abld_write(IspDev,17,17,17);

	hi_isp_dci_man_adjwgt_write(IspDev,200,100,230);

    for(i = 0; i < 3; i++)
    {
		hi_isp_dci_wgt_clip_write(IspDev,i,0,255);
    }	

	hi_isp_dci_en_write(IspDev,HI_FALSE);
    
    return;
}
	
HI_VOID ISP_SetDciImageSize(ISP_DEV IspDev)
{
	HI_U32 u32X, u32Y, u32Width, u32Height;

	u32X = 0;
	u32Y = 0;
	u32Width  = hi_ext_sync_total_width_read();
	u32Height = hi_ext_sync_total_height_read();
	
	hi_isp_dci_hpos_write(IspDev, u32X, u32Width);   
	hi_isp_dci_vpos_write(IspDev, u32Y, u32Height);


	hi_isp_acm_width_write(IspDev, u32Width-1);
	hi_isp_acm_height_write(IspDev, u32Height-1);
}

static HI_VOID DciRegsInitialize(ISP_DEV IspDev)
{
	ISP_DCI_ATTR *pstDciCtx = HI_NULL;
    
    DCI_GET_CTX(IspDev, pstDciCtx);

	pstDciCtx->u8BlackGain    = HI_ISP_DCI_BLACK_GAIN_DEFAULT;
	pstDciCtx->u8ContrastGain = HI_ISP_DCI_CONTRAST_GAIN_DEFAULT;
	pstDciCtx->u8LightGain    = HI_ISP_DCI_LIGHT_GAIN_DEFAULT;

	ISP_Dci_GlobalArray_Init(IspDev);
	
	ISP_Dci_GainPos_Thr_Set(IspDev);
	ISP_Dci_GainPos_Cbcr_Set(IspDev);
	ISP_Dci_GainPos_Slp_Set(IspDev);

	ISP_Dci_GainNeg_Thr_Set(IspDev);
	ISP_Dci_GainNeg_Cbcr_Set(IspDev);
	ISP_Dci_GainNeg_Slp_Set(IspDev);

	ISP_Dci_FullRangeWgt_Set(IspDev);
	//ISP_Dci_LimitRangeWgt_Set(IspDev);

	ISP_Dci_AdjWgt_Set(IspDev);
	ISP_Dci_DivWgt_Set(IspDev);

	hi_isp_dci_glb_gain_write(IspDev,pstDciCtx->u8BlackGain,pstDciCtx->u8ContrastGain,pstDciCtx->u8LightGain);

	DciRegsDefault(IspDev);

	ISP_SetDciImageSize(IspDev);

    return;
}

//static HI_S32 DciReadExtregs(ISP_DEV IspDev)
//{   
//	ISP_DCI_ATTR *pstDciCtx = HI_NULL;
//    
//    DCI_GET_CTX(IspDev, pstDciCtx);
//    
//    return 0;
//}

HI_S32 DciProcWrite(ISP_DEV IspDev, ISP_CTRL_PROC_WRITE_S *pstProc)
{
    //ISP_CTRL_PROC_WRITE_S stProcTmp;
    //ISP_DCI_ATTR *pstDciCtx = HI_NULL;
    
    //DCI_GET_CTX(IspDev, pstDciCtx);
 
    if ((HI_NULL == pstProc->pcProcBuff)
        || (0 == pstProc->u32BuffLen))
    {
        return HI_FAILURE;
    }

	//DciReadExtregs(IspDev);
    #if 0
    stProcTmp.pcProcBuff = pstProc->pcProcBuff;
    stProcTmp.u32BuffLen = pstProc->u32BuffLen;
   
    ISP_PROC_PRINTF(&stProcTmp, pstProc->u32WriteLen,
        "-----DCI INFO-------------------------------------------------------------\n");


    pstProc->u32WriteLen += 1;
    #endif 
    return HI_SUCCESS;
}


HI_S32 ISP_DciInit(ISP_DEV IspDev)
{
    DciRegsInitialize(IspDev);
    
    return HI_SUCCESS;
}

HI_S32 ISP_DciRun(ISP_DEV IspDev, const HI_VOID *pStatInfo,
    HI_VOID *pRegCfg, HI_S32 s32Rsv)
{
    return HI_SUCCESS;
}

HI_S32 ISP_DciCtrl(ISP_DEV IspDev, HI_U32 u32Cmd, HI_VOID *pValue)
{
    switch (u32Cmd)
    {
        case ISP_WDR_MODE_SET :
            ISP_DciInit(IspDev);
            break;
        case ISP_PROC_WRITE:
            DciProcWrite(IspDev, (ISP_CTRL_PROC_WRITE_S *)pValue);
            break;
        default :
            break;
    }
    return HI_SUCCESS;
}

HI_S32 ISP_DciExit(ISP_DEV IspDev)
{
    return HI_SUCCESS;
}

HI_S32 ISP_AlgRegisterDci(ISP_DEV IspDev)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_ALG_NODE_S *pstAlgs = HI_NULL;
    
    ISP_GET_CTX(IspDev, pstIspCtx);

    pstAlgs = ISP_SearchAlg(pstIspCtx->astAlgs);
    ISP_CHECK_POINTER(pstAlgs);

    pstAlgs->enAlgType = ISP_ALG_DCI;
    pstAlgs->stAlgFunc.pfn_alg_init = ISP_DciInit;
    pstAlgs->stAlgFunc.pfn_alg_run  = ISP_DciRun;
    pstAlgs->stAlgFunc.pfn_alg_ctrl = ISP_DciCtrl;
    pstAlgs->stAlgFunc.pfn_alg_exit = ISP_DciExit;
    pstAlgs->bUsed = HI_TRUE;

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


