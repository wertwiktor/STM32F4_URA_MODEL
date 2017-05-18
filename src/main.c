#include "stm32f4_discovery.h"
#include "stm32f4xx.h"
#include <stdio.h>
#include "adc.h"
#include "dac.h"
#include "timer.h"
#include "model.h"

void RCC_Configuration(void) {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE); //DMA1
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE); //DMA2
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//GPIOA
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);//GPIOD
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);	 //DAC
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //TIM2
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //ADC1
}

void SetupLEDS(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
}

void SetupGPIO(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_6 | GPIO_Pin_5 | GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

#define BLOCKSIZE 1
uint32_t InputBuffer[2][BLOCKSIZE];
uint32_t OutputBuffer[2][BLOCKSIZE];

void DMA2_Stream0_IRQHandler(void) //Przerwanie DMA2 - kana³ od ADC1
{

	if (DMA_GetITStatus(DMA2_Stream0, DMA_IT_HTIF0)) {
		DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_HTIF0);
		GPIO_ToggleBits(GPIOD, GPIO_Pin_13);
	}

	OutputBuffer[0][0] = calculateResponse(x);

	if (DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0)) {
		DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);
		GPIO_ToggleBits(GPIOD, GPIO_Pin_13);
	}
	GPIO_ToggleBits(GPIOD, GPIO_Pin_15);
}

int main(void) {

	RCC_Configuration();

	SetupLEDS();
	SetupGPIO();

	TIM2_Configuration(); //timer.c

	ADC1_Config(BLOCKSIZE,&InputBuffer[0][0]);//adc.c
	DAC1_Config(BLOCKSIZE,&OutputBuffer[0][0]); //dac.c
	DAC2_Config(BLOCKSIZE,&InputBuffer[0][0]); //dac.c

	SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2)); //enable FPU

	initializeParameters();

	while (1) {
		InputBuffer;
		OutputBuffer;
	}
}
