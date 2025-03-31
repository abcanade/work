 #include "adc.h"
 #include "delay.h"
		   
//��ʼ��ADC
//�������ǽ��Թ���ͨ��Ϊ��
//����Ĭ�Ͻ�����ͨ��0~3																	   
void  Adc_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB |RCC_APB2Periph_ADC1	, ENABLE );	  //ʹ��ADC1ͨ��ʱ��
 

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //����ADC��Ƶ����6 72M/6=12,ADC���ʱ�䲻�ܳ���14M

	//PB1 ��Ϊģ��ͨ����������                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//ģ����������
	GPIO_Init(GPIOB, &GPIO_InitStructure);	

	ADC_DeInit(ADC1);  //��λADC1,������ ADC1 ��ȫ���Ĵ�������Ϊȱʡֵ

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//ģ��ת�������ڵ�ͨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//ģ��ת�������ڵ���ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure);	//����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���   

  
	ADC_Cmd(ADC1, ENABLE);	//ʹ��ָ����ADC1
	
	ADC_ResetCalibration(ADC1);	//ʹ�ܸ�λУ׼  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//�ȴ���λУ׼����
	
	ADC_StartCalibration(ADC1);	 //����ADУ׼
 
	while(ADC_GetCalibrationStatus(ADC1));	 //�ȴ�У׼����
 
//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//ʹ��ָ����ADC1�����ת����������

}				  
//���ADCֵ
//ch:ͨ��ֵ 0~3
u16 Get_Adc(u8 ch)   
{
  	//����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADCͨ��,����ʱ��Ϊ239.5����	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//ʹ��ָ����ADC1�����ת����������	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//�ȴ�ת������

	return ADC_GetConversionValue(ADC1);	//�������һ��ADC1�������ת�����
}

u16 Get_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=Get_Adc(ch);
		delay_ms(5);
	}
	return temp_val/times;
} 	 

/***************************************************************************
     ��λֵƽ���˲������ֳƷ��������ƽ���˲�����
������ ��һ�����ȥ�����ֵ����Сֵ��ȡƽ��ֵ�� 
      �൱�ڡ���λֵ�˲�����+������ƽ���˲�������
      ��������N�����ݣ�ȥ��һ�����ֵ��һ����Сֵ��
      Ȼ�����N-2�����ݵ�����ƽ��ֵ��    
�ŵ㣺 �ں��ˡ���λֵ�˲�����+������ƽ���˲����������˲������ŵ㡣
       ����żȻ���ֵ������Ը��ţ�����������������Ĳ���ֵƫ�
       �����ڸ��������õ��������á�
       ƽ���ȸߣ����ڸ�Ƶ�񵴵�ϵͳ��
****************************************************************************/

#define N 500 //�������������Ĵ���

u16 Get_Adc_filter(u8 ch)
{
    u16  filter_buf[N]; //����N�β���ֵ�Ĵ洢����
    u32  filter_temp,filter_sum;           
	  u16  i,j; 
	
    for(i = 0; i < N; i++)  //��������N��ADֵ
    {
      filter_buf[i] = Get_Adc(ch);
      delay_us(50);
    }
     // ����ֵ��С�������У�ð�ݷ���
		for(j = 0; j < N - 1; j++) 
		{
			for(i = 0; i < N - 1 - j; i++) 
			{
				if(filter_buf[i] > filter_buf[i + 1]) 
				{
					filter_temp = filter_buf[i];
					filter_buf[i] = filter_buf[i + 1];
					filter_buf[i + 1] = filter_temp;
				}
			}
		}
		 // ȥ�������С��ֵ����ƽ��
		 for(i = 1; i < N - 1; i++)
		 {
				filter_sum += filter_buf[i];
			  delay_us(50);
		 }
     return (u16)(filter_sum/(N-2));
}

























