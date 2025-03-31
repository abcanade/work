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

bool mode = 0;               //0是自动模式，1是手动模式
unsigned char miao = 0;      //没有人的计时时间
unsigned char PWM_Val = 0xFF;//用于PWM调节
unsigned char GearTemp = 0;  //0~3档
u16 light=0;
unsigned char command = 0;
unsigned char STR_BUF[]={0x0e,0x0e,0x04,0x0e,0x15,0x04,0x0a,0x11};   //特殊图形
bool usart_send_flag = 0;
bool shuaxin = 0;

void AutoModeHandle(void)//自动模式处理函数，根据光线调节LED亮度
{
		u16 test_adc=0;
	
	  /////////////获取光线值
	  test_adc = Get_Adc_Average(ADC_Channel_9,10);//读取通道9的5次AD平均值
		light = test_adc*99/4096;//转换成0-99百分比
		light = light >= 99? 99: light;//最大只能到百分之99
	  LCD_Write_Char(6,0,light/10+'0'); 
	  LCD_Write_Char(7,0,light%10+'0'); 
	
	  if(mode==0)//在自动模式下，根据光线调节档位
		{
			  if(miao==0)GearTemp=0;//秒计时到0，说明没有人了，关闭LED
			  else//否则根据光线调节
				{
						if(light>=85)GearTemp=0;
						if(light<85)GearTemp=1;
						if(light<55)GearTemp=2;
						if(light<25)GearTemp=3;
				}
		}
}

void BRIGHTNESS(u8 index)//调节亮度
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
	if(KEY1 == 0)//模式切换按键
	{
	 	delay_ms(10);//消抖
		if(KEY1 == 0)
		{
		 	while(KEY1 == 0);//等待按键松开
			mode = 0;
      LCD_Write_String(0,1,"    < AUTO >    ");
		}
	}
	if(KEY2 == 0)//模式切换按键
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
			if(KEY3 == 0)//加键
			{
				delay_ms(10);
				if(KEY3 == 0 )
				{
					while(KEY3 == 0);
					if(GearTemp < 3)GearTemp++;
				}
			}
			
			if(KEY4 == 0)//减键
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
			  memset(STM32_RX1_BUF, 0, STM32_RX1BUFF_SIZE);//清除缓存
		    STM32_Rx1Counter = 0;							
		}
		if(usart_send_flag==1)    //1秒钟上传一次数据
		{
			  usart_send_flag = 0;		
			   
			  printf("$light:%d#,$gear:%d#",(int)light,(int)GearTemp);
		}
}

int main(void)
{	
		delay_init();	    //延时函数初始化	  
	  NVIC_Configuration();
		delay_ms(500);       //上电瞬间加入一定延时在初始化
	  Adc_Init();          //ADC初始化
	  KEY_GPIO_Init();    //按键初始化
	  uart1_Init(9600);
		LCD_Init();         //屏幕初始化
	  LCD_Write_String(0,0,"light:00%    G:0");//显示字符串
	  LCD_Write_String(0,1,"    < AUTO >    ");
	  memset(STM32_RX1_BUF, 0, STM32_RX1BUFF_SIZE);//清除缓存
		STM32_Rx1Counter = 0;
	  USART3_Init(9600);
	  TIM2_Init(99,71);     //定时器初始化，定时100us
		TIM3_Init(499,7199);  //定时器初始化，定时50ms
	  //Tout = ((arr+1)*(psc+1))/Tclk ;  
	  //Tclk:定时器输入频率(单位MHZ)
	  //Tout:定时器溢出时间(单位us)
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
			 if(shuaxin == 1)//延时一段时间执行以下函数
			 {
					  shuaxin = 0;
            AutoModeHandle();					
			 }	
			 UsartSendReceiveData();
			 BRIGHTNESS(GearTemp);
			 if(miao>0)Lcd_Custom_Char(0,10,4,STR_BUF); //有人则显示特殊图形 
			 else      LCD_Write_Char(10,0,' ');        //无人显示空白符
			 delay_ms(1);
		}	
}

void USART3_IRQHandler(void)              //串口1中断服务程序,用于接收语音识别模块发送过来的数据   
{
     if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  
     {   
        command = USART_ReceiveData(USART3);//接收模块的数据
     }

     if(USART_GetFlagStatus(USART3,USART_FLAG_ORE) == SET)
     {
         USART_ClearFlag(USART3,USART_FLAG_ORE);
     }
     USART_ClearITPendingBit(USART3, USART_IT_RXNE);
}

void TIM2_IRQHandler(void)//定时器2中断服务程序	 
{ 
	  static u8 timeCount = 0;
	
		if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
		{ 
				TIM_ClearITPendingBit(TIM2, TIM_IT_Update); //清除中断标志位  
			  timeCount++;
			  if(timeCount > 100)timeCount = 0;          //10ms一个周期 ，频率为100Hz 
				if(timeCount >= PWM_Val)PWM_LED = 1;      //占空比调节  
				else PWM_LED = 0;  
	  }
}

void TIM3_IRQHandler(void)   //TIM3中断,50毫秒一次中断
{
		static u8 timeCount1 = 0;
	  static u8 timeCount2 = 0;
	  static u8 timeCount3 = 0;
	
		if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
		{
				TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //清除TIMx的中断待处理位:TIM 中断源 

        if(timeCount1++ >= 20)//1秒时间到
				{
						timeCount1 = 0;
						if(SR501==1)miao=10;//检测到有人，时间赋值10秒
					  else
						{
								if(miao>0)miao--;//没有人，10秒倒计时
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

