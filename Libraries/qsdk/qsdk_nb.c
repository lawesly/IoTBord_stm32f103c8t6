/*
 * File      : qsdk_nb.c
 * This file is part of nb in qsdk
 * Copyright (c) 2018-2030, longmain Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-14     longmain     first version
 */
#define LOG_TAG              "qsdk_nb"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>
#include "qsdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//���� AT ������Ӧ�ṹ��ָ��
at_response_t at_resp = RT_NULL;

//����������ƿ�
rt_thread_t hand_thread_id=RT_NULL;

//���� ����
void hand_thread_entry(void* parameter);

//����������ƿ�
rt_mailbox_t event_mail=NULL;


struct NB_DEVICE nb_device;

/*************************************************************
*	�������ƣ�	qsdk_at_send_cmd
*
*	�������ܣ�	��ģ�鷢��CMD����
*
*	��ڲ�����	cmd:����		result:��Ҫ�жϵ���Ӧ
*
*	���ز�����	0���ɹ�   1��ʧ��
*
*	˵����
*************************************************************/
int qsdk_at_send_cmd(char *cmd,char *result)
{
    if(at_exec_cmd(at_resp,"%s",cmd)!=RT_EOK)	return RT_ERROR;

    if(at_resp_get_line_by_kw(at_resp,result)==RT_NULL)	return RT_ERROR;

    return RT_EOK;
}

/*************************************************************
*	�������ƣ�	qsdk_at_resp_cmd
*
*	�������ܣ�	��ģ�鷢��CMD����
*
*	��ڲ�����	cmd:����		result:ģ����Ӧ����
*
*	���ز�����	0���ɹ�   1��ʧ��
*
*	˵����
*************************************************************/
int qsdk_at_resp_cmd(char *cmd,int line,char *result)
{
    if(at_exec_cmd(at_resp,"%s",cmd)!=RT_EOK)	return RT_ERROR;

    at_resp_parse_line_args(at_resp,line,"%s\r\n",result);

    return RT_EOK;
}
/*************************************************************
*	�������ƣ�	qsdk_at_send_data
*
*	�������ܣ�	��ģ�鷢������
*
*	��ڲ�����	data:��Ҫ���͵�����
*
*	���ز�����	0���ɹ�   1��ʧ��
*
*	˵����
*************************************************************/
int qsdk_at_send_data(char *data)
{
    if(at_exec_cmd(at_resp,"%s",data)!=RT_EOK)	return RT_ERROR;

    return RT_EOK;
}

