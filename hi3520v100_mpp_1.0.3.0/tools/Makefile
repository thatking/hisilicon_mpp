ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../Makefile.param
     include $(PARAM_FILE)
endif

SRC  := $(wildcard *.c)
OBJ  := $(SRC:%.c=%.o)

INC_FLAGS := -I$(REL_INC)
CFLAGS += -Wall -g -lpthread $(INC_FLAGS)
#LIBS += $(REL_LIB)/libisp.a
LIBS += $(REL_LIB)/libmpi.a
LIBS += $(REL_LIB)/libhdmi.a

ifeq ($(MAKECMDGOALS), 9p031)
LIBS += $(REL_LIB)/libsns_aptn.a
CFLAGS += -DSNS_9P031
else
ifeq ($(MAKECMDGOALS), 9p031_5m)
LIBS += $(REL_LIB)/libsns_aptn_5m.a
CFLAGS += -DSNS_9P031_5M
else
ifeq ($(MAKECMDGOALS), 9p031_720p)
LIBS += $(REL_LIB)/libsns_aptn_720p.a
CFLAGS += -DSNS_9P031_720P
else
ifeq ($(MAKECMDGOALS), ov2715)
LIBS += $(REL_LIB)/libsns_ov.a
CFLAGS += -DSNS_OV2715
else
ifeq ($(MAKECMDGOALS), imx036)
LIBS += $(REL_LIB)/libsns_imx036.a
CFLAGS += -DSNS_IMX036
else
ifeq ($(MAKECMDGOALS), panso34041)
LIBS += $(REL_LIB)/libsns_pana.a
CFLAGS += -DSNS_PANSO34041
else
ifeq ($(MAKECMDGOALS), alta)
LIBS += $(REL_LIB)/libsns_alta.a
CFLAGS += -DSNS_ALTA
else
#LIBS += $(REL_LIB)/libsns_aptn.a
CFLAGS += -DSNS_9P031
endif
endif
endif
endif
endif
endif
endif

TARGET := $(OBJ:%.o=%)

.PHONY : clean all 9p031 9p031_5m 9p031_720p imx036 panso34041 ov2715 alta

all: $(TARGET)

$(TARGET):%:%.o 
	$(CC) $(CFLAGS)  -o $@ $^ $(LIBS) $(AUDIO_LIBA) $(JPEGD_LIBA)
	@rm *.o
9p031: all
9p031_5m: all
9p031_720p: all
imx036: all
panso34041: all
ov2715: all
alta: all
clean:
	@rm -f $(TARGET)
	@rm -f $(OBJ)
