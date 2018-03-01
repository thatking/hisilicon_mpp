/*  extdrv/interface/i2c.c
 *
 * Copyright (c) 2006 Hisilicon Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 * History:
 *      26-Jan-2011 create this file
 */

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/fcntl.h>

#include <linux/init.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>

#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/io.h>

#include "i2c.h"

#define READ_OPERATION     (1)
#define WRITE_OPERATION    0xfe


/* I2C_CTRL_REG */
#define I2C_ENABLE             (1 << 8)
#define I2C_UNMASK_TOTAL       (1 << 7)
#define I2C_UNMASK_START       (1 << 6)
#define I2C_UNMASK_END         (1 << 5)
#define I2C_UNMASK_SEND        (1 << 4)
#define I2C_UNMASK_RECEIVE     (1 << 3)
#define I2C_UNMASK_ACK         (1 << 2)
#define I2C_UNMASK_ARBITRATE   (1<< 1)
#define I2C_UNMASK_OVER        (1 << 0)
#define I2C_UNMASK_ALL         (I2C_UNMASK_START | I2C_UNMASK_END | \
                                I2C_UNMASK_SEND | I2C_UNMASK_RECEIVE | \
                                I2C_UNMASK_ACK | I2C_UNMASK_ARBITRATE | \
                                I2C_UNMASK_OVER)

/* I2C_COM_REB */
#define I2C_SEND_ACK (~(1 << 4))
#define I2C_START (1 << 3)
#define I2C_READ (1 << 2)
#define I2C_WRITE (1 << 1)
#define I2C_STOP (1 << 0)

/* I2C_ICR_REG */
#define I2C_CLEAR_START (1 << 6)
#define I2C_CLEAR_END (1 << 5)
#define I2C_CLEAR_SEND (1 << 4)
#define I2C_CLEAR_RECEIVE (1 << 3)
#define I2C_CLEAR_ACK (1 << 2)
#define I2C_CLEAR_ARBITRATE (1 << 1)
#define I2C_CLEAR_OVER (1 << 0)
#define I2C_CLEAR_ALL (I2C_CLEAR_START | I2C_CLEAR_END | \
                       I2C_CLEAR_SEND | I2C_CLEAR_RECEIVE | \
                       I2C_CLEAR_ACK | I2C_CLEAR_ARBITRATE | \
                       I2C_CLEAR_OVER)

/* I2C_SR_REG */
#define I2C_BUSY (1 << 7)
#define I2C_START_INTR (1 << 6)
#define I2C_END_INTR (1 << 5)
#define I2C_SEND_INTR (1 << 4)
#define I2C_RECEIVE_INTR (1 << 3)
#define I2C_ACK_INTR (1 << 2)
#define I2C_ARBITRATE_INTR (1 << 1)
#define I2C_OVER_INTR (1 << 0)


#define I2C_WAIT_TIME_OUT       0x1000 
#define I2C_ADRESS_BASE         0x200D0000

#ifdef HI_FPGA
#define I2C_DFT_CLK            (50000000)
#else
#define I2C_DFT_CLK            (110000000)
#endif

#define I2C_DFT_RATE           (100000)

void __iomem *reg_i2c_base_va = 0;
#define HI_IO_ADDRESS(x) (reg_i2c_base_va + ((x)-(I2C_ADRESS_BASE)))

#define I2C_CTRL_REG      HI_IO_ADDRESS(I2C_ADRESS_BASE + 0x000)
#define I2C_COM_REB       HI_IO_ADDRESS(I2C_ADRESS_BASE + 0x004)
#define I2C_ICR_REG       HI_IO_ADDRESS(I2C_ADRESS_BASE + 0x008)
#define I2C_SR_REG        HI_IO_ADDRESS(I2C_ADRESS_BASE + 0x00C)
#define I2C_SCL_H_REG     HI_IO_ADDRESS(I2C_ADRESS_BASE + 0x010)
#define I2C_SCL_L_REG     HI_IO_ADDRESS(I2C_ADRESS_BASE + 0x014)
#define I2C_TXR_REG       HI_IO_ADDRESS(I2C_ADRESS_BASE + 0x018)
#define I2C_RXR_REG       HI_IO_ADDRESS(I2C_ADRESS_BASE + 0x01C)


#define  I2C_WRITE_REG(Addr, Value) ((*(volatile unsigned int *)(Addr)) = (Value))
#define  I2C_READ_REG(Addr)         (*(volatile unsigned int *)(Addr))

spinlock_t  gpioi2c_lock;

