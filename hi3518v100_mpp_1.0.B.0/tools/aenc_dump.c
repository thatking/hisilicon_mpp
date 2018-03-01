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
#include "mpi_aenc.h"


#define VALUE_BETWEEN(x,min,max) (((x)>=(min)) && ((x) <= (max)))

HI_S32 main(int argc, char *argv[])
{
    AENC_CHN AencChn;
    HI_U32 u32Size = 1024;
    HI_CHAR path[256] = "./";
    AUDIO_SAVE_FILE_INFO_S stSaveFileInfo;
    HI_S32 s32Ret;

    printf("Usage: ./aenc_dump [chn] [size] [path]\n");
    printf("chn: aenc chn id.\n");
    printf("size: file size(KB)(defalutly 1M).\n");
    printf("path: path for saving(current path if NULL).\n\n\n");

    if ((argc < 2)||(argc > 4))
    {
    	printf("wrong arg!!!!\n\n");
    	return -1;
    }

    AencChn = atoi(argv[1]);/* chn id*/
    if (!VALUE_BETWEEN(AencChn, 0, AENC_MAX_CHN_NUM))
    {
    	printf("chn id must be [0,32]!!!!\n\n");
    	return -1;
    }
    if (argc>=3)
    {
        if (!VALUE_BETWEEN(atoi(argv[2]), 1, 8*1024))
        {
        	printf("file size must be [1,8*1024]!!!!\n\n");
        	return -1;
        }
        u32Size = atoi(argv[2]);/* frame count*/
    }
    if (argc>=4)
    {
        if (!VALUE_BETWEEN(strlen(argv[3]), 0, 256))
        {
        	printf("path lenth must be [0,256]!!!!\n\n");
        	return -1;
        }
        memcpy(stSaveFileInfo.aFilePath,argv[3],strlen(argv[3]));
        stSaveFileInfo.aFilePath[strlen(argv[3])] = '\0';
    }
    else
    {
        memcpy(stSaveFileInfo.aFilePath,"./",strlen("./")+1);
    }
    stSaveFileInfo.u32FileSize = u32Size;
    stSaveFileInfo.bCfg = HI_TRUE;
    printf("File path:%s, file size:%d*1024\n\n",stSaveFileInfo.aFilePath,stSaveFileInfo.u32FileSize);
    s32Ret = HI_MPI_AENC_Save_File(AencChn,&stSaveFileInfo);
    if (s32Ret != HI_SUCCESS)
    {
    	printf("HI_MPI_AENC_Save_File() error,ret=%x!!!!\n\n",s32Ret);
    	return -1;
    }

    return HI_SUCCESS;
}

