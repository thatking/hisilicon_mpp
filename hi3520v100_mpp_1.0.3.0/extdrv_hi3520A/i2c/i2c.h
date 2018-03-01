#ifndef __HI_I2C_H__
#define __HI_I2C_H__

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */


typedef struct hiI2C_DATA_S
{
	unsigned char dev_addr; 
	unsigned int reg_addr; 
	unsigned int addr_byte; 
	unsigned int data; 
	unsigned int data_byte;
    
} I2C_DATA_S;

#define I2C_CMD_WRITE      0x01
#define I2C_CMD_READ       0x03


unsigned int i2c_write(unsigned char dev_addr, unsigned int reg_addr, unsigned int data);
unsigned int i2c_read(unsigned char dev_addr, unsigned int reg_addr);

unsigned int i2c_write_ex(unsigned char dev_addr, unsigned int reg_addr, unsigned int addr_byte, unsigned int data, unsigned int data_byte);
unsigned int i2c_read_ex(unsigned char dev_addr, unsigned int reg_addr, unsigned int addr_byte, unsigned int data_byte);


#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif	/* __HI_I2C_H__ */


