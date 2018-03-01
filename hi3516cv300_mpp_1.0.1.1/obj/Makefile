#*******************************************************************************
#The rule of make Linux kernel module
#*******************************************************************************
ifeq ($(PARAM_FILE), ) 
    PARAM_FILE:=../Makefile.param
    include $(PARAM_FILE)
endif

EXTRA_CFLAGS += $(MPP_CFLAGS)
EXTRA_CFLAGS += -I$(REL_INC)
EXTRA_CFLAGS += -I$(OSAL_ROOT)/include

$(HIARCH)_base-objs = hi_base.o ../init/base_init.o
$(HIARCH)_sys-objs  = hi_sys.o ../init/sys_init.o
$(HIARCH)_viu-objs  = hi_viu.o ../init/viu_init.o
$(HIARCH)_vou-objs  = hi_vou.o ../init/vou_init.o
$(HIARCH)_vpss-objs = hi_vpss.o ../init/vpss_init.o
$(HIARCH)_vgs-objs  = hi_vgs.o ../init/vgs_init.o
$(HIARCH)_chnl-objs = hi_chnl.o ../init/chnl_init.o
$(HIARCH)_region-objs = hi_region.o ../init/rgn_init.o
$(HIARCH)_rc-objs     = hi_rc.o ../init/rc_init.o
$(HIARCH)_venc-objs   = hi_venc.o ../init/venc_init.o
$(HIARCH)_h264e-objs  = hi_h264e.o ../init/h264e_init.o
$(HIARCH)_h265e-objs  = hi_h265e.o ../init/h265e_init.o
$(HIARCH)_jpege-objs  = hi_jpege.o ../init/jpege_init.o
$(HIARCH)_vedu-objs  = hi_vedu.o ../init/vedu_init.o
$(HIARCH)_isp-objs  = hi_isp.o ../init/isp_init.o
$(HIARCH)_ive-objs  = hi_ive.o ../init/ive_init.o
$(HIARCH)_ai-objs     = hi_ai.o ../init/ai_init.o
$(HIARCH)_ao-objs   = hi_ao.o ../init/ao_init.o
$(HIARCH)_aenc-objs  = hi_aenc.o ../init/aenc_init.o
$(HIARCH)_adec-objs  = hi_adec.o ../init/adec_init.o
$(HIARCH)_aio-objs  = hi_aio.o ../init/aiao_init.o
hi_acodec-objs = acodec.o ../init/acodec_init.o

obj-m += $(HIARCH)_base.o
obj-m += $(HIARCH)_sys.o
obj-m += $(HIARCH)_viu.o
obj-m += $(HIARCH)_vou.o
obj-m += $(HIARCH)_vpss.o
obj-m += $(HIARCH)_vgs.o
obj-m += $(HIARCH)_region.o
obj-m += $(HIARCH)_chnl.o
obj-m += $(HIARCH)_rc.o
obj-m += $(HIARCH)_venc.o
obj-m += $(HIARCH)_h264e.o
obj-m += $(HIARCH)_h265e.o
obj-m += $(HIARCH)_jpege.o
obj-m += $(HIARCH)_vedu.o
obj-m += $(HIARCH)_isp.o
obj-m += $(HIARCH)_ive.o
obj-m += $(HIARCH)_ai.o
obj-m += $(HIARCH)_ao.o
obj-m += $(HIARCH)_aenc.o
obj-m += $(HIARCH)_adec.o
obj-m += $(HIARCH)_aio.o
obj-m += hi_acodec.o

default:
	@echo -e "\e[0;32;1m--Compiling '$(TARGET)'... Configs as follow:\e[0;36;1m"
	@echo ---- CROSS=$(CROSS)
	@echo ---- HIARCH=$(HIARCH), HICHIP=$(HICHIP), CVER=$(CVER), DBG=$(HIDBG), HI_FPGA=$(HI_FPGA)
	@echo ---- CPU_TYPE=$(CPU_TYPE)
	@echo ---- MPP_CFLAGS=$(MPP_CFLAGS)
	@echo ---- SDK_PATH=$(SDK_PATH) , PARAM_FILE=$(PARAM_FILE)
	@echo ---- KERNEL_ROOT=$(KERNEL_ROOT)
	@echo ---- ARCH_ROOT=$(ARCH_ROOT), ARCH_HAL=$(ARCH_HAL)
	@@echo -e "\e[0m"
	@$(MAKE) -C $(KERNEL_ROOT) M=$(PWD) modules
	@mkdir -p $(REL_KO) && cp -rf *.ko $(REL_KO)

clean:
	@rm -f *.ko
	@rm .tmp_versions  -rf
	@rm Module.symvers -f
	@rm modules.order  -f
	@rm .*.cmd  -f
	@rm *.mod.c  -f
	@rm hi*.o  -f
	@rm ../init/*.o  -f
	@rm ../init/.*.cmd  -f
