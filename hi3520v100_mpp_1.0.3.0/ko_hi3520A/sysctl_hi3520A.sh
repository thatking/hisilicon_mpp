#!/bin/sh

# mddrc0 pri&timeout setting
himm 0x20110150  0x03ff6         #DMA1 DMA2
himm 0x20110154  0x03ff6         #ETH TOE
himm 0x20110158  0x03ff6         #SCD CIPHER 
himm 0x2011015c  0x03ff6         #SDIO NANDC
himm 0x20110160  0x03ff6         #PCIE
himm 0x20110164  0x03ff6         #SATA USB
himm 0x20110168  0x03ff5         #A9
himm 0x2011016c  0x03ff6         #JPGD JPGE
himm 0x20110170  0x03ff6         #IVE, MD, DDR_TEST
himm 0x20110174  0x03ff6         #VOIE
himm 0x20110178  0x03ff3         #VDH
himm 0x2011017c  0x10c02         #VEDU
#himm 0x2011017c  0x10c82         #VEDU ESL
himm 0x20110180  0x03ff4         #VCMP TDE0 TDE1
himm 0x20110184  0x03ff2         #VPSS
himm 0x20110188  0x10101         #VICAP FPGA
#himm 0x20110188  0x10640         #VICAP ESL
himm 0x2011018c  0x10200         #VDP

#mddrc order control idmap_mode
#himm 0x20110100 0xe7      #mddrc order enable mddrc idmap mode select
himm 0x20110100 0x6b      #mddrc order enable mddrc idmap mode select
#himm 0x20110020 0x784     #双ddr操作挂死问题规避

himm 0x200500d8 0x3             #DDR0只使能VICAP和VDP乱序

#outstanding
#vdp, 2  W:[19:16];R:[7:4]
###himm 0x205CCE00 0x80021220
#vicap, 2 master1:[19:16];master0:[3:0],21只有m0
###himm 0x20580004 0x00020002
#vpss0, 初始值:2 - 1 = 1  W:[7:4];R:[3:0]
himm 0x20600314 0x00640011
#venc0, 4 - 1 = 3 [2:0]
himm 0x206200A4 0x3
#tde0, 2 R:[23:20];W:[19:16]
himm 0x20610844 0x80220000
#tde1, 2
himm 0x20611844 0x80220000
#vdh0 6 - 1 = 5 R:[15:12];W:[11:8]用李中陪
###himm 0x20630030 0x00012200
#jpge [2:0]
himm 0x206600a4 0x3
#vcmp W:[7:4];R:[3:0]，用例的设置需要注释掉，且不能为0
himm 0x206b0004 0x11
#mdu [2:0] ，用例中配
###himm 0x206c002c 0x7
#voie [2:0],用例中的设置需要注释掉；不可读
himm 0x20640014 0x7
#ive NO!
#sio NO!
#jpgd NO!


himm  0x20580004 0xf0f00002      #vicap outstanding  在脚本中配置      viu_hal.c                          
himm 0x205CCE00 0x80021220       #vou outstanding    在脚本中配置      vou_drv.c VOU_DRV_DefaultSetting   
himm 0x20600314 0x01640011       #VPSS outstanding   在脚本中配置                                         
himm 0x206200A4 0x3              #vedu outstanding   在脚本中配置      vedu_drv.c                         
himm 0x206600a4 0x1              #JPEGU outstanding  在脚本中配置      jpeg_drv.c                         
himm 0x20640014 0x2              #void outstanding    void_hal.c                                          
#himm 0x206b0004 0x11            #vcmp outstanding    在代码中配置                                        
#himm 0x20610844 0x80220000      #tde outstanding     在代码中配置                                        
#himm 0x20610844 0x80220000      #tde outstanding     在代码中配置      tde_hal.c                         
#himm 0x20630030 0x00012200      #vdh outstanding     在代码中配置                                       
himm 0x206c002c 0x7              #MDU outstanding    原代码中没有配置                                    
