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

//�������IOT֧��

extern at_response_t at_resp;
extern char nb_buffer[QSDK_NET_REV_MAX_LEN];
//�������M5310����IOTƽ̨
#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
/*************************************************************
*	�������ƣ�	qsdk_iot_check_address
*
*	�������ܣ�	���iot��������ַ�Ƕ���ȷ
*
*	��ڲ�����	��
*
*	���ز�����	0 ��ȷ  1	ʧ��
*
*	˵����
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
*	�������ƣ�	nb_set_ncdp
*
*	�������ܣ�	���� NCDP ������
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1	ʧ��
*
*	˵����
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
//�������M5310����IOTƽ̨
#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
/*************************************************************
*	�������ƣ�	qsdk_iot_open_update_status
*
*	�������ܣ�	�����ϱ���Ϣ��ʾ
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1	ʧ��
*
*	˵����
*************************************************************/
int qsdk_iot_open_update_status(void)
{
    if(at_exec_cmd(at_resp,"AT+NSMI=1")!=RT_EOK)	return RT_ERROR;

    return RT_EOK;
}
/*************************************************************
*	�������ƣ�	qsdk_iot_open_down_date_status
*
*	�������ܣ�	�����·���Ϣ��ʾ
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1	ʧ��
*
*	˵����
*************************************************************/
int qsdk_iot_open_down_date_status(void)
{
    if(at_exec_cmd(at_resp,"AT+NNMI=1")!=RT_EOK)	return RT_ERROR;

    return RT_EOK;
}
/*************************************************************
*	�������ƣ�	qsdk_iot_send_date
*
*	�������ܣ�	�ϱ���Ϣ��IOTƽ̨
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1	ʧ��
*
*	˵����
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
*	�������ƣ�	qsdk_iot_reg
*
*	�������ܣ�	ע�ᵽ����ƽ̨
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1	ʧ��
*
*	˵����
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
*	�������ƣ�	qsdk_iot_del_reg
*
*	�������ܣ�	�ڵ���ƽ̨ע���豸
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1	ʧ��
*
*	˵����
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
*	�������ƣ�	qsdk_iot_update
*
*	�������ܣ�	�ڵ���ƽ̨ע���豸
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1	ʧ��
*
*	˵����
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


