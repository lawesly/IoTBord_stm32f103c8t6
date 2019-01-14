/*
 * File      : qsdk_onenet.c
 * This file is part of onenet in qsdk
 * Copyright (c) 2018-2030, longmain Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-14     longmain     first version
 */
#define LOG_TAG              "qsdk_onenet"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>
#include "qsdk.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef QSDK_USING_ONENET

extern at_response_t at_resp;

//����������ƿ�
extern rt_thread_t hand_thread_id;

//����������ƿ�
extern rt_mailbox_t event_mail;

//��ʼ���������ṹ��
DATA_STREAM data_stream= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,NULL,NULL,NULL,NULL};

//ע�������ɺ���Ԥ����
static int head_node_parse(char *str);
static int init_node_parse(char *str);
static int net_node_parse(char *str);
static int sys_node_parse(char *str);
/*
************************************************************
*	�������ƣ�	onenet_init
*
*	�������ܣ�	���onenet��ʼ�����񣬲������ӵ�onenetƽ̨
*
*	��ڲ�����	*device  �豸�����������ṹ��
*
*	��ڲ�����	len �ṹ�峤��
*
*	��ڲ�����	lifetime  �豸��¼��ʱʱ��
*
*   ��ڲ�����	*config_t	�豸ע����
*
*	���ز�����	0 �ɹ�		1  ʧ��
*
*	˵����
************************************************************
*/
int qsdk_onenet_init(DEVICE *device,int len,int lifetime)
{
    int count;
    memset(&data_stream,0,sizeof(data_stream));		 //����������ṹ��
    data_stream.dev=device;												 //�豸����ӳ��
    data_stream.dev_len=len;											 //�豸�������ȹ���
    data_stream.connect_status=qsdk_onenet_status_init;			 //onenet����״̬��ʼ��
    data_stream.initstep=0;												 //��ʼ����������Ϊ0
    data_stream.write_callback=qsdk_onenet_write_rsp_callback; //write�ص�����ӳ��
    data_stream.read_callback=qsdk_onenet_read_rsp_callback;	 //read �ص�����ӳ��
    data_stream.execute_callback=qsdk_onenet_exec_rsp_callback;//exec �ص�����ӳ��
    if(qsdk_create_onenet_instance()!=RT_EOK)	 //���� instance
    {
        data_stream.error=1;
        goto failure;
    }
    data_stream.initstep=1;
    if(qsdk_create_onenet_object()!=RT_EOK)							//���� object
    {
        data_stream.error=2;
        goto failure;
    }
//���ģ����M5310����Ҫ�ϱ�����
#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
    data_stream.initstep=2;
    if(qsdk_notify_data_to_onenet(0)!=RT_EOK)						// notify �豸������ģ��
    {
        data_stream.error=3;
        goto failure;
    }
#endif
    data_stream.initstep=3;
    if(qsdk_onenet_open(lifetime)!=RT_EOK)								// ִ�� onenet ��¼����
    {
        data_stream.error=4;
        goto failure;
    }
    data_stream.initstep=4;
    data_stream.event_status=qsdk_onenet_status_init;
    count=300;
    do {
        count--;
        rt_thread_mdelay(100);
    } while(data_stream.event_status==qsdk_onenet_status_init&& count>0);		//�豸���� event��ʼ��

    if(data_stream.event_status==qsdk_onenet_status_init||count<=0||data_stream.event_status==qsdk_onenet_status_failure)
    {
        data_stream.error=5;
        goto failure;
    }
    else
    {
#ifdef QSDK_USING_DEBUD
        LOG_D("onenet reg success\r\n");
#endif
        data_stream.initstep=5;
        count=300;
    }
    do {
        count--;
        rt_thread_mdelay(100);
    } while(data_stream.event_status==qsdk_onenet_status_run&&count>0);	//�ȴ�ģ�鷵�� ��¼�ɹ��� event�¼�
    if(data_stream.event_status==qsdk_onenet_status_run)
    {
        data_stream.error=6;
        goto failure;
    }
    else
    {
#ifdef QSDK_USING_DEBUD
        LOG_D("open run success\r\n");
#endif
        data_stream.initstep=6;
        count=300;
    }

    if(data_stream.event_status==qsdk_onenet_status_success)					// �ж� onenet�Ƿ񷵻�ע��ɹ� event
    {
#ifdef QSDK_USING_DEBUD
        LOG_D("open success\r\n");
#endif
        data_stream.initstep=7;
        count=300;
    }
    else
    {
        data_stream.error=7;
        goto failure;
    }

    do {
        count--;
        rt_thread_mdelay(100);
    } while(data_stream.observer_status==qsdk_onenet_status_init&& count>0);			//�豸���� observer��ʼ��

    if(data_stream.observer_status==qsdk_onenet_status_init)
    {
        data_stream.error=8;
        goto failure;
    }
    else
    {
#ifdef QSDK_USING_DEBUD
        LOG_D("observer init success\r\n");
#endif
        data_stream.initstep=8;
        count=300;
    }
    do {
        count--;
        rt_thread_mdelay(100);
    } while(data_stream.observer_status==qsdk_onenet_status_run&&count>0);		//�ж��豸�Ƿ��յ� observer��Ϣ
    if(data_stream.observer_status==qsdk_onenet_status_run)
    {
        data_stream.error=9;
        goto failure;
    }
    else
    {
#ifdef QSDK_USING_DEBUD
        LOG_D("observer run success\r\n");
#endif
        data_stream.initstep=9;
        count=300;
    }

    if(data_stream.observer_status==qsdk_onenet_status_success)				//�ж� observer��ʼ���Ƿ�ɹ�
    {
#ifdef QSDK_USING_DEBUD
        LOG_D("observer success\r\n");
#endif
        data_stream.initstep=10;
        count=300;
    }
    else
    {
        data_stream.error=10;
        goto failure;
    }
    do {
        count--;
        rt_thread_mdelay(100);
    } while(data_stream.discover_status==qsdk_onenet_status_init&& count>0);//�ȴ������ discover	��ʼ��

    if(data_stream.discover_status==qsdk_onenet_status_init)	//�ж��Ƿ���� discover��ʼ��
    {
        data_stream.error=11;
        goto failure;
    }
    else
    {
#ifdef QSDK_USING_DEBUD
        LOG_D("discover init success\r\n");
#endif
        data_stream.initstep=11;
        count=300;
    }
    do {
        count--;
        rt_thread_mdelay(100);
    } while(data_stream.discover_status==qsdk_onenet_status_run&&count>0);//�ȴ�ģ�鷵�� discover��Ϣ

    if(data_stream.discover_status==qsdk_onenet_status_run)
    {
        data_stream.error=12;
        goto failure;
    }
    else
    {
#ifdef QSDK_USING_DEBUD
        LOG_D("discover run success\r\n");
#endif
        data_stream.initstep=12;
    }

    if(data_stream.discover_status==qsdk_onenet_status_success)	//�ж�	discover �Ƿ��ʼ���ɹ�
    {
#ifdef QSDK_USING_DEBUD
        LOG_D("discover success\r\n");
#endif
        data_stream.initstep=13;
        return RT_EOK;
    }
    else
    {
        data_stream.error=13;
        goto failure;
    }

//��������
failure:
    if(data_stream.initstep>3)				//�������ֵ���� 3 ���ر��豸 instance
    {
        if(qsdk_onenet_close()!=RT_EOK)
            LOG_D("close onenet instance failure\r\n");
    }
    if(qsdk_delete_onenet_instance()!=RT_EOK)				//ɾ�� ģ�� instance
        LOG_D("delete onenet instance failure\r\n");
#ifdef QSDK_USING_LOG
    qsdk_onenet_dis_error();							//��ӡ����ֵ��Ϣ
#endif
    rt_thread_delete(hand_thread_id);			//ɾ�� hand������
    rt_mb_delete(event_mail);			        //ɾ�� event ����
    return RT_ERROR;
}

