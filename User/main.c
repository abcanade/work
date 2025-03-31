#include "sys.h"
#include "adc.h"
#include "delay.h"
#include "lcd1602.h"
#include "usart1.h"
#include "usart3.h"
#include "timer.h"
#include "gpio.h"
#include "stdio.h"
#include "stdbool.h"
#include <string.h>

#define STM32_RX1_BUF       Usart1RecBuf 
#define STM32_Rx1Counter    RxCounter
#define STM32_RX1BUFF_SIZE  USART1_RXBUFF_SIZE

bool mode = 0;               //0���Զ�ģʽ��1���ֶ�ģʽ
unsigned char miao = 0;      //û���˵ļ�ʱʱ��
unsigned char PWM_Val = 0xFF;//����PWM����
unsigned char GearTemp = 0;  //0~3��
u16 light=0;
unsigned char command = 0;
unsigned char STR_BUF[]={0x0e,0x0e,0x04,0x0e,0x15,0x04,0x0a,0x11};   //����ͼ��
bool usart_send_flag = 0;
bool shuaxin = 0;

void AutoModeHandle(void)//�Զ�ģʽ�����������ݹ��ߵ���LED����
{
		u16 test_adc=0;
	
	  /////////////��ȡ����ֵ
	  test_adc = Get_Adc_Average(ADC_Channel_9,10);//��ȡͨ��9��5��ADƽ��ֵ
		light = test_adc*99/4096;//ת����0-99�ٷֱ�
		light = light >= 99? 99: light;//���ֻ�ܵ��ٷ�֮99
	  LCD_Write_Char(6,0,light/10+'0'); 
	  LCD_Write_Char(7,0,light%10+'0'); 
	
	  if(mode==0)//���Զ�ģʽ�£����ݹ��ߵ��ڵ�λ
		{
			  if(miao==0)GearTemp=0;//���ʱ��0��˵��û�����ˣ��ر�LED
			  else//������ݹ��ߵ���
				{
						if(light>=85)GearTemp=0;
						if(light<85)GearTemp=1;
						if(light<55)GearTemp=2;
						if(light<25)GearTemp=3;
				}
		}
}

void BRIGHTNESS(u8 index)//��������
{
		switch(index)
		{
			 case(0):PWM_Val = 0xFF;PWM_LED = 0;break; 
			 case(1):PWM_Val = 0x46;break;  //70
			 case(2):PWM_Val = 0x28;break;  //40
			 case(3):PWM_Val = 0x00;PWM_LED = 1;break;
			 default:PWM_Val = 0x00;PWM_LED = 1;break;
		}
		LCD_Write_Char(15,0,index+'0'); 
}

void keyscan(void)
{
	if(KEY1 == 0)//ģʽ�л�����
	{
	 	delay_ms(10);//����
		if(KEY1 == 0)
		{
		 	while(KEY1 == 0);//�ȴ������ɿ�
			mode = 0;
      LCD_Write_String(0,1,"    < AUTO >    ");
		}
	}
	if(KEY2 == 0)//ģʽ�л�����
	{
	 	delay_ms(10);
		if(KEY2 == 0)
		{
       while(KEY2 == 0);
       mode = 1;
       LCD_Write_String(0,1,"    <Manual>    ");
		}
	}
	if(mode == 1)
	{
			if(KEY3 == 0)//�Ӽ�
			{
				delay_ms(10);
				if(KEY3 == 0 )
				{
					while(KEY3 == 0);
					if(GearTemp < 3)GearTemp++;
				}
			}
			
			if(KEY4 == 0)//����
			{
				delay_ms(10);
				if(KEY4 == 0 )
				{
					while(KEY4 == 0);
					if(GearTemp > 0)GearTemp--;
				}
			}
	}
}

void UsartSendReceiveData(void)
{
	  if(STM32_Rx1Counter > 0)
		{	
			  delay_ms(20);
			
				if(strstr(STM32_RX1_BUF,"auto_mode")!=NULL)
				{
					 	mode = 0;
						LCD_Write_String(0,1,"    < AUTO >    ");
				}	
				
				if(strstr(STM32_RX1_BUF,"manual_mode")!=NULL)
				{
						mode = 1;
						LCD_Write_String(0,1,"    <Manual>    ");
				}	
				if(mode == 1)
				{			
						if(strstr(STM32_RX1_BUF,"manual_off")!=NULL)
						{
								GearTemp = 0;
						}
						if(strstr(STM32_RX1_BUF,"gear_one")!=NULL)
						{
								GearTemp = 1;
						}	
						if(strstr(STM32_RX1_BUF,"gear_two")!=NULL)
						{
								GearTemp = 2;
						}	
            if(strstr(STM32_RX1_BUF,"gear_three")!=NULL)
						{
								GearTemp = 3;
						}							
				}
			  memset(STM32_RX1_BUF, 0, STM32_RX1BUFF_SIZE);//�������
		    STM32_Rx1Counter = 0;							
		}
		if(usart_send_flag==1)    //1�����ϴ�һ������
		{
			  usart_send_flag = 0;		
			   
			  printf("$light:%d#,$gear:%d#",(int)light,(int)GearTemp);
		}
}

