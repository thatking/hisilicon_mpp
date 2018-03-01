# Hisilicon Hi3516CV300 sample Makefile


include ../Makefile.param

SRCS += $(wildcard *.c ./src/common/*.c) 
SRCS += $(wildcard ./src/iniparser/*.c)
SRCS += $(wildcard ./src/adapt/$(HIARCH)/*.c) 

CFLAGS +=-I./src/iniparser
CFLAGS +=-I$(COMMON_DIR)/../scene_auto/src/include

TARGET  := sample_scene

# compile linux or HuaweiLite
include $(PWD)/../Make.$(OSTYPE)

