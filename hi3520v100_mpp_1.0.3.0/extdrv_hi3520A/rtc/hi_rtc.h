/*
 * include/hi_rtc.h for Linux .
 *
 * This file defines hi_rtc micro-definitions for driver developer.
 *
 * History: 
 *      10-April-2006 create this file
 */

#ifndef __HI_RTC__
#define __HI_RTC__

#define OSDRV_MODULE_VERSION_STRING "HISI_RTC-MDC030001 @Hi3531v100"

#define HI_RTC_AIE_ON    _IO('p', 0x01)	
#define HI_RTC_AIE_OFF   _IO('p', 0x02)	
#define HI_RTC_ALM_SET   _IOW('p', 0x07,  rtc_time_t) 
#define HI_RTC_ALM_READ  _IOR('p', 0x08,  rtc_time_t) 
#define HI_RTC_RD_TIME   _IOR('p', 0x09,  rtc_time_t)
#define HI_RTC_SET_TIME  _IOW('p', 0x0a,  rtc_time_t)

typedef struct
{
    	unsigned int  year;
    	unsigned int  month;
    	unsigned int  date;
    	unsigned int  hour;
    	unsigned int  minute;
    	unsigned int  second;
    	unsigned int  weekday;
} rtc_time_t;
#endif 
