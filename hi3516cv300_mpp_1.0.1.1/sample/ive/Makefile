# Hisilicon Hi35xx sample Makefile

include ../Makefile.param

SRCS := $(wildcard *.c) 
SRCS += $(wildcard ./sample/*.c)
TARGET := sample_ive_main

# compile linux or HuaweiLite
include $(PWD)/../Make.$(OSTYPE)
