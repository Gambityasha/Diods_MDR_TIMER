#include "port.h"
#include "MDR32Fx.h"
#include "MDR32F9Qx_config.h"
//����� ��� ������������!!! HSI - ���������� ���������, 8 ���. LSI - ���������� ���������, 40���. HSE - ������� ��������� 2...16 ���. LSE ������� ��������� - 32 ���
//PLL - ���������� ���� ��������� ��������� �������� �������
uint32_t timerCount=0;

void CLK_Init(){
	//������������ �� �������� ����������	
	MDR_RST_CLK->HS_CONTROL |= RST_CLK_HS_CONTROL_HSE_ON;// 0 ���: HSE ON, 0 - ��������, 1 - �������. 1 ���: HSE BYP 0 - ����� �����������, 1 - ����� �������� ����������
	while ((MDR_RST_CLK->CLOCK_STATUS & RST_CLK_CLOCK_STATUS_HSE_RDY)==0);//2 ���: HSE RDY. 0 -���������� �� ������� � �� ��������. 1 - ���������� ������� � ��������, 1 ���: PLL CPU READY.
	
	MDR_RST_CLK->CPU_CLOCK |= (2<<0);//0 � 1 ����: CPU C1 SEL - ���� ������ ��������� ��� CPU_C1. 00 - HSI, 01 - HSI/2, 10 - HSE, 11 - HSE/2, 
	MDR_RST_CLK->PLL_CONTROL |=((1<<2)|(2<<8));//2 ���: PLL CPU ON - ��� ���������/ 3 ��� PLL CPU RLD - ����� ������������ ��������� � ������� ������ -1. 8-11 ���� - ����������� ��������� -1, �.�. ���������� ���� ����� �� 1 ������ �������
	while ((MDR_RST_CLK->CLOCK_STATUS & RST_CLK_CLOCK_STATUS_PLL_CPU_RDY)==0);//0 - PLL �� �������� ��� �� ���������, 1 - PLL �������� � ���������
	
	MDR_RST_CLK->CPU_CLOCK |= ((2<<0)|(1<<2)|(1<<8));  // 2 ��� - ��� ������ ��������� ��� CPU_C2. 0 - CPU_C1, 1 - PLLCPU0. 1 ������������ ����� ���� ��������� �������	8, 9 ���� - ����� ��������� ��� HCLK
	//00-HSI, 01-CPU_C3, 10-LSE,11-LSI 
}

void TIMER1_Init(){
	//--------������ ������ � �������� ������ ����������
	
	MDR_RST_CLK->PER_CLOCK |= (1<<14); //������������ ������� ������� (��� 14), ������������ ������� (��� 15), ������������ �������� (��� 16), ��� ������ ������	
	MDR_RST_CLK->TIM_CLOCK |= (1<<24); //TIM CLK EN - ���������� �������� ������� 24 ��� - TIM1, 25 ��� - TIM2, 26 ��� - TIM3
		
	MDR_TIMER1->CNTRL = 0x00000000;  //����� ������������� �������
	//����������� ������ ��������� �������� 
	MDR_TIMER1->CNT = 0x00000000;  //��������� �������� �������� 
	MDR_TIMER1->PSG = 0x00000000;  //������������ ������� 
	MDR_TIMER1->ARR = 23999;  //��������� �����
	MDR_TIMER1->IE = 0x00000002;     //���������� ������������ ���������� ��� CNT=ARR � ��� CNT=0
	//���������� ������ �������. 
	 	
	NVIC_EnableIRQ(Timer1_IRQn);																							// ���������� ���������� �� UART1
}


int main(void)
{	
	CLK_Init();
	ConfigPortC();
	
	TIMER1_Init();
	
	MDR_PORTC->RXTX |= ((1<<0)|(1<<1)); //��������� ����������� �� 0 � 1 ����� �
	__enable_irq(); 										// Enable Interrupts
  MDR_TIMER1->CNTRL = 0x00000001;  //���� ����� �� TIM_CLK.
	while (1)
	{		
					
	}
}


void Timer1_IRQHandler(){
	MDR_TIMER1->STATUS &= 0xFFFD;
	MDR_TIMER1->CNTRL &= 0xFFFE;
	timerCount++;
	if (timerCount==1000){
		  MDR_PORTC->RXTX ^= ((1<<0)|(1<<1));
		  timerCount=0;
	}
	MDR_TIMER1->CNTRL = 0x00000001;  //���� ����� �� TIM_CLK.
}

