# $(ROOT)/mpp/Makefile

ifeq ($(PARAM_FILE),) 
    PARAM_FILE:=./Makefile.param
    include $(PARAM_FILE)
endif

.PHONY:all prepare obj drv clean
 
all: prepare obj drv

prepare:
	@cd $(SDK_PATH)/osal/source/kernel; make

drv:
	@cd $(SDK_PATH)/drv/extdrv;   make
	@cd $(SDK_PATH)/drv/interdrv; make

obj:
	@cd $(REL_OBJ); make


clean:
	@cd $(SDK_PATH)/osal/source/kernel; make clean
	@cd $(SDK_PATH)/drv/interdrv;       make clean
	@cd $(SDK_PATH)/drv/extdrv;         make clean
	
