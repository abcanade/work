#include "lcd1602.h"
#include "delay.h"
#include "sys.h"

u8 Dao_xu(u8 data)
{
    u8 i = 0 ,temp = 0;;
	  
	  for(i = 0; i < 8; i++)
	 {
	   temp += (((data >> i) & 0x01) << (7 - i));
	 }
	 return temp;
}

/*------------------------------------------------
              д�������
------------------------------------------------*/
void LCD_Write_Com(unsigned char com) 
{                              
  LCD1602_RS=0;	 //����
	delay_us(1);
  LCD1602_RW=0;
  delay_us(1);
  LCD1602_EN=1;                  
	delay_us(1);
  GPIO_Write(GPIOA,(GPIO_ReadOutputData(GPIOA)&0XFF00)+Dao_xu(com)); //�ڲ�Ӱ��A8-A15���ŵ�ǰ���£�������д��A0-A7����     
	delay_us(100);
  LCD1602_EN=0;
}

/*------------------------------------------------
              д�����ݺ���
------------------------------------------------*/
 void LCD_Write_Data(unsigned char Data) 
{                                  
  LCD1602_RS=1;   //����              
	delay_us(1);
  LCD1602_RW=0;
  delay_us(1);	
  LCD1602_EN=1;                  
	delay_us(1);
  GPIO_Write(GPIOA,(GPIO_ReadOutputData(GPIOA)&0XFF00)+Dao_xu(Data)); //�ڲ�Ӱ��A8-A15���ŵ�ǰ���£�������д��A0-A7����   
	delay_us(100);
  LCD1602_EN=0;
}
/*------------------------------------------------
              д���ַ�������
------------------------------------------------*/
 void LCD_Write_String(unsigned char x,unsigned char y,unsigned char *s) 
 {     
 if (y == 0) 
 	{     
	 LCD_Write_Com(0x80 + x);  //��һ��   
 	}
 else 
 	{     
 	LCD_Write_Com(0xC0 + x);  //�ڶ���   
 	}        
	while (*s) //�ж��Ƿ��⵽��β��
 	{     
		 LCD_Write_Data( *s);//��ʾ�ַ�     
		 s ++;  //ָ���1   
 	}
 }
/*------------------------------------------------
              д���ַ�����
------------------------------------------------*/
 void LCD_Write_Char(unsigned char x,unsigned char y,unsigned char Data) 
{     
 if (y == 0) 
 	{     
 	LCD_Write_Com(0x80 + x);    //��һ��    
 	}    
 else 
 	{     
 	LCD_Write_Com(0xC0 + x);  //�ڶ���     
 	}        
 LCD_Write_Data( Data); //��ʾ�ַ�      
}
 
/******************************************************************************/
// �Զ����ַ�����(CGRAM)
/******************************************************************************/
void Lcd_Custom_Char(unsigned char row, unsigned char col, unsigned char pos, unsigned char *customValueTable)
{
	unsigned char i = 0;
	for(i=0;i<8;i++){
		LCD_Write_Com(0x40+pos*8+i);//��CGRAM��һ���Զ����ַ����������ڶ�������0X48���������
		LCD_Write_Data(*(customValueTable+i));//д����ʾ����
	}
	if(row==0){
		LCD_Write_Com(0x00+0x80+col);//�ڵ�һ����ʾ
		LCD_Write_Data(0x00+pos);//��ʾ�Զ����ַ���0x40��Ӧ��һ����0x00���Դ����ƣ�
	}else{
		LCD_Write_Com(0x40+0x80+col);//�ڵڶ�����ʾ
		LCD_Write_Data(0x00+pos);	
	}
}

 /*------------------------------------------------
                ��������
------------------------------------------------*/
 void LCD_Clear(void) 
{ 
 LCD_Write_Com(0x01); 
 delay_ms(5);
}

/*------------------------------------------------*/
 void LCD_Init(void) 
{
	  GPIO_InitTypeDef GPIO_InitStructure;
	  
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE);  //����GPIOA GPIOC ʱ��
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 |
    GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //����ٶ�50MHZ
    GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��GPIOA
	
	
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 |GPIO_Pin_14 |GPIO_Pin_15;  // LCD1602 RS-RW-EN 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //����ٶ�50MHZ
    GPIO_Init(GPIOC, &GPIO_InitStructure);   //GPIOC
	
   LCD_Write_Com(0x38);
   delay_ms(5);  
	 LCD_Write_Com(0x38);
   delay_ms(5);  
	 LCD_Write_Com(0x38);
   delay_ms(5);  
   LCD_Write_Com(0x08);    /*��ʾ�ر�*/
   delay_ms(5); 	
   LCD_Write_Com(0x01);    /*��ʾ����*/
   delay_ms(5); 	
   LCD_Write_Com(0x06);    /*��ʾ����ƶ�����*/ 
	 delay_ms(5); 
   LCD_Write_Com(0x0C);    /*��ʾ�����������*/
	 delay_ms(5); 
}

