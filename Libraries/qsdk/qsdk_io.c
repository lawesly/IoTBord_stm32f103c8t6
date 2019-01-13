/*
 * File      : qsdk_io.c
 * This file is part of io in qsdk
 * Copyright (c) 2018-2030, longmain Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-14     longmain     first version
 */

#include "qsdk_io.h"
/*************************************************************
*	�������ƣ�	nb_hw_io_init
*
*	�������ܣ�	NB-IOT ģ��������ų�ʼ��
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1 ʧ��
*
*	˵����
*************************************************************/
int qsdk_hw_io_init(void)
{
	rt_pin_mode(NB_POWER_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(NB_RESET_PIN, PIN_MODE_OUTPUT);
	
    //�������� pwrkey �����ϵ�
	rt_pin_write(NB_POWER_PIN,PIN_HIGH);
    rt_thread_mdelay(500);
    //��λ NB-IOT ģ��
    if(qsdk_hw_io_reboot()==RT_EOK)
        return RT_EOK;
    else return RT_ERROR;
}

/*************************************************************
*	�������ƣ�	qsdk_nb_hw_io_reboot
*
*	�������ܣ�	NB-IOT ģ������
*
*	��ڲ�����	��
*
*	���ز�����	0 �ɹ�  1 ʧ��
*
*	˵����
*************************************************************/
int qsdk_hw_io_reboot(void)
{
	rt_pin_write(NB_RESET_PIN, PIN_HIGH);
    rt_thread_mdelay(500);
    rt_pin_write(NB_RESET_PIN, PIN_LOW);
    rt_thread_mdelay(5000);
    return RT_EOK;
}
