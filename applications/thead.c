/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-16     111       the first version
 */

#include <thead.h>
#include <rtdevice.h>
#include <rtthread.h>
#include <board.h>
#include "drv_common.h"
#define LEDR_PIN GET_PIN(B, 5)
#define LEDG_PIN GET_PIN(B, 0)
#define LEDB_PIN GET_PIN(B, 1)
double speed_float=0.2;
double dir_float=0.5;


#define THREAD_PRIORITY 25
#define THREAD_STACK_SIZE 512
#define THREAD_TIMESLICE 5
int HZ=100;//数据处理频率
#define ADC_DEV_NAME "adc1" /* ADC 设 备 名 称 */
/*
 * C2 转向
 * C3 速度
 * ADC 通 道
 * C4 C5 保留
 *   */
#define ADC_DEV_CHANNEL_C2 0   //上 3.3v 下 0v
#define ADC_DEV_CHANNEL_C3 1  // 左右

#define REFER_VOLTAGE 330 /* 参 考 电 压 3.3V,数 据 精 度 乘 以100保 留2位 小 数*/
#define CONVERT_BITS (1 << 12) /* 转 换 位 数 为12位 */
#define time_get 1
static rt_thread_t tid1 = RT_NULL;
double C2_max_v=0;
double C2_min_v=0;
double C2_mid_v=0;
double C3_max_v=0;
double C3_min_v=0;
double C3_mid_v=0;
extern rt_sem_t nrf_sem;
char  command_dir_pool[4];
char  command_move_pool[4];
//读取电压函数
double get_ADC_V(rt_adc_device_t adc_dev,int ADC_DEV_CHANNEL)
{
   double value,vol;
    /* 读 取 采 样 值 */
    value = rt_adc_read(adc_dev, ADC_DEV_CHANNEL);
    /* 转 换 为 对 应 电 压 值 */
    vol = value * REFER_VOLTAGE / CONVERT_BITS/100;
    return vol;
}

