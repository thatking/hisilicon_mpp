
ifeq ($(PARAM_FILE), )
		PARAM_FILE:=../../Makefile.param
		include $(PARAM_FILE)
endif

obj-m := sil9024.o
sil_9024-y += sil9024.o

EXTRA_CFLAGS += -D$(HI_FPGA)
#EXTRA_CFLAGS+=-I$(PWD)/../gpio-i2c

default:
	@make -C $(LINUX_ROOT) M=$(PWD) modules 
	#cp sil9024.ko ../../../user 
clean: 
	@make -C $(LINUX_ROOT) M=$(PWD) clean 
	#rm ../../../user/sil9024.ko



