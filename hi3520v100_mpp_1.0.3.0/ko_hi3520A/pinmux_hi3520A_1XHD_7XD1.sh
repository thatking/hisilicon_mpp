#pin select  muxctrl_reg0
himm 0x200f0000 0x00000000 #VIU0_CLK  / GPIO12_0 /            / muxctrl_reg0
himm 0x200f0004 0x00000000 #VIU0_VS   / GPIO12_2 / VIU0_CLKA  / muxctrl_reg1
himm 0x200f0008 0x00000000 #VIU0_HS   / GPIO12_3 / UART2_RXD  / muxctrl_reg2
himm 0x200f000c 0x00000000 #VIU0_DAT15/ GPIO0_0  /            / muxctrl_reg3
himm 0x200f0010 0x00000000 #VIU0_DAT14/ GPIO0_1  /            / muxctrl_reg4
himm 0x200f0014 0x00000000 #VIU0_DAT13/ GPIO0_2  /            / muxctrl_reg5
himm 0x200f0018 0x00000000 #VIU0_DAT12/ GPIO0_3  /            / muxctrl_reg6
himm 0x200f001c 0x00000000 #VIU0_DAT11/ GPIO0_4  /            / muxctrl_reg7
himm 0x200f0020 0x00000000 #VIU0_DAT10/ GPIO0_5  /            / muxctrl_reg8
himm 0x200f0024 0x00000000 #VIU0_DAT9 / GPIO0_6  /            / muxctrl_reg9
himm 0x200f0028 0x00000000 #VIU0_DAT8 / GPIO0_7  /            / muxctrl_reg10
himm 0x200f002c 0x00000000 #VIU0_DAT7 / GPIO1_0  /            / muxctrl_reg11
himm 0x200f0030 0x00000000 #VIU0_DAT6 / GPIO1_1  /            / muxctrl_reg12
himm 0x200f0034 0x00000000 #VIU0_DAT5 / GPIO1_2  /            / muxctrl_reg13
himm 0x200f0038 0x00000000 #VIU0_DAT4 / GPIO1_3  /            / muxctrl_reg14
himm 0x200f003c 0x00000000 #VIU0_DAT3 / GPIO1_4  /            / muxctrl_reg15
himm 0x200f0040 0x00000000 #VIU0_DAT2 / GPIO1_5  /            / muxctrl_reg16
himm 0x200f0044 0x00000000 #VIU0_DAT1 / GPIO1_6  /            / muxctrl_reg17
himm 0x200f0048 0x00000000 #VIU0_DAT0 / GPIO1_7  /            / muxctrl_reg18

#VIU1 muxctrl_reg19
himm 0x200f004c 0x00000000 #VIU1_CLK  / GPIO12_1 /            / muxctrl_reg19
himm 0x200f0050 0x00000002 #VIU1_VS   / GPIO12_4 / VIU1_CLKA  / muxctrl_reg20
himm 0x200f0054 0x00000000 #VIU1_HS   / GPIO12_5 / UART2_TXD  / muxctrl_reg21
himm 0x200f0058 0x00000000 #VIU1_DAT15/ GPIO2_0  /            / muxctrl_reg22
himm 0x200f005c 0x00000000 #VIU1_DAT14/ GPIO2_1  /            / muxctrl_reg23
himm 0x200f0060 0x00000000 #VIU1_DAT13/ GPIO2_2  /            / muxctrl_reg24
himm 0x200f0064 0x00000000 #VIU1_DAT12/ GPIO2_3  /            / muxctrl_reg25
himm 0x200f0068 0x00000000 #VIU1_DAT11/ GPIO2_4  /            / muxctrl_reg26
himm 0x200f006c 0x00000000 #VIU1_DAT10/ GPIO2_5  /            / muxctrl_reg27
himm 0x200f0070 0x00000000 #VIU1_DAT9 / GPIO2_6  /            / muxctrl_reg28
himm 0x200f0074 0x00000000 #VIU1_DAT8 / GPIO2_7  /            / muxctrl_reg29
himm 0x200f0078 0x00000000 #VIU1_DAT7 / GPIO3_0  /            / muxctrl_reg30
himm 0x200f007c 0x00000000 #VIU1_DAT6 / GPIO3_1  /            / muxctrl_reg31
himm 0x200f0080 0x00000000 #VIU1_DAT5 / GPIO3_2  /            / muxctrl_reg32
himm 0x200f0084 0x00000000 #VIU1_DAT4 / GPIO3_3  /            / muxctrl_reg33
himm 0x200f0088 0x00000000 #VIU1_DAT3 / GPIO3_4  /            / muxctrl_reg34
himm 0x200f008c 0x00000000 #VIU1_DAT2 / GPIO3_5  /            / muxctrl_reg35
himm 0x200f0090 0x00000000 #VIU1_DAT1 / GPIO3_6  /            / muxctrl_reg36
himm 0x200f0094 0x00000000 #VIU1_DAT0 / GPIO3_7  /            / muxctrl_reg37