/*************************************************************
*	�������ƣ�	qsdk_init_environment
*
*	�������ܣ�	QSDK ���л�����ʼ��
*
*	��ڲ�����	��
*
*	���ز�����	0���ɹ�   1��ʧ��
*
*	˵����
*************************************************************/
int qsdk_init_environment(void)
{
#ifdef QSDK_USING_LOG
    LOG_D("\r\nWelcome to use QSDK. This SDK by longmain.\r\n Our official website is www.longmain.cn.\r\n\r\n");
#endif
    //���� AT ������Ӧ�ṹ��
    at_resp = at_create_resp(QSDK_CMD_REV_MAX_LEN+50, 0, rt_tick_from_millisecond(5000));

    //�ж��Ƿ񴴽��ɹ�
    if (at_resp == RT_NULL)
    {
        nb_device.error=qsdk_nb_status_create_at_resp_failure;
        goto fail;;
    }

    //�����¼�����
    event_mail=rt_mb_create("event_mail",
                            10,
                            RT_IPC_FLAG_FIFO);
    if(event_mail==RT_NULL)
    {
        nb_device.error=qsdk_nb_status_create_event_mail_failure;
        goto fail;
    }

    //�����¼���Ӧ����
    hand_thread_id=rt_thread_create("hand_thread",
                                    hand_thread_entry,
                                    RT_NULL,
                                    1000,
                                    7,
                                    50);
    if(hand_thread_id!=RT_NULL)
        rt_thread_startup(hand_thread_id);
    else
    {
        nb_device.error=qsdk_nb_status_create_hand_fun_failure;
        goto fail;
    }

    return RT_EOK;


fail:
#ifdef QSDK_USING_LOG
    qsdk_nb_dis_error();
#endif
    return RT_ERROR;
}
/*************************************************************
*	�������ƣ�	nb_hw_init
*
*	�������ܣ�	NB-IOT ģ��������ʼ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����
*************************************************************/
int qsdk_nb_hw_init(void)
{
    static char status=0;
    int i=5;
    //����豸�ṹ��
    rt_memset(&nb_device,0,sizeof(nb_device));
    if(status!=1)
    {
        //ģ�����ų�ʼ��
        if(qsdk_hw_io_init()!=RT_EOK)
        {
            nb_device.error=qsdk_nb_status_io_init_failure;
            goto fail;
        }
        else
        {
            if(qsdk_init_environment()!=RT_EOK) return RT_ERROR;

            status=1;
        }

//�������֧��M5310����IOTƽ̨
#ifdef QSDK_USING_M5310A
start:
#endif	//QSDK_USING_M5310_IOT  END

        //�ȴ�ģ������
        if(qsdk_nb_wait_connect()!=RT_EOK)
            return RT_ERROR;
    }

//�������M5310����IOTƽ̨
#ifdef QSDK_USING_M5310A
    if(qsdk_iot_check_address()!=RT_EOK)
    {
#ifdef QSDK_USING_LOG
        LOG_D("ncdp��ַ���ԣ����ڽ�������\r\n");
#endif

        if(qsdk_iot_set_address()==RT_EOK)
            goto start;
        else goto fail;
    }
#ifdef QSDK_USING_LOG
    LOG_D("ncdp address check success\r\n");
#endif

#endif

#if QSDK_USING_PSM_MODE
    if(qsdk_nb_set_psm_mode("01000111","10100100")!=RT_EOK)
    {
        nb_device.ERROR=qsdk_nb_status_set_low_power_failure;
        goto fail;
    }
#endif

//����ȷ��ģ���Ƿ񿪻�
    do {
        i--;
        if(qsdk_nb_sim_check()!=RT_EOK)
        {
            rt_thread_delay(500);
        }
#ifdef QSDK_USING_LOG
        LOG_D("+CFUN=%d\r\n",nb_device.sim_state);
#endif
        if(nb_device.sim_state!=1)
            rt_thread_delay(1000);

    }	while(nb_device.sim_state==0&&i>0);

    if(i<=0)
    {
        nb_device.error=qsdk_nb_status_module_start_failure;
        goto fail;
    }
    else {
        i=3;
        rt_thread_delay(1000);
    }

//��ȡSIM����IMSI����
    do {
        i--;
        if(qsdk_nb_get_imsi()!=RT_EOK)
        {
            rt_thread_delay(500);
        }
        else
        {
#ifdef QSDK_USING_LOG
            LOG_D("IMSI=%s\r\n",nb_device.imsi);
#endif
            break;
        }
    } while(i>0);

    if(i<=0)
    {
        nb_device.error=qsdk_nb_status_no_find_sim;
        goto fail;

    }
    else
    {
        i=15;
        rt_thread_delay(100);
    }

//��ȡģ��IMEI
    if(qsdk_nb_get_imei()!=RT_EOK)
    {
        nb_device.error=qsdk_nb_status_read_module_imei_failure;
        goto fail;
    }
    else
    {
#ifdef QSDK_USING_LOG
        LOG_D("IMEI=%s\r\n",nb_device.imei);
#endif
    }

//�������IOTƽ̨֧��
#ifdef QSDK_USING_IOT

//�������M5310����IOTƽ̨
#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
    rt_thread_delay(100);
    if(qsdk_iot_open_update_status()!=RT_EOK)
    {
        nb_device.error=qsdk_iot_status_open_update_dis_failure;
        goto fail;
    }
#ifdef QSDK_USING_DEBUG
    else LOG_D("qsdk open iot update status success\r\n");
#endif
    rt_thread_delay(100);
    if(qsdk_iot_open_down_date_status()!=RT_EOK)
    {
        nb_device.error=qsdk_iot_status_open_down_dis_failure;
        goto fail;
    }
#ifdef QSDK_USING_DEBUG
    else LOG_D("qsdk open iot down date status success\r\n");
#endif

#endif

#endif
//��ȡ�ź�ֵ
    do {
        i--;
        if(qsdk_nb_get_csq()!=RT_EOK)
        {
            rt_thread_delay(500);
        }
        else if(nb_device.csq!=99&&nb_device.csq!=0)
        {
            break;
        }
        else
        {
#ifdef QSDK_USING_LOG
            LOG_D("CSQ=%d\r\n",nb_device.csq);
#endif
            rt_thread_delay(3000);
        }

    } while(i>0);

    if(i<=0)
    {
        nb_device.error=qsdk_nb_status_module_no_find_csq;
        goto fail;
    }
    else
    {
#ifdef QSDK_USING_LOG
        LOG_D("CSQ=%d\r\n",nb_device.csq);
#endif
        i=30;
        rt_thread_delay(100);
    }
//�ֶ���������
#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
    if(qsdk_nb_get_net_start()!=RT_EOK)
    {
        rt_thread_delay(500);
    }
    else if(!nb_device.net_connect_ok)
    {
        if(qsdk_nb_set_net_start()!=RT_EOK)
        {
            nb_device.error=qsdk_nb_status_set_net_start_failure;
            goto fail;
        }
    }
#endif
//��ȡ���總��״̬
    do {
        i--;
        if(qsdk_nb_get_net_start()!=RT_EOK)
        {
            rt_thread_delay(500);
        }
        else if(nb_device.net_connect_ok)
        {
            break;
        }
        else
        {
#ifdef QSDK_USING_LOG
            LOG_D("CEREG=%d\r\n",nb_device.net_connect_ok);
#endif
            rt_thread_delay(1000);
        }

    } while(i>0);

    if(i<=0)
    {
        nb_device.error=qsdk_nb_status_fine_net_start_failure;
        goto fail;
    }
    rt_thread_delay(1000);
#ifdef QSDK_USING_LOG
    LOG_D("CEREG=%d\r\n",nb_device.net_connect_ok);
#endif

//��ȡntp������ʱ��

    if(qsdk_nb_get_time()!=RT_EOK)
    {
        nb_device.error=qsdk_nb_status_get_ntp_time_failure;
    }

#ifdef QSDK_USING_LOG
    LOG_D("net connect ok\r\n");
#endif

    return RT_EOK;

fail:

#ifdef QSDK_USING_LOG
    qsdk_nb_dis_error();
#endif
    return RT_ERROR;
}
/*************************************************************
*	�������ƣ�	qsdk_nb_wait_connect
*
*	�������ܣ�	�ȴ�ģ������
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�   1 ʧ��
*
*	˵����
*************************************************************/
int qsdk_nb_wait_connect(void)
{
    if(at_client_wait_connect(2000)!=RT_EOK)
    {
        nb_device.error=qsdk_nb_status_no_find_nb_module;
        rt_thread_delete(hand_thread_id);			//ɾ�� hand��������
        rt_mb_delete(event_mail);							//ɾ�� event ����
        at_delete_resp(at_resp);							//ɾ�� AT ������Ӧ�ṹ��
        goto fail;
    }

    return RT_EOK;

fail:
#ifdef QSDK_USING_LOG
    qsdk_nb_dis_error();
#endif
    return RT_ERROR;
}

