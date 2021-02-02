/**
  ******************************************************************************
  * @file    bmp_encode.c
  * @author  fire
  * @version V1.0
  * @date    2020-xx-xx
  * @brief   LTDC-屏幕截图BMP格式
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火  STM32 F429 开发板  
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
#include <string.h>

#include "./ff.h"
#include "./lcd/bsp_lcd.h"
#include "./bmp/bmp_ScreenShot.h"

/* 定义bmp缓冲区大小，刚好一帧 */
#define BMP_BUF_SIZE		(800 * 480 * 2)

/* 定义bmp缓冲区，指定到了SDRAM中，请结合实际项目进行修改 */
static uint8_t bmpbuf[BMP_BUF_SIZE] __attribute__ ((at(0xD0400000))) = {0};

/* bmp图片路径 */
static uint8_t BMP_PATH[] = "0:/ScreenShot";

static FIL f_bfile;					/* file objects */
static DIR bdir;
static FRESULT res; 
static UINT br;            	/* File R/W count */


/**
  * @brief 读点函数
  * @param layer：LTDC图层
  * @param x：读点的横坐标
  * @param y：读点的纵坐标
  * @retval 颜色值
  */
static uint32_t LTDC_Read_Point(uint8_t layer, uint16_t x, uint16_t y)
{
  if(layer == LCD_FOREGROUND_LAYER)
    return *(uint16_t*)((LCD_FRAME_BUFFER + BUFFER_OFFSET) + 2 * (LCD_PIXEL_WIDTH * y + x));
  else
    return *(uint16_t*)(LCD_FRAME_BUFFER + 2 * (LCD_PIXEL_WIDTH * y + x));
}

/**
  * @brief 通过指定路径，得到完整带路径的文件名，以数字序号命名
  * @param path：bmp存放路径
  * @param pname：完整带路径的文件名，如"0:/ScreenShot/ScreenShot12345.bmp"
  * @retval none
  */
static void new_pathname(uint8_t *path, uint8_t *pname)
{  
	uint16_t index = 0;
  
  /* 尝试打开目录，无目录则创建 */
  res = f_opendir(&bdir, (const TCHAR*)path);
  if(res == FR_NO_PATH)
  {
    f_mkdir((const TCHAR*)path);
  }
    
	while(index < 0xFFFF)
	{
		sprintf((char*)pname, "%s/ScreenShot%05d.bmp", path, index);
		res = f_open(&f_bfile, (const TCHAR*)pname, FA_READ);/* 尝试打开文件 */
    if(res == FR_OK)
      f_close(&f_bfile);
    if(res == FR_NO_FILE)
      break;/* 文件名不存在，可以新建文件 */
		index++;
	}
}

uint16_t  data_reversal(uint16_t data ,uint8_t b) // 指定长度数据大小端反转
{
	char i = 0;
	uint16_t reve_data =0;
	
	for( i = 1; i <= b; i++)
	{
		reve_data |= ((data & 0x01) << (b - i));
		data = data >> 1;
	}
	return reve_data;
}


uint16_t data_negate(uint16_t data ,uint8_t b) // 指定长度数据位反转
{
	uint16_t temp = 1;
	char i = 0;
	
	for(i = 1; i< b; i++)
	{
		temp = ((temp << 1)+1);
	}
	return ((~data) & temp);
}
/**
  * @brief bmp编码函数
  * @note  这个函数用于将LTDC显存数据保存为bmp图像
  * @param filename：保存路径
  * @param layer：LTDC图层
  * @param x0：编码起始坐标
  * @param y0：编码起始坐标
  * @param width：横向宽度
  * @param height：纵向高度
  * @retval  0：成功；非0，失败 
  */