/******************************************************
* �������ƣ� create_onenet_instance
*
*	�������ܣ� ����ģ��ʵ�� instance
*
* ��ڲ����� config_t �豸ע����
*
* ����ֵ�� 0 �ɹ�  1ʧ��
*
********************************************************/
int qsdk_create_onenet_instance(void)
{
    char str[100]="";
    char config_t[120]="";
    if(head_node_parse(config_t)==RT_EOK)
    {
        if(init_node_parse(str)==RT_EOK)
        {
            if(net_node_parse(str)==RT_EOK)
            {
                if(sys_node_parse(str)!=RT_EOK)
                {
                    data_stream.error=qsdk_onenet_status_reg_code_failure;
                    return RT_ERROR;
                }
            }
            else
            {
                data_stream.error=qsdk_onenet_status_reg_code_failure;
                return RT_ERROR;
            }
        }
        else
        {
            data_stream.error=qsdk_onenet_status_reg_code_failure;
            return RT_ERROR;
        }

    }
    else
    {
        data_stream.error=qsdk_onenet_status_reg_code_failure;
        return RT_ERROR;
    }


    rt_sprintf(config_t,"%s%04x%s",config_t,strlen(str)/2+3,str);

#ifdef QSDK_USING_DEBUD
    LOG_D("ע����==%s\r\n",config_t);
#endif

    //����ע�����ģ��
    if(at_exec_cmd(at_resp,"AT+MIPLCREATE=%d,%s,0,%d,0",strlen(config_t)/2,config_t,strlen(config_t)/2)!=RT_EOK) return RT_ERROR;

    //����ģ�鷵�ص���Ӧֵ
    at_resp_parse_line_args(at_resp,2,"+MIPLCREATE:%d",&data_stream.instance);

#ifdef QSDK_USING_LOG
    LOG_D("onenet create success,instance=%d\r\n",data_stream.instance);
#endif
    return RT_EOK;
}
/******************************************************
* �������ƣ� delete_onenet_instance
*
*	�������ܣ� ɾ��ģ��ʵ�� instance
*
* ��ڲ����� ��
*
* ����ֵ�� 0 �ɹ�  1ʧ��
*
********************************************************/
int qsdk_delete_onenet_instance(void)
{
    //����ɾ��instance ����
    if(at_exec_cmd(at_resp,"AT+MIPLDELETE=%d",data_stream.instance)!=RT_EOK) return RT_ERROR;

#ifdef QSDK_USING_LOG
    LOG_D("onenet instace delete success\r\n");
#endif
    return RT_EOK;
}
/******************************************************
* �������ƣ� create_onenet_object
*
*	�������ܣ� ��� object ��ģ��
*
* ��ڲ����� ��
*
* ����ֵ�� 0 �ɹ�  1ʧ��
*
********************************************************/
int qsdk_create_onenet_object(void)
{
    int i=0,j=0,status=1;
    int str[QSDK_MAX_OBJECT_COUNT];

    //ѭ����� object
    for(i = 0; i<data_stream.dev_len; i++)
    {
        //ѭ����ѯobject ������ӵ����飬���ڼ�¼����ֹ�ظ����
        for(j=0; j<data_stream.objectcount; j++)
        {
            if(str[j]!=data_stream.dev[i].objectid)
                status=1;
            else
            {
                status=0;
                break;
            }
        }
        //���object ��ģ��
        if(status)
        {
            //��¼��ǰobject
            str[data_stream.objectcount]=data_stream.dev[i].objectid;

            //objectid ��������һ��
            data_stream.objectcount++;
#ifdef QSDK_USING_ME3616
            data_stream.instancecount+=data_stream.dev[i].instancecount;
#else
            data_stream.instancecount=data_stream.dev_len;
#endif

            LOG_D("AT+MIPLADDOBJ=%d,%d,%d,\"%s\",%d,%d\r\n",data_stream.instance,data_stream.dev[i].objectid,data_stream.dev[i].instancecount,data_stream.dev[i].instancebitmap,data_stream.dev[i].attributecount,data_stream.dev[i].actioncount);
            //��ģ����� objectid
            if(at_exec_cmd(at_resp,"AT+MIPLADDOBJ=%d,%d,%d,\"%s\",%d,%d",data_stream.instance,data_stream.dev[i].objectid,data_stream.dev[i].instancecount,data_stream.dev[i].instancebitmap,data_stream.dev[i].attributecount,data_stream.dev[i].actioncount)!=RT_EOK) return RT_ERROR;
#ifdef QSDK_USING_LOG
            LOG_D("create object success id:%d\r\n",data_stream.dev[i].objectid);
#endif
        }
    }
#ifdef QSDK_USING_DEBUD
    LOG_D("object count=%d\r\n",data_stream.objectcount);
#endif

    return RT_EOK;
}
/******************************************************
* �������ƣ� delete_onenet_object
*
*	�������ܣ� ɾ��ģ������ע�� object
*
* ��ڲ����� objectid
*
* ����ֵ�� 0 �ɹ�  1ʧ��
*
********************************************************/
int qsdk_delete_onenet_object(int objectid)
{
    int i=0,j=0;
    //ѭ����ѯ����ɾ�� object
    for(i=0; i<data_stream.dev_len; i++)
    {
        //�жϵ�ǰ object �Ƿ�ΪҪɾ����object
        if(data_stream.dev[i].objectid==objectid)
        {
            //��������ɾ�� object
            if(at_exec_cmd(at_resp,"AT+MIPLDELOBJ=%d,%d",data_stream.instance,data_stream.dev[i].objectid)!=RT_EOK) return RT_ERROR;

            //ɾ���������豸��Ϣ�е� object
            for(j=i; j<data_stream.dev_len; j++)
            {
                //ɾ����ǰ object
                rt_memset(&data_stream.dev[j],0,sizeof(data_stream.dev[j]));
                //�жϺ���ᶼ����object������п�������ǰλ��
                if(j<data_stream.dev_len-1)
                    rt_memcpy(&data_stream.dev[j],&data_stream.dev[j+1],sizeof(data_stream.dev[j+1]));
            }
            //���������豸����-1
            data_stream.dev_len-=1;
#ifdef QSDK_USING_LOG
            LOG_D("delete onenet object success id:%d \r\n",objectid);
#endif
            return RT_EOK;
        }
    }
    return RT_ERROR;
}
/****************************************************
* �������ƣ� onenet_open
*
* �������ã� �豸��¼�� onenet ƽ̨
*
* ��ڲ����� lifetime �豸��onenet ά��ʱ��
*
* ����ֵ�� 0 �ɹ�	1ʧ��
*****************************************************/
int qsdk_onenet_open(int lifetime)
{
    data_stream.lifetime=lifetime;
    if(at_exec_cmd(at_resp,"AT+MIPLOPEN=%d,%d",data_stream.instance,data_stream.lifetime)!=RT_EOK) return RT_ERROR;
#ifdef QSDK_USING_LOG
    LOG_D("onenet open success\r\n");
#endif
    return RT_EOK;
}
/****************************************************
* �������ƣ� onenet_close
*
* �������ã� ��onenet ƽ̨ע���豸
*
* ��ڲ����� ��
*
* ����ֵ�� 0 �ɹ�	1ʧ��
*****************************************************/
int qsdk_onenet_close(void)
{
    if(at_exec_cmd(at_resp,"AT+MIPLCLOSE=%d",data_stream.instance)!=RT_EOK) return RT_ERROR;
#ifdef QSDK_USING_LOG
    LOG_D("onenet instance close success\r\n");
#endif
    return RT_EOK;
}
/****************************************************
* �������ƣ� time_onenet_update
*
* �������ã� ����onenet �豸ά��ʱ��
*
* ��ڲ����� ��
*
* ����ֵ�� 0 �ɹ�	1ʧ��
*****************************************************/
int qsdk_time_onenet_update(int flge)
{
    if(at_exec_cmd(at_resp,"AT+MIPLUPDATE=%d,%d,%d",data_stream.instance,data_stream.lifetime,flge)!=RT_EOK) return RT_ERROR;
#ifdef QSDK_USING_LOG
    LOG_D("onenet update time success\r\n");
#endif
    return RT_EOK;
}
/****************************************************
* �������ƣ� rsp_onenet_read
*
* �������ã� onenet read ��Ӧ
*
* ��ڲ����� msgid	��ϢID
*
*							objectid	����������objectid
*
*							instanceid	���������� instanceid
*
*							resourceid ���������� instanceid
*
* ����ֵ�� 0 �ɹ�	1ʧ��
*****************************************************/
int qsdk_rsp_onenet_read(int msgid,int objectid,int instanceid,int resourceid)
{
    int i=0,result=qsdk_onenet_status_result_Not_Found;

    //ѭ�����objectid
    for(; i<data_stream.dev_len; i++)
    {
        //�жϵ�ǰobjectid �Ƿ�Ϊ����Ҫ������Ϣ
        if(data_stream.dev[i].objectid==objectid&&data_stream.dev[i].instanceid==instanceid&&data_stream.dev[i].resourceid==resourceid)
        {
            //������ص�����
            if(data_stream.read_callback(objectid,instanceid,resourceid)==RT_EOK)
            {
                result=qsdk_onenet_status_result_read_success;
#ifdef QSDK_USING_LOG
                LOG_D("onenet read rsp success\r\n");
#endif
            }
            else	// read �ص���������ʧ�ܲ���
            {
                result=qsdk_onenet_status_result_Bad_Request;
                LOG_D("onenet read rsp failure\r\n");
            }
            break;
        }
    }
    //���� onenet read ��Ӧ
    if(at_exec_cmd(at_resp,"AT+MIPLREADRSP=%d,%d,%d,%d,%d,%d,%d,%d,\"%s\",0,0",data_stream.instance,msgid,result,data_stream.dev[i].objectid,
                   data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].valuetype,strlen(data_stream.dev[i].value),
                   data_stream.dev[i].value)!=RT_EOK) return RT_ERROR;

    return RT_EOK;
}
/****************************************************
* �������ƣ� rsp_onenet_write
*
* �������ã� onenet write ��Ӧ
*
* ��ڲ����� msgid	��ϢID
*
*							objectid	����������objectid
*
*							instanceid	���������� instanceid
*
*							resourceid ���������� instanceid
*
*							valuetype writeֵ����
*
*							len writeֵ����
*
*							value ����writeֵ
*
* ����ֵ�� 0 �ɹ�	1ʧ��
*****************************************************/
int qsdk_rsp_onenet_write(int msgid,int objectid,int instanceid,int resourceid,int valuetype,int len,char* value)
{
    int result=qsdk_onenet_status_result_Not_Found;;

    //���� write �ص�����
    if(data_stream.write_callback(objectid,instanceid,resourceid,len,value)==RT_EOK)
    {
        result=qsdk_onenet_status_result_write_success;
#ifdef QSDK_USING_LOG
        LOG_D("onenet write rsp success\r\n");
#endif
    }
    else	//write�ص���������ʧ�ܲ���
    {
        result=qsdk_onenet_status_result_Bad_Request;
        LOG_D("onenet write rsp failure\r\n");
    }

    //���� write	������Ӧ
    if(at_exec_cmd(at_resp,"AT+MIPLWRITERSP=%d,%d,%d",data_stream.instance,msgid,result)!=RT_EOK) return RT_ERROR;

    return RT_EOK;
}
/****************************************************
* �������ƣ� qsdk_rsp_onenet_execute
*
* �������ã� onenet execute ��Ӧ
*
* ��ڲ����� msgid	��ϢID
*
*							objectid	����������objectid
*
*							instanceid	���������� instanceid
*
*							resourceid ���������� instanceid
*
*							len cmdֵ����
*
*							cmd ����cmdֵ
*
* ����ֵ�� 0 �ɹ�	1ʧ��
*****************************************************/
int qsdk_rsp_onenet_execute(int msgid,int objeceid,int instanceid,int resourceid,int len,char *cmd)
{
    int result=qsdk_onenet_status_result_Not_Found;;

    //���� execute �ص�����
    if(data_stream.execute_callback(objeceid,instanceid,resourceid,len,cmd)==RT_EOK)
    {
        result=qsdk_onenet_status_result_write_success;
#ifdef QSDK_USING_LOG
        LOG_D("onenet execute rsp success\r\n");
#endif
    }
    else	//execute�ص���������ʧ�ܲ���
    {
        result=qsdk_onenet_status_result_Bad_Request;
        LOG_D("onenet execute rsp failure\r\n");
    }

    //���� execute	������Ӧ
    if(at_exec_cmd(at_resp,"AT+MIPLEXECUTERSP=%d,%d,%d",data_stream.instance,msgid,result)!=RT_EOK) return RT_ERROR;

    return RT_EOK;
}

