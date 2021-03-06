/*
 * File      : qsdk_callback.c
 * This file is part of callback in qsdk
 * Copyright (c) 2018-2030, longmain Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-14     longmain     first version
 */
#define LOG_TAG              "qsdk_callback"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>
#include "qsdk_callback.h"
#include "stdio.h"
//#include "sht20.h"
//#include "led.h"
#include "stdlib.h"
#include "drv_rtc.h"

extern char temp[10];
extern char hump[10];

void qsdk_rtc_set_time_callback(int year,char month,char day,char hour,char min,char sec,char week)
{
    RTC_TimeTypeDef RTC_TimeStructure;
    RTC_DateTypeDef RTC_DateStructure;

    RTC_TimeStructure.Hours=hour;
    RTC_TimeStructure.Minutes=min;
    RTC_TimeStructure.Seconds=sec;
    RTC_DateStructure.Year=year;
    RTC_DateStructure.Month=month;
    RTC_DateStructure.Date=day;
    RTC_DateStructure.WeekDay=week;

    HAL_RTC_SetTime(&hrtc,&RTC_TimeStructure,RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc,&RTC_DateStructure,RTC_FORMAT_BIN);


}
int qsdk_net_data_callback(int socket,char *data,int len)
{
    LOG_D("enter net callback\r\n");
    LOG_D("rev data=%d,%s\r\n",len,data);
    return RT_EOK;
}
int qsdk_gps_callback(double lon,double lat,double speed)
{
    LOG_D("lon=%f,lat=%f,speed=%f\r\n",lon,lat,speed);
    return RT_EOK;
}

int qsdk_iot_data_callback(char *data,int len)
{
    LOG_D("enter iot callback\r\n");
    LOG_D("rev data=%d,%s\r\n",len,data);
    return RT_EOK;
}
#ifdef QSDK_USING_ONENET
int qsdk_onenet_open_callback()
{
    LOG_D("enter open onenet callback\r\n");

    return RT_EOK;


}
int qsdk_onenet_close_callback()
{
    LOG_D("enter close onenent callback\r\n");

    return RT_EOK;


}

int qsdk_onenet_read_rsp_callback(int objectid,int instanceid,int resourceid)
{
    LOG_D("enter read dsp callback\r\n");
//	if(objectid==temp_objectid)
//	{
//		sht20_get_value();
//		sLOG_D(temp,"%0.2f",sht20Info.tempreture);
//		sLOG_D(hump,"%0.2f",sht20Info.humidity);
//	}
//	else if(objectid==hump_objectid)
//	{
//		sht20_get_value();
//		sLOG_D(temp,"%0.2f",sht20Info.tempreture);
//		sLOG_D(hump,"%0.2f",sht20Info.humidity);
//	}
    if(objectid==light0_objectid&&instanceid==light0_instanceid&&resourceid==light0_resourceid)
    {

    }
    else if(objectid==light1_objectid&&instanceid==light1_instanceid&&resourceid==light1_resourceid)
    {

    }
    else if(objectid==light2_objectid&&instanceid==light2_instanceid&&resourceid==light2_resourceid)
    {

    }
    return RT_EOK;
}
int qsdk_onenet_write_rsp_callback(int objectid,int instanceid,int resourceid,int len,char* value)
{
    LOG_D("enter write dsp callback \r\n");
    LOG_D("value=%s \r\n",value);
    if(objectid==light0_objectid&&instanceid==light0_instanceid&&resourceid==light0_resourceid)
    {

        data_stream.update_status=1;
        qsdk_notify_data_to_status(objectid,instanceid,resourceid);
    }
    else if(objectid==light1_objectid&&instanceid==light1_instanceid&&resourceid==light1_resourceid)
    {
        data_stream.update_status=1;
        qsdk_notify_data_to_status(objectid,instanceid,resourceid);
    }
    else if(objectid==light2_objectid&&instanceid==light2_instanceid&&resourceid==light2_resourceid)
    {
        data_stream.update_status=1;
        qsdk_notify_data_to_status(objectid,instanceid,resourceid);
    }
    return RT_EOK;


}
int qsdk_onenet_exec_rsp_callback(int objectid,int instanceid,int resourceid,int len,char* cmd)
{

    LOG_D("enter exec dsp callback\r\n");

    LOG_D("exec data len:%d   data=%s\r\n",len,cmd);
    return RT_EOK;

}
#endif
int reboot_callback()
{
    LOG_D("enter reboot callback\r\n");

    return RT_EOK;


}




