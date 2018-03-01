ifeq ($(PARAM_FILE), )
     PARAM_FILE:=../Makefile.param
     include $(PARAM_FILE)
endif

SRC  := $(wildcard *.c)
OBJ  := $(SRC:%.c=%.o)

INC_FLAGS := -I$(REL_INC)
CFLAGS += -Wall -g -lpthread $(INC_FLAGS)
CFLAGS += $(LIBS_CFLAGS)
CFLAGS += -ldl

LIBS += $(REL_LIB)/lib_hiae.a
LIBS += $(REL_LIB)/libisp.a

LIBS += $(REL_LIB)/lib_hiawb.a
LIBS += $(REL_LIB)/lib_hiaf.a
LIBS += $(REL_LIB)/lib_hidefog.a

LIBS += $(REL_LIB)/libmpi.a
CFLAGS += -lm

include $(PWD)/Make.$(OSTYPE)
