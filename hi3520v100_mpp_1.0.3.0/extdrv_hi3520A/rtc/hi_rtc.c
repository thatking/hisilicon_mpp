/* extdrv/peripheral/rtc/hi_rtc.c
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
 *      10-April-2006 create this file
 */


//#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>

//#include <asm/hardware.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>

#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/rtc.h>
//#include <asm/rtc.h>
#include "hi_rtc.h"


#define RTC_PHY_ADDR            IO_ADDRESS(0x20060000)	         
#define RTC_DR                  RTC_PHY_ADDR + 0x000  
#define RTC_MR                  RTC_PHY_ADDR + 0x004
#define RTC_LR                  RTC_PHY_ADDR + 0x008  
#define RTC_CR                  RTC_PHY_ADDR + 0x00c  
#define RTC_CR_MASK             0x00000001
#define RTC_CR_START            0x00000001
#define RTC_CR_STOP             0x00000000
#define RTC_IMSC                RTC_PHY_ADDR + 0x010  
#define RTC_IMSC_MASK           0x00000001
#define RTC_IMSC_ENABLE         0x00000001
#define RTC_IMSC_DISABLE        0x00000000
#define RTC_RIS                 RTC_PHY_ADDR + 0x014  
#define RTC_RIS_MASK            0x00000001
#define RTC_MIS                 RTC_PHY_ADDR + 0x018  
#define RTC_MIS_MASK            0x00000001
#define RTC_ICR                 RTC_PHY_ADDR + 0x01C  
#define RTC_ICR_MASK            0x00000001
#define RTC_ICR_CLEAR           0x00000001
#define IRQ_RTC                 39
#define RTC_LOCK                RTC_PHY_ADDR + 0x020 
#define RTC_LOCK_VAL		0x1ACCE551
#define SC_PERI_CRG57    IO_ADDRESS(0x200300E4)
spinlock_t  rtc_lock;
static unsigned long 	rtc_status = 0;
static DECLARE_WAIT_QUEUE_HEAD(rtc_wait);


/*
 *	interrupt function
 *	do nothing. left for future
 */

static irqreturn_t rtc_alm_interrupt(int irq, void *dev_id)
{
    	int handled;
    	writel(0x01,RTC_ICR);
    	handled = 1;
    	return IRQ_RETVAL(handled);   
}


/*
 *	converse the data type from year.mouth.data.hour.minite.second to second
 *      define 2000.1.1.0.0.0 as jumping-off point
 */
static int   rtcdate2second(rtc_time_t  compositetime, unsigned long  *ptimeOfsecond)
{
	struct rtc_time tmp;
	if (compositetime.weekday > 6){
		return -1;
	}
	tmp.tm_year = compositetime.year - 1900;
	tmp.tm_mon = compositetime.month - 1;
	tmp.tm_mday = compositetime.date;
	tmp.tm_wday = compositetime.weekday;
	tmp.tm_hour = compositetime.hour;
	tmp.tm_min = compositetime.minute;
	tmp.tm_sec = compositetime.second;
 	return rtc_valid_tm(&tmp) == 0 ? rtc_tm_to_time(&tmp, ptimeOfsecond) : -1;	
}

/*
 *	converse the data type from second to year.mouth.data.hour.minite.second
 *      define 2000.1.1.0.0.0 as jumping-off point 
 */

int  rtcSecond2Date(rtc_time_t *compositetime,unsigned long timeOfsecond)
{
	struct rtc_time tmp ;
	rtc_time_to_tm(timeOfsecond,&tmp);
	compositetime->year = (unsigned int)tmp.tm_year + 1900;
	compositetime->month = (unsigned int)tmp.tm_mon + 1;
	compositetime->date = (unsigned int)tmp.tm_mday;
	compositetime->hour = (unsigned int)tmp.tm_hour;
	compositetime->minute = (unsigned int)tmp.tm_min;
	compositetime->second = (unsigned int)tmp.tm_sec;
	compositetime->weekday = (unsigned int)tmp.tm_wday;
printk("RTC read time\n");
printk("\tyear %d\n", compositetime->year);
printk("\tmonth %d\n", compositetime->month);
printk("\tdate %d\n", compositetime->date);
printk("\thour %d\n", compositetime->hour);
printk("\tminute %d\n", compositetime->minute);
printk("\tsecond %d\n", compositetime->second);
printk("\tweekday %d\n", compositetime->weekday);
	return 0;
}
/*
 *	set time
 */