int qsdk_rsp_onenet_parameter(int instance,int msgid,int result)
{


    return 0;
}
/****************************************************
* �������ƣ� notify_data_to_onenet
*
* �������ã� notify �豸���ݵ�ƽ̨����ACK��
*
* ��ڲ����� up_status �����ϱ�����  0 ȫ���ϱ�   1 ֻ�ϱ������ϱ���ʶ��
*
* ����ֵ�� 0 �ɹ�	1ʧ��
*****************************************************/
int qsdk_notify_data_to_onenet(_Bool up_status)
{
    int i=0;
    if(up_status!=0)	//�ж��Ƿ�Ϊֻ�ϱ����б�ʶ������
    {
        //ѭ������豸����
        for(i=0; i<data_stream.dev_len; i++)
        {
            //�жϵ�ǰ�豸�����Ƿ��������ϱ���ʶ
            if(data_stream.dev[i].up_status==1)
            {
                //�ϱ���ǰ�豸����
                if(at_exec_cmd(at_resp,"AT+MIPLNOTIFY=%d,%d,%d,%d,%d,%d,%d,\"%s\",0,%d",data_stream.instance,data_stream.dev[i].msgid,data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].valuetype,strlen(data_stream.dev[i].value),data_stream.dev[i].value,data_stream.dev[i].flge)!=RT_EOK)  return RT_ERROR;

                //��յ�ǰ�������ϱ���ʶ
                data_stream.dev[i].up_status=0;
#ifdef QSDK_USING_LOG
                //LOG_D("\r\n notify to onenet success, objectid:%d 	instanceid:%d 	resourceid:%d		msgid=%d   flge:%d",data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].msgid,data_stream.dev[i].flge);
                LOG_D("AT+MIPLNOTIFY=%d,%d,%d,%d,%d,%d,%d,\"%s\",0,%d\r\n",data_stream.instance,data_stream.dev[i].msgid,data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].valuetype,strlen(data_stream.dev[i].value),data_stream.dev[i].value,data_stream.dev[i].flge);
#endif
                rt_thread_mdelay(100);
            }
        }
        //����豸���ϱ���ʶ
        data_stream.update_status=0;
    }
    else if(up_status==0)	//����ϱ���ʶΪȫ����Ϣ�ϱ�
    {
        //ѭ���ϱ���Ϣ��ƽ̨
        for(i=0; i<data_stream.dev_len; i++)
        {
            //����AT�����ϱ���Ϣ��ƽ̨
            if(at_exec_cmd(at_resp,"AT+MIPLNOTIFY=%d,%d,%d,%d,%d,%d,%d,\"%s\",0,%d",data_stream.instance,data_stream.dev[i].msgid,data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].valuetype,strlen(data_stream.dev[i].value),data_stream.dev[i].value,data_stream.dev[i].flge)!=RT_EOK) return RT_ERROR;
#ifdef QSDK_USING_LOG
            //LOG_D("\r\n notify to onenet success, objectid:%d 	instanceid:%d 	resourceid:%d		msgid=%d   flge:%d",data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].msgid,data_stream.dev[i].flge);
            LOG_D("AT+MIPLNOTIFY=%d,%d,%d,%d,%d,%d,%d,\"%s\",0,%d\r\n",data_stream.instance,data_stream.dev[i].msgid,data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].valuetype,strlen(data_stream.dev[i].value),data_stream.dev[i].value,data_stream.dev[i].flge);