#VOU1120_CLK internal sync
himm 0x200f0098 0x00000001 #GPIO12_6  / VOU1120_CLK  / LCD_CLK     / muxctrl_reg38 
himm 0x200f009c 0x00000001 #GPIO4_0   / VOU1120_DAT15/ UART3_TXD   / muxctrl_reg39
himm 0x200f00a0 0x00000001 #GPIO4_1   / VOU1120_DAT14/ LCD_DATA_G1 / muxctrl_reg40
himm 0x200f00a4 0x00000001 #GPIO4_2   / VOU1120_DAT13/ LCD_DATA_G2 / muxctrl_reg41
himm 0x200f00a8 0x00000001 #GPIO4_3   / VOU1120_DAT12/ LCD_DATA_G3 / muxctrl_reg42
himm 0x200f00ac 0x00000001 #GPIO4_4   / VOU1120_DAT11/ LCD_DATA_G4 / muxctrl_reg43
himm 0x200f00b0 0x00000001 #GPIO4_5   / VOU1120_DAT10/ LCD_DATA_G5 / muxctrl_reg44
himm 0x200f00b4 0x00000001 #GPIO4_6   / VOU1120_DAT9 / LCD_DATA_G6 / muxctrl_reg45
himm 0x200f00b8 0x00000001 #GPIO4_7   / VOU1120_DAT8 / LCD_DATA_G7 / muxctrl_reg46
himm 0x200f00bc 0x00000001 #GPIO5_0   / VOU1120_DAT7 / LCD_DATA_B0 / muxctrl_reg47
himm 0x200f00c0 0x00000001 #GPIO5_1   / VOU1120_DAT6 / LCD_DATA_B1 / muxctrl_reg48
himm 0x200f00c4 0x00000001 #GPIO5_2   / VOU1120_DAT5 / LCD_DATA_B2 / muxctrl_reg49
himm 0x200f00c8 0x00000001 #GPIO5_3   / VOU1120_DAT4 / LCD_DATA_B3 / muxctrl_reg50
himm 0x200f00cc 0x00000001 #GPIO5_4   / VOU1120_DAT3 / LCD_DATA_B4 / muxctrl_reg51
himm 0x200f00d0 0x00000001 #GPIO5_5   / VOU1120_DAT2 / LCD_DATA_B5 / muxctrl_reg52
himm 0x200f00d4 0x00000001 #GPIO5_6   / VOU1120_DAT1 / LCD_DATA_B6 / muxctrl_reg53
himm 0x200f00d8 0x00000001 #GPIO5_7   / VOU1120_DAT0 / LCD_DATA_B7 / muxctrl_reg54

#VGA 
himm 0x200f00dc 0x00000002 #GPIO13_0  / VOU1120_HS   / VGA_HS      / muxctrl_reg55
himm 0x200f00e0 0x00000002 #GPIO13_1  / VOU1120_VS   / VGA_VS      / muxctrl_reg56

