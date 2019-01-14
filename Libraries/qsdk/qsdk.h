/*
 * File      : qsdk.h
 * This file is part of fun in qsdk
 * Copyright (c) 2018-2030, longmain Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-14     longmain     first version
 */

#ifndef __QSDK_H__

#define __QSDK_H__

#include "qsdk_config.h"
#include <rtthread.h>
#include <at.h>
#include "main.h"
#include "qsdk_callback.h"
#include "qsdk_io.h"






enum qsdk_nb_status_type
{
    qsdk_nb_status_module_init_ok=0,
    qsdk_nb_status_io_init_failure,
    qsdk_nb_status_create_at_resp_failure,
    qsdk_nb_status_create_event_mail_failure,
    qsdk_nb_status_create_hand_fun_failure,
    qsdk_nb_status_no_find_nb_module,
    qsdk_nb_status_set_low_power_failure,
    qsdk_nb_status_module_start_failure,
    qsdk_nb_status_no_find_sim,
    qsdk_nb_status_read_module_imei_failure,
    qsdk_nb_status_module_no_find_csq,
    qsdk_nb_status_set_net_start_failure,
    qsdk_nb_status_fine_net_start_failure,
    qsdk_nb_status_get_ntp_time_failure,
    qsdk_iot_status_set_mini_sim_failure,
    qsdk_iot_status_set_address_failure,
#ifdef QSDK_USING_IOT
    qsdk_iot_status_open_update_dis_failure,
    qsdk_iot_status_open_down_dis_failure,
    qsdk_iot_status_reg_init,
    qsdk_iot_status_reg_failure,
    qsdk_iot_status_reg_success,
    qsdk_iot_status_observer_failure,
    qsdk_iot_status_observer_success,
    qsdk_iot_status_unreg_success,
    qsdk_iot_status_notify_init,
    qsdk_iot_status_notify_failure,
    qsdk_iot_status_notify_success,
    qsdk_iot_status_notify_timeout
#endif
};

enum  qsdk_onenet_value_type
{
    qsdk_onenet_value_String=1,
    qsdk_onenet_value_Opaque,
    qsdk_onenet_value_Integer,
    qsdk_onenet_value_Float,
    qsdk_onenet_value_Bool,
    qsdk_onenet_value_HexStr
};

enum qsdk_onenet_status_type
{
    qsdk_onenet_status_init=0,
    qsdk_onenet_status_run,
    qsdk_onenet_status_failure,
    qsdk_onenet_status_success=4,
    qsdk_onenet_status_reg_code_failure,
    qsdk_onenet_status_update_init=10,
    qsdk_onenet_status_update_failure,
    qsdk_onenet_status_update_success,
    qsdk_onenet_status_update_timeout=14,
    qsdk_onenet_status_update_need=18,
    qsdk_onenet_status_result_read_success=1,
    qsdk_onenet_status_result_write_success,
    qsdk_onenet_status_result_Bad_Request=11,
    qsdk_onenet_status_result_Unauthorized,
    qsdk_onenet_status_result_Not_Found,
    qsdk_onenet_status_result_Method_Not_Allowed,
    qsdk_onenet_status_result_Not_Acceptable
};


typedef struct
{
    int objectid;
    int instancecount;
    char *instancebitmap;
    int attributecount;
    int actioncount;
    int instanceid;
    int resourceid;
    int valuetype;
    int len;
    int flge;
    int msgid;
    int up_status;
    char *value;
} DEVICE;

//�������ṹ��
typedef struct
{
    int instance;
    int lifetime;
    int result;
    int error;
    int dev_len;
    int initstep;
    int objectcount;
    int instancecount;
    int observercount;
    int discovercount;
    int event_status;
    int observer_status;
    int discover_status;
    int update_status;
    int notify_status;
    int connect_status;
    int notify_ack;
    DEVICE *dev;
    int(*read_callback)(int objectid,int instanceid,int resourceid);
    int(*write_callback)(int objectid,int instanceid,int resourceid,int len,char* value);
    int(*execute_callback)(int objectid,int instanceid,int resourceid,int len,char* cmd);
} DATA_STREAM;



struct NB_DEVICE
{
    int  sim_state;
    int  device_ok;
    int  device_reboot;
    int  net_connect_ok;
    int  error;
    int  csq;
    int	 notify_status;
#ifdef QSDK_USING_IOT
    int	iot_connect_status;
#endif
    char imsi[16];
    char imei[16];
    char ip[16];
};