#endif
            rt_thread_mdelay(100);
        }
    }

    return RT_EOK;
}
/****************************************************
* �������ƣ� notify_ack_data_to_onenet
*
* �������ã� notify ���ݵ�onenetƽ̨����ACK��
*
* ��ڲ����� up_status �����ϱ�����  0 ȫ���ϱ�   1 ֻ�ϱ������ϱ���ʶ��
*
* ����ֵ�� 0 �ɹ�	1ʧ��
*****************************************************/
int qsdk_notify_ack_data_to_onenet(_Bool up_status)
{
    int i=0,count=300;
    if(up_status!=0)	//�ж��ϱ������Ƿ�Ϊֻ�ϱ����ϱ���ʶ��
    {
        //ѭ����Ⲣ���ϱ����ݵ�ƽ̨
        for(i=0; i<data_stream.dev_len; i++)
        {
            //�жϵ�ǰ�豸�����Ƿ���ϱ���ʶ
            if(data_stream.dev[i].up_status==1)
            {
                //����AT�����ϱ����ݵ� onenetƽ̨
                if(at_exec_cmd(at_resp,"AT+MIPLNOTIFY=%d,%d,%d,%d,%d,%d,%d,\"%s\",0,%d,%d",data_stream.instance,data_stream.dev[i].msgid,data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].valuetype,strlen(data_stream.dev[i].value),data_stream.dev[i].value,data_stream.dev[i].flge,data_stream.notify_ack)!=RT_EOK) return RT_ERROR;

                //�ȴ�ƽ̨����ACK
                do {
                    count--;
                    rt_thread_mdelay(100);
                } while(data_stream.notify_status==qsdk_onenet_status_init&&count>0);

                //�ж� notify ״̬�Ƿ�ʧ��
                if(data_stream.notify_status==qsdk_onenet_status_failure||count<0)
                {
                    LOG_D("ack notify to onenet failure, objectid:%d 	instanceid:%d 	resourceid:%d		msgid:%d\r\n",data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].msgid);
                    return RT_ERROR;
                }
                else if(data_stream.notify_status==qsdk_onenet_status_success)	//notify �ɹ������
                {
#ifdef QSDK_USING_LOG
                    LOG_D("ack notify to onenet success, objectid:%d 	instanceid:%d 	resourceid:%d		msgid:%d\r\n",data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].msgid);
#endif
                    data_stream.notify_status=qsdk_onenet_status_init;
                }
                //������ ack����һ��  ��objectid �ϱ���ʶ���
                data_stream.notify_ack++;
                data_stream.dev[i].up_status=0;
            }
        }
        //�豸���ϱ���ʶ���
        data_stream.update_status=0;
    }
    else if(up_status==0) //�ж��ϱ�״̬�Ƿ�Ϊȫ���ϱ�
    {
        //ѭ������豸��������
        for(i=0; i<data_stream.dev_len; i++)
        {
            //���� AT �����ϱ����ݵ� onenet ƽ̨
            if(at_exec_cmd(at_resp,"AT+MIPLNOTIFY=%d,%d,%d,%d,%d,%d,%d,\"%s\",0,%d,%d",data_stream.instance,data_stream.dev[i].msgid,data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].valuetype,strlen(data_stream.dev[i].value),data_stream.dev[i].value,data_stream.dev[i].flge,data_stream.notify_ack)!=RT_EOK) return RT_ERROR;

            //�ȴ�ģ�鷵�� ACK��Ӧ
            do {
                count--;
                rt_thread_mdelay(100);
            } while(data_stream.notify_status==qsdk_onenet_status_init&&count>0);

            //�ж� ACK ��Ӧ�Ƿ�Ϊʧ��
            if(data_stream.notify_status==qsdk_onenet_status_failure||count<0)
            {
                LOG_D("ack notify to onenet failure, objectid:%d 	instanceid:%d 	resourceid:%d		msgid:%d\r\n",data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].msgid);
                return RT_ERROR;
            }
            else if(data_stream.notify_status==qsdk_onenet_status_success) // ACK��Ӧ�ɹ������
            {
#ifdef QSDK_USING_LOG
                LOG_D("ack notify to onenet success, objectid:%d 	instanceid:%d 	resourceid:%d		msgid:%d\r\n",data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].msgid);
