#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"
#include "mpi_vb.h"
#include "mpi_sys.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "hi_comm_isp.h"
#include "mpi_isp.h"
#include "hi_sns_ctrl.h"

#ifdef ISP_V2
#include "hi_ae_comm.h"
#include "hi_awb_comm.h"
#endif


static inline HI_VOID Useage(HI_VOID)
{
    printf("\nNOTICE: This tool only can be used for DEBUG_ING !!!\n");
    printf("usage: ./Isp debug [ispdepth] [aedepth] [awbdepth]. sample: ./isp_debug 10 10 10\n");
    printf("[ispdepth]----------the frame number of isp\n");
    printf("[aedepth] ----------the frame number of ae\n");
    printf("[awbdepth]----------the frame number of awb\n\n");

    return;
}


HI_S32 main(int argc, char *argv[])
{
#ifdef ISP_V1
    HI_S32 s32Ret = 0;
    HI_U32 u32Nbit = 0;
    HI_U32 enable = 0;
    HI_U32 u32depth = 10;
    HI_U32 u32SizeHi;
    HI_U32 u32SizeLo;
    HI_U32 u32MemSize;
    
    HI_U32 u32PhyAddr;
    HI_VOID *pVitAddr;
    FILE *fp;    
    ISP_DEBUG_INFO_S  stIspDebug;    

    printf("\nNOTICE: This tool only can be used for DEBUG_ING !!!\n");
    
    if(argc < 3)
    {
        printf("usage: ./Isp debug [0/1/2] [enable] [depth]. sample: ./isp_debug 0 1 10\n");
        printf("[0/1/2]----------AE/AWB/SYS:0;1;2\r\n");
        printf("[enable/disable]---------1/0");
        printf("[depth]----------the frame number \r\n\n\n");

        return s32Ret;
    }

    s32Ret = HI_MPI_ISP_GetDebug(&stIspDebug);

    if (argc > 1)
    {
        u32Nbit = atoi(argv[1]);    /* AE/AWB/SYS */       
    }
    if(argc > 2)
    {
       enable = atoi(argv[2]);   /*enable/disable*/
       if((0 == u32Nbit)&&(1 == enable))
       {
         stIspDebug.bAEDebugEnable = 1;
       }
       else if((0 == u32Nbit)&&(0 == enable))
       {
         stIspDebug.bAEDebugEnable = 0;
       }
       else if((1 == u32Nbit)&&(1 == enable))
       {
         stIspDebug.bAWBDebugEnable = 1;
       }
       else if((1 == u32Nbit)&&(0 == enable))
       {
         stIspDebug.bAWBDebugEnable = 0;
       }
       else if((2 == u32Nbit)&&(1 == enable))
       {
        stIspDebug.bSysDebugEnable = 1;
       }
       else if((2 == u32Nbit)&&(0 == enable))
       {
        stIspDebug.bSysDebugEnable = 0;
       }
    }
    if (argc > 3)
    {
        u32depth = atoi(argv[3]);/* the frame depth */
        stIspDebug.u32DebugDepth = u32depth;
        stIspDebug.u32DebugDepth = (0 == stIspDebug.u32DebugDepth)? 1: stIspDebug.u32DebugDepth;
    }
    


   if(stIspDebug.bAEDebugEnable == 1)
   {
     u32SizeHi = (stIspDebug.u32AESize & 0xFFFF0000) >> 16;  /*status*/
     u32SizeLo = stIspDebug.u32AESize & 0xFFFF;              /*cfg*/
     u32MemSize = u32SizeLo + u32SizeHi * stIspDebug.u32DebugDepth;
   }
   else if(stIspDebug.bAWBDebugEnable == 1)
   {
     u32SizeHi = (stIspDebug.u32AWBSize & 0xFFFF0000) >> 16;  /*status*/
     u32SizeLo = stIspDebug.u32AWBSize & 0xFFFF;              /*cfg*/
     u32MemSize = u32SizeLo + u32SizeHi * stIspDebug.u32DebugDepth;
   }
   else if(stIspDebug.bSysDebugEnable == 1)
   {
     u32SizeHi = (stIspDebug.u32SysSize & 0xFFFF0000) >> 16;  /*status*/
     u32SizeLo = stIspDebug.u32SysSize & 0xFFFF;              /*cfg*/
     u32MemSize = u32SizeLo + u32SizeHi * stIspDebug.u32DebugDepth;
   }
    
    s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&u32PhyAddr, &pVitAddr, NULL, NULL, u32MemSize);

    if(stIspDebug.bAEDebugEnable == 1)
    {
      stIspDebug.u32AEAddr = u32PhyAddr;
    }
    else if(stIspDebug.bAWBDebugEnable == 1)
    {
      stIspDebug.u32AWBAddr = u32PhyAddr;
    }
    else if(stIspDebug.bSysDebugEnable == 1)
    {
        stIspDebug.u32SysAddr = u32PhyAddr;
    }
    
    HI_MPI_ISP_SetDebug(&stIspDebug);
    
    printf("Waiting  ,press any key to stop adjust!\n");
    getchar();
    
    HI_U32 *pu32VirAddr = NULL;

    if(stIspDebug.bAEDebugEnable == 1)
    {
        pu32VirAddr = (HI_U32 *)HI_MPI_SYS_Mmap(stIspDebug.u32AEAddr, u32MemSize);
    }
    else if(stIspDebug.bAWBDebugEnable == 1)
    {
        pu32VirAddr = (HI_U32 *)HI_MPI_SYS_Mmap(stIspDebug.u32AWBAddr, u32MemSize);
    }
    else if(stIspDebug.bSysDebugEnable == 1)
    {
        pu32VirAddr = (HI_U32 *)HI_MPI_SYS_Mmap(stIspDebug.u32SysAddr, u32MemSize);
    }

    char *filename;
    if(stIspDebug.bAEDebugEnable == 1)
    {
      filename = "isp_debug_ae.dat";
    }
    else if(stIspDebug.bAWBDebugEnable == 1)
    {
      filename = "isp_debug_awb.dat";;
    }
    else if(stIspDebug.bSysDebugEnable == 1)
    {
      filename = "isp_debug_sys.dat";;
    }
    
    fp=fopen(filename,"wb");
    if(fp==NULL)
    {
      printf("open file %s error \n",filename);
      return -1;
    }   

      fwrite(pu32VirAddr,1,u32MemSize,fp);

      sleep(1);

     fclose(fp);
    
     stIspDebug.bAEDebugEnable = 0;
     stIspDebug.bAWBDebugEnable = 0;
     stIspDebug.bSysDebugEnable = 0;
     HI_MPI_ISP_SetDebug(&stIspDebug);
     
     HI_MPI_SYS_Munmap(pu32VirAddr, u32MemSize);
     HI_MPI_SYS_MmzFree(u32PhyAddr, pVitAddr);   
     