int  rtc_set(rtc_time_t compositetime)
{   
    	int  status;
    	unsigned long   timeofsecond;    	
printk("RTC set time\n");
printk("\tyear %d\n", compositetime.year);
printk("\tmonth %d\n", compositetime.month);
printk("\tdate %d\n", compositetime.date);
printk("\thour %d\n", compositetime.hour);
printk("\tminute %d\n", compositetime.minute);
printk("\tsecond %d\n", compositetime.second);
printk("\tweekday %d\n", compositetime.weekday);

    	status = rtcdate2second(compositetime , &timeofsecond);    	
    	if (0 != status)
	{
		printk("rtc_set : rtcdate2second fail\n");
		return status;
	}

	printk("rtc_time = %d\n", (int)timeofsecond);

    	writel(0x01,RTC_CR);       
	writel(timeofsecond,RTC_LR); 
    	return 0; 	
}                          


/*
 *	get current time
 *	the type of return value is rtc_time_t
 */
int  rtc_get(rtc_time_t *pcompositetime)
{ 
    	int  status;
    	unsigned int timeofsecond;
    	timeofsecond = readl(RTC_DR);    	
    	status = rtcSecond2Date(pcompositetime , timeofsecond);
	printk("rtc_time = %d\n", timeofsecond);
    	return status;
}


/*
 *	get current ALM time
 *	the type of return value is rtc_time_t
 */
int  rtc_getalm(rtc_time_t *pcompositetime)
{ 
    	int  status;
    	unsigned int timeofsecond;
    	timeofsecond = readl(RTC_MR);    	
    	status = rtcSecond2Date(pcompositetime , timeofsecond);
    	return 0;
}


/*
 *	set alarm. param type is second	
 *      After u32DelaySecond(s), a int will generate
 */
int  rtc_alarm_simple_set(unsigned int u32DelaySecond)
{
    	unsigned int  PdataRegValue = 0;
    	unsigned int  matchRegValue = 0;
    
    	if ((u32DelaySecond <= 0) || (u32DelaySecond == 0xFFFFFFFF))
    	{
        	#ifdef DEBUG_AMBA_RTC
        		printk("\nERRER:RTC delay time low than 1 second or too long! \n");
        	#endif        
        	return  - EINVAL;
    	} 
    	PdataRegValue = readl(RTC_DR);    
    	matchRegValue = PdataRegValue + u32DelaySecond;    
    	/* Judge whether the value (dataRegValue + delaySecond)> 0xFFFFFFFF or not*/
    	if (matchRegValue < PdataRegValue )
    	{
        	#ifdef DEBUG_AMBA_RTC
        		printk("\nWarning:RTC alarm time wraps arounded ! \n");
        	#endif	
    	}
    	writel(matchRegValue,RTC_MR);    	
    	return 0;   
}

/* 
 *	set alam. param type is rtc_time_t
 *	when the time comes to stCompositeTime, a init will generate
 */
int  rtc_alarm_complex_set(rtc_time_t stCompositeTime)
{
    	unsigned long timeofsecond;
    	unsigned int dataRegValue = 0;
    	unsigned int matchRegValue = 0;
    
    	if(rtcdate2second(stCompositeTime,&timeofsecond) == 0)
    	{    		
        	if ((timeofsecond <= 0) || (timeofsecond == 0xFFFFFFFF))
        	{
            		#ifdef DEBUG_AMBA_RTC
            			printk("\nERRER:RTC delay time low than 1 second or too long! \n");
            		#endif
            		return  - EINVAL;
        	}               	     	 
        	matchRegValue = timeofsecond;           	  
        	/* Judge whether the value (dataRegValue + delaySecond)> 0xFFFFFFFF or not*/
        	if (matchRegValue < dataRegValue )
        	{
            		#ifdef DEBUG_AMBA_RTC
            			printk("\nWarning:RTC alarm time wraps arounded ! \n");
            		#endif	
        	}
        	writel(matchRegValue,RTC_MR);         	
        	return 0;         
    	}
    	else
    	{
    		printk("set time error!");
        	return - EINVAL;    
   	}
}


/*
 *	ioctl function. for usr to control RTC	
 * 	parameter:
 *		RTC_AIE_ON:   enable interrupt
 *		RTC_AIE_OFF:  disable interrupt
 *		RTC_ALM_READ: get current alm time
 *		RTC_ALM_SET:  set alm. type of input is rtc_time_t. 
 *		RTC_RD_TIME:  get current time
 *		RTC_SET_TIME: set time. type of input is rtc_time_t. 
 */