/*************************************************************
*	�������ƣ�	nb_sim_check
*
*	�������ܣ�	���ģ���Ƿ��Ѿ�����
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1	ʧ��
*
*	˵����
*************************************************************/
int qsdk_nb_sim_check(void)
{
    if(at_exec_cmd(at_resp,"AT+CFUN?")!=RT_EOK)	return RT_ERROR;
    at_resp_parse_line_args(at_resp,2,"+CFUN:%d",&nb_device.sim_state);

    return  RT_EOK;
}
/*************************************************************
*	�������ƣ�	nb_set_psm_mode
*
*	�������ܣ�	ģ�� PSM ģʽ����
*
*	��ڲ�����	tau_time	TAU ʱ��		active_time activeʱ��
*
*	���ز�����	0 �ɹ�  1	ʧ��
*
*	˵����
*************************************************************/
int qsdk_nb_set_psm_mode(char *tau_time,char *active_time)
{
    if(at_exec_cmd(at_resp,"AT+CPSMS=1,,,%s,%s",tau_time,active_time)!=RT_EOK)	return RT_ERROR;

    return  RT_EOK;
}
/*************************************************************
*	�������ƣ�	nb_get_imsi
*
*	�������ܣ�	��ȡ SIM ���� imsi
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1	ʧ��
*
*	˵����
*************************************************************/
int qsdk_nb_get_imsi(void)
{
    if(at_exec_cmd(at_resp,"AT+CIMI")!=RT_EOK)	return RT_ERROR;

    at_resp_parse_line_args(at_resp,2,"%s\r\n",nb_device.imsi);
    return  RT_EOK;
}
/*************************************************************
*	�������ƣ�	nb_get_imei
*
*	�������ܣ�	��ȡģ��� imei
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1	ʧ��
*
*	˵����
*************************************************************/
int qsdk_nb_get_imei(void)
{
    if(at_exec_cmd(at_resp,"AT+CGSN=1")!=RT_EOK)	return RT_ERROR;

    at_resp_parse_line_args(at_resp,2,"+CGSN:%s",nb_device.imei);
    return  RT_EOK;
}
/*************************************************************
*	�������ƣ�	nb_get_time
*
*	�������ܣ�	��ȡ����ʱ��
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1	ʧ��
*
*	˵����
*************************************************************/
int qsdk_nb_get_time(void)
{
    int year,mouth,day,hour,min,sec;
    if(at_exec_cmd(at_resp,"AT+CCLK?")!=RT_EOK)	return RT_ERROR;

    at_resp_parse_line_args(at_resp,2,"+CCLK:%d/%d/%d,%d:%d:%d+",&year,&mouth,&day,&hour,&min,&sec);

#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
    qsdk_nb_set_rtc_time(year,mouth,day,hour,min,sec);
#else
    qsdk_nb_set_rtc_time(year%100,mouth,day,hour,min,sec);
#endif
    return  RT_EOK;
}

/*************************************************************
*	�������ƣ�	nb_get_csq
*
*	�������ܣ�	��ȡ��ǰ�ź�ֵ
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1	ʧ��
*
*	˵����
*************************************************************/
int qsdk_nb_get_csq(void)
{
    if(at_exec_cmd(at_resp,"AT+CSQ")!=RT_EOK)	return RT_ERROR;

    at_resp_parse_line_args(at_resp,2,"+CSQ:%d\r\n",&nb_device.csq);
    return  RT_EOK;

}
/*************************************************************
*	�������ƣ�	nb_set_net_start
*
*	�������ܣ�	�ֶ���������
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1	ʧ��
*
*	˵����
*************************************************************/
int qsdk_nb_set_net_start(void)
{
    if(at_exec_cmd(at_resp,"AT+CGATT=1")!=RT_EOK)	return RT_ERROR;

    return  RT_EOK;
}
/*************************************************************
*	�������ƣ�	nb_get_net_start
*
*	�������ܣ�	��ȡ��ǰ����״̬
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1	ʧ��
*
*	˵����
*************************************************************/
int qsdk_nb_get_net_start(void)
{
    int i,j;
    if(at_exec_cmd(at_resp,"AT+CEREG?")!=RT_EOK)	return RT_ERROR;

    at_resp_parse_line_args(at_resp,2,"+CEREG:%d,%d\r\n",&i,&j);

    if(j==1) nb_device.net_connect_ok=1;
    else nb_device.net_connect_ok=0;
    return  RT_EOK;
}

