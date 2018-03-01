ifeq ($(PARAM_FILE), )
        PARAM_FILE:=../../Makefile.param
        include $(PARAM_FILE)
endif

obj-m := adv_7179.o
adv_7179-y += adv7179.o

EXTRA_CFLAGS += -I$(PWD)/ -DHI_GPIO_I2C -D$(HI_FPGA)
EXTRA_CFLAGS += -I$(PWD)/../gpio_i2c_8b

default:
	@make -C $(LINUX_ROOT) M=$(PWD) modules
clean:
	@make -C $(LINUX_ROOT) M=$(PWD) clean