//static int hi_rtc_ioctl(struct inode *inode, struct file *file,
//                    unsigned int cmd, unsigned long arg)
static int hi_rtc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    	rtc_time_t  tm;       
    	switch (cmd) 
    	{
		/* Alarm interrupt on/off */
    		case HI_RTC_AIE_ON:
        		writel(RTC_IMSC_ENABLE , RTC_IMSC);     		
			return 0;
    		case HI_RTC_AIE_OFF:
        		writel(RTC_IMSC_DISABLE , RTC_IMSC);
			return 0;
    		case HI_RTC_ALM_READ:
    		        rtc_getalm(&tm);    		      
    		        if (copy_to_user((void *)arg, &tm, sizeof(tm)))
    		        	return -EFAULT;
			return 0;
    		case HI_RTC_ALM_SET:
		        if (copy_from_user(&tm, (struct rtc_time_t*) arg, sizeof(tm))) 
		        	return -EFAULT;		       
		        return rtc_alarm_complex_set(tm);
    		case HI_RTC_RD_TIME:
printk("HI_RTC_RD_TIME\n");
		        rtc_get(&tm);
		        return copy_to_user((void *)arg, &tm, sizeof(tm)) ? -EFAULT : 0;

    		case HI_RTC_SET_TIME:
printk("HI_RTC_SET_TIME\n");
		        if (copy_from_user(&tm, (struct rtc_time_t *) arg, sizeof(tm))) 
		        	return -EFAULT;			      	       
		        return rtc_set(tm);

    		default:
			return -EINVAL;
	}
}


#ifdef CONFIG_PROC_FS
	static int rtc_read_proc(char *page, char **start, off_t off,
                       int count, int *eof, void *data)
	{
    		char *p = page;
    		int len;
    		rtc_time_t tm;
    		rtc_get(&tm);
    		p += sprintf(
    			     	p, "rtc_time\t: %02d:%02d:%02d\n"
		     		"rtc_date\t: %04d-%02d-%02d\n",
		     		tm.hour, tm.minute, tm.second,
		     		tm.year + 1900, tm.month + 1, tm.date
		     	     );
    		len = (p - page) - off;
    		if (len < 0)len = 0;
    		*eof = (len <= count) ? 1 : 0;
    		*start = page + off;
    		return len;
	}
#endif

/* open device*/
static int hi_rtc_open(struct inode *inode, struct file *file)
{    	
    	spin_lock_init(&rtc_lock);
    	spin_lock_irq(&rtc_lock);
    	if (rtc_status) 
    	{
		spin_unlock_irq(&rtc_lock);
		return -EBUSY;
    	}
    	/* set dev is busy */
    	rtc_status = 1;
    	spin_unlock_irq(&rtc_lock);
    	return 0;
}


static int hi_rtc_release(struct inode *inode, struct file *file)
{
        rtc_status = 0;
        return 0;
}


static struct file_operations hi_rtc_fops = 
{
    	owner:		THIS_MODULE,
    	unlocked_ioctl:	hi_rtc_ioctl,
    	open:		hi_rtc_open,
    	release:	hi_rtc_release,
};


static struct miscdevice hi_rtc_dev=
{
    	RTC_MINOR,
    	"rtc",
    	&hi_rtc_fops
};


/*
 *	int hi_rtc_init(void)
 *	default time is 2000.1.1.0.0.0
 */
static int __init rtc_init(void)
{
    	int ret;   

    	    	
    	ret = misc_register(&hi_rtc_dev);
    	if (0 != ret)
    	{
        	printk("rtc device register failed!\n");
        	return -EFAULT;
    	}

    	
	#ifdef CONFIG_PROC_FS
    		create_proc_read_entry("driver/rtc", 0, 0, rtc_read_proc, NULL);
	#endif

    	ret = request_irq(IRQ_RTC, &rtc_alm_interrupt, 0, \
			  "RTC Alarm", NULL);
    	if (0 != ret) 
    	{
        	printk(" hi3511 rtc: failed to register IRQ_RTC(%d)\n", IRQ_RTC);
		goto IRQ_RTC_failed;
    	}

	printk("rtc_init\n");
	// start rtc 
    	{
    		unsigned int d;
    		d = readl(SC_PERI_CRG57);
    		//d |= 0x8;
		d &= ~(0x4);
    		writel(d,SC_PERI_CRG57);
    	}
    	
    	writel(RTC_LOCK_VAL,RTC_LOCK);
	writel(0x01,RTC_ICR);
    	writel(0x01,RTC_CR);  

	//writel(0x01,RTC_MR);
    	//writel(0x01,RTC_LR);  
    	printk(KERN_INFO OSDRV_MODULE_VERSION_STRING);
    	return 0;
    	
IRQ_RTC_failed:
	#ifdef CONFIG_PROC_FS
    		remove_proc_entry("driver/rtc", NULL);
	#endif
    	misc_deregister(&hi_rtc_dev);
    	return ret;
}


static void __exit rtc_exit(void)
{    	 
    	free_irq(IRQ_RTC, NULL);	
	#ifdef CONFIG_PROC_FS
    		remove_proc_entry("driver/rtc", NULL);
	#endif
    	misc_deregister(&hi_rtc_dev);
}


module_init(rtc_init);
module_exit(rtc_exit);

MODULE_AUTHOR("Digital Media Team ,Hisilicon crop ");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Real Time Clock interface for HI3511");
MODULE_ALIAS_MISCDEV(RTC_MINOR);
MODULE_VERSION("HI_VERSION=" OSDRV_MODULE_VERSION_STRING);

