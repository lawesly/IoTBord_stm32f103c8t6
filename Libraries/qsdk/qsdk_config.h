/*
 * File      : qsdk_config.c
 * This file is part of config in qsdk
 * Copyright (c) 2018-2030, longmain Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-14     longmain     first version
 */

#ifndef __QSDK_CONFIG_H__

#define __QSDK_CONFIG_H__

/*********************************
*	NB-IOTģ���Լ�����ѡ��
*********************************/
//#define QSDK_USING_M5310

//#define QSDK_USING_M5310A

#define QSDK_USING_ME3616

//ģ�鲨����ѡ��
#define USART2_BOUND        BAUD_RATE_9600
//#define USART2_BOUND        BAUD_RATE_115200


/*********************************
*
*	ONENET ����ѡ��
*
*********************************/
//����ONENET֧�ֹ���
#define QSDK_USING_ONENET

//����objectID�������
#define QSDK_MAX_OBJECT_COUNT		10

//onenet������������Ϣ
#ifdef QSDK_USING_ME3616
#define QSDK_ONENET_ADDRESS      "183.230.40.39"
#else
#define QSDK_ONENET_ADDRESS      "183.230.40.40"
#endif
#define QSDK_ONENET_PORT			"5683"

/*********************************
*
*	IOT����ѡ��
*
*********************************/

//��������IOT֧�ֹ���
//#define QSDK_USING_IOT
#ifdef QSDK_USING_IOT
#define QSDK_IOT_ADDRESS      "117.60.157.137"
#else
#define QSDK_IOT_ADDRESS      "0.0.0.0"
#endif
#define QSDK_IOT_PORT					"5683"

//ME3616ע�ᳬʱʱ��
#define QSDK_IOT_REG_TIME_OUT			90

/*********************************
*
*	NET����ѡ��
*
*********************************/
//����UDP/TCP����
//#define QSDK_USING_NET

/*********************************
*
*	SYS����ѡ��
*
*********************************/
//����LOG��ʾ
#define QSDK_USING_LOG

//��������ģʽ
#define QSDK_USING_DBUG
#define QSDK_USING_DEBUD
//����TCP/UDP�����ճ���
#define QSDK_NET_REV_MAX_LEN		512

//����ģ����������ݳ���
#define QSDK_CMD_REV_MAX_LEN		1000

//����ʱ����
#define QSDK_TIME_ZONE  8


#endif	//qsdk_config.h end