#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
struct NET_INFO
{
    int socket;
    char server_ip[16];
    int server_port;
    int rev_socket;
    int rev_len;

};
extern struct NET_INFO  net_info;
#endif

enum	QSDK_NET_DATA_TYPE
{
    QSDK_NET_TYPE_TCP=1,
    QSDK_NET_TYPE_UDP


};
extern rt_mailbox_t event_mail;
extern DATA_STREAM data_stream;
extern struct NB_DEVICE nb_device;



//qsdk ���л�����ʼ��
int qsdk_init_environment(void);

//qsdk ����
int qsdk_at_send_cmd(char *cmd,char *result);
int qsdk_at_resp_cmd(char *cmd,int line,char *result);
int qsdk_at_send_data(char *data);

//qsdk_nb_fun
int qsdk_nb_hw_init(void);
int qsdk_nb_wait_connect(void);
//int qsdk_nb_reboot(void);
int qsdk_nb_sim_check(void);
int qsdk_nb_set_psm_mode(char *tau_time,char *active_time);
int qsdk_nb_get_imsi(void);
int qsdk_nb_get_imei(void);
int qsdk_nb_get_time(void);
int qsdk_nb_get_csq(void);
int qsdk_nb_set_net_start(void);
int qsdk_nb_get_net_start(void);
int qsdk_nb_query_ip(void);

#ifdef QSDK_USING_ME3616_GPS
int qsdk_agps_config(void);
int qsdk_gps_config(void);
#endif
int string_to_hex(const char *pHex, int len, char *pString);
void qsdk_nb_set_rtc_time(int year,int month,int day,int hour,int min,int sec);
void qsdk_nb_dis_error(void);

//qsdk_iot_fun
#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
int qsdk_iot_check_address(void);
int qsdk_iot_set_address(void);
int qsdk_iot_open_update_status(void);
int qsdk_iot_open_down_date_status(void);
int qsdk_iot_send_date(char *str);
#endif

#ifdef QSDK_USING_ME3616
int qsdk_iot_reg(void);
int qsdk_iot_del_reg(void);
int qsdk_iot_update(char *str);
#endif



//qsdk_net_fun
#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
int qsdk_net_create_socket(int type,int port,char *server_ip,unsigned short server_port);
int qsdk_net_connect_to_server(void);
int qsdk_net_send_data(int type,char *str,unsigned int len);
int qsdk_net_rev_data(int port,int len);
int qsdk_net_close_socket(void);
#endif

#ifdef QSDK_USING_ME3616
int qsdk_net_create_socket(int type,int *socket);
int qsdk_net_connect_to_server(int socket,char *server_ip,unsigned short server_port);
int qsdk_net_send_data(int socket,char *str,unsigned int len);
int qsdk_net_close_socket(int socket);
#endif

//qsdk_onenet_fun

#ifdef QSDK_USING_ONENET

int qsdk_onenet_init(DEVICE *device,int len,int lifetime);
int qsdk_create_onenet_instance(void);
int qsdk_delete_onenet_instance(void);
int qsdk_create_onenet_object(void);
int qsdk_delete_onenet_object(int objectid);
int qsdk_onenet_open(int lifetime);
int qsdk_onenet_close(void);
int qsdk_time_onenet_update(int flge);
int qsdk_rsp_onenet_read(int msgid,int objectid,int instanceid,int resourceid);
int qsdk_rsp_onenet_write(int msgid,int objectid,int instanceid,int resourceid,int valuetype,int len,char* value);
int qsdk_rsp_onenet_execute(int msgid,int objeceid,int instanceid,int resourceid,int len,char *cmd);
int qsdk_rsp_onenet_parameter(int instance,int msgid,int result);
int qsdk_notify_data_to_onenet(_Bool up_status);
int qsdk_notify_data_to_status(int objectid,int instanceid,int resourceid);
int qsdk_notify_urgent_event(int objectid,int instanceid,int resourceid);
int qsdk_get_onenet_version(void);
void qsdk_onenet_dis_error(void);

#if	(defined QSDK_USING_M5310)||(defined QSDK_USING_M5310A)
int qsdk_notify_ack_data_to_onenet(_Bool up_status);
#endif

#endif

#endif	//qsdk.h end