#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
/*************************************************************
*	�������ƣ�	nb_query_ip
*
*	�������ܣ�	��ѯģ���ں�������IP��ַ
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1	ʧ��
*
*	˵����
*************************************************************/
int qsdk_nb_query_ip(void)
{
    if(at_exec_cmd(at_resp,"AT+CGPADDR")!=RT_EOK) return RT_ERROR;

    at_resp_parse_line_args(at_resp,2,"+CGPADDR:0,%s",nb_device.ip);
    return RT_EOK;
}
#endif	//qsdk_enable_m5310	end



#ifdef QSDK_USING_ME3616_GPS
/*************************************************************
*	�������ƣ�	qsdk_gps_config
*
*	�������ܣ�	���ò�������AGPS
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1	ʧ��
*
*	˵����
*************************************************************/
int qsdk_agps_config(void)
{
    int i=90;
    char str[20];

    if(at_exec_cmd(at_resp,"AT+ZGMODE=1")!=RT_EOK)	return RT_ERROR;
    at_resp_parse_line_args(at_resp,2,"%s",str);
    if(rt_strstr(str,"+ZGPS: DATA DOWNLOAD SUCCESS")==NULL)
    {
#ifdef QSDK_USING_DEBUG
        LOG_D("seting gps \r\n");
#endif
        rt_thread_delay(100);
        if(at_exec_cmd(at_resp,"AT+ZGDATA")!=RT_EOK)	return RT_ERROR;

        do {
            i--;
            rt_thread_delay(500);
            if(at_exec_cmd(at_resp,"AT+ZGDATA?")!=RT_EOK)	return RT_ERROR;
            at_resp_parse_line_args(at_resp,2,"+ZGDATA: %s\r\n",str);
            if(rt_strstr(str,"READY")!=NULL)
                if(rt_strstr(str,"NO READY")==NULL)
                    break;
        } while(i>0);
        if(i<=0)
            return RT_ERROR;
    }
#ifdef QSDK_USING_DEBUG
    LOG_D("gps runing\r\n");
#endif
    rt_thread_delay(100);
    if(at_exec_cmd(at_resp,"AT+ZGNMEA=2")!=RT_EOK)	return RT_ERROR;
    rt_thread_delay(100);
    if(at_exec_cmd(at_resp,"AT+ZGRUN=1")!=RT_EOK)	return RT_ERROR;

    return	RT_EOK;
}
/*************************************************************
*	�������ƣ�	qsdk_gps_config
*
*	�������ܣ�	���ò�������GPS
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1	ʧ��
*
*	˵����
*************************************************************/
int qsdk_gps_config(void)
{
    int i=90;
    char str[20];
    if(at_exec_cmd(at_resp,"AT+ZGMODE=2")!=RT_EOK)	return RT_ERROR;
    rt_thread_delay(100);
    if(at_exec_cmd(at_resp,"AT+ZGNMEA=2")!=RT_EOK)	return RT_ERROR;
    rt_thread_delay(100);
    if(at_exec_cmd(at_resp,"AT+ZGRUN=2")!=RT_EOK)	return RT_ERROR;

    return	RT_EOK;
}
#endif	//qsdk_enable_me3616_gps	end

