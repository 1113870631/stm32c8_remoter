/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-14     RealThread   first version
 */

#include <rtthread.h>
#include <board.h>
#include <drv_common.h>

RT_WEAK void rt_hw_board_init()
{
    extern void hw_board_init(char *clock_src, int32_t clock_src_freq, int32_t clock_target_freq);

    /* Heap initialization */
#if defined(RT_USING_HEAP)
    rt_system_heap_init((void *) HEAP_BEGIN, (void *) HEAP_END);
#endif

    hw_board_init(BSP_CLOCK_SOURCE, BSP_CLOCK_SOURCE_FREQ_MHZ, BSP_CLOCK_SYSTEM_FREQ_MHZ);

    /* Set the shell console output device */
#if defined(RT_USING_DEVICE) && defined(RT_USING_CONSOLE)
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif

    /* Board underlying hardware initialization */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif


    GPIO_InitTypeDef GPIO_InitStruct = {0};

      __HAL_RCC_SPI1_CLK_ENABLE();

      __HAL_RCC_GPIOA_CLK_ENABLE();
      /**SPI1 GPIO Configuration
      PA5     ------> SPI1_SCK
      PA6     ------> SPI1_MISO
      PA7     ------> SPI1_MOSI
      */
      GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_7;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

      GPIO_InitStruct.Pin = GPIO_PIN_6;
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);



      __HAL_RCC_ADC1_CLK_ENABLE();

      __HAL_RCC_GPIOA_CLK_ENABLE();
      /**ADC1 GPIO Configuration
      PA0-WKUP     ------> ADC1_IN0
      PA1     ------> ADC1_IN1
      */
      GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
      GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

      __HAL_RCC_GPIOC_CLK_ENABLE();


}



void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hadc->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* Peripheral clock enable */
       __HAL_RCC_ADC1_CLK_ENABLE();

       __HAL_RCC_GPIOA_CLK_ENABLE();
       /**ADC1 GPIO Configuration
       PA0-WKUP     ------> ADC1_IN0
       PA1     ------> ADC1_IN1
       */
       GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
       GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
       HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }



}



