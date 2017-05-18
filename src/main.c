#include "stm32f4_discovery.h"
#include "stm32f4xx.h"
#include <stdio.h>
#include "adc.h"
#include "dac.h"
#include "timer.h"

double a = 0.0f;
double b = 0;
double c = 0;
double d = 0;

double a1, a2, a3, b0, b1, b2, b3;
double x, y;
double w[4] = { 0, 0, 0, 0 };

void RCC_Configuration(void) {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
}

void SetupLEDS(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14
			| GPIO_Pin_15;
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
uint32_t ADC_ConvertedValue[2][BLOCKSIZE];
uint32_t OutputBuffer[2][BLOCKSIZE];

void DMA2_Stream0_IRQHandler(void) // Called at 1 KHz for 200 KHz sample rate, LED Toggles at 500 Hz
{

	if (DMA_GetITStatus(DMA2_Stream0, DMA_IT_HTIF0)) {
		DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_HTIF0);
		GPIO_ToggleBits(GPIOD, GPIO_Pin_13);
	}

	y = (b0 * x + w[0]) / (b + c + d + 1);
	w[0] = b1 * x + w[1] - a1 * y;
	w[1] = b2 * x + w[2] - a2 * y;
	w[2] = b3 * x - a3 * y;

	OutputBuffer[0][0] = y;
	//OutputBuffer[1][0] = 4095;
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

	//NVIC_Configuration();

	TIM2_Configuration(); //timer.c

	ADC1_Config(BLOCKSIZE,&ADC_ConvertedValue[0][0]);//adc.c
	DAC1_Config(BLOCKSIZE,&OutputBuffer[0][0]); //dac.c
	DAC2_Config(BLOCKSIZE,&ADC_ConvertedValue[0][0]); //dac.c

	SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2)); //enable FPU

	double Tp = 0.01f;
	double T1 = 1;
	double T2 = 3;
	double zeta = 0.3;
	double k = 1;

	a = 2 / Tp;
	b = a * a * a * T1 * T2 * T2;
	c = a * a * (T2 * T2 + 2 * zeta * T1 * T2);
	d = a * (T1 + 2 * zeta * T2);

	a1 = (3 + d - c - 3 * b);
	a2 = (3 - d - c + 3 * b);
	a3 = (1 - d + c - b);

	b0 = k;
	b1 = 3 * k;
	b2 = 3 * k;
	b3 = k;
	x = 2000;

	while (1) {
		//OutputBuffer[0][0] = 4095;
		//OutputBuffer[1][0] = 0;

		ADC_ConvertedValue;
		OutputBuffer;
	}
}
