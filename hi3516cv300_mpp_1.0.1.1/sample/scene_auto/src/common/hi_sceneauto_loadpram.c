#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "iniparser.h"
#include "hi_sceneauto_comm.h"
#include "hi_math.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

extern SCENEAUTO_INIPARA_S g_stINIPara;
static dictionary* g_Sceneautodictionary = NULL;

static int MAEWeight[512];
static int  Weight(const char* b)
{
    const  char*    pszVRBegin     = b;
    const char*    pszVREnd       = pszVRBegin;
    int      u32Count = 0;
    char     temp[20];
    int      mycount = 0;
    int      length = strlen(b);
    unsigned int re;
    memset(temp, 0, 20);
    int i = 0;
    HI_BOOL bx = HI_FALSE;

    while ((pszVREnd != NULL))
    {
        if ((mycount > length) || (mycount == length))
        {
            break;
        }
        while ((*pszVREnd != '|') && (*pszVREnd != '\0') && (*pszVREnd != ','))
        {
            if (*pszVREnd == 'x')
            {
                bx = HI_TRUE;
            }
            pszVREnd++;
            u32Count++;
            mycount++;
        }
        memcpy(temp, pszVRBegin, u32Count);

        if (bx == HI_TRUE)
        {
            char* str;
            re = (int)strtol(temp + 2, &str, 16);
            MAEWeight[i] = re;

        }
        else
        {
            MAEWeight[i] = atoi(temp);
        }
        memset(temp, 0, 20);
        u32Count = 0;
        pszVREnd++;
        pszVRBegin = pszVREnd;
        mycount++;
        i++;
    }
    return i;
}


HI_S32 Load3DNR(SCENEAUTO_INIPARAM_3DNR_S *pstIni3dnr, const char *pstr)
{

	HI_S32 s32Temp;
	HI_S32 i;
	HI_CHAR szTempStr[128];
	

   for (i = 0; i < pstIni3dnr->u323DnrIsoCount; i++)
   {
	   //3dnr:s32IES0 
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32IES0_%d",pstr,i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		   printf("%s:s32IES0_%d failed\n", pstr,i);
		   return HI_FAILURE;
	   }
	   pstIni3dnr->pst3dnrParam[i].s32IES0 = s32Temp;  

		//IR:s32SBS0 
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32SBS0_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		   printf("%s:s32SBS0_%d failed\n",pstr, i);
		   return HI_FAILURE;
	   }
	   pstIni3dnr->pst3dnrParam[i].s32SBS0 = s32Temp;  



	  //IR:s32SBS1 
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32SBS1_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		   printf("%s:s32SBS1_%d failed\n",pstr, i);
		   return HI_FAILURE;
	   }
	   pstIni3dnr->pst3dnrParam[i].s32SBS1 = s32Temp;  

	  //IR:s32SBS2 
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32SBS2_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		   printf("%s:s32SBS2_%d failed\n",pstr, i);
		   return HI_FAILURE;
	   }
	  pstIni3dnr->pst3dnrParam[i].s32SBS2 = s32Temp;  

	  //IR:s32SBS3 
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32SBS3_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		   printf("%s:s32SBS3_%d failed\n",pstr, i);
		   return HI_FAILURE;
	   }
	   pstIni3dnr->pst3dnrParam[i].s32SBS3 = s32Temp;  


		//IR:s32SDS0 
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32SDS0_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		   printf("%s:s32SDS0_%d failed\n",pstr, i);
		   return HI_FAILURE;
	   }
	   pstIni3dnr->pst3dnrParam[i].s32SDS0 = s32Temp;  

		//IR:s32SDS1 
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32SDS1_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		   printf("%s:s32SDS1_%d failed\n",pstr, i);
		   return HI_FAILURE;
	   }
	   pstIni3dnr->pst3dnrParam[i].s32SDS1 = s32Temp; 
	   
		//IR:s32SDS2 
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32SDS2_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		   printf("%s:s32SDS2_%d failed\n",pstr, i);
		   return HI_FAILURE;
	   }
	   pstIni3dnr->pst3dnrParam[i].s32SDS2 = s32Temp; 
	   
		//IR:s32SDS3 
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32SDS3_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		   printf("%s:s32SDS3_%d failed\n",pstr, i);
		   return HI_FAILURE;
	   }
	    pstIni3dnr->pst3dnrParam[i].s32SDS3 = s32Temp; 

		//IR:s32STH0 
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32STH0_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		   printf("%s:s32STH0_%d failed\n",pstr, i);
		   return HI_FAILURE;
	   }
	   pstIni3dnr->pst3dnrParam[i].s32STH0 = s32Temp; 

	   //IR:s32STH1 
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32STH1_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		   printf("%s:s32STH1_%d failed\n",pstr, i);
		   return HI_FAILURE;
	   }
	    pstIni3dnr->pst3dnrParam[i].s32STH1 = s32Temp;  

	   //IR:s32STH2 
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32STH2_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		   printf("%s:s32STH2_%d failed\n",pstr, i);
		   return HI_FAILURE;
	   }
	   pstIni3dnr->pst3dnrParam[i].s32STH2 = s32Temp; 

	   //IR:s32STH3 
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32STH3_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		   printf("%s:s32STH3_%d failed\n",pstr, i);
		   return HI_FAILURE;
	   }
	    pstIni3dnr->pst3dnrParam[i].s32STH3 = s32Temp;  

	   //IR:s32MDP 
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32MDP_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		   printf("%s:s32MDP_%d failed\n",pstr, i);
		   return HI_FAILURE;
	   }
	    pstIni3dnr->pst3dnrParam[i].s32MDP = s32Temp;	

	   //IR:s32MATH1 
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32MATH1_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		 printf("%s:s32MATH1_%d failed\n",pstr, i);
		 return HI_FAILURE;
	   }
	    pstIni3dnr->pst3dnrParam[i].s32MATH1 = s32Temp;

	   //IR:s32MATH2 
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32MATH2_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		 printf("%s:s32MATH2_%d failed\n",pstr, i);
		 return HI_FAILURE;
	   }
	    pstIni3dnr->pst3dnrParam[i].s32MATH2 = s32Temp;  

	   
		 //IR:s32Pro3 
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32Pro3_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		   printf("%s:s32Pro3_%d failed\n",pstr, i);
		   return HI_FAILURE;
	   }
	    pstIni3dnr->pst3dnrParam[i].s32Pro3 = s32Temp; 
	   
	   //IR:s32MDDZ1 
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32MDDZ1_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		   printf("%s:s32MDDZ1_%d failed\n",pstr, i);
		   return HI_FAILURE;
	   }
	    pstIni3dnr->pst3dnrParam[i].s32MDDZ1 = s32Temp; 

	   //IR:s32MDDZ2 
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32MDDZ2_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		   printf("%s:s32MDDZ2_%d failed\n",pstr, i);
		   return HI_FAILURE;
	   }
	    pstIni3dnr->pst3dnrParam[i].s32MDDZ2 = s32Temp;   


	   //IR:s32TFS1 
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32TFS1_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		   printf("%s:s32TFS1_%d failed\n",pstr, i);
		   return HI_FAILURE;
	   }
	   pstIni3dnr->pst3dnrParam[i].s32TFS1= s32Temp; 
	   
	   //IR:s32TFS2 
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32TFS2_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		   printf("%s:s32TFS2_%d failed\n",pstr, i);
		   return HI_FAILURE;
	   }
	   pstIni3dnr->pst3dnrParam[i].s32TFS2 = s32Temp;


	   //IR:s32SFC
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32SFC_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		 printf("%s:s32SFC_%d failed\n",pstr, i);
		 return HI_FAILURE;
	   }

	    pstIni3dnr->pst3dnrParam[i].s32SFC = s32Temp;


	   //IR:s32TFC
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32TFC_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		 printf("%s:s32TFC_%d failed\n",pstr, i);
		 return HI_FAILURE;
	   }
	    pstIni3dnr->pst3dnrParam[i].s32TFC = s32Temp;

	   //IR:s32TPC
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32TPC_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		 printf("%s:s32TPC_%d failed\n",pstr, i);
		 return HI_FAILURE;
	   }
	    pstIni3dnr->pst3dnrParam[i].s32TPC = s32Temp;

	   //IR:s32TRC	
	   snprintf_s(szTempStr, 128,sizeof(szTempStr), "%s:s32TRC_%d",pstr, i);
	   s32Temp = 0;
	   s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
	   if (HI_FAILURE == s32Temp)
	   {
		 printf("%s:s32TRC_%d failed\n",pstr, i);
		 return HI_FAILURE;
	   }
	    pstIni3dnr->pst3dnrParam[i].s32TRC = s32Temp;


   }   

	return HI_SUCCESS;

}

