ifeq ($(PARAM_FILE), )
		PARAM_FILE:=../../Makefile.param
		include $(PARAM_FILE)
endif

obj-m := hi_rtc.o
hirtc-y += hi_rtc.o 


EXTRA_CFLAGS+=-I$(PWD)/

default:	
	@$(MAKE) -C $(LINUX_ROOT) M=$(PWD) modules 

clean:
	@make -C $(LINUX_ROOT) M=$(PWD) clean 

