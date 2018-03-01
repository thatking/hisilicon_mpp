Please read the next instruction depend on os type:linux or HuaweiLite.
linux:
1. modify the file in kernel directory as below: arch/arm/boot/dts/hi3516cv300-demb.dts, and then build uImage.
   a) open config of audio node, which is commented default 

&audio {
	
	pinctrl-names = "default";
	
	pinctrl-0 = <&i2s_with_jtag_pmux &i2s_with_jtag_pconf>;

};

   b) change 'status = "okay";' to 'status = "disabled";' in config of spi_bus1
   c) change '0x113200E0 0xd' to '0x113200E0 0xe' in config of pin_ctrl_ddr

2. modify the file as below: mpp\ko\load3516cv300, and then reload it
   change '#insmod extdrv/hi_tlv320aic31.ko' to 'insmod extdrv/hi_tlv320aic31.ko' 


3. modify the makefile parameter: mpp/sample/Makefile.param from ACODEC_TYPE to  ACODEC_TYPE_TLV320AIC31. 
   It means use the external codec tlv_320aic31 sample code.
	################ select audio codec type for your sample ################
	#ACODEC_TYPE ?= ACODEC_TYPE_INNER
	#external acodec
	ACODEC_TYPE ?= ACODEC_TYPE_TLV320AIC31

4. Rebuild the sample and get the sample_audio.




HuaweiLite:
1.update the file:mpp/init/sdk_init.c
    modify the code in function sysctl_hi3516cv300 as below to use the external audio codec.
    //himm(0x113200E0,0xd);           // internal codec: AIO MCLK out, CODEC AIO TX MCLK
    himm(0x113200E0,0xe);         // external codec: AIC31?¨º?AIO MCLK out, CODEC AIO TX MCLK
    
    modify the code in function audio_mutex_loop_mux to disable mute
    static HI_VOID audio_mutex_loop_mux(void)
    {
        //demo board
	//himm(0x1204008C, 0x0);
	//himm(0x12143400, 0x4);	
	//himm(0x12143010, 0x4);

        //socket board
	himm(0x12040008, 0x0);
	himm(0x12146400, 0x40);	
	himm(0x12146100, 0x40);
    }
    
    modify the code in function pinmux_hi3516cv300 as below to open i2s pin mux.
    vicap_pin_mux(sensor_bus_type);
    i2c1_pin_mux();
    //i2s_with_vi_pin_mux();
    i2s_with_jtag_pin_mux();
    audio_mutex_loop_mux(); 

2. modify the makefile parameter: mpp/sample/Makefile.param£¬from ACODEC_TYPE to  ACODEC_TYPE_TLV320AIC31. 
   It means use the external codec tlv_320aic31 sample code.
	################ select audio codec type for your sample ################
	#ACODEC_TYPE ?= ACODEC_TYPE_INNER
	#external acodec
	ACODEC_TYPE ?= ACODEC_TYPE_TLV320AIC31

3. Rebuild the sample and get the sample_audio.
