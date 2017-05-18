//
// TIMER.C
// Written by J.W.Meissner
//

#include "stm32f4xx.h"

#include "hw.h"
#include "timer.h"

void TIM2_Configuration(void) {
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	//TIM_TimeBaseStructure.TIM_Period = 1749;  // 1 MHz down to 10 KHz (0.1 ms)
	//TIM_TimeBaseStructure.TIM_Prescaler = 24 - 1; // 24 MHz Clock down to 1 MHz (adjust per your clock)
	//TIM_TimeBaseStructure.TIM_ClockDivision = 1;
	TIM_TimeBaseStructure.TIM_Period = (84000000 / 200) - 1; // 200 KHz, from 84 MHz TIM2CLK (ie APB1 = HCLK/4, TIM2CLK = HCLK/2)
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update); // ADC_ExternalTrigConv_T2_TRGO
	TIM_Cmd(TIM2, ENABLE);
}