int main(void)
{	
		delay_init();	    //��ʱ������ʼ��	  
	  NVIC_Configuration();
		delay_ms(500);       //�ϵ�˲�����һ����ʱ�ڳ�ʼ��
	  Adc_Init();          //ADC��ʼ��
	  KEY_GPIO_Init();    //������ʼ��
	  uart1_Init(9600);
		LCD_Init();         //��Ļ��ʼ��
	  LCD_Write_String(0,0,"light:00%    G:0");//��ʾ�ַ���
	  LCD_Write_String(0,1,"    < AUTO >    ");
	  memset(STM32_RX1_BUF, 0, STM32_RX1BUFF_SIZE);//�������
		STM32_Rx1Counter = 0;
	  USART3_Init(9600);
	  TIM2_Init(99,71);     //��ʱ����ʼ������ʱ100us
		TIM3_Init(499,7199);  //��ʱ����ʼ������ʱ50ms
	  //Tout = ((arr+1)*(psc+1))/Tclk ;  
	  //Tclk:��ʱ������Ƶ��(��λMHZ)
	  //Tout:��ʱ�����ʱ��(��λus)
		while(1)
		{  
			 keyscan();
			 if(command!=0)
			 {
				  delay_ms(10);
					switch(command)
					{
						case(0x01): mode = 0;LCD_Write_String(0,1,"    < AUTO >    ");  break;
						case(0x02): mode = 1;LCD_Write_String(0,1,"    <Manual>    ");  break;		
            case(0x03):	mode = 1;LCD_Write_String(0,1,"    <Manual>    ");if(mode==1)	GearTemp=0;			 break;	
						case(0x04):	mode = 1;LCD_Write_String(0,1,"    <Manual>    ");if(mode==1)	GearTemp=1;			 break;	
						case(0x05):	mode = 1;LCD_Write_String(0,1,"    <Manual>    ");if(mode==1)	GearTemp=2;			 break;	
						case(0x06):	mode = 1;LCD_Write_String(0,1,"    <Manual>    ");if(mode==1)	GearTemp=3;			 break;	
						default: break;
					}
					command=0;
			 }
			 if(shuaxin == 1)//��ʱһ��ʱ��ִ�����º���
			 {
					  shuaxin = 0;
            AutoModeHandle();					
			 }	
			 UsartSendReceiveData();
			 BRIGHTNESS(GearTemp);
			 if(miao>0)Lcd_Custom_Char(0,10,4,STR_BUF); //��������ʾ����ͼ�� 
			 else      LCD_Write_Char(10,0,' ');        //������ʾ�հ׷�
			 delay_ms(1);
		}	
}

void USART3_IRQHandler(void)              //����1�жϷ������,���ڽ�������ʶ��ģ�鷢�͹���������   
{
     if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  
     {   
        command = USART_ReceiveData(USART3);//����ģ�������
     }

     if(USART_GetFlagStatus(USART3,USART_FLAG_ORE) == SET)
     {
         USART_ClearFlag(USART3,USART_FLAG_ORE);
     }
     USART_ClearITPendingBit(USART3, USART_IT_RXNE);
}

void TIM2_IRQHandler(void)//��ʱ��2�жϷ������	 
{ 
	  static u8 timeCount = 0;
	
		if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM �ж�Դ 
		{ 
				TIM_ClearITPendingBit(TIM2, TIM_IT_Update); //����жϱ�־λ  
			  timeCount++;
			  if(timeCount > 100)timeCount = 0;          //10msһ������ ��Ƶ��Ϊ100Hz 
				if(timeCount >= PWM_Val)PWM_LED = 1;      //ռ�ձȵ���  
				else PWM_LED = 0;  
	  }
}

void TIM3_IRQHandler(void)   //TIM3�ж�,50����һ���ж�
{
		static u8 timeCount1 = 0;
	  static u8 timeCount2 = 0;
	  static u8 timeCount3 = 0;
	
		if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM �ж�Դ 
		{
				TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //���TIMx���жϴ�����λ:TIM �ж�Դ 

        if(timeCount1++ >= 20)//1��ʱ�䵽
				{
						timeCount1 = 0;
						if(SR501==1)miao=10;//��⵽���ˣ�ʱ�丳ֵ10��
					  else
						{
								if(miao>0)miao--;//û���ˣ�10�뵹��ʱ
						}
				}
				
				timeCount2++;
				if(timeCount2 >= 10)
				{
						timeCount2 = 0;
					  
					  shuaxin = 1;
				}
				
				timeCount3++;
				if(timeCount3 >= 13)
				{
						timeCount3 = 0;
					  
					  usart_send_flag = 1;
				}
		}
}

