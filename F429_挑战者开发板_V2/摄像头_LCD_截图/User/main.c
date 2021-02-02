/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   OV2640摄像头显示例程
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 STM32  F429开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "stm32f4xx.h"
#include "./usart/bsp_debug_usart.h"
#include "./sdram/bsp_sdram.h"
#include "./lcd/bsp_lcd.h"
#include "./camera/bsp_ov2640.h"
#include "./systick/bsp_SysTick.h"
#include "./key/bsp_key.h"
#include "./bmp/bmp_ScreenShot.h"
#include "./font/fonts.h"
#include "./ff.h"
/*简单任务管理*/
uint32_t Task_Delay[NumOfTask];

uint8_t dispBuf[100];
OV2640_IDTypeDef OV2640_Camera_ID;

uint8_t fps=0;

//显示帧率数据，默认不显示，需要显示时把这个宏设置为1即可，（会有轻微花屏现象！）
//经测试液晶显示摄像头数据800*480像素，帧率为14.2帧/秒。
//#define FRAME_RATE_DISPLAY 	1

// WIFI和SD卡公用SDIO总线，当使用SD卡时，避免WIFI的冲突
// 需要把WIFI的使能引脚PDN拉低。
void WIFI_PDNPIN_SET(void)
{
  /*定义一个GPIO_InitTypeDef类型的结构体*/
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOB, ENABLE); 							   
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
  GPIO_Init(GPIOB, &GPIO_InitStructure);	
  
  GPIO_ResetBits(GPIOB,GPIO_Pin_13);  //禁用WiFi模块
} 
extern __IO uint8_t dma_flag;
char OV2640_dma_iic_flag=0;
/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{
  static FATFS fs;
	static FRESULT res_sd; 
	/*摄像头与RGB LED灯共用引脚，不要同时使用LED和摄像头*/
	
  Debug_USART_Config();
	
		// 失能WIFI
	WIFI_PDNPIN_SET();
	
	/* 配置SysTick 为10us中断一次,时间到后触发定时中断，
	*进入stm32fxx_it.c文件的SysTick_Handler处理，通过数中断次数计时
	*/
	SysTick_Init();
	 
	/*初始化液晶屏*/
  LCD_Init();
  LCD_LayerInit();
  LTDC_Cmd(ENABLE);
	
	/*把背景层刷黑色*/
  LCD_SetLayer(LCD_BACKGROUND_LAYER);
	LCD_SetTransparency(0xFF);
	
	LCD_Clear(LCD_COLOR_BLACK);
	
  /*初始化后默认使用前景层*/
	LCD_SetLayer(LCD_FOREGROUND_LAYER);
	/*默认设置不透明	，该函数参数为不透明度，范围 0-0xff ，0为全透明，0xff为不透明*/
  LCD_SetTransparency(0xFF);
	LCD_Clear(TRANSPARENCY);
	
	LCD_SetColors(LCD_COLOR_RED,TRANSPARENCY);

	LCD_ClearLine(LINE(18));
  LCD_DisplayStringLine_EN_CH(LINE(18),(uint8_t* )" 模式:UXGA 800x480");

  CAMERA_DEBUG("STM32F429 DCMI 驱动OV2640例程");

  /* 初始化摄像头GPIO及IIC */
  OV2640_HW_Init();

  /* 读取摄像头芯片ID，确定摄像头正常连接 */
  OV2640_ReadID(&OV2640_Camera_ID);

   if(OV2640_Camera_ID.PIDH  == 0x26)
  {
//    sprintf((char*)dispBuf, "              OV2640 摄像头,ID:0x%x", OV2640_Camera_ID.PIDH);
//		LCD_DisplayStringLine_EN_CH(LINE(0),(uint8_t*)dispBuf);
    CAMERA_DEBUG("%x %x",OV2640_Camera_ID.Manufacturer_ID1 ,OV2640_Camera_ID.Manufacturer_ID2);

  }
  else
  {
    LCD_SetTextColor(LCD_COLOR_RED);
    LCD_DisplayStringLine_EN_CH(LINE(0),(uint8_t*) "         没有检测到OV2640，请重新检查连接。");
    CAMERA_DEBUG("没有检测到OV2640摄像头，请重新检查连接。");

    while(1);
  }

	while(1)
 {
		res_sd = f_mount(&fs,"0:",1);
		if(res_sd == FR_OK)
		{
			printf("文件系统挂载成功！");
			break;
		}
		else
			printf("文件系统挂载失败！");
		Delay(10);
 }
	 
 LCD_SetLayer(LCD_BACKGROUND_LAYER);
 
  OV2640_Init();
  OV2640_UXGAConfig();
  
	//使能DCMI采集数据
  DCMI_Cmd(ENABLE);
  DCMI_CaptureCmd(ENABLE);



	/*DMA直接传输摄像头数据到LCD屏幕显示*/
  while(1)
	{

//显示帧率，默认不显示
#if FRAME_RATE_DISPLAY
		if(Task_Delay[0]==0)
		{
						
			LCD_SetColors(LCD_COLOR_RED,TRANSPARENCY);

			LCD_ClearLine(LINE(17));
			sprintf((char*)dispBuf, " 帧率:%.1f/s", (float)fps/5.0);
			
			/*输出帧率*/
			LCD_DisplayStringLine_EN_CH(LINE(17),dispBuf);
			//重置
			fps =0;
			
			
			Task_Delay[0]=5000; //此值每1ms会减1，减到0才可以重新进来这里


		}
			
#endif
		if(Key_Scan(KEY1_GPIO_PORT, KEY1_PIN) == KEY_ON)
    {
      uint8_t res = 0;
      
      printf("开始截图\r\n");
			
			OV2640_dma_iic_flag = 1;	//开始截图，需要再中断中置零，关闭DMA
			
			while(OV2640_dma_iic_flag){Delay(1);};
			
      res = screen_shot(0);  //LCD_FOREGROUND_LAYER
      
      if(res == 0)
      {
        printf("截图成功\r\n");
      }
      else
      {
        printf("截图失败\r\n");
      }
			OV2640_dma_iic_flag = 0;
      dma_flag = 0;
			OV2640_start(); //开启DMA
    }
		if(Key_Scan(KEY2_GPIO_PORT, KEY2_PIN) == KEY_ON)
    {
      OV2640_Stop();
      dma_flag = 1;
    }
	}
}



/*********************************************END OF FILE**********************/

