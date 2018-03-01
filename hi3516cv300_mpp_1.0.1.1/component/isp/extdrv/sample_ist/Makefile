
ifeq ($(EXTDRV_PARAM_FILE), ) 
    EXTDRV_PARAM_FILE:=../Makefile.param
    include $(EXTDRV_PARAM_FILE)
endif



SRCS := sample_ist.c

INC := -I$(REL_INC)
INC += -I$(shell pwd)/../../firmware/drv
INC += -I$(OSAL_ROOT)/include

EXTDRV_CFLAGS += $(INC)


		
#*************************************************************************
TARGET := hi_sample_ist
PROG := ist_test

#*************************************************************************
# compile linux or HuaweiLite
SUB_BUILD=$(PROG)
SUB_CLEAN=app_clean
include $(PWD)/../Make.$(OSTYPE)

$(PROG):%:%.o 
	$(CC) -g -Wall -o ist_test ist_test.c	strfunc.c
	@rm *.o

app_clean:
	@rm -f ist_test *.bak

