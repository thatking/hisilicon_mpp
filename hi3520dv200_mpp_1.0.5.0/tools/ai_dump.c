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
#include "hi_comm_aio.h"
#include "mpi_ai.h"


#define VALUE_BETWEEN(x,min,max) (((x)>=(min)) && ((x) <= (max)))

HI_S32 main(int argc, char *argv[])
{
    AUDIO_DEV AiDevId;
    AI_CHN AiChn;
    HI_U32 u32Size = 1024;
    HI_CHAR path[256] = "./";
    AUDIO_SAVE_FILE_INFO_S stSaveFileInfo;
    HI_S32 s32Ret;

    printf("Usage: ./ai_dump [dev] [chn] [size] [path]\n");
    printf("dev: ai dev id.\n");
    printf("chn: ai chn id.\n");
    printf("size: file size(KB)(defalutly 1M).\n");
    printf("path: path for saving(current path if NULL).\n\n\n");

    if ((argc < 3)||(argc > 5))
    {
    	printf("wrong arg!!!!\n\n");
    	return -1;
    }

    AiDevId = atoi(argv[1]);
    if (!VALUE_BETWEEN(AiDevId, 0, AI_DEV_MAX_NUM))
    {
    	printf("grp id must be [0,63]!!!!\n\n");
    	return -1;
    }

    AiChn = atoi(argv[2]);/* chn id*/
    if (!VALUE_BETWEEN(AiDevId, 0, AIO_MAX_CHN_NUM))
    {
    	printf("chn id must be [0,4]!!!!\n\n");
    	return -1;
    }
    if (argc>=4)
    {
        if (!VALUE_BETWEEN(atoi(argv[3]), 1, 8*1024))
        {
        	printf("file size must be [1,8*1024]!!!!\n\n");
        	return -1;
        }
        u32Size = atoi(argv[3]);/* frame count*/
    }
    if (argc>=5)
    {
        if (!VALUE_BETWEEN(strlen(argv[4]), 0, 256))
        {
        	printf("path lenth must be [0,256]!!!!\n\n");
        	return -1;
        }
        memcpy(stSaveFileInfo.aFilePath,argv[4],strlen(argv[4]));
        stSaveFileInfo.aFilePath[strlen(argv[4])] = '\0';
    }
    else
    {
        memcpy(stSaveFileInfo.aFilePath,"./",strlen("./")+1);
    }
    stSaveFileInfo.u32FileSize = u32Size;
    stSaveFileInfo.bCfg = HI_TRUE;
    printf("File path:%s, file size:%d*1024\n\n",stSaveFileInfo.aFilePath,stSaveFileInfo.u32FileSize);
    s32Ret = HI_MPI_AI_Save_File(AiDevId, AiChn,&stSaveFileInfo);
    if (s32Ret != HI_SUCCESS)
    {
    	printf("HI_MPI_AI_Save_File() error,ret=%x!!!!\n\n",s32Ret);
    	return -1;
    }

    return HI_SUCCESS;
}

