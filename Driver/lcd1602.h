#ifndef __LCD1602_H
#define __LCD1602_H
#include "delay.h"
#include "sys.h"

#define LCD1602_RS PCout(13)	//��������
#define LCD1602_RW PCout(14)	//��д
#define LCD1602_EN PCout(15)	//ʹ��
 
 void LCD_Init(void); 
 void LCD_Clear(void); 
 void LCD_Write_Com(unsigned char com); 
 void LCD_Write_Data(unsigned char Data); 
 void LCD_Write_String(unsigned char x,unsigned char y,unsigned char *s);
 void LCD_Write_Char(unsigned char x,unsigned char y,unsigned char Data); 
 void Lcd_Custom_Char(unsigned char row, unsigned char col, unsigned char pos, unsigned char *customValueTable);
 
#endif



