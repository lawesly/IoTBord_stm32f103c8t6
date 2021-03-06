/*
 * File      : qsdk_iot.c
 * This file is part of iot in qsdk
 * Copyright (c) 2018-2030, longmain Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-14     longmain     first version
 */

#include "qsdk.h"
#include "string.h"

//如果启用IOT支持

extern at_response_t at_resp;
extern char nb_buffer[QSDK_NET_REV_MAX_LEN];
//如果启用M5310连接IOT平台
#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
/*************************************************************
*	函数名称：	qsdk_iot_check_address
*
*	函数功能：	检查iot服务器地址是都正确
*
*	入口参数：	无
*
*	返回参数：	0 正确  1	失败
*
*	说明：
*************************************************************/
int qsdk_iot_check_address(void)
{
    char str[50];
    int port;
    if(at_exec_cmd(at_resp,"AT+NCDP?")!=RT_EOK) return RT_ERROR;

    at_resp_parse_line_args(at_resp,2,"%s",str);

    if(rt_strstr(str,QSDK_IOT_ADDRESS)!=RT_NULL)
        return RT_EOK;
    return RT_ERROR;
}
/*************************************************************
*	函数名称：	nb_set_ncdp
*
*	函数功能：	设置 NCDP 服务器
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_iot_set_address(void)
{
    if(at_exec_cmd(at_resp,"AT+CFUN=0")!=RT_EOK)
    {
        nb_device.error=qsdk_iot_status_set_mini_sim_failure;
        goto fail;
    }
    rt_thread_delay(100);
    if(at_exec_cmd(at_resp,"AT+NCDP=%s,%s",QSDK_IOT_ADDRESS,QSDK_IOT_PORT)!=RT_EOK)
    {
        nb_device.error=qsdk_iot_status_set_address_failure;
        goto fail;
    }
    rt_thread_delay(100);

    if(qsdk_hw_io_reboot()==RT_EOK)
        return RT_EOK;
    else goto fail;;

fail:
#ifdef QSDK_USING_LOG
    qsdk_nb_dis_error();
#endif
    return RT_ERROR;

}
#endif
#ifdef QSDK_USING_IOT
//如果启用M5310连接IOT平台
#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
/*************************************************************
*	函数名称：	qsdk_iot_open_update_status
*
*	函数功能：	开启上报信息提示
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_iot_open_update_status(void)
{
    if(at_exec_cmd(at_resp,"AT+NSMI=1")!=RT_EOK)	return RT_ERROR;

    return RT_EOK;
}
/*************************************************************
*	函数名称：	qsdk_iot_open_down_date_status
*
*	函数功能：	开启下发消息提示
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_iot_open_down_date_status(void)
{
    if(at_exec_cmd(at_resp,"AT+NNMI=1")!=RT_EOK)	return RT_ERROR;

    return RT_EOK;
}
/*************************************************************
*	函数名称：	qsdk_iot_send_date
*
*	函数功能：	上报信息到IOT平台
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_iot_send_date(char *str)
{
    nb_device.notify_status=qsdk_iot_status_notify_init;
    string_to_hex(str,strlen(str),nb_buffer);
    if(at_exec_cmd(at_resp,"AT+NMGS=%d,%s",strlen(nb_buffer)/2,nb_buffer)!=RT_EOK)	return RT_ERROR;

    rt_thread_delay(100);
    if(nb_device.notify_status==qsdk_iot_status_notify_success)
    {
#ifdef QSDK_USING_DEBUD
        LOG_D("qsdk iot notify success\r\n");
#endif
        return RT_EOK;
    }

    nb_device.error=qsdk_iot_status_notify_failure;
#ifdef QSDK_USING_LOG
    qsdk_nb_dis_error();
#endif
    return RT_ERROR;
}
#endif

#ifdef	QSDK_USING_ME3616

/*************************************************************
*	函数名称：	qsdk_iot_reg
*
*	函数功能：	注册到电信平台
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_iot_reg(void)
{
    int i=QSDK_IOT_REG_TIME_OUT;
    nb_device.iot_connect_status=qsdk_iot_status_reg_init;
    if(at_exec_cmd(at_resp,"AT+M2MCLINEW=%s,%s,\"%s\",%d",QSDK_IOT_ADDRESS,QSDK_IOT_PORT,nb_device.imei,QSDK_IOT_REG_TIME_OUT)!=RT_EOK) return RT_ERROR;

    do {
        i--;
        rt_thread_delay(500);
    } while(nb_device.iot_connect_status==qsdk_iot_status_reg_init&&i>0);

    if(nb_device.iot_connect_status==qsdk_iot_status_reg_failure||i<=0)
        return RT_ERROR;
    i=90;
    do {
        i--;
        rt_thread_delay(500);
    } while(nb_device.iot_connect_status==qsdk_iot_status_reg_success&&i>0);

    if(nb_device.iot_connect_status==qsdk_iot_status_observer_success)
        return RT_EOK;

    return RT_ERROR;
}

/*************************************************************
*	函数名称：	qsdk_iot_del_reg
*
*	函数功能：	在电信平台注销设备
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_iot_del_reg(void)
{
    int i=90;
    if(at_exec_cmd(at_resp,"AT+M2MCLIDEL")!=RT_EOK)	return RT_ERROR;

    do {
        i--;
        rt_thread_delay(500);
    } while(nb_device.iot_connect_status==qsdk_iot_status_observer_success&&i>0);

    if(nb_device.iot_connect_status!=qsdk_iot_status_reg_init||i<=0)
        return RT_ERROR;

    return	RT_EOK;
}
/*************************************************************
*	函数名称：	qsdk_iot_update
*
*	函数功能：	在电信平台注销设备
*
*	入口参数：	无
*
*	返回参数：	0 成功  1	失败
*
*	说明：
*************************************************************/
int qsdk_iot_update(char *str)
{
    int i=90;
    nb_device.notify_status=qsdk_iot_status_notify_init;
    if(at_exec_cmd(at_resp,"AT+M2MCLISEND=%s",str)!=RT_EOK)	return RT_ERROR;

    do {
        rt_thread_delay(500);
    } while(nb_device.notify_status==qsdk_iot_status_notify_init&&i>0);

    if(nb_device.notify_status==qsdk_iot_status_notify_failure||i<=0)
        return RT_ERROR;
    if(nb_device.notify_status==qsdk_iot_status_notify_success)
        return	RT_EOK;

    return RT_ERROR;
}
#endif


#endif


