#ifndef __HI_SCENE_AUTO_VPSS_H__
#define __HI_SCENE_AUTO_VPSS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */


typedef struct { int  NRc, SFc, TFc, TFp, HHb, IHb, IEx, TEx; } tVppNRc;
typedef struct 
{ 
	int  ENy,ROd, Mdn,Swp, DPy,DSy, bJm,Pro, DPs,DSs, Dp[5],Ds[5]; 

	int  MMW,     WFy,WTy, SFy,     PIy,SIy ,MMy,TRy, PJy,SJy, RWy,SWy, ADy;
	int  Blc,BRt, Wht,WRt, IEs[4],  PIi,SIi, MMi,TRi, PJi,SJi, RWi,SWi, ADi;
	int  MDp,MDz, MDt,MDs, IEr,ROW, ADt,DAf, Big,ADh, bZs,rZt, ADs,Slp, ADp;
} tVppNRy;

typedef struct
{
	int  tss,      sfc, tfc;
	int  sfs, tfs, mdz, ies;

}tVppNRs;

typedef struct
{
	int  SFS,  SFS1, SPS1,SPT1,  TFP1,TFR1,SBF1,  MDDZ, MAAT,  SFC;
	int  TFS,  SFS2, SPS2,SPT2,  TFP2,TFR2,SBF2,  MATH, MATW,  TFC;

}tVppNRz;

typedef struct 
{
	int  StStrength, StProSpati, StProTempo, StBaseFreq;
	int  TFS,   TFP, TFQ, TFR,     SHP,      MDDZ, MATH;

}tVppNRx;

typedef struct 
{ 
	tVppNRs iNRs;  
	tVppNRz iNRz;  
	tVppNRx iNRx[3];  
	tVppNRy iNRy[3];
	tVppNRc iNRc;

}VppNR;

typedef struct  hiVPSS_NRY_S
{
	tVppNRy  iNRy[3];
	tVppNRc  iNRc;
}VPSS_NRY_S;

typedef struct
{
	HI_U16  TFR : 5 ;
	HI_U16  TFP : 7 ;
	HI_U16  TFS : 4 ;
	HI_U8   SHP     ;

	HI_U8   SBS  ;
	HI_U8   SDS  ;
	HI_U8   SFB  ;
	HI_U8   SBF  : 3 ;
	HI_U8   SBF1 : 3, _reserved_b_ : 2, _reserved_B_;
	HI_U16  SBFR : 4;
	HI_U16  MDDZ : 7;

	HI_U16  MTFR : 5 ;
	HI_U16  MTFP : 6 ;
	HI_U16  MTFS : 4 ;
	HI_U16  MSHP : 6 ;

} tVppNRbUnit;  

typedef struct
{
	int  ISO;

	tVppNRbUnit Unit[4];  

	HI_U8   SFC     , _reserved_B_[2];
	HI_U8   TFC : 6 ,  _reserved_b_ : 2;

	HI_U16  PostROW : 6, _reserved_b1_ : 6;
	HI_U16  PostSFS : 4;

	HI_U16  MATH : 9, MATW : 3, MABW : 1, MDAF : 3; 

} tVppNRb;

typedef struct  hiVPSS_NRB_S
{
	tVppNRb  iNRb;
}VPSS_NRB_S;

typedef struct { HI_U8   Pro : 2, DualDir : 1, EdgEx : 2, SaSfrEn : 1, _reserved_b_ : 1; 
	HI_U8   MamiMode : 1, TexThr, MinFactor, MaxFactor; }  tV19zMainSFyEx;

	typedef struct { HI_U8   SBS,SDS, SBT,SDT;  tV19zMainSFyEx  EX; } tV19zMainSFy;  

	typedef struct { HI_U8   BlcTxtMax, BlcIEB /*[0,63]*/, BlcSlope; 
		HI_U8   WhtTxtMax, WhtIEB /*[0,63]*/, WhtSlope; 

		HI_U16  BlcDzLeft, WhtDzLeft; } tV19zIEyEx;  

		typedef struct { HI_U8   BlcIES, BlcSht/*[0,32]*/; 
			HI_U8   WhtIES, WhtSht/*[0,32]*/;  tV19zIEyEx  EX; } tV19zIEy;  
			typedef struct 
{ 
	HI_U16  MADiMax0 : 7, MADiThr0 : 9, MABW0 : 5, MADiSlp0 : 4, MDAF : 4, _reserved_b0_ : 3;
	HI_U16  MADiMax1 : 7, MADiThr1 : 9, MABW1 : 5, MADiSlp1 : 4, DADF : 4, _reserved_b1_ : 3; 

	HI_U8   HorDila : 5, HorDilaMode : 1, MSLP : 2; 
	HI_U8   VerDila : 5, VerDilaMode : 1, MATW : 2;

} tV19zMDyEx;  