static void I2C_DRV_SetRate(unsigned int I2cRate)
{
    unsigned int  Value = 0;
    unsigned int  SclH = 0;
    unsigned int  SclL = 0;


    /* Read CTRL */
    Value = I2C_READ_REG(I2C_CTRL_REG);

    /* maskable interrupt of i2c */
    I2C_WRITE_REG(I2C_CTRL_REG, (Value & (~I2C_UNMASK_TOTAL)));

    SclH = (I2C_DFT_CLK / (I2cRate * 2)) / 2 - 1;
    I2C_WRITE_REG(I2C_SCL_H_REG, SclH);

    SclL = (I2C_DFT_CLK / (I2cRate * 2)) / 2 - 1;
    I2C_WRITE_REG(I2C_SCL_L_REG, SclL);

    /* enable interrupt of i2c */
    I2C_WRITE_REG(I2C_CTRL_REG, Value);

    return;
}

int I2C_DRV_WaitWriteEnd(unsigned int I2cNum)
{
    unsigned int  I2cSrReg;
    unsigned int  i = 0;

	do
	{
        I2cSrReg = I2C_READ_REG(I2C_SR_REG);

        if (i > I2C_WAIT_TIME_OUT)
        {
            //printk("wait write data timeout!\n");
            return -1;
        }
        i++;
	}while((I2cSrReg & I2C_OVER_INTR) != I2C_OVER_INTR);

  I2C_WRITE_REG(I2C_ICR_REG, I2C_CLEAR_ALL);

	return 0;
}

int I2C_DRV_WaitRead(unsigned int I2cNum)
{
    unsigned int  I2cSrReg;
    unsigned int  i = 0;

	do
	{
		I2cSrReg = I2C_READ_REG(I2C_SR_REG);

		if (i > I2C_WAIT_TIME_OUT)
        {
            //printk("wait write data timeout!\n");
            return -1;
        }
        i++;
	}while((I2cSrReg & I2C_RECEIVE_INTR) != I2C_RECEIVE_INTR);

	return 0;
}


/*
add by Jiang Lei 2010-08-24
i2C write confirm function.
it use for e2prom device. 
e2prom:  set stop bit -> write cycle -> i2c master confirm -> next ....
*/
int I2C_DRV_WriteConfig(char I2cDevAddr)
{
    unsigned int          i = 0;
    unsigned int          j = 0;
    unsigned int          I2cSrReg;

    do
    {
        I2C_WRITE_REG(I2C_TXR_REG,(I2cDevAddr & WRITE_OPERATION));
        I2C_WRITE_REG(I2C_COM_REB, (I2C_WRITE | I2C_START));

        j = 0;
    	do
    	{
            I2cSrReg = I2C_READ_REG(I2C_SR_REG);

            if (j > I2C_WAIT_TIME_OUT)
            {
                //printk("wait write data timeout!\n");
                return -1;
            }
            j++;
    	}while((I2cSrReg & I2C_OVER_INTR) != I2C_OVER_INTR);

        I2cSrReg = I2C_READ_REG( I2C_SR_REG);
        I2C_WRITE_REG(I2C_ICR_REG, I2C_CLEAR_ALL);

        i++;

        if (i > 0x200000) //I2C_WAIT_TIME_OUT)
        {
            printk("wait write ack ok timeout!\n");
            return -1;
        }
    }while ((I2cSrReg & I2C_ACK_INTR));

    return 0;
}

unsigned int I2C_DRV_Write(unsigned int I2cNum, char I2cDevAddr, unsigned int I2cRegAddr, unsigned int I2cRegAddrByteNum, unsigned int Data, unsigned int DataLen)
{
    unsigned int   i       ;
    unsigned int   RegAddr ;
    unsigned int   TXR_data;
    
    spin_lock(&gpioi2c_lock);

    /* clear interrupt flag */
    //I2C_WRITE_REG(I2C_ICR_REG, I2C_CLEAR_ALL);//08,03

    I2C_WRITE_REG(I2C_ICR_REG , 0x03 );//08,03
    I2C_WRITE_REG(I2C_CTRL_REG, 0x187);//0x00
    /* send the device addr */
    I2C_WRITE_REG(I2C_TXR_REG, (I2cDevAddr & WRITE_OPERATION));
    I2C_WRITE_REG(I2C_COM_REB, (I2C_WRITE | I2C_START));//04,0a

    if (I2C_DRV_WaitWriteEnd(I2cNum))
    {
        //printk("write devicAdd in write process!\n");
    }

    /* send reg addr */
    for(i=0; i<I2cRegAddrByteNum; i++)
    {
        RegAddr = I2cRegAddr >> ((I2cRegAddrByteNum -i -1) * 8);
        I2C_WRITE_REG(I2C_TXR_REG, RegAddr  );//018,register
        I2C_WRITE_REG(I2C_COM_REB, I2C_WRITE);//04,02

        if (I2C_DRV_WaitWriteEnd(I2cNum))
        {
            //local_irq_restore(IntFlag);
            //printk("write write register ok in write process!\n");
        }
        
        //I2C_WRITE_REG(I2C_ICR_REG, I2C_CLEAR_ALL);//08,3f
    }

	/* send data */
	for (i=0; i<DataLen; i++)
	{
      TXR_data = Data >> ((DataLen -i -1) * 8);
      I2C_WRITE_REG(I2C_TXR_REG, TXR_data );
    	I2C_WRITE_REG(I2C_COM_REB, I2C_WRITE);

        if (I2C_DRV_WaitWriteEnd(I2cNum))
        {
            //printk("write register data!\n");
        }
      I2C_WRITE_REG(I2C_ICR_REG, I2C_CLEAR_ALL);//08,3f
	}

	/* send stop flag */
	I2C_WRITE_REG(I2C_COM_REB, I2C_STOP);
    if (I2C_DRV_WaitWriteEnd(I2cNum))
    {
        //printk("wait write process end!\n");
    }
    I2C_WRITE_REG(I2C_ICR_REG, 0x01);

    spin_unlock(&gpioi2c_lock);

    return 0;
}