static int8_t bmp_encode(uint8_t *filename, uint8_t layer, uint16_t x0, uint16_t y0, uint16_t width, uint16_t height)
{				
	FIL* f_bmp;
  
  BITMAPFULLDATA hbmp = {0};    /* bmp头 */
	uint16_t bmpheadsize = 0;     /* bmp头长度 */
	uint16_t tx, ty = 0;          /* 图像尺寸 */
	uint32_t pixcnt = 0;          /* 像素计数器 */
  
  uint16_t *pbmpbuf = (uint16_t*)bmpbuf;
  
	if(width==0 || height==0)
    return -1;	/* 区域错误 */
	if((x0 + width - 1) > LCD_PIXEL_WIDTH)
    return -1;	/* 区域错误 */
	if((y0 + height - 1) > LCD_PIXEL_HEIGHT)
    return -1;	/* 区域错误 */
  
	f_bmp = &f_bfile;
  
	bmpheadsize = sizeof(hbmp);/* 得到bmp头的大小 */
	hbmp.bmp_InfoHeader.biSize = sizeof(BITMAPINFOHEADER);/* 信息头长度 */
	hbmp.bmp_InfoHeader.biWidth = width;	 	/* bmp的宽度 */
	hbmp.bmp_InfoHeader.biHeight = height; 	/* bmp的高度 */
	hbmp.bmp_InfoHeader.biPlanes = 1;	 		  /* 恒为1 */
	hbmp.bmp_InfoHeader.biBitCount = 16;	 	/* 16bit色深 */
	hbmp.bmp_InfoHeader.biCompression = 0;  /* 不压缩 */
 	hbmp.bmp_InfoHeader.biSizeImage = hbmp.bmp_InfoHeader.biHeight * hbmp.bmp_InfoHeader.biWidth
                                    * hbmp.bmp_InfoHeader.biBitCount / 8;/* bmp数据区长度 */
 				   
	hbmp.bmp_FileHeader.bfType = ((uint16_t)'M' << 8) + 'B';/* BMP格式标志 */
	hbmp.bmp_FileHeader.bfSize = bmpheadsize + hbmp.bmp_InfoHeader.biSizeImage;/* 整个bmp的长度 */
  hbmp.bmp_FileHeader.bfOffBits = bmpheadsize;/* 到数据区的偏移 */

  /* 新建文件 */
 	res = f_open(f_bmp, (const TCHAR*)filename, FA_WRITE|FA_CREATE_NEW);
	printf("8 bit: %x\r\n",data_negate(0x0f ,8));
	printf("16 bit: %x\r\n",data_negate(0x00ff ,16));
 	if(res == FR_OK)//创建成功
	{
    /* 写入bmp头 */
		res = f_write(f_bmp, (uint8_t*)&hbmp, bmpheadsize, &br);
    /* 按照bmp刷新顺序整理图像数据，从左到右，从下到上，
       数据整理结束前，LTDC显存不得做任何修改 */
		for(ty = y0 + height - 1; hbmp.bmp_InfoHeader.biHeight; ty--)
		{
 			for(tx = x0; tx < hbmp.bmp_InfoHeader.biWidth; tx++)
			{
				//uint16_t R = 0,G = 0,B = 0,temp = 0, l = 0 , h = 0;

				pbmpbuf[pixcnt] = LTDC_Read_Point(layer, tx, ty);
				pixcnt++;
			}
			hbmp.bmp_InfoHeader.biHeight--;
		}
    res = f_write(f_bmp, (uint8_t*)pbmpbuf, hbmp.bmp_InfoHeader.biSizeImage, &br);//写入数据
		f_close(f_bmp);
	}

	return res;
}



/**
  * @brief 截图函数
  * @param layer：需要截取的LTDC图层
  * @retval 0，截图成功，非0，截图失败
  * @note 此函数只支持RGB565非压缩BMP格式截图
  */
uint8_t screen_shot(uint8_t layer)
{
  uint8_t pname[40] = {0};
  uint8_t res = 0;
  
  /* 创建带完整路径的文件名 */
  new_pathname(BMP_PATH, pname);
  
  /* 获取显存数据并编码为bmp格式 */
  res = bmp_encode(pname, layer, 0, 0, 800, 480);

  return res;
}
