#include "port.h"
#include "MDR32Fx.h"
#include "MDR32F9Qx_config.h"
//Важно для тактирования!!! HSI - встроенный генератор, 8 МГЦ. LSI - встроенный генератор, 40кГц. HSE - внешний генератор 2...16 МГц. LSE внешний генератор - 32 кГц
//PLL - встроенный блок умножения системной тактовой частоты
uint32_t timerCount=0;

void CLK_Init(){
	//Тактирование от внешнего резонатора	
	MDR_RST_CLK->HS_CONTROL |= RST_CLK_HS_CONTROL_HSE_ON;// 0 бит: HSE ON, 0 - выключен, 1 - включен. 1 бит: HSE BYP 0 - режим осциллятора, 1 - режим внешнего генератора
	while ((MDR_RST_CLK->CLOCK_STATUS & RST_CLK_CLOCK_STATUS_HSE_RDY)==0);//2 бит: HSE RDY. 0 -осциллятор не запущен и не стабилен. 1 - осциллятор запущен и стабилен, 1 бит: PLL CPU READY.
	
	MDR_RST_CLK->CPU_CLOCK |= (2<<0);//0 и 1 биты: CPU C1 SEL - биты выбора источника для CPU_C1. 00 - HSI, 01 - HSI/2, 10 - HSE, 11 - HSE/2, 
	MDR_RST_CLK->PLL_CONTROL |=((1<<2)|(2<<8));//2 бит: PLL CPU ON - бит включения/ 3 бит PLL CPU RLD - смена коэффициента умножения в рабочем режиме -1. 8-11 биты - коэффициент умножения -1, т.е. передавать надо число на 1 меньше нужного
	while ((MDR_RST_CLK->CLOCK_STATUS & RST_CLK_CLOCK_STATUS_PLL_CPU_RDY)==0);//0 - PLL не запущена или не стабильна, 1 - PLL запущена и стабильна
	
	MDR_RST_CLK->CPU_CLOCK |= ((2<<0)|(1<<2)|(1<<8));  // 2 бит - бит выбора источника для CPU_C2. 0 - CPU_C1, 1 - PLLCPU0. 1 используется когда есть умножение частоты	8, 9 биты - выбор источника для HCLK
	//00-HSI, 01-CPU_C3, 10-LSE,11-LSI 
}

void TIMER1_Init(){
	//--------Начало работы с таймером общего назначения
	
	MDR_RST_CLK->PER_CLOCK |= (1<<14); //тактирование первого таймера (бит 14), тактирование второго (бит 15), тактирование третьего (бит 16), тут только первый	
	MDR_RST_CLK->TIM_CLOCK |= (1<<24); //TIM CLK EN - разрешение тактовой частоты 24 бит - TIM1, 25 бит - TIM2, 26 бит - TIM3
		
	MDR_TIMER1->CNTRL = 0x00000000;  //Режим инициализации таймера
	//Настраиваем работу основного счетчика 
	MDR_TIMER1->CNT = 0x00000000;  //Начальное значение счетчика 
	MDR_TIMER1->PSG = 0x00000000;  //Предделитель частоты 
	MDR_TIMER1->ARR = 23999;  //Основание счета
	MDR_TIMER1->IE = 0x00000002;     //Разрешение генерировать прерывание при CNT=ARR и при CNT=0
	//Разрешение работы таймера. 
	 	
	NVIC_EnableIRQ(Timer1_IRQn);																							// Разрешение прерываний от UART1
}


int main(void)
{	
	CLK_Init();
	ConfigPortC();
	
	TIMER1_Init();
	
	MDR_PORTC->RXTX |= ((1<<0)|(1<<1)); //установка светодиодов на 0 и 1 порте С
	__enable_irq(); 										// Enable Interrupts
  MDR_TIMER1->CNTRL = 0x00000001;  //Счет вверх по TIM_CLK.
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
	MDR_TIMER1->CNTRL = 0x00000001;  //Счет вверх по TIM_CLK.
}