/*************************************************************
*	�������ƣ�	string_to_hex
*
*	�������ܣ�	�ַ���תhex
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1	ʧ��
*
*	˵����
*************************************************************/
int string_to_hex(const char *pString, int len, char *pHex)
{
    int i = 0;
    if (NULL == pString || len <= 0 || NULL == pHex)
    {
        return RT_ERROR;
    }
    for(i = 0; i < len; i++)
    {
        rt_sprintf(pHex+i*2, "%02X", pString[i]);
    }
    return RT_EOK;
}
/*************************************************************
*	�������ƣ�	nb_set_rtc_time
*
*	�������ܣ�	����RTCʱ��Ϊ��ǰʱ��
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1	ʧ��
*
*	˵����
*************************************************************/
void qsdk_nb_set_rtc_time(int year,int month,int day,int hour,int min,int sec)
{
    int week,lastday;
    hour+=QSDK_TIME_ZONE;
    if ((0==year%4 && 0!=year%100) || 0==year%400)
        lastday=29;
    else if(month==1||month==3||month==5||month==7||month==8||month==10||month==12)
        lastday=31;
    else if(month==4||month==6||month==9||month==11)
        lastday=30;
    else
        lastday=28;
    if(hour>24)
    {
        hour-=24;
        day++;
        if(day>lastday)
        {
            day-=lastday;
            month++;
        }
        if(month>12)
        {
            month-=12;
            year++;
        }
    }
    week=(day+2*month+3*(month+1)/5+year+year/4-year/100+year/400)%7+1;
#ifdef QSDK_USING_DEBUG
    LOG_D("time=%d-%d-%d,%d-%d-%d,week=%d\r\n",year,month,day,hour,min,sec,week);
#endif
    qsdk_rtc_set_time_callback(year,month,day,hour,min,sec,week);

}
/*************************************************************
*	�������ƣ�	hand_thread_entry
*
*	�������ܣ�	ģ�������ϱ����ݴ�������
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����
*************************************************************/
void hand_thread_entry(void* parameter)
{
    rt_err_t status=RT_EOK;
    char *event;
    char *result=NULL;
#ifdef QSDK_USING_ONENET
    char *instance=NULL;
    char *msgid=NULL;
    char *objectid=NULL;
    char *instanceid=NULL;
    char *resourceid=NULL;
#endif
    while(1)
    {
        //�ȴ��¼��ʼ� event_mail
        status=rt_mb_recv(event_mail,(rt_uint32_t *)&event,RT_WAITING_FOREVER);

        //�ж��Ƿ���ճɹ�
        if(status==RT_EOK)
        {
#ifdef QSDK_USING_NET

#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
            //�ж��ǲ���M5310 tcp ���� udp ��Ϣ
            if(rt_strstr(event,"+NSONMI:")!=RT_NULL)
            {
                char *eventid=NULL;
                char *socket=NULL;
                char *len=NULL;
#ifdef QSDK_USING_LOG
                LOG_D("%s\r\n ",event);
#endif
                eventid=strtok((char*)event,":");
                socket=strtok(NULL,",");
                len=strtok(NULL,",");

                //�����������ݴ����ص�����
                if(qsdk_net_rev_data(atoi(socket),atoi(len))!=RT_EOK)
                    LOG_D("rev net data failure\r\n");
            }
#elif (defined QSDK_USING_ME3616)
            //�ж��ǲ��� tcp ���� udp ��Ϣ
            if(rt_strstr(event,"+ESONMI=")!=RT_NULL)
            {
                char *result;
                char *socket;
                char *rev_len;
                char *rev_data;
#ifdef QSDK_USING_LOG
                LOG_D("%s\r\n",event);
#endif
                result=strtok(event,"=");
                socket=strtok(NULL,",");
                rev_len=strtok(NULL,",");
                rev_data=strtok(NULL,"\r\n");

                if(qsdk_net_data_callback(atoi(socket),rev_data,atoi(rev_len))!=RT_EOK)
                    LOG_D("QSDK net data callback failure\r\n");

            }
#endif	//QSDK_USING_ME3616_NET	END

#endif	//QSDK_USING_NET END
#ifdef QSDK_USING_IOT

#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
            if(rt_strstr(event,"+NNMI:")!=RT_NULL)
            {
                char *len;
                char *str;
#ifdef QSDK_USING_LOG
                LOG_D("%s\r\n",event);
#endif
                result=strtok(event,":");
                len=strtok(NULL,",");
                str=strtok(NULL,",");

                if(qsdk_iot_data_callback(str,atoi(len))!=RT_EOK)
                    LOG_D("qsdk iot data callback failure\r\n");
            }
            else if(rt_strstr(event,"+NSMI:")!=RT_NULL)
            {
#ifdef QSDK_USING_DEBUD
                LOG_D("%s\r\n",event);
#endif
                nb_device.notify_status=qsdk_iot_status_notify_success;
            }

#elif (defined QSDK_USING_ME3616)
            if(rt_strstr(event,"+M2MCLI:")!=RT_NULL)
            {
#ifdef QSDK_USING_LOG
                LOG_D("%s\r\n",event);
#endif
                if(rt_strstr(event,"+M2MCLI:register failure"))
                    nb_device.iot_connect_status=qsdk_iot_status_reg_failure;
                if(rt_strstr(event,"+M2MCLI:register success"))
                    nb_device.iot_connect_status=qsdk_iot_status_reg_success;
                if(rt_strstr(event,"+M2MCLI:observe success"))
                    nb_device.iot_connect_status=qsdk_iot_status_observer_success;
                if(rt_strstr(event,"+M2MCLI:deregister success"))
                    nb_device.iot_connect_status=qsdk_iot_status_reg_init;
                if(rt_strstr(event,"+M2MCLI:notify success"))
                    nb_device.notify_status=qsdk_iot_status_notify_success;
            }
            if(rt_strstr(event,"+M2MCLIRECV:")!=RT_NULL)
            {
                char *str;
#ifdef QSDK_USING_LOG
                LOG_D("%s\r\n",event);
#endif
                result=strtok(event,":");
                str=strtok(NULL,",");
                if(qsdk_iot_data_callback(str,(strlen(str)/2)-1)!=RT_EOK)
                {
#ifdef QSDK_USING_DEBUD
                    LOG_D("iot data rev failure\r\n");
#endif
                }
            }
            if(rt_strstr(event,"$GNRMC")!=RT_NULL)
            {
                char *gps_time;
                char *gps_status;
                char *gps_lat;
                char *result4;
                char *gps_lon;
                char *result6;
                char *gps_speed;
                double temp = 0;
                uint32_t dd = 0;
#ifdef QSDK_USING_LOG
                LOG_D("%s,  len=%d",event,strlen(event));
#endif
                if(strlen(event)>27)
                {
                    double lat;
                    double lon;
                    double speed;
                    result=strtok(event,",");
                    gps_time=strtok(NULL,",");
                    gps_status=strtok(NULL,",");
                    if(*gps_status=='A')
                    {
                        gps_lat=strtok(NULL,",");
                        result4=strtok(NULL,",");
                        gps_lon=strtok(NULL,",");
                        result6=strtok(NULL,",");
                        gps_speed=strtok(NULL,",");

                        //GPRMC��γ��ֵ��ʽΪddmm.mmmm,Ҫת����dd.dddddd
                        temp = atof(gps_lat);
                        dd = (uint32_t)(temp / 100);  //ȡ��������
                        lat = dd + ((temp - dd * 100)/60);

                        //GPRMC�ľ��ȸ�ʽΪdddmm.mmmm��Ҫת����dd.dddddd
                        temp = atof(gps_lon);
                        dd = (uint32_t)(temp / 100);
                        lon = dd + ((temp - dd * 100)/60);

                        speed=atof(gps_speed);

                        if(qsdk_gps_callback(lon,lat,speed)!=RT_EOK)
                            LOG_D("GPS callback fallure");

                    }
                }
            }
#endif

#endif
#ifdef QSDK_USING_ONENET
            //�ж��Ƿ�Ϊ�¼���Ϣ
            if(rt_strstr(event,"+MIPLEVENT:")!=RT_NULL)
            {
                char *eventid=NULL;
#ifdef QSDK_USING_LOG
                LOG_D("%s\r\n ",event);
#endif
                result=strtok((char*)event,":");
                instance=strtok(NULL,",");
                eventid=strtok(NULL,",");
#ifdef QSDK_USING_DEBUD
                LOG_D("instance=%d,eventid=%d\r\n",atoi(instance),atoi(eventid));
#endif
                //�����¼���������
                switch(atoi(eventid))
                {
                case  1:
#ifdef QSDK_USING_LOG
                    LOG_D("Bootstrap start    \r\n");
#endif
                    data_stream.event_status=qsdk_onenet_status_run;
                    break;
                case  2:
#ifdef QSDK_USING_LOG
                    LOG_D("Bootstrap success  \r\n");
#endif
                    data_stream.event_status=qsdk_onenet_status_run;
                    break;
                case  3:
#ifdef QSDK_USING_LOG
                    LOG_D("Bootstrap failure\r\n");
#endif
                    data_stream.event_status=qsdk_onenet_status_failure;
                    break;
                case  4:
#ifdef QSDK_USING_LOG
                    LOG_D("Connect success\r\n");
#endif
                    data_stream.event_status=qsdk_onenet_status_run;
                    break;
                case  5:
#ifdef QSDK_USING_LOG
                    LOG_D("Connect failure\r\n");
#endif
                    data_stream.event_status=qsdk_onenet_status_failure;
                    break;
                case  6:
#ifdef QSDK_USING_LOG
                    LOG_D("Reg onenet success\r\n");
#endif
                    data_stream.event_status=qsdk_onenet_status_success;
                    break;
                case  7:
#ifdef QSDK_USING_LOG
                    LOG_D("Reg onenet failure\r\n");
#endif
                    data_stream.event_status=qsdk_onenet_status_failure;
                    break;
                case  8:
#ifdef QSDK_USING_LOG
                    LOG_D("Reg onenet timeout\r\n");
#endif
                    data_stream.event_status=qsdk_onenet_status_failure;
                    break;
                case  9:
#ifdef QSDK_USING_LOG
                    LOG_D("Life_time timeout\r\n");
#endif
                    break;
                case 10:
#ifdef QSDK_USING_LOG
                    LOG_D("Status halt\r\n");
#endif
                    break;
                case 11:
#ifdef QSDK_USING_LOG
                    LOG_D("Update success\r\n");
#endif
                    data_stream.update_status=qsdk_onenet_status_update_success;
                    break;
                case 12:
#ifdef QSDK_USING_LOG
                    LOG_D("Update failure\r\n");
#endif
                    data_stream.update_status=qsdk_onenet_status_update_failure;
                    break;
                case 13:
#ifdef QSDK_USING_LOG
                    LOG_D("Update timeout\r\n");
#endif
                    data_stream.update_status=qsdk_onenet_status_update_timeout;
                    break;
                case 14:
#ifdef QSDK_USING_LOG
                    LOG_D("Update need\r\n");
#endif
                    data_stream.update_status=qsdk_onenet_status_update_need;
                    break;
                case 15:
#ifdef QSDK_USING_LOG
                    LOG_D("Unreg success\r\n");
#endif
                    data_stream.connect_status=qsdk_onenet_status_failure;
                    break;
                case 20:
#ifdef QSDK_USING_LOG
                    LOG_D("Response failure\r\n");
#endif
                    break;
                case 21:
#ifdef QSDK_USING_LOG
                    LOG_D("Response success\r\n");
#endif
                    break;
                case 25:
#ifdef QSDK_USING_LOG
                    LOG_D("Notify failure\r\n");
#endif
                    data_stream.notify_status=qsdk_onenet_status_failure;
                    break;
                case 26:
#ifdef QSDK_USING_LOG
                    LOG_D("Notify success\r\n");
#endif
                    data_stream.notify_status=qsdk_onenet_status_success;
                    break;
                default:
                    break;
                }
            }
            //�ж��Ƿ�Ϊ onenet ƽ̨ read �¼�
            else if(rt_strstr(event,"+MIPLREAD:")!=RT_NULL)
            {
#ifdef QSDK_USING_LOG
                LOG_D("%s\r\n",event);
#endif
                result=strtok(event,":");
                instance=strtok(NULL,",");
                msgid=strtok(NULL,",");
                objectid=strtok(NULL,",");
                instanceid=strtok(NULL,",");
                resourceid=strtok(NULL,",");

                //���� onenet read ��Ӧ����
                status=qsdk_rsp_onenet_read(atoi(msgid),atoi(objectid),atoi(instanceid),atoi(resourceid));
                //�ж��Ƿ���Ӧ�ɹ�
                if(status!=RT_EOK)
                    LOG_D("rsp onener read failure\r\n");
#ifdef QSDK_USING_LOG
                else
                {
                    LOG_D("rsp onener read success\r\n");
                }
#endif
            }
            //�ж��Ƿ�Ϊ onenet write �¼�
            else if(rt_strstr(event,"+MIPLWRITE:")!=RT_NULL)
            {
                char *valuetype=NULL;
                char *value_len=NULL;
                char *value=NULL;
                char *flge=NULL;
#ifdef QSDK_USING_LOG
                LOG_D("%s\r\n",event);
#endif
                result=strtok(event,":");
                instance=strtok(NULL,",");
                msgid=strtok(NULL,",");
                objectid=strtok(NULL,",");
                instanceid=strtok(NULL,",");
                resourceid=strtok(NULL,",");
                valuetype=strtok(NULL,",");
                value_len=strtok(NULL,",");
                value=strtok(NULL,",");
                flge=strtok(NULL,",");
                //�жϱ�ʶ�Ƿ�Ϊ0
                if(atoi(flge)==0)
                {
                    //ִ�� onenet write ��Ӧ����
                    if(qsdk_rsp_onenet_write(atoi(msgid),atoi(objectid),atoi(instanceid),atoi(resourceid),atoi(valuetype),atoi(value_len),value)!=RT_EOK)
                        LOG_D("rsp onenet write failure\r\n");
#ifdef QSDK_USING_LOG
                    else LOG_D("rsp onenet write success\r\n");
#endif
                }
            }
            //�ж��Ƿ�Ϊ exec �¼�
            else if(rt_strstr(event,"+MIPLEXECUTE:")!=RT_NULL)
            {
                char *value_len=NULL;
                char *value=NULL;
#ifdef QSDK_USING_LOG
                LOG_D("%s\r\n",event);
#endif
                result=strtok(event,":");
                instance=strtok(NULL,",");
                msgid=strtok(NULL,",");
                objectid=strtok(NULL,",");
                instanceid=strtok(NULL,",");
                resourceid=strtok(NULL,",");
                value_len=strtok(NULL,",");
                value=strtok(NULL,",");

                //ִ�� onenet write ��Ӧ����
                if(qsdk_rsp_onenet_execute(atoi(msgid),atoi(objectid),atoi(instanceid),atoi(resourceid),atoi(value_len),value)!=RT_EOK)
                    LOG_D("rsp onenet execute failure\r\n");
#ifdef QSDK_USING_LOG
                else LOG_D("rsp onenet execute success\r\n");
#endif
            }
            //�ж��Ƿ�Ϊ observe �¼�
            else if(rt_strstr(event,"+MIPLOBSERVE:")!=RT_NULL)
            {
                char *oper=NULL;
                int i=0,j=0,status=1,resourcecount=0;
                data_stream.observercount++;
                data_stream.observer_status=qsdk_onenet_status_run;
#ifdef QSDK_USING_LOG
                LOG_D("%s\r\n",event);
#endif
                result=strtok(event,":");
                instance=strtok(NULL,",");
                msgid=strtok(NULL,",");
                oper=strtok(NULL,",");
                objectid=strtok(NULL,",");
                instanceid=strtok(NULL,",");

                for(; i<data_stream.dev_len; i++)
                {
                    if(data_stream.dev[i].objectid==atoi(objectid)&&data_stream.dev[i].instanceid==atoi(instanceid))
                    {
                        data_stream.dev[i].msgid=atoi(msgid);
#ifdef QSDK_USING_DEBUG
                        LOG_D("objece=%d,instanceid=%d msg=%d\r\n",data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].msgid);
#endif
                    }
                }
#ifdef QSDK_USING_ME3616
                LOG_D("AT+MIPLOBSERVERSP=%d,%d,%d\r\n",data_stream.instance,atoi(msgid),atoi(oper));
                if(at_exec_cmd(at_resp,"AT+MIPLOBSERVERSP=%d,%d,%d",data_stream.instance,atoi(msgid),atoi(oper))!=RT_EOK)
                {
                    LOG_D("+MIPLOBSERVERSP  failure \r\n");
                    data_stream.observer_status=qsdk_onenet_status_failure;
                }
#endif

#ifdef QSDK_USING_DEBUD
                LOG_D("observercount=%d\r\n",data_stream.observercount);
#endif
                //�ж� obcerve �¼��Ƿ�ִ�����
                if(data_stream.observercount==data_stream.instancecount)
                {
                    //observe �¼�ִ�����
                    data_stream.observer_status=qsdk_onenet_status_success;
#ifdef QSDK_USING_DEBUD
                    LOG_D("+MIPLOBSERVERSP  success\r\n ");
#endif
                }
            }
            //�ж��Ƿ�Ϊ discover �¼�
            else if(rt_strstr(event,"+MIPLDISCOVER:")!=RT_NULL)
            {
                char resourcemap[50]="";
                int str[50];
                int i=0,j=0,status=1,resourcecount=0;
#ifdef QSDK_USING_LOG
                LOG_D("%s\r\n",event);
#endif
                result=strtok(event,":");
                instance=strtok(NULL,",");
                msgid=strtok(NULL,",");
                objectid=strtok(NULL,",");

                //ѭ������豸��������
                for(; i<data_stream.dev_len; i++)
                {
                    //�жϵ�ǰobjectid �Ƿ�Ϊ discover �ظ��� objectid
                    if(data_stream.dev[i].objectid==atoi(objectid))
                    {
                        j=0;
                        //�жϵ�ǰ resourceid �Ƿ��ϱ���
                        for(; j<resourcecount; j++)
                        {
                            //��� resourceidû���ϱ���
                            if(str[j]!=data_stream.dev[i].resourceid)
                                status=1;		//�ϱ���ʶ��һ
                            else
                            {
                                status=0;
                                break;
                            }
                        }
                        //�жϸ� resourceid �Ƿ���Ҫ�ϱ�
                        if(status)
                        {
                            //��¼��Ҫ�ϱ��� resourceid
                            str[resourcecount++]=data_stream.dev[i].resourceid;

                            //�жϱ����ϱ��� resourceid �Ƿ�Ϊ��һ���ҵ�
                            if(resourcecount-1)
                                rt_sprintf(resourcemap,"%s;%d",resourcemap,data_stream.dev[i].resourceid);
                            else
                                rt_sprintf(resourcemap,"%s%d",resourcemap,data_stream.dev[i].resourceid);
                        }
                    }
                    //			}
                }
#ifdef QSDK_USING_DEBUD
                j=0;
                //ѭ����ӡ�Ѿ���¼�� resourceid
                for(; j<resourcecount; j++)
                    LOG_D("resourcecount=%d\r\n",str[j]);

                //��ӡ resourceid map
                LOG_D("map=%s\r\n",resourcemap);
#endif

#ifdef QSDK_USING_ME3616
                LOG_D("AT+MIPLDISCOVERRSP=%d,%d,1,%d,\"%s\"\r\n",data_stream.instance,atoi(msgid),strlen(resourcemap),resourcemap);
                if(at_exec_cmd(at_resp,"AT+MIPLDISCOVERRSP=%d,%d,1,%d,\"%s\"",data_stream.instance,atoi(msgid),strlen(resourcemap),resourcemap)!=RT_EOK)
                {
                    LOG_D("+MIPLDISCOVERRSP  failure \r\n");
                    data_stream.discover_status=qsdk_onenet_status_failure;
                }
#endif
                data_stream.discovercount++;
                data_stream.discover_status=qsdk_onenet_status_run;

//				LOG_D("discover_count=%d    objcoun=%d",data_stream.discovercount,data_stream.objectcount);
                //�ж� discover �¼��Ƿ����
                if(data_stream.discovercount==data_stream.objectcount)
                {
                    //discover �¼��Ѿ����
#ifdef QSDK_USING_LOG
                    LOG_D("onenet connect success\r\n");
#endif
                    //onenet ���ӳɹ�
                    data_stream.discover_status=qsdk_onenet_status_success;
                    data_stream.connect_status=qsdk_onenet_status_success;
                }
            }
#endif
        }
        else
            LOG_D("event_mail recv fail\r\n");
    }
}
/*************************************************************
*	�������ƣ�	nb_dis_error
*
*	�������ܣ�	��ӡģ���ʼ��������Ϣ
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����
*************************************************************/
void qsdk_nb_dis_error(void)
{
    switch(nb_device.error)
    {
    case qsdk_nb_status_module_init_ok:
        LOG_D("ģ���ʼ���ɹ�\r\n");
        break;
    case qsdk_nb_status_io_init_failure:
        LOG_D("ģ�����ų�ʼ��ʧ��\r\n");
        break;
    case qsdk_nb_status_create_at_resp_failure:
        LOG_D("����AT��Ӧ�ṹ��ʧ��\r\n");
        break;
    case qsdk_nb_status_create_event_mail_failure:
        LOG_D("�����¼�����ʧ��\r\n");
        break;
    case qsdk_nb_status_create_hand_fun_failure:
        LOG_D("�����¼���Ӧ����ʧ��\r\n");
        break;
    case qsdk_nb_status_no_find_nb_module:
        LOG_D("û���ҵ�NB-IOTģ��\r\n");
        break;
    case qsdk_nb_status_set_low_power_failure:
        LOG_D("���õ͹���ģʽʧ��\r\n");
        break;
    case qsdk_nb_status_module_start_failure:
        LOG_D("ģ�鿪��ʧ��\r\n");
        break;
    case qsdk_nb_status_no_find_sim:
        LOG_D("û��ʶ��SIM��\r\n");
        break;
    case qsdk_nb_status_read_module_imei_failure:
        LOG_D("��ȡģ��IMEIʧ��\r\n");
        break;
    case qsdk_nb_status_module_no_find_csq:
        LOG_D("ģ��û���ҵ��ź�\r\n");
        break;
    case qsdk_nb_status_set_net_start_failure:
        LOG_D("�ֶ���������ʧ��\r\n");
        break;
    case qsdk_nb_status_fine_net_start_failure:
        LOG_D("ģ�鸽������ʧ��\r\n");
        break;
    case qsdk_nb_status_get_ntp_time_failure:
        LOG_D("��ȡNTPʱ��ʧ��\r\n");
        break;
#ifdef QSDK_USING_IOT
    case qsdk_iot_status_set_mini_sim_failure:
        LOG_D("����ģ����С����ʧ��\r\n");
        break;
    case qsdk_iot_status_set_address_failure:
        LOG_D("����NCDP������ʧ��\r\n");
        break;
    case qsdk_iot_status_open_update_dis_failure:
        LOG_D("�����ϱ�����ʧ��\r\n");
        break;
    case qsdk_iot_status_open_down_dis_failure:
        LOG_D("�����·�����ʧ��\r\n");
        break;
    case qsdk_iot_status_notify_failure:
        LOG_D("�������ݵ�IOTƽ̨ʧ��\r\n");
        break;
#endif
    default:
        break;
    }
}