static void thread1_entry(void *parameter) {

            //进行ADC数据的读取  并根据变化情况并发送给nrf24l01
            rt_adc_device_t adc_dev;
            //rt_uint32_t value, vol;
            rt_err_t ret = RT_EOK;
            rt_pin_mode(LEDR_PIN,  PIN_MODE_OUTPUT);
            rt_pin_mode(LEDG_PIN,  PIN_MODE_OUTPUT);
            rt_pin_mode(LEDB_PIN,  PIN_MODE_OUTPUT);
            rt_pin_write(LEDR_PIN, PIN_LOW);//亮红灯
            rt_pin_write(LEDG_PIN, PIN_HIGH);
            rt_pin_write(LEDB_PIN, PIN_HIGH);


            /* 查 找 ADC设 备 */
            adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
            if (adc_dev == RT_NULL) {
            rt_kprintf("adc sample run failed! can't find %s device!\n", ADC_DEV_NAME);
            return RT_ERROR; }
            /* 使 能 设 备 */
            ret = rt_adc_enable(adc_dev, ADC_DEV_CHANNEL_C2|ADC_DEV_CHANNEL_C3);
            /*校准摇杆
             * */
            rt_kprintf("摇杆校准开始..........\n");

            /*c2 校准*/
            rt_pin_write(LEDR_PIN, PIN_HIGH);//亮红灯

            rt_kprintf("请移动到最下方\n");
            rt_thread_mdelay(1000);

            double tmp=get_ADC_V(adc_dev, ADC_DEV_CHANNEL_C2);
            //电压小于3.1 堵塞
            while(get_ADC_V(adc_dev, ADC_DEV_CHANNEL_C2)<3.1){
                rt_pin_write(LEDG_PIN, PIN_LOW);//亮绿灯
            };
            rt_pin_write(LEDG_PIN, PIN_HIGH);//灭绿灯
            rt_pin_write(LEDB_PIN, PIN_LOW);//亮蓝灯
            rt_thread_mdelay(500);
            for(int i=0;i<10;i++)
             {
               C2_max_v=C2_max_v+get_ADC_V(adc_dev, ADC_DEV_CHANNEL_C2);
               rt_kprintf("%f\n",C2_max_v);
               rt_thread_mdelay(time_get);
               rt_kprintf("第%d次数据采集成功\n",i);
             }
            rt_kprintf("请松开摇杆\n");
            rt_thread_mdelay(1000);

            rt_kprintf("请移动到最上方\n");
            rt_thread_delay(1000);
             while(get_ADC_V(adc_dev, ADC_DEV_CHANNEL_C2)>1)
                 {
                 rt_pin_write(LEDG_PIN, PIN_LOW);//亮绿灯
                 };//电压大于1 堵塞
             rt_pin_write(LEDG_PIN, PIN_HIGH);//灭绿灯
              rt_pin_write(LEDB_PIN, PIN_LOW);//亮蓝灯
             rt_thread_mdelay(500);
             for(int i=0;i<10;i++)
             {
                C2_min_v+=get_ADC_V(adc_dev, ADC_DEV_CHANNEL_C2);
                rt_kprintf("%f\n",C2_min_v);
                rt_thread_mdelay(time_get);
                rt_kprintf("第%d次数据采集成功\n",i);
             }
             rt_kprintf("请松开摇杆\n");
             rt_thread_mdelay(1000);
             C2_max_v=C2_max_v/10;
             C2_min_v=C2_min_v/10;
             /*c3 校准*/
             rt_kprintf("请移动到最右方\n");
             rt_thread_mdelay(50);
             while(get_ADC_V(adc_dev, ADC_DEV_CHANNEL_C3)<3.1)
                 {
                 rt_pin_write(LEDG_PIN, PIN_LOW);//亮绿灯
                 };//电压小于3.1 堵塞
             rt_pin_write(LEDG_PIN, PIN_HIGH);//灭绿灯
             rt_pin_write(LEDB_PIN, PIN_LOW);//亮蓝灯
             rt_thread_mdelay(500);
             for(int i=0;i<10;i++)
              {
                C3_max_v+=get_ADC_V(adc_dev, ADC_DEV_CHANNEL_C3);
                rt_kprintf("%f\n",C3_max_v);
                rt_thread_mdelay(time_get);
                rt_kprintf("第%d次数据采集成功\n",i);
              }
             rt_kprintf("请松开摇杆\n");
             rt_thread_mdelay(1000);

             rt_kprintf("请移动到最左\n");
             rt_thread_delay(50);
              while(get_ADC_V(adc_dev, ADC_DEV_CHANNEL_C3)>0.5){
                  rt_pin_write(LEDG_PIN, PIN_LOW);//亮绿灯
              };//电压大于0.5 堵塞
              rt_pin_write(LEDG_PIN, PIN_HIGH);//灭绿灯
              rt_pin_write(LEDB_PIN, PIN_LOW);//亮蓝灯
              rt_thread_mdelay(500);
              for(int i=0;i<10;i++)
              {
                 C3_min_v+=get_ADC_V(adc_dev, ADC_DEV_CHANNEL_C3);
                 rt_kprintf("%f\n",C3_min_v);
                 rt_thread_mdelay(time_get);
                 rt_kprintf("第%d次数据采集成功\n",i);
              }
              rt_kprintf("请松开摇杆\n");
              rt_pin_write(LEDG_PIN, PIN_LOW);//亮绿灯
              rt_thread_mdelay(1000);
              //采集中位数
              for(int i=0;i<3;i++)
              {
                  rt_kprintf("请拨动左摇杆，并放开\n");
                  rt_thread_mdelay(2000);
                  double tmp=get_ADC_V(adc_dev, ADC_DEV_CHANNEL_C2);
                  C2_mid_v+=tmp;
                  rt_kprintf("采集中，结果为%f\n",tmp);
              }

              for(int i=0;i<3;i++)
              {
                  rt_kprintf("请拨动右摇杆，并放开\n");
                  rt_thread_mdelay(2000);
                  double tmp=get_ADC_V(adc_dev, ADC_DEV_CHANNEL_C3);
                  C3_mid_v+=tmp;
                  rt_kprintf("采集中，结果为%f\n",tmp);
              }
              C2_mid_v=C2_mid_v/3;
              C3_mid_v=C3_mid_v/3;
              rt_kprintf("结果为%f\n",C2_mid_v);
              rt_kprintf("结果为%f\n",C3_mid_v);
              rt_pin_write(LEDG_PIN, PIN_HIGH);//亮绿灯
              rt_thread_mdelay(1000);
              C3_max_v=C3_max_v/10;
              C3_min_v=C3_min_v/10;
              /* rt_kprintf("%f\n",C3_max_v);
              rt_kprintf("%f\n",C3_min_v);
              rt_kprintf("%f\n",C2_max_v);
              rt_kprintf("%f\n",C2_min_v);
               * */

            //循环读取速度和方向ADC值
            while(1)
            {

                double dir_v,speed_v;
                /*dir 50  正前方 30 ~ 50 ~ 70
                 *direction 0 停止 1 前进 2 后退
                 * */
                int speed=0,direction=0,dir=50;
                //得到速度电压值   处理速度
                speed_v = get_ADC_V(adc_dev, ADC_DEV_CHANNEL_C2);
                if(speed_v>=C2_mid_v){//后退电压
                    if(speed_v-C2_mid_v>speed_float){//超出合理间隙范围
                        double tmp=speed_v-C2_mid_v;
                        tmp=tmp/(C2_max_v-C2_mid_v);
                        if(tmp==1){
                            tmp=0.99;
                        }
                        speed=tmp*100;
                        direction=2;
                    }else{//未超出合理间隙范围
                        direction=0;
                    }
                }else{//前进电压  C2_mid_v  >   speed_v
                    if(C2_mid_v-speed_v>speed_float){//超出合理间隙范围
                        double tmp=C2_mid_v-speed_v;
                        tmp=tmp/(C2_max_v-C2_mid_v);
                        if(tmp==1){
                            tmp=0.99;
                        }

                        speed=tmp*100;
                        direction=1;
                         }
                    else{//未超出合理间隙范围
                        direction=0;
                         }
                }
                 //rt_kprintf("speed:%d\n",speed);
                 //rt_kprintf("direction: %d \n",direction);
                //获得方向电压 处理方向数据
                dir_v= get_ADC_V(adc_dev, ADC_DEV_CHANNEL_C3);
                if(dir_v>=C3_mid_v){//右转电压
                                    if(dir_v-C3_mid_v>dir_float){//超出合理间隙范围
                                        double tmp=dir_v-C3_mid_v;
                                        tmp=tmp/(C3_max_v-C3_mid_v);
                                        dir=tmp*20+50;
                                    }else{//未超出合理间隙范围
                                        dir=50;
                                    }
                                }else{//左转电压  C3_mid_v  >   dir_v
                                    if(C2_mid_v-dir_v>dir_float){//超出合理间隙范围
                                        double tmp=C3_mid_v-dir_v;
                                        tmp=tmp/(C3_max_v-C3_mid_v);
                                        dir=50-tmp*20;
                                         }
                                    else{//未超出合理间隙范围
                                           dir=50;
                                         }
                                }
                                 //rt_kprintf("dir:%d\n",dir);
                //解析速度和方向
                command_move_pool[0]='0';
                command_dir_pool[0]='1';
                switch (direction) {
                    case 0:
                        command_move_pool[1]='0';
                        break;
                    case 1:
                        command_move_pool[1]='1';
                        break;
                    case 2:
                        command_move_pool[1]='2';
                        break;
                    default:command_move_pool[1]='0';
                        break;
                }
                command_move_pool[2]=speed/10+'0';
                command_move_pool[3]=speed%10+'0';

                command_dir_pool[1]=dir/10+'0';
                command_dir_pool[2]=dir%10+'0';
                command_dir_pool[3]='0';
                //rt_kprintf("command \n");
                //rt_kprintf(command_move_pool);
                //rt_kprintf("\n");

                //释放信号量
                rt_sem_release(nrf_sem);
                //rt_kprintf("release sem ok\n");
                rt_thread_mdelay(1000/HZ);//控制处理频率

            }

}

void thead_creat(){

    /* 创 建 线 程 1， 名 称 是 thread1， 入 口 是 thread1_entry*/
    tid1 = rt_thread_create("thread1",
    thread1_entry, RT_NULL,
    1024,
    THREAD_PRIORITY, THREAD_TIMESLICE);
    /* 如 果 获 得 线 程 控 制 块， 启 动 这 个 线 程 */
    if (tid1 != RT_NULL)
    rt_thread_startup(tid1);

};