unsigned int I2C_DRV_Read(unsigned int I2cNum, char I2cDevAddr, unsigned int I2cRegAddr, unsigned int I2cRegAddrByteNum, unsigned int DataLen)
{
    unsigned int    dataTmp = 0xff;
    unsigned int    i             ;
    unsigned int    RegAddr       ;
    unsigned int    Data=0        ;

    spin_lock(&gpioi2c_lock);

    /* clear interrupt flag */
    //I2C_WRITE_REG(I2C_ICR_REG, I2C_CLEAR_ALL);
    I2C_WRITE_REG(I2C_ICR_REG , 0x03 );//08,03
    I2C_WRITE_REG(I2C_CTRL_REG, 0x187);//0x00

    /* send device addr */
    I2C_WRITE_REG(I2C_TXR_REG, (I2cDevAddr & WRITE_OPERATION));
    I2C_WRITE_REG(I2C_COM_REB,(I2C_WRITE | I2C_START));

    if (I2C_DRV_WaitWriteEnd(I2cNum))
    {
        //printk("write first device ok in reading process!\n");
    }
    
    //I2C_WRITE_REG(I2C_ICR_REG, I2C_CLEAR_ALL);//08,3f
    
    /* send reg addr */
    for(i=0; i<I2cRegAddrByteNum; i++)
    {
        RegAddr = I2cRegAddr >> ((I2cRegAddrByteNum -i -1) * 8);
        I2C_WRITE_REG(I2C_TXR_REG, RegAddr  );
        I2C_WRITE_REG(I2C_COM_REB, I2C_WRITE);

        if (I2C_DRV_WaitWriteEnd(I2cNum))
        {
            //printk("write register ok in reading process!\n");
            //return -1;
        }
        //I2C_WRITE_REG(I2C_ICR_REG, I2C_CLEAR_ALL);//08,3f
    }

    /* send data */
    I2C_WRITE_REG(I2C_TXR_REG, (I2cDevAddr | READ_OPERATION));
    I2C_WRITE_REG(I2C_COM_REB, I2C_WRITE | I2C_START);

    if (I2C_DRV_WaitWriteEnd(I2cNum))
    {
        //printk("line :  %d    \n",__LINE__);
        //printk("write second devaddr ok!\n");
    }
    //I2C_WRITE_REG(I2C_ICR_REG, I2C_CLEAR_ALL);//08,3f
    
    
    /* read data */
    for(i=0; i<DataLen; i++)
    {
        /* at the last byte, we don't return ACK */
        if (i == (DataLen - 1))
        {
            I2C_WRITE_REG(I2C_COM_REB, (I2C_READ | (~I2C_SEND_ACK)));
        }
        /* read data & return ack */
        else
        {
            I2C_WRITE_REG(I2C_COM_REB, I2C_READ);
        }

        if (I2C_DRV_WaitRead(I2cNum))
        {
            //local_irq_restore(IntFlag);
            //printk("read data!\n");
        }

        dataTmp = I2C_READ_REG(I2C_RXR_REG);
        Data= Data | (dataTmp<<((DataLen-i-1)*8)) ;

        if (I2C_DRV_WaitWriteEnd(I2cNum))
        {
            //local_irq_restore(IntFlag);
            //printk("wait write data timeout!\n");
        }
       //I2C_WRITE_REG(I2C_ICR_REG, I2C_CLEAR_ALL);//08,3f
    }

    /* send stop flag */
    I2C_WRITE_REG(I2C_COM_REB, I2C_STOP);
    if (I2C_DRV_WaitWriteEnd(I2cNum))
    {
        //local_irq_restore(IntFlag);
        //printk("line:  %d  \n",__LINE__);
        //printk("wait write data timeout!\n");
    }

    //local_irq_restore(IntFlag);
    spin_unlock(&gpioi2c_lock);
    return  Data ;

}


