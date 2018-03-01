
#include <stdio.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "strfunc.h"
#include "i2c.h"


int main(int argc , char* argv[])
{
	int fd = -1;
	int ret = 0;
	I2C_DATA_S i2c_data;	
	unsigned int device_addr, reg_addr, reg_value;
		
	if(argc != 4)
    {
    	printf("usage: %s <device_addr> <reg_addr> <value>. sample: %s 0x56 0x0 0x28\n", argv[0], argv[0]);
        return -1;
    }
    
    if (StrToNumber(argv[1], &device_addr))
    {    	
    	return 0;
    }
    
    if (StrToNumber(argv[2], &reg_addr))
    {    
    	return 0;
    }
    
    if (StrToNumber(argv[3], &reg_value))
    {    
    	return 0;
    }
	
	fd = open("/dev/i2c", 0);
    if(fd < 0)
    {
    	printf("Open i2c device error!\n");
    	return -1;
    }

    i2c_data.dev_addr = device_addr;
    i2c_data.reg_addr = reg_addr;
    i2c_data.addr_byte = 1;
    i2c_data.data = reg_value;
    i2c_data.data_byte = 1;
    
    printf("write: device_addr:0x%2x; reg_addr:0x%2x; reg_value:0x%2x.\n", device_addr, reg_addr, reg_value);
    
    ret = ioctl(fd, I2C_CMD_WRITE, &i2c_data);
    if(ret)
    {  
        printf("i2c write failed!\n");
        return -1 ;
    }

	close(fd);
       
    return 0;
}