typedef struct { HI_U16  MaThr0,  MaThr1; tV19zMDyEx  EX; } tV19zMDy;    

typedef struct 
{ 
	HI_U16  MADiSlpR0 : 5;
	HI_U16  MADiSlpR1 : 5;
	HI_U16  MADiSlpR2 : 5;

	HI_U16 _reserved_b0_ : 1;

	HI_U8   Base0 : 4,   Peak0 : 4;
	HI_U8   Base1 : 4,   Peak1 : 4;
	HI_U8   Base2 : 4,   Peak2 : 4;
	HI_U8   Base3 : 4,   Peak3 : 4;

	HI_U16 _reserved_b1_ : 1;

	HI_U16  PkRt0 : 5, BlcRt : 5; 
	HI_U16  PkRt1 : 5, WhtRt : 5; 
	HI_U16  PkRt2 : 5;
	HI_U16  PkRt3 : 5,  ExMD : 1;

} tV19zCompSFyEx;  

typedef struct 
{ 
	HI_U16  MADiThr0;
	HI_U16  MADiThr1;
	HI_U16  MADiThr2;

	HI_U8   MDDZ, _reserved_B_;  

	tV19zCompSFyEx  EX; 
} tV19zCompSFy;    

typedef struct 
{ 
	HI_U8   MamiCoSlp : 2, TFRiDelta : 4;
	HI_U8  _reserved_ : 2, MADiSlpRt;
	HI_U8   TSWi,          MADpSlpRt; 

	HI_U8   MaTFRi, MaTFRj, MamiMax, MaTFR[5];
	HI_U8   SaTFRi, SaTFRj, SaAbsTF, SaTFR[5];

	HI_U16  MaTDZi[4], SaTDZi[4], MADiThr, MADpThr;

	HI_U16  TFRiMask  : 9, ExMode : 2, MaOld : 1; 
	HI_U16  SaMaSrcEn : 1, ExType : 2, SaOld : 1; 

} tV19zTFyEx;  

typedef struct 
{ 
	HI_U16  MaTDZi; 
	HI_U16  SaTDZi;

	HI_U8   MaSHP, MaTFR, TRWi; 
	HI_U8   SaSHP, SaTFR, TRWj; 

	tV19zTFyEx  EX; 
} tV19zTFy;    

typedef struct
{
	int  ISO;

	tV19zMainSFy  iMainSFy[4]; 
	tV19zCompSFy  iMaCoSFy[4]; 
	tV19zCompSFy  iSaCoSFy[4]; 

	tV19zIEy    iIEy[4];
	tV19zMDy    iMDy[4];
	tV19zTFy    iTFy[4];

	HI_U8  PostSBS : 6, PostBW : 1, ClassicEn : 1;
	HI_U8  PostSDS : 6, PostSF : 1, V19zNRaEx : 1;

	HI_U8  PostROW : 4, En0 : 1, En1 : 1, En2 : 1, En3 : 1;
	HI_U8  SFC,TFC, RefMode : 2, _reserved_b_ : 6;

} tV19zNRa;

typedef struct  hiVPSS_NR19_S
{
	tV19zNRa  iNR19;
}VPSS_NR19_S;


//////////////////////////////////////////////////////////////////////////
/*Defined detailed image quality debug param*/
typedef struct hiVPSS_IMG_QUALITY_PARAM_S
{    
	union
	{
		VPSS_NRY_S stNRY;    // not use
		VPSS_NRB_S stNRB;    // not use 
		VPSS_NR19_S stNR19;
	};
	HI_U32 u32Reserved[6];
}VPSS_IMG_QUALITY_PARAM_S;

/*Defined private image quality debug config struct*/
typedef struct hiVPSS_IMG_QUALITY_CFG_S
{
	HI_BOOL bEnable;
	VPSS_IMG_QUALITY_PARAM_S  stImageQualityParam;
}VPSS_IMG_QUALITY_CFG_S;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif  /* __HI_SCENE_AUTO_VPSS*/
