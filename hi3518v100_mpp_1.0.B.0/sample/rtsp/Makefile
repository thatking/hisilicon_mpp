# Hisilicon Hi3516 sample Makefile

include ../Makefile.param
#ifeq ($(SAMPLE_PARAM_FILE), )
#     SAMPLE_PARAM_FILE:=../Makefile.param
#     include $(SAMPLE_PARAM_FILE)
#endif

# target source
SRC  := $(wildcard *.c)
OBJ  := $(SRC:%.c=%.o)

TARGET := $(OBJ:%.o=%)
.PHONY : clean all

all: $(TARGET)

MPI_LIBS := $(REL_LIB)/libmpi.a

$(TARGET):%:%.o $(COMM_OBJ)
	$(CC) $(CFLAGS) -lpthread -lm -o $@ $^ $(MPI_LIBS) $(AUDIO_LIBA) $(SENSOR_LIBS)

clean:
	@rm -f $(TARGET)
	@rm -f $(OBJ)
	@rm -f $(COMM_OBJ)

cleanstream:
	@rm -f *.h264
	@rm -f *.jpg
	@rm -f *.mjp
	@rm -f *.mp4


#>./lib/libVoiceEngine.a ./lib/libaec.a ./lib/libresampler.a ./lib/libanr.a ./lib/libisp.a ./lib/lib_hiae.a ./lib/lib_hiawb.a ./lib/lib_hiaf.a
#<------><------>-Dhi3518 -DHICHIP=0x35180100 -DSENSOR_TYPE=OMNI_OV9712_DC_720P_31FPS -DHI_DEBUG -DHI_XXXX -DISP_V2 \
#<------><------>main.c ringfifo.c rtputils.c rtspservice.c rtsputils.c loadbmp.c sample_comm_audio.c sample_comm_isp.c sample_comm_sys.c sample_comm_vda.c sample_comm_venc.c sample_comm_vi.c sample_comm_vo.c sample_comm_vpss.c sample_venc.c \
#<------><------>-o $(@D)/rtspd
