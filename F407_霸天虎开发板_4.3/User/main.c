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
  * 实验平台:野火 STM32  F407开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "stm32f4xx.h"
#include "./usart/bsp_debug_usart.h"
#include "./lcd/bsp_nt35510_lcd.h"
#include "./camera/bsp_ov2640.h"
#include "./systick/bsp_SysTick.h"



/*简单任务管理*/
uint32_t Task_Delay[NumOfTask];

char dispBuf[100];
OV2640_IDTypeDef OV2640_Camera_ID;

uint8_t fps=0;


extern uint16_t img_width, img_height;

/**
  * @brief  液晶屏开窗，使能摄像头数据采集
  * @param  无
  * @retval 无
  */
void ImagDisp(void)
{
		//扫描模式，横屏
    NT35510_GramScan(5);
    LCD_SetFont(&Font16x32);
		LCD_SetColors(RED,BLACK);
	
    NT35510_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	/* 清屏，显示全黑 */
    NT35510_DispStringLine_EN(LINE(0),"BH 4.8 inch LCD + OV2640");
	
		/*DMA会把数据传输到液晶屏，开窗后数据按窗口排列 */
    NT35510_OpenWindow(0,0,img_width,img_height);	
		
		OV2640_Capture_Control(ENABLE);
}

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{	
	/*图像大小，修改这两个值即可改变图像的大小*/
	//要求为4的倍数
	img_width=800;
	img_height =480;

	
	NT35510_Init ();         //LCD 初始化
	
	LCD_SetFont(&Font16x32);
	LCD_SetColors(RED,BLACK);

  NT35510_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	/* 清屏，显示全黑 */

  Debug_USART_Config();   
	
	/* 配置SysTick 为1ms中断一次,时间到后触发定时中断，
	*进入stm32fxx_it.c文件的SysTick_Handler处理，通过数中断次数计时
	*/
	SysTick_Init();

	//液晶扫描方向
	NT35510_GramScan(5);
	
  CAMERA_DEBUG("STM32F407 DCMI 驱动OV2640例程");

  /* 初始化摄像头GPIO及IIC */
  OV2640_HW_Init();   

  /* 读取摄像头芯片ID，确定摄像头正常连接 */
  OV2640_ReadID(&OV2640_Camera_ID);

  if(OV2640_Camera_ID.PIDH  == 0x26)
  {
    sprintf(dispBuf, "OV2640 camera,ID:0x%x", OV2640_Camera_ID.PIDH);
		NT35510_DispStringLine_EN(LINE(0),dispBuf);
    CAMERA_DEBUG("检测到摄像头 %x %x",OV2640_Camera_ID.Manufacturer_ID1 ,OV2640_Camera_ID.Manufacturer_ID2);

  }
  else
  {
    LCD_SetTextColor(RED);
    NT35510_DispString_EN(10,10,"Can not detect OV2640 module,please check the connection!");
    CAMERA_DEBUG("没有检测到OV2640摄像头，请重新检查连接。");

    while(1);  
  }

  /*使用寄存器表初始化摄像头*/
  OV2640_Init();
  OV2640_UXGAConfig(); 
  
	/*DMA直接传输摄像头数据到LCD屏幕显示*/
	ImagDisp();

  while(1)
	{
		//使用串口输出帧率
		if(Task_Delay[0]==0)
		{						
			/*输出帧率*/
			CAMERA_DEBUG("\r\n帧率:%.1f/s \r\n", (double)fps/5.0);
			//重置
			fps =0;			
			
			Task_Delay[0]=5000; //此值每1ms会减1，减到0才可以重新进来这里

		}		
	}
}



/*********************************************END OF FILE**********************/