/* export function                                                          */

int i2c_write_config(unsigned char dev_addr)
{
	return I2C_DRV_WriteConfig(dev_addr);
}

unsigned int i2c_write(unsigned char dev_addr, unsigned int reg_addr, unsigned int data)
{
    return I2C_DRV_Write(1, dev_addr, reg_addr, 1, data, 1);
}

unsigned int i2c_read(unsigned char dev_addr, unsigned int reg_addr)
{
    return I2C_DRV_Read(1, dev_addr, reg_addr, 1, 1);
}

unsigned int i2c_write_ex(unsigned char dev_addr, unsigned int reg_addr,  unsigned int addr_byte,  unsigned int data, unsigned int data_byte)
{
    return I2C_DRV_Write(1, dev_addr, reg_addr, addr_byte, data, data_byte);
}

unsigned int i2c_read_ex(unsigned char dev_addr, unsigned int reg_addr,  unsigned int addr_byte,  unsigned int data_byte)
{
    return I2C_DRV_Read(1, dev_addr, reg_addr, addr_byte, data_byte);
}


/* file operation                                                           */

int I2C_Open(struct inode * inode, struct file * file)
{
   return 0 ;

}

int  I2C_Close(struct inode * inode, struct file * file)
{
    return 0;
}

//static int I2C_Ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
static int I2C_Ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    I2C_DATA_S __user *argp = (I2C_DATA_S __user*)arg;
    
    char  devAdd ;
    unsigned int RegAddr; 
    unsigned int Reg_Len;
    unsigned int DataLen;
    unsigned int Wdata  ;
    
    switch (cmd)
    {
        case I2C_CMD_WRITE:
        {
            devAdd = argp->dev_addr;
            RegAddr= argp->reg_addr;
            Reg_Len= argp->addr_byte  ;
            Wdata  = argp->data    ;
            DataLen= argp->data_byte   ;
            
            I2C_DRV_Write(1, devAdd, RegAddr, Reg_Len, Wdata, DataLen);
            break;
        }

        case I2C_CMD_READ:
        {
            devAdd = argp->dev_addr;
            RegAddr= argp->reg_addr;
            Reg_Len= argp->addr_byte  ;
            DataLen= argp->data_byte   ;

            argp->data = I2C_DRV_Read(1, devAdd, RegAddr, Reg_Len, DataLen);
		    break;
        }

        default:
        {
            printk("invalid ioctl command!\n");
            return -ENOIOCTLCMD;
        }
    }

    return 0 ;
}

static struct file_operations i2c_fops = {
    .owner      = THIS_MODULE,
    .unlocked_ioctl= I2C_Ioctl  ,
    .open       = I2C_Open   ,
    .release    = I2C_Close  ,
};

static struct miscdevice i2c_dev = {
   .minor   = MISC_DYNAMIC_MINOR,
   .name    = "i2c"    ,
   .fops    = &i2c_fops,
};


/* module init and exit                                                     */

static int __init i2c_init(void)
{
    int     ret;
    reg_i2c_base_va = ioremap_nocache(I2C_ADRESS_BASE, 0x10000);
    ret = misc_register(&i2c_dev);
    if(ret != 0)
    {
    	printk("register i2c device failed with %#x!\n", ret);
    	return -1;
    }
    
    I2C_DRV_SetRate(I2C_DFT_RATE);

    spin_lock_init(&gpioi2c_lock);
        		
    return 0;    
}

static void __exit i2c_exit(void)
{
    I2C_WRITE_REG(I2C_CTRL_REG, (~I2C_ENABLE));
    iounmap((void*)reg_i2c_base_va);
    misc_deregister(&i2c_dev);
}


EXPORT_SYMBOL(i2c_write_config);
EXPORT_SYMBOL(i2c_write);
EXPORT_SYMBOL(i2c_read);
EXPORT_SYMBOL(i2c_write_ex);
EXPORT_SYMBOL(i2c_read_ex);


module_init(i2c_init);
module_exit(i2c_exit);

MODULE_DESCRIPTION("IIC Driver");
MODULE_AUTHOR("Hisilicon");
MODULE_LICENSE("GPL");