HI_S32 Sceneauto_LoadHlc()
{
    HI_S32 s32Temp;
	HI_U32 u32Temp;	
    HI_S32 s32Offset;
    HI_S32 i;
    HI_CHAR szTempStr[128];
    HI_CHAR* pszTempStr;
	HI_CHAR mode[4]= "HLC";
    
    /**************HLC:ExpCompensation**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "HLC:ExpCompensation", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
    	printf("HLC:ExpCompensation failed\n");
    }
    g_stINIPara.stIniHlc.u8ExpCompensation = (u32Temp&0xFF);

    /**************HLC:Saturation**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "HLC:Saturation");
    if (NULL == pszTempStr)
    {
        printf("HLC:Saturation error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < 16; i++)
    {
        g_stINIPara.stIniHlc.u8Saturation[i] = MAEWeight[i];
    }

    /**************HLC:BlackDelayFrame**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "HLC:BlackDelayFrame", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("HLC:BlackDelayFrame failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniHlc.u16BlackDelayFrame = (u32Temp&0xFF);

    /**************HLC:WhiteDelayFrame**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "HLC:WhiteDelayFrame", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("HLC:WhiteDelayFrame failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniHlc.u16WhiteDelayFrame = (u32Temp&0xFFFF);

    /**************HLC:u8Speed**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "HLC:u8Speed", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("HLC:u8Speed failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniHlc.u8Speed = (u32Temp&0xFF);

    /**************HLC:HistRatioSlope**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "HLC:HistRatioSlope", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("HLC:HistRatioSlope failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniHlc.u16HistRatioSlope = (u32Temp&0xFFFF);

    /**************HLC:MaxHistOffset**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "HLC:MaxHistOffset", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("HLC:MaxHistOffset failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniHlc.u8MaxHistOffset = (u32Temp&0xFF);

    /**************HLC:u8Tolerance**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "HLC:u8Tolerance", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("HLC:u8Tolerance failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniHlc.u8Tolerance = (u32Temp&0xFF);

    /**************HLC:DCIEnable**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "HLC:DCIEnable", HI_FAILURE);
    if (HI_FAILURE == s32Temp)
    {
        printf("HLC:DCIEnable failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniHlc.bDCIEnable = (HI_BOOL)u32Temp;

    /**************HLC:DCIBlackGain**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "HLC:DCIBlackGain", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("HLC:DCIBlackGain failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniHlc.u32DCIBlackGain = u32Temp;

    /**************HLC:DCIContrastGain**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "HLC:DCIContrastGain", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("HLC:DCIContrastGain failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniHlc.u32DCIContrastGain = u32Temp;

    /**************HLC:DCILightGain**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "HLC:DCILightGain", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("HLC:DCILightGain failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniHlc.u32DCILightGain = u32Temp;

    /**************HLC:DRCEnable**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "HLC:DRCEnable", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("HLC:DRCEnable failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniHlc.bDRCEnable = (HI_BOOL)u32Temp;

    /**************HLC:DRCManulEnable**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "HLC:DRCManulEnable", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("HLC:DRCManulEnable failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniHlc.bDRCManulEnable = (HI_BOOL)u32Temp;

    /**************HLC:DRCStrengthTarget**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "HLC:DRCStrengthTarget", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("HLC:DRCStrengthTarget failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniHlc.u8DRCStrengthTarget = (u32Temp&0xFF);

    /**************HLC:gamma**************/
    s32Offset = 0;
    snprintf_s(szTempStr, 0x10000,sizeof(szTempStr), "HLC:gamma_0");
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
    if (NULL == pszTempStr)
    {
        printf("HLC:gamma_0 error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < s32Temp; i++)
    {
        g_stINIPara.stIniHlc.u16GammaTable[i] = MAEWeight[i];
    }
    s32Offset += s32Temp;

    snprintf_s(szTempStr, 0x10000,sizeof(szTempStr), "HLC:gamma_1");
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
    if (NULL == pszTempStr)
    {
        printf("HLC:gamma_1 error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < s32Temp; i++)
    {
        g_stINIPara.stIniHlc.u16GammaTable[s32Offset + i] = MAEWeight[i];
    }
    s32Offset += s32Temp;

    snprintf_s(szTempStr, 128,sizeof(szTempStr), "HLC:gamma_2");
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
    if (NULL == pszTempStr)
    {
        printf("HLC:gamma_2 error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < s32Temp; i++)
    {
        g_stINIPara.stIniHlc.u16GammaTable[s32Offset + i] = MAEWeight[i];
    }

	/**************HLC:3DNR**************/
	/**************HLC:BoolLNTH**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "HLC:BoolLNTH", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("TRAFFIC:BoolLNTH failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniHlc.stHLC3dnr.BoolLNTH = (HI_BOOL)u32Temp;  
    
	/**************HLC:3DnrIsoCount**************/
	u32Temp = 0;
	u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "HLC:3DnrIsoCount", HI_FAILURE);
	if (HI_FAILURE == u32Temp)
	{
		printf("HLC:3DnrIsoCount failed\n");
		return HI_FAILURE;
	}	 
	g_stINIPara.stIniHlc.stHLC3dnr.u323DnrIsoCount= u32Temp;
	if(g_stINIPara.stIniHlc.stHLC3dnr.u323DnrIsoCount > 0)
	{
		g_stINIPara.stIniHlc.stHLC3dnr.pu323DnrIsoThresh = (HI_U32 *)malloc((g_stINIPara.stIniHlc.stHLC3dnr.u323DnrIsoCount) * sizeof(HI_U32));
		CHECK_NULL_PTR(g_stINIPara.stIniHlc.stHLC3dnr.pu323DnrIsoThresh);
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam = (SCENEAUTO_INIPARAM_NRS_S *)malloc((g_stINIPara.stIniHlc.stHLC3dnr.u323DnrIsoCount) * sizeof(SCENEAUTO_INIPARAM_NRS_S));
		CHECK_NULL_PTR(g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam);
	}
	
	/**************HLC:3DnrIsoThresh**************/
	pszTempStr = iniparser_getstr(g_Sceneautodictionary, "HLC:3DnrIsoThresh");
	if (NULL == pszTempStr)
	{
		printf("HLC:3DnrIsoThresh error\n");
		return HI_FAILURE;
	}
	s32Temp = Weight(pszTempStr);
	for (i = 0; i < g_stINIPara.stIniHlc.stHLC3dnr.u323DnrIsoCount; i++)
	{
		g_stINIPara.stIniHlc.stHLC3dnr.pu323DnrIsoThresh[i] = MAEWeight[i];
	}


	Load3DNR(&g_stINIPara.stIniHlc.stHLC3dnr,mode);
	
#if 0	
	for (i = 0; i < g_stINIPara.stIniHlc.stHLC3dnr.u323DnrIsoCount; i++)
	{	
		//HLC:s32IES0 
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "HLC:s32IES0_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32IES_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32IES0 = s32Temp;  
	
		 //HLC:s32SBS0 
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "HLC:s32SBS0_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32SBS0_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32SBS0 = s32Temp;  
	
	
	
		//HLC:s32SBS1 
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "HLC:s32SBS1_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32SBS1_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32SBS1 = s32Temp;  
	
		//HLC:s32SBS2 
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "HLC:s32SBS2_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32SBS2_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32SBS2 = s32Temp;  
	
		//HLC:s32SBS3 
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "HLC:s32SBS3_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32SBS3_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32SBS3 = s32Temp;  
	
	
		 //HLC:s32SDS0 
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "HLC:s32SDS0_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32SDS0_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32SDS0 = s32Temp;  
	
		 //HLC:s32SDS1 
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "HLC:s32SDS1_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32SDS1_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32SDS1 = s32Temp; 
		
		 //HLC:s32SDS2 
		snprintf_s(szTempStr,128, sizeof(szTempStr), "HLC:s32SDS2_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32SDS2_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32SDS2 = s32Temp; 
		
		 //HLC:s32SDS3 
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "HLC:s32SDS3_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32SDS3_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32SDS3 = s32Temp; 
	
		 //HLC:s32STH0 
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "HLC:s32STH0_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32STH0_%d failed\n", i);
		return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32STH0 = s32Temp; 
	
		//HLC:s32STH1 
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "HLC:s32STH1_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32STH1_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32STH1 = s32Temp;  
	
		//HLC:s32STH2 
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "HLC:s32STH2_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32STH2_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32STH2 = s32Temp; 
	
		//HLC:s32STH3 
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "HLC:s32STH3_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32STH3_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32STH3 = s32Temp;  
	
		//HLC:s32MDP 
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "HLC:s32MDP_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32MDP_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32MDP = s32Temp;  
	
	
		//HLC:s32MATH1 
		snprintf_s(szTempStr,128, sizeof(szTempStr), "HLC:s32MATH1_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32MATH1_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32MATH1 = s32Temp;
	
		//HLC:s32MATH2 
		snprintf_s(szTempStr, 128, sizeof(szTempStr), "HLC:s32MATH2_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32MATH2_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32MATH2 = s32Temp;	
	
		//HLC:s32Pro3 
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "HLC:s32Pro3_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32Pro3_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32Pro3 = s32Temp; 
	
			
		//HLC:s32MDDZ1 
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "HLC:s32MDDZ1_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32MDDZ1_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32MDDZ1 = s32Temp; 
	
		//HLC:s32MDDZ2 
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "HLC:s32MDDZ2_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32MDDZ2_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32MDDZ2 = s32Temp;	
	
		//HLC:s32TFS1 
		snprintf_s(szTempStr, 128, sizeof(szTempStr), "HLC:s32TFS1_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32TFS1_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32TFS1 = s32Temp; 
	
		//HLC:s32TFS2 
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "HLC:s32TFS2_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32TFS2_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32TFS2 = s32Temp; 
	
	
		//HLC:s32SFC
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "HLC:s32SFC_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32SFC_%d failed\n", i);
			return HI_FAILURE;
		}
	
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32SFC = s32Temp;
	
		//HLC:s32TFC
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "HLC:s32TFC_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32TFC_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32TFC = s32Temp;
	
		//HLC:s32TPC
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "HLC:s32TPC_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			printf("HLC:s32TPC_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32TPC = s32Temp;
	
		//HLC:s32TRC  
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "HLC:s32TRC_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
			 printf("HLC:s32TRC_%d failed\n", i);
			return HI_FAILURE;
		}
		g_stINIPara.stIniHlc.stHLC3dnr.pst3dnrParam[i].s32TRC = s32Temp;
	
			 
	}
#endif

	
    return HI_SUCCESS;
}

HI_S32 Sceneauto_LoadTraffic()
{
    HI_S32 s32Temp;
	HI_U32 u32Temp;
    HI_S32 s32Offset;
    HI_S32 i;
    HI_CHAR szTempStr[128];
    HI_CHAR* pszTempStr;
    HI_CHAR mode[8]= "TRAFFIC";

    /**************TRAFFIC:u8SpatialVar**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "TRAFFIC:DCIEnable", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("IR:DCIEnable failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniTraffic.bDCIEnable = (HI_BOOL)u32Temp;

    /**************TRAFFIC:DCIBlackGain**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "TRAFFIC:DCIBlackGain", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("IR:DCIBlackGain failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniTraffic.u32DCIBlackGain = (HI_BOOL)u32Temp;

    /**************TRAFFIC:DCIContrastGain**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "TRAFFIC:DCIContrastGain", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("IR:DCIContrastGain failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniTraffic.u32DCIContrastGain = (HI_BOOL)u32Temp;

    /**************IR:DCILightGain**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "TRAFFIC:DCILightGain", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("IR:DCILightGain failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniTraffic.u32DCILightGain = (HI_BOOL)u32Temp;

	
    /**************TRAFFIC:u8SpatialVar**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "TRAFFIC:u8SpatialVar", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("TRAFFIC:u8SpatialVar failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniTraffic.u8SpatialVar = (u32Temp&0xFF);

    /**************TRAFFIC:u8RangeVar**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "TRAFFIC:u8RangeVar", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("TRAFFIC:u8RangeVar failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniTraffic.u8RangeVar = (u32Temp&0xFF);

    /**************TRAFFIC:u8Asymmetry**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "TRAFFIC:u8Asymmetry", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("TRAFFIC:u8Asymmetry failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniTraffic.u8Asymmetry = (u32Temp&0xFF);

    /**************TRAFFIC:u8SecondPole**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "TRAFFIC:u8SecondPole", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("TRAFFIC:u8SecondPole failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniTraffic.u8SecondPole = (u32Temp&0xFF);

    /**************TRAFFIC:u8Stretch**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "TRAFFIC:u8Stretch", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("TRAFFIC:u8Stretch failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniTraffic.u8Stretch = (u32Temp&0xFF);

    /**************TRAFFIC:u8Compress**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "TRAFFIC:u8Compress", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("TRAFFIC:u8Compress failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniTraffic.u8Compress = (u32Temp&0xFF);

    /**************TRAFFIC:u8PDStrength**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "TRAFFIC:u8PDStrength", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("TRAFFIC:u8PDStrength failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniTraffic.u8PDStrength = (u32Temp&0xFF);

    /**************TRAFFIC:u8LocalMixingBrigtht**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "TRAFFIC:u8LocalMixingBrigtht", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("TRAFFIC:u8LocalMixingBrigtht failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniTraffic.u8LocalMixingBrigtht = (u32Temp&0xFF);

    /**************TRAFFIC:u8LocalMixingDark**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "TRAFFIC:u8LocalMixingDark", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("TRAFFIC:u8LocalMixingDark failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniTraffic.u8LocalMixingDark = (u32Temp&0xFF);

    /**************TRAFFIC:ExpCompensation**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "TRAFFIC:u8ExpCompensation", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
    	printf("TRAFFIC:ExpCompensation failed\n");
    }
    g_stINIPara.stIniTraffic.u8ExpCompensation = (u32Temp&0xFF);

    /**************TRAFFIC:MaxHistoffset **************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "TRAFFIC:u8MaxHistoffset", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
    	printf("TRAFFIC:MaxHistoffset failed\n");
    }
    g_stINIPara.stIniTraffic.u8MaxHistoffset = (u32Temp&0xFF);

    /**************TRAFFIC:u8ExpRatioType**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "TRAFFIC:u8ExpRatioType", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("TRAFFIC:u8ExpRatioType failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniTraffic.u8ExpRatioType = (u32Temp&0xFF);

    /**************TRAFFIC:u32ExpRatio**************/
	//u32Temp = 64;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "TRAFFIC:u32ExpRatio",HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("TRAFFIC:u32ExpRatio error\n");
        return HI_FAILURE;
    }
    //s32Temp = Weight(pszTempStr);

    g_stINIPara.stIniTraffic.u32ExpRatio = u32Temp;


    /**************TRAFFIC:u32ExpRatioMax**************/
   // u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "TRAFFIC:u32ExpRatioMax", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("TRAFFIC:u32ExpRatioMax failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniTraffic.u32ExpRatioMax = u32Temp;

    /**************TRAFFIC:u32ExpRatioMin**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "TRAFFIC:u32ExpRatioMin", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("TRAFFIC:u32ExpRatioMin failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniTraffic.u32ExpRatioMin = u32Temp;

    /**************TRAFFIC:au16SharpenUd**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "TRAFFIC:au16SharpenUd");
    if (NULL == pszTempStr)
    {
        printf("TRAFFIC:au16SharpenUd error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < EXPOSURE_LEVEL; i++)
    {
        g_stINIPara.stIniTraffic.au16SharpenUd[i] = MAEWeight[i];
    }

    /**************TRAFFIC:au8SharpenD**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "TRAFFIC:au8SharpenD");
    if (NULL == pszTempStr)
    {
        printf("TRAFFIC:au8SharpenD error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < EXPOSURE_LEVEL; i++)
    {
        g_stINIPara.stIniTraffic.au8SharpenD[i] = MAEWeight[i];
    }

    /**************TRAFFIC:au8TextureThr**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "TRAFFIC:au8TextureThr");
    if (NULL == pszTempStr)
    {
        printf("TRAFFIC:au8TextureThr error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < EXPOSURE_LEVEL; i++)
    {
        g_stINIPara.stIniTraffic.au8TextureThr[i] = MAEWeight[i];
    }

    /**************TRAFFIC:au8UnderShoot**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "TRAFFIC:au8SharpenEdge");
    if (NULL == pszTempStr)
    {
        printf("TRAFFIC:au8SharpenEdge error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < EXPOSURE_LEVEL; i++)
    {
        g_stINIPara.stIniTraffic.au8SharpenEdge[i] = MAEWeight[i];
    }

    /**************TRAFFIC:au8EdgeThr**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "TRAFFIC:au8EdgeThr");
    if (NULL == pszTempStr)
    {
        printf("TRAFFIC:au8EdgeThr error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < EXPOSURE_LEVEL; i++)
    {
        g_stINIPara.stIniTraffic.au8EdgeThr[i] = MAEWeight[i];
    }

    /**************TRAFFIC:au8OverShoot**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "TRAFFIC:au8OverShoot");
    if (NULL == pszTempStr)
    {
        printf("TRAFFIC:au8OverShoot error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < EXPOSURE_LEVEL; i++)
    {
        g_stINIPara.stIniTraffic.au8OverShoot[i] = MAEWeight[i];
    }

    /**************TRAFFIC:au8UnderShoot**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "TRAFFIC:au8UnderShoot");
    if (NULL == pszTempStr)
    {
        printf("TRAFFIC:au8UnderShoot error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < EXPOSURE_LEVEL; i++)
    {
        g_stINIPara.stIniTraffic.au8UnderShoot[i] = MAEWeight[i];
    }


    /**************TRAFFIC:au8shootSupStr**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "TRAFFIC:au8shootSupStr");
    if (NULL == pszTempStr)
    {
        printf("TRAFFIC:au8shootSupStr error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < EXPOSURE_LEVEL; i++)
    {
        g_stINIPara.stIniTraffic.au8shootSupStr[i] = MAEWeight[i];
    }


    /**************TRAFFIC:au8DetailCtrl**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "TRAFFIC:au8DetailCtrl");
    if (NULL == pszTempStr)
    {
        printf("TRAFFIC:au8DetailCtrl error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < EXPOSURE_LEVEL; i++)
    {
        g_stINIPara.stIniTraffic.au8DetailCtrl[i] = MAEWeight[i];
    }

	
    /**************TRAFFIC:gamma**************/
    s32Offset = 0;
    snprintf_s(szTempStr,128, sizeof(szTempStr), "TRAFFIC:gamma_0");
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
    if (NULL == pszTempStr)
    {
        printf("TRAFFIC:gamma_0 error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < s32Temp; i++)
    {
        g_stINIPara.stIniTraffic.u16GammaTable[i] = MAEWeight[i];
    }
    s32Offset += s32Temp;

    snprintf_s(szTempStr,128, sizeof(szTempStr), "TRAFFIC:gamma_1");
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
    if (NULL == pszTempStr)
    {
        printf("TRAFFIC:gamma_1 error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < s32Temp; i++)
    {
        g_stINIPara.stIniTraffic.u16GammaTable[s32Offset + i] = MAEWeight[i];
    }
    s32Offset += s32Temp;

    snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:gamma_2");
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
    if (NULL == pszTempStr)
    {
        printf("TRAFFIC:gamma_2 error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < s32Temp; i++)
    {
        g_stINIPara.stIniTraffic.u16GammaTable[s32Offset + i] = MAEWeight[i];
    }
    /**************TRAFFIC:BoolLNTH**************/
    s32Temp = 0;
    s32Temp = iniparser_getint(g_Sceneautodictionary, "TRAFFIC:BoolLNTH", HI_FAILURE);
    if (HI_FAILURE == s32Temp)
    {
        printf("TRAFFIC:BoolLNTH failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniTraffic.stTraffic3dnr.BoolLNTH = (HI_BOOL)s32Temp;  


    /**************TRAFFIC:3DnrIsoCount**************/
    u32Temp = 0;
	u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "TRAFFIC:3DnrIsoCount", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
    	printf("TRAFFIC:3DnrIsoCount failed\n");
        return HI_FAILURE;
    }    
    g_stINIPara.stIniTraffic.stTraffic3dnr.u323DnrIsoCount= u32Temp;
	if(g_stINIPara.stIniTraffic.stTraffic3dnr.u323DnrIsoCount > 0)
	{
		g_stINIPara.stIniTraffic.stTraffic3dnr.pu323DnrIsoThresh = (HI_U32 *)malloc((g_stINIPara.stIniTraffic.stTraffic3dnr.u323DnrIsoCount) * sizeof(HI_U32));

		CHECK_NULL_PTR(g_stINIPara.stIniTraffic.stTraffic3dnr.pu323DnrIsoThresh);
		g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam = (SCENEAUTO_INIPARAM_NRS_S *)malloc((g_stINIPara.stIniTraffic.stTraffic3dnr.u323DnrIsoCount) * sizeof(SCENEAUTO_INIPARAM_NRS_S));
		CHECK_NULL_PTR(g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam);
	}

    /**************TRAFFIC:3DnrIsoThresh**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "TRAFFIC:3DnrIsoThresh");
    if (NULL == pszTempStr)
    {
        printf("TRAFFIC:3DnrIsoThresh error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniTraffic.stTraffic3dnr.u323DnrIsoCount; i++)
    {
        g_stINIPara.stIniTraffic.stTraffic3dnr.pu323DnrIsoThresh[i] = MAEWeight[i];
    }
	
	Load3DNR( &g_stINIPara.stIniTraffic.stTraffic3dnr,mode);
#if 0
    for (i = 0; i < g_stINIPara.stIniTraffic.stTraffic3dnr.u323DnrIsoCount; i++)
    {

 
        
        //TRAFFIC:s32IES0 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32IES0_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("TRAFFIC:s32IES_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32IES0 = s32Temp;  

		 //TRAFFIC:s32SBS0 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32SBS0_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("TRAFFIC:s32SBS0_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32SBS0 = s32Temp;  



	   //TRAFFIC:s32SBS1 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32SBS1_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("TRAFFIC:s32SBS1_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32SBS1 = s32Temp;  

	   //TRAFFIC:s32SBS2 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32SBS2_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("TRAFFIC:s32SBS2_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32SBS2 = s32Temp;  

	   //TRAFFIC:s32SBS3 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32SBS3_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("TRAFFIC:s32SBS3_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32SBS3 = s32Temp;  


		 //TRAFFIC:s32SDS0 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32SDS0_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("TRAFFIC:s32SDS0_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32SDS0 = s32Temp;  

		 //TRAFFIC:s32SDS1 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32SDS1_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("TRAFFIC:s32SDS1_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32SDS1 = s32Temp; 
		
		 //TRAFFIC:s32SDS2 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32SDS2_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("TRAFFIC:s32SDS2_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32SDS2 = s32Temp; 
		
		 //TRAFFIC:s32SDS3 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32SDS3_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("TRAFFIC:s32SDS3_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32SDS3 = s32Temp; 

		 //TRAFFIC:s32STH0 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32STH0_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("TRAFFIC:s32STH0_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32STH0 = s32Temp; 

		//TRAFFIC:s32STH1 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32STH1_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("TRAFFIC:s32STH1_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32STH1 = s32Temp;  

	    //TRAFFIC:s32STH2 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32STH2_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("TRAFFIC:s32STH2_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32STH2 = s32Temp; 

		//TRAFFIC:s32STH3 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32STH3_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("TRAFFIC:s32STH3_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32STH3 = s32Temp;  

	    //TRAFFIC:s32MDP 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32MDP_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("TRAFFIC:s32MDP_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32MDP = s32Temp;  


	    //TRAFFIC:s32MATH1 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32MATH1_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("TRAFFIC:s32MATH1_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32MATH1 = s32Temp;

	    //TRAFFIC:s32MATH2 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32MATH2_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("TRAFFIC:s32MATH2_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32MATH2 = s32Temp;  

	    //TRAFFIC:s32Pro3 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32Pro3_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("TRAFFIC:s32Pro3_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32Pro3 = s32Temp; 

		
		//TRAFFIC:s32MDDZ1 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32MDDZ1_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("TRAFFIC:s32MDDZ1_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32MDDZ1 = s32Temp; 

		//TRAFFIC:s32MDDZ2 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32MDDZ2_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("TRAFFIC:s32MDDZ2_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32MDDZ2 = s32Temp; 	

		//TRAFFIC:s32TFS1 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32TFS1_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("TRAFFIC:s32TFS1_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32TFS1 = s32Temp; 

		//TRAFFIC:s32TFS2 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32TFS2_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("TRAFFIC:s32TFS2_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32TFS2 = s32Temp; 


		//TRAFFIC:s32SFC
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32SFC_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
		 printf("TRAFFIC:s32SFC_%d failed\n", i);
		 return HI_FAILURE;
		}

		g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32SFC = s32Temp;

		//TRAFFIC:s32TFC
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32TFC_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
		 printf("TRAFFIC:s32TFC_%d failed\n", i);
		 return HI_FAILURE;
		}
		g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32TFC = s32Temp;

		//TRAFFIC:s32TPC
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32TPC_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
		 printf("TRAFFIC:s32TPC_%d failed\n", i);
		 return HI_FAILURE;
		}
		g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32TPC = s32Temp;

		//TRAFFIC:s32TRC  
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "TRAFFIC:s32TRC_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
		 printf("TRAFFIC:s32TRC_%d failed\n", i);
		 return HI_FAILURE;
		}
		g_stINIPara.stIniTraffic.stTraffic3dnr.pst3dnrParam[i].s32TRC = s32Temp;

         
    }
  #endif  
    
    return HI_SUCCESS;
}


HI_S32 Sceneauto_LoadIr()
{
    HI_S32 s32Temp;
	HI_U32 u32Temp;
    HI_S32 i, j;
    HI_S32 s32Offset;
    HI_CHAR szTempStr[128];
    HI_CHAR* pszTempStr;
	HI_CHAR  mode[4]= "IR";
    
    /**************IR:ExpCount**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "IR:ExpCount", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
    	printf("IR:ExpCount failed\n");
    }
    g_stINIPara.stIniIr.u32ExpCount = u32Temp;
	
	if(g_stINIPara.stIniIr.u32ExpCount > 0)
	{
	    g_stINIPara.stIniIr.pu32ExpThreshLtoH = (HI_U32*)malloc((g_stINIPara.stIniIr.u32ExpCount) * sizeof(HI_U32));
	    CHECK_NULL_PTR(g_stINIPara.stIniIr.pu32ExpThreshLtoH);
	    g_stINIPara.stIniIr.pu32ExpThreshHtoL = (HI_U32*)malloc((g_stINIPara.stIniIr.u32ExpCount) * sizeof(HI_U32));
	    CHECK_NULL_PTR(g_stINIPara.stIniIr.pu32ExpThreshHtoL);
	    g_stINIPara.stIniIr.pu8ExpCompensation = (HI_U8*)malloc((g_stINIPara.stIniIr.u32ExpCount) * sizeof(HI_U8));
	    CHECK_NULL_PTR(g_stINIPara.stIniIr.pu8ExpCompensation);
	    g_stINIPara.stIniIr.pu8MaxHistOffset = (HI_U8*)malloc((g_stINIPara.stIniIr.u32ExpCount) * sizeof(HI_U8));
	    CHECK_NULL_PTR(g_stINIPara.stIniIr.pu8MaxHistOffset);
	}


    /**************IR:ExpThreshLtoH**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "IR:ExpThreshLtoH");
    if (NULL == pszTempStr)
    {
        printf("IR:ExpThreshLtoH error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniIr.u32ExpCount; i++)
    {
        g_stINIPara.stIniIr.pu32ExpThreshLtoH[i] = MAEWeight[i];
    }

    /**************IR:ExpThreshHtoL**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "IR:ExpThreshHtoL");
    if (NULL == pszTempStr)
    {
        printf("IR:ExpThreshHtoL error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniIr.u32ExpCount; i++)
    {
        g_stINIPara.stIniIr.pu32ExpThreshHtoL[i] = MAEWeight[i];
    }

    /**************IR:ExpCompensation**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "IR:ExpCompensation");
    if (NULL == pszTempStr)
    {
        printf("IR:ExpCompensation error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniIr.u32ExpCount; i++)
    {
        g_stINIPara.stIniIr.pu8ExpCompensation[i] = MAEWeight[i];
    }

	
    /**************IR:MaxHistOffset**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "IR:MaxHistOffset");
    if (NULL == pszTempStr)
    {
        printf("IR:MaxHistOffset error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniIr.u32ExpCount; i++)
    {
        g_stINIPara.stIniIr.pu8MaxHistOffset[i] = MAEWeight[i];
    }

    /**************IR:u16HistRatioSlope**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "IR:u16HistRatioSlope", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("IR:u16HistRatioSlope failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniIr.u16HistRatioSlope = (u32Temp&0xFFFF);

    /**************IR:BlackDelayFrame**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "IR:BlackDelayFrame", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("IR:BlackDelayFrame failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniIr.u16BlackDelayFrame = (u32Temp&0xFFFF);

    /**************IR:WhiteDelayFrame**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "IR:WhiteDelayFrame", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("IR:WhiteDelayFrame failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniIr.u16WhiteDelayFrame = (u32Temp&0xFFFF);

    /**************IR:u8Tolerance**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "IR:u8Tolerance", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("IR:u8Tolerance failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniIr.u8Tolerance = (u32Temp&0xFF);

    /**************IR:u8Speed**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "IR:u8Speed", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("IR:u8Speed failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniIr.u8Speed = (u32Temp&0xFF);

    /**************IR:DCIEnable**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "IR:DCIEnable", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("IR:DCIEnable failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniIr.bDCIEnable = (HI_BOOL)u32Temp;

    /**************IR:DCIBlackGain**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "IR:DCIBlackGain", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("IR:DCIBlackGain failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniIr.u32DCIBlackGain = (HI_BOOL)u32Temp;

    /**************IR:DCIContrastGain**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "IR:DCIContrastGain", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("IR:DCIContrastGain failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniIr.u32DCIContrastGain = (HI_BOOL)u32Temp;

    /**************IR:DCILightGain**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "IR:DCILightGain", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("IR:DCILightGain failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniIr.u32DCILightGain = (HI_BOOL)u32Temp;

    
    /**************IR:bSupTwinkleEn**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "IR:bSupTwinkleEn", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("IR:bSupTwinkleEn failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniIr.bSupTwinkleEn = (HI_BOOL)u32Temp;

	    
    /**************IR:SoftThr**************/
    s32Temp = 0;
    s32Temp = iniparser_getint(g_Sceneautodictionary, "IR:SoftThr", HI_FAILURE);
    if (HI_FAILURE == s32Temp)
    {
        printf("IR:SoftThr failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniIr.SoftThr = (s32Temp&0xFF);

	/**************IR:SoftSlope**************/
    s32Temp = 0;
    s32Temp = iniparser_getint(g_Sceneautodictionary, "IR:SoftSlope", HI_FAILURE);
    if (HI_FAILURE == s32Temp)
    {
        printf("IR:SoftSlope failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniIr.SoftSlope = (s32Temp&0xFF);


    /**************IR:au16SharpenUd**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "IR:au16SharpenUd");
    if (NULL == pszTempStr)
    {
        printf("IR:au16SharpenUd error\n");
        return HI_FAILURE;
    }
    Weight(pszTempStr);
    for (i = 0; i < EXPOSURE_LEVEL; i++)
    {
        g_stINIPara.stIniIr.au16SharpenUd[i] = MAEWeight[i];
    }

    /**************IR:au8SharpenD**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "IR:au8SharpenD");
    if (NULL == pszTempStr)
    {
        printf("IR:au8SharpenD error\n");
        return HI_FAILURE;
    }
    Weight(pszTempStr);
    for (i = 0; i < EXPOSURE_LEVEL; i++)
    {
        g_stINIPara.stIniIr.au8SharpenD[i] = MAEWeight[i];
    }

    /**************IR:au8TextureSt**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "IR:au8TextureThr");
    if (NULL == pszTempStr)
    {
        printf("IR:au8TextureThr error\n");
        return HI_FAILURE;
    }
    Weight(pszTempStr);
    for (i = 0; i < EXPOSURE_LEVEL; i++)
    {
        g_stINIPara.stIniIr.au8TextureThr[i] = MAEWeight[i];
    }

    /**************IR:au8SharpenEdge**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "IR:au8SharpenEdge");
    if (NULL == pszTempStr)
    {
        printf("IR:au8SharpenEdge error\n");
        return HI_FAILURE;
    }
    Weight(pszTempStr);
    for (i = 0; i < EXPOSURE_LEVEL; i++)
    {
        g_stINIPara.stIniIr.au8SharpenEdge[i] = MAEWeight[i];
    }

    /**************IR:au8EdgeThr**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "IR:au8EdgeThr");
    if (NULL == pszTempStr)
    {
        printf("IR:au8EdgeThr error\n");
        return HI_FAILURE;
    }
    Weight(pszTempStr);
    for (i = 0; i < EXPOSURE_LEVEL; i++)
    {
        g_stINIPara.stIniIr.au8EdgeThr[i] = MAEWeight[i];
    }

    /**************IR:au8OverShoot**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "IR:au8OverShoot");
    if (NULL == pszTempStr)
    {
        printf("IR:au8OverShoot error\n");
        return HI_FAILURE;
    }
    Weight(pszTempStr);
    for (i = 0; i < EXPOSURE_LEVEL; i++)
    {
        g_stINIPara.stIniIr.au8OverShoot[i] = MAEWeight[i];
    }
    /**************IR:au8UnderShoot**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "IR:au8UnderShoot");
    if (NULL == pszTempStr)
    {
        printf("IR:au8UnderShoot error\n");
        return HI_FAILURE;
    }
    Weight(pszTempStr);
    for (i = 0; i < EXPOSURE_LEVEL; i++)
    {
        g_stINIPara.stIniIr.au8UnderShoot[i] = MAEWeight[i];
    }
    /**************IR:au8shootSupStr**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "IR:au8shootSupStr");
    if (NULL == pszTempStr)
    {
        printf("IR:au8shootSupStr error\n");
        return HI_FAILURE;
    }
    Weight(pszTempStr);
    for (i = 0; i < EXPOSURE_LEVEL; i++)
    {
        g_stINIPara.stIniIr.au8shootSupStr[i] = MAEWeight[i];
    }
    /**************IR:au8shootSupStr**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "IR:au8DetailCtrl");
    if (NULL == pszTempStr)
    {
        printf("IR:au8DetailCtrl error\n");
        return HI_FAILURE;
    }
    Weight(pszTempStr);
    for (i = 0; i < EXPOSURE_LEVEL; i++)
    {
        g_stINIPara.stIniIr.au8DetailCtrl[i] = MAEWeight[i];
    }

    /**************IR:gamma**************/
    s32Offset = 0;
    snprintf(szTempStr, sizeof(szTempStr), "IR:gamma_0");
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
    if (NULL == pszTempStr)
    {
        printf("IR:gamma_0 error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < s32Temp; i++)
    {
        g_stINIPara.stIniIr.u16GammaTable[i] = MAEWeight[i];
    }
    s32Offset += s32Temp;

    snprintf(szTempStr, sizeof(szTempStr), "IR:gamma_1");
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
    if (NULL == pszTempStr)
    {
        printf("IR:gamma_1 error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < s32Temp; i++)
    {
        g_stINIPara.stIniIr.u16GammaTable[s32Offset + i] = MAEWeight[i];
    }
    s32Offset += s32Temp;

    snprintf(szTempStr, sizeof(szTempStr), "IR:gamma_2");
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
    if (NULL == pszTempStr)
    {
        printf("IR:gamma_2 error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < s32Temp; i++)
    {
        g_stINIPara.stIniIr.u16GammaTable[s32Offset + i] = MAEWeight[i];
    }

    //IR:WEIGHT
    for(i = 0; i < AE_WEIGHT_ROW; i++)
    {
        snprintf(szTempStr, sizeof(szTempStr), "IR:expweight_%d", i);
        pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
        if (NULL == pszTempStr)
        {
            printf("IR:expweight_%d error\n", i);
            return HI_FAILURE;
        }
        s32Temp = Weight(pszTempStr);
        for (j = 0; j < AE_WEIGHT_COLUMN; j++)
        {
            g_stINIPara.stIniIr.au8Weight[i][j] = MAEWeight[j];
        }
    }

    /**************3dnr:BoolLNTH**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "IR:BoolLNTH", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("IR:BoolLNTH failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniIr.stIni3dnr.BoolLNTH = (HI_BOOL)u32Temp;  
    /**************3dnr:3DnrIsoCount**************/
    u32Temp = 0;
	u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "IR:3DnrIsoCount", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
    	printf("IR:3DnrIsoCount failed\n");
        return HI_FAILURE;
    }    

    g_stINIPara.stIniIr.stIni3dnr.u323DnrIsoCount = u32Temp;
	if(g_stINIPara.stIniIr.stIni3dnr.u323DnrIsoCount > 0)
	{
		g_stINIPara.stIniIr.stIni3dnr.pu323DnrIsoThresh = (HI_U32 *)malloc((g_stINIPara.stIniIr.stIni3dnr.u323DnrIsoCount) * sizeof(HI_U32));
		CHECK_NULL_PTR(g_stINIPara.stIniIr.stIni3dnr.pu323DnrIsoThresh);
		g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam = (SCENEAUTO_INIPARAM_NRS_S *)malloc((g_stINIPara.stIniIr.stIni3dnr.u323DnrIsoCount) * sizeof(SCENEAUTO_INIPARAM_NRS_S));
		CHECK_NULL_PTR(g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam);
	}

    /**************3dnr:3DnrIsoThresh**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "IR:3DnrIsoThresh");
    if (NULL == pszTempStr)
    {
        printf("IR:3DnrIsoThresh error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniIr.stIni3dnr.u323DnrIsoCount; i++)
    {
        g_stINIPara.stIniIr.stIni3dnr.pu323DnrIsoThresh[i] = MAEWeight[i];
    }

   Load3DNR(&g_stINIPara.stIniIr.stIni3dnr,mode);
   
#if 0
    for (i = 0; i < g_stINIPara.stIniIr.stIni3dnr.u323DnrIsoCount; i++)
    {
  
        
        //3dnr:s32IES0 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32IES0_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("IR:s32IES0_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32IES0 = s32Temp;  

		 //IR:s32SBS0 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32SBS0_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("IR:s32SBS0_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32SBS0 = s32Temp;  



	   //IR:s32SBS1 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32SBS1_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("IR:s32SBS1_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32SBS1 = s32Temp;  

	   //IR:s32SBS2 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32SBS2_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("IR:s32SBS2_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32SBS2 = s32Temp;  

	   //IR:s32SBS3 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32SBS3_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("IR:s32SBS3_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32SBS3 = s32Temp;  


		 //IR:s32SDS0 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32SDS0_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("IR:s32SDS0_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32SDS0 = s32Temp;  

		 //IR:s32SDS1 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32SDS1_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("IR:s32SDS1_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32SDS1 = s32Temp; 
		
		 //IR:s32SDS2 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32SDS2_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("IR:s32SDS2_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32SDS2 = s32Temp; 
		
		 //IR:s32SDS3 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32SDS3_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("IR:s32SDS3_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32SDS3 = s32Temp; 

		 //IR:s32STH0 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32STH0_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("IR:s32STH0_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32STH0 = s32Temp; 

		//IR:s32STH1 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32STH1_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("IR:s32STH1_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32STH1 = s32Temp;  

	    //IR:s32STH2 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32STH2_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("IR:s32STH2_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32STH2 = s32Temp; 

		//IR:s32STH3 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32STH3_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("IR:s32STH3_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32STH3 = s32Temp;  

	    //IR:s32MDP 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32MDP_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("IR:s32MDP_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32MDP = s32Temp;  

		//IR:s32MATH1 
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32MATH1_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
		  printf("IR:s32MATH1_%d failed\n", i);
		  return HI_FAILURE;
		}
		g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32MATH1 = s32Temp;

		//IR:s32MATH2 
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32MATH2_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
		  printf("IR:s32MATH2_%d failed\n", i);
		  return HI_FAILURE;
		}
		g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32MATH2 = s32Temp;  

		
		  //IR:s32Pro3 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32Pro3_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("IR:s32Pro3_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32Pro3 = s32Temp; 
		
		//IR:s32MDDZ1 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32MDDZ1_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("IR:s32MDDZ1_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32MDDZ1 = s32Temp; 

		//IR:s32MDDZ2 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32MDDZ2_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("IR:s32MDDZ2_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32MDDZ2 = s32Temp; 	


		//IR:s32TFS1 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32TFS1_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("IR:s32TFS1_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32TFS1= s32Temp; 
		
		//IR:s32TFS2 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32TFS2_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("IR:s32TFS2_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32TFS2 = s32Temp;


		//IR:s32SFC
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32SFC_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
		  printf("IR:s32SFC_%d failed\n", i);
		  return HI_FAILURE;
		}

		g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32SFC = s32Temp;


		//IR:s32TFC
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32TFC_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
		  printf("IR:s32TFC_%d failed\n", i);
		  return HI_FAILURE;
		}
		g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32TFC = s32Temp;

		//IR:s32TPC
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32TPC_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
		  printf("IR:s32TPC_%d failed\n", i);
		  return HI_FAILURE;
		}
		g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32TPC = s32Temp;

		//IR:s32TRC  
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "IR:s32TRC_%d", i);
		s32Temp = 0;
		s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
		if (HI_FAILURE == s32Temp)
		{
		  printf("IR:s32TRC_%d failed\n", i);
		  return HI_FAILURE;
		}
		g_stINIPara.stIniIr.stIni3dnr.pst3dnrParam[i].s32TRC = s32Temp;


    }	
#endif

    return HI_SUCCESS;
}


HI_S32 Sceneauto_Load3Dnr()
{
    HI_S32 s32Temp;
	HI_U32 u32Temp;
	
    HI_S32 i;
    HI_CHAR szTempStr[128];
    HI_CHAR* pszTempStr;
    
    /**************3dnr:3DnrIsoCount**************/
    s32Temp = 0;
    s32Temp = iniparser_getint(g_Sceneautodictionary, "3dnr:BoolRefMGValue", HI_FAILURE);
    if (HI_FAILURE == s32Temp)
    {
        printf("3dnr: 3dnr:BoolRefMGValue failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIni3dnr.BoolRefMGValue = (HI_BOOL)s32Temp;  

    /**************3dnr:BoolLNTH**************/
    s32Temp = 0;
    s32Temp = iniparser_getint(g_Sceneautodictionary, "3dnr:BoolLNTH", HI_FAILURE);
    if (HI_FAILURE == s32Temp)
    {
        printf("3dnr: 3dnr:BoolLNTH failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIni3dnr.BoolLNTH = (HI_BOOL)s32Temp;  
    
    /**************3dnr:3DnrIsoCount**************/
    u32Temp = 0;
	u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "3dnr:3DnrIsoCount", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
    	printf("3dnr:3DnrIsoCount failed\n");
        return HI_FAILURE;
    }    
    g_stINIPara.stIni3dnr.u323DnrIsoCount = u32Temp;
	if(g_stINIPara.stIni3dnr.u323DnrIsoCount > 0)
	{
	    g_stINIPara.stIni3dnr.pu323DnrIsoThresh = (HI_U32 *)malloc((g_stINIPara.stIni3dnr.u323DnrIsoCount) * sizeof(HI_U32));
	    CHECK_NULL_PTR(g_stINIPara.stIni3dnr.pu323DnrIsoThresh);
	    g_stINIPara.stIni3dnr.pst3dnrParam = (SCENEAUTO_INIPARAM_NRS_S *)malloc((g_stINIPara.stIni3dnr.u323DnrIsoCount) * sizeof(SCENEAUTO_INIPARAM_NRS_S));
	    CHECK_NULL_PTR(g_stINIPara.stIni3dnr.pst3dnrParam);
	}

    /**************3dnr:3DnrIsoThresh**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "3dnr:3DnrIsoThresh");
    if (NULL == pszTempStr)
    {
        printf("3dnr:3DnrIsoThresh error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIni3dnr.u323DnrIsoCount; i++)
    {
        g_stINIPara.stIni3dnr.pu323DnrIsoThresh[i] = MAEWeight[i];
    }

    for (i = 0; i < g_stINIPara.stIni3dnr.u323DnrIsoCount; i++)
    {
     
        //3dnr:s32IES0 
        snprintf_s(szTempStr,128, sizeof(szTempStr), "3dnr:s32IES0_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32IES0_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32IES0 = s32Temp;  

		 //3dnr:s32SBS0 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "3dnr:s32SBS0_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32SBS0_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32SBS0 = s32Temp;  



	   //3dnr:s32SBS1 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "3dnr:s32SBS1_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32SBS1_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32SBS1 = s32Temp;  

	   //3dnr:s32SBS2 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "3dnr:s32SBS2_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32SBS2_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32SBS2 = s32Temp;  

	   //3dnr:s32SBS3 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "3dnr:s32SBS3_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32SBS3_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32SBS3 = s32Temp;  


		 //3dnr:s32SDS0 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "3dnr:s32SDS0_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32SDS0_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32SDS0 = s32Temp;  

		 //3dnr:s32SDS1 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "3dnr:s32SDS1_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32SDS1_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32SDS1 = s32Temp; 
		
		 //3dnr:s32SDS2 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "3dnr:s32SDS2_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32SDS2_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32SDS2 = s32Temp; 
		
		 //3dnr:s32SDS3 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "3dnr:s32SDS3_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32SDS3_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32SDS3 = s32Temp; 

		 //3dnr:s32STH0 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "3dnr:s32STH0_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32STH0_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32STH0 = s32Temp; 

		//3dnr:s32STH1 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "3dnr:s32STH1_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32STH1_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32STH1 = s32Temp;  

	    //3dnr:s32STH2 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "3dnr:s32STH2_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32STH2_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32STH2 = s32Temp; 

		//3dnr:s32STH3 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "3dnr:s32STH3_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32STH3_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32STH3 = s32Temp;  


	    //3dnr:s32MDP 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "3dnr:s32MDP_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32MDP_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32MDP = s32Temp;  

	    //3dnr:s32MATH1 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "3dnr:s32MATH1_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32pm1MATH_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32MATH1 = s32Temp;

	    //3dnr:s32MATH2 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "3dnr:s32MATH2_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32MATH2_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32MATH2 = s32Temp;  

		 //3dnr:s32Pro3 
        snprintf_s(szTempStr, 128, sizeof(szTempStr), "3dnr:s32Pro3_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32Pro3_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32Pro3 = s32Temp; 	
		
		//3dnr:s32MDDZ1 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "3dnr:s32MDDZ1_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32MDDZ1_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32MDDZ1 = s32Temp; 

		//3dnr:s32MDDZ2 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "3dnr:s32MDDZ2_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32MDDZ2_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32MDDZ2 = s32Temp; 	


		
		//3dnr:s32TFS1 
        snprintf_s(szTempStr, 128, sizeof(szTempStr), "3dnr:s32TFS1_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32TFS1_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32TFS1= s32Temp; 
		
		//3dnr:s32TFS2 
        snprintf_s(szTempStr, 128, sizeof(szTempStr), "3dnr:s32TFS2_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32pbTFS2_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32TFS2 = s32Temp;

	   //3dnr:s32SFC
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "3dnr:s32SFC_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32SFC_%d failed\n", i);
            return HI_FAILURE;
        }
		
		g_stINIPara.stIni3dnr.pst3dnrParam[i].s32SFC = s32Temp;

		
        //3dnr:s32TFC
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "3dnr:s32TFC_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32TFC_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32TFC = s32Temp;
        
        //3dnr:s32TPC
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "3dnr:s32TPC_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32TPC_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32TPC = s32Temp;
        
        //3dnr:s32TRC  
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "3dnr:s32TRC_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("3dnr:s32TRC_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIni3dnr.pst3dnrParam[i].s32TRC = s32Temp;


    }
    
    return HI_SUCCESS;
}

HI_S32 Sceneauto_LoadH265Param()
{
    HI_S32 s32Temp;
    HI_S32 i, j;
    HI_CHAR szTempStr[128];
    HI_CHAR* pszTempStr;

    /**************h265venc:vencBitrateCount**************/
    s32Temp = 0;
    s32Temp = iniparser_getint(g_Sceneautodictionary, "h265venc:vencBitrateCount", HI_FAILURE);
    if (HI_FAILURE == s32Temp)
    {
        printf("h264venc:vencBitrateCount failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniH265Venc.stIniH265VencRcParam.u32BitrateCount = s32Temp;
	if(g_stINIPara.stIniH265Venc.stIniH265VencRcParam.u32BitrateCount > 0)
	{
	    g_stINIPara.stIniH265Venc.stIniH265VencRcParam.pu32BitrateThresh = (HI_U32*)malloc((g_stINIPara.stIniH265Venc.stIniH265VencRcParam.u32BitrateCount) * sizeof(HI_U32));
	    CHECK_NULL_PTR(g_stINIPara.stIniH265Venc.stIniH265VencRcParam.pu32BitrateThresh);
	    g_stINIPara.stIniH265Venc.stIniH265VencRcParam.pstH265VencRcParam = (SCENEAUTO_H265VENC_RCPARAM_S*)malloc((g_stINIPara.stIniH265Venc.stIniH265VencRcParam.u32BitrateCount) * sizeof(SCENEAUTO_H265VENC_RCPARAM_S));
	    CHECK_NULL_PTR(g_stINIPara.stIniH265Venc.stIniH265VencRcParam.pstH265VencRcParam);
	}

    /**************h265venc:vencBitrateThresh**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "h265venc:vencBitrateThresh");
    if (NULL == pszTempStr)
    {
        printf("h265venc:vencBitrateThresh error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniH265Venc.stIniH265VencRcParam.u32BitrateCount; i++)
    {
        g_stINIPara.stIniH265Venc.stIniH265VencRcParam.pu32BitrateThresh[i] = MAEWeight[i];
    }

    for (i = 0; i < g_stINIPara.stIniH265Venc.stIniH265VencRcParam.u32BitrateCount; i++)
    {
        /**************h265venc:u32RowQpDelta**************/
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "h265venc:u32RowQpDelta_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("h265venc:u32RowQpDelta_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniH265Venc.stIniH265VencRcParam.pstH265VencRcParam[i].u32DeltaQP = s32Temp;

        /**************h265venc:s32IPQPDelta**************/
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "h265venc:s32IPQPDelta_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("h265venc:s32IPQPDelta_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniH265Venc.stIniH265VencRcParam.pstH265VencRcParam[i].s32IPQPDelta = s32Temp;
     
        /**************h265venc:ThreshI**************/
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "h265venc:ThreshI_%d", i);
        pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
        if (NULL == pszTempStr)
        {
            printf("h265venc:ThreshI_%d error\n", i);
            return HI_FAILURE;
        }
        s32Temp = Weight(pszTempStr);
        for (j = 0; j < 12; j++)
        {
            g_stINIPara.stIniH265Venc.stIniH265VencRcParam.pstH265VencRcParam[i].u32ThrdI[j] = MAEWeight[j];
        }

        /**************h265venc:ThreshP**************/
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "h265venc:ThreshP_%d", i);
        pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
        if (NULL == pszTempStr)
        {
            printf("h265venc:ThreshP_%d error\n", i);
            return HI_FAILURE;
        }
        s32Temp = Weight(pszTempStr);
        for (j = 0; j < 12; j++)
        {
            g_stINIPara.stIniH265Venc.stIniH265VencRcParam.pstH265VencRcParam[i].u32ThrdP[j] = MAEWeight[j];
        }
    }
    
    return HI_SUCCESS;
}

HI_S32 Sceneauto_LoadH264Param()
{
    HI_S32 s32Temp;
	HI_U32 u32Temp;
    HI_S32 i, j;
    HI_CHAR szTempStr[128];
    HI_CHAR* pszTempStr;
    
    /**************h264venc:vencBitrateCount**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "h264venc:vencBitrateCount", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("h264venc:vencBitrateCount failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniH264Venc.u32BitrateCount = u32Temp;
	
	if(g_stINIPara.stIniH264Venc.u32BitrateCount > 0)
	{
	    g_stINIPara.stIniH264Venc.pu32BitrateThresh = (HI_U32*)malloc((g_stINIPara.stIniH264Venc.u32BitrateCount) * sizeof(HI_U32));
	    CHECK_NULL_PTR(g_stINIPara.stIniH264Venc.pu32BitrateThresh);
	    g_stINIPara.stIniH264Venc.pstH264Venc = (SCENEAUTO_H264VENC_S*)malloc((g_stINIPara.stIniH264Venc.u32BitrateCount) * sizeof(SCENEAUTO_H264VENC_S));
	    CHECK_NULL_PTR(g_stINIPara.stIniH264Venc.pstH264Venc);
	}

    /**************h264venc:vencBitrateThresh**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "h264venc:vencBitrateThresh");
    if (NULL == pszTempStr)
    {
        printf("h264venc:vencBitrateThresh error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniH264Venc.u32BitrateCount; i++)
    {
        g_stINIPara.stIniH264Venc.pu32BitrateThresh[i] = MAEWeight[i];
    }

    for (i = 0; i < g_stINIPara.stIniH264Venc.u32BitrateCount; i++)
    {
        /**************h264venc:chroma_qp_index_offset**************/
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "h264venc:chroma_qp_index_offset_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("h264venc:chroma_qp_index_offset_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniH264Venc.pstH264Venc[i].s32chroma_qp_index_offset = s32Temp;

        /**************h264venc:disable_deblocking_filter_idc**************/
        snprintf_s(szTempStr,128, sizeof(szTempStr), "h264venc:disable_deblocking_filter_idc_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("h264venc:disable_deblocking_filter_idc_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniH264Venc.pstH264Venc[i].stH264Dblk.disable_deblocking_filter_idc = s32Temp;

        /**************h264venc:slice_alpha_c0_offset_div2**************/
        snprintf_s(szTempStr,128, sizeof(szTempStr), "h264venc:slice_alpha_c0_offset_div2_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("h264venc:slice_alpha_c0_offset_div2_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniH264Venc.pstH264Venc[i].stH264Dblk.slice_alpha_c0_offset_div2 = s32Temp;

        /**************h264venc:slice_beta_offset_div2**************/
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "h264venc:slice_beta_offset_div2_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("h264venc:slice_beta_offset_div2_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniH264Venc.pstH264Venc[i].stH264Dblk.slice_beta_offset_div2 = s32Temp;

        /**************h264venc:u32DeltaQP**************/
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "h264venc:u32DeltaQP_%d", i);
        u32Temp = 0;
        u32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == u32Temp)
        {
            printf("h264venc:u32DeltaQP_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniH264Venc.pstH264Venc[i].u32DeltaQP = u32Temp;

        /**************h264venc:s32IPQPDelta**************/
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "h264venc:s32IPQPDelta_%d", i);
        s32Temp = 0;
        s32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("h264venc:s32IPQPDelta_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniH264Venc.pstH264Venc[i].s32IPQPDelta = s32Temp;

        /**************h264venc:ThreshI**************/
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "h264venc:ThreshI_%d", i);
        pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
        if (NULL == pszTempStr)
        {
            printf("h264venc:ThreshI_%d error\n", i);
            return HI_FAILURE;
        }
        s32Temp = Weight(pszTempStr);
        for (j = 0; j < 12; j++)
        {
            g_stINIPara.stIniH264Venc.pstH264Venc[i].u32ThrdI[j] = MAEWeight[j];
        }

        /**************h264venc:ThreshP**************/
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "h264venc:ThreshP_%d", i);
        pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
        if (NULL == pszTempStr)
        {
            printf("h264venc:ThreshP_%d error\n", i);
            return HI_FAILURE;
        }
        s32Temp = Weight(pszTempStr);
        for (j = 0; j < 12; j++)
        {
            g_stINIPara.stIniH264Venc.pstH264Venc[i].u32ThrdP[j] = MAEWeight[j];
        }

    }

    
    return HI_SUCCESS;
}

HI_S32 Sceneauto_LoadGamma()
{
    HI_S32 s32Temp;
    HI_S32 s32Offset;
    HI_S32 i, j;
    HI_CHAR szTempStr[128];
    HI_CHAR* pszTempStr;
    
    /**************gamma:DelayCount**************/
    s32Temp = 0;
    s32Temp = iniparser_getint(g_Sceneautodictionary, "gamma:DelayCount", HI_FAILURE);
    if (HI_FAILURE == s32Temp)
    {
        printf("gamma:DelayCount failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniGamma.u32DelayCount = s32Temp;

    /**************gamma:Interval**************/
    s32Temp = 0;
    s32Temp = iniparser_getint(g_Sceneautodictionary, "gamma:Interval", HI_FAILURE);
    if (HI_FAILURE == s32Temp)
    {
        printf("gamma:Interval failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniGamma.s32Interval = s32Temp;

    /**************gamma:ExpCount**************/
    s32Temp = 0;
    s32Temp = iniparser_getint(g_Sceneautodictionary, "gamma:ExpCount", HI_FAILURE);
    if (HI_FAILURE == s32Temp)
    {
        printf("gamma:ExpCount failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniGamma.u32ExpCount = s32Temp;
	if(g_stINIPara.stIniGamma.u32ExpCount > 0)
	{
	    g_stINIPara.stIniGamma.pu32ExpThreshLtoH = (HI_U32*)malloc((g_stINIPara.stIniGamma.u32ExpCount) * sizeof(HI_U32));
	    CHECK_NULL_PTR(g_stINIPara.stIniGamma.pu32ExpThreshLtoH);
	    g_stINIPara.stIniGamma.pu32ExpThreshHtoL = (HI_U32*)malloc((g_stINIPara.stIniGamma.u32ExpCount) * sizeof(HI_U32));
	    CHECK_NULL_PTR(g_stINIPara.stIniGamma.pu32ExpThreshHtoL);
	    g_stINIPara.stIniGamma.pstGamma = (SCENEAUTO_GAMMA_S*)malloc((g_stINIPara.stIniGamma.u32ExpCount) * sizeof(SCENEAUTO_GAMMA_S));
	    CHECK_NULL_PTR(g_stINIPara.stIniGamma.pstGamma);
	}
    /**************gamma:ExpThreshLtoD**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "gamma:ExpThreshLtoD");
    if (NULL == pszTempStr)
    {
        printf("gamma:ExpThreshLtoD error\n");
        return HI_FAILURE;
    }
    Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniGamma.u32ExpCount; i++)
    {
        g_stINIPara.stIniGamma.pu32ExpThreshLtoH[i] = MAEWeight[i];
    }

    /**************gamma:ExpThreshDtoL**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "gamma:ExpThreshDtoL");
    if (NULL == pszTempStr)
    {
        printf("gamma:ExpThreshDtoL error\n");
        return HI_FAILURE;
    }
    Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniGamma.u32ExpCount; i++)
    {
        g_stINIPara.stIniGamma.pu32ExpThreshHtoL[i] = MAEWeight[i];
    }

    /**************gamma:gammatable**************/
    for (i = 0; i < g_stINIPara.stIniGamma.u32ExpCount; i++)
    {
        s32Offset = 0;

        snprintf_s(szTempStr, 128,sizeof(szTempStr), "gamma:gamma.0_%d", i);
        pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
        if (NULL == pszTempStr)
        {
            printf("gamma:gamma.0_%d error\n", i);
            return HI_FAILURE;
        }
        s32Temp = Weight(pszTempStr);
        for (j = 0; j < s32Temp; j++)
        {
            g_stINIPara.stIniGamma.pstGamma[i].u16Table[j] = MAEWeight[j];
        }
        s32Offset += s32Temp;

        snprintf_s(szTempStr, 128,sizeof(szTempStr), "gamma:gamma.1_%d", i);
        pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
        if (NULL == pszTempStr)
        {
            printf("gamma:gamma.1_%d error\n", i);
            return HI_FAILURE;
        }
        s32Temp = Weight(pszTempStr);
        for (j = 0; j < s32Temp; j++)
        {
            g_stINIPara.stIniGamma.pstGamma[i].u16Table[s32Offset + j] = MAEWeight[j];
        }
        s32Offset += s32Temp;

        snprintf_s(szTempStr, 128,sizeof(szTempStr), "gamma:gamma.2_%d", i);
        pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
        if (NULL == pszTempStr)
        {
            printf("gamma:gamma.2_%d error\n", i);
            return HI_FAILURE;
        }
        s32Temp = Weight(pszTempStr);
        for (j = 0; j < s32Temp; j++)
        {
            g_stINIPara.stIniGamma.pstGamma[i].u16Table[s32Offset + j] = MAEWeight[j];
        }

        g_stINIPara.stIniGamma.pstGamma[i].u8CurveType = 2;
    }
    
    return HI_SUCCESS;
}

HI_S32 Sceneauto_LoadDp()
{
	HI_U32 u32Temp;
    HI_S32 i;
    HI_CHAR* pszTempStr;
    
    /**************dp:ExpCount**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "dp:ExpCount", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("dp:ExpCount failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniDP.u32ExpCount = u32Temp;
	if(g_stINIPara.stIniDP.u32ExpCount > 0)
	{
	    g_stINIPara.stIniDP.pu32ExpThresh = (HI_U32*)malloc((g_stINIPara.stIniDP.u32ExpCount) * sizeof(HI_U32));
	    CHECK_NULL_PTR(g_stINIPara.stIniDP.pu32ExpThresh);
	    g_stINIPara.stIniDP.pstDPAttr = (SCENEAUTO_DEPATTR_S*)malloc((g_stINIPara.stIniDP.u32ExpCount) * sizeof(SCENEAUTO_DEPATTR_S));
	    CHECK_NULL_PTR(g_stINIPara.stIniDP.pstDPAttr);
	}

    /**************dp:ExpThresh**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "dp:ExpThresh");
    if (NULL == pszTempStr)
    {
        printf("dp:ExpThresh error\n");
        return HI_FAILURE;
    }
    Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniDP.u32ExpCount; i++)
    {
        g_stINIPara.stIniDP.pu32ExpThresh[i] = MAEWeight[i];
    }

    /**************dp:bSupTwinkleEn**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "dp:bSupTwinkleEn");
    if (NULL == pszTempStr)
    {
        printf("dp:bSupTwinkleEn error\n");
        return HI_FAILURE;
    }
    Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniDP.u32ExpCount; i++)
    {
        g_stINIPara.stIniDP.pstDPAttr[i].bSupTwinkleEn = (HI_BOOL)MAEWeight[i];
    }
    /**************dp:SoftThr****************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "dp:SoftThr");
    if(NULL == pszTempStr)
    {
        printf("dp:SoftThr error\n");
        return HI_FAILURE;
    }
    Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniDP.u32ExpCount; i++)
    {
        g_stINIPara.stIniDP.pstDPAttr[i].SoftThr = MAEWeight[i];
    }
    /**************dp:SoftSlope****************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "dp:SoftSlope");
    if(NULL == pszTempStr)
    {
        printf("dp:SoftSlope error\n");
        return HI_FAILURE;
    }
    Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniDP.u32ExpCount; i++)
    {
        g_stINIPara.stIniDP.pstDPAttr[i].SoftSlope = MAEWeight[i];
    }
    return HI_SUCCESS;
}
HI_S32 Sceneauto_LoadDrc()
{
	HI_S32 s32Temp;
	HI_U32 u32Temp;
    HI_S32 i;
    HI_CHAR* pszTempStr;
    /**************drc:ExpCount**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "drc:ExpCount", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("drc:ExpCount failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniDrc.u32ExpCount = u32Temp;
	if(g_stINIPara.stIniDrc.u32ExpCount > 0)
	{
	    g_stINIPara.stIniDrc.pu32ExpThreshLtoD = (HI_U32*)malloc((g_stINIPara.stIniDrc.u32ExpCount) * sizeof(HI_U32));
	    CHECK_NULL_PTR(g_stINIPara.stIniDrc.pu32ExpThreshLtoD);
		g_stINIPara.stIniDrc.pu32ExpThreshDtoL = (HI_U32*)malloc((g_stINIPara.stIniDrc.u32ExpCount) * sizeof(HI_U32));
	    CHECK_NULL_PTR(g_stINIPara.stIniDrc.pu32ExpThreshDtoL);
	    g_stINIPara.stIniDrc.pstDrcAttr = (SCENEAUTO_INIPARAM_DRC_S*)malloc((g_stINIPara.stIniDrc.u32ExpCount) * sizeof(SCENEAUTO_INIPARAM_DRC_S));
	    CHECK_NULL_PTR(g_stINIPara.stIniDrc.pstDrcAttr);
	}

    /**************drc:ExpThreshLtoD**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "drc:ExpThreshLtoD");
    if (NULL == pszTempStr)
    {
        printf("drc:ExpThreshLtoD error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniDrc.u32ExpCount; i++)
    {
        g_stINIPara.stIniDrc.pu32ExpThreshLtoD[i] = MAEWeight[i];
    }

    /**************drc:ExpThreshDtoL**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "drc:ExpThreshDtoL");
    if (NULL == pszTempStr)
    {
        printf("drc:ExpThreshDtoL error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniDrc.u32ExpCount; i++)
    {
        g_stINIPara.stIniDrc.pu32ExpThreshDtoL[i] = MAEWeight[i];
    }

	
    //drc:enOpType
    u32Temp = 0;
    u32Temp = iniparser_getint(g_Sceneautodictionary, "drc:enOpType", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("drc:enOpType failed\n");
        return HI_FAILURE;
    }
    //g_stINIPara.stIniDrc.pstDrcAttr.enOpType = (HI_BOOL)s32Temp;
    g_stINIPara.stIniDrc.enOpType = (HI_BOOL)u32Temp;

	//drc:Interval
    s32Temp = 0;
    s32Temp = iniparser_getint(g_Sceneautodictionary, "drc:Interval", HI_FAILURE);
    if (HI_FAILURE == s32Temp)
    {
        printf("drc:Interval failed\n");
        return HI_FAILURE;
    }
	g_stINIPara.stIniDrc.s32Interval = s32Temp;
	/*
	//drc:stManulalStr
	s32Temp = 0;
    s32Temp = iniparser_getint(g_Sceneautodictionary, "drc:stManulalStr", HI_FAILURE);
    if (HI_FAILURE == s32Temp)
    {
        printf("drc:stManulalStr failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniDrc.pstDrcAttr.stManulalStr = s32Temp;*/

	/**************drc:stManulalStr**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "drc:stManulalStr");
    if (NULL == pszTempStr)
    {
        printf("drc:stManulalStr failed\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniDrc.u32ExpCount; i++)
    {
        g_stINIPara.stIniDrc.pstDrcAttr[i].stManulalStr = MAEWeight[i];
    }
   
    return HI_SUCCESS;
}

HI_S32 Sceneauto_LoadDci()
{
	
	HI_U32 u32Temp;
    
    /**************dci:bEnable**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "dci:DCIEnable", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("dci:bEnable failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniDci.bDCIEnable = (HI_BOOL)u32Temp;
	/**************dci:DCIBlackGain**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "dci:DCIBlackGain", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("dci:DCIBlackGain failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniDci.u32DCIBlackGain = u32Temp;
		/**************dci:DCIContrastGain**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "dci:DCIContrastGain", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("dci:DCIContrastGain failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniDci.u32DCIContrastGain = u32Temp;

	/**************dci:DCILightGain**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "dci:DCILightGain", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("dci:DCILightGain failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniDci.u32DCILightGain = u32Temp;
   
    return HI_SUCCESS;
}
HI_S32 Sceneauto_Load2DNR()
{
	
	HI_U32 u32Temp;
	/**************2dnr:WDRSFCoarseStr**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "2dnr:WDRSFCoarseStr", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("2dnr:WDRSFCoarseStr failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIni2dnr.au8WDRCoarseStr[0] = (u32Temp&0xFF);
		/**************2dnr:WDRLFCoarseStr**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "2dnr:WDRLFCoarseStr", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("2dnr:WDRLFCoarseStr failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIni2dnr.au8WDRCoarseStr[1] = (u32Temp&0xFF);
    return HI_SUCCESS;
}
HI_S32 Sceneauto_LoadDefog()
{
	
	HI_U32 u32Temp;
    
    /**************defog:bEnable**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "defog:bEnable", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("defog:bEnable failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniDefog.bEnable = (HI_BOOL)u32Temp;
	//printf("defog = %d\n",g_stINIPara.stIniDefog.bEnable);
   
    /**************defog:OpType**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "defog:OpType", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("defog:OpType failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniDefog.enOpType = u32Temp;
	   
    /**************defog:ManualStrength**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "defog:ManualStrength", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("defog:ManualStrength failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniDefog.ManualStrength = (u32Temp&0xFF);
   
    return HI_SUCCESS;
}

HI_S32 Sceneauto_LoadSharpen()
{
	HI_U32 u32Temp;
    HI_S32 i, j, k;
    HI_CHAR szTempStr[128];
    HI_CHAR* pszTempStr;
    
    /**************sharpen:BitrateCount**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "sharpen:BitrateCount", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("sharpen:BitrateCount failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniSharpen.u32BitrateCount = u32Temp;
    if(g_stINIPara.stIniSharpen.u32BitrateCount > 0)
    {
	    g_stINIPara.stIniSharpen.pu32BitrateThresh = (HI_U32*)malloc((g_stINIPara.stIniSharpen.u32BitrateCount) * sizeof(HI_U32));
	    CHECK_NULL_PTR(g_stINIPara.stIniSharpen.pu32BitrateThresh);
    }

    /**************sharpen:BitrateThresh**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "sharpen:BitrateThresh");
    if (NULL == pszTempStr)
    {
        printf("sharpen:BitrateThresh error\n");
        return HI_FAILURE;
    }
    Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniSharpen.u32BitrateCount; i++)
    {
        g_stINIPara.stIniSharpen.pu32BitrateThresh[i] = MAEWeight[i];
    }

    /**************sharpen:ExpCount**************/
    u32Temp = 0;
    u32Temp = iniparser_getint(g_Sceneautodictionary, "sharpen:ExpCount", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("sharpen:ExpCount failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniSharpen.u32ExpCount = u32Temp;
    if(g_stINIPara.stIniSharpen.u32ExpCount > 0)
    {
		g_stINIPara.stIniSharpen.pu32ExpThresh = (HI_U32*)malloc((g_stINIPara.stIniSharpen.u32ExpCount) * sizeof(HI_U32));

		CHECK_NULL_PTR(g_stINIPara.stIniSharpen.pu32ExpThresh);
     }
    /**************sharpen:ExpThresh**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "sharpen:ExpThresh");
    if (NULL == pszTempStr)
    {
        printf("sharpen:ExpThresh failed\n");
        return HI_FAILURE;
    }
    Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniSharpen.u32ExpCount; i++)
    {
        g_stINIPara.stIniSharpen.pu32ExpThresh[i] = MAEWeight[i];
    }
	
    if((g_stINIPara.stIniSharpen.u32BitrateCount * g_stINIPara.stIniSharpen.u32ExpCount) > 0)
    {
	    g_stINIPara.stIniSharpen.pstSharpen = (SCENEAUTO_SHARPEN_S*)malloc((g_stINIPara.stIniSharpen.u32BitrateCount) * (g_stINIPara.stIniSharpen.u32ExpCount) * sizeof(SCENEAUTO_SHARPEN_S));
    CHECK_NULL_PTR(g_stINIPara.stIniSharpen.pstSharpen);
    }
    for (i = 0; i < g_stINIPara.stIniSharpen.u32BitrateCount; i++)
    {
        for (j = 0; j < g_stINIPara.stIniSharpen.u32ExpCount; j++)
        {
            /**************sharpen:au16SharpenUd**************/   
            snprintf_s(szTempStr, 128,sizeof(szTempStr), "sharpen:au16SharpenUd_%d_%d", i, j);
            pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
            if (NULL == pszTempStr)
            {
                printf("sharpen:au16SharpenUd_%d_%d failed\n", i, j);
                return HI_FAILURE;
            } 
            Weight(pszTempStr);
            for (k = 0; k < EXPOSURE_LEVEL; k++)
            {
                g_stINIPara.stIniSharpen.pstSharpen[i * g_stINIPara.stIniSharpen.u32ExpCount + j].au16SharpenUd[k] = MAEWeight[k];
            }

        /**************sharpen:au8SharpenD**************/
            snprintf_s(szTempStr, 128,sizeof(szTempStr), "sharpen:au8SharpenD_%d_%d", i, j);
        pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
        if (NULL == pszTempStr)
        {
                printf("sharpen:au8SharpenD_%d_%d failed\n", i, j);
            return HI_FAILURE;
        }
        Weight(pszTempStr);
            for (k = 0; k < EXPOSURE_LEVEL; k++)
        {
                g_stINIPara.stIniSharpen.pstSharpen[i * g_stINIPara.stIniSharpen.u32ExpCount + j].au8SharpenD[k] = MAEWeight[k];
        }

            /**************sharpen:au8TextureThr**************/   
            snprintf_s(szTempStr, 128,sizeof(szTempStr), "sharpen:au8TextureThr_%d_%d", i, j);
            pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
            if (NULL == pszTempStr)
            {
                printf("sharpen:au8TextureThr_%d_%d failed\n", i, j);
                return HI_FAILURE;
            } 
            Weight(pszTempStr);
            for (k = 0; k < EXPOSURE_LEVEL; k++)
            {
                g_stINIPara.stIniSharpen.pstSharpen[i * g_stINIPara.stIniSharpen.u32ExpCount + j].au8TextureThr[k] = MAEWeight[k];
            }

            /**************sharpen:au8SharpenEdge**************/   
            snprintf_s(szTempStr, 128, sizeof(szTempStr), "sharpen:au8SharpenEdge_%d_%d", i, j);
        pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
        if (NULL == pszTempStr)
        {
                printf("sharpen:au8SharpenEdge_%d_%d failed\n", i, j);
                return HI_FAILURE;
            } 
            Weight(pszTempStr);
            for (k = 0; k < EXPOSURE_LEVEL; k++)
            {
                g_stINIPara.stIniSharpen.pstSharpen[i * g_stINIPara.stIniSharpen.u32ExpCount + j].au8SharpenEdge[k] = MAEWeight[k];
            }
        


          /**************sharpen:au8EdgeThr**************/   
            snprintf_s(szTempStr, 128,sizeof(szTempStr), "sharpen:au8EdgeThr_%d_%d", i, j);
        pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
        if (NULL == pszTempStr)
        {
                printf("sharpen:au8EdgeThr_%d_%d failed\n", i, j);
                return HI_FAILURE;
            } 
            Weight(pszTempStr);
            for (k = 0; k < EXPOSURE_LEVEL; k++)
            {
                g_stINIPara.stIniSharpen.pstSharpen[i * g_stINIPara.stIniSharpen.u32ExpCount + j].au8EdgeThr[k] = MAEWeight[k];
            }
        
		/**************sharpen:au8OverShoot**************/   
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "sharpen:au8OverShoot_%d_%d", i, j);
	pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
	if (NULL == pszTempStr)
	{
			printf("sharpen:au8OverShoot_%d_%d failed\n", i, j);
			return HI_FAILURE;
		} 
		Weight(pszTempStr);
		for (k = 0; k < EXPOSURE_LEVEL; k++)
		{
			g_stINIPara.stIniSharpen.pstSharpen[i * g_stINIPara.stIniSharpen.u32ExpCount + j].au8OverShoot[k] = MAEWeight[k];
		}
	
	/**************sharpen:au8UnderShoot**************/   
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "sharpen:au8UnderShoot_%d_%d", i, j);
	pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
	if (NULL == pszTempStr)
	{
			printf("sharpen:au8UnderShoot_%d_%d failed\n", i, j);
			return HI_FAILURE;
		} 
		Weight(pszTempStr);
		for (k = 0; k < EXPOSURE_LEVEL; k++)
		{
			g_stINIPara.stIniSharpen.pstSharpen[i * g_stINIPara.stIniSharpen.u32ExpCount + j].au8UnderShoot[k] = MAEWeight[k];
		}
	
		/**************sharpen:au8shootSupStr**************/   
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "sharpen:au8shootSupStr_%d_%d", i, j);
	pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
	if (NULL == pszTempStr)
	{
			printf("sharpen:au8shootSupStr_%d_%d failed\n", i, j);
			return HI_FAILURE;
		} 
		Weight(pszTempStr);
		for (k = 0; k < EXPOSURE_LEVEL; k++)
		{
			g_stINIPara.stIniSharpen.pstSharpen[i * g_stINIPara.stIniSharpen.u32ExpCount + j].au8shootSupStr[k] = MAEWeight[k];
		}


		/**************sharpen:au8DetailCtrl**************/   
		snprintf_s(szTempStr, 128,sizeof(szTempStr), "sharpen:au8DetailCtrl_%d_%d", i, j);
		pszTempStr = iniparser_getstr(g_Sceneautodictionary, szTempStr);
		if (NULL == pszTempStr)
		{
			printf("sharpen:au8DetailCtrl_%d_%d failed\n", i, j);
			return HI_FAILURE;
		} 
		Weight(pszTempStr);
		for (k = 0; k < EXPOSURE_LEVEL; k++)
		{
			g_stINIPara.stIniSharpen.pstSharpen[i * g_stINIPara.stIniSharpen.u32ExpCount + j].au8DetailCtrl[k] = MAEWeight[k];
		}
		
	

		
}
    	}
		return HI_SUCCESS;
}
	

HI_S32 Sceneauto_LoadAE()
{
    HI_S32 s32Temp;
	HI_U32 u32Temp;
    HI_S32 i;
    HI_CHAR szTempStr[128];
    HI_CHAR* pszTempStr;

    /**************AE:aeRunInterval**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "AE:aeRunInterval", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("AE:aeRunInterval failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniAE.u8AERunInterval = (u32Temp&0xFF);

    /**************AE:aeBitrateCount**************/ 
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "AE:aeBitrateCount", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("AE:aeBitrateCount failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniAE.u32BitrateCount = u32Temp;  
	if(g_stINIPara.stIniAE.u32BitrateCount > 0)
	{
	    g_stINIPara.stIniAE.pu32BitrateThresh = (HI_U32*)malloc((g_stINIPara.stIniAE.u32BitrateCount) * sizeof(HI_U32));
	    CHECK_NULL_PTR(g_stINIPara.stIniAE.pu32BitrateThresh);
	    g_stINIPara.stIniAE.pstAERelatedBit = (SCENEAUTO_AERELATEDBIT_S*)malloc((g_stINIPara.stIniAE.u32BitrateCount) * sizeof(SCENEAUTO_AERELATEDBIT_S));
	    CHECK_NULL_PTR(g_stINIPara.stIniAE.pstAERelatedBit);
	}

    /**************AE:aeBitrateThresh**************/ 
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "AE:aeBitrateThresh");
    if (NULL == pszTempStr)
    {
        printf("AE:aeBitrateThresh error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniAE.u32BitrateCount; i++)
    {
        g_stINIPara.stIniAE.pu32BitrateThresh[i] = MAEWeight[i];
    }

    for (i = 0; i < g_stINIPara.stIniAE.u32BitrateCount; i++)
    {
        /**************AE:u8Speed**************/ 
        snprintf_s(szTempStr,128, sizeof(szTempStr), "AE:u8Speed_%d", i);
        u32Temp = 0;
        u32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == u32Temp)
        {
            printf("AE:u8Speed_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniAE.pstAERelatedBit[i].u8Speed = (u32Temp&0xFF);

        /**************AE:u8Tolerance**************/ 
        snprintf_s(szTempStr,128, sizeof(szTempStr), "AE:u8Tolerance_%d", i);
        u32Temp = 0;
        u32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == u32Temp)
        {
            printf("AE:u8Tolerance_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniAE.pstAERelatedBit[i].u8Tolerance = (u32Temp&0xFF);

        /**************AE:u16BlackDelayFrame**************/ 
        snprintf_s(szTempStr,128, sizeof(szTempStr), "AE:u16BlackDelayFrame_%d", i);
        u32Temp = 0;
        u32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == u32Temp)
        {
            printf("AE:u16BlackDelayFrame_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniAE.pstAERelatedBit[i].u16BlackDelayFrame = (u32Temp&0xFFFF);

        /**************AE:u16WhiteDelayFrame**************/ 
        snprintf_s(szTempStr, 128,sizeof(szTempStr), "AE:u16WhiteDelayFrame_%d", i);
        u32Temp = 0;
        u32Temp = iniparser_getint(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == u32Temp)
        {
            printf("AE:u16WhiteDelayFrame_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniAE.pstAERelatedBit[i].u16WhiteDelayFrame = (u32Temp&0xFFFF);
        snprintf_s(szTempStr,128, sizeof(szTempStr), "AE:u32SysGainMax_%d", i);
        u32Temp = 0;
        u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, szTempStr, HI_FAILURE);
        if (HI_FAILURE == s32Temp)
        {
            printf("AE:u32SysGainMax_%d failed\n", i);
            return HI_FAILURE;
        }
        g_stINIPara.stIniAE.pstAERelatedBit[i].u32SysGainMax = u32Temp;
    }

    /**************AE:aeExpCount**************/ 
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "AE:aeExpCount", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("AE:aeExpCount failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniAE.u32ExpCount = u32Temp;
	if(g_stINIPara.stIniAE.u32ExpCount > 0)
	{
	    g_stINIPara.stIniAE.pu32AEExpHtoLThresh = (HI_U32*)malloc((g_stINIPara.stIniAE.u32ExpCount) * sizeof(HI_U32));
	    CHECK_NULL_PTR(g_stINIPara.stIniAE.pu32AEExpHtoLThresh);
	    g_stINIPara.stIniAE.pu32AEExpLtoHThresh = (HI_U32*)malloc((g_stINIPara.stIniAE.u32ExpCount) * sizeof(HI_U32));
	    CHECK_NULL_PTR(g_stINIPara.stIniAE.pu32AEExpLtoHThresh);
	    g_stINIPara.stIniAE.pstAERelatedExp = (SCENEAUTO_AERELATEDEXP_S*)malloc((g_stINIPara.stIniAE.u32ExpCount) * sizeof(SCENEAUTO_AERELATEDEXP_S));
	    CHECK_NULL_PTR(g_stINIPara.stIniAE.pstAERelatedExp );
	}

    /**************AE:aeExpDtoLThresh**************/ 
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "AE:aeExpDtoLThresh");
    if (NULL == pszTempStr)
    {
        printf("AE:aeExpDtoLThresh error\n");
        return HI_FAILURE;
    }
    Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniAE.u32ExpCount; i++)
    {
        g_stINIPara.stIniAE.pu32AEExpHtoLThresh[i] = MAEWeight[i];
    }

    /**************AE:aeExpLtoDThresh**************/ 
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "AE:aeExpLtoDThresh");
    if (NULL == pszTempStr)
    {
        printf("AE:aeExpLtoDThresh error\n");
        return HI_FAILURE;
    }
    Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniAE.u32ExpCount; i++)
    {
        g_stINIPara.stIniAE.pu32AEExpLtoHThresh[i] = MAEWeight[i];
    }

    /**************AE:aeCompesation**************/ 
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "AE:aeCompesation");
    if (NULL == pszTempStr)
    {
        printf("AE:aeCompesation error\n");
        return HI_FAILURE;
    }
    Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniAE.u32ExpCount; i++)
    {
        g_stINIPara.stIniAE.pstAERelatedExp[i].u8AECompesation = MAEWeight[i];
    }

    /**************AE:aeHistOffset**************/ 
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "AE:aeHistOffset");
    if (NULL == pszTempStr)
    {
        printf("AE:aeHistOffset error\n");
        return HI_FAILURE;
    }
    Weight(pszTempStr);
    for (i = 0; i < g_stINIPara.stIniAE.u32ExpCount; i++)
    {
        g_stINIPara.stIniAE.pstAERelatedExp[i].u8AEHistOffset = MAEWeight[i];
    }

	/**************AE:expRatioMax**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "AE:expRatioMax", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("AE:expRatioMax failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniAE.u32ExpRatioMax= u32Temp;

	/**************AE:expRatioMin**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "AE:expRatioMin", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("AE:expRatioMin failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniAE.u32ExpRatioMin= u32Temp;

	/**************AE:ratioBias**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "AE:ratioBias", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("AE:ratioBias failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniAE.u16RatioBias= (u32Temp&0xFFFF);

    /**************AE:u8ExpRatioType**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "AE:u8ExpRatioType", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("AE:u8ExpRatioType failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniAE.u8ExpRatioType = (u32Temp&0xFF);

    /**************AE:u8ExpRatioType**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "AE:u32ExpRatio", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("AE:u32ExpRatio failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stIniAE.u32ExpRatio = u32Temp;
   
    return HI_SUCCESS;
}

HI_S32 Sceneauto_LoadAWB()
{
	HI_U32 u32Temp;
	HI_S32 i;
	HI_CHAR* pszTempStr;

	g_stINIPara.stIniAWB.stAwbCrCbTrack = (SCENEAUTO_AWB_CBCR_TRACK_ATTR_S*)malloc(sizeof(SCENEAUTO_AWB_CBCR_TRACK_ATTR_S));
	CHECK_NULL_PTR(g_stINIPara.stIniAWB.stAwbCrCbTrack);
	g_stINIPara.stIniAWB.stAwbStatisticsPara = (SCENEAUTO_WB_STATISTICS_CFG_PARA_S*)malloc(sizeof(SCENEAUTO_WB_STATISTICS_CFG_PARA_S));
	CHECK_NULL_PTR(g_stINIPara.stIniAWB.stAwbStatisticsPara);
	/****************AwbCrMax***************/
	pszTempStr = iniparser_getstr(g_Sceneautodictionary, "AWB:CrMax");
	if(NULL == pszTempStr)
	{
		printf("AWB:CrMax error\n");
        	return HI_FAILURE;
	}
	Weight(pszTempStr);
	for(i = 0; i < 16; ++i)
	{
		g_stINIPara.stIniAWB.stAwbCrCbTrack->au16CrMax[i] = MAEWeight[i];
	}

	/****************AwbCrMin***************/
	pszTempStr = iniparser_getstr(g_Sceneautodictionary, "AWB:CrMin");
	if(NULL == pszTempStr)
	{
		printf("AWB:CrMin error\n");
        	return HI_FAILURE;
	}
	Weight(pszTempStr);
	for(i = 0; i < 16; ++i)
	{
		g_stINIPara.stIniAWB.stAwbCrCbTrack->au16CrMin[i] = MAEWeight[i];
	}
	/****************AwbCbMax***************/
	pszTempStr = iniparser_getstr(g_Sceneautodictionary, "AWB:CbMax");
	if(NULL == pszTempStr)
	{
		printf("AWB:CbMax error\n");
        	return HI_FAILURE;
	}
	Weight(pszTempStr);
	for(i = 0; i < 16; ++i)
	{
		g_stINIPara.stIniAWB.stAwbCrCbTrack->au16CbMax[i] = MAEWeight[i];
	}

	/****************AwbCbMin***************/
	
	pszTempStr = iniparser_getstr(g_Sceneautodictionary, "AWB:CbMin");
	if(NULL == pszTempStr)
	{
		printf("AWB:CbMin error\n");
        	return HI_FAILURE;
	}
	Weight(pszTempStr);
	for(i = 0; i < 16; ++i)
	{
		g_stINIPara.stIniAWB.stAwbCrCbTrack->au16CbMin[i] = MAEWeight[i];
	}
	/****************AwbISO***************/
	u32Temp = 0;
    	u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "AWB:ISO", HI_FAILURE);
    	if (HI_FAILURE == u32Temp)
    	{
      	  	printf("AWB:ISO failed\n");
        	return HI_FAILURE;
    	}
	g_stINIPara.stIniAWB.u32ISO = u32Temp;

	/****************AwbTrackBlack***************/
	u32Temp = 0;
    	u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "AWB:TrackBlack", HI_FAILURE);
    	if (HI_FAILURE == u32Temp)
    	{
      	  	printf("AWB:TrackBlack failed\n");
        	return HI_FAILURE;
    	}
	g_stINIPara.stIniAWB.stAwbStatisticsPara->u16BlackLevel = (u32Temp&0xFFFF);
	return HI_SUCCESS;
}

HI_S32 Sceneauto_LoadBlackLevel()
{
	//HI_S32 s32Temp;
	HI_S32 i;
	HI_CHAR* pszTempStr;
	g_stINIPara.stIniBlackLevel.BlackLevel = (SCENEAUTO_BLACK_LEVEL_S*)malloc(sizeof(SCENEAUTO_BLACK_LEVEL_S));
	CHECK_NULL_PTR(g_stINIPara.stIniBlackLevel.BlackLevel);
	/****************RedBlackLevel***************/
	//s32Temp = 0;
	pszTempStr = iniparser_getstr(g_Sceneautodictionary, "BlackLevel:R");
	if(NULL == pszTempStr)
	{
		printf("BlackLevel:R error\n");
        	return HI_FAILURE;
	}
	//s32Temp = Weight(pszTempStr);
	Weight(pszTempStr);
	for(i = 0; i < 16; i++)
	{
		g_stINIPara.stIniBlackLevel.BlackLevel->au16R[i] = MAEWeight[i];
	}
	/****************GreenRBlackLevel***************/
	//s32Temp = 0;
	pszTempStr = iniparser_getstr(g_Sceneautodictionary, "BlackLevel:Gr");
	if(NULL == pszTempStr)
	{
		printf("BlackLevel:Gr error\n");
        	return HI_FAILURE;
	}
	//s32Temp = Weight(pszTempStr);
	Weight(pszTempStr);
	for(i = 0; i < 16; i++)
	{
		g_stINIPara.stIniBlackLevel.BlackLevel->au16Gr[i] = MAEWeight[i];
	}
	/****************GreenBBlackLevel***************/
	//s32Temp = 0;
	pszTempStr = iniparser_getstr(g_Sceneautodictionary, "BlackLevel:Gb");
	if(NULL == pszTempStr)
	{
		printf("BlackLevel:Gb error\n");
        	return HI_FAILURE;
	}
	//s32Temp = Weight(pszTempStr);
	Weight(pszTempStr);
	for(i = 0; i < 16; i++)
	{
		g_stINIPara.stIniBlackLevel.BlackLevel->au16Gb[i] = MAEWeight[i];
	}
	/****************BlueBlackLevel***************/
	//s32Temp = 0;
	pszTempStr = iniparser_getstr(g_Sceneautodictionary, "BlackLevel:B");
	if(NULL == pszTempStr)
	{
		printf("BlackLevel:B error\n");
        	return HI_FAILURE;
	}
	//s32Temp = Weight(pszTempStr);
	Weight(pszTempStr);
	for(i = 0; i < 16; i++)
	{
		g_stINIPara.stIniBlackLevel.BlackLevel->au16B[i] = MAEWeight[i];
	}

	return HI_SUCCESS;
}

HI_S32 Sceneauto_LoadShading()
{
	HI_S32 s32Temp;
	/**************Shading enable**************/
	s32Temp = 0;
    	s32Temp = iniparser_getint(g_Sceneautodictionary, "Shading:enable", HI_FAILURE);
    	if (HI_FAILURE == s32Temp)
    	{
      	  	printf("Shading:enable failed\n");
        	return HI_FAILURE;
    	}
	g_stINIPara.stIniShading.bEnable = s32Temp;
	/**************Shading Type**************/
	s32Temp = 0;
    	s32Temp = iniparser_getint(g_Sceneautodictionary, "Shading:Type", HI_FAILURE);
    	if (HI_FAILURE == s32Temp)
    	{
      	  	printf("Shading:Type failed\n");
        	return HI_FAILURE;
    	}
	g_stINIPara.stIniShading.enOpType = s32Temp;
	return HI_SUCCESS;
}



HI_S32 Sceneauto_LoadCommon()
{   
    HI_S32 s32Temp;
	HI_U32 u32Temp;
    HI_S32 i;
    HI_CHAR* pszTempStr;

    /**************common:IspDev**************/
    s32Temp = 0;
    s32Temp = iniparser_getint(g_Sceneautodictionary, "common:IspDev", HI_FAILURE);
    if (HI_FAILURE == s32Temp)
    {
        printf("common:IspDev failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stMpInfo.s32IspDev = s32Temp;

    /**************common:ViDev**************/
    s32Temp = 0;
    s32Temp = iniparser_getint(g_Sceneautodictionary, "common:ViDev", HI_FAILURE);
    if (HI_FAILURE == s32Temp)
    {
        printf("common:ViDev failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stMpInfo.s32ViDev = s32Temp;

    /**************common:ViChn**************/
    s32Temp = 0;
    s32Temp = iniparser_getint(g_Sceneautodictionary, "common:ViChn", HI_FAILURE);
    if (HI_FAILURE == s32Temp)
    {
        printf("common:ViChn failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stMpInfo.s32ViChn = s32Temp;

    /**************common:VpssGrp**************/
    s32Temp = 0;
    s32Temp = iniparser_getint(g_Sceneautodictionary, "common:VpssGrp", HI_FAILURE);
    if (HI_FAILURE == s32Temp)
    {
        printf("common:VpssGrp failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stMpInfo.s32VpssGrp = s32Temp;

    /**************common:VpssChn**************/
    s32Temp = 0;
    s32Temp = iniparser_getint(g_Sceneautodictionary, "common:VpssChn", HI_FAILURE);
    if (HI_FAILURE == s32Temp)
    {
        printf("common:VpssChn failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stMpInfo.s32VpssChn = s32Temp;

    /**************common:VencGrp**************/
    s32Temp = 0;
    s32Temp = iniparser_getint(g_Sceneautodictionary, "common:VencGrp", HI_FAILURE);
    if (HI_FAILURE == s32Temp)
    {
        printf("common:VencGrp failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stMpInfo.s32VencGrp = s32Temp;

    /**************common:VencChn**************/
    s32Temp = 0;
    s32Temp = iniparser_getint(g_Sceneautodictionary, "common:VencChn", HI_FAILURE);
    if (HI_FAILURE == s32Temp)
    {
        printf("common:VencChn failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stMpInfo.s32VencChn = s32Temp;

    /**************common:IVE_Enable**************/
    s32Temp = 0;
    s32Temp = iniparser_getint(g_Sceneautodictionary, "common:IVE_Enable", HI_FAILURE);
    if (HI_FAILURE == s32Temp)
    {
        printf("common:IVE_Enable failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stThreshValue.bIVEEnable = (HI_BOOL)s32Temp;

    /**************common:ave_lum_thresh**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "common:ave_lum_thresh", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("common:ave_lum_thresh failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stThreshValue.u32AveLumThresh = u32Temp;

    /**************common:delta_dis_expthresh**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "common:delta_dis_expthresh", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("common:delta_dis_expthresh failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stThreshValue.u32DeltaDisExpThreash = u32Temp;

    /**************common:fpn_exp_thresh**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "common:fpn_exp_thresh", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("common:fpn_exp_thresh failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stThreshValue.u32FpnExpThresh = u32Temp;

    /**************common:u32DRCStrengthThresh**************/
    u32Temp = 0;
    u32Temp = iniparser_get_unsigned_int(g_Sceneautodictionary, "common:u32DRCStrengthThresh", HI_FAILURE);
    if (HI_FAILURE == u32Temp)
    {
        printf("common:u32DRCStrengthThresh failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.stThreshValue.u32DRCStrengthThresh = u32Temp;

    /**************common:dci_strength_lut**************/
    pszTempStr = iniparser_getstr(g_Sceneautodictionary, "common:dci_strength_lut");
    if (NULL == pszTempStr)
    {
        printf("common:dci_strength_lut error\n");
        return HI_FAILURE;
    }
    s32Temp = Weight(pszTempStr);
    for (i = 0; i < s32Temp; i++)
    {
        g_stINIPara.au8DciStrengthLut[i] = MAEWeight[i];
    }

    /**************common:u32DRCStrengthThresh**************/
    s32Temp = 0;
    s32Temp = iniparser_getint(g_Sceneautodictionary, "common:BoolRefExporeTime", HI_FAILURE);
    if (HI_FAILURE == s32Temp)
    {
        printf("common:BoolRefExporeTime failed\n");
        return HI_FAILURE;
    }
    g_stINIPara.BoolRefExporeTime = (HI_BOOL)s32Temp;
    return HI_SUCCESS;
}

HI_S32 Sceneauto_LoadINIPara()
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = Sceneauto_LoadCommon();
    if (HI_SUCCESS != s32Ret)
    {
        printf("Sceneauto_LoadCommon failed!\n");
        return HI_FAILURE;
    }

    s32Ret = Sceneauto_LoadAE();
    if (HI_SUCCESS != s32Ret)
    {
        printf("Sceneauto_LoadAE failed!\n");
        return HI_FAILURE;
    } 

    s32Ret = Sceneauto_LoadAWB();
    if(HI_SUCCESS != s32Ret)
    {
 	  printf("Sceneauto_LoadAWB failed!\n");
        return HI_FAILURE;
    }

    s32Ret = Sceneauto_LoadBlackLevel();
    if(HI_SUCCESS != s32Ret)
    {
 	  printf("Sceneauto_LoadBlackLevel failed!\n");
        return HI_FAILURE;
    }

    s32Ret = Sceneauto_LoadShading();
    if(HI_SUCCESS != s32Ret)
    {
 	  printf("Sceneauto_LoadShading failed!\n");
        return HI_FAILURE;
    }

	
    s32Ret = Sceneauto_LoadSharpen();
    if (HI_SUCCESS != s32Ret)
    {
        printf("Sceneauto_LoadSharpen failed!\n");
        return HI_FAILURE;
    }

    s32Ret = Sceneauto_LoadDp();
    if (HI_SUCCESS != s32Ret)
    {
        printf("Sceneauto_LoadDp failed!\n");
        return HI_FAILURE;
    }
	s32Ret = Sceneauto_LoadDefog();
    if (HI_SUCCESS != s32Ret)
    {
        printf("Sceneauto_LoadDefog failed!\n");
        return HI_FAILURE;
    }
    s32Ret = Sceneauto_LoadDci();
    if (HI_SUCCESS != s32Ret)
    {
        printf("Sceneauto_LoadDci failed!\n");
        return HI_FAILURE;
    }

    s32Ret = Sceneauto_Load2DNR();
    if (HI_SUCCESS != s32Ret)
    {
        printf("Sceneauto_Load2dnr failed!\n");
        return HI_FAILURE;
    }
	s32Ret = Sceneauto_LoadDrc();
    if (HI_SUCCESS != s32Ret)
    {
        printf("Sceneauto_LoadDefog failed!\n");
        return HI_FAILURE;
    }

    s32Ret = Sceneauto_LoadGamma();
    if (HI_SUCCESS != s32Ret)
    {
        printf("Sceneauto_LoadGamma failed!\n");
        return HI_FAILURE;
    }

    s32Ret = Sceneauto_LoadH264Param();
    if (HI_SUCCESS != s32Ret)
    {
        printf("Sceneauto_LoadH264Param failed!\n");
        return HI_FAILURE;
    }

    s32Ret = Sceneauto_LoadH265Param();
    if (HI_SUCCESS != s32Ret)
    {
        printf("Sceneauto_LoadH265Param failed!\n");
        return HI_FAILURE;
    }

    s32Ret = Sceneauto_Load3Dnr();
    if (HI_SUCCESS != s32Ret)
    {
        printf("Sceneauto_Load3Dnr failed!\n");
        return HI_FAILURE;
    }

    s32Ret = Sceneauto_LoadIr();
    if (HI_SUCCESS != s32Ret)
    {
        printf("Sceneauto_LoadIr failed!\n");
        return HI_FAILURE;
    }

    s32Ret = Sceneauto_LoadHlc();
    if (HI_SUCCESS != s32Ret)
    {
        printf("Sceneauto_LoadHlc failed!\n");
        return HI_FAILURE;
    }
	
    s32Ret = Sceneauto_LoadTraffic();
    if (HI_SUCCESS != s32Ret)
    {
        printf("Sceneauto_LoadTraffic failed!\n");
        return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}


HI_S32 Sceneauto_LoadFile(const HI_CHAR* pszFILENAME)
{
    if (NULL != g_Sceneautodictionary)
    {
        g_Sceneautodictionary = NULL;
    }
    else
    {
        g_Sceneautodictionary = iniparser_load(pszFILENAME);
        if (NULL == g_Sceneautodictionary)
        {
            printf("%s ini load failed\n", pszFILENAME);
            return HI_FAILURE;
        }
    }
    return HI_SUCCESS;
}

HI_VOID Sceneauto_FreeDict()
{
    if (NULL != g_Sceneautodictionary)
    {
        iniparser_freedict(g_Sceneautodictionary);
    }
    g_Sceneautodictionary = NULL;
}

HI_VOID Sceneauto_FreeMem()
{
    if (NULL != g_stINIPara.stIniAE.pu32BitrateThresh)
    {
        free(g_stINIPara.stIniAE.pu32BitrateThresh);
        g_stINIPara.stIniAE.pu32BitrateThresh = NULL;
    }

    if (NULL != g_stINIPara.stIniAE.pu32AEExpHtoLThresh)
    {
        free(g_stINIPara.stIniAE.pu32AEExpHtoLThresh);
        g_stINIPara.stIniAE.pu32AEExpHtoLThresh = NULL;
    }

    if (NULL != g_stINIPara.stIniAE.pu32AEExpLtoHThresh)
    {
        free(g_stINIPara.stIniAE.pu32AEExpLtoHThresh);
        g_stINIPara.stIniAE.pu32AEExpLtoHThresh = NULL;
    }

    if (NULL != g_stINIPara.stIniAE.pstAERelatedBit)
    {
        free(g_stINIPara.stIniAE.pstAERelatedBit);
        g_stINIPara.stIniAE.pstAERelatedBit = NULL;
    }

    if (NULL != g_stINIPara.stIniAE.pstAERelatedExp)
    {
        free(g_stINIPara.stIniAE.pstAERelatedExp);
        g_stINIPara.stIniAE.pstAERelatedExp = NULL;
    }

    if(NULL != g_stINIPara.stIniAWB.stAwbCrCbTrack)
    {
    	  free(g_stINIPara.stIniAWB.stAwbCrCbTrack);
         g_stINIPara.stIniAWB.stAwbCrCbTrack = NULL;
    }

    if(NULL != g_stINIPara.stIniAWB.stAwbStatisticsPara)
    {
    	  free(g_stINIPara.stIniAWB.stAwbStatisticsPara);
         g_stINIPara.stIniAWB.stAwbStatisticsPara = NULL;
    }

    if(NULL != g_stINIPara.stIniBlackLevel.BlackLevel)
    {
    	  free(g_stINIPara.stIniBlackLevel.BlackLevel);
         g_stINIPara.stIniBlackLevel.BlackLevel = NULL;
    }

    if (NULL != g_stINIPara.stIniSharpen.pu32BitrateThresh)
    {
        free(g_stINIPara.stIniSharpen.pu32BitrateThresh);
        g_stINIPara.stIniSharpen.pu32BitrateThresh = NULL;
    }

    if (NULL != g_stINIPara.stIniSharpen.pu32ExpThresh)
    {
        free(g_stINIPara.stIniSharpen.pu32ExpThresh);
        g_stINIPara.stIniSharpen.pu32ExpThresh = NULL;
    }

    if (NULL != g_stINIPara.stIniSharpen.pstSharpen)
    {
        free(g_stINIPara.stIniSharpen.pstSharpen);
        g_stINIPara.stIniSharpen.pstSharpen = NULL;
    }

    if (NULL != g_stINIPara.stIniDP.pu32ExpThresh)
    {
        free(g_stINIPara.stIniDP.pu32ExpThresh);
        g_stINIPara.stIniDP.pu32ExpThresh = NULL;
    }
    
    if (NULL != g_stINIPara.stIniDP.pstDPAttr)
    {
        free(g_stINIPara.stIniDP.pstDPAttr);
        g_stINIPara.stIniDP.pstDPAttr = NULL;
    }

    if (NULL != g_stINIPara.stIniGamma.pu32ExpThreshLtoH)
    {
        free(g_stINIPara.stIniGamma.pu32ExpThreshLtoH);
        g_stINIPara.stIniGamma.pu32ExpThreshLtoH = NULL;
    }
    
    if (NULL != g_stINIPara.stIniGamma.pu32ExpThreshHtoL)
    {
        free(g_stINIPara.stIniGamma.pu32ExpThreshHtoL);
        g_stINIPara.stIniGamma.pu32ExpThreshHtoL = NULL;
    }

    if (NULL != g_stINIPara.stIniGamma.pstGamma)
    {
        free(g_stINIPara.stIniGamma.pstGamma);
        g_stINIPara.stIniGamma.pstGamma = NULL;
    }

    if (NULL != g_stINIPara.stIniH264Venc.pu32BitrateThresh)
    {
        free(g_stINIPara.stIniH264Venc.pu32BitrateThresh);
        g_stINIPara.stIniH264Venc.pu32BitrateThresh = NULL;
    }

    if (NULL != g_stINIPara.stIniH264Venc.pstH264Venc)
    {
        free(g_stINIPara.stIniH264Venc.pstH264Venc);
        g_stINIPara.stIniH264Venc.pstH264Venc = NULL;
    }

    if (NULL != g_stINIPara.stIniH265Venc.stIniH265VencRcParam.pu32BitrateThresh)
    {
        free(g_stINIPara.stIniH265Venc.stIniH265VencRcParam.pu32BitrateThresh);
        g_stINIPara.stIniH265Venc.stIniH265VencRcParam.pu32BitrateThresh = NULL;
    }

    if (NULL != g_stINIPara.stIni3dnr.pu323DnrIsoThresh)
    {
        free(g_stINIPara.stIni3dnr.pu323DnrIsoThresh);
        g_stINIPara.stIni3dnr.pu323DnrIsoThresh = NULL;
    }

    if (NULL != g_stINIPara.stIni3dnr.pst3dnrParam)
    {
        free(g_stINIPara.stIni3dnr.pst3dnrParam);
        g_stINIPara.stIni3dnr.pst3dnrParam = NULL;
    }

    if (NULL != g_stINIPara.stIniIr.pu32ExpThreshHtoL)
    {
        free(g_stINIPara.stIniIr.pu32ExpThreshHtoL);
        g_stINIPara.stIniIr.pu32ExpThreshHtoL = NULL;
    }

    if (NULL != g_stINIPara.stIniIr.pu32ExpThreshLtoH)
    {
        free(g_stINIPara.stIniIr.pu32ExpThreshLtoH);
        g_stINIPara.stIniIr.pu32ExpThreshLtoH = NULL;
    }

    if (NULL != g_stINIPara.stIniIr.pu8ExpCompensation)
    {
        free(g_stINIPara.stIniIr.pu8ExpCompensation);
        g_stINIPara.stIniIr.pu8ExpCompensation = NULL;
    }

    if (NULL != g_stINIPara.stIniIr.pu8MaxHistOffset)
    {
        free(g_stINIPara.stIniIr.pu8MaxHistOffset);
        g_stINIPara.stIniIr.pu8MaxHistOffset = NULL;
    }
    
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