#endif
                //����ǰ�豸���������ϱ�״̬��Ϊ��ʼ��
                data_stream.notify_status=qsdk_onenet_status_init;
            }
            // ACK ����һ��
            data_stream.notify_ack++;
        }
    }
    //��ACK����5000 ʱ�����һ��
    if(data_stream.notify_ack>5000)
        data_stream.notify_ack=1;
    return RT_EOK;
}
/****************************************************
* �������ƣ� notify_data_to_status
*
* �������ã� notify ���ݵ����򻺴棬�������һ���ϱ�
*
* ��ڲ����� objectid ��Ҫд�뻺��� objectid
*
* 						instanceid ��Ҫд�뻺��� instanceid
*
* 						resourceid ��Ҫд�뻺��� resourceid
*
* ����ֵ�� 0 �ɹ�	1ʧ��
*****************************************************/
int qsdk_notify_data_to_status(int objectid,int instanceid,int resourceid)
{
    int i=0;

    //ѭ������豸��������
    for(i=0; i<data_stream.dev_len; i++)
    {
        //�жϵ�ǰ objectid �Ƿ�Ϊд�뻺��� objectid
        if(data_stream.dev[i].objectid==objectid&&data_stream.dev[i].instanceid==instanceid&&data_stream.dev[i].resourceid==resourceid)
        {
            //�������ϱ���ʶ�� 1
            data_stream.dev[i].up_status=1;
#ifdef QSDK_USING_LOG
            LOG_D("notify data to status success \r\n objectid=%d ,instanceid=%d ,resourcrid=%d \r\n",objectid,instanceid,resourceid);
#endif
            break;
        }
    }
    return RT_EOK;
}
/****************************************************
* �������ƣ� notify_urgent_event
*
* �������ã� �����������ݵ� onenetƽ̨
*
* ��ڲ����� objectid ��Ҫ���͵� objectid
*
* 						instanceid ��Ҫ���͵� instanceid
*
* 						resourceid ��Ҫ���͵� resourceid
*
* ����ֵ�� 0 �ɹ�	1ʧ��
*****************************************************/
int qsdk_notify_urgent_event(int objectid,int instanceid,int resourceid)
{
    int i=0;

    //ѭ������豸��������
    for(i=0; i<data_stream.dev_len; i++)
    {
        //�жϵ�ǰ objectid �Ƿ�Ϊ��Ҫ�ϱ��� objectid
        if(data_stream.dev[i].objectid==objectid&&data_stream.dev[i].instanceid==instanceid&&data_stream.dev[i].resourceid==resourceid)
        {
            //���� AT �����ϱ��� onenet ƽ̨
            if(at_exec_cmd(at_resp,"AT+MIPLNOTIFY=%d,%d,%d,%d,%d,%d,%d,\"%s\",0,%d",data_stream.instance,data_stream.dev[i].msgid,data_stream.dev[i].objectid,data_stream.dev[i].instanceid,data_stream.dev[i].resourceid,data_stream.dev[i].valuetype,strlen(data_stream.dev[i].value),data_stream.dev[i].value,data_stream.dev[i].flge)!=RT_EOK)  return RT_ERROR;

#ifdef QSDK_USING_LOG
            LOG_D("notify urgent event success \r\n objectid=%d ,instanceid=%d ,resourcrid=%d \r\n",objectid,instanceid,resourceid);
#endif
            break;
        }
    }
    return RT_EOK;
}
/****************************************************
* �������ƣ� get_onenet_version
*
* �������ã� ��ȡ onenetƽ̨�汾��
*
* ����ֵ�� 0 �ɹ�	1ʧ��
*****************************************************/
int qsdk_get_onenet_version(void)
{


    return 0;
}

