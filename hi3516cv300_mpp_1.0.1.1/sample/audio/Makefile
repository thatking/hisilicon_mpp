# Hisilicon Hi35xx sample Makefile

include ../Makefile.param

SRCS := $(wildcard *.c)
TARGET := $(SRCS:%.c=%)

# compile linux or HuaweiLite
include $(PWD)/../Make.$(OSTYPE)