#endif

#ifdef ISP_V2

    HI_S32 i, s32Ret = 0;

    HI_U32  au32Depth[3]    = {0};
    HI_U32  au32Size[3]     = {0};
    HI_U32  au32PhyAddr[3]  = {0};
    HI_VOID *apVitAddr[3]   = {HI_NULL};    /* virt addr malloc memory */
    HI_VOID *apVirtAddr[3]  = {HI_NULL};    /* virt addr mmap */
    HI_CHAR *acFilename[3]  = {HI_NULL};
    FILE    *afp[3]         = {HI_NULL};
    ISP_DEBUG_INFO_S  stDebug; 
    ALG_LIB_S stLib; 

    Useage();

    if(argc <= 1)
    {
        Useage();
        return HI_FAILURE;
    }

    if (argc > 1)
    {
        au32Depth[0] = atoi(argv[1]);/* the frame depth */
        if (0 == au32Depth[0])
        {
            Useage();
            return HI_FAILURE;
        }        

        au32Size[0] = sizeof(ISP_DBG_ATTR_S) + sizeof(ISP_DBG_STATUS_S) * au32Depth[0];
        s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&au32PhyAddr[0], &apVitAddr[0], HI_NULL, HI_NULL, au32Size[0]);
        if (HI_SUCCESS != s32Ret)
        {
            printf("Buf not enough!\n");
            return HI_FAILURE;
        }

        stDebug.bDebugEn = HI_TRUE;
        stDebug.u32Depth = au32Depth[0];
        stDebug.u32PhyAddr = au32PhyAddr[0];
        s32Ret = HI_MPI_ISP_SetDebug(&stDebug);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_ISP_SetDebug failed 0x%x!\n", s32Ret);
            return HI_FAILURE;
        }
    }

    if(argc > 2)
    {
        au32Depth[1] = atoi(argv[2]);/* the frame depth */
        if (0 == au32Depth[1])
        {
            Useage();
            return HI_FAILURE;
        }

        au32Size[1] = sizeof(AE_DBG_ATTR_S) + sizeof(AE_DBG_STATUS_S) * au32Depth[1];
        s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&au32PhyAddr[1], &apVitAddr[1], HI_NULL, HI_NULL, au32Size[1]);
        if (HI_SUCCESS != s32Ret)
        {
            printf("Buf not enough!\n");
            return HI_FAILURE;
        }

        stDebug.bDebugEn = HI_TRUE;
        stDebug.u32Depth = au32Depth[1];
        stDebug.u32PhyAddr = au32PhyAddr[1];
        stLib.s32Id = 0;
        strcpy(stLib.acLibName, HI_AE_LIB_NAME);
        s32Ret = HI_MPI_AE_Ctrl(&stLib, AE_DEBUG_ATTR_SET, (HI_VOID *)&stDebug);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_AE_Ctrl failed 0x%x!\n", s32Ret);
            return HI_FAILURE;
        }
    }

    if (argc > 3)
    {
        au32Depth[2] = atoi(argv[3]);/* the frame depth */
        if (0 == au32Depth[2])
        {
            Useage();
            return HI_FAILURE;
        }

        au32Size[2] = sizeof(AWB_DBG_ATTR_S) + sizeof(AWB_DBG_STATUS_S) * au32Depth[2];
        s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&au32PhyAddr[2], &apVitAddr[2], HI_NULL, HI_NULL, au32Size[2]);
        if (HI_SUCCESS != s32Ret)
        {
            printf("Buf not enough!\n");
            return HI_FAILURE;
        }

        stDebug.bDebugEn = HI_TRUE;
        stDebug.u32Depth = au32Depth[2];
        stDebug.u32PhyAddr = au32PhyAddr[2];
        stLib.s32Id = 0;
        strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
        s32Ret = HI_MPI_AWB_Ctrl(&stLib, AWB_DEBUG_ATTR_SET, (HI_VOID *)&stDebug);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_ISP_AwbCtrl failed 0x%x!\n", s32Ret);
            return HI_FAILURE;
        }
    }

    printf("Waiting, press any key to stop write debug info!\n");
    getchar();

    for (i=0; i<argc-1; i++)
    {
        apVirtAddr[i] = HI_MPI_SYS_Mmap(au32PhyAddr[i], au32Size[i]);
        if (HI_NULL == apVirtAddr[i])
        {
            printf("HI_MPI_SYS_Mmap failed!\n");
            return HI_FAILURE;
        }

        if (0 == i)
        {
            acFilename[i] = "isp_debug_sys.dat";
        }
        else if (1 == i)
        {
            acFilename[i] = "isp_debug_ae.dat";
        }
        else if (2 == i)
        {
            acFilename[i] = "isp_debug_awb.dat";
        }
        else
        {
        }

        afp[i] =fopen(acFilename[i], "wb");
        if (afp[i] == HI_NULL)
        {
          printf("open file %s error \n", acFilename[i]);
          return HI_FAILURE;
        }
        else
        {
            fwrite(apVirtAddr[i], 1, au32Size[i], afp[i]);
        }
    }

    // stop isp debug
    stDebug.bDebugEn = 0;
    stDebug.u32Depth = 0;
    stDebug.u32PhyAddr = 0;
    stLib.s32Id = 0;
    s32Ret = HI_MPI_ISP_SetDebug(&stDebug);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetDebug failed 0x%x! Line: %d\n", s32Ret, __LINE__);
        return HI_FAILURE;
    }

    // stop AE debug
    strcpy(stLib.acLibName, HI_AE_LIB_NAME); 
    s32Ret = HI_MPI_AE_Ctrl(&stLib, AE_DEBUG_ATTR_SET, (HI_VOID *)&stDebug);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_AE_Ctrl failed 0x%x!\n", s32Ret);
        return HI_FAILURE;
    }

    // stop AWB debug
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    s32Ret = HI_MPI_AWB_Ctrl(&stLib, AWB_DEBUG_ATTR_SET, (HI_VOID *)&stDebug);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_AwbCtrl failed 0x%x!\n", s32Ret);
        return HI_FAILURE;
    }

    sleep(1);

    for (i=0; i<3; i++)
    {
        if (HI_NULL != afp[i])
        {
            fclose(afp[i]);
            afp[i] = HI_NULL;
        }
            
        if ((HI_NULL != apVirtAddr[i]) && (0 != au32Size[i]))
        {
            HI_MPI_SYS_Munmap(apVirtAddr[i], au32Size[i]);
            apVirtAddr[i] = HI_NULL;
            au32Size[i] = 0;
        }
        
        if ((0 != au32PhyAddr[i]) && (HI_NULL != apVitAddr[i]))
        {
            HI_MPI_SYS_MmzFree(au32PhyAddr[i], apVitAddr[i]);
            au32PhyAddr[i] = 0;
            apVitAddr[i] = HI_NULL;
        }
    }

