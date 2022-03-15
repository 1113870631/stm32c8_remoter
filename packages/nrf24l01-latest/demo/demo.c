#include <nrf24l01.h>
#include <rtconfig.h>
#include "board.h"
#include "stm32f1xx_hal_gpio.h"

extern char  command_dir_pool[4];
extern char  command_move_pool[4];
extern rt_sem_t nrf_sem;
char send_pool[8]={'0'};

#if (PKG_NRF24L01_DEMO_CE_PIN < 0)
#error Please specify a valid pin
#endif

#ifdef PKG_NRF24L01_DEMO_ROLE_PTX
    #define NRF24_DEMO_ROLE     ROLE_PTX
    #define NRF24_DEMO_SEND_INTERVAL        PKG_NRF24L01_DEMO_INTERVAL_SEND
#else
    #define NRF24_DEMO_ROLE     ROLE_PRX
#endif

#define NRF24_DEMO_SPI_DEV_NAME         PKG_NRF24L01_DEMO_SPI_DEV_NAME
#define NRF24_DEMO_CE_PIN               PKG_NRF24L01_DEMO_CE_PIN
#define NRF24_DEMO_IRQ_PIN              PKG_NRF24L01_DEMO_IRQ_PIN

const static char *ROLE_TABLE[] = {"PTX", "PRX"};

static void rx_ind(nrf24_t nrf24, uint8_t *data, uint8_t len, int pipe)
{
    /*! Don't need to care the pipe if the role is ROLE_PTX */
    rt_kprintf("(p%d): ", pipe);
    rt_kprintf((char *)data);
}

static void tx_done(nrf24_t nrf24, int pipe)
{
    static int cnt = 0;
    static char tbuf[32];

    cnt++;

    /*! Here just want to tell the user when the role is ROLE_PTX
    the pipe have no special meaning except indicating (send) FAILED or OK 
        However, it will matter when the role is ROLE_PRX*/
    if (nrf24->cfg.role == ROLE_PTX)
    {
        if (pipe == NRF24_PIPE_NONE)
            rt_kprintf("tx_done failed");
        else
            rt_kprintf("tx_done ok");
    }
    else
    {
        rt_kprintf("tx_done ok");
    }

    rt_kprintf(" (pipe%d)\n", pipe);

    //获取信号量发送命令
    rt_err_t result;
    result=rt_sem_take  (nrf_sem,RT_WAITING_FOREVER);
    if (result != RT_EOK)
            {
                rt_kprintf(" take a nrf dynamic semaphore, failed.\n");
                //rt_sem_delete(dynamic_sem);
                return;
            }
            else
            {
                //rt_kprintf(" take a nrf dynamic semaphore\n");
                //发送控制命令
                for(int i=0;i<4;i++)
                    {
                        send_pool[i]=command_dir_pool[i];
                        send_pool[i+4]=command_move_pool[i];

                    }
                //rt_kprintf(send_pool);
                //rt_kprintf("\n");
                nrf24_send_data(nrf24, (uint8_t *)send_pool, rt_strlen(send_pool), pipe);
            }




#ifdef PKG_NRF24L01_DEMO_ROLE_PTX
    rt_thread_mdelay(NRF24_DEMO_SEND_INTERVAL);
#endif
}

const static struct nrf24_callback _cb = {
    .rx_ind = rx_ind,
    .tx_done = tx_done,
};

static void thread_entry(void *param)
{
    nrf24_t nrf24;
    rt_kprintf("[nrf24/demo] Version:%s\n", PKG_NRF24L01_VERSION);

    nrf24 = nrf24_default_create(NRF24_DEMO_SPI_DEV_NAME, NRF24_DEMO_CE_PIN, NRF24_DEMO_IRQ_PIN, &_cb, NRF24_DEMO_ROLE);

    if (nrf24 == RT_NULL)
    {
        rt_kprintf("\n[nrf24/demo] Failed to create nrf24. stop!\n");
        for(;;) rt_thread_mdelay(10000);
    }
    else
    {
        rt_kprintf("[nrf24/demo] running.");
    }
    //发送第一个数据
    nrf24_send_data(nrf24, (uint8_t *)send_pool, rt_strlen(send_pool), NRF24_DEFAULT_PIPE);

    while (1)
    {
        nrf24_run(nrf24);

        if(!nrf24->flags.using_irq)
            rt_thread_mdelay(10);
    }
    
}

static int nrf24l01_sample_init(void)
{
    rt_thread_t thread;
    rt_hw_spi_device_attach("spi1",NRF24_DEMO_SPI_DEV_NAME,GPIOA, GPIO_PIN_4);
    thread = rt_thread_create("nrfDemo", thread_entry, RT_NULL, 1024, RT_THREAD_PRIORITY_MAX/2, 20);
    rt_thread_startup(thread);

    return RT_EOK;
}

INIT_APP_EXPORT(nrf24l01_sample_init);