#BT656
himm 0x200f00e4 0x00000001 #GPIO12_7  / VOU656_CLK  /  reserved   / SDIO_CCLK_OUT      / muxctrl_reg57
himm 0x200f00e8 0x00000001 #GPIO6_0   / VOU656_DAT7 / LCD_DATA_R0 / SDIO_CARD_POWER_EN / muxctrl_reg58
himm 0x200f00ec 0x00000001 #GPIO6_1   / VOU656_DAT6 / LCD_DATA_R1 / SDIO_CARD_DETECT   / muxctrl_reg59
himm 0x200f00f0 0x00000001 #GPIO6_2   / VOU656_DAT5 / LCD_DATA_R2 / SDIO_CWPR          / muxctrl_reg60
himm 0x200f00f4 0x00000001 #GPIO6_3   / VOU656_DAT4 / LCD_DATA_R3 / SDIO_CCMD          / muxctrl_reg61
himm 0x200f00f8 0x00000001 #GPIO6_4   / VOU656_DAT3 / LCD_DATA_R4 / SDIO_CDATA3        / muxctrl_reg62
himm 0x200f00fc 0x00000001 #GPIO6_5   / VOU656_DAT2 / LCD_DATA_R5 / SDIO_CDATA2        / muxctrl_reg63
himm 0x200f0100 0x00000001 #GPIO6_6   / VOU656_DAT1 / LCD_DATA_R6 / SDIO_CDATA1        / muxctrl_reg64
himm 0x200f0104 0x00000001 #GPIO6_7   / VOU656_DAT0 / LCD_DATA_R7 / SDIO_CDATA0        / muxctrl_reg65

#SIO0
himm 0x200f0108 0x00000001 #GPIO7_0   / SIO0_RCLK / muxctrl_reg66
himm 0x200f010c 0x00000001 #GPIO7_1   / SIO0_RFS  / muxctrl_reg67
himm 0x200f0110 0x00000001 #GPIO7_2   / SIO0_DIN  / muxctrl_reg68

#SIO1
himm 0x200f0114 0x00000001 #GPIO7_3   / SIO1_RCLK / muxctrl_reg69
himm 0x200f0118 0x00000001 #GPIO7_4   / SIO1_RFS  / muxctrl_reg70
himm 0x200f011C 0x00000001 #GPIO7_5   / SIO1_DIN  / muxctrl_reg71

#SIO2
himm 0x200f0120 0x00000001 #GPIO7_6   / SIO2_XCLK / muxctrl_reg72
himm 0x200f0124 0x00000001 #GPIO7_7   / SIO2_XFS  / muxctrl_reg73
himm 0x200f0128 0x00000001 #GPIO8_0   / SIO2_RCLK / muxctrl_reg74
himm 0x200f012c 0x00000001 #GPIO8_1   / SIO2_RFS  / muxctrl_reg75
himm 0x200f0130 0x00000001 #GPIO8_2   / SIO2_DIN  / muxctrl_reg76

#SPI
himm 0x200f0134 0x00000001 #GPIO8_3   / SPI_SCLK  / muxctrl_reg77
himm 0x200f0138 0x00000001 #GPIO8_4   / SPI_SDO   / muxctrl_reg78
himm 0x200f013c 0x00000001 #GPIO8_5   / SPI_SDI   / muxctrl_reg79
himm 0x200f0140 0x00000001 #GPIO8_6   / SPI_CSN0  / muxctrl_reg80
#himm 0x200f0144 0x00000000 #SPI_CSN3  / NF_BOOT_PIN4  / CLK_TEST_OUT0/1/2/3

#I2C
#himm 0x200f0148 0x00000001 #GPIO13_2  / I2C_SDA   / muxctrl_reg82
#himm 0x200f014c 0x00000001 #GPIO13_3  / I2C_SCL   / muxctrl_reg83

#HDMI pinmux -uboot
#himm 0x200f01cc 0x00000001  #GPIO11_4      / HDMI_HOTPLUG/ muxctrl_reg115
#himm 0x200f01d0 0x00000001  #GPIO11_5      / HDMI_CEC    / muxctrl_reg116
#himm 0x200f01d4 0x00000001  #GPIO11_6      / HDMI_SDA    / muxctrl_reg117
#himm 0x200f01d8 0x00000001  #GPIO11_7      / HDMI_SCL    / muxctrl_reg118








