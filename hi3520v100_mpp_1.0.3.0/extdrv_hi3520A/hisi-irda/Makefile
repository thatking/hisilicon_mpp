
ifeq ($(PARAM_FILE), )
		PARAM_FILE:=../../Makefile.param
		include $(PARAM_FILE)
endif
obj-m := hi_ir.o
hi_ir-y += hiir.o hiir_codedef.o

#EXTRA_CFLAGS += -D$(HI_FPGA)
#EXTRA_CFLAGS+=-I$(PWD)/../gpio-i2c

default:
	@make -C $(LINUX_ROOT) M=$(PWD) modules 
	#cp sil9024.ko ../../../user 
clean: 
	@make -C $(LINUX_ROOT) M=$(PWD) clean 
	#rm ../../../user/sil9024.ko



