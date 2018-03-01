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
	int ret;
	unsigned int device_addr, reg_addr, reg_value;
	unsigned int addr_len = 1;
	unsigned int data_len = 1;

	I2C_DATA_S i2c_data;
	
	if (argc < 3)
    {
    	printf("usage: %s <device_addr> <reg_addr> (addr_len) (data_len).\n", argv[0]);
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

    if (argc == 4)
    {
	    if (StrToNumber(argv[3], &addr_len))
	    {    
	    	return 0;
	    }
	    addr_len = (addr_len > 2)? 1: addr_len;
    }

    if (argc == 5)
    {
	    if (StrToNumber(argv[4], &data_len))
	    {    
	    	return 0;
	    }
	    data_len = (data_len > 2)? 1: data_len;
    }
    
	fd = open("/dev/i2c", 0);
    if(fd < 0)
    {
    	printf("Open i2c device error!\n");
    	return -1;
    }
    
    i2c_data.dev_addr = device_addr;
    i2c_data.reg_addr = reg_addr;
    i2c_data.addr_byte = addr_len;
    i2c_data.data_byte = data_len;

    ret = ioctl(fd, I2C_CMD_READ, &i2c_data);
    if(ret)
    {  
        printf("i2c read failed!\n");
        return -1 ;
    }

	reg_value = i2c_data.data;

	printf("read: device_addr:0x%x; reg_addr:0x%x; reg_value:0x%x.\n", device_addr, reg_addr, reg_value);

	close(fd);
            
    return 0;
}