/****************************************************
* �������ƣ� head_node_parse
*
* �������ã� ����ע����Э�� head ����
*
* ��ڲ����� str:���ɵ�Э���ļ�
*
* ����ֵ�� 0 �ɹ�	1ʧ��
*****************************************************/
static int head_node_parse(char *str)
{
    int ver=1;
    int config=3;
    rt_sprintf(str,"%d%d",ver,config);
    //LOG_D("%s\r\n",str);
    return RT_EOK;
}
/****************************************************
* �������ƣ� init_node_parse
*
* �������ã� ����ע����Э�� init ����
*
* ��ڲ����� str:���ɵ�Э���ļ�
*
* ����ֵ�� 0 �ɹ�	1ʧ��
*****************************************************/
static int init_node_parse(char *str)
{
    char *head="F";
    int config=1;
    int size=3;
    rt_sprintf(str+strlen(str),"%s%d%04x",head,config,size);
    //LOG_D("%s\r\n",str);

    return RT_EOK;
}
/****************************************************
* �������ƣ� net_node_parse
*
* �������ã� ����ע����Э�� net ����
*
* ��ڲ����� str:���ɵ�Э���ļ�
*
* ����ֵ�� 0 �ɹ�	1ʧ��
*****************************************************/
static int net_node_parse(char *str)
{
    char buffer[100];
    char *head="F";
    int config=2;
#ifdef QSDK_USING_ME3616
    int mtu_size=1280;
#else
    int mtu_size=1024;
#endif
    int link_t=1;
    int band_t=1;
    int boot_t=00;
    int apn_len=0;
    int apn=0;
    int user_name_len=0;
    int user_name=0;
    int passwd_len=0;
    int passwd=0;
    int host_len;
    char host[38];
#ifdef QSDK_USING_ME3616
    char *user_data="4e554c4c";
#else
    char *user_data="31";
#endif
    int user_data_len=strlen(user_data)/2;
#ifdef QSDK_USING_ME3616
    string_to_hex(QSDK_ONENET_ADDRESS,strlen(QSDK_ONENET_ADDRESS),host);
#else
    LOG_D(buffer,"%s:%s",QSDK_ONENET_ADDRESS,QSDK_ONENET_PORT);
    string_to_hex(buffer,strlen(buffer),host);
#endif
    host_len=strlen(host)/2;
    //LOG_D("%s  %d\r\n",host,host_len);
    rt_sprintf(buffer,"%04x%x%x%02x%02x%02x%02x%02x%02x%02x%02x%s%04x%s",mtu_size,link_t,band_t,boot_t,apn_len,apn,user_name_len,user_name,passwd_len,passwd,host_len,host,user_data_len,user_data);
    //LOG_D("%s\r\n",buffer);
    rt_sprintf(str+strlen(str),"%s%d%04x%s",head,config,strlen(buffer)/2+3,buffer);
    //LOG_D("%s\r\n",str);

    return 0;

}
/****************************************************
* �������ƣ� sys_node_parse
*
* �������ã� ����ע����Э�� sys ����
*
* ��ڲ����� str:���ɵ�Э���ļ�
*
* ����ֵ�� 0 �ɹ�	1ʧ��
*****************************************************/
static int sys_node_parse(char *str)
{
    char buffer[20];
    char *head="F";
    int config=3;
    int size;
#ifdef QSDK_USING_ME3616
    char *debug="ea0400";
#else
    char *debug="C00000";
#endif

#ifdef QSDK_USING_ME3616
    char *user_data="4e554c4c";
    int user_data_len=strlen(user_data)/2;
#else
    char *user_data="00";
    int user_data_len=0;
#endif

    if(user_data_len)
        rt_sprintf(buffer,"%s%04x%s",debug,user_data_len,user_data);
    else
        rt_sprintf(buffer,"%s%04x",debug,user_data_len);
    rt_sprintf(str+strlen(str),"%s%x%04x%s",head,config,strlen(buffer)/2+3,buffer);
    //LOG_D("%s\r\n",str);

    return 0;

}
/****************************************************
* �������ƣ� onenet_dis_error
*
* �������ã� ��ӡ������Ϣ
*
* ����ֵ�� ��
*****************************************************/
void qsdk_onenet_dis_error(void)
{
    switch(data_stream.error)
    {
    case 0:
        LOG_D("onenet sdk init success\r\n");
        break;
    case 1:
        LOG_D("onenet create instance failure\r\n");
        break;
    case 2:
        LOG_D("onenet add objece failure\r\n");
        break;
    case 3:
        LOG_D("onenet notify failure\r\n");
        break;
    case 4:
        LOG_D("onener open failure\r\n");
        break;
    case 5:
        LOG_D("onenet open init failure\r\n");
        break;
    case 6:
        LOG_D("onenet open run failure\r\n");
        break;
    case 7:
        LOG_D("onenet open failure\r\n");
        break;
    case 8:
        LOG_D("observe init failure\r\n");
        break;
    case 9:
        LOG_D("observe run failure\r\n");
        break;
    case 10:
        LOG_D("observe  failure\r\n");
        break;
    case 11:
        LOG_D("discover init failure\r\n");
        break;
    case 12:
        LOG_D("discover run failure\r\n");
        break;
    case 13:
        LOG_D("discover failure\r\n");
        break;
    case 14:
        LOG_D("no sim\r\n");
        break;
    case 15:
        LOG_D("no sim\r\n");
        break;
    case 16:
        LOG_D("no sim\r\n");
        break;
    case 17:
        LOG_D("no sim\r\n");
        break;
    case 18:
        LOG_D("no sim\r\n");
        break;
    default:
        break;
    }
}

#endif