#if 0
    stDebug.bDebugEn = 0;
    stDebug.u32Depth = 0;
    stDebug.u32PhyAddr = 0;

    for (i=0; i<argc-1; i++)
    {
        apVirtAddr[i] = HI_MPI_SYS_Mmap(au32PhyAddr[i], au32Size[i]);
        if (HI_NULL == apVirtAddr[i])
        {
            printf("HI_MPI_SYS_Mmap failed!\n");
            return HI_FAILURE;
        }

        if (0 == i)
        {
            HI_MPI_ISP_SetDebug(&stDebug);
            acFilename[i] = "isp_debug_sys.dat";
        }
        else if (1 == i)
        {
            HI_MPI_AE_Ctrl(&stLib, AE_DEBUG_ATTR_SET, (HI_VOID *)&stDebug);
            acFilename[i] = "isp_debug_ae.dat";
        }
        else if (2 == i)
        {
            HI_MPI_AWB_Ctrl(&stLib, AWB_DEBUG_ATTR_SET, (HI_VOID *)&stDebug);
            acFilename[i] = "isp_debug_awb.dat";
        }
        else
        {
        }

        afp[i] =fopen(acFilename[i], "wb");
        if (afp[i] == HI_NULL)
        {
          printf("open file %s error \n", acFilename[i]);
          return HI_FAILURE;
        }
    }

    for (i=0; i<argc-1; i++)
    {
        if (HI_NULL != afp[i])
        {
            fwrite(apVirtAddr[i], 1, au32Size[i], afp[i]);
            sleep(1);
            fclose(afp[i]);
            HI_MPI_SYS_Munmap(apVirtAddr[i], au32Size[i]);
            HI_MPI_SYS_MmzFree(au32PhyAddr[i], apVitAddr[i]);   
        }
    }   
#endif

#endif
    return HI_SUCCESS;
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


