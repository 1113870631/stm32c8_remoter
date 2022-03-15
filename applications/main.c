/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-13     RT-Thread    first version
 */

#include <rtthread.h>
#include <thead.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
#include <board.h>
#include <rtdevice.h>

rt_sem_t nrf_sem;
int main(void)
{
    //创建给nrfdemo 的信号量
     nrf_sem = rt_sem_create("nrf_sem",0,RT_IPC_FLAG_FIFO);
     LOG_D("nrfdemo sem creat ok\n");
     rt_thread_delay(2000);
     LOG_D("delay ok \n");
     thead_creat();
     return RT_EOK;
}
