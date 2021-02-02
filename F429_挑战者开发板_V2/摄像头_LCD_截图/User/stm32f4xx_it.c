/**
  ******************************************************************************
  * @file    FMC_SDRAM/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    11-November-2013
  * @brief   Main Interrupt Service Routines.
  *         This file provides template for all exceptions handler and
  *         peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "./camera/bsp_ov2640.h"
#include "./systick/bsp_SysTick.h"

extern unsigned int Task_Delay[];

/** @addtogroup STM32F429I_DISCOVERY_Examples
  * @{
  */

/** @addtogroup FMC_SDRAM
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{}

/**
  * @brief  This function handles PendSV_Handler exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{}

	
extern uint32_t Task_Delay[];
/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	uint8_t i;
	
  TimingDelay_Decrement();
	
	for(i=0;i<NumOfTask;i++)
	{
		if(Task_Delay[i])
		{
			Task_Delay[i]--;
		}
	}
}



/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f429_439xx.s).                         */
/******************************************************************************/

extern uint16_t lcd_width, lcd_height;
extern uint16_t img_width, img_height;
extern uint8_t fps;
extern char OV2640_dma_iic_flag;
//记录传输了多少行
static uint16_t line_num =0;

__IO uint8_t dma_flag = 0;
void DMA2_Stream1_IRQHandler(void)
{
  if(DMA_GetITStatus(DMA2_Stream1, DMA_IT_TCIF1) == SET)
  {
    DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_TCIF1);
		/*行计数*/
		line_num++;
		
    if(line_num==img_height)
		{
			/*传输完一帧,计数复位*/
			line_num=0;
		}
    if(dma_flag != 0)
    {
//      dma_flag = 0;
    }
    else
    {
      /*DMA 一行一行传输*/
      OV2640_DMA_Config(FSMC_LCD_ADDRESS+(lcd_width*2*line_num), img_width*2/4);
    }
	}
}


//使用帧中断重置line_num,可防止有时掉数据的时候DMA传送行数出现偏移
void DCMI_IRQHandler(void)
{
	if(DCMI_GetITStatus (DCMI_IT_FRAME) == SET)
	{
		/*传输完一帧，计数复位*/
		line_num=0;
		fps++; //帧率计数
		if(OV2640_dma_iic_flag == 1)
		{
			OV2640_Stop();		//关闭DMA
			OV2640_dma_iic_flag = 0;
		}
		
		DCMI_ClearITPendingBit(DCMI_IT_FRAME); 
	}
}


/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
