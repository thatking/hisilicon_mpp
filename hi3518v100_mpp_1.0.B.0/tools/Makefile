ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../Makefile.param
     include $(PARAM_FILE)
endif

SRC  := $(wildcard *.c)
OBJ  := $(SRC:%.c=%.o)

INC_FLAGS := -I$(REL_INC)
CFLAGS += -Wall -g -lpthread $(INC_FLAGS)
LIBS += $(REL_LIB)/libisp.a
LIBS += $(REL_LIB)/libmpi.a

ifneq ($(HIARCH),hi3518)
LIBS += $(REL_LIB)/libhdmi.a
else
CFLAGS += -DHI_3518	
endif

# sensor input type: ar130 mn34031
SNS:=$(MAKECMDGOALS)
                    

ifeq ($(SNS),)
SNS:=ar130
endif

ifeq ($(SNS), ar130)
LIBS += $(REL_LIB)/libsns_ar0130_720p.a
CFLAGS += -DSNS_AR0130
endif

ifeq ($(SNS), 9m034)
LIBS += $(REL_LIB)/libsns_9m034.a
CFLAGS += -DSNS_9M034
endif

ifeq ($(SNS), imx104)
LIBS += $(REL_LIB)/libsns_imx104.a
CFLAGS += -DSNS_IMX104
endif

ifeq ($(SNS), imx122)
LIBS += $(REL_LIB)/libsns_imx122.a
CFLAGS += -DSNS_IMX122
endif

ifeq ($(SNS), mn34031)
LIBS += $(REL_LIB)/libsns_mn34031_720p.a
CFLAGS += -DSNS_MN34031
endif

ifeq ($(SNS), ov9712)
LIBS += $(REL_LIB)/libsns_ov9712.a
CFLAGS += -DSNS_OV9712
endif

ifeq ($(SNS), icx692)
LIBS += $(REL_LIB)/libsns_icx692.a
CFLAGS += -DSNS_ICX692
endif

ifeq ($(SNS), imx138)
LIBS += $(REL_LIB)/libsns_imx138.a
CFLAGS += -DSNS_IMX138
endif

ifeq ($(SNS), 9p006)
LIBS += $(REL_LIB)/libsns_mt9p006.a
CFLAGS += -DSNS_9P006
endif

ifeq ($(SNS), po3100k)
LIBS += $(REL_LIB)/libsns_po3100k.a
CFLAGS += -DSNS_PO3100K
endif

ifeq ($(SNS), ar0330)
LIBS += $(REL_LIB)/libsns_ar0330_1080p.a
CFLAGS += -DSNS_AR0330
endif

ifeq ($(SNS), imx236)
LIBS += $(REL_LIB)/libsns_imx236.a
CFLAGS += -DSNS_IMX236
endif

ifeq ($(CONFIG_ISP), y)
CFLAGS += -DISP_V1
else
LIBS += $(REL_LIB)/lib_hiae.a
LIBS += $(REL_LIB)/lib_hiawb.a
LIBS += $(REL_LIB)/lib_hiaf.a
CFLAGS += -DISP_V2 -lm
endif

TARGET := $(OBJ:%.o=%)

.PHONY : clean all $(SNS)

all: $(TARGET)

$(TARGET):%:%.o 
	$(CC) $(CFLAGS)  -o $@ $^ $(LIBS) $(AUDIO_LIBA) -Wl,-gc-sections
	@rm *.o
clean:
	@rm -f $(TARGET)
	@rm -f $(OBJ)
ifneq ($(SNS), clean)
$(SNS): all
	@echo
endif
