ifeq ($(PARAM_FILE), ) 
    PARAM_FILE:=../../../Makefile.param
    include $(PARAM_FILE)
endif

EXTDRV_KO=$(REL_KO)/extdrv

.PHONY:clean all rel
all:
	@echo -e "\e[0;32;1m--Compiling 'extdrv'... Configs as follow:\e[0;36;1m"
	@echo ---- CROSS=$(CROSS)
	@echo ---- HIARCH=$(HIARCH), HICHIP=$(HICHIP), CVER=$(CVER), HIDBG=$(HIDBG)
	@echo ---- SDK_PATH=$(SDK_PATH) , PARAM_FILE=$(PARAM_FILE)
	@echo ---- KERNEL_ROOT=$(KERNEL_ROOT)
	@echo -e "\e[0m"
	@mkdir -p $(EXTDRV_KO)
	@for x in `find ./ -maxdepth 2 -mindepth 2 -name "Makefile" `; do\
	   { cd `dirname $$x`; if [ $$? ]; then make || exit 1;  cd ../; fi;}& done; wait

clean:
	@for x in `find ./ -maxdepth 2 -mindepth 2 -name "Makefile" `; do\
	   { cd `dirname $$x`; if [ $$? ]; then make clean; cd ../; fi;}& done; wait
	@if [ -e $(REL_KO) ];then rmdir --ignore-fail-on-non-empty "$(REL_KO)"; fi

